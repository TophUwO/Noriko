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


#if (defined NK_TARGET_WINDOWS)
/**
 * \struct __NkInt_WindowsWindow
 * \brief  represents the internal state for the windows window
 */
NK_NATIVE typedef struct __NkInt_WindowsWindow {
    NKOM_IMPLEMENTS(NkIWindow);

    HWND          mp_nativeHandle;   /**< native window handle */
    NkWindowMode  m_allowedWndModes; /**< allowed window modes */
    NkWindowMode  m_currWndMode;     /**< current window mode */
    NkWindowFlags m_wndFlags;        /**< window flags */
    NkStringView  m_wndTitle;        /**< default window title */
    NkStringView  m_wndIdent;        /**< textual window identifier */
} __NkInt_WindowsWindow;


/**
 */
NK_INTERNAL LRESULT CALLBACK __NkInt_WindowsWindow_WndProc(HWND wndHandle, UINT msgId, WPARAM wParam, LPARAM lParam) {
    /* Get the pointer to the underlying window structure. */
    __NkInt_WindowsWindow *wndRef = (__NkInt_WindowsWindow *)GetWindowLongPtr(wndHandle, GWLP_USERDATA);

    switch (msgId) {
        case WM_CLOSE:
            DestroyWindow(wndHandle);

            /*
             * If the current window is the main window, we exit the application
             * normally.
             */
            if (wndRef->m_wndFlags & NkWndFlag_MainWindow)
                NkApplicationExit(NkErr_Ok);
            break;
    }

    return DefWindowProcA(wndHandle, msgId, wParam, lParam);
}

/**
 */
NK_INTERNAL NK_INLINE NkWindowMode NK_CALL __NkInt_WindowsWindow_QueryAllowedPlatformWindowModes(NkVoid) {
    return NkWndMode_All;
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
        /* Register the window class. */
        if (RegisterClassExA(&(WNDCLASSEXA const){
            .cbSize        = sizeof(WNDCLASSEXA),
            .style         = 0,
            .lpfnWndProc   = (WNDPROC)&__NkInt_WindowsWindow_WndProc,
            .cbClsExtra    = 0,
            .cbWndExtra    = 0,
            .hInstance     = GetModuleHandle(NULL),
            .hIcon         = LoadIcon(NULL, IDI_APPLICATION),
            .hCursor       = LoadCursor(NULL, IDI_APPLICATION),
            .hbrBackground = (HBRUSH)NULL,
            .lpszMenuName  = NULL,
            .lpszClassName = "NkInt_WindowsWindow",
            .hIconSm       = LoadIcon(NULL, IDI_APPLICATION)
        }) == FALSE) {
            NK_LOG_ERROR("Could not register window class.");

            return NkErr_RegWindowClass;
        }

        /* Actually create the window. */
        wndPtr->mp_nativeHandle = CreateWindowExA(
            WS_EX_APPWINDOW | WS_EX_OVERLAPPEDWINDOW | WS_EX_COMPOSITED,
            "NkInt_WindowsWindow",
            wndSpecs->m_wndTitle.mp_dataPtr,
            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
            wndSpecs->m_wndPos.m_xCoord == INT64_MAX ? CW_USEDEFAULT : (int)wndSpecs->m_wndPos.m_xCoord,
            wndSpecs->m_wndPos.m_yCoord == INT64_MAX ? CW_USEDEFAULT : (int)wndSpecs->m_wndPos.m_yCoord,
            (int)(wndSpecs->m_vpExtents.m_width * wndSpecs->m_glTileSize.m_width),
            (int)(wndSpecs->m_vpExtents.m_height * wndSpecs->m_glTileSize.m_height),
            wndSpecs->m_wndFlags & NkWndFlag_MessageOnlyWnd ? HWND_MESSAGE : NULL,
            (HMENU)NULL,
            (HINSTANCE)GetModuleHandle(NULL),
            (LPVOID)NULL
        );
        if (wndPtr->mp_nativeHandle == NULL) {
            NK_LOG_ERROR("Failed to create native platform window.");

            /* Cleanup properly by unregistering the class. */
            UnregisterClassA("NkInt_WindowsWindow", GetModuleHandle(NULL));
            return NkErr_CreateNativeWindow;
        }
        /* Set the window userdata pointer to the current NkIWindow instance. */
        SetWindowLongPtr(wndPtr->mp_nativeHandle, GWLP_USERDATA, (LONG_PTR)self);

        /* Set properties. */
        wndPtr->m_allowedWndModes = wndSpecs->m_allowedWndModes & __NkInt_WindowsWindow_QueryAllowedPlatformWindowModes();
        wndPtr->m_currWndMode     = wndSpecs->m_initialWndMode;
        wndPtr->m_wndFlags        = wndSpecs->m_wndFlags;

        /* Set window show state. */
        UpdateWindow(wndPtr->mp_nativeHandle);
        ShowWindow(wndPtr->mp_nativeHandle, SW_SHOWNORMAL);

        /* All good. */
        return NkErr_Ok;
    }

    /* Attaching to existing windows is currently not implemented. */
    NK_LOG_ERROR("Attaching to existing windows is currently not implemented on Windows.");
    return NkErr_NotImplemented;
}

/**
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_WindowsWindow_OnUpdate(_Inout_ NkIWindow *self, _In_ NkFloat deltaTime) {
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
NK_INTERNAL NkWindowMode NK_CALL __NkInt_WindowsWindow_GetWindowMode(_Inout_ NkIWindow *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ((__NkInt_WindowsWindow *)self)->m_currWndMode;
}


/**
 */
NKOM_DEFINE_VTABLE(NkIWindow) {
    .QueryInterface          = &__NkInt_WindowsWindow_QueryInterface,
    .AddRef                  = &__NkInt_WindowsWindow_AddRef,
    .Release                 = &__NkInt_WindowsWindow_Release,
    .Initialize              = &__NkInt_WindowsWindow_Initialize,
    .OnUpdate                = &__NkInt_WindowsWindow_OnUpdate,
    .QueryAllowedWindowModes = &__NkInt_WindowsWindow_QueryAllowedWindowModes,
    .QueryNativeWindowHandle = &__NkInt_WindowsWindow_QueryNativeWindowHandle,
    .GetWindowMode           = &__NkInt_WindowsWindow_GetWindowMode,
    .SetWindowMode           = NULL,
    .GetWindowFlag           = NULL,
    .SetWindowFlag           = NULL,
    .GetWindowPosition       = NULL,
    .SetWindowPosition       = NULL
};

/**
 * \brief actual instance of the windows window 
 */
NK_INTERNAL __NkInt_WindowsWindow gl_Window = { .NkIWindow_Iface = &NKOM_VTABLEOF(NkIWindow) };


NkIWindow *NK_CALL __NkInt_WindowQueryPlatformInstance(NkVoid) {
    return (NkIWindow *)&gl_Window;
}
#endif /* NK_TARGET_WINDOWS */


#undef NK_NAMESPACE


