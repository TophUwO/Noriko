/**********************************************************************
 * Noriko   - cross-platform 2-D role-playing game (RPG) game engine  *
 *            for desktop and mobile console platforms                *
 * NorikoRt - game launcher and runtime component of the Noriko game  *
 *            engine ecosystem                                        *
 *                                                                    *
 * (c) 2024 TophUwO <tophuwo01@gmail.com>. All rights reserved.       *
 *                                                                    *
 * The source code is licensed under the Apache License 2.0. Refer    *
 * to the LICENSE file in the root directory of this project. If this *
 * file is not present, visit                                         *
 *     https://www.apache.org/licenses/LICENSE-2.0                    *
 **********************************************************************/

/**
 * \file  window.c
 * \brief implements the platform-independent window system functions
 */


/* Noriko includes */
#include <include/Noriko/window.h>
#include <include/Noriko/noriko.h>


/** \cond INTERNAL */
/**
 * \brief  retrieves the instance of the internal platform window instance
 * 
 * Platform-dependent implementations of the \c NkIWindow interface are found in
 * different locations throughout Noriko's source code. Every file implementing a
 * platform window must export a function with the below signature. Of course, as per the
 * rules for external linkage, only one of these exported functions can be present at a
 * time. Implementations of windows which are not for the current platform, therefore,
 * should be completely stripped from the build.
 * 
 * \return pointer to the static window instance (never <tt>NULL</tt>)
 */
NK_NATIVE NK_EXTERN NkIWindow *NK_CALL __NkInt_WindowQueryPlatformInstance(NkVoid);
/** \endcond */

/* Define IID and CLSID. */
// { D9DD03B3-536E-410E-8BA5-DABF915A6AB5 }
NKOM_DEFINE_IID(NkIWindow, { 0xd9dd03b3, 0x536e, 0x410e, 0x8ba5dabf915a6ab5 });
// { 48BEF13B-9DE9-4B23-A5EC-B6AD6A81431B }
NKOM_DEFINE_CLSID(NkIWindow, { 0x48bef13b, 0x9de9, 0x4b23, 0xa5ecb6ad6a81431b });


_Return_ok_ NkErrorCode NK_CALL NkWindowStartup(NkVoid) {
    NK_LOG_INFO("startup: window");

    /* Query application specification for window specification. */
    NkApplicationSpecification const *appSpecs = NkApplicationQuerySpecification();
    
    /* Initialize the window. */
    NkIWindow *wndRef = NkWindowQueryInstance();
    return wndRef->VT->Initialize(wndRef, &(NkWindowSpecification const){
        .m_structSize      = sizeof(NkWindowSpecification),
        .m_vpAlignment     = appSpecs->m_vpAlignment,
        .m_vpExtents       = appSpecs->m_vpExtents,
        .m_glTileSize      = appSpecs->m_glTileSize,
        .m_allowedWndModes = appSpecs->m_allowedWndModes,
        .m_initialWndMode  = appSpecs->m_initialWndMode,
        .m_wndFlags        = appSpecs->m_wndFlags | NkWndFlag_MainWindow,
        .mp_nativeHandle   = appSpecs->mp_nativeHandle,
        .m_wndIdent        = NK_MAKE_STRING_VIEW("NkWnd_MainWindow"),
        .m_wndTitle        = appSpecs->m_wndTitle,
        .m_wndPos          = appSpecs->m_wndPos
    });
}

_Return_ok_ NkErrorCode NK_CALL NkWindowShutdown(NkVoid) {
    NK_LOG_INFO("shutdown: window");

    /*
     * Windows are uninitialized by themselves. Therefore, there is no need for an
     * external uninit function. This function, thus, exists merely for formality reasons
     * and is not expected to ever do anything.
     */
    return NkErr_Ok;
}


NkIWindow *NK_CALL NkWindowQueryInstance(NkVoid) {
    /* The return value can never be NULL. */
    return __NkInt_WindowQueryPlatformInstance();
}


