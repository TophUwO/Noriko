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
 * \file  window.h
 * \brief defines the public (platform-independent) window and window manager API
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>
#include <include/Noriko/nkom.h>


/**
 * \typedef NkNativeWindowHandle
 * \brief   represents a handle to an underlying platform window
 */
NK_NATIVE typedef NkVoid *NkNativeWindowHandle;


/**
 * \enum  NkWindowMode
 * \brief represents the window modes a platform window can support
 */
NK_NATIVE typedef enum NkWindowMode {
    NkWndMode_Minimized  = 1 << 0, /**< minimized window mode (taskbar icon still visible) */
    NkWndMode_Maximized  = 1 << 1, /**< maximized window mode (title bar still visible) */
    NkWndMode_Fullscreen = 1 << 2, /**< full-screen window mode (desktop invisible) */
    NkWndMode_Normal     = 1 << 3, /**< normal (= 'windowed') window mode */
    NkWndMode_Visible    = 1 << 4, /**< window is visible */
    NkWndMode_Hidden     = 1 << 5, /**< window is hidden (taskbar icon not visible) */

    /**
     * \brief default window mode
     */
    NkWndMode_Default    = NkWndMode_Normal | NkWndMode_Visible,
    /**
     * \brief all valid window modes combined
     */
    NkWndMode_All        = NkWndMode_Minimized | NkWndMode_Maximized
                            | NkWndMode_Fullscreen | NkWndMode_Normal
                            | NkWndMode_Visible | NkWndMode_Hidden,
    __NkWndMode_Count__ /**< *only used internally* */
} NkWindowMode;

/**
 * \brief  
 */
NK_NATIVE typedef enum NkWindowFlags {
    NkWndFlag_Default = 0,

    NkWndFlag_MessageOnlyWnd = 1 << 0,
    NkWndFlag_AlwaysOnTop    = 1 << 1,

    __NkWndFlag_Count__
} NkWindowFlags;

/**
 */
NK_NATIVE typedef enum NkViewportAlignment {
    NkVpAlign_Top     = 1 << 0,
    NkVpAlign_VCenter = 1 << 1,
    NkVpAlign_Bottom  = 1 << 2,
    NkVpAlign_Left    = 1 << 3,
    NkVpAlign_HCenter = 1 << 4,
    NkVpAlign_Right   = 1 << 5,

    NkVpAlign_Default = NkVpAlign_HCenter | NkVpAlign_VCenter,
    __NkVpAlign_Count__
} NkViewportAlignment;

/**
 */
NK_NATIVE typedef struct NkWindowSpecification {
    NkSize                 m_structSize;      /**< size of this structure, in bytes */
    NkViewportAlignment    m_vpAlignment;     /**< viewport alignment inside the main window */
    NkBoolean              m_isMainWnd;       /**< whether or not the window is supposed to be the main window */
    NkSize2D               m_vpExtents;       /**< size in tiles of the main window viewport */
    NkSize2D               m_glTileSize;      /**< global tile size of the main window viewport */
    NkWindowMode           m_allowedWndModes; /**< allowed window modes for the main window */
    NkWindowMode           m_initialWndMode;  /**< initial window mode for the main window */
    NkWindowFlags          m_wndFlags;        /**< additional (platform-dependent) window flags */
    NkNativeWindowHandle   mp_nativeHandle;   /**< optional existing window handle to create Noriko window for */
    NkStringView           m_wndIdent;        /**< window identifier (for querying windows) */
    NkStringView           m_wndTitle;        /**< main window title */
    NkPoint2D              m_wndPos;          /**< main window position (relative to virtual desktop) */
} NkWindowSpecification;


/**
 * \interface NkIWindow
 * \brief     represents a handle to a platform-independent Noriko desktop-based window
 */
NKOM_DECLARE_INTERFACE(NkIWindow) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(
        _Inout_  NkIWindow *self,
        _In_     NkUuid const *iId,
        _Outptr_ NkVoid **resPtr
    );
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt>
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIWindow *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt>
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIWindow *self);

    /**
     * \brief reimplements <tt>NkIInitializable::Initialize()</tt>
     */
    NkErrorCode (NK_CALL *Initialize)(_Inout_ NkIWindow *self, _Inout_opt_ NkWindowSpecification *wndSpecsPtr);

    /**
     */
    NkVoid (NK_CALL *OnEvent)(_Inout_ NkIWindow *self, _In_ struct NkEvent const *evPtr);
    /**
     */
    NkVoid (NK_CALL *OnUpdate)(_Inout_ NkIWindow *self, _In_ NkFloat deltaTime);
    /**
     */
    NkWindowMode (NK_CALL *QueryAllowedWindowModes)(_Inout_ NkIWindow *self);
    /**
     */
    NkNativeWindowHandle (NK_CALL *QueryNativeWindowHandle)(_Inout_ NkIWindow *self);
    /**
     */
    NkWindowMode (NK_CALL *GetWindowMode)(_Inout_ NkIWindow *self);
    /**
     */
    NkErrorCode (NK_CALL *SetWindowMode)(_Inout_ NkIWindow *self, _In_ NkWindowMode newMode);
    /**
     */
    NkBoolean (NK_CALL *GetWindowFlag)(_Inout_ NkIWindow *self, _In_ NkWindowFlags wndFlag);
    /**
     */
    NkErrorCode (NK_CALL *SetWindowFlag)(_Inout_ NkIWindow *self, _In_ NkWindowFlags wndFlag, _In_ NkBoolean newVal);
    /**
     */
    NkErrorCode (NK_CALL *GetWindowPosition)(_Inout_ NkIWindow *self, _Out_ NkPoint2D *resPtr);
    /**
     */
    NkErrorCode (NK_CALL *SetWindowPosition)(_Inout_ NkIWindow *self, _In_ NkPoint2D const newWndPos);
    /**
     */
    NkErrorCode (NK_CALL *GetWindowSize)(_Inout_ NkIWindow *self, _Out_ NkSize2D *resPtr);
    /**
     */
    NkErrorCode (NK_CALL *SetWindowSize)(_Inout_ NkIWindow *self, _In_ NkSize2D newWndSize);
};


/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkWindowSysStartup(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkWindowSysShutdown(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkWindowSysCreateWindow(_In_ NkWindowSpecification const *wndSpecPtr);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkWindowSysDestroyWindow(_Inout_ NkIWindow *wndRef);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkWindowSysUpdate(_In_ NkFloat deltaTime);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkWindowSysProcessEvent(_In_ struct NkEvent const *evPtr);
/**
 */
NK_NATIVE NK_API NkIWindow *NK_CALL NkWindowSysQueryMainWindow(NkVoid);
/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkWindowSysQueryIdentifierForWindow(_In_ NkIWindow const *wndHandle);
/**
 */
NK_NATIVE NK_API NkIWindow *NK_CALL NkWindowSysQueryWindow(_In_ NkStringView wndIdent);
/**
 */
NK_NATIVE NK_API NkErrorCode NK_CALL NkWindowSysEnumerateWindows(
    _In_        NkErrorCode (NK_CALL *enumFn)(_Inout_ NkIWindow *, _In_ NkSize, _In_ NkSize, _Inout_opt_ NkVoid *),
    _Inout_opt_ NkVoid *extraCxtPtr
);


