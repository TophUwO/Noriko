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

/* Use SALv2 on MSVC platform. */
/** \cond */
#if (__has_include(<sal.h>))
    #include <sal.h>

    #define _Return_ok_             _Success_(return == NkErr_Ok)
    #define _Ecode_range_           _In_range_(NkErr_Ok, __NkErr_Count__ - 1)
#else
    #define _In_
    #define _Out_
    #define _Inout_
    #define _Field_z_
    #define _Struct_size_bytes_(n)
    #define _Return_ok_
    #define _Success_(expr)
    #define _In_range_(lo, hi)
    #define _Ecode_range_
#endif
/** \endcond */


/**
 * \def   NK_ESC(...)
 * \brief escapes all parameters passed into this macro at compile-time
 * \note  The result is encoded as an UTF-8 string.
 */
#define NK_ESC(...) u8###__VA_ARGS__
/**
 * \def   NK_MESC(x)
 * \brief expands a macro parameter and escapes its expanded value
 * \param x macro expression to escape
 * \note  The result is encoded as an UTF-8 string.
 */
#define NK_MESC(x)  NK_ESC(x)

/**
 * \def   NK_INRANGE_INCL(x, lo, hi)
 * \brief checks whether lo <= x <= hi is *true*
 * \param x numeric value to compare against *lo* and *hi*
 * \param lo lower bound of the comparison
 * \param hi higher bound of the comparison
 */
#define NK_INRANGE_INCL(x, lo, hi) (_Bool)((((hi) - (x)) * ((x) - (lo))) >= 0)
/**
 * \def   NK_INRANGE_EXCL(x, lo, hi)
 * \brief checks whether lo < x < hi is *true*
 * \param x numeric value to compare against *lo* and *hi*
 * \param lo lower bound of the comparison
 * \param hi higher bound of the comparison
 */
#define NK_INRANGE_EXCL(x, lo, hi) (_Bool)(NK_INRANGE_INCL(x, (lo) + 1, (hi) - 1))

/**
 * \def   NK_ARRAYSIZE(arr)
 * \brief calculates the size in elements of a static compile-time array
 * \param arr array to calculate size of
 */
#define NK_ARRAYSIZE(arr)          (size_t)(sizeof (arr) / (sizeof *(arr)))


