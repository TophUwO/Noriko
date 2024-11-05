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
 * \file    string.h
 * \brief   defines the public API for mutable run-time string instances
 * 
 * Noriko uses UTF-8 as its internal encoding across all platforms, regardless of native
 * platform encoding. This means that on platforms which use a different native encoding
 * than UTF-8, like Windows, strings have to be converted to the native platform encoding
 * on the fly, for which reason they should be cached as much as possible.<br>
 * 
 * The string type defined by this file supports major basic Unicode string operations, 
 * such as <em>case-folding</em>, and general string operations that are UTF-8-aware.
 * 
 * \note    Strings are always <tt>NUL</tt>-terminated.
 * \warning Strings in Noriko are generally <em>unsafe</em> for performance reasons. This
 *          means that the library will never validate strings passed to it. The using
 *          module is responsible for ensuring validity of strings coming from external
 *          sources such as files, networks, or process-shared memory.
 * 
 * \par Remarks
 *   Advanced unicode operations such as <em>normalization</em>, or locale support are
 *   currently not implemented. Such functionality may be added later.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>
#include <include/Noriko/util.h>


/**
 */
NK_DEFINE_PROTOTYPE(NkString, NK_ALIGNOF(NkInt64), 16);


/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkStringCreate(
    _In_opt_z_ _Utf8_ char const *fromStr,
    _In_              NkUint32 count,
    _Out_             NkString *resPtr
);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkStringCreateView(
    _In_     NkString const *srcPtr,
    _In_     NkUint32 start,
    _In_opt_ NkUint32 maxCount,
    _Out_    NkStringView *resPtr
);
/**
 */
NK_NATIVE NK_API NkVoid NK_CALL NkStringDestroy(NkString *strPtr);

/**
 */
NK_NATIVE NK_API NkVoid NK_CALL NkStringClear(_Inout_ NkString *strPtr);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkStringJoin(
    _Inout_       NkString *strPtr,
    _In_z_ _Utf8_ char const *elemStr,
    _In_opt_      NkUint32 strLen
);

/**
 */
NK_NATIVE NK_API NkUint32 NK_CALL NkStringGetLength(_In_ NkString const *strPtr);
/**
 */
NK_NATIVE NK_API char const *NK_CALL NkStringAt(_In_ NkString const *strPtr, _In_ NkUint32 off);

/**
 */
NK_NATIVE NK_API char const *NK_CALL NkStringIterate(_In_z_ _Utf8_ char const *strPtr);


