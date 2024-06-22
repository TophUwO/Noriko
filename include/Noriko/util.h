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
 * \brief utility functions and data-structures used by multiple of Noriko's components
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>


/**
 * \brief represents a compile-time constant string view
 * 
 * This utility structure allows for extra information about the string to be stored
 * directly alongside it, making simple processing simpler and less error-prone.
 */
NK_NATIVE typedef struct NkStringView {
    char   *mp_dataPtr;    /**< pointer to static string buffer */
    size_t  m_sizeInBytes; /**< size of buffer, in bytes */
} NkStringView;

/**
 * \def   NK_MAKE_STRING_VIEW(str)
 * \brief generates a compile-time string view to a static string
 * \param str string literal to create string view for
 */
#define NK_MAKE_STRING_VIEW(str) { .mp_dataPtr = str, .m_sizeInBytes = sizeof str }


