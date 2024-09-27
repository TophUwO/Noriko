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

#include <include/Noriko/dstruct/vector.h>


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
NK_INTERNAL NkVoid NK_CALL __NkInt_OnLayerStackDestroy(_Inout_ NkILayer *layerRef) {
    if (layerRef != NULL)
        layerRef->VT->Release(layerRef);
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkLayerstackStartup(NkVoid) {
    NK_ASSERT(gl_LayerStack.mp_layerStack == NULL, NkErr_ComponentState);

    /* Initialize the layer vector. */
    NkErrorCode errCode = NkVectorCreate(&(NkVectorProperties) {
        .m_structSize = sizeof(NkVectorProperties),
        .m_initialCap = 8,
        .m_minCap     = 8,
        .m_maxCap     = SIZE_MAX - 2,
        .m_growFactor = 1.5f
    }, (NkVoid (NK_CALL *)(NkVoid *))&__NkInt_OnLayerStackDestroy, &gl_LayerStack.mp_layerStack);
    if (errCode != NkErr_Ok)
        return errCode;

    /* Initialize the synchronization primitive. */
    if (NK_INITLOCK(gl_LayerStack.m_mtxLock) != thrd_success) {
        NkVectorDestroy(&gl_LayerStack.mp_layerStack);

        return NkErr_SynchInit;
    }

    NK_LOG_INFO("startup: layer stack");
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkLayerstackShutdown(NkVoid) {
    NK_LOG_INFO("shutdown: layer stack");

    /* Destroy the synchronization primitive. */
    NK_DESTROYLOCK(gl_LayerStack.m_mtxLock);
    /* Destroy the layer vector, releasing all layers still present in the stack. */
    NkVectorDestroy(&gl_LayerStack.mp_layerStack);

    return NkErr_Ok;
}

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
    errCode = NkVectorInsert(gl_LayerStack.mp_layerStack, (NkVoid const *)layerRef, whereInd);
    NK_UNLOCK(gl_LayerStack.m_mtxLock);
    if (errCode == NkErr_Ok)
        layerRef->VT->AddRef(layerRef);
    return errCode;
}

NkILayer *NK_CALL NkLayerstackPop(_In_ NkSize whereInd) {
    NK_ASSERT(gl_LayerStack.mp_layerStack != NULL, NkErr_ComponentState);

    NK_LOCK(gl_LayerStack.m_mtxLock);
    NkILayer *layerRef = NkVectorAt(gl_LayerStack.mp_layerStack, whereInd);
    NK_UNLOCK(gl_LayerStack.m_mtxLock);
    NK_ASSERT(layerRef != NULL, NkErr_InParameter);

    /*
     * Since we are running the destructor on the layer object, we must increase its
     * ref-count so that it doesn't actually get destroyed. The user is then free to
     * destroy it.
     */
    layerRef->VT->AddRef(layerRef);
    NK_IGNORE_RETURN_VALUE(layerRef->VT->OnPop(layerRef));
    
    NK_LOCK(gl_LayerStack.m_mtxLock);
    NK_IGNORE_RETURN_VALUE(NkVectorErase(gl_LayerStack.mp_layerStack, whereInd, &layerRef));
    NK_UNLOCK(gl_LayerStack.m_mtxLock);
    return layerRef;
}

_Return_ok_ NkErrorCode NK_CALL NkLayerstackProcessEvent(_In_ NkEvent const *evPtr) {
    NK_ASSERT(evPtr != NULL, NkErr_InParameter);
    if (gl_LayerStack.mp_layerStack == NULL)
        return NkErr_Ok;

    NK_LOCK(gl_LayerStack.m_mtxLock);
    NkBoolean eventWasHandled = NK_FALSE;
    NkSize layerCount = NkVectorGetElementCount(gl_LayerStack.mp_layerStack);
    for (NkSize i = 0; i < layerCount && !eventWasHandled; i++) {
        NkILayer *currLayer = NkVectorAt(gl_LayerStack.mp_layerStack, i);
        if (currLayer == NULL)
            continue;

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


#undef NK_NAMESPACE


