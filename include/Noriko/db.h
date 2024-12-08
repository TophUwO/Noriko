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
 * \file  db.h
 * \brief presents an interface for an abstract database connection handle
 * 
 * \par Remarks
 *   The interface exposed by this file is not really meant to be used directly by a
 *   user-level module like the layer stack. User-level modules interact with database
 *   handles through kernel-level components like the asset manager. However, it is still
 *   possible for user-level components to directly use the database handles. Such
 *   modules must then do error handling and connection management manually. This may be
 *   necessary for some plug-ins.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>
#include <include/Noriko/error.h>
#include <include/Noriko/nkom.h>


/**
 * \enum  NkDatabaseMode
 * \brief lists various database connection modes
 * \bug   Certain flag combinations don't work if __NkDbMode_Count__ is just one larger
 *        than the highest bitflag. Maybe fix by using the next larger power of two.
 */
NK_NATIVE typedef enum NkDatabaseMode {
    NkDbMode_Unknown   = 0,      /**< unknown database mode */

    NkDbMode_ReadOnly  = 1 << 0, /**< read-only connection */
    NkDbMode_ReadWrite = 1 << 1, /**< connection for reading and writing */
    NkDbMode_Create    = 1 << 2, /**< create the database if it does not exist */

    __NkDbMode_Count__ = 1 << 3  /**< *only used internally* */
} NkDatabaseMode;


/**
 * \brief  is executed once for each result row, allowing to act upon the result
 * \param  [in] colCount number of columns (= number of elements in \c colResArr and
 *              <tt>colNamesArr</tt>)
 * \param  [in] colResArr array of \c NkVariant structs that hold the resulting column
 *              value in the current row, exactly \c colCount elements
 * \param  [in,out] extraCxtPtr pointer to a user-provided data structure that can be
 *                  used to modify some external state according to the row data, such as
 *                  accumulating it in a \c NkVector object
 * \return \c NkErr_Ok on success and if you wish to continue iterating over the result
 *         rows, \c NkErr_ManuallyAborted if there was no error but you want to simply
 *         cancel iterating over the results, or non-zero if there was an error
 * 
 * \par Remarks
 *   If the return value is non-zero, no more result rows will be iterated. This can be
 *   useful when looking for a specific value or when a user-provided accumulation buffer
 *   has reached its limit.
 */
typedef NkErrorCode (NK_CALL *NkDatabaseQueryIterFn)(
    _In_                 NkUint32 colCount,
    _In_reads_(colCount) NkVariant const *colResArr,
    _Inout_opt_          NkVoid *extraCxtPtr
);


/**
 * \interface NkISqlStatement 
 */
NKOM_DECLARE_INTERFACE(NkISqlStatement) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(
        _Inout_  NkISqlStatement *self,
        _In_     NkUuid const *iId,
        _Outptr_ NkVoid **resPtr
    );
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkISqlStatement *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkISqlStatement *self);

    /**
     */
    NkErrorCode (NK_CALL *Initialize)(_Inout_ NkISqlStatement *self, _Inout_opt_ NkVoid *initParam);

    /**
     */
    NkVoid (NK_CALL *Bind)(_Inout_ NkISqlStatement *self, _In_ NkUint32 index, _In_ NkVariant const *varRef);
    /**
     */
    NkVoid (NK_CALL *Unbind)(_Inout_ NkISqlStatement *self, _In_ NkUint32 index);
};

/**
 * \interface NkIDatabase 
 */
NKOM_DECLARE_INTERFACE(NkIDatabase) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkIDatabase *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIDatabase *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIDatabase *self);

    /**
     */
    NkErrorCode (NK_CALL *Create)(
        _Inout_           NkIDatabase *self,
        _In_opt_z_ _Utf8_ char const *schemaStr,
        _In_z_ _Utf8_     char const *dbPath,
        _In_              NkDatabaseMode mode
    );
    /**
     */
    NkErrorCode (NK_CALL *Open)(_Inout_ NkIDatabase *self, _In_z_ _Utf8_ char const *dbPath, _In_ NkDatabaseMode mode);
    /**
     */
    NkErrorCode (NK_CALL *Close)(_Inout_ NkIDatabase *self);
    /**
     */
    NkErrorCode (NK_CALL *CreateStatement)(
        _Inout_       NkIDatabase *self,
        _In_z_ _Utf8_ char const *sqlStr,
        _Init_ptr_    NkISqlStatement **resPtr
    );
    /**
     */
    NkErrorCode (NK_CALL *Execute)(
        _Inout_     NkIDatabase *self,
        _Inout_     NkISqlStatement *stmtRef,
        _In_opt_    NkDatabaseQueryIterFn fnResIter,
        _Inout_opt_ NkVoid *extraCxtPtr
    );
    /**
     */
    NkErrorCode (NK_CALL *ExecuteInline)(
        _Inout_       NkIDatabase *self,
        _In_z_ _Utf8_ char const *sqlStr,
        _In_opt_      NkDatabaseQueryIterFn fnResIter,
        _Inout_opt_   NkVoid *extraCxtPtr
    );
};


