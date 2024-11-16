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
#define NK_NAMESPACE "nk::window"


/* Noriko includes */
#include <include/Noriko/window.h>
#include <include/Noriko/noriko.h>
#include <include/Noriko/comp.h>


/** \cond INTERNAL */
/* Define IID and CLSID. */
// { D9DD03B3-536E-410E-8BA5-DABF915A6AB5 }
NKOM_DEFINE_IID(NkIWindow, { 0xd9dd03b3, 0x536e, 0x410e, 0x8ba5dabf915a6ab5 });
// { 48BEF13B-9DE9-4B23-A5EC-B6AD6A81431B }
NKOM_DEFINE_CLSID(NkIWindow, { 0x48bef13b, 0x9de9, 0x4b23, 0xa5ecb6ad6a81431b });


/**
 * \ingroup VirtFn 
 */
NK_EXTERN NK_VIRTUAL _Return_ok_ NkErrorCode NK_CALL __NkVirt_Window_Startup(NkVoid);
/**
 * \ingroup VirtFn 
 */
NK_EXTERN NK_VIRTUAL _Return_ok_ NkErrorCode NK_CALL __NkVirt_Window_Shutdown(NkVoid);
/**
 * \ingroup VirtFn
 * \brief   queries the static window instance
 * \return  pointer to the static window instance
 * \note    Before using the window instance obtained by this function for the first
 *          time, start up the window component.
 * 
 * \par Remarks
 *   Like with all functions that return an NkOM object, they increment the reference
 *   count of the returned object.
 */
NK_EXTERN NK_VIRTUAL NkIBase *NK_CALL __NkVirt_Window_QueryInstance(NkVoid);


/**
 */
_Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(Window)(NkVoid) {
    /* Query application specification for window specification. */
    NkApplicationSpecification const *appSpecs = NkApplicationQuerySpecification();
    
    /* Initialize the window. */
    NkIWindow *wndRef = __NkVirt_Window_QueryInstance();
    return wndRef->VT->Initialize(wndRef, &(NkWindowSpecification){
        .m_structSize      = sizeof(NkWindowSpecification),
        .m_rendererApi     = appSpecs->m_rendererApi,
        .m_vpAlignment     = appSpecs->m_vpAlignment,
        .m_isVSync         = appSpecs->m_isVSync,
        .m_vpExtents       = appSpecs->m_vpExtents,
        .m_dispTileSize    = appSpecs->m_dispTileSize,
        .m_allowedWndModes = appSpecs->m_allowedWndModes,
        .m_initialWndMode  = appSpecs->m_initialWndMode,
        .m_wndFlags        = appSpecs->m_wndFlags | NkWndFlag_MainWindow,
        .mp_nativeHandle   = appSpecs->mp_nativeHandle,
        .m_wndUuid         = { 0xf5ef2c4c, 0x6a5e, 0x4719, 0x9f3f0412dbbaf611 },
        .m_wndTitle        = appSpecs->m_wndTitle
    });
}

/**
 */
_Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(Window)(NkVoid) {
    /*
     * The default destroy handler of the window simply hides it so that we can wait for
     * the WM_QUIT message to destroy everything. Once that message is received, we leave
     * the main loop and destroy every component in the reverse order they were
     * initialized. So we need to destroy the window here.
     */
    NkIWindow *wndRef = __NkVirt_Window_QueryInstance();

    DestroyWindow(wndRef->VT->QueryNativeWindowHandle(wndRef));
    return NkErr_Ok;
}
/** \endcond */


NkStringView const *NK_CALL NkWindowGetModeStr(_In_ NkWindowMode wndMode) {
    NK_ASSERT(wndMode >= 0 && NK_ISBITFLAG(wndMode) && wndMode < __NkWndMode_Count__, NkErr_InParameter);

    /** \cond INTERNAL */
    /**
     * \brief maps numeric window mode IDs to their string representation (used for
     *        logging, etc.) 
     */
    NK_INTERNAL NkStringView const gl_WindowModeStrs[] = {
        [NkWndMode_Unknown]    = NK_MAKE_STRING_VIEW(NK_ESC(NkWndMode_Unknown)),
        [NkWndMode_Minimized]  = NK_MAKE_STRING_VIEW(NK_ESC(NkWndMode_Minimized)),
        [NkWndMode_Maximized]  = NK_MAKE_STRING_VIEW(NK_ESC(NkWndMode_Maximized)),
        [NkWndMode_Fullscreen] = NK_MAKE_STRING_VIEW(NK_ESC(NkWndMode_Fullscreen)),
        [NkWndMode_Normal]     = NK_MAKE_STRING_VIEW(NK_ESC(NkWndMode_Normal)),
        [NkWndMode_Visible]    = NK_MAKE_STRING_VIEW(NK_ESC(NkWndMode_Visible)),
        [NkWndMode_Hidden]     = NK_MAKE_STRING_VIEW(NK_ESC(NkWndMode_Hidden)),
        [NkWndMode_Default]    = NK_MAKE_STRING_VIEW(NK_ESC(NkWndMode_Default)),
        [NkWndMode_All]        = NK_MAKE_STRING_VIEW(NK_ESC(NkWndMode_All))
    };
    /** \endcond */

    return &gl_WindowModeStrs[wndMode];
}

NkStringView const *NK_CALL NkWindowGetFlagStr(_In_ NkWindowFlags wndFlag) {
    NK_ASSERT(wndFlag >= 0 && NK_ISBITFLAG(wndFlag) && wndFlag < __NkWndFlag_Count__, NkErr_InParameter);

    /** \cond INTERNAL */
    NK_INTERNAL NkStringView const gl_WindowFlagStrs[] = {
        [NkWndFlag_Default]        = NK_MAKE_STRING_VIEW(NK_ESC(NkWndFlag_Default)),
        [NkWndFlag_MessageOnlyWnd] = NK_MAKE_STRING_VIEW(NK_ESC(NkWndFlag_MessageOnlyWnd)),
        [NkWndFlag_AlwaysOnTop]    = NK_MAKE_STRING_VIEW(NK_ESC(NkWndFlag_AlwaysOnTop)),
        [NkWndFlag_MainWindow]     = NK_MAKE_STRING_VIEW(NK_ESC(NkWndFlag_MainWindow)),
        [NkWndFlag_DragResizable]  = NK_MAKE_STRING_VIEW(NK_ESC(NkWndFlag_DragResizable)),
        [NkWndFlag_DragMovable]    = NK_MAKE_STRING_VIEW(NK_ESC(NkWndFlag_DragMovable))
    };
    /** \endcond */

    return &gl_WindowFlagStrs[wndFlag];
}

NkStringView const *NK_CALL NkWindowGetViewportAlignmentStr(_In_ NkViewportAlignment vpAlignment) {
    NK_ASSERT(vpAlignment > 0 && NK_ISBITFLAG(vpAlignment) && vpAlignment < __NkVpAlign_Count__, NkErr_InParameter);

    /** \cond INTERNAL */
    NK_INTERNAL NkStringView const gl_ViewportAlignmentStrs[] = {
        [NkVpAlign_Top]     = NK_MAKE_STRING_VIEW(NK_ESC(NkVpAlign_Top)),
        [NkVpAlign_VCenter] = NK_MAKE_STRING_VIEW(NK_ESC(NkVpAlign_VCenter)),
        [NkVpAlign_Bottom]  = NK_MAKE_STRING_VIEW(NK_ESC(NkVpAlign_Bottom)),
        [NkVpAlign_Left]    = NK_MAKE_STRING_VIEW(NK_ESC(NkVpAlign_Left)),
        [NkVpAlign_HCenter] = NK_MAKE_STRING_VIEW(NK_ESC(NkVpAlign_HCenter)),
        [NkVpAlign_Right]   = NK_MAKE_STRING_VIEW(NK_ESC(NkVpAlign_Right))
    };
    /** \endcond */

    return &gl_ViewportAlignmentStrs[vpAlignment];
}

NkEventType NK_CALL NkWindowMapEventTypeFromWindowMode(_In_ NkWindowMode wndMode) {
    NK_ASSERT(wndMode >= 0 && NK_ISBITFLAG(wndMode) && wndMode < __NkWndMode_Count__, NkErr_InParameter);

    return ((NkEventType const [__NkWndMode_Count__]){
        [NkWndMode_Normal]     = NkEv_WindowRestored,
        [NkWndMode_Maximized]  = NkEv_WindowMaximized,
        [NkWndMode_Minimized]  = NkEv_WindowMinimized,
        [NkWndMode_Fullscreen] = NkEv_WindowFullscreen
    })[wndMode];
}


/** \cond INTERNAL */
/**
 * \brief global info instance of the \c Window component
 */
NK_COMPONENT_DEFINE(Window) {
    .m_compUuid     = { 0x427e1403, 0x8a3f, 0x4c77, 0x983b7ce26bb2a4f5 },
    .mp_clsId       = NKOM_CLSIDOF(NkIWindow),
    .m_compIdent    = NK_MAKE_STRING_VIEW("window"),
    .m_compFlags    = 0,
    .m_isNkOM       = NK_TRUE,

    .mp_fnQueryInst = &__NkVirt_Window_QueryInstance,
    .mp_fnStartup   = &NK_COMPONENT_STARTUPFN(Window),
    .mp_fnShutdown  = &NK_COMPONENT_SHUTDOWNFN(Window)
};
/** \endcond */


#undef NK_NAMESPACE


