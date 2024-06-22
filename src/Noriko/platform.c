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


_Return_ok_ NkErrorCode NK_CALL NkQueryPlatformInformation(_Inout_ NkPlatformInformation *platformInfoPtr) {
    /* Initialize static string views. */
    static NkStringView const gl_c_ProductNameAsView  = NK_MAKE_STRING_VIEW(NK_PRODUCT_NAME);
    static NkStringView const gl_c_ProdVerAsView      = NK_MAKE_STRING_VIEW(NK_PRODUCT_VERSION);
    static NkStringView const gl_c_ProdCpyAsView      = NK_MAKE_STRING_VIEW(NK_PRODUCT_COPYRIGHT);
    static NkStringView const gl_c_ProdConfigAsView   = NK_MAKE_STRING_VIEW(NK_PRODUCT_CONFIGURATION);
    static NkStringView const gl_c_ProdBToolsAsView   = NK_MAKE_STRING_VIEW(NK_PRODUCT_BTOOLS);
    static NkStringView const gl_c_ProdPlatformAsView = NK_MAKE_STRING_VIEW(NK_PRODUCT_PLATFORM);
    static NkStringView const gl_c_ProdInfoAsView     = NK_MAKE_STRING_VIEW(NK_PRODUCT_INFOSTR);
    static NkStringView const gl_c_ProdDateAsView     = NK_MAKE_STRING_VIEW(NK_PRODUCT_BUILDDATE);
    static NkStringView const gl_c_ProdTimeAsView     = NK_MAKE_STRING_VIEW(NK_PRODUCT_BUILDTIME);

    /* Parameter validation. */
    if (platformInfoPtr == NULL || platformInfoPtr->m_structSize == 0)
        return NkErr_InOutParameter;

    /* Calculate actual struct size. */
    size_t const c_actsize = min(sizeof(NkPlatformInformation), platformInfoPtr->m_structSize);

    /* Initialize basic *buf* structure with requested target and build information. */
    *platformInfoPtr = (struct NkPlatformInformation){
        .m_structSize       = c_actsize,
        .m_versionMajor     = NK_VER_MAJOR,
        .m_versionMinor     = NK_VER_MINOR,
        .m_versionPatch     = NK_VER_PATCH,
        .m_versionIteration = NK_VER_ITER,
        .m_platWidth        = NK_PRODUCT_ARCHITECTURE,
        .m_platBToolsVer    = NK_PRODUCT_BTOOLS_VER,
        .mp_prodName        = (NkStringView *)&gl_c_ProductNameAsView,
        .mp_prodVersion     = (NkStringView *)&gl_c_ProdVerAsView,
        .mp_prodCopyright   = (NkStringView *)&gl_c_ProdCpyAsView,
        .mp_prodConfig      = (NkStringView *)&gl_c_ProdConfigAsView,
        .mp_prodBuildTools  = (NkStringView *)&gl_c_ProdBToolsAsView,
        .mp_prodPlatform    = (NkStringView *)&gl_c_ProdPlatformAsView,
        .mp_prodFullInfoStr = (NkStringView *)&gl_c_ProdInfoAsView,
        .mp_buildDate       = (NkStringView *)&gl_c_ProdDateAsView,
        .mp_buildTime       = (NkStringView *)&gl_c_ProdTimeAsView
    };

    /* All good. */
    return NkErr_Ok;
}


