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
 * \file  error.c
 * \brief error codes used by Noriko's engine component
 *
 * This header defines error codes which are returned by Noriko's public and internal API
 * functions. These can be converted to strings and brief descriptions using the utility
 * functions declared in this file.
 *
 * \note Noriko generally uses the function return value as the error codes. Result data
 *       is returned through function parameters.
 */
#define NK_NAMESPACE "nk::error"


/* stdlib includes */
#include <stdlib.h>
#include <stdio.h>

/* Noriko includes */
#include <include/Noriko/platform.h>
#include <include/Noriko/error.h>
#include <include/Noriko/log.h>


/** \cond INTERNAL */
/**
 * \def   NK_ERROR_MSGBUF
 * \brief size in bytes of the fatal error message buffer
 */
#define NK_ERROR_MSGBUF ((NkSize)(1 << 12))


/**
 * \brief textual representations for integral error code values
 */
NK_INTERNAL NkStringView const gl_c_ErrorCodeStringTable[] = {
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_Ok)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_Unknown)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_NoOperation)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ManuallyAborted)),

    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_AccessDenied)),
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
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ItemNotFound)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ArrayOutOfBounds)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ArrayElemOutOfBounds)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InvalidRange)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_UnsignedWrapAround)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_CapLimitExceeded)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ComponentState)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ObjectType)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ObjectState)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_SynchInit)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_UnexpectedCharacter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InvalidIdentifier)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ClosingTokenNotFound)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InterfacePureVirtual)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InterfaceNotImpl)),    
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_UnknownClass)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_AggregationNotSupp)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ClassAlreadyReg)),     
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_ClassNotReg)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_RegWindowClass)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_CreateNativeWindow)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_WndModeNotSupported)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_AdjustClientArea)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_CreateMemDC)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_CreateCompBitmap)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_CreateBrush))
};
static_assert(NK_ARRAYSIZE(gl_c_ErrorCodeStringTable) == __NkErr_Count__, "Error code string array mismatch!");

/**
 * \brief brief descriptions for error codes for use in error messages that are to be
 *        shown to the user
 */
NK_INTERNAL NkStringView const gl_c_ErrorCodeDescriptionTable[] = {
    NK_MAKE_STRING_VIEW("not an error"),
    NK_MAKE_STRING_VIEW("unknown error code or totally unexpected error condition"),
    NK_MAKE_STRING_VIEW("no operation was carried out"),
    NK_MAKE_STRING_VIEW("operation was manually aborted by user or callback"),

    NK_MAKE_STRING_VIEW("cannot access resource"),
    NK_MAKE_STRING_VIEW("requested feature is not (yet) implemented"),
    NK_MAKE_STRING_VIEW("at least one erroneous input (read-only) parameter (e.g., int, char *)"),
    NK_MAKE_STRING_VIEW("at least one erroneous output (write-only) parameter (e.g., void *)"),
    NK_MAKE_STRING_VIEW("at least one erroneous input/output parameter (e.g., void *)"),
    NK_MAKE_STRING_VIEW("at least one erroneous input pointer parameter (e.g., void **)"),
    NK_MAKE_STRING_VIEW("at least one erroneous output pointer parameter (e.g., void **)"),
    NK_MAKE_STRING_VIEW("invalid callback function pointer passed (must be non-NULL)"),
    NK_MAKE_STRING_VIEW("invalid memory alignment (must be a power of two)"),
    NK_MAKE_STRING_VIEW("could not allocate memory block (likely out of memory or too much fragmentation)"),
    NK_MAKE_STRING_VIEW("could not reallocate memory block (likely out of memory or too much fragmentation)"),
    NK_MAKE_STRING_VIEW("could not find requested item"),
    NK_MAKE_STRING_VIEW("array index out of (buffer) bounds"),
    NK_MAKE_STRING_VIEW("array index out of (element) bounds"),
    NK_MAKE_STRING_VIEW("erroneous array interval [x, y]"),
    NK_MAKE_STRING_VIEW("operation caused unsigned integer wrap-around (values passed too big?)"),
    NK_MAKE_STRING_VIEW("exceeded container capacity limit"),
    NK_MAKE_STRING_VIEW("invalid component state (not yet initialized/already uninitialized?)"),
    NK_MAKE_STRING_VIEW("invalid internal object type"),
    NK_MAKE_STRING_VIEW("invalid object state (likely due to function pre-condition being not satisfied)"),
    NK_MAKE_STRING_VIEW("error while initializing synchronization object (mtx, ...)"),
    NK_MAKE_STRING_VIEW("unexpected character during string parsing"),
    NK_MAKE_STRING_VIEW("invalid/empty identifier during string parsing"),
    NK_MAKE_STRING_VIEW("closing token for compound literal not found"),
    NK_MAKE_STRING_VIEW("interface is marked as 'pure-virtual' and cannot be instantiated"),
    NK_MAKE_STRING_VIEW("current class does not implement the specified interface"),
    NK_MAKE_STRING_VIEW("class is unknown to the current class factory instance"),
    NK_MAKE_STRING_VIEW("class does not support NkOM aggregation"),
    NK_MAKE_STRING_VIEW("class is already registered in the global NkOM runtime"),
    NK_MAKE_STRING_VIEW("class is not registered in the global NkOM runtime"),
    NK_MAKE_STRING_VIEW("could not register window class"),
    NK_MAKE_STRING_VIEW("could not create native window"),
    NK_MAKE_STRING_VIEW("window mode not supported on the current platform"),
    NK_MAKE_STRING_VIEW("could not adjust client area size to fit requested viewport"),
    NK_MAKE_STRING_VIEW("could not create memory device context"),
    NK_MAKE_STRING_VIEW("failed to create memory bitmap compatible with given device context"),
    NK_MAKE_STRING_VIEW("failed to create paint brush")
};
static_assert(NK_ARRAYSIZE(gl_c_ErrorCodeDescriptionTable) == __NkErr_Count__, "Error code desc array mismatch!");


/**
 * \brief  formats the fatal error message that is then shown to the user
 * \param  [in] errCxtPtr pointer to a NkFatalErrorContext structure containing additional
 *              information on the error (used for formatting)
 * \param  [out] outBuf destination buffer of the formatted error message; must be at
 *         least \c NK_ERROR_MSGBUF bytes in size (incl. <tt>NUL</tt>-terminator)
 * \return pointer to the formatted string buffer (<tt>NUL</tt>-terminated)
 * \see    NkFatalErrorContext
 */
NK_INTERNAL char const *__NkInt_FormatFatalErrorMessage(
    _In_                          NkFatalErrorContext const *errCxtPtr,
    _Out_writes_(NK_ERROR_MSGBUF) char *outBuf
) {
    NK_INTERNAL NkStringView const gl_int_DefExtra = NK_MAKE_STRING_VIEW(
        "This error signifies abnormal program termination. Please contact the"
        " responsible developer, providing the details shown by this error message."
    );

    /* Get extra message. */
    NkStringView const *const extraMsg = errCxtPtr->m_additionalDesc.m_sizeInBytes == 0
        ? &gl_int_DefExtra
        : &errCxtPtr->m_additionalDesc
    ;
    /*
     * Shorten file name string to only show paths relative to the root directory of the
     * project.
     */
    char const *pathStartPtr = strstr(errCxtPtr->m_filePath.mp_dataPtr, "Noriko");
    if (pathStartPtr == NULL)
        pathStartPtr = errCxtPtr->m_filePath.mp_dataPtr;

    /* Format the text buffer. */
    snprintf(
        outBuf,
        NK_ERROR_MSGBUF - 1,
        "An unrecoverable error occurred and the application was forced to halt:\n\n"
        "  Expr:\t%s\n"
        "  Code:\t%s (%i)\n"
        "  Desc:\t%s\n"
        "  File:\t%s\n"
        "  Line:\t%i\n"
        "  Func:\t%s::%s()\n\n"
        "%s",
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

    return outBuf;
}
/** \endcond */


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
    /* Only print error details if there was a context passed to this function. */
    if (errCxtPtr != NULL) {
        char gl_int_FormatBuffer[NK_ERROR_MSGBUF];
        char const *msgPtr = __NkInt_FormatFatalErrorMessage(errCxtPtr, gl_int_FormatBuffer);
        NK_LOG_CRITICAL(msgPtr);

#if (defined NK_TARGET_WINDOWS)
        MessageBoxA(
            NULL, 
            msgPtr,
            "Fatal Error", 
            MB_OK | MB_ICONERROR
        );
#endif
    }

    /* Quit, passing the specified error code to the host platform. */
    exit(errCxtPtr == NULL ? NkErr_Unknown : errCxtPtr->m_errorCode);
}


#undef NK_NAMESPACE


