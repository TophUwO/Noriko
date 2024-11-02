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
 * \file  path.h
 * \brief defines the public API for the platform-independent path builder
 * \note  All classes exposed by this API are fully thread-safe.
 * 
 * \par Description
 *   This class handles the building of syntactically correct path strings according to
 *   the host platform's conventions. For example, on Windows, the preferred path
 *   separator is '\\' while on Unix-based systems, it's '/'.<br>
 *   What this class is not concerned with is whether or not the paths actually point to
 *   valid resources.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>
#include <include/Noriko/nkom.h>

#include <include/Noriko/dstruct/string.h>


/**
 */
NK_NATIVE typedef enum NkStdLocation {
    NkStdLoc_Unknown = 0,

    NkStdLoc_SystemRoot,
    NkStdLoc_Desktop,
    NkStdLoc_Documents,
    NkStdLoc_Home,
    NkStdLoc_Music,
    NkStdLoc_Videos,
    NkStdLoc_Downloads,
    NkStdLoc_Fonts,
    NkStdLoc_AppData,
    NkStdLoc_AppDir,
    NkStdLoc_GameSaves,
    NkStdLoc_GameRoot,
    NkStdLoc_GameBin,
    NkStdLoc_GameData,
    NkStdLoc_GameDocs,
    NkStdLoc_GameCache,
    NkStdLoc_GameAssets,
    NkStdLoc_GameExt,

    __NkStdLoc_Count__
} NkStdLocation;


/**
 */
NKOM_DECLARE_INTERFACE(NkIStandardPaths) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(
        _Inout_  NkIStandardPaths *self,
        _In_     NkUuid const *iId,
        _Outptr_ NkVoid **resPtr
    );
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt>
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIStandardPaths *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt>
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIStandardPaths *self);

    /**
     */
    NkStringView const *(NK_CALL *QueryStdLocation)(_Inout_ NkIStandardPaths *self, _In_ NkStdLocation locId);
};

/**
 */
NKOM_DECLARE_INTERFACE(NkIPathFactory) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(
        _Inout_  NkIPathFactory *self,
        _In_     NkUuid const *iId,
        _Outptr_ NkVoid **resPtr
    );
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt>
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIPathFactory *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt>
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIPathFactory *self);

    /**
     */
    NkString *(NK_CALL *Build)(
        _Inout_  NkIPathFactory *self,
        _In_     NkStringView const *stemCompArr,
        _In_opt_ NkStringView const *fileName,
        _In_opt_ NkStringView const *extStr
    );
    /**
     */
    NkString *(NK_CALL *ToNativeSeparators)(_Inout_ NkIPathFactory *self, _Inout_ NkString *pathStr);
};


/**
 */
NK_NATIVE NK_API NK_VIRTUAL _Return_ok_ NkErrorCode NK_CALL NkPathStartup(NkVoid);
/**
 */
NK_NATIVE NK_API NK_VIRTUAL _Return_ok_ NkErrorCode NK_CALL NkPathShutdown(NkVoid);

/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkPathQueryStandardLocIdStr(_In_ NkStdLocation locId);


