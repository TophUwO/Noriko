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
 * \file  winpath.c
 * \brief implements the path services for the Windows platform
 */
#define NK_NAMESPACE "nk::winpath"


/* Noriko includes */
#include <include/Noriko/path.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/noriko.h>
#include <include/Noriko/log.h>


/** \cond INTERNAL */
/**
 */
NK_INTERNAL KNOWNFOLDERID const *__NkInt_WindowsStandardPaths_MapToKNOWNFOLDERID(_In_ NkStdLocation locId) {
    NK_ASSERT(locId >= 0 && locId < __NkStdLoc_Count__, NkErr_InParameter);

    /**
     */
    NK_INTERNAL GUID const *gl_c_KnownFolderGUIDMap[] = {
        [NkStdLoc_Unknown]    = NULL,
        [NkStdLoc_SystemRoot] = &FOLDERID_Windows,
        [NkStdLoc_Desktop]    = &FOLDERID_Desktop,
        [NkStdLoc_Documents]  = &FOLDERID_Documents,
        [NkStdLoc_Home]       = &FOLDERID_Profile,
        [NkStdLoc_Music]      = &FOLDERID_Music,
        [NkStdLoc_Videos]     = &FOLDERID_Videos,
        [NkStdLoc_Downloads]  = &FOLDERID_Downloads,
        [NkStdLoc_Fonts]      = &FOLDERID_Fonts,
        [NkStdLoc_AppData]    = &FOLDERID_LocalAppData,
        [NkStdLoc_AppDir]     = &FOLDERID_ProgramFiles,
        [NkStdLoc_GameSaves]  = NULL,
        [NkStdLoc_GameRoot]   = NULL,
        [NkStdLoc_GameBin]    = NULL,
        [NkStdLoc_GameData]   = NULL,
        [NkStdLoc_GameDocs]   = NULL,
        [NkStdLoc_GameCache]  = NULL,
        [NkStdLoc_GameAssets] = NULL,
        [NkStdLoc_GameExt]    = NULL
    };
    NK_VERIFY_LUT(gl_c_KnownFolderGUIDMap, NkStdLocation, __NkStdLoc_Count__);

    return gl_c_KnownFolderGUIDMap[locId];
}
/** \endcond */


NkStringView const *NK_CALL __NkInt_StandardPaths_QueryNativeSeparator(NkVoid) {
    NK_INTERNAL NkStringView const gl_c_DefSep = NK_MAKE_STRING_VIEW("\\");

    return &gl_c_DefSep;
}

NkVoid NK_CALL __NkInt_StandardPaths_DestroyPlatformLocs(NkStringView *stdLocs) {
    for (NkStdLocation currLoc = NkStdLoc_Unknown + 1; currLoc < __NkStdLoc_Count__; currLoc++) {
        if (__NkInt_WindowsStandardPaths_MapToKNOWNFOLDERID(currLoc) == NULL)
            continue;
        NkStringView *currEntry = &stdLocs[currLoc];

        /* Destroy the path string. Passing NULL to NkGPFree() is a safe no-op. */
        NkGPFree(currEntry->mp_dataPtr);
        currEntry->mp_dataPtr = NULL;
    }
}

_Return_ok_ NkErrorCode NK_CALL __NkInt_StandardPaths_QueryPlatformLocs(NkStringView *stdLocs) {
    for (NkStdLocation currLoc = NkStdLoc_Unknown + 1; currLoc < __NkStdLoc_Count__; currLoc++) {
        NkStringView *currEntry = &stdLocs[currLoc];

        /*
         * First, check if it's a Windows known folder. If it is, retrieve the full path
         * and replace the placeholder.
         */
        GUID const *kfGuid = __NkInt_WindowsStandardPaths_MapToKNOWNFOLDERID(currLoc);
        if (kfGuid != NULL) {
            /* Retrieve standard folder path. */
            PWSTR stdLoc;
            char *stdLocUtf8;
            HRESULT hRes = SHGetKnownFolderPath(kfGuid, KF_FLAG_DEFAULT, NULL, &stdLoc);
            if (hRes != S_OK) {
                /* Failed to retrieve folder path. */
                __NkInt_StandardPaths_DestroyPlatformLocs(stdLocs);

                return NkErr_QueryStdLocation;
            }

            /* Get the UTF-8 buffer size, in bytes. This should not fail. */
            int reqBytes = WideCharToMultiByte(CP_UTF8, 0, stdLoc, -1, NULL, 0, NULL, NULL);
            NkErrorCode errCode = NkGPAlloc(NK_MAKE_ALLOCATION_CONTEXT(), (NkSize)reqBytes, 0, NK_FALSE, &stdLocUtf8);
            if (errCode != NkErr_Ok) {
                /* Failed to allocate memory for the string. */
                CoTaskMemFree((LPVOID)stdLoc);

                __NkInt_StandardPaths_DestroyPlatformLocs(stdLocs);
                return errCode;
            }
            /*
             * Convert the path to UTF-8. Should not fail either since the original
             * string was obtained by the system.
             */
            WideCharToMultiByte(CP_UTF8, 0, stdLoc, -1, (LPSTR)stdLocUtf8, reqBytes, NULL, NULL);

            /* Replace the string view in the static table. */
            *currEntry = (NkStringView){
                .mp_dataPtr    = stdLocUtf8,
                .m_sizeInBytes = (NkSize)reqBytes - 1
            };
            /* Finally, destroy the temporary string. */
            CoTaskMemFree((LPVOID)stdLoc);
            continue;
        }
    }

    return NkErr_Ok;
}


#undef NK_NAMESPACE


