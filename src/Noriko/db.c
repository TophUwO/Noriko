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
 * \file  db.c
 * \brief implements the generic sqlite3 database handle used by Noriko's engine and
 *        editor
 * 
 * \par Remarks
 *   The interface exposed by this file is not really meant to be used directly by a
 *   user-level module like the layer stack. User-level modules interact with database
 *   handles through kernel-level components like the asset manager. However, it is still
 *   possible for user-level components to directly use the database handles. Such
 *   modules must then do error handling and connection management manually. This may be
 *   necessary for some plug-ins.
 */
#define NK_NAMESPACE "nk::db"


/* Noriko includes */
#include <include/Noriko/db.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/log.h>

#include <include/Noriko/dstruct/string.h>

/* sqlite3 includes */
#include <ext/sqlite3/sqlite3.h>


/** cond INTERNAL */
/**
 * \struct __NkInt_Sqlite3Stmt
 * \brief  represents the internal state of a cacheable sqlite3 statement object
 */
NK_NATIVE typedef struct __NkInt_Sqlite3Stmt {
    NKOM_IMPLEMENTS(NkISqlStatement);

    NkOMRefCount  m_refCount; /**< reference count */
    NkIDatabase  *mp_dbConn;  /**< parent database connection */
    sqlite3_stmt *mp_stmtPtr; /**< pointer to the prepared sqlite3 statement */
} __NkInt_Sqlite3Stmt;

/**
 * \struct __NkInt_Sqlite3DbHandle
 * \brief  represents the internal state of a sqlite3 connection handle
 */
NK_NATIVE typedef struct __NkInt_Sqlite3DbHandle {
    NKOM_IMPLEMENTS(NkIDatabase);

    NkOMRefCount    m_refCount; /**< reference count */
    NkDatabaseMode  m_dbMode;   /**< database access mode */
    sqlite3        *mp_dbConn;  /**< sqlite3 database connection object */
} __NkInt_Sqlite3DbHandle;

/**
 * \struct __NkInt_Sqlite3StmtInit
 * \brief  initialization data structure used for a cacheable statement
 */
NK_NATIVE typedef struct __NkInt_Sqlite3StmtInit {
    char        *mp_sqlStr;     /**< raw UTF-8 SQL string */
    sqlite3     *mp_dbConn;     /**< sqlite3 database connection handle */
    NkIDatabase *mp_nkomDbConn; /**< Noriko database handle (used as parent) */
} __NkInt_Sqlite3StmtInit;


/* Define IIDs and CLSIDs of the module's types. */
// { 6D7C27B3-A5ED-40D5-AA4D-A5C5EF8BF978 }
NKOM_DEFINE_IID(NkISqlStatement, { 0x6d7c27b3, 0xa5ed, 0x40d5, 0xaa4da5c5ef8bf978 });
// { 80507575-98C6-4B81-B1E4-2A824F163709 }
NKOM_DEFINE_IID(NkIDatabase, { 0x80507575, 0x98c6, 0x4b81, 0xb1e42a824f163709 });

// { 29567054-6FD1-49DD-858F-DCE4D01800F8 }
NKOM_DEFINE_CLSID(NkISqlStatement, { 0x29567054, 0x6fd1, 0x49dd, 0x858fdce4d01800f8 });
// { FC7E8354-4B01-4F1F-9715-2CB477208AC0 }
NKOM_DEFINE_CLSID(NkIDatabase, { 0xfc7e8354, 0x4b01, 0x4f1f, 0x97152cb477208ac0 });


/**
 * \brief  checks whether the parameter index is valid
 * \param  [in,out] stPtr pointer to the prepared sqlite3 statement
 * \param  [in] index requested index that is to be verified
 * \return \c NK_TRUE if the index is valid, \c NK_FALSE if it isn't
 */
NK_INTERNAL NkBoolean __NkInt_Sqlite3Stmt_IsValidParamIndex(_Inout_ sqlite3_stmt *stPtr, _In_ NkUint32 index) {
    NK_ASSERT(stPtr != NULL, NkErr_InOutParameter);

    return (int)index < sqlite3_bind_parameter_count(stPtr);
}

/**
 * \brief  sets some database pragmas that are non-persistent for optimization purposes
 *         mostly
 * \param  [in,out] dbConnRef pointer to the open database connection
 * \param  [in] mode desired numeric database access mode identifier
 * \return \c NkErr_Ok on success, non-zero on failure
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_Sqlite3DbHandle_SetPragmas(
    _Inout_ sqlite3 *dbConnRef,
    _In_    NkDatabaseMode mode
) {
    NK_ASSERT(dbConnRef != NULL, NkErr_InOutParameter);
    NK_ASSERT(mode > NkDbMode_Unknown && mode < __NkDbMode_Count__, NkErr_InParameter);

    /**
     * \brief pragmas to set for read-only database connections
     * 
     * When the database is used in a read-only scenario, we can do some optimizations
     * via PRAGMA directives in order to optimize run-time performance if possible. For
     * more information on these directives, please refer to
     * https://www.sqlite.org/pragma.html.
     */
    NK_INTERNAL NkStringView const gl_c_RoPragmaSql = NK_MAKE_STRING_VIEW(
        "PRAGMA encoding     = 'UTF-8';"   /* use UTF-8 encoding */
        "PRAGMA journal_mode = OFF;"       /* prevent creation of journaling file */
        "PRAGMA locking_mode = EXCLUSIVE;" /* take exclusive lock */
        "PRAGMA synchronous  = OFF;"       /* disable synchronous writes */
        "PRAGMA query_only   = ON;"        /* disable UPDATE, DELETE, etc. write operations */
    );
    /**
     * \brief pragmas to use when the database is opened as read-write 
     */
    NK_INTERNAL NkStringView const gl_c_WrPragmaSql = NK_MAKE_STRING_VIEW("PRAGMA encoding = 'UTF-8';");

    int res = SQLITE_OK;
    switch (mode) {
        case NkDbMode_ReadOnly:  res = sqlite3_exec(dbConnRef, gl_c_RoPragmaSql.mp_dataPtr, NULL, NULL, NULL); break;
        case NkDbMode_ReadWrite: res = sqlite3_exec(dbConnRef, gl_c_WrPragmaSql.mp_dataPtr, NULL, NULL, NULL); break;
    }
    return res == SQLITE_OK ? NkErr_Ok : NkErr_SetDatabaseProps;
}

/**
 * \brief  converts a Noriko database access mode identifier to a native sqlite3 open
 *         mode identifier
 * \param  [in] mode Noriko database access mode identifier that is to be mapped
 * \return numeric sqlite3 database open mode
 */
NK_INTERNAL int __NkInt_Sqlite3DbHandle_MapFromDbMode(NkDatabaseMode mode) {
    switch (mode) {
        case NkDbMode_ReadOnly:  return SQLITE_OPEN_READONLY;
        case NkDbMode_ReadWrite: return SQLITE_OPEN_READWRITE;
    }

    return SQLITE_OPEN_READWRITE;
}

/**
 * \brief  converts a native sqlite3 column type to a Noriko variant type
 * \param  [in] nativeTyId native sqlite3 column type ID
 * \return type identifier useable with \c NkVariant
 * \note   Use <tt>sqlite3_column_type()</tt> to retrieve a type ID translatable with
 *         this function.
 */
NK_INTERNAL NkVariantType __NkInt_Sqlite3DbHandle_MapFromSqlite3ColumnType(_In_ int nativeTyId) {
    switch (nativeTyId) {
        case SQLITE_INTEGER: return NkVarTy_Int64;
        case SQLITE_FLOAT:   return NkVarTy_Double;
        case SQLITE_TEXT:    return NkVarTy_StringView;
        case SQLITE_BLOB:    return NkVarTy_BufferView;
        case SQLITE_NULL:    return NkVarTy_None;
    }

    /* If the type isn't know, just return the same type identifier returned for NULL. */
    return NkVarTy_None;
}

/**
 * \brief converts the column results of the current result row into an array of
 *        \c NkVariant instances for further and platform-independent processing
 * \param [in,out] stmtRef reference to the prepared native sqlite3 statement
 * \param [in] count number of columns in the current result row
 * \param [out] colResArr array of \c NkVariant objects that are to be initialized with
 *              the results
 * \note  If a column is empty, an empty/null variant will be returned for that column.
 */
NK_INTERNAL NkVoid __NkInt_Sqlite3DbHandle_PrepareResultArray(
    _Inout_             sqlite3_stmt *stmtRef,
    _In_                int count,
    _Out_writes_(count) NkVariant *colResArr
) {
    NK_ASSERT(stmtRef != NULL, NkErr_InOutParameter);
    NK_ASSERT(count > 0, NkErr_InParameter);
    NK_ASSERT(colResArr != NULL, NkErr_OutParameter);

    for (int i = 0; i < count; i++) {
        /* Determine the column type. This function cannot fail. */
        int           nativeTy = sqlite3_column_type(stmtRef, i);
        NkVariantType varTy    = __NkInt_Sqlite3DbHandle_MapFromSqlite3ColumnType(nativeTy);
        
        /* Get the column data according to the type we determined. */
        switch (varTy) {
            default:
            case NkVarTy_None:   NkVariantSet(&colResArr[i], NkVarTy_None);                             break;
            case NkVarTy_Int64:  NkVariantSet(&colResArr[i], varTy, sqlite3_column_int64(stmtRef, i));  break;
            case NkVarTy_Double: NkVariantSet(&colResArr[i], varTy, sqlite3_column_double(stmtRef, i)); break;
            case NkVarTy_StringView:
                NkVariantSet(&colResArr[i], varTy, &(NkStringView){
                    .mp_dataPtr    = (char *)sqlite3_column_text(stmtRef, i),
                    .m_sizeInBytes = (NkSize)sqlite3_column_bytes(stmtRef, i)
                });

                break;
            case NkVarTy_BufferView:
                NkVariantSet(
                    &colResArr[i],
                    varTy,
                    NK_MAKE_BUFFER_VIEW_PTR(sqlite3_column_blob(stmtRef, i), sqlite3_column_bytes(stmtRef, i))
                );

                break;
        }
    }
}


/**
 * \brief implements <tt>NkISqlStatement::AddRef()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_Sqlite3Stmt_AddRef(_Inout_ NkISqlStatement *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ++((__NkInt_Sqlite3Stmt *)self)->m_refCount;
}

/**
 * \brief implements <tt>NkISqlStatement::Release()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_Sqlite3Stmt_Release(_Inout_ NkISqlStatement *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    __NkInt_Sqlite3Stmt *actSelf = (__NkInt_Sqlite3Stmt *)self;
    if (--actSelf->m_refCount <= 0) {
        /* Destroy db handle. */
        sqlite3_finalize(actSelf->mp_stmtPtr);
        /* Release parent database instance. */
        actSelf->mp_dbConn->VT->Release(actSelf->mp_dbConn);

        /* Destroy instance. */
        NkPoolFree((NkVoid *)self);
        return 0;
    }

    return actSelf->m_refCount;
}

/**
 * \brief implements <tt>NkISqlStatement::QueryInterface()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Sqlite3Stmt_QueryInterface(
    _Inout_  NkISqlStatement *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    NK_INTERNAL NkOMImplementationInfo const gl_c_ImplInfos[] = {
        { NKOM_IIDOF(NkIBase)          },
        { NKOM_IIDOF(NkIInitializable) },
        { NKOM_IIDOF(NkISqlStatement)  },
        { NULL                         }
    };
    if (NkOMQueryImplementationIndex(gl_c_ImplInfos, iId)) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_Sqlite3Stmt_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    return NkErr_InterfaceNotImpl;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Sqlite3Stmt_Initialize(
    _Inout_     NkISqlStatement *self,
    _Inout_opt_ NkVoid *initParam
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(initParam != NULL, NkErr_InParameter);
    
    /* Get pointer to internal state. */
    __NkInt_Sqlite3Stmt *actSelf = (__NkInt_Sqlite3Stmt *)self;
    /* Get pointer to SQL source code parameter. */
    __NkInt_Sqlite3StmtInit const *initStruct = (__NkInt_Sqlite3StmtInit const *)initParam;

    /* Compile the first statement; any statements after the first are ignored. */
    int res = sqlite3_prepare_v2(
        initStruct->mp_dbConn,
        initStruct->mp_sqlStr,
        -1,
        &actSelf->mp_stmtPtr,
        NULL
    );
    if (res != SQLITE_OK)
        return NkErr_CompileSqlStatement;

    /* Set the other attributes of the statement object. */
    initStruct->mp_nkomDbConn->VT->AddRef(initStruct->mp_nkomDbConn);
    actSelf->mp_dbConn = initStruct->mp_nkomDbConn;

    /* All good. */
    return NkErr_Ok;
}

/**
 * \brief implements <tt>NkISqlStatement::Bind()</tt> 
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_Sqlite3Stmt_Bind(
    _Inout_ NkISqlStatement *self,
    _In_    NkUint32 index,
    _In_    NkVariant const *varRef
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(varRef != NULL, NkErr_InParameter);

    /* Get the pointer to the current internal statement. */
    __NkInt_Sqlite3Stmt *stmtRef = (__NkInt_Sqlite3Stmt *)self;
    /* Catch out-of-range errors. */
    if (!__NkInt_Sqlite3Stmt_IsValidParamIndex(stmtRef->mp_stmtPtr, index)) {
        NK_LOG_ERROR("Parameter index %u out of range. Cannot bind.", index);

        return;
    }

    /* Retrieve the type and value. */
    NkVariantType varType;
    union {
        NkInt64      m_i64Val;
        NkDouble     m_dblVal;
        NkStringView m_strVal;
        NkBufferView m_bufVal;
        NkUuid       m_uuidVal;
    } varVal = { 0 };
    NkVariantGet(varRef, &varType, (NkVoid *)&varVal);
    /* Depending on the value, bind the corresponding value to the indexed parameter. */
    switch (varType) {
        case NkVarTy_Int8:
        case NkVarTy_Int16:
        case NkVarTy_Int32:
        case NkVarTy_Int64:
        case NkVarTy_Uint8:
        case NkVarTy_Uint16:
        case NkVarTy_Uint32:
        case NkVarTy_Uint64:
        case NkVarTy_Boolean:
            /*
             * Whatever the integer type, the value is saved as a 64-bit signed integer.
             * It's up to the caller to cast the result back when it is queried.
             */
            sqlite3_bind_int64(stmtRef->mp_stmtPtr, (int)index, varVal.m_i64Val);
            
            break;
        case NkVarTy_StringView:
            sqlite3_bind_text64(
                stmtRef->mp_stmtPtr,
                (int)index,
                varVal.m_strVal.mp_dataPtr,
                varVal.m_strVal.m_sizeInBytes,
                SQLITE_TRANSIENT,
                SQLITE_UTF8
            );

            break;
        case NkVarTy_Double:
        case NkVarTy_Float:
            /*
             * No need to cast between float and doubles here since variants can only
             * hold doubles, even if the internal type is NkVarTy_Float.
             */
            sqlite3_bind_double(stmtRef->mp_stmtPtr, (int)index, varVal.m_dblVal);

            break;
        case NkVarTy_Uuid:
        case NkVarTy_BufferView:
            /*
             * When binding memory, we can only differentiate between UUIDs and raw
             * buffers as a general case. We are being explicit here so that nothing
             * breaks when the ABI of the corresponding types change.
             */
            sqlite3_bind_blob64(
                stmtRef->mp_stmtPtr,
                (int)index,
                varType == NkVarTy_Uuid
                    ? (NkVoid const *)&varVal.m_uuidVal
                    : (NkVoid const *)varVal.m_bufVal.mp_dataPtr,
                varType == NkVarTy_Uuid 
                    ? sizeof varVal.m_uuidVal
                    : varVal.m_bufVal.m_sizeInBytes,
                SQLITE_TRANSIENT
            );

            break;
        case NkVarTy_None:
            /* If the variant is empty, we simply unbind the value, aka. bind NULL. */
            self->VT->Unbind(self, index);

            break;
        default:
            /* Unsupported type. */
            NK_LOG_ERROR(
                "Cannot bind value of type \"%s\" (%i). Unsupported type.",
                NkVariantQueryTypeStr(varType)->mp_dataPtr,
                varType
            );
    }
}

/**
 * \brief implements <tt>NkISqlStatement::Unbind()</tt> 
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_Sqlite3Stmt_Unbind(_Inout_ NkISqlStatement *self, _In_ NkUint32 index) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Get the pointer to the current internal statement. */
    __NkInt_Sqlite3Stmt *stmtRef = (__NkInt_Sqlite3Stmt *)self;
    /* Catch out-of-range errors. */
    if (!__NkInt_Sqlite3Stmt_IsValidParamIndex(stmtRef->mp_stmtPtr, index)) {
        NK_LOG_ERROR("Parameter index %u out of range. Cannot bind.", index);

        return;
    }

    /* Bind NULL for the given index. */
    sqlite3_bind_null(stmtRef->mp_stmtPtr, (int)index);
}


/**
 * \brief defines the VTable for the default implementation of the \c NkISqlStatement
 *        class 
 */
NKOM_DEFINE_VTABLE(NkISqlStatement) {
    .AddRef            = &__NkInt_Sqlite3Stmt_AddRef,
    .Release           = &__NkInt_Sqlite3Stmt_Release,
    .QueryInterface    = &__NkInt_Sqlite3Stmt_QueryInterface,
    .Initialize        = &__NkInt_Sqlite3Stmt_Initialize,
    .Bind              = &__NkInt_Sqlite3Stmt_Bind,
    .Unbind            = &__NkInt_Sqlite3Stmt_Unbind
};


/**
 * \brief implements <tt>NkIDatabase::AddRef()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_Sqlite3DbHandle_AddRef(_Inout_ NkIDatabase *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ++((__NkInt_Sqlite3DbHandle *)self)->m_refCount;
}

/**
 * \brief implements <tt>NkIDatabase::Release()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_Sqlite3DbHandle_Release(_Inout_ NkIDatabase *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    __NkInt_Sqlite3DbHandle *actSelf = (__NkInt_Sqlite3DbHandle *)self;

    if (--actSelf->m_refCount <= 0) {
        /* Close the database connection if it's still open. */
        self->VT->Close(self);

        NkPoolFree((NkVoid *)self);
        return 0;
    }

    return actSelf->m_refCount;
}

/**
 * \brief implements <tt>NkIDatabase::QueryInterface()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Sqlite3DbHandle_QueryInterface(
    _Inout_  NkIDatabase *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    if (NkUuidIsEqual(iId, NKOM_IIDOF(NkIBase)) || NkUuidIsEqual(iId, NKOM_IIDOF(NkIDatabase))) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_Sqlite3DbHandle_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    return NkErr_InterfaceNotImpl;
}

/**
 * \brief implements <tt>NkIDatabase::Open()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Sqlite3DbHandle_Open(
    _Inout_       NkIDatabase *self,
    _In_z_ _Utf8_ char const *dbPath,
    _In_          NkDatabaseMode mode
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(mode > NkDbMode_Unknown && mode < __NkDbMode_Count__, NkErr_InParameter);

    __NkInt_Sqlite3DbHandle *actSelf = (__NkInt_Sqlite3DbHandle *)self;

    /* If a database is already open, cannot open another one. */
    if (actSelf->mp_dbConn != NULL)
        return NkErr_ObjectState;
    
    /* Open the database connection. */
    int res = sqlite3_open_v2(dbPath, &actSelf->mp_dbConn, __NkInt_Sqlite3DbHandle_MapFromDbMode(mode), NULL);
    if (res != SQLITE_OK) {
        /*
         * Even if an error occurs, the function will still return a valid database
         * handle. In such a case, we must still free it explicitly.
         * A NULL-pointer is a no-op when passed to sqlite3_close().
         */
        sqlite3_close(actSelf->mp_dbConn);

        actSelf->mp_dbConn = NULL;
        return NkErr_DatabaseOpen;
    }
    actSelf->m_dbMode = mode;

    /*
     * Connection was successfully opened. Set optimization pragmas according to the
     * connection mode, if needed.
     */
    NkErrorCode errCode = __NkInt_Sqlite3DbHandle_SetPragmas(actSelf->mp_dbConn, mode);
    if (errCode != NkErr_Ok) {
        /* Could not set some pragmas. Just close DB in such a case. */
        sqlite3_close(actSelf->mp_dbConn);

        actSelf->mp_dbConn = NULL;
    }
    return errCode;
}

/**
 * \brief implements <tt>NkIDatabase::Close()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Sqlite3DbHandle_Close(_Inout_ NkIDatabase *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Retrieve and close the database connection. This may fail. */
    sqlite3 **dbHdPtr;
    int res = sqlite3_close(*(dbHdPtr = &((__NkInt_Sqlite3DbHandle *)self)->mp_dbConn));

    if (res == SQLITE_OK)
        *dbHdPtr = NULL;
    return res == SQLITE_OK ? NkErr_Ok : NkErr_DatabaseClose;
}

/**
 * \brief implements <tt>NkIDatabase::CreateStatement()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Sqlite3DbHandle_CreateStatement(
    _Inout_       NkIDatabase *self,
    _In_z_ _Utf8_ char const *sqlStr,
    _Init_ptr_    NkISqlStatement **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(sqlStr != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    /*
     * Prepare the statement initialization structure. This assumes that all parameters
     * are valid.
     */
    __NkInt_Sqlite3StmtInit stmtInit = {
        .mp_sqlStr     = (char *)sqlStr,
        .mp_dbConn     = ((__NkInt_Sqlite3DbHandle *)self)->mp_dbConn,
        .mp_nkomDbConn = self
    };

    /* Create and prepare the statement. */
    return NkOMCreateInstance(
        NKOM_CLSIDOF(NkISqlStatement),
        NULL,
        NKOM_IIDOF(NkISqlStatement),
        (NkVoid *)&stmtInit,
        (NkIBase **)resPtr
    );
}

/**
 * \brief implements <tt>NkIDatabase::Execute()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Sqlite3DbHandle_Execute(
    _Inout_     NkIDatabase *self,
    _In_        NkISqlStatement *stmtRef,
    _In_opt_    NkDatabaseQueryIterFn fnResIter,
    _Inout_opt_ NkVoid *extraCxtPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(stmtRef != NULL, NkErr_InParameter);

    __NkInt_Sqlite3DbHandle *actSelf = (__NkInt_Sqlite3DbHandle *)self;

    /* Verify that the given handle actually refers to an open database connection. */
    if (actSelf->mp_dbConn == NULL)
        return NkErr_ObjectState;

    /* Iterate over all result rows, invoking the callback once per row. */
    NkErrorCode eCode = NkErr_Ok;
    NkVariant *resArr = NULL;
    sqlite3_stmt *actStmt = ((__NkInt_Sqlite3Stmt *)stmtRef)->mp_stmtPtr;
    while (sqlite3_step(actStmt) == SQLITE_ROW) {
        /* Allocate the column array the first time a row is processed. */
        if (resArr == NULL) {
            eCode = NkPoolAlloc(NK_MAKE_ALLOCATION_CONTEXT(), sizeof *resArr, sqlite3_column_count(actStmt), &resArr);

            if (eCode != NkErr_Ok)
                goto lbl_ONFIN;
        }
        /* Fill the result array dynamically. */
        __NkInt_Sqlite3DbHandle_PrepareResultArray(actStmt, sqlite3_column_count(actStmt), resArr);

        /* Run the callback on the result. */
        if (fnResIter != NULL) {
            eCode = (*fnResIter)((NkUint32)sqlite3_column_count(actStmt), (NkVariant const *)resArr, extraCxtPtr);
            
            /* If the return value is not NkErr_Ok, then we stop iterating. */
            if (eCode != NkErr_Ok) {
                eCode = eCode == NkErr_ManuallyAborted ? NkErr_Ok : eCode;

                goto lbl_ONFIN;
            }
        }
    }

lbl_ONFIN:
    /*
     * Destroy the memory used for the result array. If this is NULL, NkPoolFree() is a
     * no-op.
     */
    NkPoolFree((NkVoid *)resArr);
    /* Reset the statement so it can be reused. */
    sqlite3_reset(actStmt);

    return eCode;
}


/**
 * \brief static NkIDatabase VTable instance 
 */
NKOM_DEFINE_VTABLE(NkIDatabase) {
    .AddRef          = &__NkInt_Sqlite3DbHandle_AddRef,
    .Release         = &__NkInt_Sqlite3DbHandle_Release,
    .QueryInterface  = &__NkInt_Sqlite3DbHandle_QueryInterface,
    .Open            = &__NkInt_Sqlite3DbHandle_Open,
    .Close           = &__NkInt_Sqlite3DbHandle_Close,
    .CreateStatement = &__NkInt_Sqlite3DbHandle_CreateStatement,
    .Execute         = &__NkInt_Sqlite3DbHandle_Execute
};


/**
 * \brief implements <tt>NkIClassFactory::AddRef()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_Sqlite3HdFactory_AddRef(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub because static object. */
    return 1;
}

/**
 * \brief implements <tt>NkIClassFactory::Release()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_Sqlite3HdFactory_Release(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Same as in __NkInt_Sqlite3HdFactory_AddRef(). */
    return 1;
}

/**
 * \brief implements <tt>NkIClassFactory::QueryInterface()</tt>
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Sqlite3HdFactory_QueryInterface(
    _Inout_  NkIClassFactory *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    if (NkUuidIsEqual(iId, NKOM_IIDOF(NkIBase)) || NkUuidIsEqual(iId, NKOM_IIDOF(NkIClassFactory))) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_Sqlite3HdFactory_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    return NkErr_InterfaceNotImpl;
}

/**
 * \brief implements <tt>NkIClassFactory::QueryInstantiableClasses()</tt>
 */
NK_INTERNAL NkUuid const **NK_CALL __NkInt_Sqlite3HdFactory_QueryInstantiableClasses(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);
    
    /**
     * \brief lists all classes instantiable by the current factory
     */
    NK_INTERNAL NkUuid const *gl_c_InstClasses[] = {
        NKOM_CLSIDOF(NkISqlStatement),
        NKOM_CLSIDOF(NkIDatabase),
        NULL
    };

    return gl_c_InstClasses;
}

/**
 * \brief implements <tt>NkIClassFactory::CreateInstance()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Sqlite3HdFactory_CreateInstance(
    _Inout_     NkIClassFactory *self,
    _In_        NkUuid const *clsId,
    _Inout_opt_ NkIBase *ctrlInst,
    _Outptr_    NkIBase **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(clsId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);
    NK_UNREFERENCED_PARAMETER(ctrlInst);

    /**
     * \brief list implementation infos of the classes that can be instantiated by the
     *        current factory instance 
     */
NK_DISABLE_WARNING(NK_WARN_DIFFERENT_CONST_QUALIFIERS,
    NK_INTERNAL NkOMImplementationInfo const gl_c_ImplInfos[] = {
        { NKOM_CLSIDOF(NkISqlStatement), sizeof(__NkInt_Sqlite3Stmt),     NK_FALSE, &NKOM_VTABLEOF(NkISqlStatement) },
        { NKOM_CLSIDOF(NkIDatabase),     sizeof(__NkInt_Sqlite3DbHandle), NK_FALSE, &NKOM_VTABLEOF(NkIDatabase)     },
        { NULL                                                                                                      }
    };
);
    NkSize implIndex;
    if ((implIndex = NkOMQueryImplementationIndex(gl_c_ImplInfos, clsId)) == SIZE_MAX)
        return NkErr_UnknownClass;
    /* Get implementation entry. */
    NkOMImplementationInfo const *iInfo = &gl_c_ImplInfos[implIndex];

    /* Instantiate the class. */
    NkErrorCode eCode = NkPoolAlloc(NK_MAKE_ALLOCATION_CONTEXT(), (NkUint32)iInfo->m_structSize, 1, (NkVoid **)resPtr);
    if (eCode != NkErr_Ok)
        return eCode;
    /* Initialize instance memory. */
    memset((NkVoid *)*resPtr, 0, iInfo->m_structSize);
    (*resPtr)->VT = (struct __NkIBase_VTable__ *)iInfo->mp_vtabPtr;

    /* All went well. */
    (*resPtr)->VT->AddRef(*resPtr);
    return NkErr_Ok;
}


/**
 * \brief represents the actual instance of the sqlite3 handle factory which is void of
 *        any internal state (see <tt>gl_c_RendererFactory</tt>) 
 */
NK_INTERNAL NkIClassFactory const gl_c_Sqlite3HdFactory = {
    .VT = &(struct __NkIClassFactory_VTable__) {
        .AddRef                   = &__NkInt_Sqlite3HdFactory_AddRef,
        .Release                  = &__NkInt_Sqlite3HdFactory_Release,
        .QueryInterface           = &__NkInt_Sqlite3HdFactory_QueryInterface,
        .QueryInstantiableClasses = &__NkInt_Sqlite3HdFactory_QueryInstantiableClasses,
        .CreateInstance           = &__NkInt_Sqlite3HdFactory_CreateInstance
    }
};
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkDatabaseStartup(NkVoid) {
    NK_LOG_INFO("startup: sqlite3 database services");

    return NkOMInstallClassFactory((NkIClassFactory *)&gl_c_Sqlite3HdFactory);
}

_Return_ok_ NkErrorCode NK_CALL NkDatabaseShutdown(NkVoid) {
    NK_LOG_INFO("shutdown: sqlite3 database services");

    return NkOMUninstallClassFactory((NkIClassFactory *)&gl_c_Sqlite3HdFactory);
}


#undef NK_NAMESPACE


