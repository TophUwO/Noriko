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
 * \brief implements the platform-specific input translator for Windows
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

}

/**
 */
NK_INTERNAL NkMouseButton NK_CALL __NkInt_WindowsInput_MapFromNativeMouseButton(
    _Inout_ NkIInput *self,
    _In_    NkInt32 ntMouseBtn
) {

}

/**
 */
NK_INTERNAL NkInt32 NK_CALL __NkInt_WindowsInput_MapToNativeKey(_Inout_ NkIInput *self, _In_ NkKeyboardKey keyCode) {

}

/**
 */
NK_INTERNAL NkInt32 NK_CALL __NkInt_WindowsInput_MapToNativeMouseButton(
    _Inout_ NkIInput *self,
    _In_    NkMouseButton mouseBtn
) {

}

/**
 */
NK_INTERNAL NkBoolean NK_CALL __NkInt_WindowsInput_IsKeyPressed(_Inout_ NkIInput *self, _In_ NkKeyboardKey keyCode) {

}

/**
 */
NK_INTERNAL NkBoolean NK_CALL __NkInt_WindowsInput_IsMouseButtonPressed(
    _Inout_ NkIInput *self,
    _In_    NkMouseButton mouseBtn
) {

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
    return (NkPoint2D) {
        .m_xCoord = curPos.x,
        .m_yCoord = curPos.y
    };
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkInputStartup(NkVoid) {
    NK_LOG_INFO("startup: IAL");

    /* Stub. */
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkInputShutdown(NkVoid) {
    NK_LOG_INFO("shutdown: IAL");

    /* Same as above. */
    return NkErr_Ok;
}


NkIInput *NK_CALL NkInputQueryInstance(NkVoid) {
    /** \cond INTERNAL */
    /**
     * \brief actual instance of the input translator's VTable 
     */
    NKOM_DEFINE_VTABLE(NkIInput) {
        .QueryInterface           = &__NkInt_WindowsInput_QueryInterface,
        .AddRef                   = &__NkInt_WindowsInput_AddRef,
        .Release                  = &__NkInt_WindowsInput_Release,
        .MapFromNativeKey         = &__NkInt_WindowsInput_MapFromNativeKey,
        .MapFromNativeMouseButton = &__NkInt_WindowsInput_MapFromNativeMouseButton,
        .MapToNativeKey           = &__NkInt_WindowsInput_MapToNativeKey,
        .MapToNativeMouseButton   = &__NkInt_WindowsInput_MapToNativeMouseButton,
        .IsKeyPressed             = &__NkInt_WindowsInput_IsKeyPressed,
        .IsMouseButtonPressed     = &__NkInt_WindowsInput_IsMouseButtonPressed,
        .GetMousePosition         = &__NkInt_WindowsInput_GetMousePosition
    };
    /** \endcond */

    /*
     * Since this instance is static and does not have any data, we do not need to
     * increment the instance's reference count.
     */
    return (NkIInput *)&NKOM_VTABLEOF(NkIInput);
}


#undef NK_NAMESPACE


