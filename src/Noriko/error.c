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

/* stdlib includes */
#include <stdlib.h>

/* Noriko includes */
#include <include/Noriko/error.h>


/**
 * \brief textual representations for integral error code values
 * \note  The string values in this array are UTF-8-encoded.
 */
static char const *const gl_c_ErrorCodeStringTable[] = {
    NK_ESC(NkErr_Ok),
    NK_ESC(NkErr_Unknown),

    NK_ESC(NkErr_NotImplemented),
    NK_ESC(NkErr_InParameter),
    NK_ESC(NkErr_OutParameter),
    NK_ESC(NkErr_InOutParameter)
};
static_assert(NK_ARRAYSIZE(gl_c_ErrorCodeStringTable) == __NkErr_Count__, u8"Error code string array mismatch.");

/**
 * \brief brief descriptions for error codes for use in error messages that are to be
 *        shown to the user
 * \note  The string values in this array are UTF-8-encoded.
 */
static char const *const gl_c_ErrorCodeDescriptionTable[] = {
    u8"not an error",
    u8"unknown error code or totally unexpected error condition",

    u8"requested feature is not (yet) implemented",
    u8"at least one errornous input (read-only) parameter",
    u8"at least one errornous output (write-only) parameter",
    u8"at least one errornous input/output parameter"
};
static_assert(NK_ARRAYSIZE(gl_c_ErrorCodeDescriptionTable) == __NkErr_Count__, u8"Error code desc array mismatch.");


_Return_ok_ char const *NK_CALL NkGetErrorCodeString(_In_ _Ecode_range_ NkErrorCode const ecode) {
    /* Check if parameter is in range. */
    if (!NK_INRANGE_INCL(ecode, NkErr_Ok, __NkErr_Count__ - 1))
        return NULL;

    /* All good. */
    return gl_c_ErrorCodeStringTable[ecode];
}

_Return_ok_ char const *NK_CALL NkGetErrorCodeDesc(_In_ _Ecode_range_ NkErrorCode const ecode) {
    /* Check if parameter is in range. */
    if (!NK_INRANGE_INCL(ecode, NkErr_Ok, __NkErr_Count__ - 1))
        return NULL;

    /* All good. */
    return gl_c_ErrorCodeDescriptionTable[ecode];
}


