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
 * \file  layer.h
 * \brief defines the public layer API
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


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/nkom.h>
#include <include/Noriko/event.h>


/**
 * \def   NK_AS_OVERLAY
 * \brief specifies that the layer should be pushed as an overlay (i.e., a layer that
 *        sits on top of all other layers)
 */
#define NK_AS_OVERLAY (0)
/**
 * \def   NK_AS_NORMAL
 * \brief specifies that the layer should be added as normal (i.e., to the back of the
 *        stack (that is, gets a chance to process incoming events last))
 */
#define NK_AS_NORMAL  (SIZE_MAX)


/**
 * \interface NkILayer
 * \brief     represents a layer object that can be managed and used by the layer system
 * 
 * \par Remarks
 *   This interface inherits from <tt>NkIBase</tt>.
 */
NKOM_DECLARE_INTERFACE(NkILayer) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkILayer *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt>
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkILayer *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt>
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkILayer *self);

    /**
     * \brief invoked right before the layer is pushed onto the layer stack
     *
     * The purpose of this method is to let the layer do some initialization that cannot
     * be done at creation time, but rather when the layer is about to be actually used.
     * 
     * \param  [in,out] self pointer to the current \c NkILayer instance
     * \param  [in] beforeRef pointer to the \c NkILayer instance that \c self is to be
     *              inserted <em>after</em> (i.e., what comes <em>before</em>)
     * \param  [in] afterRef pointer to the \c NkILayer instance that \c self is to be
     *              inserted <em>before</em> (i.e., what comes <em>after</em>)
     * \param  [in] index numeric index of where the layer is to be inserted; do not
     *              cache this value since it may change when new layers are pushed or
     *              existing layers are popped
     * \return \c NkErr_Ok on success, non-zero on failure
     * \note   \li If the return value is non-zero, the layer is *not* pushed onto the
     *             layer stack.
     * \note   \li If there is no adjacent layer in the layer stack, \c beforeRef and
     *             \c afterRef can be <tt>NULL</tt>, respectively.
     */
    NkErrorCode (NK_CALL *OnPush)(
        _Inout_  NkILayer *self,
        _In_opt_ NkILayer const *beforeRef,
        _In_opt_ NkILayer const *afterRef,
        _In_     NkSize index
    );
    /**
     */
    NkErrorCode (NK_CALL *OnPop)(_Inout_ NkILayer *self);
    /**
     */
    NkErrorCode (NK_CALL *OnEvent)(_Inout_ NkILayer *self, _In_ NkEvent const *evPtr);
    /**
     */
    NkErrorCode (NK_CALL *OnRender)(_Inout_ NkILayer *self, _In_ NkFloat deltaTime);
};


/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLayerstackStartup(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLayerstackShutdown(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLayerstackPush(_Inout_ NkILayer *layerRef, _In_ NkSize whereInd);
/**
 */
NK_NATIVE NK_API NkILayer *NK_CALL NkLayerstackPop(_In_ NkSize whereInd);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLayerstackOnEvent(_In_ NkEvent const *evPtr);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLayerstackOnRender(_In_ NkFloat deltaTime);


