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
 * \file    env.h
 * \brief   defines the the public API for Noriko's command-line parser
 * 
 * Noriko generally runs by invoking Noriko's runtime component with certain config
 * parameters such as input file, and other runtime options. The command line arguments
 * need to be parsed once at application startup and can then be accessed throughout the
 * lifetime of the application. Synchronization is not necessary since the command-line
 * arguments are constant.
 * 
 * \warning Due to the nature of the command-line arguments storage, it should not be
 *          accessed in functions that can be called after \c main() has returned such as
 *          functions registered in <tt>atexit()</tt>.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/util.h>
#include <include/Noriko/error.h>


/**
 * \brief  parses the command-line arguments
 * \param  [in] argc number of arguments passed
 * \param  [in] argv parameters as list of strings
 * \param  [in] envp an optional <tt>NUL</tt>-terminated array that points to the process'
 *              environment variables
 * \return \c NkErr_Ok on success, non-zero on failure
 * \see    NkEnvCleanup
 * \note   \li If an error occurs during initialization, calling \c NkEnvGetValue()
 *         will not result in undefined behavior.
 * \note   \li Since command-line parameters are parsed independently, an error during
 *         parsing will not abort the entire parsing process but rather the parsing
 *         process of that parameter.
 * \note   \li After this function returns, the user can query and parameter that may
 *         have been passed to the application until \c NkEnvCleanup() is called.
 * \note   \li This function may allocate dynamic memory. Thus, \c NkEnvCleanup()
 *         must be called after the application is done with the command-line arguments.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkEnvParse(
    _In_             int argc,
    _In_reads_(argc) char **argv,
    _In_to_null_     char **envp
);
/**
 * \brief   frees all memory that may be used by the command-line arguments
 * \warning After this function returns, it is no longer safe to query command-line
 *          arguments.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkEnvCleanup(NkVoid);
/**
 * \brief   queries the command-line argument with the given identifier
 * \param   [in] keyStr <tt>NUL</tt>-terminated C-string representing the key, must be
 *               UTF-8-encoded
 * \param   [out] valPtr pointer to an NkVariant variable that will receive the copy of
 *                command-line option's value
 * \return  \c NkErr_Ok on success, non-zero on failure.
 * \note    \li If the function fails, the contents of \c valPtr are indeterminate.
 * \note    \li Calling this function in a single-threaded environment before parsing or
 *          after cleanup is safe.
 * \warning If this function is called from a thread other than the main thread before
 *          \c NkEnvParse() has returned in the main thread, the behavior is not
 *          defined.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkEnvGetValue(_In_z_ char const *keyStr, _Out_ NkVariant *valPtr);


