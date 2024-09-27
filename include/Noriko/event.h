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
 * \file  event.h
 * \brief defines the API for the Noriko event system
 *
 * User input and asynchronous message handling is done through a message-pump, 'pumping
 * out' and dispatching events to the layer stack. Each layer gets the chance to process
 * the event. If the layer chooses to process the event, layers underneath it in the
 * stack will not receive the event. If the layer does not process the event, the event
 * is propagated through the layer stack until the entire stack has been traversed or the
 * event has been handled.<br>
 * 
 * \note  Note that this event system is for application- and host-system events, not for
 *        in-game events.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>
#include <include/Noriko/error.h>
#include <include/Noriko/nkom.h>
#include <include/Noriko/window.h>


/**
 * \enum  NkEventType
 * \brief represents the event type IDs for Noriko's event system
 */
NK_NATIVE typedef enum NkEventType {
    NkEv_None = 0,            /**< unknown/default/invalid event type */

    /* window events */
    NkEv_WindowClosed,        /**< when the window is closed */
    NkEv_WindowGotFocus,      /**< when the window got focus */
    NkEv_WindowLostFocus,     /**< when the window lost focus */
    NkEv_WindowResized,       /**< when the window size got changed */
    NkEv_WindowMinimized,     /**< when the window was minimized */
    NkEv_WindowMaximized,     /**< when the window was maximized */
    NkEv_WindowRestored,      /**< when the window was restored */
    NkEv_WindowMoved,         /**< when the window position changed */
    NkEv_WindowFullscreen,    /**< when the window entered full-screen mode */
    NkEv_WindowShown,         /**< when the window was shown */
    NkEv_WindowHidden,        /**< when the window was hidden */

    /* keyboard events */
    NkEv_KeyboardKeyDown,     /**< when a key is pressed */
    NkEv_KeyboardKeyUp,       /**< when a key is released */
    NkEv_KeyboardKeyRepeated, /**< when a key is repeated */

    /* mouse events */
    NkEv_MouseButtonDown,     /**< when a mouse button is pressed */
    NkEv_MouseButtonUp,       /**< when a mouse button is released */
    NkEv_MouseMoved,          /**< when the mouse is moved */
    NkEv_MouseScrollUp,       /**< when the scroll wheel is scrolled up */
    NkEv_MouseScrollDown,     /**< when the scroll wheel is scrolled down */

    __NkEv_Count__,           /**< *only used internally* */
} NkEventType;

/**
 * \enum  NkEventCategory
 * \brief represents the category of an event
 * 
 * Each event type is associated with at least one of the following categories. This is
 * to help grouping events together for easier sorting, filtering, and processing. An
 * event can be associated to multiple event categories. For example, both keyboard- and
 * mouse events are also in the (generic) 'input' category.
 */
NK_NATIVE typedef enum NkEventCategory {
    NkEvCat_None = 0,             /**< unknown/invalid category */

    NkEvCat_Application = 1 << 0, /**< for application events */
    NkEvCat_Window      = 1 << 1, /**< for window events */
    NkEvCat_Input       = 1 << 2, /**< for input events of all types */
    NkEvCat_Keyboard    = 1 << 3, /**< for keyboard events */
    NkEvCat_Mouse       = 1 << 4, /**< for mouse events */
    NkEvCat_User        = 1 << 5, /**< for user- (and special application-) events */

    __NkEvCat_Count__             /**< *only used internally* */
} NkEventCategory;


/**
 * \struct NkWindowEvent
 * \brief  represents the additional data used by some window events
 */
NK_NATIVE typedef struct NkWindowEvent {
    NkIWindow    *mp_wndRef;      /**< reference to the Noriko window handle */
    NkPoint2D     m_wndPos;       /**< global window position (screen coordinates) */
    NkSize2D      m_wndSize;      /**< window size (in pixels, client area only) */
    NkSize2D      m_totalWndSize; /**< total window size (in pixels, incl. non-client area, etc.) */
    NkWindowMode  m_wndMode;      /**< \todo current window mode */
} NkWindowEvent;

/**
 * \struct NkKeyboardEvent
 * \brief  additional data for keyboard events
 */
NK_NATIVE typedef struct NkKeyboardEvent {
    NkInt32 m_pKeyCode;    /**< physical key-code */
    NkInt32 m_vKeyCode;    /**< virtual key-code */
    NkInt32 m_repeatCount; /**< repeat-count */
} NkKeyboardEvent;

/**
 * \struct  NkMouseEvent
 * \brief   contains the extra mouse-event data
 * \warning In some cases, for example in the case of a 'mouse-move' event, there may not
 *          be any button pressed, so the \c m_mouseBtn field may be invalid.
 */
NK_NATIVE typedef struct NkMouseEvent {
    NkPoint2D m_curPos;   /**< current (window-local) cursor position */
    NkPoint2D m_glCurPos; /**< global (screen-wide) cursor position */
    NkInt32   m_mouseBtn; /**< mouse button */
} NkMouseEvent;

/**
 * \struct NkEvent
 * \brief  represents an Noriko system event
 */
NK_NATIVE typedef struct NkEvent {
    NkEventType     m_evType;    /**< numeric type ID */
    NkEventCategory m_evCat;     /**< bitfield representing the event type's categories */
    NkUint64        m_timestamp; /**< high-precision timestamp (if available) */

    /* additional data specifying event parameters; not used by some events */
    union {
        NkWindowEvent   m_wndEvent;   /**< used by some window events */
        NkKeyboardEvent m_kbEvent;    /**< used by keyboard events */
        NkMouseEvent    m_mouseEvent; /**< used by some mouse events */
    };
} NkEvent;


/**
 * \brief  creates an event with the given properties and synchronously invokes the layer
 *         stack
 * \param  [in] evType numeric type ID of the event
 * \return On success, <tt>NkErr_Ok</tt>; on failure, non-zero
 * \note   If the event was not handled, the function returns <tt>NkErr_NoOperation</tt>.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkEventDispatch(_In_ NkEventType evType, ...);
/**
 * \brief duplicates the given event
 * \param [in] srcPtr pointer to the \c NkEvent instance that is to be duplicated
 * \param [out] dstPtr pointer to the destination event
 */
NK_NATIVE NK_API NkVoid NK_CALL NkEventCopy(_In_ NkEvent const *srcPtr, _Out_ NkEvent *dstPtr);

/**
 * \brief   queries the string representation of the given event type ID
 * \param   [in] evType numeric event type ID
 * \return  pointer to an \c NkStringView that contains the representation
 * \note    The returned value is a pointer to a statically-allocated string-view
 *          instance and as such does not have to be freed.
 * \warning Passing an invalid event type ID results in undefined behavior.
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkEventQueryTypeString(_In_ NkEventType evType);
/**
 * \brief   queries the string representation of the given event category ID
 * \param   [in] evCat numeric event category ID
 * \return  pointer to an \c NkStringView that contains the representation
 * \note    The returned value is a pointer to a statically-allocated string-view
 *          instance and as such does not have to be freed.
 * \warning Passing an invalid event category ID results in undefined behavior.
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkEventQueryCategoryString(_In_ NkEventCategory evCat);
/**
 * \brief   queries the event categories the given event type is associated with
 * \param   [in] evType numeric event type ID
 * \return  event categories (as bitfield) that the event type is associated with
 * \warning Passing an invalid event type ID results in undefined behavior.
 */
NK_NATIVE NK_API NkEventCategory NK_CALL NkEventQueryCategories(_In_ NkEventType evType);


