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

    NkBoolean    m_isStandalone; /**< whether or not the asset manager is running in standalone mode */
    NkHashtable *mp_assetCache;  /**< asset cache, used for querying */
    NkIDatabase *mp_dbConn;      /**< database connection handle */
    NkString     m_dbFileName;   /**< path to the database file */

    NK_DECL_LOCK(m_mtxLock);     /**< synchronization object */
} __NkInt_AssetManager;


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
/** \endcond */


NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(AssetManager)(NkVoid) {
    /* Get pointer to application startup info. */
    NkApplicationSpecification const *appSpecs = NkApplicationQuerySpecification();

    /* Initialize asset cache. */
    NkHashtable *assetCache;
    NkErrorCode errCode = NkHashtableCreate(&(NkHashtableProperties const){
        .m_structSize  = sizeof(NkHashtableProperties),
        .m_initCap     = 64,
        .m_keyType     = NkHtKeyTy_Uuid,
        .m_minCap      = 16,
        .m_maxCap      = UINT32_MAX - 2,
        .mp_fnElemFree = NULL
    }, &assetCache);
    if (errCode != NkErr_Ok)
        return errCode;

    /* Initialize synchronization primitive. */
    NK_INITLOCK(gl_AssetManager.m_mtxLock);

    /* Initialize data-structure. */
    gl_AssetManager = (__NkInt_AssetManager){
        .mp_assetCache  = assetCache,
        .mp_dbConn      = NULL,
        .m_isStandalone = NK_TRUE
    };
    /* All good. */
    return NkErr_Ok;
}

NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(AssetManager)(NkVoid) {
    /* Asset registry should be empty by now. If it isn't, then there is an issue. */
    NkUint32 htCount;
    if ((htCount = NkHashtableCount(gl_AssetManager.mp_assetCache)) > 0U) {
        NK_LOG_CRITICAL(
            "There are still %u assets registered in the asset manager. This means that there must be resource "
            "leaks or pending asset handles.",
            htCount
        );

        /* Print the IDs and names of all assets that are still registered. */
        NK_LOG_CRITICAL("The following asset handles are still pending:");
        NK_IGNORE_RETURN_VALUE(NkHashtableForEach(gl_AssetManager.mp_assetCache, &__NkInt_AssetManager_IterPendingFn));
    }
    /* Destroy asset cache. */
    NkHashtableDestroy(&gl_AssetManager.mp_assetCache);

    /* Close the connection. */
    if (gl_AssetManager.m_isStandalone)
        NK_IGNORE_RETURN_VALUE(NkAssetManagerCloseDatabase());
    /* Destroy synchronization object. */
    NK_DESTROYLOCK(gl_AssetManager.m_mtxLock);

    return NkErr_Ok;
}


_Return_ok_ NkErrorCode NK_CALL NkAssetManagerCreateDatabase(_In_z_ _Utf8_ char const *dbPath) {
    NK_ASSERT(dbPath != NULL, NkErr_InParameter);
}

_Return_ok_ NkErrorCode NK_CALL NkAssetManagerOpenDatabase(_In_z_ _Utf8_ char const *dbPath) {

}

_Return_ok_ NkErrorCode NK_CALL NkAssetManagerCloseDatabase(NkVoid) {

}


NkIAsset *NK_CALL NkAssetManagerQueryAsset(_In_ NkUuid const *assetId) {

}


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


/**
* \brief global asset manager instance 
*/
NK_INTERNAL __NkInt_AssetManager gl_AssetManager = {
    .NkIAssetManager_Iface = &(struct __NkIAssetManager_VTable__) {
        .QueryInterface = NULL,
        .AddRef         = NULL,
        .Release        = NULL,
        .Initialize     = NULL,
        .CreateDatabase = NULL,
        .OpenDatabase   = NULL,
        .CloseDatabase  = NULL,
        .QueryAsset     = NULL
    }
};


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
   
    .mp_fnQueryInst = NULL,
    .mp_fnStartup   = &NK_COMPONENT_STARTUPFN(AssetManager),
    .mp_fnShutdown  = &NK_COMPONENT_SHUTDOWNFN(AssetManager)
};
/** \endcond */


#undef NK_NAMESPACE


