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
 *        routines as well as the global component registry
 */
#define NK_NAMESPACE "nk::app"


/* Noriko includes */
#include <include/Noriko/noriko.h>


/** \cond */
NK_COMPONENT_IMPORT(Logging);
NK_COMPONENT_IMPORT(Allocators);
NK_COMPONENT_IMPORT(PRNG);
NK_COMPONENT_IMPORT(TimingDevCxt);
NK_COMPONENT_IMPORT(Env);
NK_COMPONENT_IMPORT(NkOM);
NK_COMPONENT_IMPORT(PathSrv);
NK_COMPONENT_IMPORT(IAL);
NK_COMPONENT_IMPORT(RdFactory);
NK_COMPONENT_IMPORT(Layerstack);
NK_COMPONENT_IMPORT(Window);
NK_COMPONENT_IMPORT(DbSrv);
NK_COMPONENT_IMPORT(AssetManager);
NK_COMPONENT_IMPORT(WorldLayer);
/** \endcond */


/** \cond INTERNAL */
/**
 */
NK_NATIVE typedef enum __NkInt_CompCallbackIndex {
    NkCompCb_PreStartup,
    NkCompCb_Startup,
    NkCompCb_PostStartup,
    NkCompCb_PreShutdown,
    NkCompCb_Shutdown,
    NkCompCb_PostShutdown,

    __NkCompCb_Count__
} __NkInt_CompCallbackIndex;


/**
 * \struct __NkInt_StartupErrorInfo
 * \brief  represents the startup error state
 */
NK_NATIVE typedef struct __NkInt_StartupErrorInfo {
    NkInt64   m_compIndex;   /**< component index */
    NkBoolean m_compSucc[3]; /**< stage success flags */
} __NkInt_StartupErrorInfo;

/**
 * \struct __NkInt_ExtComponent
 * \brief  represents a component entry
 */
NK_NATIVE typedef struct __NkInt_ExtComponent {
    NkComponent const *mp_compInfo;             /**< imported component info */

    NkErrorCode (NK_CALL *mp_preStFn)(NkVoid);  /**< (optional) pre-startup callback */
    NkErrorCode (NK_CALL *mp_postStFn)(NkVoid); /**< (optional) post-startup callback */
    NkErrorCode (NK_CALL *mp_preShFn)(NkVoid);  /**< (optional) pre-shutdown callback */
    NkErrorCode (NK_CALL *mp_postShFn)(NkVoid); /**< (optional) post-shutdown callback */
} __NkInt_ExtComponent;

/**
 * \struct __NkInt_Application
 * \brief  represents the fundamental application data-structure, holding global data
 *         and references to instantiable internal engine components
 */
NK_NATIVE typedef struct __NkInt_Application {
    NkApplicationSpecification  m_appSpecs;     /**< application specification */

    NkHashtable                *mp_compReg;     /**< global NkOM component registry */
    NkBoolean                   m_isStandalone; /**< whether or not the current instance runs standalone */
    __NkInt_StartupErrorInfo    m_initErrInfo;  /**< component initialization error info */

    NK_DECL_LOCK(m_compRegLock);                /**< lock for component registry */
} __NkInt_Application;
/**
 * \brief actual instance of the global application context 
 */
NK_INTERNAL __NkInt_Application gl_Application;


/**
 * \brief lists all components that Noriko must initialize before the main-loop can be
 *        entered
 * 
 * \par Remarks
 *   The listed components are initialized in the order they appear in the array, and are
 *   uninitialized in the reverse order they are specified.
 */
NK_INTERNAL __NkInt_ExtComponent const gl_c_CompInitTable[] = {
    { &NK_COMPONENT(Logging),      NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(Allocators),   NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(PRNG),         NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(TimingDevCxt), NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(Env),          NULL, &__NkInt_Env_PostStartup,  NULL,                      NULL },
    { &NK_COMPONENT(NkOM),         NULL, &__NkInt_NkOM_PostStartup, &__NkInt_NkOM_PreShutdown, NULL },
    { &NK_COMPONENT(PathSrv),      NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(IAL),          NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(RdFactory),    NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(Layerstack),   NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(Window),       NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(DbSrv),        NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(AssetManager), NULL, NULL,                      NULL,                      NULL },
    { &NK_COMPONENT(WorldLayer),   NULL, NULL,                      NULL,                      NULL }
};
/**
 * \brief number of elements in the component-init table 
 */
NK_INTERNAL NkInt64 const gl_c_CompInitTblSize = NK_ARRAYSIZE(gl_c_CompInitTable);


/**
 * \ingroup VirtFn
 * \brief   invokes platform-dependent message handling facilities
 * \param   [out] isLeave set to \c NK_TRUE if the application needs to exit the main
 *                loop
 * \param   [in,out] extraCxt pointer to a variable that contains extra context needed
 *                   for the platform
 * \return  error state of the event handling; if \c isLeave is <tt>NK_FALSE</tt>, the
 *          return value is always <tt>NkErr_Ok</tt>
 */
NK_EXTERN NK_VIRTUAL _Return_ok_ NkErrorCode NK_CALL __NkInt_Application_PlatformLoop(
    _Out_       NkBoolean *isLeave,
    _Inout_opt_ NkVoid *extraCxt
);


/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_ValidateAppSpecification(_In_ NkApplicationSpecification const *specsPtr) {
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

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode (NK_CALL *__NkInt_Application_QueryCompCallback(
    _In_ NkInt64 index,
    _In_ __NkInt_CompCallbackIndex cbIndex
))(NkVoid) {
    NK_ASSERT(NK_INRANGE_INCL(index, 0, gl_c_CompInitTblSize - 1), NkErr_InParameter);
    NK_ASSERT(NK_INRANGE_INCL(cbIndex, NkCompCb_PreStartup, __NkCompCb_Count__ - 1), NkErr_InParameter);

    /* Get the component entry. */
    __NkInt_ExtComponent const *compPtr = &gl_c_CompInitTable[index];

    /* Retrieve the function corresponding to the given callback index. */
    switch (cbIndex) {
        case NkCompCb_PreStartup:   return compPtr->mp_preStFn;
        case NkCompCb_Startup:      return compPtr->mp_compInfo->mp_fnStartup;
        case NkCompCb_PostStartup:  return compPtr->mp_postStFn;
        case NkCompCb_PreShutdown:  return compPtr->mp_preShFn;
        case NkCompCb_Shutdown:     return compPtr->mp_compInfo->mp_fnShutdown;
        case NkCompCb_PostShutdown: return compPtr->mp_postShFn;
    }

    /* The following should never happen. */
    NK_ASSERT_EXTRA(NK_FALSE, NkErr_InParameter, "Invalid component callback index.");
    return NULL;
}

/**
 */
NK_INTERNAL __NkInt_CompCallbackIndex __NkInt_Application_QueryOppositeCbIndex(_In_ __NkInt_CompCallbackIndex cb) {
    NK_ASSERT(NK_INRANGE_INCL(cb, NkCompCb_PreStartup, __NkCompCb_Count__ - 1), NkErr_InParameter);

    /**
     * \brief lookup table returning the \e opposite callback for a given input callback
     */
    NK_INTERNAL __NkInt_CompCallbackIndex const gl_c_OppositeCompIndices[] = {
        [NkCompCb_PreStartup]   = NkCompCb_PostShutdown,
        [NkCompCb_Startup]      = NkCompCb_Shutdown,
        [NkCompCb_PostStartup]  = NkCompCb_PreShutdown,
        [NkCompCb_PreShutdown]  = NkCompCb_PostStartup,
        [NkCompCb_Shutdown]     = NkCompCb_Startup,
        [NkCompCb_PostShutdown] = NkCompCb_PreStartup
    };
    NK_VERIFY_LUT(gl_c_OppositeCompIndices, __NkInt_CompCallbackIndex, __NkCompCb_Count__);

    return gl_c_OppositeCompIndices[cb];
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_Application_InvokeCompCallback(
    _In_ __NkInt_ExtComponent const *extCompPtr,
    _In_ NkInt64 index,
    _In_ __NkInt_CompCallbackIndex cbIndex
) {
    NK_ASSERT(extCompPtr != NULL, NkErr_InParameter);

    /**
     * \brief static lookup table for initialization stage labels (used for debugging) 
     */
    NK_INTERNAL NkStringView const gl_c_InitStageLabels[] = {
        NK_MAKE_STRING_VIEW("pre-startup"),
        NK_MAKE_STRING_VIEW("startup"),
        NK_MAKE_STRING_VIEW("post-startup"),
        NK_MAKE_STRING_VIEW("pre-shutdown"),
        NK_MAKE_STRING_VIEW("shutdown"),
        NK_MAKE_STRING_VIEW("post-shutdown")
    };
    /* Query the callback corresponding to the input parameters. */
    NkErrorCode (NK_CALL *compCb)(NkVoid) = __NkInt_Application_QueryCompCallback(index, cbIndex);

    /* Print the component stage info message. */
    NK_LOG_INFO(
        "%s: %*s%s%s",
        gl_c_InitStageLabels[cbIndex].mp_dataPtr,
        (NkUint32)(sizeof "post-shutdown" - strlen(gl_c_InitStageLabels[cbIndex].mp_dataPtr)) - 1,
        "",
        extCompPtr->mp_compInfo->m_compIdent.mp_dataPtr,
        compCb == NULL ? " ... nothing to do" : ""
    );
    /* Run the callback. */
    NkErrorCode errCode = NkErr_Ok;
    if (compCb != NULL && (errCode = (*compCb)()) != NkErr_Ok) {
        NK_LOG_CRITICAL(
            "Failed to run %s of component \"%s\". "
            "Failed with error code \"%s\" (%i). Check logs for more details.",
            gl_c_InitStageLabels[cbIndex].mp_dataPtr,
            extCompPtr->mp_compInfo->m_compIdent.mp_dataPtr,
            NkGetErrorCodeStr(errCode)->mp_dataPtr,
            errCode
        );

        goto lbl_ONRETURN;
    }

lbl_ONRETURN:
    gl_Application.m_initErrInfo.m_compIndex         = index;
    gl_Application.m_initErrInfo.m_compSucc[cbIndex] = errCode == NkErr_Ok;

    return errCode;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_NkOM_CompRegIterFn(_Inout_ struct NkHashtablePair *pairPtr) {
    __NkInt_ExtComponent const *currComp = (__NkInt_ExtComponent const *)pairPtr->mp_valuePtr;

    /*
     * Print warning message to let developer know that there is a problem with a user
     * module.
     */
    char compUuid[NK_UUIDLEN];
    NK_LOG_WARNING(
        "Component \"%s\" ({%s}, nkom=%s) still registered in component registry. "
        "You have to unregister in the component's shutdown function to preserve proper "
        "shutdown order.",
        currComp->mp_compInfo->m_compIdent.mp_dataPtr,
        NkUuidToString(&currComp->mp_compInfo->m_compUuid, compUuid),
        currComp->mp_compInfo->m_isNkOM ? "yes" : "no"
    );

    return NkErr_Ok;
}

/**
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_NkOM_CompRegDestFn(
    _Inout_opt_ NkHashtableKey *keyPtr,
    _Inout_opt_ NkVoid *valPtr
) {
    NK_ASSERT(keyPtr != NULL, NkErr_InOutParameter);

    /* If the entry isn't empty, release the contained NkOM instance. */
    if (valPtr != NULL)
        ((NkIBase *)valPtr)->VT->Release((NkIBase *)valPtr);
}


/**
 */
_Return_ok_ NkErrorCode NK_CALL __NkInt_Env_PostStartup(NkVoid) {
    /* Check if the application was started with the '--attached' option. */
    NkVariant dummyVar;
    gl_Application.m_isStandalone = NkEnvGetValue("attached", &dummyVar) != NkErr_Ok;

    NK_LOG_INFO("Running Noriko in %s mode.", gl_Application.m_isStandalone ? "standalone" : "attached");
    return NkErr_Ok;
}

/**
 */
_Return_ok_ NkErrorCode NK_CALL __NkInt_NkOM_PostStartup(NkVoid) {
    /* Initialize the static component registry. */
    NK_INITLOCK(gl_Application.m_compRegLock);

    return NkHashtableCreate(&(NkHashtableProperties const){
        .m_structSize  = sizeof(NkHashtableProperties),
        .m_keyType     = NkHtKeyTy_Uuid,
        .m_initCap     = 16,
        .m_minCap      = 16,
        .m_maxCap      = 1024,
        .mp_fnElemFree = &__NkInt_NkOM_CompRegDestFn
    }, &gl_Application.mp_compReg);
}

/**
 */
_Return_ok_ NkErrorCode NK_CALL __NkInt_NkOM_PreShutdown(NkVoid) {
    /* Destroy the static component registry. */
    NK_DESTROYLOCK(gl_Application.m_compRegLock);

    /* Print component info of all components that are still (erroneously) registered. */
    NK_IGNORE_RETURN_VALUE(NkHashtableForEach(gl_Application.mp_compReg, &__NkInt_NkOM_CompRegIterFn));
    NkHashtableDestroy(&gl_Application.mp_compReg);
    return NkErr_Ok;
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
    /* Initialize global application context. */
    gl_Application = (__NkInt_Application){
        .m_appSpecs     = *specsPtr,
        .mp_compReg     = NULL,
        .m_isStandalone = NK_FALSE,
        .m_initErrInfo  = { 0, 0 }
    };

    /* Initialize the internal components in order. */
    __NkInt_ExtComponent const *currComp = &gl_c_CompInitTable[0];
    for (NkInt64 i = 0; i < gl_c_CompInitTblSize; i++, currComp = &gl_c_CompInitTable[i]) {
        /* Run the startup callbacks in order. */
           (errCode = __NkInt_Application_InvokeCompCallback(currComp, i, NkCompCb_PreStartup))  != NkErr_Ok
        || (errCode = __NkInt_Application_InvokeCompCallback(currComp, i, NkCompCb_Startup))     != NkErr_Ok
        || (errCode = __NkInt_Application_InvokeCompCallback(currComp, i, NkCompCb_PostStartup)) != NkErr_Ok;

        /*
         * If we failed at one point, we exit the loop. The exact point of failure is
         * recorded by __NkInt_Application_InvokeCompCallback().
         */
        if (errCode != NkErr_Ok)
            return errCode;

        /* If the component is an NkOM component, add it to the registry. */
        if (currComp->mp_compInfo->m_isNkOM) {
            char uuidBuf[NK_UUIDLEN];
            NkComponent const *compPtr = currComp->mp_compInfo;

            NkApplicationReplaceInstance(compPtr->mp_clsId, (*compPtr->mp_fnQueryInst)());
            NK_LOG_INFO(
                "Registered NkOM component \"%s\" ({%s}).",
                compPtr->m_compIdent.mp_dataPtr,
                NkUuidToString(compPtr->mp_clsId, uuidBuf)
            );
        }
    }

    /* Startup was successful. */
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkApplicationShutdown(NkVoid) {
    /* If no component was initialized, do nothing. */
    if (gl_Application.m_initErrInfo.m_compIndex == 0)
        return NkErr_Ok;

    NkErrorCode errCode = NkErr_Ok;
    /*
     * Shutdown all components in the reverse order they were started up. Only run
     * callbacks of which the opposite returned NkErr_Ok.
     */
    for (NkInt64 i = gl_Application.m_initErrInfo.m_compIndex; i >= 0; i--) {
        __NkInt_ExtComponent const *currComp = &gl_c_CompInitTable[i];

        /* If the component is an NkOM component, unregister it first. */
        if (currComp->mp_compInfo->m_isNkOM) {
            char uuidBuf[NK_UUIDLEN];
            NkComponent const *compPtr = currComp->mp_compInfo;

            NkApplicationReplaceInstance(compPtr->mp_clsId, (NkIBase *)NULL);
            NK_LOG_INFO(
                "Unregistered component \"%s\" ({%s}). ",
                compPtr->m_compIdent.mp_dataPtr,
                NkUuidToString(compPtr->mp_clsId, uuidBuf)
            );
        }

        /* Run stages that succeeded in opposite order. */
        for (__NkInt_CompCallbackIndex j = sizeof gl_Application.m_initErrInfo.m_compSucc - 1; j >= 0; j--) {
            if (gl_Application.m_initErrInfo.m_compSucc[j] == NK_FALSE)
                continue;

            /* For every stage, run the opposite callback if it succeeded. */
            errCode = __NkInt_Application_InvokeCompCallback(
                currComp,
                i,
                __NkInt_Application_QueryOppositeCbIndex(j)
            );
            if (errCode != NkErr_Ok)
                return errCode;
        }
    }

    /* Shutdown was successful. */
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkApplicationRun(NkVoid) {
    /** \cond INTERNAL */
    NkFloat const tiFreq         = (NkFloat)NkTimerGetFrequency();
    /**
     * \brief fixed update rate of the game's physics, animation, etc.; currently 120 Hz
     * \note  This is subject to change in the future.
     */
    NkFloat const ticksPerUpdate = (8.333f / 1000.f) * tiFreq;
    /** \endcond */

    /* Query main window renderer. */
    NkIWindow      *mainWnd    = (NkIWindow *)NkApplicationQueryInstance(NKOM_CLSIDOF(NkIWindow));
    NkIRenderer    *mainWndRd  = mainWnd->VT->GetRenderer(mainWnd);
    /* Initialize miscellaneous state. */
    NkBoolean       isLeave    = NK_TRUE;
    NkErrorCode     errCode    = NkErr_Ok;
    NkUint64        prevTime   = NkTimerGetCurrentTicks();
    NkUint64        currLag    = 0;
    NkUint64 const  maxElapsed = (NkUint64)(0.016f * tiFreq);

    for (;;) {
        /* First, calculate timestep. */
        NkUint64 currTime    = NkTimerGetCurrentTicks();
        NkUint64 elapsedTime = NK_MIN(currTime - prevTime, maxElapsed);
        prevTime = currTime;
        currLag += elapsedTime;

        /*
         * Then, run the platform-dependent main loop portion. This, for example, invokes
         * platform-dependent event handling facilities like the message pump on Windows,
         * etc.
         */
        errCode = __NkInt_Application_PlatformLoop(&isLeave, NULL);
        if (isLeave == NK_TRUE)
            goto lbl_CLEANUP;

        /** \todo dispatch messages which are in thread-safe queue */

        /*
         * Update the game's layers. If the game cannot keep up with the framerate,
         * simulate multiple frames before rendering to ensure the physics stay
         * consistent.
         */
        while (currLag > ticksPerUpdate) {
            /* Update game objects and everything. */
            /** \todo change to OnFixedUpdate */
            NK_IGNORE_RETURN_VALUE(NkLayerstackOnUpdate(ticksPerUpdate / tiFreq));

            /* Frame was processed; go ahead and catch up more possibly. */
            currLag -= (NkUint64)ticksPerUpdate;
        }

        /** \todo do per-frame update */

        /* Run the renderer at the variable timestep. */
        mainWndRd->VT->BeginDraw(mainWndRd);
        NK_IGNORE_RETURN_VALUE(NkLayerstackOnRender(currLag / ticksPerUpdate));
        mainWndRd->VT->EndDraw(mainWndRd);
    }

lbl_CLEANUP:
    /*
     * Release the renderer since 'GetRenderer()' acquired it; also release the window
     * itself.
     */
    mainWndRd->VT->Release(mainWndRd);
    mainWnd->VT->Release(mainWnd);

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


NkVoid *NK_CALL NkApplicationQueryInstance(_In_ NkUuid const *clsId) {
    NK_ASSERT(clsId != NULL, NkErr_InParameter);

    /* Query the component. */
    NkIBase *resPtr;
    NkErrorCode errCode = NkHashtableAt(
        gl_Application.mp_compReg,
        &(NkHashtableKey const){ .mp_uuidKey = (NkUuid *)clsId },
        (NkVoid **)&resPtr
    );
    if (errCode != NkErr_Ok) {
        char uuidBuf[NK_UUIDLEN];

        NK_LOG_ERROR("Failed to query component with UUID {%s}.", NkUuidToString(clsId, uuidBuf));
        return NULL;
    }

    /*
     * Component could be queried. As always, add reference. Caller must use '::Release()'
     * on the returned instance.
     */
    resPtr->VT->AddRef(resPtr);
    return (NkVoid *)resPtr;
}

NkVoid NK_CALL NkApplicationReplaceInstance(_In_ NkUuid const *clsId, _Inout_opt_ NkIBase *newCompRef) {
    NK_ASSERT(clsId != NULL, NkErr_InParameter);

    /*
     * If the component does not exist, still add it. Erase the old entry first. Erasing
     * the element from the hashtable will decrease the associated NkOM object's
     * reference count.
     */
    NK_IGNORE_RETURN_VALUE(
        NkHashtableErase(
            gl_Application.mp_compReg, 
            &(NkHashtableKey const){ .mp_uuidKey = (NkUuid *)clsId }
        )
    );

    /* Add the entry with the new NkOM component instance. */
    if (newCompRef != NULL) {
        NkErrorCode errCode = NkHashtableInsert(gl_Application.mp_compReg, &(NkHashtablePair const){
            .m_keyVal    = (NkHashtableKey){ .mp_uuidKey = (NkUuid *)clsId },
            .mp_valuePtr = (NkVoid *)newCompRef
        });

        if (errCode != NkErr_Ok) {
            char uuidBuf[NK_UUIDLEN];

            NK_LOG_INFO("Failed to register NkOM instance with UUID {%s}.", NkUuidToString(clsId, uuidBuf));
        }
        newCompRef->VT->AddRef(newCompRef);
    }
}


NkBoolean NK_CALL NkApplicationIsStandalone(NkVoid) {
    return gl_Application.m_isStandalone;
}


#undef NK_NAMESPACE


/**
 * \page StartupOrder Startup Order
 *   Noriko, just like a lot of other software these days, is structured in a modular
 *   way. This means that different functionalities are implemented by logically and in
 *   some cases even physically separated units. Since these components are operating
 *   independently from each other but may still depend on each other, they must be
 *   started-up and shut-down in a specific order.
 * 
 *   The following diagram shows the component dependency graph.
 *   \dot
 *     digraph G {
 *         Noriko             -> Logging
 *         Noriko             -> PRNG
 *         Noriko             -> Allocators
 *         Noriko             -> Timers
 *         Allocators         -> NkOM
 *         Allocators         -> "Command Line"
 *         PRNG               -> "Command Line"
 *         NkOM               -> IAL
 *         NkOM               -> "Renderer Factory"
 *         NkOM               -> "Layer Stack"
 *         NkOM               -> Window
 *         NkOM               -> sqlite3
 *         NkOM               -> "Path Services"
 *         sqlite3            -> "Asset Manager"
 *         "Path Services"    -> "Asset Manager"
 *         "Renderer Factory" -> Renderer
 *         Window             -> Renderer
 *         "Asset Manager"    -> World
 *         Renderer           -> World
 *         IAL                -> World
 *         "Layer Stack"      -> World
 *     }
 *   \enddot
 *   
 *   Component startup and shutdown consist of three stages: <tt>pre</tt>, <tt>main</tt>,
 *   and <tt>post</tt>. While \c main is provided and implemented by the component itself,
 *   both \c pre and \c post are implemented by the module initialization facility,
 *   implemented by <tt>NkApplicationStartup()</tt> and <tt>NkApplicationShutdown()</tt>.
 *   Most work is done in <tt>main</tt>, either preparing the component for use or
 *   freeing its resources. \c Pre and \c post serve as auxiliary stages that are totally
 *   optional, and, if provided, do work related to the global application context. For
 *   example, Noriko registers components that are NkOM objects in a global component
 *   registry upon component startup. These components can then be queried by invoking
 *   <tt>NkApplicationQueryInstance()</tt> providing the corresponding component CLSID.
 *   This allows for the application and the user to register component implementations,
 *   crossing module boundaries and cleaning up the API. In this example, the creation of
 *   the component registry is invoked in the <tt>post-startup</tt> step of the \c NkOM
 *   module. Now, in order to satisfy symmetry requirements, the component registry must
 *   be shutdown in the <tt>pre-shutdown</tt> stage of the \c NkOM module. Generally,
 *   the mapping is as follows:
 * 
 *      \li pre-startup  -> post-shutdown
 *      \li post-startup -> pre-shutdown
 * 
 *   This ensures that all components and their resources are shutdown **in the reverse
 *   order** they were started-up.
 * 
 *   Based on the graph above, the current startup order in Noriko is as follows:
 *   \dot
 *     digraph F {
 *            Logging -> Allocators -> PRNG -> Timers -> "Command Line" -> NkOM -> "Path Services"
 *         -> IAL -> "Renderer Factory" -> "Layer Stack" -> "Main Window" -> Renderer -> sqlite3
 *         -> "Asset Manager" -> World
 *     }
 *   \enddot
 * 
 *   If any startup-stage of any component fails, the stage that fails is expected to
 *   roll-back all potentially succeeded initialization.
 */


