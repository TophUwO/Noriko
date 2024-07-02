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
 * \file  error.h
 * \brief error codes used by Noriko's engine component
 *
 * This header defines error codes which are returned by Noriko's public and internal API
 * functions. These can be converted to strings and brief descriptions using the utility
 * functions declared in this file.
 *
 * \note Noriko generally uses the function return value as the error codes. Result data
 *       is returned through function parameters.
 */
#define NK_NAMESPACE u8"nk::error"


/* stdlib includes */
#include <stdlib.h>
#include <stdio.h>

/* Noriko includes */
#include <include/Noriko/platform.h>
#include <include/Noriko/error.h>


/**
 * \internal
 * \brief textual representations for integral error code values
 * \note  The string values in this array are UTF-8-encoded.
 * \endinternal
 */
NK_INTERNAL NkStringView const gl_c_ErrorCodeStringTable[] = {
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_Ok)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_Unknown)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_NoOperation)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ManuallyAborted)),

    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_NotImplemented)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_OutParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InOutParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InptrParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_OutptrParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_CallbackParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_MemoryAlignment)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_MemoryAllocation)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_MemoryReallocation)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_NamedItemNotFound)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ArrayOutOfBounds)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ArrayElemOutOfBounds)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InvalidRange)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_UnsignedWrapAround)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_CapLimitExceeded))
};
static_assert(NK_ARRAYSIZE(gl_c_ErrorCodeStringTable) == __NkErr_Count__, u8"Error code string array mismatch.");

/**
 * \internal
 * \brief brief descriptions for error codes for use in error messages that are to be
 *        shown to the user
 * \note  The string values in this array are UTF-8-encoded.
 * \endinternal
 */
NK_INTERNAL NkStringView const gl_c_ErrorCodeDescriptionTable[] = {
    NK_MAKE_STRING_VIEW(u8"not an error"),
    NK_MAKE_STRING_VIEW(u8"unknown error code or totally unexpected error condition"),
    NK_MAKE_STRING_VIEW(u8"no operation was carried out"),
    NK_MAKE_STRING_VIEW(u8"operation was manually aborted by user or callback"),

    NK_MAKE_STRING_VIEW(u8"requested feature is not (yet) implemented"),
    NK_MAKE_STRING_VIEW(u8"at least one erroneous input (read-only) parameter (e.g., int, char *)"),
    NK_MAKE_STRING_VIEW(u8"at least one erroneous output (write-only) parameter (e.g., void *)"),
    NK_MAKE_STRING_VIEW(u8"at least one erroneous input/output parameter (e.g., void *)"),
    NK_MAKE_STRING_VIEW(u8"at least one erroneous input pointer parameter (e.g., void **)"),
    NK_MAKE_STRING_VIEW(u8"at least one erroneous output pointer parameter (e.g., void **)"),
    NK_MAKE_STRING_VIEW(u8"invalid callback function pointer passed (must be non-NULL)"),
    NK_MAKE_STRING_VIEW(u8"invalid memory alignment (must be a power of two)"),
    NK_MAKE_STRING_VIEW(u8"could not allocate memory block (likely out of memory or too much fragmentation)"),
    NK_MAKE_STRING_VIEW(u8"could not reallocate memory block (likely out of memory or too much fragmentation)"),
    NK_MAKE_STRING_VIEW(u8"could not find requested named item"),
    NK_MAKE_STRING_VIEW(u8"array index out of (buffer) bounds"),
    NK_MAKE_STRING_VIEW(u8"array index out of (element) bounds"),
    NK_MAKE_STRING_VIEW(u8"erroneous array interval [x, y]"),
    NK_MAKE_STRING_VIEW(u8"operation caused unsigned integer wrap-around (values passed too big?)"),
    NK_MAKE_STRING_VIEW(u8"exceeded container capacity limit")
};
static_assert(NK_ARRAYSIZE(gl_c_ErrorCodeDescriptionTable) == __NkErr_Count__, u8"Error code desc array mismatch.");


/**
 * \internal
 * \brief  formats the fatal error message that is then shown to the user
 * \param  [in] errCxtPtr pointer to a NkFatalErrorContext structure containing additional
 *              information on the error (used for formatting)
 * \return pointer to the formatted string buffer (*NUL*-terminated)
 * \see    NkFatalErrorContext
 * \endinternal
 */
NK_INTERNAL char const *NK_CALL NkInternalFormatFatalErrorMessage(_In_ NkFatalErrorContext const *errCxtPtr) {
    NK_INTERNAL char               gl_int_FormatBuffer[2 << 12] = { 0x00 };
    NK_INTERNAL NkStringView const gl_int_DefExtra = NK_MAKE_STRING_VIEW(
        u8"This error signifies an abnormal program termination. Please contact the"
        u8" responsible developer, providing the details shown by this error message."
    );

    /* Get extra message. */
    NkStringView const *const extraMsg = errCxtPtr->m_additionalDesc.m_sizeInBytes == 1
        ? &gl_int_DefExtra
        : &errCxtPtr->m_additionalDesc
    ;
    /*
     * Shorten file name string to only show paths relative to the root directory of the
     * project.
     */
    char const *pathStartPtr = strstr(errCxtPtr->m_filePath.mp_dataPtr, u8"Noriko");
    if (pathStartPtr == NULL)
        pathStartPtr = errCxtPtr->m_filePath.mp_dataPtr;

    /* Format the text buffer. */
    snprintf(
        gl_int_FormatBuffer,
        NK_ARRAYSIZE(gl_int_FormatBuffer) - 1,
        u8"An unrecoverable error occurred and the application was forced to halt:\n\n"
        u8"  Expr:\t%s\n"
        u8"  Code:\t%s (%i)\n"
        u8"  Desc:\t%s\n"
        u8"  File:\t%s\n"
        u8"  Line:\t%i\n"
        u8"  Func:\t%s::%s()\n\n"
        u8"%s",
        errCxtPtr->m_failedExpr.mp_dataPtr,                     /* expression */
        NkGetErrorCodeStr(errCxtPtr->m_errorCode)->mp_dataPtr,  /* string rep for error code */
        errCxtPtr->m_errorCode,                                 /* numeric error code */
        NkGetErrorCodeDesc(errCxtPtr->m_errorCode)->mp_dataPtr, /* desc for error code */
        pathStartPtr,                                           /* stripped file path */
        errCxtPtr->m_fileLine,                                  /* line in file */
        errCxtPtr->m_namespaceIdent.mp_dataPtr,                 /* namespace identifier */
        errCxtPtr->m_functionName.mp_dataPtr,                   /* name of function */
        extraMsg->mp_dataPtr                                    /* extra message */
    );

    return &gl_int_FormatBuffer[0];
}


_Return_ok_ NkStringView const *NK_CALL NkGetErrorCodeStr(_In_ _Ecode_range_ NkErrorCode code) {
    /* Check if parameter is in range. */
    if (!NK_INRANGE_INCL(code, NkErr_Ok, __NkErr_Count__ - 1))
        return NkGetErrorCodeStr(NkErr_Unknown);

    /* All good. */
    return &gl_c_ErrorCodeStringTable[code];
}

_Return_ok_ NkStringView const *NK_CALL NkGetErrorCodeDesc(_In_ _Ecode_range_ NkErrorCode code) {
    /* Check if parameter is in range. */
    if (!NK_INRANGE_INCL(code, NkErr_Ok, __NkErr_Count__ - 1))
        return NkGetErrorCodeDesc(NkErr_Unknown);

    /* All good. */
    return &gl_c_ErrorCodeDescriptionTable[code];
}


NK_API NK_NORETURN NkVoid NK_CALL NkFatalTerminate(_In_ NkFatalErrorContext const *errCxtPtr) {
    NK_ASSERT(errCxtPtr != NULL, NkErr_InptrParameter);
    
#if (defined NK_TARGET_WINDOWS)
    MessageBoxA(
        NULL, 
        NkInternalFormatFatalErrorMessage(errCxtPtr),
        u8"Fatal Error", 
        MB_OK | MB_ICONERROR
    );
#endif

    /* Quit, passing the specified error code to the host platform. */
    exit(errCxtPtr->m_errorCode);
}


#undef NK_NAMESPACE


