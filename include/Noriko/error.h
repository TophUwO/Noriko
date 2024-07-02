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


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>


#if (defined NK_CONFIG_DEBUG) || (defined NK_CONFIG_DEBUG_OPTIMIZED) || (defined NK_USE_ASSERTIONS)
    /**
     * \def   NK_ASSERT_EXTRA(expr, ec, extra)
     * \brief collects context information and raises a fatal error, resulting in a
     *        notice to the user and subsequent debugger invocation (if possible)
     * \param expr expression to check (must evaluate to *true* or else the assertion
     *        fails)
     * \param ec error code to propagate to the notice
     * \param extra (optional) extra text to show, providing additional context
     *        information
     * \see   NK_ASSERT
     * \note  The codebase is stripped of this macro in deploy builds, unless the
     *        **NK_USE_ASSERTIONS** macro is defined.
     * \note  The **NK_NAMESPACE** macro must be defined in each compilation unit this
     *        macro is used in. **NK_NAMESPACE** must expand to a plain C-string literal,
     *        UTF-8 encoded.
     *        \code{.c}
     *        #define NK_NAMESPACE u8"nk::utils"
     *        \endcode
     */
    #define NK_ASSERT_EXTRA(expr, ec, extra)                         \
        do {                                                         \
            if (!(expr)) {                                           \
                NkFatalTerminate(&(NkFatalErrorContext){             \
                    (ec),                                            \
                    (uint32_t)__LINE__,                              \
                    (NkStringView)NK_MAKE_STRING_VIEW(#expr),        \
                    (NkStringView)NK_MAKE_STRING_VIEW(u8##extra),    \
                    (NkStringView)NK_MAKE_STRING_VIEW(__FILE__),     \
                    (NkStringView)NK_MAKE_STRING_VIEW(NK_NAMESPACE), \
                    (NkStringView)NK_MAKE_STRING_VIEW(__func__),     \
                    NULL                                             \
                });                                                  \
            } else { }                                               \
        } while (0)

    /**
     * \def   NK_ASSERT(expr, ec)
     * \brief like NK_ASSERT_EXTRA but omits the *extra* parameter
     * \param expr expression to check (must evaluate to *true* or else the assertion
     *        fails)
     * \param ec error code to propagate to the notice
     * \note  The codebase is stripped of this macro in deploy builds.
     * \see   NK_ASSERT_EXTRA
     */
    #define NK_ASSERT(expr, ec) NK_ASSERT_EXTRA(expr, ec, "")
#else
    #define NK_ASSERT_EXTRA(expr, ec, extra)
    #define NK_ASSERT(expr, ec)
#endif


/**
 * \enum  NkErrorCode
 * \brief numeric error code definitions
 */
typedef _In_range_(0, __NkErr_Count__ - 1) enum NkErrorCode {
    NkErr_Ok,                   /**< no error */
    NkErr_Unknown,              /**< unknown error condition */
    NkErr_NoOperation,          /**< function did nothing */
    NkErr_ManuallyAborted,      /**< operation was manually aborted by user or callback */

    NkErr_NotImplemented,       /**< feature not implemented */
    NkErr_InParameter,          /**< erroneous input parameter */
    NkErr_OutParameter,         /**< erroneous output parameter */
    NkErr_InOutParameter,       /**< erroneous input/output parameter */
    NkErr_InptrParameter,       /**< erroneous input pointer parameter */
    NkErr_OutptrParameter,      /**< errornous output pointer parameter */
    NkErr_CallbackParameter,    /**< erroneous function pointer (callback) parameter */
    NkErr_MemoryAlignment,      /**< invalid memory alignment specified */
    NkErr_MemoryAllocation,     /**< error during memory allocation */
    NkErr_MemoryReallocation,   /**< error during memory reallocation */
    NkErr_NamedItemNotFound,    /**< requested item could not be found */
    NkErr_ArrayOutOfBounds,     /**< array index out of buffer bounds */
    NkErr_ArrayElemOutOfBounds, /**< array index out of element bounds */
    NkErr_InvalidRange,         /**< invalid range tuple */
    NkErr_UnsignedWrapAround,   /**< operation caused unsigned wrap-around */
    NkErr_CapLimitExceeded,     /**< container capacity limit exceeded */

    __NkErr_Count__             /**< used internally */
} NkErrorCode;

/**
 * \struct NkFatalErrorContext
 * \brief  represents additional information passed to fatal error handlers, intended for
 *         display and/or logging
 */
NK_NATIVE typedef struct NkFatalErrorContext {
    NkErrorCode   m_errorCode;      /**< fatal error code */
    uint32_t      m_fileLine;       /**< line in the file where the error was raised */
    NkStringView  m_failedExpr;     /**< string representation of the expression that failed */
    NkStringView  m_additionalDesc; /**< additional text shown to the user */
    NkStringView  m_filePath;       /**< file in which the throwing function is located */
    NkStringView  m_namespaceIdent; /**< namespace identifier of the function */
    NkStringView  m_functionName;   /**< function in which the error occurred */
    NkVoid       *mp_reservedPtr;   /**< *reserved for future use* */
} NkFatalErrorContext;


/**
 * \brief  retrieves the textual representation of the provided integral error code
 * \param  [in] code integer error code to get string representation of
 * \return textual representation of error code, as UTF-8 C-string view, or a string view
 *         signifying an error if the error code representation could not be retrieved
 * \note   The return value of this function is a pointer to static read-only memory.
 * \note   The resulting string value is generally the identifier of the enum literal
 *         associated with the provided integer error code.
 */
NK_NATIVE NK_API _Return_ok_ NkStringView const *NK_CALL NkGetErrorCodeStr(_In_ _Ecode_range_ NkErrorCode code);
/**
 * \brief  retrieves a brief textual description of the provided integral error code
 * \param  [in] code integer error code to get the associated error description of
 * \return textual representation of error code, as UTF-8 C-string view, or a string view
 *         signifying an error if the error description could not be retrieved
 * \note   The return value of this function is a pointer to static read-only memory.
 */
NK_NATIVE NK_API _Return_ok_ NkStringView const *NK_CALL NkGetErrorCodeDesc(_In_ _Ecode_range_ NkErrorCode code);


/**
 * \brief terminates the application immediately providing additional information on the
 *        error that caused the abnormal termination
 * \param [in] errCxtPtr pointer to a NkFatalErrorContext structure that contains more
 *             information on the errors
 * \see   NkFatalErrorContext
 * \see   NkErrorCode
 * \note  This function does not return.
 * \note  Any *atexit()*-handlers will be executed before quitting.
 */
NK_NATIVE NK_API NK_NORETURN NkVoid NK_CALL NkFatalTerminate(_In_ NkFatalErrorContext const *errCxtPtr);


