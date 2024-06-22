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


/**
 * \enum  NkErrorCode
 * \brief numeric error code definitions
 */
typedef _In_range_(NkErr_Ok, __NkErr_Count__ - 1) enum NkErrorCode {
    NkErr_Ok,             /**< no error */
    NkErr_Unknown,        /**< unknown error condition */
    
    NkErr_NotImplemented, /**< feature not implemented */
    NkErr_InParameter,    /**< errornous input parameter */
    NkErr_OutParameter,   /**< errornous output parameter */
    NkErr_InOutParameter, /**< errornous input/output parameter */

    __NkErr_Count__       /**< used internally */
} NkErrorCode;


/**
 * \brief  retrieves the textual representation of the provided integral error code
 * \param  [in] ecode integer error code to get string representation of
 * \return textual representation of error code, as UTF-8 C-string, or NULL if the error
 *         code representation could not be retrieved
 * \note   The return value of this function is a pointer to static read-only memory.
 * \note   The resulting string value is generally the identifier of the enum literal
 *         associated with the provided integer error code.
 */
NK_NATIVE NK_API _Return_ok_ char const *NK_CALL NkGetErrorCodeString(_In_ _Ecode_range_ NkErrorCode const ecode);
/**
 * \brief  retrieves a brief textual description of the provided integral error code
 * \param  [in] ecode integer error code to get the associated error description of
 * \return textual description of error code, as UTF-8 C-string, or NULL if the error
 *         description could not be retrieved
 * \note   The return value of this function is a pointer to static read-only memory.
 */
NK_NATIVE NK_API _Return_ok_ char const *NK_CALL NkGetErrorCodeDesc(_In_ _Ecode_range_ NkErrorCode const ecode);


