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


/* Define IID and CLSID of NkIAssetManager interface. */
// { 5D1DB360-8D98-4EAA-B867-256CB2A37A05 }
NKOM_DEFINE_IID(NkIAssetManager, { 0x5d1db360, 0x8d98, 0x4eaa, 0xb867256cb2a37a05 });
// { CB9D5E50-DD65-4798-8D7C-110CC86C5C98 }
NKOM_DEFINE_CLSID(NkIAssetManager, { 0xcb9d5e50, 0xdd65, 0x4798, 0x8d7c110cc86c5c98 });


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

    /** \todo IMPL */
    return NkErr_Ok;
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
 * \brief implements <tt>NkIAssetManager::Initialize()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_AssetManager_Initialize(
    _Inout_     NkIAssetManager *self,
    _Inout_opt_ NkVoid *initParam
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(initParam);

    /* Get pointer to actual asset manager instance. */
    __NkInt_AssetManager *actSelf = (__NkInt_AssetManager *)self;

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
    /* All good. */
    return NkErr_Ok;
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
        .Initialize     = &__NkInt_AssetManager_Initialize,
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


NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(AssetManager)(NkVoid) {
    NkIAssetManager *self = (NkIAssetManager *)__NkInt_AssetManager_QueryInstance();

    /* Initialize the asset manager itself. */
    NkErrorCode errCode = self->VT->Initialize(self, NULL);
    if (errCode != NkErr_Ok)
        return errCode;

    /*
     * Try to locate the asset database, if it does not exist, create a new one. Then,
     * open the database. Only do this if we are running in standalone mode, that is,
     * without the editor.
     */
    if (NkApplicationIsStandalone()) {
        NkIFilesystem *fileSysSrv = (NkIFilesystem *)NkApplicationQueryInstance(NKOM_CLSIDOF(NkIFilesystem));
        if (!fileSysSrv->VT->Exists(fileSysSrv, "assets.db")) {
            NK_LOG_WARNING("Asset database \"%s\" could not be found; creating new database.", "assets.db");

            if ((errCode = self->VT->CreateDatabase(self, "assets.db")) != NkErr_Ok) {
                NK_LOG_ERROR("Could not create database \"%s\".", "assets.db");

                return errCode;
            }

            NK_LOG_INFO("Successfully created asset database \"%s\".", "assets.db");
        }
        fileSysSrv->VT->Release(fileSysSrv);

        /* Open the database. */
        return self->VT->OpenDatabase(self, "assets.db");
    }

    /* All good. */
    return NkErr_Ok;
}

NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(AssetManager)(NkVoid) {
    __NkInt_AssetManager *actSelf = (__NkInt_AssetManager *)__NkInt_AssetManager_QueryInstance();

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

    /* Close the connection. */
    if (NkApplicationIsStandalone())
        actSelf->NkIAssetManager_Iface.VT->CloseDatabase((NkIAssetManager *)actSelf);
    /* Destroy synchronization object. */
    NK_DESTROYLOCK(actSelf->m_mtxLock);

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


