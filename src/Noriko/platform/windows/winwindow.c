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
 * \file  winwindow.c
 * \brief implements the \c NkIWindow interface for the win32 platform
 */
#define NK_NAMESPACE "nk::winwindow"


/* Noriko includes */
#include <include/Noriko/window.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/log.h>
#include <include/Noriko/noriko.h>
#include <include/Noriko/input.h>


#if (defined NK_TARGET_WINDOWS)
/**
 * \struct __NkInt_WindowsWindow
 * \brief  represents the internal state for the windows window
 */
NK_NATIVE typedef struct __NkInt_WindowsWindow {
    NKOM_IMPLEMENTS(NkIWindow);

    HWND          mp_nativeHandle;   /**< native window handle */
    POINT         m_lastMousePos;    /**< last mouse position */
    NkWindowMode  m_allowedWndModes; /**< allowed window modes */
    NkWindowMode  m_currWndMode;     /**< current window mode */
    NkWindowFlags m_wndFlags;        /**< window flags */
    NkStringView  m_wndTitle;        /**< default window title */
    NkUuid        m_wndUuid;         /**< unique window identifier */
    NkIRenderer  *mp_rendererRef;    /**< reference to the renderer for this window */
    NkIInput     *mp_ialRef;         /**< reference to IAL */
} __NkInt_WindowsWindow;


/**
 */
NK_INTERNAL NkWindowMode NK_CALL __NkInt_WindowsWindow_GetNewWindowMode(_In_ HWND wndHandle) {
    if (IsZoomed(wndHandle)) return NkWndMode_Maximized;
    if (IsIconic(wndHandle)) return NkWndMode_Minimized;

    return NkWndMode_Normal;
}

/**
 */
NK_INTERNAL NkEventType NK_CALL __NkInt_WindowsWindow_MapFromNativeInputEvent(_In_ UINT msgId) {
    switch (msgId) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:  return NkEv_KeyboardKeyDown;
        case WM_KEYUP:
        case WM_SYSKEYUP:    return NkEv_KeyboardKeyUp;
        case WM_LBUTTONDOWN: 
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_XBUTTONDOWN: return NkEv_MouseButtonDown;
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:   
        case WM_XBUTTONUP:   return NkEv_MouseButtonUp;
    }

    return NkEv_None;
}

/**
 */
NK_INTERNAL LRESULT CALLBACK __NkInt_WindowsWindow_WndProc(HWND wndHandle, UINT msgId, WPARAM wParam, LPARAM lParam) {
    /* Get the pointer to the underlying window structure. */
    __NkInt_WindowsWindow *wndRef = (__NkInt_WindowsWindow *)GetWindowLongPtr(wndHandle, GWLP_USERDATA);

    switch (msgId) {
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP: {
            /* Determine event type. */
            NkEventType const evType = msgId == WM_KEYDOWN && (lParam & (1 << 30))
                ? NkEv_KeyboardKeyRepeated
                : __NkInt_WindowsWindow_MapFromNativeInputEvent(msgId)
            ;

            /*
             * Dispatch the event to the layer-stack. The return value is ignored since
             * Windows mandates that the window procedure return 0 if one of the WM_KEY*
             * messages was handled.
             */
            NK_IGNORE_RETURN_VALUE(
                NkEventDispatch(evType, &(NkKeyboardEvent const){
                    .m_pKeyCode   = (NkInt32)(lParam & 0x00FF0000),
                    .m_vNtKeyCode = (NkInt32)wParam,
                    .m_vKeyCode   = wndRef->mp_ialRef->VT->MapFromNativeKey(wndRef->mp_ialRef, (int)wParam)
                })
            );
            return 0;
        }
        case WM_LBUTTONDOWN:
        case WM_MBUTTONDOWN:
        case WM_RBUTTONDOWN:
        case WM_XBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_MBUTTONUP:
        case WM_RBUTTONUP:
        case WM_XBUTTONUP: {
            /* Determine event type. */
            NkEventType const evType = __NkInt_WindowsWindow_MapFromNativeInputEvent(msgId);
            WPARAM      const ntBtn  = wParam >> (msgId == WM_XBUTTONDOWN || msgId == WM_XBUTTONUP) * 16;
            /* Determine global mouse position. */
            POINT glPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ClientToScreen(wndHandle, &glPos);

            /* Dispatch event. */
            NK_IGNORE_RETURN_VALUE(
                NkEventDispatch(evType, &(NkMouseEvent const){
                    .m_curPos   = (NkPoint2D){ (NkInt64)GET_X_LPARAM(lParam), (NkInt64)GET_Y_LPARAM(lParam) },
                    .m_glCurPos = (NkPoint2D){ (NkInt64)glPos.x, (NkInt64)glPos.y },
                    .m_mouseBtn = wndRef->mp_ialRef->VT->MapFromNativeMouseButton(wndRef->mp_ialRef, (int)ntBtn)
                })
            );
            return 0;
        }
        case WM_MOUSEMOVE: {
            /*
             * If the previous mouse position is equal to the current mouse position, the
             * position hasn't changed so we can safely ignore it. This is significant
             * since the system may send spurious WM_MOUSEMOVE message to initiate mouse
             * recalculations. For more information, see:
             *     https://devblogs.microsoft.com/oldnewthing/20160616-00/?p=93685
             */
            if (wndRef->m_lastMousePos.x == GET_X_LPARAM(lParam) && wndRef->m_lastMousePos.y == GET_Y_LPARAM(lParam))
                return 0;
            /* Update the mouse position. */
            wndRef->m_lastMousePos = (POINT){ GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

            /* Determine global mouse position. */
            POINT glPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ClientToScreen(wndHandle, &glPos);

            /* Dispatch event. */
            NK_IGNORE_RETURN_VALUE(
                NkEventDispatch(NkEv_MouseMoved, &(NkMouseEvent const){
                    .m_curPos   = (NkPoint2D){ (NkInt64)GET_X_LPARAM(lParam), (NkInt64)GET_Y_LPARAM(lParam) },
                    .m_glCurPos = (NkPoint2D){ (NkInt64)glPos.x, (NkInt64)glPos.y },
                    .m_mouseBtn = NkBtn_Unknown
                })
            );
            return 0;
        }
        case WM_MOUSEWHEEL: {
            /*
             * Determine if the wheel was moved up (that is, away from the user), or down
             * (i.e., towards the user).
             * Note: This code currently assumes a constant wheel-delta of 120 (default).
             */
            if (wParam >> 16 == 0)
                return 0;
            NkEventType const evType = GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? NkEv_MouseScrollUp : NkEv_MouseScrollDown;
            /* Determine global mouse position. */
            POINT glPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
            ClientToScreen(wndHandle, &glPos);

            /* Dispatch the event. */
            NK_IGNORE_RETURN_VALUE(
                NkEventDispatch(evType, &(NkMouseEvent const){
                    .m_curPos   = (NkPoint2D){ (NkInt64)GET_X_LPARAM(lParam), (NkInt64)GET_Y_LPARAM(lParam) },
                    .m_glCurPos = (NkPoint2D){ (NkInt64)glPos.x, (NkInt64)glPos.y },
                    .m_mouseBtn = NkBtn_Unknown
                })
            );
            return 0;
        };
        case WM_INITMENU:
        case WM_INITMENUPOPUP: {
            /*
             * If we disabled moving by dragging, we also want to disable the
             * corresponding menu item in the system menu.
             */
            HMENU sysMenu = GetSystemMenu(wndHandle, FALSE);
            if ((HMENU)wParam == sysMenu && (wndRef->m_wndFlags & NkWndFlag_DragMovable) == NK_FALSE) {
                EnableMenuItem(sysMenu, SC_MOVE, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

                return 0;
            }

            break;
        }
        case WM_SYSCOMMAND:
            /* Disable moving by dragging the title bar if the specific flag is cleared. */
            if ((wParam & 0xFFF0) == SC_MOVE && (wndRef->m_wndFlags & NkWndFlag_DragMovable) == NK_FALSE)
                return 0;

            break;
        case WM_SIZE:
            /* Window size has changed; do nothing but resize the renderer. */
            if (wndRef != NULL && wndRef->mp_rendererRef != NULL)
                wndRef->mp_rendererRef->VT->Resize(
                    wndRef->mp_rendererRef,
                    wndRef->NkIWindow_Iface.VT->GetClientDimensions((NkIWindow *)wndRef)
                );

            break;
        case WM_WINDOWPOSCHANGED: {
            NkWindowMode const newWndMode = __NkInt_WindowsWindow_GetNewWindowMode(wndHandle);

            /* If the window mode has changed, update it. */
            if (wndRef->m_currWndMode ^ newWndMode)
                wndRef->NkIWindow_Iface.VT->SetWindowMode((NkIWindow *)wndRef, newWndMode);

            break;
        }
        case WM_GETMINMAXINFO: {
            /* Retrieve current viewport dimensions. */
            if (wndRef == NULL || wndRef->mp_rendererRef == NULL)
                break;
            NkSize2D const currVpDim = wndRef->mp_rendererRef->VT->QueryViewportDimensions(wndRef->mp_rendererRef);

            /* Set the minimum client size. */
            ((LPMINMAXINFO)lParam)->ptMinTrackSize = (POINT){
                (LONG)currVpDim.m_width,
                (LONG)currVpDim.m_height
            };
            return 0;
        }
        case WM_ERASEBKGND:
            /* See comment in the 'WM_PAINT' section. */
            return 1;
        case WM_PAINT:
            /*
             * We paint manually in the main loop, so we ignore all WM_PAINT messages. We
             * need to validate the update region or else the system keeps sending
             * WM_PAINT messages until it's painted.
             */
            ValidateRect(wndHandle, NULL);

            return 0;
        case WM_CLOSE:
            /*
             * First, dispatch 'window-closed' event. This lets layers do what they need
             * to do right before the window is closed.
             */
            NK_IGNORE_RETURN_VALUE(
                NkEventDispatch(NkEv_WindowClosed, &(NkWindowEvent const){ .mp_wndRef = (NkIWindow *)wndRef })
            );
            /* Don't destroy the window yet, just hide it. */
            ShowWindow(wndHandle, SW_HIDE);

            /*
             * If the current window is the main window, we exit the application
             * normally. This posts a WM_QUIT message to the thread's message queue which
             * will then initiate destroying of all components, including the window
             * itself.
             */
            if (wndRef->m_wndFlags & NkWndFlag_MainWindow)
                NkApplicationExit(NkErr_Ok);
            return 0;
        case WM_DESTROY:
            /* Destroy the renderer. */
            wndRef->mp_rendererRef->VT->Release(wndRef->mp_rendererRef);

            return 0;
    }

    /*
     * Let the default window procedure handle the rest. This ensures that basic window
     * functions are always there.
     */
    return DefWindowProc(wndHandle, msgId, wParam, lParam);
}

/**
 */
NK_INTERNAL NK_INLINE DWORD NK_CALL __NkInt_WindowsWindow_MapFromWindowModes(
    _In_ NkWindowMode allowedWndModes,
    _In_ NkWindowFlags wndFlags
) {
    DWORD initialValue = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_CLIPSIBLINGS;

    if (allowedWndModes & NkWndMode_Maximized) initialValue |= WS_MAXIMIZEBOX;
    if (allowedWndModes & NkWndMode_Minimized) initialValue |= WS_MINIMIZEBOX;
    if (wndFlags & NkWndFlag_DragResizable)    initialValue |= WS_SIZEBOX;

    return initialValue;
}

/**
 */
NK_INTERNAL NK_INLINE int NK_CALL __NkInt_WindowsWindow_TranslateWindowModeToShowCommand(_In_ NkWindowMode wndMode) {
    switch (wndMode) {
        case NkWndMode_Hidden:     return SW_HIDE;
        case NkWndMode_Fullscreen: 
        case NkWndMode_Maximized:  return SW_SHOWMAXIMIZED;
        case NkWndMode_Minimized:  return SW_SHOWMINIMIZED;
        case NkWndMode_Normal:     return SW_SHOWNORMAL;
    }

    return INT_MAX;
}

/**
 */
NK_INTERNAL NK_INLINE NkSize2D NK_CALL __NkInt_WindowsWindow_AdjustViewportExtents(
    _In_ NkSize2D const *vpExtents,
    _In_ NkSize2D const *dspTsize,
    _In_ NkInt32 wndStyle,
    _In_ NkInt32 extWndStyle
) {
    NK_ASSERT(vpExtents != NULL, NkErr_InParameter);
    NK_ASSERT(dspTsize != NULL, NkErr_InParameter);

    /* Get the maximum viewport extents. */
    NkSize2D maxVpExtents = NkCalculateMaximumViewportExtents(wndStyle, extWndStyle, dspTsize);

    /*
     * Compare the calculated maximum viewport extents to the ones we got, and adjust if
     * necessary.
     */
    return (NkSize2D){
        NK_MIN(vpExtents->m_width,  maxVpExtents.m_width),
        NK_MIN(vpExtents->m_height, maxVpExtents.m_height)
    };
}

/**
 */
NK_INTERNAL NK_INLINE NkSize2D NK_CALL __NkInt_WindowsWindow_GetWindowSize(
    _In_ HWND wndHandle,
    _In_ NkBoolean isClientArea
) {
    RECT res;
    if (isClientArea ? !GetClientRect(wndHandle, &res) : !GetWindowRect(wndHandle, &res))
        return (NkSize2D){ 0, 0 };

    return (NkSize2D) { res.right - res.left, res.bottom - res.top };
}

/**
 */
NK_INTERNAL NK_INLINE NkBoolean NK_CALL __NkInt_WindowsWindow_IsWindowFlagMutable(_In_ NkWindowFlags const wndFlag) {
    switch (wndFlag) {
        case NkWndFlag_MessageOnlyWnd:
        case NkWndFlag_MainWindow:     return NK_FALSE;
        case NkWndFlag_AlwaysOnTop:
        case NkWndFlag_DragMovable:
        case NkWndFlag_DragResizable:  return NK_TRUE;
    }

    return NK_FALSE;
}


/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WindowsWindow_AddRef(NkIWindow *self) {
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub because static instance. */
    return 1;
}

/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WindowsWindow_Release(NkIWindow *self) {
    NK_UNREFERENCED_PARAMETER(self);

    /* Same as above. */
    return 1;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WindowsWindow_QueryInterface(
    _Inout_  NkIWindow *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    NK_INTERNAL NkOMImplementationInfo const gl_c_ImplInfos[] = {
        { NKOM_IIDOF(NkIBase)          },
        { NKOM_IIDOF(NkIInitializable) },
        { NKOM_IIDOF(NkIWindow)        },
        { NULL                         }
    };
    if (NkOMQueryImplementationIndex(gl_c_ImplInfos, iId) != SIZE_MAX) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_WindowsWindow_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WindowsWindow_Initialize(
    _Inout_     NkIWindow *self,
    _Inout_opt_ NkVoid *initParam
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(initParam != NULL, NkErr_InParameter);

    /* Get the pointer to the window specification. */
    NkWindowSpecification const *wndSpecs = (NkWindowSpecification const *)initParam;
    /* Get the pointer to the internal window data-structure. */
    __NkInt_WindowsWindow *wndPtr = (__NkInt_WindowsWindow *)self;

    /*
     * If we do not attach to an existing window, we must first register a window class
     * and then create the platform window.
     */
    if (wndSpecs->mp_nativeHandle == NULL) {
        /** \cond INTERNAL */
        /**
         * \brief name of the windows window class used for creating native windows 
         */
        NK_INTERNAL NkStringView const gl_WndClassName = NK_MAKE_STRING_VIEW("NkInt_WindowsWindow");
        /** \endcond */

        /* Register the window class. */
        if (RegisterClassEx(&(WNDCLASSEX const){
            .cbSize        = sizeof(WNDCLASSEX),
            .style         = CS_OWNDC,
            .lpfnWndProc   = (WNDPROC)&__NkInt_WindowsWindow_WndProc,
            .cbClsExtra    = 0,
            .cbWndExtra    = 0,
            .hInstance     = GetModuleHandle(NULL),
            .hIcon         = LoadIcon(NULL, IDI_APPLICATION),
            .hCursor       = LoadCursor(NULL, IDI_APPLICATION),
            .hbrBackground = NULL,
            .lpszMenuName  = NULL,
            .lpszClassName = gl_WndClassName.mp_dataPtr,
            .hIconSm       = LoadIcon(NULL, IDI_APPLICATION)
        }) == FALSE) {
            NK_LOG_ERROR("Could not register window class.");

            return NkErr_RegWindowClass;
        }

        /* Calculate window metrics, that is, initial size (to fit the viewport). */
        DWORD wndStyle    = __NkInt_WindowsWindow_MapFromWindowModes(wndSpecs->m_allowedWndModes, wndSpecs->m_wndFlags);
        DWORD extWndStyle = WS_EX_WINDOWEDGE | WS_EX_APPWINDOW;
        NkSize2D actVpExtents = __NkInt_WindowsWindow_AdjustViewportExtents(
            &wndSpecs->m_vpExtents,
            &wndSpecs->m_dispTileSize,
            (NkInt32)wndStyle,
            (NkInt32)extWndStyle
        );
        /*
         * Calculate window size from the client area size we got through calculating the 
         * actual viewport extents. This will then be fed into 'CreateWindowEx()' to
         * create a window that has exactly our requested client size.
         */
        RECT wndSize = (RECT){
            0,
            0,
            (LONG)(actVpExtents.m_width  * wndSpecs->m_dispTileSize.m_width),
            (LONG)(actVpExtents.m_height * wndSpecs->m_dispTileSize.m_height)
        };
        if (!AdjustWindowRectEx(&wndSize, wndStyle, FALSE, extWndStyle)) {
            NK_LOG_ERROR("Could not adjust client area size to fit viewport.");

            /* Cleanup properly by unregistering the class. */
            UnregisterClass("NkInt_WindowsWindow", GetModuleHandle(NULL));
            return NkErr_AdjustClientArea;
        }
        /* Calculate initial window position (centered on primary display). */
        NkPoint2D const initialPos = NkCalculateInitialWindowPos(
            &(NkSize2D const){
                wndSize.right - wndSize.left,
                wndSize.bottom - wndSize.top
            }
        );

        /* Actually create the window. */
        wndPtr->mp_nativeHandle = CreateWindowEx(
            extWndStyle,
            gl_WndClassName.mp_dataPtr,
            wndSpecs->m_wndTitle.mp_dataPtr,
            wndStyle,
            (int)initialPos.m_xCoord,
            (int)initialPos.m_yCoord,
            wndSize.right - wndSize.left,
            wndSize.bottom - wndSize.top,
            wndSpecs->m_wndFlags & NkWndFlag_MessageOnlyWnd ? HWND_MESSAGE : NULL,
            (HMENU)NULL,
            (HINSTANCE)GetModuleHandle(NULL),
            (LPVOID)NULL
        );
        if (wndPtr->mp_nativeHandle == NULL) {
            NK_LOG_ERROR("Failed to create native platform window.");

            /* Cleanup properly by unregistering the class. */
            UnregisterClass("NkInt_WindowsWindow", GetModuleHandle(NULL));
            return NkErr_CreateNativeWindow;
        }
        /* Set the window userdata pointer to the current NkIWindow instance. */
        SetWindowLongPtr(wndPtr->mp_nativeHandle, GWLP_USERDATA, (LONG_PTR)self);
        /* Set title bar to dark mode if possible. */
        DwmSetWindowAttribute(wndPtr->mp_nativeHandle, DWMWA_USE_IMMERSIVE_DARK_MODE, &(BOOL){ TRUE }, sizeof(BOOL));

        /* Create a renderer for the window. */
        NkUuid const *implCLSID = NkRendererQueryCLSIDFromApi(wndSpecs->m_rendererApi);
        NkErrorCode errCode = NkOMCreateInstance(implCLSID, NULL, NKOM_IIDOF(NkIRenderer), &(NkRendererSpecification){
            .m_structSize   = sizeof(NkRendererSpecification),
            .mp_wndRef      = self,
            .m_isVSync      = wndSpecs->m_isVSync,
            .m_rendererApi  = wndSpecs->m_rendererApi,
            .m_vpExtents    = wndSpecs->m_vpExtents,
            .m_dispTileSize = wndSpecs->m_dispTileSize,
            .m_vpAlignment  = wndSpecs->m_vpAlignment,
            .m_clearCol     = NK_MAKE_RGB(0, 0, 0),
            .m_texInterMode = NkTexIMd_NearestNeighbor
        }, (NkIBase **)&wndPtr->mp_rendererRef);
        if (errCode != NkErr_Ok) {
            /** \todo change to release() and destroy window */

            return errCode;
        }

        /* Set properties. */
        NkUuidCopy(&wndSpecs->m_wndUuid, &wndPtr->m_wndUuid);
        wndPtr->m_allowedWndModes = wndSpecs->m_allowedWndModes & (NkWndMode_All & ~NkWndMode_Fullscreen);
        wndPtr->m_wndFlags        = wndSpecs->m_wndFlags;
        wndPtr->mp_ialRef         = NkInputQueryInstance();
        wndPtr->m_lastMousePos    = (POINT){ 0, 0 };

        /* Send initial events. */
        NK_IGNORE_RETURN_VALUE(NkEventDispatch(NkEv_WindowOpened, &(NkWindowEvent){ .mp_wndRef = self }));
        self->VT->SetWindowMode(self, wndSpecs->m_initialWndMode);
        return NkErr_Ok;
    }

    /* Attaching to existing windows is currently not implemented. */
    NK_LOG_ERROR("Attaching to existing windows is currently not implemented on Windows.");
    return NkErr_NotImplemented;
}

/**
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_WindowsWindow_OnUpdate(_Inout_ NkIWindow *self, _In_ NkFloat deltaTime) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Stub. */
    NK_UNREFERENCED_PARAMETER(self);
    NK_UNREFERENCED_PARAMETER(deltaTime);
}

/**
 */
NK_INTERNAL NkWindowMode NK_CALL __NkInt_WindowsWindow_QueryAllowedWindowModes(_Inout_ NkIWindow *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ((__NkInt_WindowsWindow *)self)->m_allowedWndModes;
}

/**
 */
NK_INTERNAL NkNativeWindowHandle NK_CALL __NkInt_WindowsWindow_QueryNativeWindowHandle(_Inout_ NkIWindow *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return (NkNativeWindowHandle)((__NkInt_WindowsWindow *)self)->mp_nativeHandle;
}

/**
 */
NK_INTERNAL NkUuid const *NK_CALL __NkInt_WindowsWindow_QueryWindowIdentifier(_Inout_ NkIWindow *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return &((__NkInt_WindowsWindow *)self)->m_wndUuid;
}

/**
 */
NK_INTERNAL NkWindowMode NK_CALL __NkInt_WindowsWindow_GetWindowMode(_Inout_ NkIWindow *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ((__NkInt_WindowsWindow *)self)->m_currWndMode;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WindowsWindow_SetWindowMode(
    _Inout_ NkIWindow *self,
    _In_    NkWindowMode newMode
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT((newMode & (newMode - 1)) == 0, NkErr_InParameter);
    
    /* Check if the current window mode is even supported. */
    if ((newMode & self->VT->QueryAllowedWindowModes(self)) == NK_FALSE) {
        NK_LOG_ERROR("Window mode '%s' not supported for this window.", NkWindowGetModeStr(newMode)->mp_dataPtr);

        return NkErr_WndModeNotSupported;
    }

    /* Apply the new window mode. */
    int const newShowCmd = __NkInt_WindowsWindow_TranslateWindowModeToShowCommand(newMode);
    if (newShowCmd != INT_MAX) {
        /* Get pointer to the internal window data. */
        __NkInt_WindowsWindow *wndRef = (__NkInt_WindowsWindow *)self;
        /* Update the internal flag. */
        wndRef->m_currWndMode = newMode;

        /* Do the native window mode changing. */
        ShowWindow(wndRef->mp_nativeHandle, newShowCmd);

        /* Lastly, dispatch an event to let the layers know. */
        return NkEventDispatch(NkWindowMapEventTypeFromWindowMode(newMode), &(NkWindowEvent){
            .mp_wndRef      = self,
            .m_wndSize      = __NkInt_WindowsWindow_GetWindowSize(wndRef->mp_nativeHandle, NK_FALSE),
            .m_totalWndSize = __NkInt_WindowsWindow_GetWindowSize(wndRef->mp_nativeHandle, NK_TRUE),
            .m_wndPos       = (NkPoint2D){ 0, 0 },
            .m_wndMode      = newMode
        });
    }
    
    /* Setting the window mode is currently not implemented. */
    return NkErr_NotImplemented;
}

/**
 */
NK_INTERNAL NkBoolean NK_CALL __NkInt_WindowsWindow_GetWindowFlag(_Inout_ NkIWindow *self, _In_ NkWindowFlags wndFlag) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return (NkBoolean)(((__NkInt_WindowsWindow *)self)->m_wndFlags & wndFlag);
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WindowsWindow_SetWindowFlag(
    _Inout_ NkIWindow *self,
    _In_    NkWindowFlags wndFlag,
    _In_    NkBoolean newVal
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    __NkInt_WindowsWindow *wndRef = (__NkInt_WindowsWindow *)self;

    /* Set or unset the flag. */
    NkBoolean oldVal = __NkInt_WindowsWindow_GetWindowFlag(self, wndFlag);
    if (oldVal == newVal || !__NkInt_WindowsWindow_IsWindowFlagMutable(wndFlag))
        return NkErr_NoOperation;
    wndRef->m_wndFlags = wndRef->m_wndFlags & ~wndFlag | (newVal ? wndFlag : 0);

    /* If the flag was changed, do what has to be done to apply the changes. */
    switch (wndFlag) {
        case NkWndFlag_DragResizable: {
            /**
             * \brief show flags for the 'ShowWindow()' function after manually updating
             *        the window style
             */
            NK_INTERNAL UINT const gl_ShowFlags = SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED;

            /* Change the window style to reflect the new setting. */
            LONG_PTR currWndStyle = GetWindowLongPtr(wndRef->mp_nativeHandle, GWL_STYLE);

            SetWindowLongPtr(
                wndRef->mp_nativeHandle,
                GWL_STYLE,
                currWndStyle & ~WS_SIZEBOX | (newVal ? WS_SIZEBOX : 0)
            );
            SetWindowPos(wndRef->mp_nativeHandle, NULL, 0, 0, 0, 0, gl_ShowFlags);

            return NkErr_Ok;
        }
        case NkWndFlag_DragMovable:
            /*
             * This flag does not require anything else to be done. Thus, we return
             * *NkErr_Ok* so that the caller does not think it has no effect.
             */
            return NkErr_Ok;
    }

    /* Changing the given flag is not implemented or has no effect. */
    return NkErr_NotImplemented;
}

NK_INTERNAL NkSize2D NK_CALL __NkInt_WindowsWindow_GetClientDimensions(_Inout_ NkIWindow *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* The 'client dimensions' are the dimensions of the native window viewport. */
    RECT clRect;
    GetClientRect((HWND)self->VT->QueryNativeWindowHandle(self), &clRect);

    return (NkSize2D) {
        (NkUint64)(clRect.right  - clRect.left),
        (NkUint64)(clRect.bottom - clRect.top)
    };
}

NK_INTERNAL NkIRenderer *NK_CALL __NkInt_WindowsWindow_GetRenderer(_Inout_ NkIWindow *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    __NkInt_WindowsWindow *wndRef = (__NkInt_WindowsWindow *)self;

    wndRef->mp_rendererRef->VT->AddRef(wndRef->mp_rendererRef);
    return wndRef->mp_rendererRef;
}


/**
 * \brief instance of the __NkInt_WindowsWindow class' VTable
 */
NKOM_DEFINE_VTABLE(NkIWindow) {
    .QueryInterface          = &__NkInt_WindowsWindow_QueryInterface,
    .AddRef                  = &__NkInt_WindowsWindow_AddRef,
    .Release                 = &__NkInt_WindowsWindow_Release,
    .Initialize              = &__NkInt_WindowsWindow_Initialize,
    .OnUpdate                = &__NkInt_WindowsWindow_OnUpdate,
    .QueryAllowedWindowModes = &__NkInt_WindowsWindow_QueryAllowedWindowModes,
    .QueryNativeWindowHandle = &__NkInt_WindowsWindow_QueryNativeWindowHandle,
    .QueryWindowIdentifier   = &__NkInt_WindowsWindow_QueryWindowIdentifier,
    .GetWindowMode           = &__NkInt_WindowsWindow_GetWindowMode,
    .SetWindowMode           = &__NkInt_WindowsWindow_SetWindowMode,
    .GetWindowFlag           = &__NkInt_WindowsWindow_GetWindowFlag,
    .SetWindowFlag           = &__NkInt_WindowsWindow_SetWindowFlag,
    .GetClientDimensions     = &__NkInt_WindowsWindow_GetClientDimensions,
    .GetRenderer             = &__NkInt_WindowsWindow_GetRenderer
};


NkIWindow *NK_CALL NkWindowQueryInstance(NkVoid) {
    /**
     * \brief actual instance of the windows window 
     */
    NK_INTERNAL __NkInt_WindowsWindow gl_Window = { .NkIWindow_Iface = &NKOM_VTABLEOF(NkIWindow) };

    /* Only for formality's sake. */
    gl_Window.NkIWindow_Iface.VT->AddRef((NkIWindow *)&gl_Window);
    return (NkIWindow *)&gl_Window;
}
#endif /* NK_TARGET_WINDOWS */


#undef NK_NAMESPACE


