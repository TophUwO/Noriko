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

/* stdlib includes */
#if (defined NK_TARGET_MULTITHREADED)
    #include <threads.h>
#endif

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>


/* Determine build tools. */
#if (defined _MSC_VER)
    #define NK_TOOLCHAIN_MSVC
#else
    #error Currently, Noriko only supports compilation via MSVC.
#endif

/* Determine target platform. */
#if (defined _WIN32) || (defined _WIN64)
    #define NK_TARGET_WINDOWS

    /* Enable memory leak detection of Microsoft Visual Studio in debug builds. */
    #if (defined NK_TOOLCHAIN_MSVC)
        #if (!defined NK_EDITOR)
            #define _CRTDBG_MAP_ALLOC
        #endif
        #define NK_USE_MSVC_MEMORY_LEAK_DETECTOR

        #define _CRT_RAND_S /* use Microsoft-specific "rand_s()" function */
        #include <stdlib.h>
        #if (!defined NK_EDITOR)
            #include <crtdbg.h>
        #endif
    #endif

    /* Suppress some non-important warnings. */
    #pragma warning (disable: 4668) /* macro not defined; replacing with '0' */
    #pragma warning (disable: 5045) /* spectre migitation for memory loads */
    #pragma warning (disable: 4061) /* enumerator not explicitly handled in switch-case */
    #pragma warning (disable: 4062) /* enumerator not handled in switch-case */
    #pragma warning (disable: 4065) /* switch-case contains default but no case labels */
    #pragma warning (disable: 4710) /* function not inlined */
    #pragma warning (disable: 4711) /* function selected for inline expansion */
    #pragma warning (disable: 4115) /* named type definition in parentheses */
    #pragma warning (disable: 4255) /* no function prototype; convert () to (void) */
    /*
     * Disable some warnings in deploy builds that are not substantial but may be
     * important when debugging.
     */
    #if (defined NK_CONFIG_DEPLOY)
        #pragma warning (disable: 4820) /* padding added after data member in struct */
    #endif

    /* Include windows headers. */
    #include <windows.h>
    #include <dwmapi.h>
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


/* Make some basic assumptions. */
static_assert(sizeof(float)  == 4, "sizeof(float) must be 4 (four) bytes.");
static_assert(sizeof(int)    == 4, "sizeof(int) must be 4 (four) bytes.");
static_assert(sizeof(double) == 8, "sizeof(double) must be 4 (four) bytes.");
static_assert(sizeof(char)   == 1, "sizeof(char) must be 1 (one) byte.");


/**
 * \struct NkPlatformInformation
 * \brief  target platform and build information
 * 
 * This structure can be used to query target platform properties as well as
 * Noriko build properties.
 * 
 * \note   All pointers that are part of this data-structure are pointers to static
 *         read-only memory. Thus, writing to the passed addresses is undefined behavior.
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkPlatformInformation {
    NkSize       m_structSize;       /**< size of this structure, in bytes */
    NkInt32      m_versionMajor;     /**< engine major version component */
    NkInt32      m_versionMinor;     /**< engine minor version component */
    NkInt32      m_versionPatch;     /**< engine patch version component */
    NkInt32      m_versionIteration; /**< engine patch iteration component */
    NkInt32      m_platWidth;        /**< target platform width */
    NkInt32      m_platBToolsVer;    /**< version of build tools used */
    NkStringView m_prodName;         /**< name of engine component */
    NkStringView m_prodVersion;      /**< full engine version string */
    NkStringView m_prodCopyright;    /**< engine copyright string */
    NkStringView m_prodConfig;       /**< engine build configuration */
    NkStringView m_prodBuildTools;   /**< compiler/build tools used for compilation */
    NkStringView m_prodPlatform;     /**< engine target platform ID */
    NkStringView m_prodFullInfoStr;  /**< full target information string */
    NkStringView m_buildDate;        /**< local build date (<Month> <Day> <Year>) */
    NkStringView m_buildTime;        /**< local build time (HH:MM:SS) */
} NkPlatformInformation;

/**
 * \brief   queries target platform and build information
 * \param   [in,out] pfInfoPtr buffer the target information is written to
 * \note    If \c pfInfoPtr is \c NULL or \c pfInfoPtr is improperly initialized,
 *          the function fails.
 * \warning Before you run this function, initialize the \c m_structSize member variable
 *          of \c pfInfoPtr to the size of the buffer that is being passed to the
 *          function. To do this, use <tt>sizeof(NkPlatformInformation)</tt>.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkQueryPlatformInformation(_Inout_ NkPlatformInformation *pfInfoPtr);


