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
#include <include/Noriko/log.h>W


/**
 */
NK_NATIVE typedef struct __NkInt_WorldLayer {
    NKOM_IMPLEMENTS(NkILayer);

    NkIWindow          *mp_renderTarget; /**< cached reference to the render window */
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
    self->mp_renderTarget->VT->Release(self->mp_renderTarget);
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

        .mp_renderTarget = mainWnd,
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
    actWorldLayer->mp_renderTarget->VT->Release(actWorldLayer->mp_renderTarget);

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
    NK_UNREFERENCED_PARAMETER(self);
    NK_UNREFERENCED_PARAMETER(evPtr);

    /* Stub for now. */
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnRender(_Inout_ NkILayer *self, _In_ NkFloat aheadBy) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Get internal structure of world layer. */
    __NkInt_WorldLayer *actWorldLayer = (__NkInt_WorldLayer *)self;

    /* Draw world. */
    actWorldLayer->mp_rdRef->VT->DrawTexture(
        actWorldLayer->mp_rdRef,
        &(NkRectF){ 0, 0, 8 * 32, 8 * 32 },
        actWorldLayer->mp_mainTexAtlas,
        &(NkRectF){ 0, 0, 8 * 32, 8 * 32 }
    );

    /* All good. */
    return NkErr_Ok;
}


/**
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
    .mp_renderTarget = NULL,
    .mp_rdRef        = NULL,
    .mp_mainTexAtlas = NULL
};


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


