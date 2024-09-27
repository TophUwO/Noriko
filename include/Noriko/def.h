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
 * \file  def.h
 * \brief global definitions used by Noriko's public API
 *
 * This header defines annotation macros that can be used to modify symbols that
 * are to be exported and used by other components of the Noriko ecosystem.
 */


#pragma once

/* stdlib includes */
#include <stdint.h>


/* Annotations for exporting symbols. */
#if (defined __cplusplus)
    #define NK_NATIVE               extern "C"
#else
    #define NK_NATIVE
#endif
#if (defined NK_BUILD_ENGINE)
    #define NK_API                  extern __declspec(dllexport)
#elif (defined NK_IMPORT_ENGINE)
    #define NK_API                  extern __declspec(dllimport)
#endif
#define NK_CALL                     __cdecl
#define NK_EXTERN                   extern
#define NK_INTERNAL                 static
#define NK_NORETURN                 __declspec(noreturn)
#define NK_INLINE                   inline
/**
 * \def   NK_VIRTUAL
 * \brief marks a function as 'virtual', that is, must be implemented per-platform and
 *        has no default implementation
 * 
 * \par Remarks
 *   While this symbol has no significance to the compiler, it is there for annotation
 *   purposes. It makes it obvious what functions a developer porting Noriko to a new
 *   platform must implement in order for it to work. Each virtual functions has detailed
 *   behavioral documentation attached to it.
 */
#define NK_VIRTUAL

/* Use SALv2 on MSVC platform. */
/** \cond */
#if ((defined _WIN32) && (defined __has_include) && __has_include(<sal.h>))
    #define NK_USE_SAL
    #include <sal.h>

    #define _Return_ok_             _Check_return_ _Success_(return == NkErr_Ok)
    #define _Return_false_          _Check_return_ _Success_(return == NK_FALSE)
    #define _Return_notnull_        _Check_return_ _Success_(return != NULL)
    #define _Ecode_range_           _In_range_(NkErr_Ok, __NkErr_Count__ - 1)
    #define _Init_ptr_              _Outptr_ _Deref_post_notnull_ 
    #define _Reinit_ptr_            _Init_ptr_ _Deref_pre_valid_
    #define _Uninit_ptr_            _Pre_valid_ _Deref_post_null_
    #define _Init_ptr_maybe_        _Outptr_result_maybenull_
    #define _Init_ptr_mbnull_       _Outptr_opt_result_maybenull_
    #define _I_array_(s)            _In_reads_(s)
    #define _O_array_(s)            _Out_writes_(s)
    #define _O_array_opt_(s)        _Out_writes_opt_(s)
    #define _I_bytes_(s)            _In_reads_bytes_(s)
    #define _O_bytes_(s)            _Out_writes_bytes_(s)
    #define _O_bytes_opt_(s)        _Out_writes_bytes_opt_(s)
    #define _Format_str_            _Printf_format_string_
    #define _Maybe_reinit_          _Init_ptr_ _Deref_pre_opt_valid_
    #define _In_to_null_            _In_reads_to_ptr_opt_(NULL)
    #define _Pre_maybevalid_        _Pre_ _Maybevalid_
#else
    #define _In_
    #define _In_opt_
    #define _Out_
    #define _Outptr_
    #define _Inout_
    #define _Field_z_
    #define _Struct_size_bytes_(n)
    #define _Return_ok_
    #define _Return_false_
    #define _Return_notnull_
    #define _Success_(expr)
    #define _Check_return_
    #define _In_range_(lo, hi)
    #define _Ecode_range_
    #define _Pre_notnull_
    #define _Pre_valid_
    #define _Post_valid_
    #define _Deref_pre_valid_
    #define _Deref_post_null_
    #define _Deref_post_notnull_
    #define _Init_ptr_
    #define _Reinit_ptr_
    #define _Uninit_ptr_
    #define _Init_ptr_maybe_
    #define _In_reads_(s)
    #define _Out_writes_(s)
    #define _Init_ptr_mbnull_
    #define _I_array_(s)
    #define _O_array_(s)
    #define _O_array_opt_(s)
    #define _I_bytes_(s)
    #define _O_bytes_(s)
    #define _O_bytes_opt_(s) 
    #define _Format_str_
    #define _Maybe_reinit_
    #define _In_to_null_
    #define _Pre_maybevalid_
    #define _Outptr_result_maybenull_
    #define _Outptr_opt_result_maybenull_
#endif

/*
 * Define compatibility macros for _Alignof and _Alignas (since they are not valid in
 * C++).
 */
#if (!defined __cplusplus)
    #define alignas(x) _Alignas(x)
    #define alignof(x) _Alignof(x)
#endif
/** \endcond */


/**
 * \defgroup Aliases
 * \brief    makes some primitive standard types conforming to Noriko's naming convention
 */
/** @{ */
#if (defined __cplusplus)
    NK_NATIVE typedef bool  NkBoolean;
#else
    NK_NATIVE typedef _Bool NkBoolean;
#endif
NK_NATIVE typedef void      NkVoid;
NK_NATIVE typedef size_t    NkSize;
NK_NATIVE typedef ptrdiff_t NkOffset;
NK_NATIVE typedef float     NkFloat, NkSingle;
NK_NATIVE typedef double    NkDouble;
NK_NATIVE typedef int8_t    NkInt8;
NK_NATIVE typedef int16_t   NkInt16;
NK_NATIVE typedef int32_t   NkInt32;
NK_NATIVE typedef int64_t   NkInt64;
NK_NATIVE typedef uint8_t   NkUint8, NkByte;
NK_NATIVE typedef uint16_t  NkUint16;
NK_NATIVE typedef uint32_t  NkUint32;
NK_NATIVE typedef uint64_t  NkUint64, NkFlags;
/** @} */


