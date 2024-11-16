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
#include <include/Noriko/comp.h>


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

/**
 * \brief default path size, in bytes, actual path can grow beyond this value
 */
NK_INTERNAL NkUint32 const gl_c_DefPathSize = 64U;


/**
 * \ingroup VirtFn
 * \brief   returns the native path separator for the current platform
 * \return  static string variable that contains the path separator formatted as UTF-8
 * \warning This function must not return <tt>NULL</tt>. That's undefined behavior.
 *
 * \par Remarks
 *   The string returned by this function must be statically allocated or else there
 *   could be a memory-leak.<br>
 *   Noriko only supports path separators that are one character long. This should cover
 *   pretty much all platforms. The returned value can contain more characters which will
 *   be considered when building the path.
 */
NK_EXTERN NK_INLINE NK_VIRTUAL NkStringView const *NK_CALL __NkInt_StandardPaths_QueryNativeSeparator(NkVoid);
/**
 */
NK_EXTERN NK_VIRTUAL _Return_ok_ NkErrorCode NK_CALL __NkInt_StandardPaths_QueryPlatformLocs(NkStringView *stdLocs);
/**
 */
NK_EXTERN NK_VIRTUAL NkVoid NK_CALL __NkInt_StandardPaths_DestroyPlatformLocs(NkStringView *stdLocs);


/**
 */
NK_INTERNAL NK_INLINE NkBoolean NK_CALL __NkInt_Path_IsSeparator(_In_ char sepCh) {
    return sepCh == '/' || sepCh == '\\';
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(PathSrv)(NkVoid) {
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
            NK_ASSERT_EXTRA(NK_FALSE, NkErr_Unknown, "System paths initialized game paths. Unexpected behavior.");

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

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(PathSrv)(NkVoid) {
    /* First, destroy platform-dependent paths. */
    __NkInt_StandardPaths_DestroyPlatformLocs(gl_StandardPaths);
    /* Then, destroy the ones that are game-specific. */
    for (NkSize i = 0; i < NK_ARRAYSIZE(gl_c_GameLocs); i++) {
        NkStdLocation currLoc = gl_c_GameLocs[i];

        NkGPFree(gl_StandardPaths[currLoc].mp_dataPtr);
        gl_StandardPaths[currLoc].mp_dataPtr = NULL;
    }

    return NkErr_Ok;
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkPathBuild(
    _In_opt_ NkStringView const *sepCh,
    _In_opt_ NkStringView const **stemCompArr,
    _In_opt_ NkStringView const *fileName,
    _In_opt_ NkStringView const *extStr,
    _Out_    NkString *resStr
) {
    NK_ASSERT(resStr != NULL, NkErr_OutParameter);
    /* If there is nothing to build, do simply nothing. */
    if ((stemCompArr == NULL || *stemCompArr == NULL) && fileName == NULL && extStr == NULL)
        return NkErr_InParameter;

    /* Allocate memory for result string and prepare native separator. */
    NkErrorCode errCode = NkStringCreate(NULL, gl_c_DefPathSize, resStr);
    if (errCode != NkErr_Ok)
        return errCode;
    sepCh = sepCh != NULL ? sepCh : __NkInt_StandardPaths_QueryNativeSeparator();

    /*
     * Go through stem array and append all path components. Stop when we arrive at a
     * terminating NULL-pointer.
     */
    for (NkSize i = 0; stemCompArr != NULL && stemCompArr[i] != NULL; i++) {
        if ((errCode = NkStringJoin(resStr, stemCompArr[i]->mp_dataPtr, (NkUint32)(-1))) != NkErr_Ok)
            goto lbl_ONERROR;

        /* If we are not at the end yet, append a separator. */
        if (stemCompArr[i + 1] != NULL)
            if ((errCode = NkStringJoin(resStr, sepCh->mp_dataPtr, (NkUint32)(-1))) != NkErr_Ok)
                goto lbl_ONERROR;
    }

    /* Append the rest. */
    NkStringView const *tailStrs[] = {
        fileName != NULL && stemCompArr != NULL && stemCompArr[0] != NULL ? sepCh : NULL,
        fileName,
        extStr != NULL && fileName == NULL && stemCompArr != NULL && stemCompArr[0] != NULL ? sepCh : NULL,
        extStr == NULL ? NULL : NK_MAKE_STRING_VIEW_PTR("."),
        extStr
    };
    for (NkSize i = 0; i < NK_ARRAYSIZE(tailStrs); i++)
        if (tailStrs[i] != NULL) {
            errCode = NkStringJoin(resStr, tailStrs[i]->mp_dataPtr, (NkUint32)(-1));

            if (errCode != NkErr_Ok)
                goto lbl_ONERROR;
        }

    /* All good. */
    return NkErr_Ok;

lbl_ONERROR:
    NkStringDestroy(resStr);

    return errCode;
}

NkString *NK_CALL NkPathToNativeSeparators(_Inout_ NkString *strPtr) {
    NK_ASSERT(strPtr != NULL, NkErr_InOutParameter);

    /* Get native separator. */
    NkStringView const *nativeSep = __NkInt_StandardPaths_QueryNativeSeparator();

    /* Iterate over string, replacing all separators with the native one. */
    char *currCh = (char *)NkStringAt(strPtr, 0);
    do {
        /*
         * If we found a separator, replace it with the native separator character.
         * Separators are always ASCII characters, and UTF-8 is backwards-compatible with
         * ASCII, so we can simply replace them without having to worry about breaking
         * anything.
         */
        if (__NkInt_Path_IsSeparator(*currCh))
            *currCh = nativeSep->mp_dataPtr[0];
    }
    while ((currCh = (char *)NkStringIterate(currCh)) != NULL);

    /*
     * Simply return the input string again. Since this function does not change the
     * string properties, it cannot really "fail".
     */
    return strPtr;
}


NkStringView const *NK_CALL NkPathQueryStdLocation(_In_ NkStdLocation locId) {
    NK_ASSERT(locId >= 0 && locId < __NkStdLoc_Count__, NkErr_InParameter);

    return &gl_StandardPaths[locId];
}

NkStringView const *NK_CALL NkPathQueryGameDirectory(_In_ NkGameDirectory dirId) {
    NK_ASSERT(dirId >= 0 && dirId < __NkGameDir_Count__, NkErr_InParameter);

    /** \cond INTERNAL */
    /**
     */
    NK_INTERNAL NkStringView const gl_c_GameDirs[] = {
        [NkGameDir_BinRoot]       = NK_MAKE_STRING_VIEW("bin"),
        [NkGameDir_DataRoot]      = NK_MAKE_STRING_VIEW("data"),
        [NkGameDir_DocsRoot]      = NK_MAKE_STRING_VIEW("docs"),
        [NkGameDir_CacheRoot]     = NK_MAKE_STRING_VIEW("cache"),
        [NkGameDir_ExtRoot]       = NK_MAKE_STRING_VIEW("ext"),
        [NkGameDir_AssetRoot]     = NK_MAKE_STRING_VIEW("assets"),
        [NkGameDir_MapAssets]     = NK_MAKE_STRING_VIEW("maps"),
        [NkGameDir_TilesetAssets] = NK_MAKE_STRING_VIEW("tilesets")
    };
    NK_VERIFY_LUT(gl_c_GameDirs, NkGameDirectory, __NkGameDir_Count__);
    /** \endcond */

    return &gl_c_GameDirs[dirId];
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


/**
 */
NK_COMPONENT_DEFINE(PathSrv) {
    .m_compUuid     = { 0xe66604e9, 0xabd1, 0x4e6c, 0x822feb5a2b9e9624 },
    .mp_clsId       = NULL,
    .m_compIdent    = NK_MAKE_STRING_VIEW("path services"),
    .m_compFlags    = 0,
    .m_isNkOM       = NK_FALSE,

    .mp_fnQueryInst = NULL,
    .mp_fnStartup   = &NK_COMPONENT_STARTUPFN(PathSrv),
    .mp_fnShutdown  = &NK_COMPONENT_SHUTDOWNFN(PathSrv)
};


#undef NK_NAMESPACE


