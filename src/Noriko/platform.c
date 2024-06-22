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


/* stdlib includes */
#include <stdlib.h>
#include <string.h>

/* Noriko includes */
#include <include/Noriko/platform.h>


/**
 * \defgroup Noriko Version and Build Information
 * \brief    define and gather target platform and build information
 */
/** @{ */
#define NK_VER_MAJOR                         0
#define NK_VER_MINOR                         0
#define NK_VER_PATCH                         1
#define NK_VER_ITER                          1

#define NK_PRODUCT_NAME                      u8"Noriko"
#define NK_PRODUCT_COPYRIGHT                 u8"(c) TophUwO <tophuwo01@gmail.com>"
#if (defined NK_CONFIG_DEBUG)                
    #define NK_PRODUCT_CONFIGURATION         u8"Debug"
#elif (defined NK_CONFIG_DEBUG_OPTIMIZED)
    #define NK_PRODUCT_CONFIGURATION         u8"Debug Optimized"
#elif (defined NK_CONFIG_DEPLOY)             
    #define NK_PRODUCT_CONFIGURATION         u8"Deploy"
#endif                                       
#if (defined NK_TOOLCHAIN_MSVC)              
    #define NK_PRODUCT_BTOOLS                u8"Microsoft Visual C v" NK_MESC(NK_PRODUCT_BTOOLS_VER)
    #define NK_PRODUCT_BTOOLS_VER            _MSC_VER
#endif                                       
#if (defined NK_TARGET_WINDOWS)              
    #define NK_PRODUCT_PLATFORM              u8"Microsoft Windows"
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
#define NK_PRODUCT_VERSION                        \
    NK_MESC(NK_VER_MAJOR) "."                     \
    NK_MESC(NK_VER_MINOR) "."                     \
    NK_MESC(NK_VER_PATCH) "-" NK_MESC(NK_VER_ITER)

#define NK_PRODUCT_INFOSTR                        \
    NK_PRODUCT_NAME                  u8" "        \
    NK_PRODUCT_VERSION               u8" :: "     \
    NK_PRODUCT_PLATFORM              u8" ("       \
    NK_MESC(NK_PRODUCT_ARCHITECTURE) u8"-bit) - " \
    NK_PRODUCT_CONFIGURATION
/** @} */
/** @} */


_Return_ok_ NkErrorCode NK_CALL NkQueryPlatformInformation(_Inout_ NkPlatformInformation *const buf) {
    /* Parameter validation. */
    if (buf == NULL || buf->m_structSize == 0)
        return NkErr_InOutParameter;

    /* Calculate actual struct size. */
    size_t const c_actsize = min(sizeof(NkPlatformInformation), buf->m_structSize);

    /* Initialize basic *buf* structure with requested target and build information. */
    *buf = (struct NkPlatformInformation){
        .m_structSize       = c_actsize,
        .m_versionMajor     = NK_VER_MAJOR,
        .m_versionMinor     = NK_VER_MINOR,
        .m_versionPatch     = NK_VER_PATCH,
        .m_versionIteration = NK_VER_ITER,
        .m_platWidth        = NK_PRODUCT_ARCHITECTURE,
        .m_platBToolsVer    = NK_PRODUCT_BTOOLS_VER,
        .mp_prodName        = NK_PRODUCT_NAME,
        .mp_prodVersion     = NK_PRODUCT_VERSION,
        .mp_prodCopyright   = NK_PRODUCT_COPYRIGHT,
        .mp_prodConfig      = NK_PRODUCT_CONFIGURATION,
        .mp_prodBuildTools  = NK_PRODUCT_BTOOLS,
        .mp_prodPlatform    = NK_PRODUCT_PLATFORM,
        .mp_prodFullInfoStr = NK_PRODUCT_INFOSTR,
        .mp_buildDate       = NK_PRODUCT_BUILDDATE,
        .mp_buildTime       = NK_PRODUCT_BUILDTIME
    };

    /* All good. */
    return NkErr_Ok;
}


