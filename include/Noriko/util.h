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
 * \def   NK_ERROR_CODE
 * \brief defines the name of the function-local error code variable
 */
#define NK_ERROR_CODE          errorCode
/**
 * \def   NK_DEFINE_ERROR_CODE
 * \brief defines the function-local error code variable
 */
#define NK_DEFINE_ERROR_CODE   NkErrorCode NK_ERROR_CODE = NkErr_Ok;
/**
 * \def   NK_SET_ERROR_CODE(v)
 * \brief updates the value of the function-local error code variable
 * \param v new numeric error code
 */
#define NK_SET_ERROR_CODE(v)   NK_ERROR_CODE = v;


/**
 * \def   NK_ON_ERROR
 * \brief defines the common error handling section label used by Noriko
 */
#define NK_ON_ERROR            lbl_NKONERROR
/**
 * \def   NK_GOTO_ERROR()
 * \brief jumps to the common error handling section
 */
#define NK_GOTO_ERROR()        goto NK_ON_ERROR;
/**
 * \def   NK_BASIC_ASSERT(e, c)
 * \brief checks whether *e* evaluates to *true*
 * 
 * If *e* does evaluate to *true*, this does nothing, else it sets the function-local
 * error code variable and jumps to the common error handling section.
 * 
 * \param e expression to evaluate
 * \param c error code that will be set if *e* evaluates to *false*
 */
#define NK_BASIC_ASSERT(e, c)  if (!(e)) { NK_SET_ERROR_CODE((c)); NK_GOTO_ERROR(); }
/**
 * \def   NK_FAIL_WITH(c)
 * \brief sets the function-local error code to *c* and jumps to common error handling
 *        section
 * \param c error-code to set local error variable to
 */
#define NK_FAIL_WITH(c)        NK_BASIC_ASSERT(0, c)


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


