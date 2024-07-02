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
#define NK_PROTOTYPE                extern
#define NK_INTERNAL                 static
#define NK_NORETURN                 __declspec(noreturn)
#define NK_INLINE                   inline

/* Use SALv2 on MSVC platform. */
/** \cond */
#if (__has_include(<sal.h>))
    #include <sal.h>

    #define _Return_ok_             _Check_return_ _Success_(return == NkErr_Ok)
    #define _Ecode_range_           _In_range_(NkErr_Ok, __NkErr_Count__ - 1)
    #define _Init_ptr_              _Outptr_ _Deref_post_notnull_ 
    #define _Reinit_ptr_            _Init_ptr_ _Deref_pre_valid_
    #define _Uninit_ptr_            _Deref_pre_notnull_ _Deref_post_null_
    #define _Init_ptr_mbnull_       _Outptr_opt_result_maybenull_
    #define _I_array_(s)            _In_reads_(s)
    #define _O_array_(s)            _Out_writes_(s)
    #define _O_array_opt_(s)        _Out_writes_opt_(s)
#else
    #define _In_
    #define _In_opt_
    #define _Out_
    #define _Outptr_
    #define _Inout_
    #define _Field_z_
    #define _Struct_size_bytes_(n)
    #define _Return_ok_
    #define _Success_(expr)
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
    #define _In_reads_(s)
    #define _Out_writes_(s)
    #define _Init_ptr_mbnull_
    #define _I_array_(s)
    #define _O_array_(s)
    #define _O_array_opt_(s)
#endif
/** \endcond */


/**
 * \defgroup Primitive Type Aliases
 * \brief    makes some primitive standard types conforming to Noriko's naming convention
 */
/** @{ */
typedef _Bool     NkBoolean;
typedef void      NkVoid;
typedef size_t    NkSize;
typedef ptrdiff_t NkOffset;
typedef float     NkFloat, NkSingle;
typedef double    NkDouble;
typedef long      NkInt32;
typedef long long NkInt64;

/** @} */


