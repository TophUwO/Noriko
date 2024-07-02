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
 * \file  util.h
 * \brief auxiliary utility functions and **static** data-structures used by multiple of
 *        Noriko's components
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>


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
 * \def   NK_MIN(x, y)
 * \brief expands to the maximum of the numeric values \c x and \c y
 * \param x first value
 * \param y second value
 */
#define NK_MIN(x, y)               (((x) > (y)) ? (y) : (x))
/**
 * \def   NK_MAX(x, y)
 * \brief expands to the minimum of the numeric values \c x and \c y
 * \param x first value
 * \param y second value
 */
#define NK_MAX(x, y)               (((x) > (y)) ? (x) : (y))

/**
 * \def   NK_ARRAYSIZE(arr)
 * \brief calculates the size in elements of a static compile-time array
 * \param arr array to calculate size of
 */
#define NK_ARRAYSIZE(arr)          (NkSize)(sizeof (arr) / (sizeof *(arr)))


/**
 * \def   NK_OFFSETOF(st, m)
 * \brief calculates the offset (in bytes) of the given member relative to the address of
 *        the first byte of its parent structure
 * \param st structure type name
 * \param m name of member variable of which the offset is to be calculated
 * \see   https://en.wikipedia.org/wiki/Offsetof
 */
#define NK_OFFSETOF(st, m)         ((NkSize)((char *)&((st *)0)->m - (char *)0))
/**
 * \def   NK_SIZEOF(st, m)
 * \brief returns the size in bytes of the member \c m of data-structure \c st, i.e.
 *        <tt>sizeof st.m</tt>
 * \param st type name of the data-structure
 * \param m identifier of the member
 */
#define NK_SIZEOF(st, m)           ((NkSize)(sizeof ((st *)0)->m))


/**
 * \def   NK_TRUE
 * \brief use *NK_TRUE* instead of just plain *1*
 */
#define NK_TRUE  ((NkBoolean)(1))
/**
 * \def   NK_FALSE
 * \brief use *NK_FALSE* instead of just plain *0*
 */
#define NK_FALSE ((NkBoolean)(0))


/**
 * \def   NK_UNREFERENCED_PARAMETER(p)
 * \brief suppress "unreferenced parameter 'x'" warnings
 * \param p name of the parameter
 */
#define NK_UNREFERENCED_PARAMETER(p) ((void)(p))


/**
 * \brief represents a compile-time constant string view
 * 
 * This utility structure allows for extra information about the string to be stored
 * directly alongside it, making simple processing simpler and less error-prone.
 */
NK_NATIVE typedef struct NkStringView {
    char   *mp_dataPtr;    /**< pointer to static string buffer */
    NkSize  m_sizeInBytes; /**< size of buffer, in bytes */
} NkStringView;

/**
 * \def   NK_MAKE_STRING_VIEW(str)
 * \brief generates a compile-time string view to a static string
 * \param str string literal to create string view for
 */
#define NK_MAKE_STRING_VIEW(str) { .mp_dataPtr = str, .m_sizeInBytes = sizeof str }


