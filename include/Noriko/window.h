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
    NkWndMode_Unknown    = 0,      /**< unknown/invalid window mode */

    NkWndMode_Minimized  = 1 << 0, /**< minimized window mode (taskbar icon still visible) */
    NkWndMode_Maximized  = 1 << 1, /**< maximized window mode (title bar still visible) */
    NkWndMode_Fullscreen = 1 << 2, /**< full-screen window mode (desktop invisible) */
    NkWndMode_Normal     = 1 << 3, /**< normal (= 'windowed' = non-'full-screen') window mode */
    NkWndMode_Visible    = 1 << 4, /**< window is visible; currently unused */
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
    NkWndFlag_MainWindow     = 1 << 2,
    NkWndFlag_DragResizable  = 1 << 3,
    NkWndFlag_DragMovable    = 1 << 4,

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
    enum NkRendererApi          m_rendererApi;     /**< API to use for rendering the window */
         NkViewportAlignment    m_vpAlignment;     /**< viewport alignment inside the main window */
         NkBoolean              m_isVSync;         /**< whether or not VSync is used */
         NkSize2D               m_vpExtents;       /**< size in tiles of the main window viewport */
         NkSize2D               m_dispTileSize;    /**< tile size of the viewport */
         NkWindowMode           m_allowedWndModes; /**< allowed window modes for the main window */
         NkWindowMode           m_initialWndMode;  /**< initial window mode for the main window */
         NkWindowFlags          m_wndFlags;        /**< additional (platform-dependent) window flags */
         NkNativeWindowHandle   mp_nativeHandle;   /**< optional existing window handle to create Noriko window for */
         NkStringView           m_wndIdent;        /**< window identifier (for querying windows) */
         NkStringView           m_wndTitle;        /**< main window title */
} NkWindowSpecification;


/**
 * \interface NkIWindow
 * \brief     represents a handle to a platform-independent Noriko desktop-based window
 */
NKOM_DECLARE_INTERFACE(NkIWindow) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkIWindow *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
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
    NkErrorCode (NK_CALL *Initialize)(_Inout_ NkIWindow *self, _Inout_opt_ NkVoid *initParam);

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
    struct NkIRenderer *(NK_CALL *GetRenderer)(_Inout_ NkIWindow *self);
};


/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkWindowStartup(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkWindowShutdown(NkVoid);

/**
 * \brief  queries the static window instance
 * \return pointer to the static window instance
 * \note   Before using the window instance obtained by this function for the first time,
 *         call \c NkWindowStartup() once to actually create the platform window.
 */
NK_NATIVE NK_API NkIWindow *NK_CALL NkWindowQueryInstance(NkVoid);

/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkWindowGetModeStr(_In_ NkWindowMode wndMode);
/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkWindowGetFlagStr(_In_ NkWindowFlags wndFlag);
/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkWindowGetViewportAlignmentStr(_In_ NkViewportAlignment vpAlignment);
/**
 */
NK_NATIVE NK_API enum NkEventType NK_CALL NkWindowMapEventTypeFromWindowMode(_In_ NkWindowMode wndMode);


