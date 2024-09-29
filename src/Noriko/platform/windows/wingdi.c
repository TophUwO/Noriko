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
        HDC      mp_memDC;   /**< memory DC to render contents to */
        HBITMAP  mp_memBmp;  /**< bitmap to render to */
        HBITMAP  mp_oldBmp;  /**< initial bitmap of the memory DC */
        HBRUSH   mp_clearBr; /**< brush used for clearing the screen */
        NkSize2D m_bbDim;    /**< dimensions of the internal back buffer */
    } m_gdiRes;
} __NkInt_GdiRenderer;
/* Define IID and CLSID. */
// { F2CD4199-E8F2-45FF-89EC-14F8785AF2C6 }
NKOM_DEFINE_IID(NkIGdiRenderer, { 0xf2cd4199, 0xe8f2, 0x45ff, 0x89ec14f8785af2c6 });
// { 819653F5-28C1-4EDF-A49F-09613C47A5E6 }
NKOM_DEFINE_CLSID(NkIGdiRenderer, { 0x819653f5, 0x28c1, 0x4edf, 0xa49f09613c47a5e6 });


/**
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
    HDC memDC;
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

    /* Initialize the fields. */
    *resPtr = (struct __NkInt_GdiResources){
        .mp_memDC   = memDC,
        .mp_memBmp  = memBmp,
        .mp_oldBmp  = oldBmp,
        .mp_clearBr = clBrush,
        .m_bbDim    = clDim
    };

lbl_END:
    //ReleaseDC(wndHandle, wndDC);
    return errCode;
}

/**
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_GdiRenderer_Destroy(_Inout_ __NkInt_GdiRenderer *self) {
    /*
     * Select the old bitmap into the DC to free it when the DC is destroyed. Our actual
     * bitmap must be freed by us since it was not indirectly created by the memory DC
     * when it was created.
     */
    HGDIOBJ currBmp = SelectObject(self->m_gdiRes.mp_memDC, self->m_gdiRes.mp_oldBmp);
    /* Make sure we still have the correct bitmap. This should never fail. */
    NK_ASSERT(currBmp == self->m_gdiRes.mp_memBmp, NkErr_ObjectState);

    /* Destroy our memory bitmap and memory DC. */
    DeleteObject(self->m_gdiRes.mp_memBmp);
    DeleteDC(self->m_gdiRes.mp_memDC);

    /* Release the parent window. */
    self->mp_wndRef->VT->Release(self->mp_wndRef);
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

    /* All went well. */
    rdRef->m_gdiRes.m_bbDim = clAreaSize;
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_GdiRenderer_BeginDraw(_Inout_ NkIRenderer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Clear the back buffer. */
    __NkInt_GdiRenderer *rdRef = (__NkInt_GdiRenderer *)self;

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
NKOM_DEFINE_VTABLE(NkIRenderer) {
    .QueryInterface     = &__NkInt_GdiRenderer_QueryInterface,
    .AddRef             = &__NkInt_GdiRenderer_AddRef,
    .Release            = &__NkInt_GdiRenderer_Release,
    .Initialize         = &__NkInt_GdiRenderer_Initialize,
    .QueryRendererApi   = &__NkInt_GdiRenderer_QueryRendererApi,
    .QuerySpecification = &__NkInt_GdiRenderer_QuerySpecification,
    .QueryWindow        = &__NkInt_GdiRenderer_QueryWindow,
    .Resize             = &__NkInt_GdiRenderer_Resize,
    .BeginDraw          = &__NkInt_GdiRenderer_BeginDraw,
    .EndDraw            = &__NkInt_GdiRenderer_EndDraw
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


