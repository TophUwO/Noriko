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
 * \file  layer.c
 * \brief implements the layer stack
 * 
 * In Noriko, applications are layered, that is, there are different organizational units
 * that make up the game. A layer defines a point in the engine's event handling chain.
 * By default, there is a world layer, a HUD layer, and a GUI layer. Additional layers
 * can be added through the use of plug-ins in any position. There may be additional
 * debug layers inserted at specific positions.<br>
 * User input and asynchronous message handling is done through a message-pump, 'pumping
 * out' and dispatching events to the layer stack. Each layer gets the chance to process
 * the event. If the layer chooses to process the event, layers underneath it in the
 * stack will not receive the event. If the layer does not process the event, the event
 * is propagated through the layer stack until the entire stack has been traversed or the
 * event has been handled.
 */
#define NK_NAMESPACE "nk::layer"


/* Noriko includes */
#include <include/Noriko/util.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/layer.h>
#include <include/Noriko/log.h>
#include <include/Noriko/comp.h>

#include <include/Noriko/dstruct/vector.h>


/* Define NkILayer IID and CLSID. */
// { 50EBA425-5F11-4FDB-9C29-2E0EB49D3204 }
NKOM_DEFINE_IID(NkILayer, { 0x50eba425, 0x5f11, 0x4fdb, 0x9c292e0eb49d3204 });
// { 00000000-0000-0000-0000-000000000000 }
NKOM_DEFINE_CLSID(NkILayer, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });


/** \cond INTERNAL */
/**
 * \struct __NkInt_LayerStack
 * \brief  represents the internal state of the global layer stack
 */
NK_NATIVE typedef struct __NkInt_LayerStack {
    NK_DECL_LOCK(m_mtxLock);  /**< synchronization primitive */

    NkVector *mp_layerStack;  /**< ordered layer array */
} __NkInt_LayerStack;

/**
 * \brief actual instance of the global layer stack 
 */
NK_INTERNAL __NkInt_LayerStack gl_LayerStack;


/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(Layerstack)(NkVoid) {
    NK_ASSERT(gl_LayerStack.mp_layerStack == NULL, NkErr_ComponentState);

    /* Initialize the layer vector. */
    NkErrorCode errCode = NkVectorCreate(&(NkVectorProperties) {
        .m_structSize = sizeof(NkVectorProperties),
            .m_initialCap = 8,
            .m_minCap     = 8,
            .m_maxCap     = SIZE_MAX - 2,
            .m_growFactor = 1.5f
    }, NULL, &gl_LayerStack.mp_layerStack);
    if (errCode != NkErr_Ok)
        return errCode;

    /* Initialize the synchronization primitive. */
    if (NK_INITLOCK(gl_LayerStack.m_mtxLock) != thrd_success) {
        NkVectorDestroy(&gl_LayerStack.mp_layerStack);

        return NkErr_SynchInit;
    }
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(Layerstack)(NkVoid) {
    NK_ASSERT(gl_LayerStack.mp_layerStack != NULL, NkErr_ComponentState);

    /* Destroy the synchronization primitive. */
    NK_DESTROYLOCK(gl_LayerStack.m_mtxLock);
    /* Destroy the layer vector, releasing all layers still present in the stack. */
    NkVectorDestroy(&gl_LayerStack.mp_layerStack);

    return NkErr_Ok;
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkLayerstackPush(_Inout_ NkILayer *layerRef, _In_ NkSize whereInd) {
    NK_ASSERT(layerRef != NULL, NkErr_InOutParameter);
    NK_ASSERT(gl_LayerStack.mp_layerStack != NULL, NkErr_ComponentState);

    NK_LOCK(gl_LayerStack.m_mtxLock);
    /*
     * Invoke the 'OnPush' callback. If this callback returns non-zero, do not push the
     * layer.
     */
    NkILayer const *beforeRef = NkVectorGetElementCount(gl_LayerStack.mp_layerStack) == 0 || whereInd == 0
        ? NULL
        : NkVectorAt(gl_LayerStack.mp_layerStack, whereInd - 1)
    ;
    NkILayer const *afterRef = NkVectorGetElementCount(gl_LayerStack.mp_layerStack) == 0
        || whereInd + 1 >= NkVectorGetElementCount(gl_LayerStack.mp_layerStack)
            ? NULL
            : NkVectorAt(gl_LayerStack.mp_layerStack, whereInd + 1)
    ;
    NK_UNLOCK(gl_LayerStack.m_mtxLock);
    NkErrorCode errCode = layerRef->VT->OnPush(layerRef, beforeRef, afterRef, whereInd);
    if (errCode != NkErr_Ok)
        return errCode;

    /*
     * 'OnPush' callback didn't fail, so we insert the layer. If the addition succeeded,
     * increase the ref-count of the layer.
     */
    NK_LOCK(gl_LayerStack.m_mtxLock);
    errCode = NkVectorInsert(
        gl_LayerStack.mp_layerStack,
        (NkVoid const *)layerRef,
        whereInd != NK_AS_NORMAL
            ? whereInd
            : NkVectorGetElementCount(gl_LayerStack.mp_layerStack)
    );
    NK_UNLOCK(gl_LayerStack.m_mtxLock);
    if (errCode == NkErr_Ok)
        layerRef->VT->AddRef(layerRef);
    return errCode;
}

NkILayer *NK_CALL NkLayerstackPop(_In_ NkSize whereInd) {
    NK_ASSERT(gl_LayerStack.mp_layerStack != NULL, NkErr_ComponentState);
    
    NkILayer *layerRef;
    /*
     * Remove the layer from the stack. The layer stack vector will not destroy the layer,
     * so it will return the layer in 'layerRef'.
     */
    NK_SYNCHRONIZED(gl_LayerStack.m_mtxLock, {
        NK_IGNORE_RETURN_VALUE(NkVectorErase(gl_LayerStack.mp_layerStack, whereInd, &layerRef));
    });

    return layerRef;
}

_Return_ok_ NkErrorCode NK_CALL NkLayerstackOnEvent(_In_ NkEvent const *evPtr) {
    NK_ASSERT(evPtr != NULL, NkErr_InParameter);
    NK_ASSERT(gl_LayerStack.mp_layerStack != NULL, NkErr_ComponentState);

    NK_LOCK(gl_LayerStack.m_mtxLock);
    NkBoolean eventWasHandled = NK_FALSE;
    NkSize const layerCount = NkVectorGetElementCount(gl_LayerStack.mp_layerStack);
    for (NkSize i = 0; i < layerCount && !eventWasHandled; i++) {
        NkILayer *currLayer = NkVectorAt(gl_LayerStack.mp_layerStack, i);
        NK_ASSERT(currLayer != NULL, NkErr_ObjectState);

        NK_UNLOCK(gl_LayerStack.m_mtxLock);
        /*
         * Call the event handler for the current layer. If the return value is NkErr_Ok,
         * we assume the event was handled in which case the event is not propagated
         * further down the stack.
         */
        eventWasHandled = currLayer->VT->OnEvent(currLayer, evPtr) == NkErr_Ok;
        NK_LOCK(gl_LayerStack.m_mtxLock);
    }

    NK_UNLOCK(gl_LayerStack.m_mtxLock);
    return eventWasHandled ? NkErr_Ok : NkErr_NoOperation;
}

_Return_ok_ NkErrorCode NK_CALL NkLayerstackOnUpdate(_In_ NkFloat updTime) {
    NK_ASSERT(gl_LayerStack.mp_layerStack != NULL, NkErr_ComponentState);

    NK_LOCK(gl_LayerStack.m_mtxLock);
    NkSize const layerCount = NkVectorGetElementCount(gl_LayerStack.mp_layerStack);
    for (NkSize i = 0; i < layerCount; i++) {
        NkILayer *currLayer = NkVectorAt(gl_LayerStack.mp_layerStack, i);
        NK_ASSERT(currLayer != NULL, NkErr_ObjectState);

        NK_UNLOCK(gl_LayerStack.m_mtxLock);
        currLayer->VT->OnUpdate(currLayer, updTime);
        NK_LOCK(gl_LayerStack.m_mtxLock);
    }

    NK_UNLOCK(gl_LayerStack.m_mtxLock);
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkLayerstackOnRender(_In_ NkFloat aheadBy) {
    NK_ASSERT(gl_LayerStack.mp_layerStack != NULL, NkErr_ComponentState);

    NK_LOCK(gl_LayerStack.m_mtxLock);
    /*
     * Iterate over the layer-stack backwards, that is, from the lowest layer to the
     * top-most layer.
     */
    NkSize const layerCount = NkVectorGetElementCount(gl_LayerStack.mp_layerStack);
    if (layerCount == 0)
        goto lbl_END;
    for (NkInt64 i = (NkInt64)layerCount - 1; i >= 0; i--) {
        NkILayer *currLayer = NkVectorAt(gl_LayerStack.mp_layerStack, i);
        NK_ASSERT(currLayer != NULL, NkErr_ObjectState);

        NK_UNLOCK(gl_LayerStack.m_mtxLock);
        /*
         * Call the render method of the current layer. The return value is ignored.
         * Layers must be able to be drawn independently from each other.
         */
        NK_IGNORE_RETURN_VALUE(currLayer->VT->OnRender(currLayer, aheadBy));
        NK_LOCK(gl_LayerStack.m_mtxLock);
    }

lbl_END:
    NK_UNLOCK(gl_LayerStack.m_mtxLock);
    return layerCount == 0 ? NkErr_NoOperation : NkErr_Ok;
}

NkSize NK_CALL NkLayerstackQueryIndex(_In_ NkILayer const *layerRef) {
    NK_ASSERT(layerRef != NULL, NkErr_InOutParameter);

    NK_LOCK(gl_LayerStack.m_mtxLock);
    NkSize const layerCount = NkVectorGetElementCount(gl_LayerStack.mp_layerStack);
    for (NkSize i = 0; i < layerCount; i++)
        if (NkVectorAt(gl_LayerStack.mp_layerStack, i) == layerRef) {
            NK_UNLOCK(gl_LayerStack.m_mtxLock);

            return i;
        }
    NK_UNLOCK(gl_LayerStack.m_mtxLock);

    /* Layer could not be found. */
    return SIZE_MAX;
}


/** \cond INTERNAL */
/**
 * \brief component instance of the global layerstack
 */
NK_COMPONENT_DEFINE(Layerstack) {
    .m_compUuid     = { 0x2c7a0c9e, 0xc799, 0x4480, 0xa87760d83c8a1549 },
    .mp_clsId       = NULL,
    .m_compIdent    = NK_MAKE_STRING_VIEW("layerstack"),
    .m_compFlags    = 0,
    .m_isNkOM       = NK_FALSE,

    .mp_fnQueryInst = NULL,
    .mp_fnStartup   = &NK_COMPONENT_STARTUPFN(Layerstack),
    .mp_fnShutdown  = &NK_COMPONENT_SHUTDOWNFN(Layerstack)
};
/** \endcond */


#undef NK_NAMESPACE


