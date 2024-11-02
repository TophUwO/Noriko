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
NK_INTERNAL NkStringView gl_WinStandardPaths[] = {
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
NK_VERIFY_LUT(gl_WinStandardPaths, NkStdLocation, __NkStdLoc_Count__);


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

    return gl_c_KnownFolderGUIDMap[locId];
}

/**
 */
NK_INTERNAL NkVoid __NkInt_WindowsStandardPaths_DestroyStdLocs(NkVoid) {
    for (NkStdLocation currLoc = NkStdLoc_Unknown + 1; currLoc < __NkStdLoc_Count__; currLoc++) {
        NkStringView *currEntry = &gl_WinStandardPaths[currLoc];

        /* Destroy the path string. Passing NULL to NkGPFree() is a safe no-op. */
        NkGPFree(currEntry->mp_dataPtr);
        currEntry->mp_dataPtr = NULL;
    }
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_WindowsStandardPaths_QueryStdLocs(NkVoid) {
    /* Query game root directory. */
    NkStringView const *gameRootDir = &NkApplicationQuerySpecification()->m_gameRootDir;

    for (NkStdLocation currLoc = NkStdLoc_Unknown + 1; currLoc < __NkStdLoc_Count__; currLoc++) {
        NkStringView *currEntry = &gl_WinStandardPaths[currLoc];

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
                __NkInt_WindowsStandardPaths_DestroyStdLocs();

                return NkErr_QueryStdLocation;
            }

            /* Get the UTF-8 buffer size, in bytes. This should not fail. */
            int reqBytes = WideCharToMultiByte(CP_UTF8, 0, stdLoc, -1, NULL, 0, NULL, NULL);
            NkErrorCode errCode = NkGPAlloc(NK_MAKE_ALLOCATION_CONTEXT(), (NkSize)reqBytes, 0, NK_FALSE, &stdLocUtf8);
            if (errCode != NkErr_Ok) {
                /* Failed to allocate memory for the string. */
                CoTaskMemFree((LPVOID)stdLoc);

                __NkInt_WindowsStandardPaths_DestroyStdLocs();
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

        /*
         * Okay, so it wasn't a windows known folder, it must be related to the game's
         * directory in that case.
         */
        switch (currLoc) {
            case NkStdLoc_GameRoot: *currEntry = *gameRootDir; break;
            default:
                NK_LOG_WARNING(
                    "There is currently no standard path available for '%s'.",
                    NkPathQueryStandardLocIdStr(currLoc)->mp_dataPtr
                );
        }
    }

    return NkErr_Ok;
}


/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WindowsStandardPaths_AddRef(_Inout_ NkIStandardPaths *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WindowsStandardPaths_Release(_Inout_ NkIStandardPaths *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WindowsStandardPaths_QueryInterface(
    _Inout_  NkIStandardPaths *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    if (NkUuidIsEqual(iId, NKOM_IIDOF(NkIBase)) || NkUuidIsEqual(iId, NKOM_IIDOF(NkIStandardPaths))) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_WindowsStandardPaths_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface not implemented. */
    return NkErr_InterfaceNotImpl;
}

/**
 */
NK_INTERNAL NkStringView const *NK_CALL __NkInt_WindowsStandardPaths_QueryStdLocation(
    _Inout_ NkIStandardPaths *self,
    _In_    NkStdLocation locId
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(locId >= NkStdLoc_Unknown && locId < __NkStdLoc_Count__, NkErr_InParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Get result. If the result is an empty string view, return NULL instead. */
    return gl_WinStandardPaths[locId].mp_dataPtr != NULL ? &gl_WinStandardPaths[locId] : NULL;
}


/**
 * \brief actual Windows NkIStandardPaths instance
 * 
 * Since none of the methods actually change any state, we can simply omit all state
 * and declare the class as a simple VTable instance.
 */
NK_INTERNAL NkIStandardPaths const gl_c_WinStandardPaths = {
    .VT = &(struct __NkIStandardPaths_VTable__){
        .QueryInterface   = &__NkInt_WindowsStandardPaths_QueryInterface,
        .AddRef           = &__NkInt_WindowsStandardPaths_AddRef,
        .Release          = &__NkInt_WindowsStandardPaths_Release,
        .QueryStdLocation = &__NkInt_WindowsStandardPaths_QueryStdLocation
    }
};
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkPathStartup(NkVoid) {
    NK_LOG_INFO("startup: path services");

    /* Initialize standard location strings. */
    return __NkInt_WindowsStandardPaths_QueryStdLocs();
}

_Return_ok_ NkErrorCode NK_CALL NkPathShutdown(NkVoid) {
    NK_LOG_INFO("shutdown: path services");

    /* Destroy standard location strings. */
    __NkInt_WindowsStandardPaths_DestroyStdLocs();
    return NkErr_Ok;
}


#undef NK_NAMESPACE


