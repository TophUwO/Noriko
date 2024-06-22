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
 * \file  platform.h
 * \brief Noriko's platform and feature detection
 *
 * This header defines macros that can be used to do basic as well as advanced platform
 * and feature detection.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>


/* Determine target platform. */
#if (defined _WIN32) || (defined _WIN64)
    #define NK_TARGET_WINDOWS
#else
    #error Noriko's engine component currently only supports Microsoft Windows.
#endif

/* Determine target platform architecture. */
#if (defined _WIN64)
    #define NK_ARCHITECTURE_64BIT
#elif (defined _WIN32)
    #define NK_ARCHITECTURE_32BIT
#else
    #error Unknown target platform architecture.
#endif

/* Determine build tools. */
#if (defined _MSC_VER)
    #define NK_TOOLCHAIN_MSVC
#else
    #error Currently, Noriko only supports compilation via MSVC.
#endif


/**
 * \struct NkPlatformInformation
 * \brief  target platform and build information
 * 
 * This structure can be used to query target platform properties as well as
 * Noriko build properties.
 * 
 * \note   All pointer fields within this function are pointers to static
 *         read-only memory. Thus, writing to the passed addresses is undefined behavior.
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkPlatformInformation {
    size_t        m_structSize;       /**< size of this structure, in bytes */
    size_t        m_versionMajor;     /**< engine major version component */
    size_t        m_versionMinor;     /**< engine minor version component */
    size_t        m_versionPatch;     /**< engine patch version component */
    size_t        m_versionIteration; /**< engine patch iteration component */
    size_t        m_platWidth;        /**< target platform width */
    size_t        m_platBToolsVer;    /**< version of build tools used */
    NkStringView *mp_prodName;        /**< name of engine component */
    NkStringView *mp_prodVersion;     /**< full engine version string */
    NkStringView *mp_prodCopyright;   /**< engine copyright string */
    NkStringView *mp_prodConfig;      /**< engine build configuration */
    NkStringView *mp_prodBuildTools;  /**< compiler/build tools used for compilation */
    NkStringView *mp_prodPlatform;    /**< engine target platform ID */
    NkStringView *mp_prodFullInfoStr; /**< full target information string */
    NkStringView *mp_buildDate;       /**< local build date (<Month> <Day> <Year>) */
    NkStringView *mp_buildTime;       /**< local build time (HH:MM:SS) */
} NkPlatformInformation;

/**
 * \brief   query target platform and build information
 * \param   [in,out] platformInfoPtr buffer the target information is written to
 * \return  *NkErr_Ok* on success; non-zero if the target platform could not be retrieved
 * \note    If *platformInfoPtr* is *NULL* or *platformInfoPtr* is improperly initialized,
 *          the function fails.
 * \warning Before you run this function, initialize the *m_structSize* member variable of
 *          *platformInfoPtr* to the size of the buffer that is being passed to the function.
 *          To do this, use *sizeof(NkPlatformInformation)*.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NkQueryPlatformInformation(_Inout_ NkPlatformInformation *platformInfoPtr);


