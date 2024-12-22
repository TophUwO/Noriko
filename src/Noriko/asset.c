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
 * \file  asset.h
 * \brief represents the public API for one of Noriko's central components, the asset
 *        manager, and asset types to be consumed by other components such as the
 *        renderer
 * \note  The public methods exposed by this module are thread-safe.
 */
#define NK_NAMESPACE "nk::asset"


/* Noriko includes */
#include <include/Noriko/asset.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/log.h>
#include <include/Noriko/db.h>
#include <include/Noriko/noriko.h>

#include <include/Noriko/dstruct/htable.h>


/** \cond INTERNAL */
/**
 */
NK_NATIVE typedef struct __NkInt_AssetManager {
    NKOM_IMPLEMENTS(NkIAssetManager);

    NkHashtable     *mp_assetCache;     /**< asset cache, used for querying */
    NkIDatabase     *mp_dbConn;         /**< database connection handle */
    NkString         m_dbFileName;      /**< path to the database file */
    NkISqlStatement *mp_queryAssetStmt; /**< statement to query a single asset */

    NK_DECL_LOCK(m_mtxLock);            /**< synchronization object */
} __NkInt_AssetManager;

/**
 */
NK_NATIVE typedef struct __NkInt_Asset {
    NKOM_IMPLEMENTS(NkIAsset);

    NkOMRefCount         m_refCount;   /**< asset reference count */
    NkAssetState         m_assetState; /**< asset loading state */
    NkAssetSpecification m_assetSpec;  /**< general asset specification */
} __NkInt_Asset;

/**
 */
NK_NATIVE typedef struct __NkInt_AssetFacClsEntry {
    NkUuid                 const *mp_clsId;
    NkOMImplementationInfo const *mp_implInfo;
} __NkInt_AssetFacClsEntry;


/* Define IID and CLSID of the NkIAsset and NkIAssetManager interfaces. */
// { 59DD958C-1BC2-4425-A7C5-F805184B0AEA }
NKOM_DEFINE_IID(NkIAsset, { 0x59dd958c, 0x1bc2, 0x4425, 0xa7c5f805184b0aea });
// { 5D1DB360-8D98-4EAA-B867-256CB2A37A05 }
NKOM_DEFINE_IID(NkIAssetManager, { 0x5d1db360, 0x8d98, 0x4eaa, 0xb867256cb2a37a05 });
// { 00000000-0000-0000-0000-000000000000 }
NKOM_DEFINE_CLSID(NkIAsset, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });
// { CB9D5E50-DD65-4798-8D7C-110CC86C5C98 }
NKOM_DEFINE_CLSID(NkIAssetManager, { 0xcb9d5e50, 0xdd65, 0x4798, 0x8d7c110cc86c5c98 });


/*
 * The following functions define standard asset-related methods that can be imported
 * into any module implementing specialized asset types in order not to have to duplicate
 * them for every asset subtype.
 */
#pragma region Template Asset Functions
/**
 * \brief implements <tt>NkIAsset::AddRef()</tt> 
 */
NkOMRefCount NK_CALL __NkTempl_Asset_AddRef(_Inout_ NkIAsset *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ++((__NkInt_Asset *)self)->m_refCount;
}

/**
 * \brief implements <tt>NkIAsset::Release()</tt> 
 */
NkOMRefCount NK_CALL __NkTempl_Asset_Release(_Inout_ NkIAsset *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    if (--((__NkInt_Asset *)self)->m_refCount <= 0) {
        __NkInt_Asset *actSelf = (__NkInt_Asset *)self;

        /* Destroy internal state. */
        NkFreeString(actSelf->m_assetSpec.mp_assetName);
        NkFreeString(actSelf->m_assetSpec.mp_assetPath);
        NkFreeString(actSelf->m_assetSpec.mp_assetDocs);
        NkPoolFree((NkVoid *)self);
        return 0;
    }

    return ((__NkInt_Asset *)self)->m_refCount;
}

/**
 * \brief implements <tt>NkIAsset::QueryInterface()</tt> 
 */
_Return_ok_ NkErrorCode NK_CALL __NkTempl_Asset_QueryInterface(
    _Inout_  NkIAsset *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    /**
     * \brief lists all interfaces implemented by the generic implementation of the
     *        \c NkIAsset interface
     */
    NK_INTERNAL NkOMImplementationInfo const gl_c_AssetImplInfos[] = {
        { NKOM_IIDOF(NkIBase)          },
        { NKOM_IIDOF(NkIInitializable) },
        { NKOM_IIDOF(NkIAsset)         },
        { NULL                         }
    };
    if (NkOMQueryImplementationIndex(gl_c_AssetImplInfos, iId) != SIZE_MAX) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkTempl_Asset_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    return NkErr_InterfaceNotImpl;
}

/**
 * \brief implements <tt>NkIAsset::Initialize()</tt> 
 */
_Return_ok_ NkErrorCode NK_CALL __NkTempl_Asset_Initialize(
    _Inout_     NkIAsset *self,
    _Inout_opt_ NkVoid *initParam
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(initParam != NULL, NkErr_InParameter);

    ((__NkInt_Asset *)self)->m_assetSpec = *(NkAssetSpecification *)initParam;
    return NkErr_Ok;
}

/**
 * \brief implements <tt>NkIAsset::GetUuid()</tt> 
 */
NkUuid const *NK_CALL __NkTempl_Asset_GetUuid(_Inout_ NkIAsset *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return &((__NkInt_Asset *)self)->m_assetSpec.m_assetUuid;
}

/**
 * \brief implements <tt>NkIAsset::GetType()</tt> 
 */
NkAssetType NK_CALL __NkTempl_Asset_GetType(_Inout_ NkIAsset *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ((__NkInt_Asset *)self)->m_assetSpec.m_type;
}

/**
 * \brief implements <tt>NkIAsset::GetState()</tt> 
 */
NkAssetState NK_CALL __NkTempl_Asset_GetState(_Inout_ NkIAsset *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ((__NkInt_Asset *)self)->m_assetState;
}

/**
 * \brief implements <tt>NkIAsset::GetName()</tt> 
 */
char const *NK_CALL __NkTempl_Asset_GetName(_Inout_ NkIAsset *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ((__NkInt_Asset *)self)->m_assetSpec.mp_assetName;
}

/**
 * \brief implements <tt>NkIAsset::GetDocumentation()</tt> 
 */
char const *NK_CALL __NkTempl_Asset_GetDocumentation(_Inout_ NkIAsset *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ((__NkInt_Asset *)self)->m_assetSpec.mp_assetDocs;
}

/**
 * \brief implements <tt>NkIAsset::GetPath()</tt>
 */
char const *NK_CALL __NkTempl_Asset_GetPath(_Inout_ NkIAsset *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ((__NkInt_Asset *)self)->m_assetSpec.mp_assetPath;
}
#pragma endregion (Template Asset Functions)


/**
 */
NK_INTERNAL NkUuid const *NK_CALL __NkInt_AssetManager_QueryCLSIDOfAssetType(_In_ NkAssetType assetType) {
    NK_ASSERT(assetType > NkAsTy_Unknown && assetType < __NkAsTy_Count__, NkErr_InParameter);

    /**
     * \brief lists all CLSIDs for each asset type
     */
    NK_INTERNAL NkUuid const *const gl_c_AssetCLSIDs[] = {
        NKOM_CLSIDOF(NkIAsset),
        NULL,
        NULL,
        NULL
    };
    NK_VERIFY_LUT(gl_c_AssetCLSIDs, NkAssetType, __NkAsTy_Count__ - 1);

    return gl_c_AssetCLSIDs[assetType];
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetManager_ExtractBasicInfo(
    _In_  NkVariant const *colResArr,
    _Out_ NkAssetSpecification *resPtr
) {
    NK_ASSERT(colResArr != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    /* Extract the data from the given result tuple. */
    NkInt64      assetType;
    NkBufferView assetUuid;
    NkStringView assetName, assetDocs, assetPath;
    NkVariantGet(&colResArr[0], NULL, &assetUuid);
    NkVariantGet(&colResArr[1], NULL, &assetType);
    NkVariantGet(&colResArr[2], NULL, &assetName);
    NkVariantGet(&colResArr[3], NULL, &assetPath);
    NkVariantGet(&colResArr[4], NULL, &assetDocs);

    /* Initialize output structure. */
    *resPtr = (NkAssetSpecification){
        .m_type       = (NkAssetType)assetType,
        .mp_assetName = NkAllocString(NK_MAKE_ALLOCATION_CONTEXT(), assetName.mp_dataPtr),
        .mp_assetPath = NkAllocString(NK_MAKE_ALLOCATION_CONTEXT(), assetPath.mp_dataPtr),
        .mp_assetDocs = NkAllocString(NK_MAKE_ALLOCATION_CONTEXT(), assetDocs.mp_dataPtr)
    };
    NkUuidCopy((NkUuid const *)assetUuid.mp_dataPtr, &resPtr->m_assetUuid);

    /* Verify result. */
    if (resPtr->mp_assetName != NULL || resPtr->mp_assetPath == NULL || resPtr->mp_assetDocs == NULL) {
        NkFreeString(resPtr->mp_assetName);
        NkFreeString(resPtr->mp_assetPath);
        NkFreeString(resPtr->mp_assetDocs);

        return NkErr_MemoryAllocation;
    }
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL NkErrorCode NK_CALL __NkInt_AssetManager_IterPendingFn(_Inout_ struct NkHashtablePair *pairPtr) {
    NK_ASSERT(pairPtr != NULL, NkErr_InOutParameter);

    /* Extract data from pair. */
    NkUuid   *uuidRef  = pairPtr->m_keyVal.mp_uuidKey;
    NkIAsset *assetRef = (NkIAsset *)pairPtr->mp_valuePtr;

    /* Log basic info of current asset handle. */
    char uuidStr[NK_UUIDLEN];
    NK_LOG_NONE(
        "    [0x%p]: uuid=%s, name=%s, path=%s, type=%s (%i)",
        (NkVoid *)assetRef,
        NkUuidToString(uuidRef, uuidStr),
        assetRef->VT->GetName(assetRef),
        assetRef->VT->GetPath(assetRef),
        NkAssetManagerQueryAssetTypeStr(assetRef->VT->GetType(assetRef))->mp_dataPtr,
        (int)assetRef->VT->GetType(assetRef)
    );

    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetManager_QueryAssetIterFn(
    _In_                 NkUint32 colCount,
    _In_reads_(colCount) NkVariant const *colResArr,
    _Inout_opt_          NkVoid *extraCxtPtr
) {
    NK_ASSERT(colCount > 0, NkErr_InParameter);
    NK_ASSERT(colResArr != NULL, NkErr_InParameter);
    NK_ASSERT(extraCxtPtr != NULL, NkErr_InOutParameter);

    /*
     * First, check if the input parameter is not NULL. This happens if the callback is
     * run for multiple rows, which should never happen since we can only have one asset
     * with the same ID. If this fails, then something is terribly wrong.
     */
    if (extraCxtPtr != NULL) {
        NkUuid uuidRes;
        char uuidBuf[NK_UUIDLEN];

        NkVariantGet(&colResArr[0], NULL, &uuidRes);
        NK_LOG_ERROR(
            "Asset with ID {%s} exists more than once. There's something VEEEEEERY wrong.",
            NkUuidToString(&uuidRes, uuidBuf)
        );
        return NkErr_AssetUUIDIntegrity;
    }

    /* Determine the asset's basic properties. */
    NkAssetSpecification assetSpecs;
    NkErrorCode errCode = __NkInt_AssetManager_ExtractBasicInfo(colResArr, &assetSpecs);
    if (errCode != NkErr_Ok)
        return errCode;

    /* Construct the asset handle. */
    errCode = NkOMCreateInstance(
        __NkInt_AssetManager_QueryCLSIDOfAssetType(assetSpecs.m_type),
        NULL,
        NKOM_IIDOF(NkIAsset),
        (NkVoid *)&assetSpecs,
        (NkIBase **)extraCxtPtr
    );
    return errCode;
}


/**
 * \brief implements <tt>NkIAssetManager::AddRef()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_AssetManager_AddRef(_Inout_ NkIAssetManager *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 * \brief implements <tt>NkIAssetManager::Release()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_AssetManager_Release(_Inout_ NkIAssetManager *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 * \brief implements <tt>NkIAssetManager::QueryInterface()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetManager_QueryInterface(
    _Inout_  NkIAssetManager *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    /**
     * \brief lists all interfaces implemented by the standard asset manager 
     */
    NK_INTERNAL NkOMImplementationInfo const gl_ImplIfaces[] = {
        { NKOM_IIDOF(NkIBase)          },
        { NKOM_IIDOF(NkIInitializable) },
        { NKOM_IIDOF(NkIAssetManager)  },
        { NULL                         }
    };
    if (NkOMQueryImplementationIndex(gl_ImplIfaces, iId) != SIZE_MAX) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_AssetManager_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 * \brief implements <tt>NkIAssetManager::CreateDatabase()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetManager_CreateDatabase(
    _Inout_       NkIAssetManager *self,
    _In_z_ _Utf8_ char const *dbPath
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(dbPath != NULL, NkErr_InParameter);

    /**
     * \brief pointer to current database schema 
     */
    NK_INTERNAL NkStringView const gl_c_CurrDbSchema = NK_MAKE_STRING_VIEW(
        "PRAGMA foreign_keys = OFF;\n"
        "PRAGMA user_version = 1;\n"

        "/*"
        " * table assets"
        " * defines the assets that are part of the game"
        " */"
        "CREATE TABLE assets(\n"
            "uuid BLOB,                  -- unique identifier, saved as a literal 16-byte integer\n"
            "type INT          NOT NULL, -- type (integral)\n"
            "name VARCHAR(128) NOT NULL, -- name, up to 128 characters\n"
            "path TEXT         NOT NULL, -- path string, using '/' as separator\n"
            "docs TEXT,                  -- (optional) documentation string\n"
            ""
            "PRIMARY KEY (uuid)"
        ");"

        "/*"
        " * table dependencies"
        " * defines the dependency graph"
        " */"
        "CREATE TABLE dependencies(\n"
            "depender BLOB NOT NULL, -- UUID of the asset that depends on 'dependee'\n"
            "dependee BLOB NOT NULL, -- UUID of the asset that is being depended on\n"

            "FOREIGN KEY (depender) REFERENCES assets(uuid),\n"
            "FOREIGN KEY (dependee) REFERENCES assets(uuid),\n"
            "UNIQUE      (depender, dependee),\n"
            "CHECK       (depender != dependee)\n"
        ");\n"

        "PRAGMA foreign_keys = ON;"
    );

    /* Create new database handle. */
    NkIDatabase *dbHandle;
    NkErrorCode errCode = NkOMCreateInstance(
        NKOM_CLSIDOF(NkIDatabase),
        NULL,
        NKOM_IIDOF(NkIDatabase),
        NULL,
        (NkIBase **)&dbHandle
    );
    if (errCode != NkErr_Ok)
        return errCode;

    /* Create the database. */
    errCode = dbHandle->VT->Create(dbHandle, gl_c_CurrDbSchema.mp_dataPtr, dbPath, NkDbMode_ReadWrite);
    if (errCode != NkErr_Ok) {
        dbHandle->VT->Release(dbHandle);

        return errCode;
    }

    /*
     * Close the database again. Require a subsequent call to '::OpenDatabase()' to
     * finish the opening procedure. Releasing it will also close it if the connection
     * handle itself is destroyed.
     */
    dbHandle->VT->Release(dbHandle);
    return NkErr_Ok;
}

/**
 * \brief implements <tt>NkIAssetManager::OpenDatabase()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetManager_OpenDatabase(
    _Inout_       NkIAssetManager *self,
    _In_z_ _Utf8_ char const *dbPath
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(dbPath != NULL && *dbPath ^ '\0', NkErr_InParameter);

    /* Get pointer to actual asset manager instance. */
    __NkInt_AssetManager *actSelf = (__NkInt_AssetManager *)self;

    /* Create new database handle. */
    NkErrorCode errCode = NkOMCreateInstance(
        NKOM_CLSIDOF(NkIDatabase),
        NULL,
        NKOM_IIDOF(NkIDatabase),
        NULL,
        (NkIBase **)&actSelf->mp_dbConn
    );
    if (errCode != NkErr_Ok)
        return errCode;

    /*
     * Attach the database to the handle. If the application is running in standalone
     * mode, we assume that the application is running in a freestanding environment,
     * that is, without the editor running. In such a case, the database is only
     * readable; otherwise, that is, when running in 'attached' mode, the database must
     * be opened in read-write mode.
     */
    errCode = actSelf->mp_dbConn->VT->Open(
        actSelf->mp_dbConn,
        dbPath,
        NkApplicationIsStandalone()
            ? NkDbMode_ReadOnly
            : NkDbMode_ReadWrite
    );
    if (errCode != NkErr_Ok) {
        /* If we failed to open the database, we destroy the handle, too. */
        actSelf->mp_dbConn->VT->Release(actSelf->mp_dbConn);

        actSelf->mp_dbConn = NULL;
        return errCode;
    }

    /* Create the 'query asset' statement. */
    errCode = actSelf->mp_dbConn->VT->CreateStatement(
        actSelf->mp_dbConn,
        "SELECT * FROM assets WHERE uuid = ?",
        &actSelf->mp_queryAssetStmt
    );
    if (errCode != NkErr_Ok) {
        /*
         * If we could not create the statement, we shutdown the database since we cannot
         * really use the asset manager without the statements being ready.
         */
        actSelf->mp_dbConn->VT->Release(actSelf->mp_dbConn);

        actSelf->mp_dbConn = NULL;
        return errCode;
    }

    /* All good. */
    return NkErr_Ok;
}

/**
 * \brief implements <tt>NkIAssetManager::CloseDatabase()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetManager_CloseDatabase(_Inout_ NkIAssetManager *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Get pointer to actual asset manager instance. */
    __NkInt_AssetManager *actSelf = (__NkInt_AssetManager *)self;

    /* If no database is open, fail. */
    if (actSelf->mp_dbConn == NULL)
        return NkErr_ComponentState;

    /* Close the database and release all database-specific resources. */
    actSelf->mp_queryAssetStmt->VT->Release(actSelf->mp_queryAssetStmt);
    actSelf->mp_dbConn->VT->Release(actSelf->mp_dbConn);
    actSelf->mp_dbConn         = NULL;
    actSelf->mp_queryAssetStmt = NULL;

    /* All good. */
    return NkErr_Ok;
}

/**
 * \brief implements <tt>NkIAssetManager::QueryAsset()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetManager_QueryAsset(
    _Inout_  NkIAssetManager *self,
    _In_     NkUuid const *assetId,
    _Outptr_ NkIAsset **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(assetId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    /* Get pointer to actual asset manager instance. */
    __NkInt_AssetManager *actSelf = (__NkInt_AssetManager *)self;

    /* Check if the given asset handle is already present in the cache. */
    NkErrorCode eCode = NkHashtableAt(
        actSelf->mp_assetCache,
        &(NkHashtableKey const){
            .mp_uuidKey = (NkUuid *)assetId
        },
        resPtr
    );
    if (eCode == NkErr_Ok) {
        /* Present in cache; add reference and return. */
        (*resPtr)->VT->AddRef(*resPtr);

        return NkErr_Ok;
    }
    *resPtr = NULL;

    /* Bind the given UUID parameter. */
    NkVariant paramVar;
    NkVariantSet(&paramVar, NkVarTy_Uuid, assetId);
    actSelf->mp_queryAssetStmt->VT->Bind(actSelf->mp_queryAssetStmt, 1U, &paramVar);

    /* Query the asset and add it to the cache. */
    eCode = actSelf->mp_dbConn->VT->Execute(
        actSelf->mp_dbConn,
        actSelf->mp_queryAssetStmt,
        &__NkInt_AssetManager_QueryAssetIterFn,
        (NkVoid *)resPtr
    );
    if (eCode != NkErr_Ok)
        *resPtr = NULL;

    /* Unbind param and return. */
    actSelf->mp_queryAssetStmt->VT->Unbind(actSelf->mp_queryAssetStmt, 1U);
    return eCode;
}


/**
 * \brief global asset manager instance 
 */
NK_INTERNAL __NkInt_AssetManager gl_AssetManager = {
    .NkIAssetManager_Iface.VT = &(struct __NkIAssetManager_VTable__){
        .QueryInterface = &__NkInt_AssetManager_QueryInterface,
        .AddRef         = &__NkInt_AssetManager_AddRef,
        .Release        = &__NkInt_AssetManager_Release,
        .CreateDatabase = &__NkInt_AssetManager_CreateDatabase,
        .OpenDatabase   = &__NkInt_AssetManager_OpenDatabase,
        .CloseDatabase  = &__NkInt_AssetManager_CloseDatabase,
        .QueryAsset     = &__NkInt_AssetManager_QueryAsset
    }
};

/**
 */
NK_INTERNAL NkIBase *NK_CALL __NkInt_AssetManager_QueryInstance(NkVoid) {
    return (NkIBase *)&gl_AssetManager;
}


/**
 * \brief <tt>NkIClassFactory::AddRef()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_AssetFac_AddRef(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 * \brief <tt>NkIClassFactory::Release()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_AssetFac_Release(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 * \brief <tt>NkIClassFactory::QueryInterface()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetFac_QueryInterface(
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

        __NkInt_AssetFac_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 * \brief <tt>NkIClassFactory::QueryInstantiableClasses()</tt>
 */
NK_INTERNAL NkUuid const **NK_CALL __NkInt_AssetFac_QueryInstantiableClasses(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /**
     * \brief lists all IDs of the classes that are instantiable by the current factory
     *        instance
     */
    NK_INTERNAL NkUuid const *gl_c_AssetFacInstCls[] = { NULL };

    return gl_c_AssetFacInstCls;
}

/**
 * \brief <tt>NkIClassFactory::CreateInstance()</tt>
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetFac_QueryInstance(
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
     * \brief list implementation infos for asset classes
     */
    NK_INTERNAL __NkInt_AssetFacClsEntry gl_c_AssetFacClsInfos[] = {
        { NULL, NULL }
    };
    NK_INTERNAL NkSize const gl_c_AssetFacClsInfoSize = NK_ARRAYSIZE(gl_c_AssetFacClsInfos);

    /* Retrieve the entry for the given class ID. */
    NkOMImplementationInfo const *implInfo = NULL;
    for (NkSize i = 0; i < gl_c_AssetFacClsInfoSize; i++)
        if (gl_c_AssetFacClsInfos[i].mp_clsId != NULL && NkUuidIsEqual(gl_c_AssetFacClsInfos[i].mp_clsId, clsId)) {
            implInfo = gl_c_AssetFacClsInfos[i].mp_implInfo;

            break;
        }
    if (implInfo == NULL) {
        /* Class entry not found; class is not implemented. */
        *resPtr = NULL;

        return NkErr_UnknownClass;
    }

    /* Instantiate the class. */
    NkErrorCode eCode = NkGPAlloc(NK_MAKE_ALLOCATION_CONTEXT(), implInfo->m_structSize, 0, NK_TRUE, (NkVoid **)resPtr);
    if (eCode != NkErr_Ok) {
        *resPtr = NULL;

        return eCode;
    }
    (*resPtr)->VT = (struct __NkIBase_VTable__ *)implInfo->mp_vtabPtr;

    /* All went well. */
    (*resPtr)->VT->AddRef(*resPtr);
    return NkErr_Ok;
}


/**
 * \brief static asset factory instance; has no state, thus is a static and constant
 *        object 
 */
NK_INTERNAL NkIClassFactory const gl_c_AssetFactory = {
    .VT = &(struct __NkIClassFactory_VTable__ const) {
        .QueryInterface           = &__NkInt_AssetFac_QueryInterface,
        .AddRef                   = &__NkInt_AssetFac_AddRef,
        .Release                  = &__NkInt_AssetFac_Release,
        .QueryInstantiableClasses = &__NkInt_AssetFac_QueryInstantiableClasses,
        .CreateInstance           = &__NkInt_AssetFac_QueryInstance
    }
};


NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(AssetManager)(NkVoid) {
    __NkInt_AssetManager *actSelf = (__NkInt_AssetManager *)__NkInt_AssetManager_QueryInstance();

    /* Initialize asset cache. */
    NkErrorCode errCode = NkHashtableCreate(&(NkHashtableProperties const){
        .m_structSize  = sizeof(NkHashtableProperties),
            .m_initCap     = 64,
            .m_keyType     = NkHtKeyTy_Uuid,
            .m_minCap      = 16,
            .m_maxCap      = UINT32_MAX - 2,
            .mp_fnElemFree = NULL
    }, &actSelf->mp_assetCache);
    if (errCode != NkErr_Ok)
        return errCode;
    /* Initialize synchronization primitive. */
    NK_INITLOCK(actSelf->m_mtxLock);

    /* Register the asset factory. */
    errCode = NkOMInstallClassFactory((NkIClassFactory *)&gl_c_AssetFactory);
    if (errCode != NkErr_Ok) {
        NK_DESTROYLOCK(actSelf->m_mtxLock);

        NkHashtableDestroy(&actSelf->mp_assetCache);
        return errCode;
    }

    /*
     * Try to locate the asset database, if it does not exist, create a new one. Then,
     * open the database. Only do this if we are running in standalone mode, that is,
     * without the editor.
     */
    if (NkApplicationIsStandalone()) {
        NkIFilesystem *fileSysSrv = (NkIFilesystem *)NkApplicationQueryInstance(NKOM_CLSIDOF(NkIFilesystem));
        if (!fileSysSrv->VT->Exists(fileSysSrv, "assets.db")) {
            NK_LOG_WARNING("Asset database \"%s\" could not be found; creating new database.", "assets.db");

            errCode = actSelf->NkIAssetManager_Iface.VT->CreateDatabase((NkIAssetManager *)actSelf, "assets.db");
            if (errCode != NkErr_Ok) {
                NK_LOG_ERROR("Could not create database \"%s\".", "assets.db");

                NK_DESTROYLOCK(actSelf->m_mtxLock);
                NkHashtableDestroy(&actSelf->mp_assetCache);
                fileSysSrv->VT->Release(fileSysSrv);
                return errCode;
            }

            NK_LOG_INFO("Successfully created asset database \"%s\".", "assets.db");
        }
        fileSysSrv->VT->Release(fileSysSrv);

        /* Open the database. */
        errCode = actSelf->NkIAssetManager_Iface.VT->OpenDatabase((NkIAssetManager *)actSelf, "assets.db");
        if (errCode != NkErr_Ok) {
            NK_DESTROYLOCK(actSelf->m_mtxLock);

            NkHashtableDestroy(&actSelf->mp_assetCache);
            return errCode;
        }
    }

    /* All good. */
    return NkErr_Ok;
}

NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(AssetManager)(NkVoid) {
    __NkInt_AssetManager *actSelf = (__NkInt_AssetManager *)__NkInt_AssetManager_QueryInstance();

    /* Close the connection. */
    if (NkApplicationIsStandalone())
        actSelf->NkIAssetManager_Iface.VT->CloseDatabase((NkIAssetManager *)actSelf);

    /* Unregister the asset factory. */
    NkErrorCode errCode = NkOMUninstallClassFactory((NkIClassFactory *)&gl_c_AssetFactory);
    if (errCode != NkErr_Ok)
        return errCode;
    /* Destroy synchronization object. */
    NK_DESTROYLOCK(actSelf->m_mtxLock);

    /* Asset registry should be empty by now. If it isn't, then there is an issue. */
    NkUint32 htCount;
    if ((htCount = NkHashtableCount(actSelf->mp_assetCache)) > 0U) {
        NK_LOG_CRITICAL(
            "There are still %u assets registered in the asset manager. This means that there must be resource "
            "leaks or pending asset handles.",
            htCount
        );

        /* Print the IDs and names of all assets that are still registered. */
        NK_LOG_CRITICAL("The following asset handles are still pending:");
        NK_IGNORE_RETURN_VALUE(NkHashtableForEach(actSelf->mp_assetCache, &__NkInt_AssetManager_IterPendingFn));
    }
    /* Destroy asset cache. */
    NkHashtableDestroy(&actSelf->mp_assetCache);

    return NkErr_Ok;
}
/** \endcond */


NkStringView const *NK_CALL NkAssetManagerQueryAssetTypeStr(_In_ NkAssetType typeId) {
    NK_ASSERT(typeId >= 0 && typeId < __NkAsTy_Count__, NkErr_InParameter);
    
    /** \cond INTERNAL */
    /**
     * \brief lookup table containing the string representations of all existing asset
     *        types 
     */
    NK_INTERNAL NkStringView const gl_c_AssetTypeStrs[] = {
        [NkAsTy_Unknown]      = NK_MAKE_STRING_VIEW(NK_ESC(NkAsTy_Unknown)),
        [NkAsTy_World]        = NK_MAKE_STRING_VIEW(NK_ESC(NkAsTy_World)),
        [NkAsTy_TextureAtlas] = NK_MAKE_STRING_VIEW(NK_ESC(NkAsTy_TextureAtlas)),
        [NkAsTy_Level]        = NK_MAKE_STRING_VIEW(NK_ESC(NkAsTy_Level))
    };
    NK_VERIFY_LUT(gl_c_AssetTypeStrs, NkAssetType, __NkAsTy_Count__);
    /** \endcond */

    return &gl_c_AssetTypeStrs[typeId];
}


/** \cond INTERNAL */
/**
 * \brief info for the \e AssetManager component 
 */
NK_COMPONENT_DEFINE(AssetManager) {
    .m_compUuid     = { 0x99a5a203, 0xccf2, 0x49c2, 0x854862b2cf86be1a },
    .mp_clsId       = NKOM_CLSIDOF(NkIAssetManager),
    .m_compIdent    = NK_MAKE_STRING_VIEW("asset manager"),
    .m_compFlags    = 0,
    .m_isNkOM       = NK_TRUE,
   
    .mp_fnQueryInst = &__NkInt_AssetManager_QueryInstance,
    .mp_fnStartup   = &NK_COMPONENT_STARTUPFN(AssetManager),
    .mp_fnShutdown  = &NK_COMPONENT_SHUTDOWNFN(AssetManager)
};
/** \endcond */


#undef NK_NAMESPACE


