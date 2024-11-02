/**********************************************************************
 * Noriko - cross-platform 2-D role-playing game (RPG) game engine    *
 *          for desktop and mobile console platforms                  *
 *                                                                    *
 * (c) 2024 TophUwO <tophuwo01@gmail.com>. All rights reserved.       *
 *                                                                    *
 * The source code is licensed under the Apache License 2.0. Refer    *
 * to the LICENSE file in the root directory of this project. If this *
 * file is not present, visit                                         *
 *     https://www.apache.org/licenses/LICENSE-2.0                    *
 **********************************************************************/

/**
 * \file  wingdi.c
 * \brief implements a GDI-based renderer usable on any platform supporting Win32
 * 
 * Windows supports multiple rendering technologies. One of them is a legacy Win32 API
 * called GDI (Graphics Device Interface). It allows for low-level drawing application
 * onto a variety of devices, such as windows, bitmaps, etc. The renderer implemented by
 * this file is primarily intended to be (1) a fallback if all other rendering APIs are
 * unavailable, or (2) for development purposes as long as no other renderer is
 * implemented.
 */
#define NK_NAMESPACE "nk::rdgdi"


/* Noriko includes */
#include <include/Noriko/alloc.h>
#include <include/Noriko/renderer.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/log.h>
#include <include/Noriko/bmp.h>


/* All code is stripped from the compilation if we are not on Windows. */
#if (defined NK_TARGET_WINDOWS)
/** \cond INTERNAL */
/**
 * \class __NkInt_GdiRenderer
 * \brief represents the instance-specific internal state of the GDI-based renderer
 */
NK_NATIVE typedef struct __NkInt_GdiRenderer {
    NKOM_IMPLEMENTS(NkIGdiRenderer);

    NkOMRefCount             m_refCount; /**< reference count */
    NkIWindow               *mp_wndRef;  /**< reference to the Noriko window */
    NkRendererSpecification  m_initSpec; /**< initial specification */
    NkRendererSpecification  m_currSpec; /**< current renderer settings */
    
    /**
     * \struct __NkInt_GdiResources
     * \brief  represents the collection of basic resources used by the GDI renderer
     */
    struct __NkInt_GdiResources {
        HDC       mp_memDC;     /**< memory DC to render contents to */
        HDC       mp_texDC;     /**< DC holding the currently bound texture */
        HBITMAP   mp_memBmp;    /**< bitmap to render to */
        HBITMAP   mp_oldBmp;    /**< initial bitmap of the memory DC */
        HBITMAP   mp_defTexBmp; /**< default bitmap for the texture DC */
        HBRUSH    mp_clearBr;   /**< brush used for clearing the screen */
#if (!defined NK_CONFIG_DEPLOY)
        HBRUSH    m_vpBkgndBr;  /**< brush used for the viewport background */
#endif /* NK_CONFIG_DEPLOY */
        NkSize2D  m_bbDim;      /**< dimensions of the internal back buffer */
        NkPoint2D m_vpOri;      /**< viewport origin, in client space */
    } m_gdiRes;
} __NkInt_GdiRenderer;
/* Define IID and CLSID. */
// { F2CD4199-E8F2-45FF-89EC-14F8785AF2C6 }
NKOM_DEFINE_IID(NkIGdiRenderer, { 0xf2cd4199, 0xe8f2, 0x45ff, 0x89ec14f8785af2c6 });
// { 819653F5-28C1-4EDF-A49F-09613C47A5E6 }
NKOM_DEFINE_CLSID(NkIGdiRenderer, { 0x819653f5, 0x28c1, 0x4edf, 0xa49f09613c47a5e6 });


/**
 */
NK_INTERNAL int __NkInt_GdiRenderer_MapToStretchBltMode(_In_ NkTextureInterpolationMode iMode) {
    switch (iMode) {
        case NkTexIMd_Default:
        case NkTexIMd_NearestNeighbor: return COLORONCOLOR;
        case NkTexIMd_Bilinear:        return HALFTONE;
    }

    return -1;
}

/**
 * \todo free resources properly in case of an error 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_CreateBasicResources(
    _In_  NkRendererSpecification const *rdSpecs,
    _Out_ struct __NkInt_GdiResources *resPtr
) {
    NkErrorCode errCode = NkErr_Ok;

    /* Get some constant window properties. */
    HWND     wndHandle = rdSpecs->mp_wndRef->VT->QueryNativeWindowHandle(rdSpecs->mp_wndRef);
    HDC      wndDC     = GetDC(wndHandle);
    NkSize2D clDim     = rdSpecs->mp_wndRef->VT->GetClientDimensions(rdSpecs->mp_wndRef);

    /* Create resources. */
    HDC memDC, texDC;
    if ((memDC = CreateCompatibleDC(wndDC)) == NULL) {
        NK_LOG_ERROR("Could not create memory device context from window device context.");

        errCode = NkErr_CreateMemDC;
        goto lbl_END;
    }
    HBITMAP memBmp; 
    if ((memBmp = CreateCompatibleBitmap(wndDC, (int)clDim.m_width, (int)clDim.m_height)) == NULL) {
        NK_LOG_ERROR("Could not create memory bitmap compatible with window device context.");

        errCode = NkErr_CreateCompBitmap;
        goto lbl_END;
    }
    HBITMAP oldBmp = SelectObject(memDC, memBmp);
    if ((texDC = CreateCompatibleDC(wndDC)) == NULL) {
        NK_LOG_ERROR("Could not create texture device context.");

        errCode = NkErr_CreateMemDC;
        goto lbl_END;
    }
    HBRUSH clBrush = CreateSolidBrush(
        RGB(
            rdSpecs->m_clearCol.m_rVal,
            rdSpecs->m_clearCol.m_gVal,
            rdSpecs->m_clearCol.m_bVal
        )
    );
    if (clBrush == NULL) {
        NK_LOG_ERROR("Could not create clear color brush.");

        errCode = NkErr_CreateBrush;
        goto lbl_END;
    }
#if (!defined NK_CONFIG_DEPLOY)
    HBRUSH vpBrush = CreateSolidBrush(RGB(255, 255, 255));
    if (vpBrush == NULL) {
        NK_LOG_ERROR("Could not create viewport background brush.");

        errCode = NkErr_CreateBrush;
        DeleteObject(clBrush);
        goto lbl_END;
    }
#endif /* NK_CONFIG_DEPLOY */

    /* Initialize the fields. */
    *resPtr = (struct __NkInt_GdiResources){
        .mp_memDC     = memDC,
        .mp_texDC     = texDC,
        .mp_memBmp    = memBmp,
        .mp_oldBmp    = oldBmp,
        .mp_defTexBmp = (HBITMAP)GetCurrentObject(texDC, OBJ_BITMAP),
        .mp_clearBr   = clBrush,
#if (!defined NK_CONFIG_DEPLOY)
        .m_vpBkgndBr  = vpBrush,
#endif /* NK_CONFIG_DEPLOY */
        .m_bbDim      = clDim,
        .m_vpOri      = NkCalculateViewportOrigin(
            rdSpecs->m_vpAlignment,
            rdSpecs->m_vpExtents,
            rdSpecs->m_dispTileSize,
            clDim
        )
    };

    /* Set some DC properties. */
    SetStretchBltMode(resPtr->mp_memDC, __NkInt_GdiRenderer_MapToStretchBltMode(rdSpecs->m_texInterMode));

lbl_END:
    ReleaseDC(wndHandle, wndDC);
    return errCode;
}

/**
 */
NK_INTERNAL NkVoid __NkInt_GdiRenderer_Destroy(_Inout_ __NkInt_GdiRenderer *self) {
    NK_LOG_INFO("shutdown: GDI renderer");
    
    /*
     * Select the default texture. In an ideal world, the resources get deleted before
     * the renderer gets deleted, so this should really not do anything as deleting a
     * bound texture will automatically unbind it.
     */
    SelectObject(self->m_gdiRes.mp_texDC, self->m_gdiRes.mp_defTexBmp);
    /*
     * Select the old bitmap into the DC to free it when the DC is destroyed. Our actual
     * bitmap must be freed by us since it was not indirectly created by the memory DC
     * when it was created.
     */
    HGDIOBJ currBmp = SelectObject(self->m_gdiRes.mp_memDC, self->m_gdiRes.mp_oldBmp);
    /* Make sure we still have the correct bitmap. This should never fail. */
    NK_ASSERT(currBmp == self->m_gdiRes.mp_memBmp, NkErr_ObjectState);

    /* Destroy our memory bitmap, memory DC, and the other resources. */
    DeleteObject(self->m_gdiRes.mp_memBmp);
    DeleteObject(self->m_gdiRes.mp_clearBr);
#if (!defined NK_CONFIG_DEPLOY)
    DeleteObject(self->m_gdiRes.m_vpBkgndBr);
#endif /* NK_CONFIG_DEPLOY */
    DeleteDC(self->m_gdiRes.mp_memDC);
    DeleteDC(self->m_gdiRes.mp_texDC);

    /* Release the parent window. */
    self->mp_wndRef->VT->Release(self->mp_wndRef);
}

/**
 */
NK_INTERNAL NkVoid __NkInt_GdiRenderer_InternalDeleteResource(
    _Inout_ NkIRenderer *self,
    _Inout_ NkRendererResource *resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(resPtr != NULL, NkErr_InOutParameter);

    /* Get pointer to internal renderer structure. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;

    switch (resPtr->m_resType) {
        case NkRdResTy_Texture:
        case NkRdResTy_TextureMask:
            /* If the texture is currently bound to our texture DC, unbind it first. */
            if (GetCurrentObject(rdRef->m_gdiRes.mp_texDC, OBJ_BITMAP) == (HGDIOBJ)resPtr->m_resHandle)
                SelectObject(rdRef->m_gdiRes.mp_texDC, rdRef->m_gdiRes.mp_defTexBmp);

            DeleteObject((HGDIOBJ)resPtr->m_resHandle);
            break;
        default:
            NK_LOG_CRITICAL("Unknown resource type: %i", (int)resPtr->m_resType);
#pragma warning (suppress: 4127)
            NK_ASSERT_EXTRA(NK_FALSE, NkErr_InOutParameter, "Cannot delete resource of this type.");

            return;
    }

    /*
     * At last, release the reference to the renderer since the resource held a reference
     * to it.
     */
    resPtr->mp_rdRef->VT->Release(resPtr->mp_rdRef);
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_GdiRenderer_AppropriateResource(
    _Inout_        NkIRenderer *self,
    _Maybe_reinit_ NkRendererResource **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Delete old resource if needed. */
    if (*resPtr != NULL)
        __NkInt_GdiRenderer_InternalDeleteResource(self, *resPtr);
    else {
        /* Allocate new resource if it was NULL previously. */
        NkErrorCode errCode = NkPoolAlloc(NULL, sizeof **resPtr, 1, resPtr);

        if (errCode != NkErr_Ok)
            return errCode;
    }

    /* All good. */
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL NkIRenderer *__NkInt_GdiRenderer_RefInstance(_Inout_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    self->VT->AddRef(self);
    return self;
}


/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_GdiRenderer_AddRef(_Inout_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ++((__NkInt_GdiRenderer *)self)->m_refCount;
}

/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_GdiRenderer_Release(_Inout_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    if (--((__NkInt_GdiRenderer *)self)->m_refCount <= 0) {
        __NkInt_GdiRenderer_Destroy((__NkInt_GdiRenderer *)self);

        NkGPFree((NkVoid *)self);
        return 0;
    }

    return ((__NkInt_GdiRenderer *)self)->m_refCount;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_QueryInterface(
    _Inout_  NkIRenderer *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    NK_INTERNAL NkOMImplementationInfo const gl_ImplInfos[] = {
        { NKOM_IIDOF(NkIBase)          },
        { NKOM_IIDOF(NkIInitializable) },
        { NKOM_IIDOF(NkIRenderer)      },
        { NKOM_IIDOF(NkIGdiRenderer)   },
        { NULL                         }                      
    };
    if (NkOMQueryImplementationIndex(gl_ImplInfos, iId) != SIZE_MAX) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_GdiRenderer_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_Initialize(
    _Inout_     NkIRenderer *self,
    _Inout_opt_ NkVoid *initParam
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(initParam != NULL, NkErr_InParameter);

    NK_LOG_INFO("startup: GDI renderer");

    /* Get the pointer to the renderer specification. */
    NkRendererSpecification const *rdSpecs = (NkRendererSpecification const *)initParam;

    /* Initialize main resources. */
    NkErrorCode errCode;
    struct __NkInt_GdiResources gdiRes;
    if ((errCode = __NkInt_GdiRenderer_CreateBasicResources(rdSpecs, &gdiRes)) != NkErr_Ok)
        return errCode;
    rdSpecs->mp_wndRef->VT->AddRef(rdSpecs->mp_wndRef);

    /* Initialize renderer fields. */
    *(__NkInt_GdiRenderer *)self = (__NkInt_GdiRenderer){
        .NkIGdiRenderer_Iface.VT = self->VT,
        
        .m_refCount = ((__NkInt_GdiRenderer *)self)->m_refCount,
        .mp_wndRef  = rdSpecs->mp_wndRef,
        .m_initSpec = *rdSpecs,
        .m_currSpec = *rdSpecs
    };
    memcpy(&((__NkInt_GdiRenderer *)self)->m_gdiRes, &gdiRes, sizeof gdiRes);
    
    /* All good. */
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL NkRendererApi NK_CALL __NkInt_GdiRenderer_QueryRendererApi(_Inout_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    return NkRdApi_Win32GDI;
}

/**
 */
NK_INTERNAL NkRendererSpecification const *NK_CALL __NkInt_GdiRenderer_QuerySpecification(_Inout_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return &((__NkInt_GdiRenderer *)self)->m_initSpec;
}

/**
 */
NK_INTERNAL NkIWindow *NK_CALL __NkInt_GdiRenderer_QueryWindow(_Inout_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    NkIWindow *wndRef = ((__NkInt_GdiRenderer *)self)->mp_wndRef;

    wndRef->VT->AddRef(wndRef);
    return wndRef;
}

/**
 */
NK_INTERNAL NkSize2D NK_CALL __NkInt_GdiRenderer_QueryViewportDimensions(_In_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Get pointer to internal renderer state. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;

    /* Calculate current viewport dimensions. */
    return (NkSize2D) {
        rdRef->m_currSpec.m_dispTileSize.m_width  * rdRef->m_currSpec.m_vpExtents.m_width,
        rdRef->m_currSpec.m_dispTileSize.m_height * rdRef->m_currSpec.m_vpExtents.m_height
    };
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_Resize(
    _Inout_ NkIRenderer *self,
    _In_    NkSize2D clAreaSize
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Get pointer to internal renderer state. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;

    /* Delete the old bitmap first. */
    SelectObject(rdRef->m_gdiRes.mp_memDC, rdRef->m_gdiRes.mp_oldBmp);
    DeleteObject(rdRef->m_gdiRes.mp_memBmp);

    /* Create new bitmap with appropriate size. */
    HDC wndDC = GetDC((HWND)rdRef->mp_wndRef->VT->QueryNativeWindowHandle(rdRef->mp_wndRef));
    rdRef->m_gdiRes.mp_memBmp = CreateCompatibleBitmap(wndDC, (int)clAreaSize.m_width, (int)clAreaSize.m_height);
    if (rdRef->m_gdiRes.mp_memBmp == NULL) {
        NK_LOG_ERROR(
            "Failed to resize window back buffer. Requested Dimensions: (%llu, %llu)",
            clAreaSize.m_width,
            clAreaSize.m_height
        );

        return NkErr_CreateCompBitmap;
    }
    SelectObject(rdRef->m_gdiRes.mp_memDC, rdRef->m_gdiRes.mp_memBmp);
    ReleaseDC((HWND)rdRef->mp_wndRef->VT->QueryNativeWindowHandle(rdRef->mp_wndRef), wndDC);

    /* All went well. Update client size and recalculate viewport origin. */
    rdRef->m_gdiRes.m_bbDim = clAreaSize;
    rdRef->m_gdiRes.m_vpOri = NkCalculateViewportOrigin(
        rdRef->m_currSpec.m_vpAlignment,
        rdRef->m_currSpec.m_vpExtents,
        rdRef->m_currSpec.m_dispTileSize,
        rdRef->m_gdiRes.m_bbDim
    );
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_BeginDraw(_Inout_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Get pointer to renderer state. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;

    /* Clear the back buffer. */
    FillRect(
        rdRef->m_gdiRes.mp_memDC,
        &(RECT const){
            0,
            0,
            (LONG)rdRef->m_gdiRes.m_bbDim.m_width,
            (LONG)rdRef->m_gdiRes.m_bbDim.m_height
        },
        rdRef->m_gdiRes.mp_clearBr
    );
    
#if (!defined NK_CONFIG_DEPLOY)
    /* Draw background of viewport. */
    NkSize2D const vpDim = {
        rdRef->m_currSpec.m_vpExtents.m_width  * rdRef->m_currSpec.m_dispTileSize.m_width,
        rdRef->m_currSpec.m_vpExtents.m_height * rdRef->m_currSpec.m_dispTileSize.m_height
    };

    FillRect(
        rdRef->m_gdiRes.mp_memDC,
        &(RECT const){
            (LONG)rdRef->m_gdiRes.m_vpOri.m_xCoord,
            (LONG)rdRef->m_gdiRes.m_vpOri.m_yCoord,
            (LONG)(rdRef->m_gdiRes.m_vpOri.m_xCoord + vpDim.m_width),
            (LONG)(rdRef->m_gdiRes.m_vpOri.m_yCoord + vpDim.m_height),
        },
        rdRef->m_gdiRes.m_vpBkgndBr
    );
#endif /* NK_CONFIG_DEPLOY */

    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_EndDraw(_Inout_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Blit the back buffer to the window surface and wait for VBlank if necessary. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;

    HDC windowDC = GetDC(rdRef->mp_wndRef->VT->QueryNativeWindowHandle(rdRef->mp_wndRef));
    BitBlt(
        windowDC,
        0,
        0,
        (int)rdRef->m_gdiRes.m_bbDim.m_width,
        (int)rdRef->m_gdiRes.m_bbDim.m_height,
        rdRef->m_gdiRes.mp_memDC,
        0,
        0,
        SRCCOPY
    );
    ReleaseDC(rdRef->mp_wndRef->VT->QueryNativeWindowHandle(rdRef->mp_wndRef), windowDC);
    
    if (rdRef->m_currSpec.m_isVSync)
        DwmFlush();
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_DrawTexture(
    _Inout_  NkIRenderer *self,
    _In_     NkRectF const *dstRect,
    _In_     NkRendererResource const *texPtr,
    _In_opt_ NkRectF const *srcRect
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(dstRect != NULL, NkErr_InParameter);
    NK_ASSERT(texPtr != NULL && texPtr->m_resType == NkRdResTy_Texture, NkErr_InParameter);

    /* Get pointer to renderer structure. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;

    /*
     * To know which blit function we need to use, we must first determine if scaling is
     * needed. This may require us to first 'normalize' our source rectangle.
     */
    NkRectF normSrcRect;
    /* Get bitmap width and height if the source rectangle is not entirely "valid". */
    if (srcRect == NULL || srcRect->m_width == -1 || srcRect->m_height == -1) {
        BITMAP tInfo;
        GetObject((HANDLE)texPtr->m_resHandle, (int)sizeof(BITMAP), (NkVoid *)&tInfo);

        /* Normalize upper-left corner. */
        NkFloat const xCoord = srcRect ? srcRect->m_xCoord : 0.f;
        NkFloat const yCoord = srcRect ? srcRect->m_yCoord : 0.f;
        /* Normalize width and height. */
        NkFloat const width  = !srcRect
            ? tInfo.bmWidth
            : (srcRect->m_width < 0.f ? tInfo.bmWidth - srcRect->m_width : srcRect->m_width)
        ;
        NkFloat const height = !srcRect
            ? tInfo.bmHeight
            : (srcRect->m_height < 0.f ? tInfo.bmHeight - srcRect->m_height : srcRect->m_height)
        ;

        normSrcRect = (NkRectF){ xCoord, yCoord, width, height };
    } else normSrcRect = *srcRect;

    /* Bind the new bitmap. */
    if (GetCurrentObject(rdRef->m_gdiRes.mp_texDC, OBJ_BITMAP) != (HGDIOBJ)texPtr->m_resHandle)
        SelectObject(rdRef->m_gdiRes.mp_texDC, (HGDIOBJ)texPtr->m_resHandle);
    /*
     * Determine if scaling is needed by simply checking if the source and destination
     * rectangles are the same size, and draw the bitmap.
     */
    if (NkRendererCompareRectangles(&normSrcRect, dstRect) == NK_TRUE) {
        /*
         * Both rectangles are exactly the same size. Great, no scaling is required. That
         * should be the normal case.
         */
        BitBlt(
            rdRef->m_gdiRes.mp_memDC,
            (int)dstRect->m_xCoord + (int)rdRef->m_gdiRes.m_vpOri.m_xCoord,
            (int)dstRect->m_yCoord + (int)rdRef->m_gdiRes.m_vpOri.m_yCoord,
            (int)dstRect->m_width,
            (int)dstRect->m_height,
            rdRef->m_gdiRes.mp_texDC,
            (int)normSrcRect.m_xCoord,
            (int)normSrcRect.m_yCoord,
            SRCCOPY
        );
    } else {
        /* Fuck, scaling is required. Well, that sucks but what we gonna do? */
        StretchBlt(
            rdRef->m_gdiRes.mp_memDC,
            (int)dstRect->m_xCoord + (int)rdRef->m_gdiRes.m_vpOri.m_xCoord,
            (int)dstRect->m_yCoord + (int)rdRef->m_gdiRes.m_vpOri.m_yCoord,
            (int)dstRect->m_width,
            (int)dstRect->m_height,
            rdRef->m_gdiRes.mp_texDC,
            (int)normSrcRect.m_xCoord,
            (int)normSrcRect.m_yCoord,
            (int)normSrcRect.m_width,
            (int)normSrcRect.m_height,
            SRCCOPY
        );
    }
    
    /* All good. */
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_DrawMaskedTexture(
    _Inout_ NkIRenderer *self,
    _In_    NkRectF const *dstRect,
    _In_    NkRendererResource const *texPtr, 
    _In_    NkVec2F srcOff, 
    _In_    NkRendererResource const *maskPtr,
    _In_    NkVec2F maskOff
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(dstRect != NULL, NkErr_InParameter);
    NK_ASSERT(texPtr != NULL, NkErr_InParameter);
    NK_ASSERT(maskPtr != NULL, NkErr_InParameter);
    NK_ASSERT(texPtr->m_resType == NkRdResTy_Texture, NkErr_InParameter);
    NK_ASSERT(maskPtr->m_resType == NkRdResTy_TextureMask, NkErr_InParameter);

    /* Get pointer to renderer structure. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;
    /* Select the new texture into the texture slot. */
    if (GetCurrentObject(rdRef->m_gdiRes.mp_texDC, OBJ_BITMAP) != (HGDIOBJ)texPtr->m_resHandle)
        SelectObject(rdRef->m_gdiRes.mp_texDC, (HGDIOBJ)texPtr->m_resHandle);

    /* Draw the bitmap with the transparency information. */
    MaskBlt(
        rdRef->m_gdiRes.mp_memDC,
        (int)dstRect->m_xCoord + (int)rdRef->m_gdiRes.m_vpOri.m_xCoord,
        (int)dstRect->m_yCoord + (int)rdRef->m_gdiRes.m_vpOri.m_yCoord,
        (int)dstRect->m_width,
        (int)dstRect->m_height,
        rdRef->m_gdiRes.mp_texDC,
        (int)srcOff.m_xVal,
        (int)srcOff.m_yVal,
        (HBITMAP)maskPtr->m_resHandle,
        (int)maskOff.m_xVal,
        (int)maskOff.m_yVal,
        MAKEROP4(SRCCOPY, 0xAA0000)
    );

    /* All good. */
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_CreateTexture(
    _Inout_        NkIRenderer *self,
    _In_           NkDIBitmap const *dibPtr,
    _Maybe_reinit_ NkRendererResource **resourcePtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(dibPtr != NULL, NkErr_InParameter);

    /* Get pointer to renderer structure. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;

    /* Query bitmap specification. */
    NkBitmapSpecification const *bmSpecs = NkDIBitmapGetSpecification(dibPtr);
    /* Create device-dependent texture. */
    HBITMAP ddTex = CreateCompatibleBitmap(rdRef->m_gdiRes.mp_memDC, bmSpecs->m_bmpWidth, bmSpecs->m_bmpHeight);
    if (ddTex == NULL)
        return NkErr_CreateCompBitmap;

    /* Copy DIB pixels into the created DDB. */
    NkUint32 dibPxBufSize;
    NkByte *dibPixels = NkDIBitmapGetPixels(dibPtr, &dibPxBufSize);
    int cLinesCopied = SetDIBits(NULL, ddTex, 0, bmSpecs->m_bmpHeight, (NkVoid const *)dibPixels, &(BITMAPINFO const){
        .bmiHeader = {
            .biSize          = sizeof(BITMAPINFOHEADER),
            .biWidth         = bmSpecs->m_bmpWidth,
            .biHeight        = bmSpecs->m_bmpHeight,
            .biBitCount      = bmSpecs->m_bitsPerPx,
            .biPlanes        = 1,
            .biCompression   = BI_RGB,
            .biSizeImage     = (DWORD)dibPxBufSize,
            .biXPelsPerMeter = 0,
            .biYPelsPerMeter = 0,
            .biClrUsed       = 0,
            .biClrImportant  = 0
        }
    }, DIB_RGB_COLORS);
    if ((NkInt32)cLinesCopied ^ bmSpecs->m_bmpHeight) {
        /* None or not all scan lines were copied successfully; an error happened. */
        DeleteObject(ddTex);

        return NkErr_CreateDDBFromDIB;
    }

    /*
     * Initialize the result structure. But first, we must check if the result structure
     * is already valid. In such a case, we must first delete the old instance. This
     * allows us to reuse instances without having to reallocate memory all the time.
     */
    NkErrorCode errCode;
    if ((errCode = __NkInt_GdiRenderer_AppropriateResource(self, resourcePtr)) != NkErr_Ok) {
        DeleteObject(ddTex);

        return errCode;
    }

    /* (Re-)initialize result structure. */
    **resourcePtr = (NkRendererResource){
        .mp_rdRef    = __NkInt_GdiRenderer_RefInstance(self),
        .m_resType   = NkRdResTy_Texture,
        .m_resHandle = (NkRendererResourceHandle)ddTex,
        .m_resFlags  = 0
    };
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_CreateTextureMask(
    _Inout_        NkIRenderer *self,
    _In_           NkRendererResource const *texPtr,
    _In_           NkRgbaColor colKey,
    _Maybe_reinit_ NkRendererResource **resourcePtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(texPtr != NULL, NkErr_InParameter);
    NK_ASSERT(resourcePtr != NULL, NkErr_OutptrParameter);
    NK_ASSERT(texPtr->m_resType == NkRdResTy_Texture, NkErr_InParameter);

    /* Get pointer to renderer structure. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;
    /* Retrieve source bitmap properties. */
    BITMAP tInfo;
    GetObject((HANDLE)texPtr->m_resHandle, (int)sizeof(BITMAP), (NkVoid *)&tInfo);

    /* Create monochrome bitmap of the required size. */
    HBITMAP monoBmp = CreateBitmap(tInfo.bmWidth, tInfo.bmHeight, 1U, 1U, NULL);
    if (monoBmp == NULL)
        return NkErr_CreateCompBitmap;
    /* Setup two temporary DCs that are needed for the bitmask creation. */
    COLORREF oldCol = SetBkColor(rdRef->m_gdiRes.mp_texDC, RGB(colKey.m_rVal, colKey.m_gVal, colKey.m_bVal));
    HGDIOBJ  oldObj = SelectObject(rdRef->m_gdiRes.mp_texDC, (HGDIOBJ)texPtr->m_resHandle);
    HGDIOBJ  oldBmp = SelectObject(rdRef->m_gdiRes.mp_memDC, (HGDIOBJ)monoBmp);

    /*
     * Perform the blit operation. Since the DC that holds the source bitmap has the key
     * color set as its background color, when BitBlt is performed, this basically maps
     * the background color, that is the key color, to white pixels. Since MaskBlt()
     * expects transparent pixels to be mapped to black, we use the NOTSRCCOPY to invert
     * the colors as we write it to the mask bitmap.
     */
    BitBlt(rdRef->m_gdiRes.mp_memDC, 0, 0, tInfo.bmWidth, tInfo.bmHeight, rdRef->m_gdiRes.mp_texDC, 0, 0, NOTSRCCOPY);

    /* Restore the old DC settings. */
    SelectObject(rdRef->m_gdiRes.mp_memDC, oldBmp);
    SelectObject(rdRef->m_gdiRes.mp_texDC, oldObj);
    SetBkColor(rdRef->m_gdiRes.mp_texDC, oldCol);

    /* Create new resource, delete old if needed. */
    NkErrorCode errCode;
    if ((errCode = __NkInt_GdiRenderer_AppropriateResource(self, resourcePtr)) != NkErr_Ok)
        return errCode;
    /* (Re-)initialize new resource. */
    **resourcePtr = (NkRendererResource){
        .mp_rdRef    = __NkInt_GdiRenderer_RefInstance(self),
        .m_resType   = NkRdResTy_TextureMask,
        .m_resHandle = (NkRendererResourceHandle)monoBmp,
        .m_resFlags  = 0
    };
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_DeleteResource(
    _Inout_      NkIRenderer *self,
    _Uninit_ptr_ NkRendererResource **resourcePtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(resourcePtr != NULL && *resourcePtr != NULL, NkErr_InOutParameter);
    NK_ASSERT((*resourcePtr)->mp_rdRef == self, NkErr_InOutParameter);

    /* Delete the resource on the device first. */
    __NkInt_GdiRenderer_InternalDeleteResource(self, *resourcePtr);

    /* Deallocate memory on host. */
    NkPoolFree(*resourcePtr);
    *resourcePtr = NULL;
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_GrabFramebuffer(
    _Inout_ NkIRenderer *self,
    _Out_   NkDIBitmap *resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(resPtr != NULL, NkErr_InOutParameter);

    NkErrorCode errCode = NkErr_Ok;
    /* Get pointer to renderer structure. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;

    /* Create bitmap of the appropriate size that will hold the framebuffer. */
    errCode = NkDIBitmapCreate(&(NkBitmapSpecification){
        .m_structSize = sizeof(NkBitmapSpecification),
        .m_bmpWidth   = (NkInt32)rdRef->m_gdiRes.m_bbDim.m_width,
        .m_bmpHeight  = (NkInt32)rdRef->m_gdiRes.m_bbDim.m_height,
        .m_bitsPerPx  = 32,
        .m_bmpFlags   = NkBmpFlag_Flipped
    }, NULL, resPtr);
    if (errCode != NkErr_Ok) {
        /* Failed to create the bitmap. */
        NK_LOG_ERROR(
            "Could not create DIB for screenshot. Reason: %s (%i)",
            NkGetErrorCodeStr(errCode)->mp_dataPtr,
            errCode
        );

        return errCode;
    }

    /* Make sure that all pending draw commands have been written to the device. */
    GdiFlush();
    /*
     * Before we can copy the pixels, we must unbind the framebuffer from the rendering
     * device context.
     * See: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/nf-wingdi-getdibits
     */
    HGDIOBJ currFB = SelectObject(rdRef->m_gdiRes.mp_memDC, rdRef->m_gdiRes.mp_oldBmp);

    /* Get pointer to DIB pixel buffer. */
    NkUint32 pxBufSz; 
    LPVOID pxBuf = (LPVOID)NkDIBitmapGetPixels(resPtr, &pxBufSz);
    /* Copy the current framebuffer state. */
    int cLinesCopied = GetDIBits(
        rdRef->m_gdiRes.mp_memDC,
        (HBITMAP)currFB,
        0U,
        (UINT)rdRef->m_gdiRes.m_bbDim.m_height,
        pxBuf,
        &(BITMAPINFO){
            .bmiHeader = {
                .biSize          = sizeof(BITMAPINFOHEADER),
                .biWidth         = (LONG)rdRef->m_gdiRes.m_bbDim.m_width,
                .biHeight        = (LONG)rdRef->m_gdiRes.m_bbDim.m_height,
                .biBitCount      = 32,
                .biPlanes        = 1,
                .biCompression   = BI_RGB,
                .biSizeImage     = (DWORD)pxBufSz,
                .biXPelsPerMeter = 0,
                .biYPelsPerMeter = 0,
                .biClrUsed       = 0,
                .biClrImportant  = 0
            }
        },
        DIB_RGB_COLORS
    );
    if ((NkUint64)cLinesCopied ^ rdRef->m_gdiRes.m_bbDim.m_height) {
        /* There was an error copying the DDB pixels. */
        NK_LOG_ERROR("There was an error copying framebuffer DDB pixels to the DIB.");

        goto lbl_END;
    }

lbl_END:
    /* Reselect the framebuffer into the rendering device context. */
    SelectObject(rdRef->m_gdiRes.mp_memDC, currFB);

    return errCode;
}


/**
 * \brief VTable for the NkIGdiRenderer class 
 */
NKOM_DEFINE_VTABLE(NkIRenderer) {
    .QueryInterface          = &__NkInt_GdiRenderer_QueryInterface,
    .AddRef                  = &__NkInt_GdiRenderer_AddRef,
    .Release                 = &__NkInt_GdiRenderer_Release,
    .Initialize              = &__NkInt_GdiRenderer_Initialize,
    .QueryRendererApi        = &__NkInt_GdiRenderer_QueryRendererApi,
    .QuerySpecification      = &__NkInt_GdiRenderer_QuerySpecification,
    .QueryWindow             = &__NkInt_GdiRenderer_QueryWindow,
    .QueryViewportDimensions = &__NkInt_GdiRenderer_QueryViewportDimensions,
    .Resize                  = &__NkInt_GdiRenderer_Resize,
    .BeginDraw               = &__NkInt_GdiRenderer_BeginDraw,
    .EndDraw                 = &__NkInt_GdiRenderer_EndDraw,
    .DrawTexture             = &__NkInt_GdiRenderer_DrawTexture,
    .DrawMaskedTexture       = &__NkInt_GdiRenderer_DrawMaskedTexture,
    .CreateTexture           = &__NkInt_GdiRenderer_CreateTexture,
    .CreateTextureMask       = &__NkInt_GdiRenderer_CreateTextureMask,
    .DeleteResource          = &__NkInt_GdiRenderer_DeleteResource,
    .GrabFramebuffer         = &__NkInt_GdiRenderer_GrabFramebuffer
};

/**
 * \brief defines the implementation details of the GDI renderer class (for exposure
 *        to the global renderer factory)
 */
NkOMImplementationInfo const __gl_GdiRdImplInfo__ = {
    .mp_uuidRef       = NKOM_CLSIDOF(NkIGdiRenderer),
    .m_structSize     = sizeof(__NkInt_GdiRenderer),
    .m_isAggSupported = NK_FALSE,
    .mp_vtabPtr       = (NkVoid *)&NKOM_VTABLEOF(NkIRenderer)
};
/** \endcond */
#endif /* NK_TARGET_WINDOWS */


#undef NK_NAMESPACE


