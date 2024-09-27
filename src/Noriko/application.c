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
 */
NK_NATIVE typedef struct __NkInt_ComponentInitInfo {
    NkStringView m_compIdent;                   /**< component identifier (for logging) */

    NkErrorCode (NK_CALL *mp_initFn)(NkVoid);   /**< component startup function */
    NkErrorCode (NK_CALL *mp_uninitFn)(NkVoid); /**< component shutdown function */
} __NkInt_ComponentInitInfo;

/**
 */
NK_INTERNAL __NkInt_ComponentInitInfo const gl_c_CompInitTable[] = {
    { NK_MAKE_STRING_VIEW("allocators"),                 &NkAllocInitialize,   &NkAllocUninitialize  },
    { NK_MAKE_STRING_VIEW("logging"),                    &NkLogInitialize,     &NkLogUninitialize    },
    { NK_MAKE_STRING_VIEW("timing devices"),             &NkTimerInitialize,   &NkTimerUninitialize  },
    { NK_MAKE_STRING_VIEW("PRNG"),                       &NkPRNGInitialize,    &NkPRNGUninitialize   },
    { NK_MAKE_STRING_VIEW("command-line"),               &NkEnvStartup,        &NkEnvShutdown        },
    { NK_MAKE_STRING_VIEW("Noriko Object Model (NkOM)"), &NkOMInitialize,      &NkOMUninitialize     },
    { NK_MAKE_STRING_VIEW("main window"),                &NkWindowStartup,     &NkWindowShutdown     },
    { NK_MAKE_STRING_VIEW("layer stack"),                &NkLayerstackStartup, &NkLayerstackShutdown }
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
#if (defined NK_TARGET_WINDOWS)
    NkErrorCode errCode = NkErr_Ok;
    MSG currMsg;

    for (;;) {
        /* First, dispatch windows messages. */
        while (PeekMessage(&currMsg, NULL, 0, 0, PM_REMOVE) ^ 0) {
            /*
             * Check if the message was actually the Windows 'WM_QUIT' message. The
             * 'WM_QUIT' message is not sent to any window procedures.
             */
            if (currMsg.message == WM_QUIT) {
                errCode = (NkErrorCode)currMsg.wParam;

                goto lbl_CLEANUP;
            }

            TranslateMessage(&currMsg);
            DispatchMessage(&currMsg);
        }

        /* Update everything and render. */
        // ...
    }

lbl_CLEANUP:
    return errCode;
#else
    #error Implementation for 'NkApplicationRun()' missing on this platform.
#endif
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


