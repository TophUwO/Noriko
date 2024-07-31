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
 * \file    cmdline.h
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
 * \enum  NkCmdlineValueType
 * \brief represents the supported value type for the command-line arguments
 */
NK_NATIVE typedef _In_range_(0, __NkCmdVTy_Count__ - 1) enum NkCmdlineValueType {
    NkCmdVTy_Any,      /**< any/unknown type */

    NkCmdVTy_Bool,     /**< boolean type */
    NkCmdVTy_Int,      /**< (signed) integer type (always 64-bit) */
    NkCmdVTy_Uint,     /**< (unsigned) integer type (always 64-bit) */
    NkCmdVTy_Float,    /**< floating-point number (64-bit) */
    NkCmdVTy_String,   /**< string type (UTF-8) */
    NkCmdVTy_List,     /**< list type */

    __NkCmdVTy_Count__ /**< *only used internally* */
} NkCmdlineValueType;


/**
 * \brief  parses the command-line arguments
 * \param  [in] argc number of arguments passed
 * \param  [in] argv parameters as list of strings
 * \return \c NkErr_Ok on success, non-zero on failure
 * \see    NkCmdlineCleanup
 * \note   \li After this function returns, the user can query and parameter that may
 *         have been passed to the application until \c NkCmdlineCleanup() is called.
 * \note   \li This function may allocate dynamic memory. Thus, \c NkCmdlineCleanup()
 *         must be called after the application is done with the command-line arguments.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkCmdlineParse(_In_ int argc, _In_reads_(argc) char **argv);
/**
 * \brief   frees all memory that may be used by the command-line arguments
 * \warning After this function returns, it is no longer safe to query command-line
 *          arguments.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkCmdlineCleanup(NkVoid);
/**
 * \brief  queries the command-line argument with the given identifier
 * \param  [in] keyStr <tt>NUL</tt>-terminated C-string representing the key, must be
 *              UTF-8-encoded
 * \param  [in,out] expAndOutType expected type of <tt>keyStr</tt>'s value
 * \param  [out] outPtr pointer to a variable of the appropriate type that will receive
 *               the value
 * \return \c NkErr_Ok on success, non-zero on failure.
 * \note   \li If the function fails, the contents of \c outPtr are indeterminate.
 * \note   \li If \c expAndOutType is any other type than <tt>NkCmdVTy_Any</tt>, the
 *         function will check this type ID against the actual type of <tt>keyStr</tt>'s
 *         value. If the types do not match, the function fails. Otherwise, the value is
 *         written to <tt>outPtr</tt>. If \c expAndOutType is <tt>NkCmdVTy_Any</tt>, no
 *         type checking is carried out, \c outPtr is populated with <tt>keyStr</tt>'s
 *         value and \c expAndOutType is overwritten with the value's actual type. The
 *         returned type should be used to properly examine the data in <tt>outPtr</tt>.
 * \note   \li If the value that is to be written to \c outPtr is a \e primitive type,
 *         \c outPtr is interpreted as a pointer to the corresponding primitive type. If
 *         the value is a \e composite type (i.e., a list), \c outPtr is interpreted as a
 *         pointer to a pointer to the corresponding type. **Supplying an inadequately
 *         indirected pointer to this function results in undefined behavior.**
 * \note   \li If the returned values are pointers themselves, then they point to memory
 *         that is not supposed to be written to. **Writing to this memory in any way,
 *         also through means appropriate (such as using NkVector's functions for list
 *         values), results in undefined behavior.**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkCmdlineGetValue(
    _In_z_      char const *keyStr,
    _Inout_opt_ NkCmdlineValueType *expAndOutType,
    _Out_opt_   NkVoid *outPtr
);


