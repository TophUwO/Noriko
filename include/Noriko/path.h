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
NK_NATIVE typedef enum NkGameDirectory {
    NkGameDir_Unknown = 0,

    NkGameDir_BinRoot,
    NkGameDir_DataRoot,
    NkGameDir_DocsRoot,
    NkGameDir_CacheRoot,
    NkGameDir_ExtRoot,
    NkGameDir_AssetRoot,
    NkGameDir_MapAssets,
    NkGameDir_TilesetAssets,

    __NkGameDir_Count__
} NkGameDirectory;


/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkPathStartup(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkPathShutdown(NkVoid);

/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkPathBuild(
    _In_opt_ NkStringView const *sepCh,
    _In_opt_ NkStringView const **stemCompArr,
    _In_opt_ NkStringView const *fileName,
    _In_opt_ NkStringView const *extStr,
    _Out_    NkString *resStr
);
/**
 */
NK_NATIVE NK_API NkString *NK_CALL NkPathToNativeSeparators(_Inout_ NkString *strPtr);

/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkPathQueryStdLocation(_In_ NkStdLocation locId);
/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkPathQueryGameDirectory(_In_ NkGameDirectory dirId);
/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkPathQueryStandardLocIdStr(_In_ NkStdLocation locId);


