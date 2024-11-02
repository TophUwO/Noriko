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
 * \file  path.c
 * \brief implements the public API for the platform-independent path builder
 * \note  All classes exposed by this API are fully thread-safe.
 * 
 * \par Description
 *   This class handles the building of syntactically correct path strings according to
 *   the host platform's conventions. For example, on Windows, the preferred path
 *   separator is '\\' while on Unix-based systems, it's '/'.<br>
 *   What this class is not concerned with is whether or not the paths actually point to
 *   valid resources.
 */
#define NK_NAMESPACE "nk::path"


/* Noriko includes */
#include <include/Noriko/path.h>
#include <include/Noriko/log.h>
#include <include/Noriko/noriko.h>


/** \cond INTERNAL */
/**
 */
NK_INTERNAL NkStringView gl_StandardPaths[] = {
    [NkStdLoc_SystemRoot] = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_Desktop]    = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_Documents]  = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_Home]       = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_Music]      = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_Videos]     = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_Downloads]  = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_Fonts]      = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_AppData]    = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_AppDir]     = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_GameSaves]  = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_GameRoot]   = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_GameBin]    = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_GameData]   = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_GameDocs]   = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_GameCache]  = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_GameAssets] = NK_MAKE_STRING_VIEW(NULL),
    [NkStdLoc_GameExt]    = NK_MAKE_STRING_VIEW(NULL)
};
NK_VERIFY_LUT(gl_StandardPaths, NkStdLocation, __NkStdLoc_Count__);

/**
 * \brief array of game-specific standard locations. 
 */
NK_INTERNAL NkStdLocation const gl_c_GameLocs[] = {
    NkStdLoc_GameSaves,  NkStdLoc_GameRoot, 
    NkStdLoc_GameBin,    NkStdLoc_GameData,
    NkStdLoc_GameDocs,   NkStdLoc_GameCache,
    NkStdLoc_GameAssets, NkStdLoc_GameExt
};
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkPathStartup(NkVoid) {
    NK_LOG_INFO("startup: path services");

    /** \cond INTERNAL */
    /**
     */
    NK_EXTERN NK_VIRTUAL _Return_ok_ NkErrorCode NK_CALL __NkInt_StandardPaths_QueryPlatformLocs(NkStringView *stdLocs);
    /** \endcond */

    /* Query platform-dependent paths. */
    NkErrorCode errCode = __NkInt_StandardPaths_QueryPlatformLocs(gl_StandardPaths);
    if (errCode != NkErr_Ok)
        return errCode;

    /* Now, do the game-dependent paths. */
    NkStringView const *gameRootDir = &NkApplicationQuerySpecification()->m_gameRootDir;
    for (NkSize i = 0; i < NK_ARRAYSIZE(gl_c_GameLocs); i++) {
        NkStdLocation const currLoc = gl_c_GameLocs[i];
        if (gl_StandardPaths[currLoc].mp_dataPtr != NULL) {
#pragma warning (suppress: 4127)
            NK_ASSERT_EXTRA(NK_FALSE, NkErr_Unknown, "System paths initialized game paths. Unexpected behavior");

            continue;
        }

        /* Initialize the current path. */
        switch (currLoc) {
            case NkStdLoc_GameRoot: gl_StandardPaths[currLoc] = *gameRootDir; break;
            default:
                NK_LOG_WARNING(
                    "No standard location available for \"%s\".",
                    NkPathQueryStandardLocIdStr(currLoc)->mp_dataPtr
                );
        }
    }

    /* All good. */
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkPathShutdown(NkVoid) {
    NK_LOG_INFO("shutdown: path services");

    /** \cond INTERNAL */
    /**
     */
    NK_EXTERN NK_VIRTUAL NkVoid NK_CALL __NkInt_StandardPaths_DestroyPlatformLocs(NkStringView *stdLocs);
    /** \endcond */

    /* First, destroy platform-dependent paths. */
    __NkInt_StandardPaths_DestroyPlatformLocs(gl_StandardPaths);

    return NkErr_Ok;
}

NkStringView const *NK_CALL NkPathQueryStdLocation(_In_ NkStdLocation locId) {
    NK_ASSERT(locId >= 0 && locId < __NkStdLoc_Count__, NkErr_InParameter);

    return &gl_StandardPaths[locId];
}


NkStringView const *NK_CALL NkPathQueryStandardLocIdStr(_In_ NkStdLocation locId) {
    NK_ASSERT(locId >= NkStdLoc_Unknown && locId < __NkStdLoc_Count__, NkErr_InParameter);

    /** \cond INTERNAL */
    /**
     * \brief string representation table for standard location IDs 
     */
    NK_INTERNAL NkStringView const gl_c_LocIdStrReps[] = {
        [NkStdLoc_Unknown]    = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_Unknown)),
        [NkStdLoc_SystemRoot] = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_SystemRoot)),
        [NkStdLoc_Desktop]    = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_Desktop)),
        [NkStdLoc_Documents]  = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_Documents)),
        [NkStdLoc_Home]       = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_Home)),
        [NkStdLoc_Music]      = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_Music)),
        [NkStdLoc_Videos]     = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_Videos)),
        [NkStdLoc_Downloads]  = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_Downloads)),
        [NkStdLoc_Fonts]      = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_Fonts)),
        [NkStdLoc_AppData]    = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_AppData)),
        [NkStdLoc_AppDir]     = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_AppDir)),
        [NkStdLoc_GameSaves]  = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_GameSaves)),
        [NkStdLoc_GameRoot]   = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_GameRoot)),
        [NkStdLoc_GameBin]    = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_GameBin)),
        [NkStdLoc_GameData]   = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_GameData)),
        [NkStdLoc_GameDocs]   = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_GameDocs)),
        [NkStdLoc_GameCache]  = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_GameCache)),
        [NkStdLoc_GameAssets] = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_GameAssets)),
        [NkStdLoc_GameExt]    = NK_MAKE_STRING_VIEW(NK_ESC(NkStdLoc_GameExt))
    };
    NK_VERIFY_LUT(gl_c_LocIdStrReps, NkStdLocation, __NkStdLoc_Count__);
    /** \endcond */

    return &gl_c_LocIdStrReps[locId];
}


#undef NK_NAMESPACE


