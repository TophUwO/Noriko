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
 * \file  log.c
 * \brief implements the interface to Noriko's logging facility
 *
 * The logger is designed as a singleton that is fully modular. Rather than implementing
 * all features directly, the logger defers the implementation of the "sinks" to modules
 * that use its API.
 */
#define NK_NAMESPACE "nk::log"


/* stdlib includes */
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

/* Noriko includes */
#include <include/Noriko/platform.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/log.h>
#include <include/Noriko/timer.h>
#include <include/Noriko/component.h>


/** \cond INTERNAL */
/**
 * \def   NK_LOG_PADDING(x)
 * \brief calculates the padding required for aligning log message severities based on a
 *        given log level
 * \param x string representation of log level
 */
#define NK_LOG_PADDING(x) (NkSize)(sizeof "CRITICAL" - sizeof x)


/**
 * \brief forward-declaration of the log level properties type for internal use 
 */
NK_NATIVE typedef struct NkLogLevelProperties NkLogLevelProperties;

/**
 * \struct __NkInt_LogSinkExtProperties
 * \brief  represents the log sink properties enhanced with synchronization facilities
 */
NK_NATIVE typedef struct __NkInt_LogSinkExtProperties {
    NkLogSinkProperties m_regProps; /**< public properties */

    NK_DECL_LOCK(m_mtxLock);        /**< synchronization object for sink */
} __NkInt_LogSinkExtProperties;

/**
 * \struct __NkInt_LogExtContext
 * \brief  represents the internal log context
 */
NK_NATIVE typedef struct __NkInt_LogExtContext {
    NkComponent;
    NK_DECL_LOCK(m_mtxLock);

    char                         *mp_msgBuffer;  /**< message buffer */
    char                         *mp_tsBuffer;   /**< timestamp textual representation buffer */
    NkSize                        m_msgLen;      /**< length of current message, in bytes */
    NkSize                        m_tsLen;       /**< length of current timestamp, in bytes */
    NkSize                        m_nOfSinks;    /**< current number of registered sinks */
    NkTimer                      *mp_tsFmtTimer; /**< timer used for formatting the timestamp */
    __NkInt_LogSinkExtProperties *mp_sinkArray;  /**< sink array */
    NkLogContext                  m_logCxt;      /**< global logger settings */
} __NkInt_LogExtContext;


/**
 * \brief represents the global log context instance
 * 
 * This instance is not constant since it may be altered at startup by command-line args
 * and such. However, after initialization, the data in \c m_logCxt is not touched, so it
 * may be accessed by multiple threads simultaneously without problems.
 */
NK_INTERNAL __NkInt_LogExtContext gl_LogContext = {
    .mp_msgBuffer  = NULL,
    .mp_tsBuffer   = NULL,
    .m_msgLen      = 0,
    .m_tsLen       = sizeof "%m-%d-%y %H:%M:%S",
    .m_nOfSinks    = 0,
    .mp_tsFmtTimer = NULL,
    .mp_sinkArray  = NULL,

    .m_logCxt = {
        .m_structSize = sizeof gl_LogContext.m_logCxt,
        .m_maxMsgSize = 2 << 13,
        .m_glMinLevel = NkLogLvl_None,
        .m_glMaxLevel = NkLogLvl_Critical,
        .mp_defFmtStr = NK_MAKE_STRING_VIEW_PTR("\033[97m"),
        .mp_rstFmtStr = NK_MAKE_STRING_VIEW_PTR("\033[0m"),
        .mp_tsFmtStr  = NK_MAKE_STRING_VIEW_PTR("%m-%d-%y %H:%M:%S"),
        .m_nSinks     = 32,

        .m_lvlProps = { { 0, NK_MAKE_RGBA(0, 0, 0, 0), NULL, NULL },
            { NK_LOG_PADDING("TRACE"),    NK_MAKE_RGB(120, 120, 120), NK_MAKE_STRING_VIEW_PTR("TRACE"),    NK_MAKE_STRING_VIEW_PTR("\033[90;40m") },
            { NK_LOG_PADDING("DEBUG"),    NK_MAKE_RGB(120, 120, 120), NK_MAKE_STRING_VIEW_PTR("DEBUG"),    NK_MAKE_STRING_VIEW_PTR("\033[90;40m") },
            { NK_LOG_PADDING("INFO"),     NK_MAKE_RGB(0, 255, 0),     NK_MAKE_STRING_VIEW_PTR("INFO"),     NK_MAKE_STRING_VIEW_PTR("\033[92;40m") },
            { NK_LOG_PADDING("WARNING"),  NK_MAKE_RGB(255, 255, 0),   NK_MAKE_STRING_VIEW_PTR("WARNING"),  NK_MAKE_STRING_VIEW_PTR("\033[33;40m") },
            { NK_LOG_PADDING("ERROR"),    NK_MAKE_RGB(255, 0, 0),     NK_MAKE_STRING_VIEW_PTR("ERROR"),    NK_MAKE_STRING_VIEW_PTR("\033[91;40m") },
            { NK_LOG_PADDING("CRITICAL"), NK_MAKE_RGB(150, 0, 0),     NK_MAKE_STRING_VIEW_PTR("CRITICAL"), NK_MAKE_STRING_VIEW_PTR("\033[97;41m") }
        }
    }
};
static_assert(NK_ARRAYSIZE(gl_LogContext.m_logCxt.m_lvlProps) == __NkLogLvl_Count__, "Log level table size mismatch!");


/*
 * implementation of the log callbacks for the built-in debug console logger (host
 * console window)
 * 
 * This logger is not used in deploy builds.
 */
#pragma region Debug Console Logger
/** \cond */
NK_INTERNAL NkErrorCode NK_CALL __NkInt_ConLogOnInit(
    _In_    NkLogSinkHandle sinkHandle,
    _Inout_ NkLogSinkProperties *sinkPropsPtr
) {
    NK_UNREFERENCED_PARAMETER(sinkHandle);
    NK_UNREFERENCED_PARAMETER(sinkPropsPtr);

    /* Initialize console window. */
#if (defined NK_TARGET_WINDOWS)
    /* Redirect standard output to console window. */
    if (AllocConsole()) {
        FILE *tmpFile = NULL;

        freopen_s(&tmpFile, "CONOUT$", "w", stdout);
    }
    /* Set console codepage to UTF-8. */
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    /* Enable Vt-100 sequence processing. */
    DWORD  conMode;
    HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(stdOutHandle, &conMode);
    conMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(stdOutHandle, conMode);
#endif

    return NkErr_Ok;
}

NK_INTERNAL NkErrorCode NK_CALL __NkInt_ConOnUninit(
    _In_    NkLogSinkHandle sinkHandle,
    _Inout_ NkLogSinkProperties *sinkPropsPtr
) {
    NK_UNREFERENCED_PARAMETER(sinkHandle);
    NK_UNREFERENCED_PARAMETER(sinkPropsPtr);

    /* Destroy console window. */
#if (defined NK_TARGET_WINDOWS)
    /*
     * Reset format right before exiting so that subsequent debug messages, etc.
     * generated by the debugger are not looking funny.
     */
    fputs(gl_LogContext.m_logCxt.mp_rstFmtStr->mp_dataPtr, stdout);

    FreeConsole();
#endif

    return NkErr_Ok;
}

NK_INTERNAL NkVoid NK_CALL __NkInt_ConOnLog(
    _In_     NkLogSinkHandle sinkHandle,
    _In_     NkLogLevel lvlId,
    _In_     NkStringView const *tsPtr,
    _In_     NkStringView const *fmtMsgPtr,
    _In_opt_ NkLogFrame const *framePtr,
    _Inout_  NkLogSinkProperties *sinkPropsPtr
) {
    NK_UNREFERENCED_PARAMETER(sinkHandle);
    NK_UNREFERENCED_PARAMETER(framePtr);
    NK_UNREFERENCED_PARAMETER(sinkPropsPtr);
    
    /* Retrieve log level output properties. */
    NkLogLevelProperties *logLvlPropsPtr = (NkLogLevelProperties *)&gl_LogContext.m_logCxt.m_lvlProps[lvlId];

    /* Compose log message from the "compiled" components. */
    if (lvlId ^ NkLogLvl_None) {
        /* If the logging level is "none", do not print the level or timestamp information. */
        fputs("[", stdout);
        fputs(tsPtr->mp_dataPtr, stdout);
        fputs("] <", stdout);
        fputs(logLvlPropsPtr->mp_lvlFmtStr->mp_dataPtr, stdout);
        fputs(logLvlPropsPtr->mp_lvlStrRep->mp_dataPtr, stdout);
        fputs(gl_LogContext.m_logCxt.mp_rstFmtStr->mp_dataPtr, stdout);
        fputs(">", stdout);
        for (NkInt32 i = 0; i <= logLvlPropsPtr->m_nSpace; i++)
            fputc(' ', stdout);
    }
    fputs(fmtMsgPtr->mp_dataPtr, stdout);
    fputc('\n', stdout);
}
/** \endcond */
#pragma endregion


/**
 * \brief formats the new log message and also updates the timestamp if necessary
 * \param [in] fmtStr pointer to the format template of the current log message
 * \param [in] vlArgs extra arguments to format the message with
 * \note  The timestamp is only updated when its current representation would not align
 *        with the currently saved version. This is the case if the last log message is
 *        at most one second in the past.
 */
NK_INTERNAL NkVoid __NkInt_LogFormatMessageAndTimestamp(
    _In_opt_ _Format_str_ char const *fmtStr,
    _In_opt_              va_list vlArgs
) {
    /*
     * If the first parameter is NULL, use this as an internal signal to only format the
     * timestamp, independently from the timer. This behavior is not publicly documented
     * and only used by the logging facility internally.
     */
    if (fmtStr == NULL)
        goto lbl_FMTTS;

    /* Format string. */
    int const nCharsWritten = vsnprintf(
        gl_LogContext.mp_msgBuffer,
        gl_LogContext.m_logCxt.m_maxMsgSize * sizeof *gl_LogContext.mp_msgBuffer,
        fmtStr,
        vlArgs
    );
    gl_LogContext.m_msgLen = (NkSize)nCharsWritten;

    /*
     * If the time since the last timestamp formatting has changed sufficiently, reformat
     * the timestamp.
     */
    if (NkElapsedTimerGetAs(gl_LogContext.mp_tsFmtTimer, NkTiPrec_Seconds) >= 1.0) {
lbl_FMTTS:
        NkInt64 currLTime;
        struct tm currTime;

        /* Get current time. */
        _time64(&currLTime);
        localtime_s(&currTime, &currLTime);
        /* Reformat timestamp. */
        strftime(
            gl_LogContext.mp_tsBuffer,
            sizeof *gl_LogContext.mp_tsBuffer * (gl_LogContext.m_tsLen + 1),
            gl_LogContext.m_logCxt.mp_tsFmtStr->mp_dataPtr,
            &currTime
        );

        /* Restart timer. */
        NkTimerRestart((NkTimer *)gl_LogContext.mp_tsFmtTimer);
    }
}

/**
 * \brief  checks whether the given log level is enabled for a sink
 * \param  [in] sProps pointer to the properties for the sink
 * \param  [in] lvlId numeric level ID
 * \return non-zero if the log level is enabled for the sink, zero if it is not
 */
NK_INTERNAL NkBoolean __NkInt_LogIsLevelEnabledForSink(
    _In_ __NkInt_LogSinkExtProperties const *sProps,
    _In_ NkLogLevel lvlId
) {
    return NK_INRANGE_INCL(lvlId, sProps->m_regProps.m_minLevel, sProps->m_regProps.m_maxLevel);
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkLogInitialize(NkVoid) {
    NK_ENSURE_NOT_INITIALIZED(gl_LogContext);
    NK_INITIALIZE(gl_LogContext);

    /* Allocate log buffer. */
    NkErrorCode errorCode = NkAllocateMemory(
        NK_MAKE_ALLOCATION_CONTEXT(),
        sizeof *gl_LogContext.mp_msgBuffer * gl_LogContext.m_logCxt.m_maxMsgSize,
        0,
        NK_TRUE,
        &gl_LogContext.mp_msgBuffer
    );
    if (errorCode != NkErr_Ok)
        goto lbl_ONERROR;

    /* Allocate timestamp buffer. */
    errorCode = NkAllocateMemory(
        NK_MAKE_ALLOCATION_CONTEXT(),
        sizeof *gl_LogContext.mp_tsBuffer * gl_LogContext.m_tsLen,
        0,
        NK_TRUE,
        &gl_LogContext.mp_tsBuffer
    );
    if (errorCode != NkErr_Ok)
        goto lbl_ONERROR;

    /* Allocate sink array. */
    errorCode = NkAllocateMemory(
        NK_MAKE_ALLOCATION_CONTEXT(),
        sizeof *gl_LogContext.mp_sinkArray * gl_LogContext.m_logCxt.m_nSinks,
        0,
        NK_TRUE,
        &gl_LogContext.mp_sinkArray
    );
    if (errorCode != NkErr_Ok)
        goto lbl_ONERROR;

    /* Create the timestamp timer. */
    errorCode = NkTimerCreate(NkTiType_Elapsed, NK_TRUE, (NkTimer **)&gl_LogContext.mp_tsFmtTimer);
    if (errorCode != NkErr_Ok)
        goto lbl_ONERROR;
    /* Format timestamp. */
    __NkInt_LogFormatMessageAndTimestamp(NULL, NULL);
    NK_INITLOCK(gl_LogContext.m_mtxLock);

#if (!defined NK_CONFIG_DEPLOY)
    /* Register built-in log sinks. */
    NkLogSinkProperties dbgconLogProps = {
        .m_structSize      = sizeof dbgconLogProps,
        .m_sinkIdent       = NK_MAKE_STRING_VIEW("nk_dbgcon"),
        .m_minLevel        = NkLogLvl_Trace,
        .m_maxLevel        = NkLogLvl_Critical,
        .mp_extraCxt       = NULL,
        .mp_fnOnSinkInit   = (NkLogSinkInitFn)&__NkInt_ConLogOnInit,
        .mp_fnOnSinkUninit = (NkLogSinkUninitFn)&__NkInt_ConOnUninit,
        .mp_fnOnSinkWrite  = (NkLogSinkWriteFn)&__NkInt_ConOnLog
    };
    NkLogSinkHandle dummyHandle;
    NK_IGNORE_RETURN_VALUE(NkLogRegisterSink(&dbgconLogProps, &dummyHandle));
#endif

    NK_LOG_INFO("init: logging");
    return NkErr_Ok;

lbl_ONERROR:
    NkFreeMemory(gl_LogContext.mp_msgBuffer);
    NkFreeMemory(gl_LogContext.mp_tsBuffer);

    NK_UNINITIALIZE(gl_LogContext);
    return errorCode;
}

NkVoid NK_CALL NkLogUninitialize(NkVoid) {
    NK_ENSURE_INITIALIZED_VOID(gl_LogContext);

    NK_LOG_INFO("shutdown: logging");
    NK_UNINITIALIZE(gl_LogContext);
    /*
     * Traverse the entire sink array and run sink-specific uninitialization on all of
     * them.
     */
    for (NkSize i = 0, j = 0; i < gl_LogContext.m_logCxt.m_nSinks && j < gl_LogContext.m_nOfSinks; i++) {
        __NkInt_LogSinkExtProperties *sinkProps = (__NkInt_LogSinkExtProperties *)&gl_LogContext.mp_sinkArray[i];
        if (sinkProps->m_regProps.m_structSize == 0 || sinkProps->m_regProps.mp_fnOnSinkUninit == NULL)
            continue;

        /* Call 'onUninit' handler. */
        (*sinkProps->m_regProps.mp_fnOnSinkUninit)((NkLogSinkHandle)i, sinkProps->m_regProps.mp_extraCxt);
    }
    NkFreeMemory(gl_LogContext.mp_sinkArray);

    NkFreeMemory(gl_LogContext.mp_msgBuffer);
    NkFreeMemory(gl_LogContext.mp_tsBuffer);
    NkTimerDestroy((NkTimer **)&gl_LogContext.mp_tsFmtTimer);
    NK_DESTROYLOCK(gl_LogContext.m_mtxLock);
}

NkVoid NK_CALL NkLogQueryContext(_Out_ NkLogContext *cxtStructPtr) {
    NK_ENSURE_INITIALIZED_VOID(gl_LogContext);

    /* Copy current state into result pointer. */
    memcpy(
        (NkVoid *)cxtStructPtr,
        (NkVoid const *)&gl_LogContext.m_logCxt,
        NK_MIN(cxtStructPtr->m_structSize, gl_LogContext.m_logCxt.m_structSize)
    );
}

_Return_ok_ NkErrorCode NK_CALL NkLogRegisterSink(
    _Inout_ NkLogSinkProperties *sinkPropsPtr,
    _Out_   NkLogSinkHandle *sinkHandlePtr
) {
    NK_ENSURE_INITIALIZED(gl_LogContext);
    NkErrorCode errorCode = NkErr_Ok;

    /* Check if we can even add a sink. If we cannot, return invalid sink ID. */
    NK_LOCK(gl_LogContext.m_mtxLock);
    if (gl_LogContext.m_nOfSinks >= gl_LogContext.m_logCxt.m_nSinks) {
        *sinkHandlePtr = -1;

        errorCode = NkErr_CapLimitExceeded;
        goto lbl_CLEANUP;
    }

    /*
     * Insert the sink into the first free slot. Free slots are marked with their
     * m_structSize member being set to 0.
     */
    for (NkSize i = 0; i < gl_LogContext.m_logCxt.m_nSinks; i++) {
        /* As soon as we reach an empty slot, attempt to insert sink in there. */
        if (gl_LogContext.mp_sinkArray[i].m_regProps.m_structSize == 0) {
            /* Call sink-specific 'onInit()' slot if possible. */
            if (sinkPropsPtr->mp_fnOnSinkInit != NULL) {
                NK_UNLOCK(gl_LogContext.m_mtxLock);
                errorCode = (*sinkPropsPtr->mp_fnOnSinkInit)((NkLogSinkHandle)i, sinkPropsPtr);
                NK_LOCK(gl_LogContext.m_mtxLock);

                if (errorCode != NkErr_Ok)
                    goto lbl_CLEANUP;
            }

            /* Copy sink properties. */
            memcpy(&gl_LogContext.mp_sinkArray[i], sinkPropsPtr, sizeof * sinkPropsPtr);
            /* Make sure the size field is set correctly. */
            gl_LogContext.mp_sinkArray[i].m_regProps.m_structSize = sizeof * sinkPropsPtr;
            /* Update registered sink count. */
            ++gl_LogContext.m_nOfSinks;

            /* Return slot ID for use as sink handle. */
            *sinkHandlePtr = (NkLogSinkHandle)i;
            goto lbl_CLEANUP;
        }
    }
    *sinkHandlePtr = -1;

lbl_CLEANUP:
    NK_UNLOCK(gl_LogContext.m_mtxLock);

    return errorCode;
}

_Return_ok_ NkErrorCode NK_CALL NkLogUnregisterSink(_Inout_ NkLogSinkHandle *sinkHandlePtr) {
    NK_ENSURE_INITIALIZED(gl_LogContext);
    NkErrorCode errorCode = NkErr_Ok;

    /* If the sink ID is invalid, do nothing. */
    NK_LOCK(gl_LogContext.m_mtxLock);
    if (NK_INRANGE_INCL(*sinkHandlePtr, 0, (NkInt32)gl_LogContext.m_logCxt.m_nSinks - 1) == NK_FALSE) {
        errorCode = NkErr_InOutParameter;

        goto lbl_CLEANUP;
    }

    /* Get pointer to sink slot in question. */
    __NkInt_LogSinkExtProperties *sProps = &gl_LogContext.mp_sinkArray[*sinkHandlePtr];

    /* Call sink-specific 'onUninit()' handler is possible. */
    if (sProps->m_regProps.mp_fnOnSinkUninit != NULL) {
        NK_UNLOCK(gl_LogContext.m_mtxLock);
        errorCode = (*sProps->m_regProps.mp_fnOnSinkUninit)(*sinkHandlePtr, &sProps->m_regProps);
        NK_LOCK(gl_LogContext.m_mtxLock);

        if (errorCode != NkErr_Ok)
            goto lbl_CLEANUP;
    }
    /* Remove the sink from the table. */
    sProps->m_regProps.m_structSize = 0;
    /*
     * Extra service: Set handle to -1 so that nobody will use the sink handle again for
     * shenanigans if they fail to set it to something invalid on their own.
     */
    *sinkHandlePtr = -1;

lbl_CLEANUP:
    return errorCode;
}

NkVoid NK_CALL NkLog(
    _In_opt_     NkLogFrame const *framePtr,
    _In_         NkLogLevel lvlId,
    _Format_str_ char const *fmtStr,
    ...
) {
    NK_ENSURE_INITIALIZED_VOID(gl_LogContext);

    // \todo Put static log msg array here to make it thread-safe.
    //char buf[2 << 20];

    NK_LOCK(gl_LogContext.m_mtxLock);
    /*
     * If the log level is globally disabled or there are currently no sinks registered,
     * do nothing.
     */
    NkBoolean const isEnabled = NK_INRANGE_INCL(
        lvlId,
        gl_LogContext.m_logCxt.m_glMinLevel,
        gl_LogContext.m_logCxt.m_glMaxLevel
    );
    if (!isEnabled || gl_LogContext.m_nOfSinks == 0)
        goto lbl_CLEANUP;

    /* Format message and timestamp (timestamp only if needed). */
    va_list vlArgs;
    va_start(vlArgs, fmtStr);
    __NkInt_LogFormatMessageAndTimestamp(fmtStr, vlArgs);
    va_end(vlArgs);

    /*
     * Propagate formatted message and additional context data to sinks. Stop when we
     * reach the end of the sink array or we have processed all registered sinks,
     * whatever comes first.
     */
    for (NkSize i = 0, j = 0; i < gl_LogContext.m_logCxt.m_nSinks && j < gl_LogContext.m_nOfSinks; i++) {
        /*
         * Get pointer to sink slot and check if the slot is used. If not, ignore. Also
         * ignore if the current sink is set to ignore the log level of the current
         * message.
         */
        __NkInt_LogSinkExtProperties *sinkProps = (__NkInt_LogSinkExtProperties *)&gl_LogContext.mp_sinkArray[i];
        if (sinkProps->m_regProps.m_structSize == 0 || __NkInt_LogIsLevelEnabledForSink(sinkProps, lvlId) == NK_FALSE)
            continue;

        /* Call the sink's 'onLog' handler. */
        NK_UNLOCK(gl_LogContext.m_mtxLock);
        NK_SYNCHRONIZED(sinkProps->m_mtxLock,
            (*sinkProps->m_regProps.mp_fnOnSinkWrite)(
                (NkLogSinkHandle)i,
                lvlId,
                &(NkStringView){ gl_LogContext.mp_tsBuffer,  gl_LogContext.m_tsLen  },
                &(NkStringView){ gl_LogContext.mp_msgBuffer, gl_LogContext.m_msgLen },
                framePtr,
                &sinkProps->m_regProps
            )
        );
        NK_LOCK(gl_LogContext.m_mtxLock);

        ++j;
    }

lbl_CLEANUP:
    NK_UNLOCK(gl_LogContext.m_mtxLock);
}


#undef NK_NAMESPACE


