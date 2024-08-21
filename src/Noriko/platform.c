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
 * \file  platform.c
 * \brief implementation of Noriko's platform and feature detection
 */
#define NK_NAMESPACE "nk::platform"


/* stdlib includes */
#include <stdlib.h>
#include <string.h>

/* Noriko includes */
#include <include/Noriko/platform.h>
#include <include/Noriko/error.h>


/**
 * \defgroup Noriko Version and Build Information
 * \brief    define and gather target platform and build information
 */
/** @{ */
#define NK_VER_MAJOR                         0
#define NK_VER_MINOR                         0
#define NK_VER_PATCH                         1
#define NK_VER_ITER                          1

#define NK_PRODUCT_NAME                      "Noriko"
#define NK_PRODUCT_COPYRIGHT                 "(c) 2024 TophUwO <tophuwo01@gmail.com>. All rights reserved."
#if (defined NK_CONFIG_DEBUG)                
    #define NK_PRODUCT_CONFIGURATION         "Debug"
#elif (defined NK_CONFIG_DEBUG_OPTIMIZED)
    #define NK_PRODUCT_CONFIGURATION         "Debug Optimized"
#elif (defined NK_CONFIG_DEPLOY)             
    #define NK_PRODUCT_CONFIGURATION         "Deploy"
#endif                                       
#if (defined NK_TOOLCHAIN_MSVC)              
    #define NK_PRODUCT_BTOOLS                "Microsoft Visual C v" NK_MESC(NK_PRODUCT_BTOOLS_VER)
    #define NK_PRODUCT_BTOOLS_VER            _MSC_VER
#endif                                       
#if (defined NK_TARGET_WINDOWS)              
    #define NK_PRODUCT_PLATFORM              "Microsoft Windows"
#endif                                       
#if (defined NK_ARCHITECTURE_64BIT)          
    #define NK_PRODUCT_ARCHITECTURE          64
#elif (defined NK_ARCHITECTURE_32BIT)        
    #define NK_PRODUCT_ARCHITECTURE          32
#endif
#define NK_PRODUCT_BUILDDATE                 __DATE__
#define NK_PRODUCT_BUILDTIME                 __TIME__

/**
 * \defgroup Composite Information Strings
 * \brief    information strings composed from other atomic information strings
 */
/** @{ */
#define NK_PRODUCT_VERSION                         \
    NK_MESC(NK_VER_MAJOR) "."                      \
    NK_MESC(NK_VER_MINOR) "."                      \
    NK_MESC(NK_VER_PATCH) "-" NK_MESC(NK_VER_ITER)

#define NK_PRODUCT_INFOSTR                         \
    NK_PRODUCT_NAME                    " "         \
    NK_PRODUCT_VERSION                 " :: "      \
    NK_PRODUCT_PLATFORM                " ("        \
    NK_MESC(NK_PRODUCT_ARCHITECTURE)   "-bit) - "  \
    NK_PRODUCT_CONFIGURATION
/** @} */
/** @} */


NkVoid NK_CALL NkQueryPlatformInformation(_Inout_ NkPlatformInformation *platformInfoPtr) {
    /* Parameter validation. */
    NK_ASSERT(platformInfoPtr != NULL && platformInfoPtr->m_structSize ^ 0, NkErr_InOutParameter);

    /**
     * \brief represents the static platform information for the current target
     */
    NK_INTERNAL NkPlatformInformation const gl_PlatformInfo = {
        .m_structSize       = sizeof gl_PlatformInfo,
        .m_versionMajor     = NK_VER_MAJOR,
        .m_versionMinor     = NK_VER_MINOR,
        .m_versionPatch     = NK_VER_PATCH,
        .m_versionIteration = NK_VER_ITER,
        .m_platWidth        = NK_PRODUCT_ARCHITECTURE,
        .m_platBToolsVer    = NK_PRODUCT_BTOOLS_VER,
        .m_prodName         = NK_MAKE_STRING_VIEW(NK_PRODUCT_NAME),
        .m_prodVersion      = NK_MAKE_STRING_VIEW(NK_PRODUCT_VERSION),
        .m_prodCopyright    = NK_MAKE_STRING_VIEW(NK_PRODUCT_COPYRIGHT),
        .m_prodConfig       = NK_MAKE_STRING_VIEW(NK_PRODUCT_CONFIGURATION),
        .m_prodBuildTools   = NK_MAKE_STRING_VIEW(NK_PRODUCT_BTOOLS),
        .m_prodPlatform     = NK_MAKE_STRING_VIEW(NK_PRODUCT_PLATFORM),
        .m_prodFullInfoStr  = NK_MAKE_STRING_VIEW(NK_PRODUCT_INFOSTR),
        .m_buildDate        = NK_MAKE_STRING_VIEW(NK_PRODUCT_BUILDDATE),
        .m_buildTime        = NK_MAKE_STRING_VIEW(NK_PRODUCT_BUILDTIME)
    };
    /* Calculate actual struct size. */
    NkSize const actSize = NK_MIN(sizeof(NkPlatformInformation), platformInfoPtr->m_structSize);

    /* Initialize basic *buf* structure with requested target and build information. */
    memcpy(platformInfoPtr, &gl_PlatformInfo, actSize);
    platformInfoPtr->m_structSize = actSize;
}


#undef NK_NAMESPACE


