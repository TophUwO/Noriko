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
static NkStringView const gl_c_ErrorCodeStringTable[] = {
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_Ok)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_Unknown)),

    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_NotImplemented)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_OutParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InOutParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_InptrParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_OutptrParameter)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_MemoryAlignment)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_MemoryAllocation)),
    NK_MAKE_STRING_VIEW(NK_ESC(NkErr_MemoryReallocation))
};
static_assert(NK_ARRAYSIZE(gl_c_ErrorCodeStringTable) == __NkErr_Count__, u8"Error code string array mismatch.");

/**
 * \brief brief descriptions for error codes for use in error messages that are to be
 *        shown to the user
 * \note  The string values in this array are UTF-8-encoded.
 */
static NkStringView const gl_c_ErrorCodeDescriptionTable[] = {
    NK_MAKE_STRING_VIEW(u8"not an error"),
    NK_MAKE_STRING_VIEW(u8"unknown error code or totally unexpected error condition"),

    NK_MAKE_STRING_VIEW(u8"requested feature is not (yet) implemented"),
    NK_MAKE_STRING_VIEW(u8"at least one errornous input (read-only) parameter (e.g., int, char *)"),
    NK_MAKE_STRING_VIEW(u8"at least one errornous output (write-only) parameter (e.g., void *)"),
    NK_MAKE_STRING_VIEW(u8"at least one errornous input/output parameter (e.g., void *)"),
    NK_MAKE_STRING_VIEW(u8"at least one errornous input pointer parameter (e.g., void **)"),
    NK_MAKE_STRING_VIEW(u8"at least one errorous output pointer parameter (e.g., void **)"),
    NK_MAKE_STRING_VIEW(u8"invalid memory alignment (must be a power of two)"),
    NK_MAKE_STRING_VIEW(u8"could not allocate memory block (likely out of memory or too much fragmentation)"),
    NK_MAKE_STRING_VIEW(u8"could not reallocate memory block (likely out of memory or too much fragmentation)")
};
static_assert(NK_ARRAYSIZE(gl_c_ErrorCodeDescriptionTable) == __NkErr_Count__, u8"Error code desc array mismatch.");


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


