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
 * \file  wininput.c
 * \brief implements the platform-specific input abstraction layer (IAL) for Windows
 */
#define NK_NAMESPACE "nk::wininput"


/* Noriko includes */
#include <include/Noriko/input.h>
#include <include/Noriko/log.h>
#include <include/Noriko/platform.h>


/** \cond INTERNAL */
/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WindowsInput_AddRef(_Inout_ NkIInput *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WindowsInput_Release(_Inout_ NkIInput *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WindowsInput_QueryInterface(
    _Inout_  NkIInput *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    if (NkUuidIsEqual(iId, NKOM_IIDOF(NkIBase)) || NkUuidIsEqual(iId, NKOM_IIDOF(NkIInput))) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_WindowsInput_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 */
NK_INTERNAL NkKeyboardKey NK_CALL __NkInt_WindowsInput_MapFromNativeKey(
    _Inout_ NkIInput *self,
    _In_    NkInt32 ntKeyCode
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /**
     * \brief key-code Native->Noriko translation mapping
     */
    NK_INTERNAL NkKeyboardKey const gl_Nt2NkKeyMapping[NK_MAX_NUM_KEY_CODES] = {
        [VK_BACK]       = NkKey_Backspace,
        [VK_TAB]        = NkKey_Tab,
        [VK_RETURN]     = NkKey_Enter,
        [VK_PAUSE]      = NkKey_Pause,
        [VK_CAPITAL]    = NkKey_CapsLock,
        [VK_ESCAPE]     = NkKey_Escape,
        [VK_SPACE]      = NkKey_Space,
        ['0']           = NkKey_Alnum0,
        ['1']           = NkKey_Alnum1,
        ['2']           = NkKey_Alnum2,
        ['3']           = NkKey_Alnum3,
        ['4']           = NkKey_Alnum4,
        ['5']           = NkKey_Alnum5,
        ['6']           = NkKey_Alnum6,
        ['7']           = NkKey_Alnum7,
        ['8']           = NkKey_Alnum8,
        ['9']           = NkKey_Alnum9,
        ['A']           = NkKey_AlnumA,
        ['B']           = NkKey_AlnumB,
        ['C']           = NkKey_AlnumC,
        ['D']           = NkKey_AlnumD,
        ['E']           = NkKey_AlnumE,
        ['F']           = NkKey_AlnumF,
        ['G']           = NkKey_AlnumG,
        ['H']           = NkKey_AlnumH,
        ['I']           = NkKey_AlnumI,
        ['J']           = NkKey_AlnumJ,
        ['K']           = NkKey_AlnumK,
        ['L']           = NkKey_AlnumL,
        ['M']           = NkKey_AlnumM,
        ['N']           = NkKey_AlnumN,
        ['O']           = NkKey_AlnumO,
        ['P']           = NkKey_AlnumP,
        ['Q']           = NkKey_AlnumQ,
        ['R']           = NkKey_AlnumR,
        ['S']           = NkKey_AlnumS,
        ['T']           = NkKey_AlnumT,
        ['U']           = NkKey_AlnumU,
        ['V']           = NkKey_AlnumV,
        ['W']           = NkKey_AlnumW,
        ['X']           = NkKey_AlnumX,
        ['Y']           = NkKey_AlnumY,
        ['Z']           = NkKey_AlnumZ,
        [VK_PRIOR]      = NkKey_PageUp,
        [VK_NEXT]       = NkKey_PageDown,
        [VK_END]        = NkKey_End,
        [VK_HOME]       = NkKey_Home,
        [VK_LEFT]       = NkKey_Left,
        [VK_UP]         = NkKey_Up,
        [VK_RIGHT]      = NkKey_Right,
        [VK_DOWN]       = NkKey_Down,
        [VK_SNAPSHOT]   = NkKey_PrintScreen,
        [VK_INSERT]     = NkKey_Insert,
        [VK_DELETE]     = NkKey_Delete,
        [VK_LWIN]       = NkKey_LSuper,
        [VK_RWIN]       = NkKey_RSuper,
        [VK_NUMPAD0]    = NkKey_Numpad0,
        [VK_NUMPAD1]    = NkKey_Numpad1,
        [VK_NUMPAD2]    = NkKey_Numpad2,
        [VK_NUMPAD3]    = NkKey_Numpad3,
        [VK_NUMPAD4]    = NkKey_Numpad4,
        [VK_NUMPAD5]    = NkKey_Numpad5,
        [VK_NUMPAD6]    = NkKey_Numpad6,
        [VK_NUMPAD7]    = NkKey_Numpad7,
        [VK_NUMPAD8]    = NkKey_Numpad8,
        [VK_NUMPAD9]    = NkKey_Numpad9,
        [VK_MULTIPLY]   = NkKey_NumpadMultiply,
        [VK_ADD]        = NkKey_NumpadPlus,
        [VK_SEPARATOR]  = NkKey_NumpadSeparator,
        [VK_SUBTRACT]   = NkKey_NumpadMinus,
        [VK_DECIMAL]    = NkKey_NumpadDecimal,
        [VK_DIVIDE]     = NkKey_NumpadDivide,
        [VK_F1]         = NkKey_F1,
        [VK_F2]         = NkKey_F2,
        [VK_F3]         = NkKey_F3,
        [VK_F4]         = NkKey_F4,
        [VK_F5]         = NkKey_F5,
        [VK_F6]         = NkKey_F6,
        [VK_F7]         = NkKey_F7,
        [VK_F8]         = NkKey_F8,
        [VK_F9]         = NkKey_F9,
        [VK_F10]        = NkKey_F10,
        [VK_F11]        = NkKey_F11,
        [VK_F12]        = NkKey_F12,
        [VK_NUMLOCK]    = NkKey_NumLock,
        [VK_SCROLL]     = NkKey_Scroll,
        [VK_LSHIFT]     = NkKey_LShift,
        [VK_RSHIFT]     = NkKey_RShift,
        [VK_LCONTROL]   = NkKey_LControl,
        [VK_RCONTROL]   = NkKey_RControl,
        [VK_LMENU]      = NkKey_LAlt,
        [VK_RMENU]      = NkKey_RAlt,
        [VK_OEM_1]      = NkKey_Oem1,
        [VK_OEM_PLUS]   = NkKey_Oem2,
        [VK_OEM_COMMA]  = NkKey_Oem3,
        [VK_OEM_MINUS]  = NkKey_Oem4,
        [VK_OEM_PERIOD] = NkKey_Oem5,
        [VK_OEM_2]      = NkKey_Oem6,
        [VK_OEM_3]      = NkKey_Oem7,
        [VK_OEM_4]      = NkKey_Oem8,
        [VK_OEM_5]      = NkKey_Oem9,
        [VK_OEM_6]      = NkKey_Oem10,
        [VK_OEM_7]      = NkKey_Oem11
    };

    return gl_Nt2NkKeyMapping[ntKeyCode & (NK_MAX_NUM_KEY_CODES - 1)];
}

/**
 */
NK_INTERNAL NkMouseButton NK_CALL __NkInt_WindowsInput_MapFromNativeMouseButton(
    _Inout_ NkIInput *self,
    _In_    NkInt32 ntMouseBtn
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /**
     * \brief maps Win32 mouse button codes to Noriko mouse button codes 
     */
    NK_INTERNAL NkMouseButton const gl_Nt2NkMouseBtnMapping[NK_MAX_NUM_MOUSE_BTNS] = {
        [VK_LBUTTON]  = NkBtn_LeftButton,  
        [VK_MBUTTON]  = NkBtn_MiddleButton,
        [VK_RBUTTON]  = NkBtn_RightButton, 
        [VK_XBUTTON1] = NkBtn_Button4,     
        [VK_XBUTTON2] = NkBtn_Button5      
    };

    return gl_Nt2NkMouseBtnMapping[ntMouseBtn & (NK_MAX_NUM_MOUSE_BTNS - 1)];
}

/**
 */
NK_INTERNAL NkInt32 NK_CALL __NkInt_WindowsInput_MapToNativeKey(_Inout_ NkIInput *self, _In_ NkKeyboardKey keyCode) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /**
     * \brief key-code Native->Noriko translation mapping
     */
    NK_INTERNAL NkKeyboardKey const gl_Nk2NtKeyMapping[NK_MAX_NUM_KEY_CODES] = {
        [NkKey_Backspace]       = VK_BACK,
        [NkKey_Tab]             = VK_TAB,
        [NkKey_Enter]           = VK_RETURN,
        [NkKey_Pause]           = VK_PAUSE,
        [NkKey_CapsLock]        = VK_CAPITAL,
        [NkKey_Escape]          = VK_ESCAPE,
        [NkKey_Space]           = VK_SPACE,
        [NkKey_Alnum0]          = '0',
        [NkKey_Alnum1]          = '1',
        [NkKey_Alnum2]          = '2',
        [NkKey_Alnum3]          = '3',
        [NkKey_Alnum4]          = '4',
        [NkKey_Alnum5]          = '5',
        [NkKey_Alnum6]          = '6',
        [NkKey_Alnum7]          = '7',
        [NkKey_Alnum8]          = '8',
        [NkKey_Alnum9]          = '9',
        [NkKey_AlnumA]          = 'A',
        [NkKey_AlnumB]          = 'B',
        [NkKey_AlnumC]          = 'C',
        [NkKey_AlnumD]          = 'D',
        [NkKey_AlnumE]          = 'E',
        [NkKey_AlnumF]          = 'F',
        [NkKey_AlnumG]          = 'G',
        [NkKey_AlnumH]          = 'H',
        [NkKey_AlnumI]          = 'I',
        [NkKey_AlnumJ]          = 'J',
        [NkKey_AlnumK]          = 'K',
        [NkKey_AlnumL]          = 'L',
        [NkKey_AlnumM]          = 'M',
        [NkKey_AlnumN]          = 'N',
        [NkKey_AlnumO]          = 'O',
        [NkKey_AlnumP]          = 'P',
        [NkKey_AlnumQ]          = 'Q',
        [NkKey_AlnumR]          = 'R',
        [NkKey_AlnumS]          = 'S',
        [NkKey_AlnumT]          = 'T',
        [NkKey_AlnumU]          = 'U',
        [NkKey_AlnumV]          = 'V',
        [NkKey_AlnumW]          = 'W',
        [NkKey_AlnumX]          = 'X',
        [NkKey_AlnumY]          = 'Y',
        [NkKey_AlnumZ]          = 'Z',
        [NkKey_PageUp]          = VK_PRIOR,
        [NkKey_PageDown]        = VK_NEXT,
        [NkKey_End]             = VK_END,
        [NkKey_Home]            = VK_HOME,
        [NkKey_Left]            = VK_LEFT,
        [NkKey_Up]              = VK_UP,
        [NkKey_Right]           = VK_RIGHT,
        [NkKey_Down]            = VK_DOWN,
        [NkKey_PrintScreen]     = VK_SNAPSHOT,
        [NkKey_Insert]          = VK_INSERT,
        [NkKey_Delete]          = VK_DELETE,
        [NkKey_LSuper]          = VK_LWIN,
        [NkKey_RSuper]          = VK_RWIN,
        [NkKey_Numpad0]         = VK_NUMPAD0,
        [NkKey_Numpad1]         = VK_NUMPAD1,
        [NkKey_Numpad2]         = VK_NUMPAD2,
        [NkKey_Numpad3]         = VK_NUMPAD3,
        [NkKey_Numpad4]         = VK_NUMPAD4,
        [NkKey_Numpad5]         = VK_NUMPAD5,
        [NkKey_Numpad6]         = VK_NUMPAD6,
        [NkKey_Numpad7]         = VK_NUMPAD7,
        [NkKey_Numpad8]         = VK_NUMPAD8,
        [NkKey_Numpad9]         = VK_NUMPAD9,
        [NkKey_NumpadMultiply]  = VK_MULTIPLY,
        [NkKey_NumpadPlus]      = VK_ADD,
        [NkKey_NumpadSeparator] = VK_SEPARATOR,
        [NkKey_NumpadMinus]     = VK_SUBTRACT,
        [NkKey_NumpadDecimal]   = VK_DECIMAL,
        [NkKey_NumpadDivide]    = VK_DIVIDE,
        [NkKey_F1]              = VK_F1,
        [NkKey_F2]              = VK_F2,
        [NkKey_F3]              = VK_F3,
        [NkKey_F4]              = VK_F4,
        [NkKey_F5]              = VK_F5,
        [NkKey_F6]              = VK_F6,
        [NkKey_F7]              = VK_F7,
        [NkKey_F8]              = VK_F8,
        [NkKey_F9]              = VK_F9,
        [NkKey_F10]             = VK_F10,
        [NkKey_F11]             = VK_F11,
        [NkKey_F12]             = VK_F12,
        [NkKey_NumLock]         = VK_NUMLOCK,
        [NkKey_Scroll]          = VK_SCROLL,
        [NkKey_LShift]          = VK_LSHIFT,
        [NkKey_RShift]          = VK_RSHIFT,
        [NkKey_LControl]        = VK_LCONTROL,
        [NkKey_RControl]        = VK_RCONTROL,
        [NkKey_LAlt]            = VK_LMENU,
        [NkKey_RAlt]            = VK_RMENU,
        [NkKey_Oem1]            = VK_OEM_1,
        [NkKey_Oem2]            = VK_OEM_PLUS,
        [NkKey_Oem3]            = VK_OEM_COMMA,
        [NkKey_Oem4]            = VK_OEM_MINUS,
        [NkKey_Oem5]            = VK_OEM_PERIOD,
        [NkKey_Oem6]            = VK_OEM_2,
        [NkKey_Oem7]            = VK_OEM_3,
        [NkKey_Oem8]            = VK_OEM_4,
        [NkKey_Oem9]            = VK_OEM_5,
        [NkKey_Oem10]           = VK_OEM_6,
        [NkKey_Oem11]           = VK_OEM_7
    };

    return gl_Nk2NtKeyMapping[keyCode & (NK_MAX_NUM_KEY_CODES - 1)];
}

/**
 */
NK_INTERNAL NkInt32 NK_CALL __NkInt_WindowsInput_MapToNativeMouseButton(
    _Inout_ NkIInput *self,
    _In_    NkMouseButton mouseBtn
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /**
     * \brief mapping table from Noriko mouse button codes to Win32 mouse button codes
     */
    NK_INTERNAL NkInt32 const gl_Nk2NtMouseBtnMapping[NK_MAX_NUM_MOUSE_BTNS] = {
        [NkBtn_LeftButton]   = VK_LBUTTON,
        [NkBtn_MiddleButton] = VK_MBUTTON,
        [NkBtn_RightButton]  = VK_RBUTTON,
        [NkBtn_Button4]      = VK_XBUTTON1,
        [NkBtn_Button5]      = VK_XBUTTON2
    };

    return gl_Nk2NtMouseBtnMapping[mouseBtn & (NK_MAX_NUM_MOUSE_BTNS - 1)];
}

/**
 */
NK_INTERNAL NkBoolean NK_CALL __NkInt_WindowsInput_IsKeyPressed(_Inout_ NkIInput *self, _In_ NkKeyboardKey keyCode) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(keyCode >= 0 && keyCode < NK_MAX_NUM_KEY_CODES, NkErr_InParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Check if the key is currently down. */
    return GetAsyncKeyState((int)__NkInt_WindowsInput_MapToNativeKey(self, keyCode)) & 0x8000;
}

/**
 */
NK_INTERNAL NkBoolean NK_CALL __NkInt_WindowsInput_IsMouseButtonPressed(
    _Inout_ NkIInput *self,
    _In_    NkMouseButton mouseBtn
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(mouseBtn >= 0 && mouseBtn < NK_MAX_NUM_MOUSE_BTNS, NkErr_InParameter);

    /* Check if mouse button is pressed. */
    return GetAsyncKeyState((int)__NkInt_WindowsInput_MapToNativeMouseButton(self, mouseBtn)) & 0x8000;
}

/**
 */
NK_INTERNAL NkPoint2D NK_CALL __NkInt_WindowsInput_GetMousePosition(_Inout_ NkIInput *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Retrieve global cursor position (in screen coordinates). */
    POINT curPos = { 0, 0 };
    GetCursorPos(&curPos);

    /* Initialize and return structure. */
    return (NkPoint2D){
        .m_xCoord = (NkInt64)curPos.x,
        .m_yCoord = (NkInt64)curPos.y
    };
}

/**
 */
NK_INTERNAL NkModifierKeys NK_CALL __NkInt_WindowsInput_GetModifierKeyStates(_Inout_ NkIInput *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return (
          __NkInt_WindowsInput_IsKeyPressed(self, NkKey_LShift)   << 0
        | __NkInt_WindowsInput_IsKeyPressed(self, NkKey_RShift)   << 1
        | __NkInt_WindowsInput_IsKeyPressed(self, NkKey_LControl) << 2
        | __NkInt_WindowsInput_IsKeyPressed(self, NkKey_RControl) << 3
        | __NkInt_WindowsInput_IsKeyPressed(self, NkKey_Context)  << 4
        | __NkInt_WindowsInput_IsKeyPressed(self, NkKey_LAlt)     << 5
        | __NkInt_WindowsInput_IsKeyPressed(self, NkKey_RAlt)     << 6
        | __NkInt_WindowsInput_IsKeyPressed(self, NkKey_LSuper)   << 7
        | __NkInt_WindowsInput_IsKeyPressed(self, NkKey_RSuper)   << 8
    );
}


/**
 * \brief actual instance of the Win32 IAL 
 */
NK_INTERNAL NkIInput const gl_Win32Ial = {
    .VT = &(struct __NkIInput_VTable__){
        .QueryInterface           = &__NkInt_WindowsInput_QueryInterface,
        .AddRef                   = &__NkInt_WindowsInput_AddRef,
        .Release                  = &__NkInt_WindowsInput_Release,
        .MapFromNativeKey         = &__NkInt_WindowsInput_MapFromNativeKey,
        .MapFromNativeMouseButton = &__NkInt_WindowsInput_MapFromNativeMouseButton,
        .MapToNativeKey           = &__NkInt_WindowsInput_MapToNativeKey,
        .MapToNativeMouseButton   = &__NkInt_WindowsInput_MapToNativeMouseButton,
        .IsKeyPressed             = &__NkInt_WindowsInput_IsKeyPressed,
        .IsMouseButtonPressed     = &__NkInt_WindowsInput_IsMouseButtonPressed,
        .GetMousePosition         = &__NkInt_WindowsInput_GetMousePosition,
        .GetModifierKeyStates     = &__NkInt_WindowsInput_GetModifierKeyStates
    }
};
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL __NkVirt_IAL_Startup(NkVoid) {
    /* Stub. */
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL __NkVirt_IAL_Shutdown(NkVoid) {
    /* Stub. */
    return NkErr_Ok;
}

NkIBase *NK_CALL __NkVirt_IAL_QueryInstance(NkVoid) {
    /*
     * Since this instance is static and does not have any data, we do not need to
     * increment the instance's reference count.
     */
    return (NkIBase *)&gl_Win32Ial;
}


#undef NK_NAMESPACE


