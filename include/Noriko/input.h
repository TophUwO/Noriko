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
 * \file  input.h
 * \brief defines the platform-independent API for basic input handling
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>
#include <include/Noriko/nkom.h>


/**
 * \emum  NkKeyboardKey
 * \brief represents all virtual key-codes supported by Noriko
 * 
 * \par Remarks
 *   The mapping below is a custom mapping somewhat resembling the virtual key-code
 *   mapping used in the Win32 API.
 */
NK_NATIVE typedef enum NkKeyboardKey {
    NkKey_Unknown = 0,     /**< unknown/invalid/unsupported key-code */

    NkKey_Backspace,       /**< backspace key */
    NkKey_Enter,           /**< 'return/enter' key */
    NkKey_Escape,          /**< escape key key */
    NkKey_Tab,             /**< tabulator key */
    NkKey_LShift,          /**< left shift key */
    NkKey_RShift,          /**< right shift key */
    NkKey_LControl,        /**< left CTRL key */
    NkKey_RControl,        /**< right CTRL key */
    NkKey_LAlt,            /**< left ALT/menu key */
    NkKey_RAlt,            /**< right ALT/menu key */
    NkKey_LSuper,          /**< left 'Windows/...' key */
    NkKey_RSuper,          /**< right 'Windows/...' key */
    NkKey_F1,              /**< dedicated F1 key */
    NkKey_F2,              /**< dedicated F2 key */
    NkKey_F3,              /**< dedicated F3 key */
    NkKey_F4,              /**< dedicated F4 key */
    NkKey_F5,              /**< dedicated F5 key */
    NkKey_F6,              /**< dedicated F6 key */
    NkKey_F7,              /**< dedicated F7 key */
    NkKey_F8,              /**< dedicated F8 key */
    NkKey_F9,              /**< dedicated F9 key */
    NkKey_F10,             /**< dedicated F10 key */
    NkKey_F11,             /**< dedicated F11 key */
    NkKey_F12,             /**< dedicated F12 key */
    NkKey_Home,            /**< start-of-line key */
    NkKey_End,             /**< end-of-line key */
    NkKey_PageUp,          /**< page-up control key */
    NkKey_PageDown,        /**< page-down control key */
    NkKey_Insert,          /**< 'insert' (INS) key */
    NkKey_Delete,          /**< 'delete' (DEL) key */
    NkKey_Print,           /**< print-screen/snapshot key */
    NkKey_Scroll,          /**< scroll-lock key */
    NkKey_Pause,           /**< pause key */
    NkKey_Up,              /**< arrow-up key */
    NkKey_Down,            /**< arrow-down key */
    NkKey_Left,            /**< arrow-left key */
    NkKey_Right,           /**< arrow-right key */
    NkKey_NumLock,         /**< num-lock toggle key */
    NkKey_Numpad0,         /**< numeric numpad 0 key */
    NkKey_Numpad1,         /**< numeric numpad 1 key */
    NkKey_Numpad2,         /**< numeric numpad 2 key */
    NkKey_Numpad3,         /**< numeric numpad 3 key */
    NkKey_Numpad4,         /**< numeric numpad 4 key */
    NkKey_Numpad5,         /**< numeric numpad 5 key */
    NkKey_Numpad6,         /**< numeric numpad 6 key */
    NkKey_Numpad7,         /**< numeric numpad 7 key */
    NkKey_Numpad8,         /**< numeric numpad 8 key */
    NkKey_Numpad9,         /**< numeric numpad 9 key */
    NkKey_NumpadPlus,      /**< numpad '+' key */
    NkKey_NumpadMinus,     /**< numpad '-' key */
    NkKey_NumpadMultiply,  /**< numpad '*' key */
    NkKey_NumpadDivide,    /**< numpad '/' key */
    NkKey_NumpadSeparator, /**< numpad '.' key */
    NkKey_NumpadDecimal,   /**< numpad '' key */
    NkKey_Space,           /**< alpha-numeric ' ' key */
    NkKey_Key0,            /**< alpha-numeric '0' key */
    NkKey_Key1,            /**< alpha-numeric '1' key */
    NkKey_Key2,            /**< alpha-numeric '2' key */
    NkKey_Key3,            /**< alpha-numeric '3' key */
    NkKey_Key4,            /**< alpha-numeric '4' key */
    NkKey_Key5,            /**< alpha-numeric '5' key */
    NkKey_Key6,            /**< alpha-numeric '6' key */
    NkKey_Key7,            /**< alpha-numeric '7' key */
    NkKey_Key8,            /**< alpha-numeric '8' key */
    NkKey_Key9,            /**< alpha-numeric '9' key */
    NkKey_KeyA,            /**< alpha-numeric 'A' key */
    NkKey_KeyB,            /**< alpha-numeric 'B' key */
    NkKey_KeyC,            /**< alpha-numeric 'C' key */
    NkKey_KeyD,            /**< alpha-numeric 'D' key */
    NkKey_KeyE,            /**< alpha-numeric 'E' key */
    NkKey_KeyF,            /**< alpha-numeric 'F' key */
    NkKey_KeyG,            /**< alpha-numeric 'G' key */
    NkKey_KeyH,            /**< alpha-numeric 'H' key */
    NkKey_KeyI,            /**< alpha-numeric 'I' key */
    NkKey_KeyJ,            /**< alpha-numeric 'J' key */
    NkKey_KeyK,            /**< alpha-numeric 'K' key */
    NkKey_KeyL,            /**< alpha-numeric 'L' key */
    NkKey_KeyM,            /**< alpha-numeric 'M' key */
    NkKey_KeyN,            /**< alpha-numeric 'N' key */
    NkKey_KeyO,            /**< alpha-numeric 'O' key */
    NkKey_KeyP,            /**< alpha-numeric 'P' key */
    NkKey_KeyQ,            /**< alpha-numeric 'Q' key */
    NkKey_KeyR,            /**< alpha-numeric 'R' key */
    NkKey_KeyS,            /**< alpha-numeric 'S' key */
    NkKey_KeyT,            /**< alpha-numeric 'T' key */
    NkKey_KeyU,            /**< alpha-numeric 'U' key */
    NkKey_KeyV,            /**< alpha-numeric 'V' key */
    NkKey_KeyW,            /**< alpha-numeric 'W' key */
    NkKey_KeyX,            /**< alpha-numeric 'X' key */
    NkKey_KeyY,            /**< alpha-numeric 'Y' key */
    NkKey_KeyZ,            /**< alpha-numeric 'Z' key */
    NkKey_Oem1,            /**< ';:' for US */
    NkKey_Oem2,            /**< '+' for any country/region */
    NkKey_Oem3,            /**< ',' for any country/region */
    NkKey_Oem4,            /**< '-' for any country/region */
    NkKey_Oem5,            /**< '.' for any country/region */
    NkKey_Oem6,            /**< '/?' for US */
    NkKey_Oem7,            /**< '`~' for US */
    NkKey_Oem8,            /**< '[{' for US */
    NkKey_Oem9,            /**< '\|' for US */
    NkKey_Oem10,           /**< ']}' for US */
    NkKey_Oem11,           /**< ''"' for US */

    __NkKey_Count__        /**< *only used internally* */
} NkKeyboardKey;

/**
 * \enum  NkModifierKeys
 * \brief represents all supported modifier keys, that is, keys that may modify the
 *        current key action but may not mean anything on their own
 */
NK_NATIVE typedef enum NkModifierKeys {
    NkModKey_Unknown = 0,      /**< unknown modifier key */

    NkModKey_LShift  = 1 << 0, /**< left shift (LSHIFT) */
    NkModKey_RShift  = 1 << 1, /**< right shift (RSHIFT) */
    NkModKey_LCtrl   = 1 << 2, /**< left control (LCTRL) */
    NkModKey_RCtrl   = 1 << 3, /**< right control (RCTRL) */
    NkModKey_Menu    = 1 << 4, /**< menu key */
    NkModKey_LAlt    = 1 << 5, /**< left alt (LALT) */
    NkModKey_RAlt    = 1 << 6, /**< right alt (RALT, ALTGR) */
    NkModKey_LSuper  = 1 << 7, /**< left win (LWIN) */
    NkModKey_RSuper  = 1 << 8  /**< right win (RWIN) */
} NkModifierKeys;

/**
 * \enum  NkMouseButton
 * \brief represents platform-independent 
 */
NK_NATIVE typedef enum NkMouseButton {
    NkBtn_Unknown = 0,  /**< unknown/unsupported mouse buttom */

    NkBtn_LeftButton,   /**< left mouse buttom */
    NkBtn_MiddleButton, /**< middle mouse (scroll-wheel) button */
    NkBtn_RightButton,  /**< right mouse buttom */
    NkBtn_Button4,      /**< first side button ('UP') */
    NkBtn_Button5       /**< second side buttom ('DOWN') */
} NkMouseButton;


/**
 */
NKOM_DECLARE_INTERFACE(NkIInput) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkIInput *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIInput *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIInput *self);

    /**
     */
    NkKeyboardKey (NK_CALL *MapFromNativeKey)(_Inout_ NkIInput *self, _In_ int ntKeyCode);
    /**
     */
    NkMouseButton (NK_CALL *MapFromNativeMouseButton)(_Inout_ NkIInput *self, _In_ int ntMouseBtn);
    /**
     */
    NkInt32 (NK_CALL *MapToNativeKey)(_Inout_ NkIInput *self, _In_ NkKeyboardKey keyCode);
    /**
     */
    NkInt32 (NK_CALL *MapToNativeMouseButton)(_Inout_ NkIInput *self, _In_ NkMouseButton mouseBtn);

    /**
     */
    NkBoolean (NK_CALL *IsKeyPressed)(_Inout_ NkIInput *self, _In_ NkKeyboardKey keyCode);
    /**
     */
    NkBoolean (NK_CALL *IsMouseButtonPressed)(_Inout_ NkIInput *self, _In_ NkMouseButton mouseBtn);
    /**
     */
    NkPoint2D (NK_CALL *GetMousePosition)(_Inout_ NkIInput *self);
};


/**
 * \brief  runs some platform-dependent input abstraction layer (IAL) initialization
 *         routines
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   Run this function once per process before you call <tt>NkInputQueryInstance()</tt>
 *         for the first time.
 */
NK_NATIVE NK_API NK_VIRTUAL _Return_ok_ NkErrorCode NK_CALL NkInputStartup(NkVoid);
/**
 * \brief  runs some platform-dependent input abstraction layer (IAL) uninitialization
 *         routines
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   Do not use <tt>NkInputQueryInstance()</tt> or its return value after you run
 *         this function. To use it again, run <tt>NkInputStartup()</tt> again first.
 */
NK_NATIVE NK_API NK_VIRTUAL _Return_ok_ NkErrorCode NK_CALL NkInputShutdown(NkVoid);

/**
 * \brief  retrieves the platform-dependent input translator instance
 * \return pointer to the instance
 * \note   The reference count of the returned instance is incremented. Please call
 *         <tt>Release()</tt> on the returned instance after you are done with it.
 */
NK_NATIVE NK_API NK_VIRTUAL NkIInput *NK_CALL NkInputQueryInstance(NkVoid);


