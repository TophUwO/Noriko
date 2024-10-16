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
 * \file  world.h
 * \brief defines the public API for the 'world layer', that is, the layer that manages
 *        and displays the world and all in-game objects
 */
#define NK_NAMESPACE "nk::world"


/* Noriko includes */
#include <include/Noriko/world.h>
#include <include/Noriko/layer.h>
#include <include/Noriko/nkom.h>
#include <include/Noriko/renderer.h>
#include <include/Noriko/window.h>
#include <include/Noriko/layer.h>
#include <include/Noriko/log.h>


/** \cond INTERNAL */
/**
 */
NK_NATIVE typedef struct __NkInt_WorldLayer {
    NKOM_IMPLEMENTS(NkILayer);

    NkIWindow          *mp_rdTarget;     /**< cached reference to the render window */
    NkIRenderer        *mp_rdRef;        /**< cached reference to the window's renderer */
    NkRendererResource *mp_mainTexAtlas; /**< texture atlas for world, etc. */
} __NkInt_WorldLayer;


/**
 */
NK_INTERNAL NkVoid __NkInt_WorldLayer_DeleteResources(_Inout_ __NkInt_WorldLayer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Delete resources. */
    self->mp_rdRef->VT->DeleteResource(self->mp_rdRef, &self->mp_mainTexAtlas);

    /* Release renderer and window. */
    self->mp_rdRef->VT->Release(self->mp_rdRef);
    self->mp_rdTarget->VT->Release(self->mp_rdTarget);
}

/**
 */
NK_INTERNAL NkVoid __NkInt_WorldLayer_ActionScreenshot(_Inout_ __NkInt_WorldLayer *self, char const *filePath) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(filePath != NULL && *filePath ^ '\0', NkErr_InParameter);

    /* Retrieve current client area dimensions. */
    NkSize2D const currDim = self->mp_rdTarget->VT->GetClientDimensions(self->mp_rdTarget);
    /* Create device-independent bitmap to hold the current framebuffer's contents. */
    NkDIBitmap newScreenshot;
    NkErrorCode errCode = NkDIBitmapCreate(&(NkBitmapSpecification const) {
        .m_structSize = sizeof(NkBitmapSpecification),
            .m_bmpWidth   = (NkInt32)currDim.m_width,
            .m_bmpHeight  = (NkInt32)currDim.m_height,
            .m_bitsPerPx  = 32
    }, NULL, &newScreenshot);
    if (errCode != NkErr_Ok) {
        /* Failed to create DI bitmap. */
        NK_LOG_ERROR(
            "Failed to create device-independent bitmap for receiving the pixel buffer. Reason: %s (%i)",
            NkGetErrorCodeStr(errCode)->mp_dataPtr,
            (int)errCode
        );

        return;
    }

    /* Grab the pixel data from the framebuffer. */
    if ((errCode = self->mp_rdRef->VT->GrabFramebuffer(self->mp_rdRef, &newScreenshot)) != NkErr_Ok) {
        /* Failed to copy pixels. */
        NK_LOG_ERROR(
            "Failed to grab current framebuffer. Reason: %s (%i)",
            NkGetErrorCodeStr(errCode)->mp_dataPtr,
            (int)errCode
        );

        NkDIBitmapDestroy(&newScreenshot);
        return;
    }

    /*
     * Save the bitmap to the file. We use one filePath for now, can only save the
     * latest screenshot.
     * TODO: Generate random number or use timestamp for screenshot file names.
     */
    if ((errCode = NkDIBitmapSave(&newScreenshot, filePath)) != NkErr_Ok)
        /* Failed to write bitmap file to disk. */
        NK_LOG_ERROR(
            "Failed to write screenshot file to \"%s\". Reason: %s (%i)",
            filePath,
            NkGetErrorCodeStr(errCode)->mp_dataPtr,
            (int)errCode
        );

    NK_LOG_INFO("Successfully wrote screenshot to \"%s\".", filePath);
    /* At last, delete bitmap. */
    NkDIBitmapDestroy(&newScreenshot);
}


/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WorldLayer_AddRef(_Inout_ NkILayer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WorldLayer_Release(_Inout_ NkILayer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Same as above. */
    return 1;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_QueryInterface(
    _Inout_  NkILayer *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    /* Define interface table for this class. */
    NK_INTERNAL NkOMImplementationInfo const gl_ImplInfos[] = {
        { NKOM_IIDOF(NkIBase)  },
        { NKOM_IIDOF(NkILayer) },
        { NULL                 }
    };

    if (NkOMQueryImplementationIndex(gl_ImplInfos, iId) != SIZE_MAX) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_WorldLayer_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnPush(
    _Inout_  NkILayer *self,
    _In_opt_ NkILayer const *beforeRef,
    _In_opt_ NkILayer const *afterRef,
    _In_     NkSize index
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(index);
    NK_UNREFERENCED_PARAMETER(beforeRef);
    NK_UNREFERENCED_PARAMETER(afterRef);

    /* Get internal structure of world layer. */
    __NkInt_WorldLayer *actWorldLayer = (__NkInt_WorldLayer *)self;

    /* Query renderer and create resources. */
    NkIWindow   *mainWnd = NkWindowQueryInstance();
    NkIRenderer *mainRd  = mainWnd->VT->GetRenderer(mainWnd);

    /* Create main resources. */
    NkDIBitmap mainTs;
    NkRendererResource *mainTsRes = NULL;
    NkErrorCode errCode = NkDIBitmapLoad("../res/def/ts_main.bmp", &mainTs);
    if (errCode != NkErr_Ok)
        goto lbl_ONERROR;
    if ((errCode = mainRd->VT->CreateTexture(mainRd, &mainTs, &mainTsRes)) != NkErr_Ok) {
        NkDIBitmapDestroy(&mainTs);

        goto lbl_ONERROR;
    }
    NkDIBitmapDestroy(&mainTs);

    /* Initialize instance. */
    *actWorldLayer = (__NkInt_WorldLayer){
        .NkILayer_Iface = actWorldLayer->NkILayer_Iface,

        .mp_rdTarget     = mainWnd,
        .mp_rdRef        = mainRd,
        .mp_mainTexAtlas = mainTsRes
    };
    /* All good. */
    return NkErr_Ok;

lbl_ONERROR:
    mainRd->VT->Release(mainRd);
    mainWnd->VT->Release(mainWnd);

    return errCode;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnPop(_Inout_ NkILayer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Get internal structure of world layer. */
    __NkInt_WorldLayer *actWorldLayer = (__NkInt_WorldLayer *)self;

    /* Destroy resources. */
    actWorldLayer->mp_rdRef->VT->DeleteResource(actWorldLayer->mp_rdRef, &actWorldLayer->mp_mainTexAtlas);
    /* Release components. */
    actWorldLayer->mp_rdRef->VT->Release(actWorldLayer->mp_rdRef);
    actWorldLayer->mp_rdTarget->VT->Release(actWorldLayer->mp_rdTarget);

    /* All good. */
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnEvent(
    _Inout_ NkILayer *self,
    _In_    NkEvent const *evPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(evPtr != NULL, NkErr_InParameter);

    /* Allow taking screenshots. */
    if (evPtr->m_evType == NkEv_KeyboardKeyDown && evPtr->m_kbEvent.m_vKeyCode == NkKey_F11) {
        __NkInt_WorldLayer_ActionScreenshot((__NkInt_WorldLayer *)self, "latestScreenshot.bmp");

        /* Event was handled. */
        return NkErr_Ok;
    }

    /* Event was not handled. */
    return NkErr_NoOperation;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnRender(_Inout_ NkILayer *self, _In_ NkFloat aheadBy) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(aheadBy);

    /* Get internal structure of world layer. */
    __NkInt_WorldLayer *actWorldLy = (__NkInt_WorldLayer *)self;

    /* Draw world. */
    actWorldLy->mp_rdRef->VT->DrawTexture(
        actWorldLy->mp_rdRef,
        &(NkRectF){ 0, 0, 8 * 32, 8 * 32 },
        actWorldLy->mp_mainTexAtlas,
        &(NkRectF){ 0, 0, 8 * 32, 8 * 32 }
    );

    /* All good. */
    return NkErr_Ok;
}


/**
 * \brief actual world layer instance
 */
NK_INTERNAL __NkInt_WorldLayer gl_WorldLayer = {
    .NkILayer_Iface = {
        .VT = &(struct __NkILayer_VTable__){
            .AddRef         = &__NkInt_WorldLayer_AddRef,
            .Release        = &__NkInt_WorldLayer_Release,
            .QueryInterface = &__NkInt_WorldLayer_QueryInterface,
            .OnPush         = &__NkInt_WorldLayer_OnPush,
            .OnPop          = &__NkInt_WorldLayer_OnPop,
            .OnEvent        = &__NkInt_WorldLayer_OnEvent,
            .OnRender       = &__NkInt_WorldLayer_OnRender
        }
    },
    .mp_rdTarget     = NULL,
    .mp_rdRef        = NULL,
    .mp_mainTexAtlas = NULL
};
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkWorldStartup(NkVoid) {
    NK_LOG_INFO("startup: world layer");

    /*
     * Push the world layer to the end of the layer stack so that it's updated last but
     * rendered first.
     */
    return NkLayerstackPush((NkILayer *)&gl_WorldLayer, NK_AS_NORMAL);
}

_Return_ok_ NkErrorCode NK_CALL NkWorldShutdown(NkVoid) {
    NK_LOG_INFO("shutdown: world layer");

    /* Retrieve the layer index. */
    NkSize worldLayerIndex = NkLayerstackQueryIndex((NkILayer *)&gl_WorldLayer);
    if (worldLayerIndex == SIZE_MAX)
        return NkErr_ArrayElemOutOfBounds;

    /* Delete all resources. */
    __NkInt_WorldLayer_DeleteResources((__NkInt_WorldLayer *)NkLayerstackPop(worldLayerIndex));
    return NkErr_Ok;
}


#undef NK_NAMESPACE


