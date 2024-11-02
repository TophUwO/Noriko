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


/* Define IID and CLSID for NkIStandardPaths and NkIPathFactory. */
// { B9049E25-6A42-48B3-84E5-3D85EBD4217C }
NKOM_DEFINE_IID(NkIStandardPaths, { 0xb9049e25, 0x6a42, 0x48b3, 0x84e53d85ebd4217c });
// { 9DA2DEA4-1D4B-44C4-AB50-B06E91A23A95 }
NKOM_DEFINE_IID(NkIPathFactory, { 0x9da2dea4, 0x1d4b, 0x44c4, 0xab50b06e91a23a95 });

// { 51C11A7F-1F3C-4F2A-A179-778167D5746A }
NKOM_DEFINE_CLSID(NkIStandardPaths, { 0x51c11a7f, 0x1f3c, 0x4f2a, 0xa179778167d5746a });
// { 27509DCE-B0C4-4F2F-A4B6-1D95098B67C8 }
NKOM_DEFINE_CLSID(NkIPathFactory, { 0x27509dce, 0xb0c4, 0x4f2f, 0xa4b61d95098b67c8 });


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


