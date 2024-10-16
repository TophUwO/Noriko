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
 * \file  input.c
 * \brief implements platform-independent input helper functions
 */
#define NK_NAMESPACE "nk::input"


/* Noriko includes */
#include <include/Noriko/input.h>


NkStringView const *NK_CALL NkInputQueryKeyString(_In_ NkKeyboardKey keyCode) {
    NK_ASSERT(keyCode >= 0 && keyCode < NK_MAX_NUM_KEY_CODES, NkErr_InParameter);

    /** \cond INTERNAL */
    /**
     * \brief table of key-code string representations; can be used for debugging and
     *        is going to be used for the key-action-mapper
     */
    NK_INTERNAL NkStringView const gl_KeyCodeStrs[NK_MAX_NUM_KEY_CODES] = {
        [NkKey_Backspace]       = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Backspace)),
        [NkKey_Enter]           = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Enter)),
        [NkKey_Escape]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Escape)),
        [NkKey_Tab]             = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Tab)),
        [NkKey_CapsLock]        = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_CapsLock)),
        [NkKey_LShift]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_LShift)),
        [NkKey_RShift]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_RShift)),
        [NkKey_LControl]        = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_LControl)),
        [NkKey_RControl]        = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_RControl)),
        [NkKey_LAlt]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_LAlt)),
        [NkKey_RAlt]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_RAlt)),
        [NkKey_LSuper]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_LSuper)),
        [NkKey_RSuper]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_RSuper)),
        [NkKey_Context]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Context)),
        [NkKey_F1]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F1)),
        [NkKey_F2]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F2)),
        [NkKey_F3]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F3)),
        [NkKey_F4]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F4)),
        [NkKey_F5]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F5)),
        [NkKey_F6]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F6)),
        [NkKey_F7]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F7)),
        [NkKey_F8]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F8)),
        [NkKey_F9]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F9)),
        [NkKey_F10]             = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F10)),
        [NkKey_F11]             = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F11)),
        [NkKey_F12]             = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_F12)),
        [NkKey_Home]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Home)),
        [NkKey_End]             = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_End)),
        [NkKey_PageUp]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_PageUp)),
        [NkKey_PageDown]        = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_PageDown)),
        [NkKey_Insert]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Insert)),
        [NkKey_Delete]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Delete)),
        [NkKey_PrintScreen]     = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_PrintScreen)),
        [NkKey_Scroll]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Scroll)),
        [NkKey_Pause]           = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Pause)),
        [NkKey_Up]              = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Up)),
        [NkKey_Down]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Down)),
        [NkKey_Left]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Left)),
        [NkKey_Right]           = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Right)),
        [NkKey_NumLock]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_NumLock)),
        [NkKey_Numpad0]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad0)),
        [NkKey_Numpad1]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad1)),
        [NkKey_Numpad2]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad2)),
        [NkKey_Numpad3]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad3)),
        [NkKey_Numpad4]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad4)),
        [NkKey_Numpad5]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad5)),
        [NkKey_Numpad6]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad6)),
        [NkKey_Numpad7]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad7)),
        [NkKey_Numpad8]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad8)),
        [NkKey_Numpad9]         = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Numpad9)),
        [NkKey_NumpadPlus]      = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_NumpadPlus)),
        [NkKey_NumpadMinus]     = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_NumpadMinus)),
        [NkKey_NumpadMultiply]  = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_NumpadMultiply)),
        [NkKey_NumpadDivide]    = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_NumpadDivide)),
        [NkKey_NumpadSeparator] = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_NumpadSeparator)),
        [NkKey_NumpadDecimal]   = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_NumpadDecimal)),
        [NkKey_Space]           = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Space)),
        [NkKey_Alnum0]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum0)),
        [NkKey_Alnum1]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum1)),
        [NkKey_Alnum2]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum2)),
        [NkKey_Alnum3]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum3)),
        [NkKey_Alnum4]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum4)),
        [NkKey_Alnum5]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum5)),
        [NkKey_Alnum6]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum6)),
        [NkKey_Alnum7]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum7)),
        [NkKey_Alnum8]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum8)),
        [NkKey_Alnum9]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Alnum9)),
        [NkKey_AlnumA]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumA)),
        [NkKey_AlnumB]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumB)),
        [NkKey_AlnumC]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumC)),
        [NkKey_AlnumD]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumD)),
        [NkKey_AlnumE]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumE)),
        [NkKey_AlnumF]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumF)),
        [NkKey_AlnumG]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumG)),
        [NkKey_AlnumH]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumH)),
        [NkKey_AlnumI]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumI)),
        [NkKey_AlnumJ]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumJ)),
        [NkKey_AlnumK]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumK)),
        [NkKey_AlnumL]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumL)),
        [NkKey_AlnumM]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumM)),
        [NkKey_AlnumN]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumN)),
        [NkKey_AlnumO]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumO)),
        [NkKey_AlnumP]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumP)),
        [NkKey_AlnumQ]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumQ)),
        [NkKey_AlnumR]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumR)),
        [NkKey_AlnumS]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumS)),
        [NkKey_AlnumT]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumT)),
        [NkKey_AlnumU]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumU)),
        [NkKey_AlnumV]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumV)),
        [NkKey_AlnumW]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumW)),
        [NkKey_AlnumX]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumX)),
        [NkKey_AlnumY]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumY)),
        [NkKey_AlnumZ]          = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_AlnumZ)),
        [NkKey_Oem1]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem1)),
        [NkKey_Oem2]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem2)),
        [NkKey_Oem3]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem3)),
        [NkKey_Oem4]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem4)),
        [NkKey_Oem5]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem5)),
        [NkKey_Oem6]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem6)),
        [NkKey_Oem7]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem7)),
        [NkKey_Oem8]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem8)),
        [NkKey_Oem9]            = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem9)),
        [NkKey_Oem10]           = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem10)),
        [NkKey_Oem11]           = NK_MAKE_STRING_VIEW(NK_ESC(NkKey_Oem11))
    };
    /** \endcond */

    return &gl_KeyCodeStrs[keyCode & (NK_MAX_NUM_KEY_CODES - 1)];
}

NkStringView const *NK_CALL NkInputQueryMouseButtonString(_In_ NkMouseButton mouseBtn) {
    NK_ASSERT(mouseBtn >= 0 && mouseBtn < NK_MAX_NUM_MOUSE_BTNS, NkErr_InParameter);

    /** \cond INTERNAL */
    /**
     * \brief table representing string representations for all supported mouse buttons;
     *        can be used for debugging and is going to be used for the key-action-mapper
     */
    NK_INTERNAL NkStringView const gl_MouseBtnStrs[NK_MAX_NUM_MOUSE_BTNS] = {
        [NkBtn_LeftButton]   = NK_MAKE_STRING_VIEW(NK_ESC(NkBtn_LeftButton)),
        [NkBtn_MiddleButton] = NK_MAKE_STRING_VIEW(NK_ESC(NkBtn_MiddleButton)),
        [NkBtn_RightButton]  = NK_MAKE_STRING_VIEW(NK_ESC(NkBtn_RightButton)),
        [NkBtn_Button4]      = NK_MAKE_STRING_VIEW(NK_ESC(NkBtn_Button4)),
        [NkBtn_Button5]      = NK_MAKE_STRING_VIEW(NK_ESC(NkBtn_Button5))
    };
    /** \endcond */

    return &gl_MouseBtnStrs[mouseBtn & (NK_MAX_NUM_MOUSE_BTNS - 1)];
}


#undef NK_NAMESPACE


