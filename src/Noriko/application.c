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
 * \file  application.c
 * \brief implements the Noriko engine component's startup-, shutdown- and main loop
 *        routines
 */
#define NK_NAMESPACE "nk::app"


/* Noriko includes */
#include <include/Noriko/noriko.h>


/** \cond INTERNAL */
/**
 * \struct __NkInt_Application
 * \brief  represents the fundamental application data-structure, holding global data
 *         and references to instantiable internal engine components
 */
NK_NATIVE typedef struct __NkInt_Application {
    NkApplicationSpecification m_appSpecs; /**< application specification */
} __NkInt_Application;
/**
 * \brief actual instance of the global application context 
 */
NK_INTERNAL __NkInt_Application gl_Application;


/**
 * \struct __NkInt_ComponentInitInfo
 * \brief  represents a component init entry
 */
NK_NATIVE typedef struct __NkInt_ComponentInitInfo {
    NkStringView m_compIdent;                   /**< component identifier (for logging) */

    NkErrorCode (NK_CALL *mp_initFn)(NkVoid);   /**< component startup function */
    NkErrorCode (NK_CALL *mp_uninitFn)(NkVoid); /**< component shutdown function */
} __NkInt_ComponentInitInfo;

/**
 * \brief lists all components that Noriko must initialize before the main-loop can be
 *        entered
 * 
 * \par Remarks
 *   The listed components are initialized in the order they appear in the array, and are
 *   uninitialized in the reverse order they are specified.
 */
NK_INTERNAL __NkInt_ComponentInitInfo const gl_c_CompInitTable[] = {
    { NK_MAKE_STRING_VIEW("allocators"),                 &NkAllocInitialize,   &NkAllocUninitialize  },
    { NK_MAKE_STRING_VIEW("logging"),                    &NkLogStartup,        &NkLogShutdown        },
    { NK_MAKE_STRING_VIEW("timing devices"),             &NkTimerInitialize,   &NkTimerUninitialize  },
    { NK_MAKE_STRING_VIEW("PRNG"),                       &NkPRNGInitialize,    &NkPRNGUninitialize   },
    { NK_MAKE_STRING_VIEW("command-line"),               &NkEnvStartup,        &NkEnvShutdown        },
    { NK_MAKE_STRING_VIEW("Noriko Object Model (NkOM)"), &NkOMInitialize,      &NkOMUninitialize     },
    { NK_MAKE_STRING_VIEW("input abstraction layer"),    &NkInputStartup,      &NkInputShutdown      },
    { NK_MAKE_STRING_VIEW("renderer factory"),           &NkRendererStartup,   &NkRendererShutdown   },
    { NK_MAKE_STRING_VIEW("layer stack"),                &NkLayerstackStartup, &NkLayerstackShutdown },
    { NK_MAKE_STRING_VIEW("main window"),                &NkWindowStartup,     &NkWindowShutdown     },
    { NK_MAKE_STRING_VIEW("world layer"),                &NkWorldStartup,      &NkWorldShutdown      },
};
/**
 * \brief number of elements in the component-init table 
 */
NK_INTERNAL NkSize const gl_c_CompInitTblSize = NK_ARRAYSIZE(gl_c_CompInitTable);


/**
 */
_Return_ok_ NK_INTERNAL NkErrorCode __NkInt_ValidateAppSpecification(_In_ NkApplicationSpecification const *specsPtr) {
    NkErrorCode errCode = NkErr_Ok;

    /* Validate command-line specification. */
    NK_WEAK_ASSERT(errCode, NkErr_InParameter, specsPtr->m_argc > 0, ERROR, "argc must be greater than or equal to 1!");
    NK_WEAK_ASSERT(errCode, NkErr_InptrParameter, specsPtr->mp_argv != NULL, ERROR, "argv must be non-zero!");

    /* Validate window specification. */
    NK_WEAK_ASSERT(
        errCode,
        NkErr_InParameter,
        specsPtr->m_allowedWndModes & NkWndMode_Normal,
        ERROR,
        "The window must support the 'NkWndMode_Normal' window mode!"
    );
    /** \todo validate app spec more */

    return errCode;
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkApplicationStartup(_In_ NkApplicationSpecification const *specsPtr) {
    /*
     * First of all, validate the application specification. This can catch some
     * high-level error conditions that would later fail to initialize the components.
     */
    NkErrorCode errCode;
    if ((errCode = __NkInt_ValidateAppSpecification(specsPtr)) != NkErr_Ok) {
        NK_LOG_CRITICAL(
            "Invalid application specification passed; validation failed with error '%s' (%i)."
            " Check logs for more details.",
            NkGetErrorCodeStr(errCode)->mp_dataPtr,
            errCode
        );

        return errCode;
    }
    /* 
     * Copy the application specs before initializing since the init-functions may need
     * to query for the application specification.
     */
    gl_Application.m_appSpecs = *specsPtr;

    /* Initialize the internal components in order. */
    for (NkSize i = 0; i < gl_c_CompInitTblSize; i++) {
        errCode = (*gl_c_CompInitTable[i].mp_initFn)();

        if (errCode != NkErr_Ok) {
            NK_LOG_CRITICAL(
                "Failed to initialize component '%s'. Failed with error code '%s' (%i). Check logs for more details.",
                gl_c_CompInitTable[i].m_compIdent.mp_dataPtr,
                NkGetErrorCodeStr(errCode)->mp_dataPtr,
                errCode
            );

            return errCode;
        }
    }

    /* Startup was successful. */
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkApplicationShutdown(NkVoid) {
    /* Shutdown all components in the reverse order they were started up. */
    for (NkInt64 i = (NkInt64)gl_c_CompInitTblSize - 1; i >= 0; i--) {
        NkErrorCode const errCode = (*gl_c_CompInitTable[i].mp_uninitFn)();

        if (errCode != NkErr_Ok) {
            NK_LOG_CRITICAL(
                "Failed to shutdown component '%s'. Failed with error code '%s' (%i). Checks logs for more details.",
                gl_c_CompInitTable[i].m_compIdent.mp_dataPtr,
                NkGetErrorCodeStr(errCode)->mp_dataPtr,
                errCode
            );

            return errCode;
        }
    }

    /* Shutdown was successful. */
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkApplicationRun(NkVoid) {
    /** \cond INTERNAL */
    /**
     * \brief fixed update rate of the game's physics, animation, etc.; currently 120 Hz
     * \note  This is subject to change in the future.
     */
    NkFloat ticksPerUpdate = (8.333f / 1000.f) * NkGetTimerFrequency();
    /** \endcond */

    NkErrorCode    errCode    = NkErr_Ok;
    NkUint64       prevTime   = NkGetCurrentTime();
    NkUint64       currLag    = 0;
    NkUint64 const maxElapsed = (NkUint64)(0.016f * (NkFloat)NkGetTimerFrequency());

#if (defined NK_TARGET_WINDOWS)
    MSG currMsg;
#endif

    for (;;) {
        /* First, calculate timestep. */
        NkUint64 currTime    = NkGetCurrentTime();
        NkUint64 elapsedTime = NK_MIN(currTime - prevTime, maxElapsed);
        prevTime = currTime;
        currLag += elapsedTime;

#if (defined NK_TARGET_WINDOWS)
        /* Then, dispatch windows messages. */
        while (PeekMessage(&currMsg, NULL, 0, 0, PM_REMOVE) ^ 0) {
            if (currMsg.message == WM_QUIT) {
                /*
                 * Check if the message was actually the Windows 'WM_QUIT' message.
                 * The 'WM_QUIT' message is not sent to any window procedures. When
                 * this message is received, we leave the main loop because the app
                 * was instructed to quit in an orderly manner.
                 */
                errCode = (NkErrorCode)currMsg.wParam;

                goto lbl_CLEANUP;
            }

            TranslateMessage(&currMsg);
            DispatchMessage(&currMsg);
        }
#else
    #error Need to implement main loop message handling on this platform.
#endif

        /*
         * Update the game's layers. If the game cannot keep up with the framerate,
         * simulate multiple frames before rendering to ensure the physics stay
         * consistent.
         */
        while (currLag > ticksPerUpdate) {
            /* \todo Update game objects and everything. */

            /* Frame was processed; go ahead and catch up more possibly. */
            currLag -= (NkUint64)ticksPerUpdate;
        }

        /* Run the renderer at the variable timestep. */
        NkIWindow   *mainWnd   = NkWindowQueryInstance();
        NkIRenderer *mainWndRd = mainWnd->VT->GetRenderer(mainWnd);

        mainWndRd->VT->BeginDraw(mainWndRd);
        NK_IGNORE_RETURN_VALUE(NkLayerstackOnRender(currLag / ticksPerUpdate));
        mainWndRd->VT->EndDraw(mainWndRd);

        /* Release the renderer since 'GetRenderer()' acquired it. */
        mainWnd->VT->Release(mainWnd);
        mainWndRd->VT->Release(mainWndRd);
    }

lbl_CLEANUP:
    return errCode;
}

NkVoid NK_CALL NkApplicationExit(_Ecode_range_ NkErrorCode errCode) {
#if (defined NK_TARGET_WINDOWS)
    PostQuitMessage((int)errCode);
#else
    #error Implementation for 'NkApplicationExit()' missing on this platform.
#endif
}


NkApplicationSpecification const *NK_CALL NkApplicationQuerySpecification(NkVoid) {
    /*
     * If this function is called before having called 'NkApplicationStartup()', this
     * will simply return a structure in which all members are initialized to 0. That's
     * alright.
     */
    return (NkApplicationSpecification const *)&gl_Application.m_appSpecs;
}


#undef NK_NAMESPACE


