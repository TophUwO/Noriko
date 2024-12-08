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


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>
#include <include/Noriko/nkom.h>
#include <include/Noriko/util.h>

#include <include/Noriko/dstruct/string.h>


/**
 */
NK_NATIVE typedef enum NkAssetType {
    NkAsTy_Unknown = 0,

    NkAsTy_World,
    NkAsTy_TextureAtlas,
    NkAsTy_Level,

    __NkAsTy_Count__
} NkAssetType;

/**
 */
NK_NATIVE typedef enum NkAssetState {
    NkAsSt_Unspecified = 0,

    NkAsSt_Ready,
    NkAsSt_Loading,
    NkAsSt_Invalid,
    NkAsSt_ReadyForLoading,

    __NkAsSt_Count__
} NkAssetState;


/**
 */
NK_NATIVE typedef struct NkAssetSpecification {
    NkUuid       m_assetUuid;  /**< unique asset identifier */
    NkAssetType  m_type;       /**< type of the asset */
    char        *mp_assetName; /**< name of the asset, for debugging and during development */
    char        *mp_assetPath; /**< path of the asset, relative to root asset directory */
    char        *mp_assetDocs; /**< asset description/documentation (optional) */
} NkAssetSpecification;


/**
 */
NKOM_DECLARE_INTERFACE(NkIAssetTree) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkIAssetTree *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIAssetTree *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIAssetTree *self);

    /**
     */
    NkErrorCode (NK_CALL *Traverse)(
        _Inout_     NkIAssetTree *self,
        _In_        NkBoolean (NK_CALL *fnIter)(NkIAssetTree *self, struct NkIAsset *currAsset, NkVoid *extraCxt),
        _Inout_opt_ NkVoid *extraCxtPtr
    );
};

/**
 */
NKOM_DECLARE_INTERFACE(NkIAsset) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkIAsset *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIAsset *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIAsset *self);
    
    /**
     */
    NkUuid const *(NK_CALL *GetUuid)(_Inout_ NkIAsset *self);
    /**
     */
    NkAssetType (NK_CALL *GetType)(_Inout_ NkIAsset *self);
    /**
     */
    char const *(NK_CALL *GetName)(_Inout_ NkIAsset *self);
    /**
     */
    char const *(NK_CALL *GetDocumentation)(_Inout_ NkIAsset *self);
    /**
     */
    char const *(NK_CALL *GetPath)(_Inout_ NkIAsset *self);
    /**
     */
    NkIAssetTree *(NK_CALL *GetDependencyTree)(_Inout_ NkIAsset *self);
    /**
     */
    NkAssetState (NK_CALL *GetAssetState)(_Inout_ NkIAsset *self);

    /**
     */
    NkErrorCode (NK_CALL *Load)(_Inout_ NkIAsset *self, _Inout_opt_ NkVoid *extraCxtPtr);
    /**
     */
    NkErrorCode (NK_CALL *Unload)(_Inout_ NkIAsset *self, _Inout_opt_ NkVoid *extraCxtPtr);
};

/**
 */
NKOM_DECLARE_INTERFACE(NkIAssetManager) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(
        _Inout_  NkIAssetManager *self,
        _In_     NkUuid const *iId,
        _Outptr_ NkVoid **resPtr
    );
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIAssetManager *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIAssetManager *self);

    /**
     */
    NkErrorCode (NK_CALL *Initialize)(_Inout_ NkIAssetManager *self, _Inout_opt_ NkVoid *initParam);

    /**
     */
    NkErrorCode (NK_CALL *CreateDatabase)(_Inout_ NkIAssetManager *self, _In_z_ _Utf8_ char const *dbPath);
    /**
     */
    NkErrorCode (NK_CALL *OpenDatabase)(_Inout_ NkIAssetManager *self, _In_z_ _Utf8_ char const *dbPath);
    /**
     */
    NkErrorCode (NK_CALL *CloseDatabase)(_Inout_ NkIAssetManager *self);
    /**
     */
    NkErrorCode (NK_CALL *QueryAsset)(
        _Inout_  NkIAssetManager *self,
        _In_     NkUuid const *assetId,
        _Outptr_ NkIAsset **resPtr
    );
};


/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkAssetManagerQueryAssetTypeStr(_In_ NkAssetType typeId);


