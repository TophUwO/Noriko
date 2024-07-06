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
#define NK_NAMESPACE u8"nk::log"


/* stdlib includes */
#include <stdarg.h>
#include <stdio.h>
#include <time.h>

/* Noriko includes */
#include <include/Noriko/platform.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/log.h>
#include <include/Noriko/timer.h>

/**
 * \internal
 * \def   NK_LOG_PADDING(x)
 * \brief calculates the padding required for aligning log message severities based on a
 *        given log level
 * \param x string representation of log level
 * \endinternal
 */
#define NK_LOG_PADDING(x) (NkSize)(sizeof u8"CRITICAL" - sizeof x)


/**
 * \brief forward-declaration of the log level properties type for internal use 
 */
typedef struct NkLogLevelProperties NkLogLevelProperties;

/**
 * \struct NkInternalLogContext
 * \brief  represents the internal log context
 */
struct NkInternalLogContext {
    NkBoolean            m_isInit;      /**< whether or not the logging facility is initialized and ready */
    char                *mp_msgBuffer;  /**< message buffer */
    char                *mp_tsBuffer;   /**< timestamp textual representation buffer */
    NkSize               m_msgLen;      /**< length of current message, in bytes */
    NkSize               m_tsLen;       /**< length of current timestamp, in bytes */
    NkSize               m_nOfSinks;    /**< current number of registered sinks */
    NkElapsedTimer      *mp_tsFmtTimer; /**< timer used for formatting the timestamp */
    NkLogSinkProperties *mp_sinkArray;  /**< sink array */
    NkLogContext         m_logCxt;      /**< global logger settings */
};


/**
 * \internal
 * \brief represents the global log context instance
 * 
 * This instance is not constant since it may be altered at startup by command-line args
 * and such. However, after initialization, the data here is not touched, so it may be
 * accessed by multiple threads simultaneously without problems.
 * \endinternal
 */
NK_INTERNAL struct NkInternalLogContext gl_logContext = {
    .m_isInit      = NK_FALSE,
    .mp_msgBuffer  = NULL,
    .mp_tsBuffer   = NULL,
    .m_msgLen      = 0,
    .m_tsLen       = sizeof u8"%m-%d-%y %H:%M:%S",
    .m_nOfSinks    = 0,
    .mp_tsFmtTimer = NULL,
    .mp_sinkArray  = NULL,

    .m_logCxt = {
        .m_structSize = sizeof gl_logContext.m_logCxt,
        .m_maxMsgSize = 2 << 13,
        .m_glMinLevel = NkLogLvl_Trace,
        .m_glMaxLevel = NkLogLvl_Critical,
        .mp_defFmtStr = NK_MAKE_STRING_VIEW_PTR(u8"\033[97m"),
        .mp_rstFmtStr = NK_MAKE_STRING_VIEW_PTR(u8"\033[0m"),
        .mp_tsFmtStr  = NK_MAKE_STRING_VIEW_PTR(u8"%m-%d-%y %H:%M:%S"),
        .m_nSinks     = 32,

        .m_lvlProps = { { 0, NULL, NULL },
            { NK_LOG_PADDING(u8"TRACE"),    NK_MAKE_STRING_VIEW_PTR(u8"TRACE"),    NK_MAKE_STRING_VIEW_PTR(u8"\033[90;40m") },
            { NK_LOG_PADDING(u8"DEBUG"),    NK_MAKE_STRING_VIEW_PTR(u8"DEBUG"),    NK_MAKE_STRING_VIEW_PTR(u8"\033[90;40m") },
            { NK_LOG_PADDING(u8"INFO"),     NK_MAKE_STRING_VIEW_PTR(u8"INFO"),     NK_MAKE_STRING_VIEW_PTR(u8"\033[92;40m") },
            { NK_LOG_PADDING(u8"WARNING"),  NK_MAKE_STRING_VIEW_PTR(u8"WARNING"),  NK_MAKE_STRING_VIEW_PTR(u8"\033[33;40m") },
            { NK_LOG_PADDING(u8"ERROR"),    NK_MAKE_STRING_VIEW_PTR(u8"ERROR"),    NK_MAKE_STRING_VIEW_PTR(u8"\033[91;40m") },
            { NK_LOG_PADDING(u8"CRITICAL"), NK_MAKE_STRING_VIEW_PTR(u8"CRITICAL"), NK_MAKE_STRING_VIEW_PTR(u8"\033[97;41m") }
        }
    }
};
static_assert(NK_ARRAYSIZE(gl_logContext.m_logCxt.m_lvlProps) == __NkLogLvl_Count__, u8"Log level table size mismatch!");


/*
 * implementation of the log callbacks for the built-in debug console logger (host
 * console window)
 * 
 * This logger is not used in deploy builds.
 */
#pragma region Debug Console Logger
/** \cond */
NK_INTERNAL NkVoid NK_CALL __NkInternal_ConLogOnInit(_In_ NkLogSinkHandle sinkHandle, _In_opt_ NkVoid *extraCxtPtr) {
    NK_UNREFERENCED_PARAMETER(sinkHandle);
    NK_UNREFERENCED_PARAMETER(extraCxtPtr);

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
    HANDLE stdOutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  conMode;
    GetConsoleMode(stdOutHandle, &conMode);
    conMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(stdOutHandle, conMode);
#endif
}

NK_INTERNAL NkVoid NK_CALL __NkInternal_ConOnUninit(_In_ NkLogSinkHandle sinkHandle, _In_opt_ NkVoid *extraCxtPtr) {
    NK_UNREFERENCED_PARAMETER(sinkHandle);
    NK_UNREFERENCED_PARAMETER(extraCxtPtr);

    /* Destroy console window. */
#if (defined NK_TARGET_WINDOWS)
    /*
     * Reset format right before exiting so that subsequent debug messages, etc.
     * generated by the debugger are not looking funny.
     */
    fputs(gl_logContext.m_logCxt.mp_rstFmtStr->mp_dataPtr, stdout);

    FreeConsole();
#endif
}

NK_INTERNAL NkVoid NK_CALL __NkInternal_ConOnLog(
    _In_     NkLogLevel lvlId,
    _In_opt_ NkLogFrame const *framePtr,
    _In_     NkStringView const *tsPtr,
    _In_     NkStringView const *fmtMsgPtr,
    _Inout_  NkLogSinkProperties *sinkPropsPtr
) {
    NK_UNREFERENCED_PARAMETER(framePtr);
    NK_UNREFERENCED_PARAMETER(sinkPropsPtr);

    /* Retrieve log level output properties. */
    NkLogLevelProperties *logLvlPropsPtr = (NkLogLevelProperties *)&gl_logContext.m_logCxt.m_lvlProps[lvlId];

    /* Compose log message from the "compiled" components. */
    fputs("[", stdout);
    fputs(tsPtr->mp_dataPtr, stdout);
    fputs("] <", stdout);
    fputs(logLvlPropsPtr->mp_lvlFmtStr->mp_dataPtr, stdout);
    fputs(logLvlPropsPtr->mp_lvlStrRep->mp_dataPtr, stdout);
    fputs(gl_logContext.m_logCxt.mp_rstFmtStr->mp_dataPtr, stdout);
    fputs(">", stdout);
    for (NkInt32 i = 0; i <= logLvlPropsPtr->m_nSpace; i++)
        fputc(' ', stdout);
    fputs(fmtMsgPtr->mp_dataPtr, stdout);
    fputc('\n', stdout);
}
/** \endcond */
#pragma endregion


/**
 * \internal
 * \brief formats the new log message and also updates the timestamp if necessary
 * \param [in] fmtStr pointer to the format template of the current log message
 * \param [in] vlArgs extra arguments to format the message with
 * \note  The timestamp is only updated when its current representation would not align
 *        with the currently saved version. This is the case if the last log message is
 *        at most one second in the past.
 * \endinternal
 */
NK_INTERNAL NkVoid NK_CALL __NkInternal_LogFormatMessageAndTimestamp(
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
        gl_logContext.mp_msgBuffer,
        gl_logContext.m_logCxt.m_maxMsgSize * sizeof *gl_logContext.mp_msgBuffer,
        fmtStr,
        vlArgs
    );
    NK_ASSERT(nCharsWritten >= 0, NkErr_InParameter);
    gl_logContext.m_msgLen = (NkSize)nCharsWritten;

    /*
     * If the time since the last timestamp formatting has changed sufficiently, reformat
     * the timestamp.
     */
    if (NkElapsedTimerGetAs(gl_logContext.mp_tsFmtTimer, NkTiPrec_Seconds) >= 1.0) {
lbl_FMTTS:
        NkInt64 currLTime;
        struct tm currTime;

        /* Get current time. */
        _time64(&currLTime);
        localtime_s(&currTime, &currLTime);
        /* Reformat timestamp. */
        strftime(
            gl_logContext.mp_tsBuffer,
            sizeof *gl_logContext.mp_tsBuffer * (gl_logContext.m_tsLen + 1),
            gl_logContext.m_logCxt.mp_tsFmtStr->mp_dataPtr,
            &currTime
        );

        /* Restart timer. */
        NkTimerRestart((NkTimer *)gl_logContext.mp_tsFmtTimer);
    }
}

/**
 * \internal
 * \brief validates the sink properties for a new sink
 * \param [in] sinkPropsPtr pointer to a NkLogSinkProperties instance that is to be
 *             validated
 * \endinternal
 */
NK_INTERNAL NkVoid NK_CALL __NkInternal_LogValidateSinkProperties(_In_ NkLogSinkProperties const *sinkPropsPtr) {
    NK_ASSERT(sinkPropsPtr->m_structSize > 0, NkErr_InParameter);
    NK_ASSERT(sinkPropsPtr->m_maxLevel >= sinkPropsPtr->m_minLevel, NkErr_InvalidRange);
    NK_ASSERT(NK_INRANGE_INCL(sinkPropsPtr->m_minLevel, 0, __NkLogLvl_Count__ - 1), NkErr_InvalidRange);
    NK_ASSERT(NK_INRANGE_INCL(sinkPropsPtr->m_maxLevel, 0, __NkLogLvl_Count__ - 1), NkErr_InvalidRange);

    NK_ASSERT(sinkPropsPtr->mp_fnOnLog != NULL, NkErr_CallbackParameter);
}


_Return_ok_ NkErrorCode NK_CALL NkLogInitialize(NkVoid) {
    NK_ASSERT(gl_logContext.m_isInit == NK_FALSE, NkErr_ComponentState);

    /* Allocate log buffer. */
    NkErrorCode errorCode = NkAllocateMemory(
        NK_MAKE_ALLOCATION_CONTEXT(),
        sizeof *gl_logContext.mp_msgBuffer * gl_logContext.m_logCxt.m_maxMsgSize,
        0,
        NK_TRUE,
        &gl_logContext.mp_msgBuffer
    );
    if (errorCode != NkErr_Ok)
        goto lbl_ONERROR;

    /* Allocate timestamp buffer. */
    errorCode = NkAllocateMemory(
        NK_MAKE_ALLOCATION_CONTEXT(),
        sizeof *gl_logContext.mp_tsBuffer * gl_logContext.m_tsLen,
        0,
        NK_TRUE,
        &gl_logContext.mp_tsBuffer
    );
    if (errorCode != NkErr_Ok)
        goto lbl_ONERROR;

    /* Allocate sink array. */
    errorCode = NkAllocateMemory(
        NK_MAKE_ALLOCATION_CONTEXT(),
        sizeof *gl_logContext.mp_sinkArray * gl_logContext.m_logCxt.m_nSinks,
        0,
        NK_TRUE,
        &gl_logContext.mp_sinkArray
    );
    if (errorCode != NkErr_Ok)
        goto lbl_ONERROR;

    /* Create the timestamp timer. */
    errorCode = NkTimerCreate(NkTiType_Elapsed, NK_TRUE, (NkTimer **)&gl_logContext.mp_tsFmtTimer);
    if (errorCode != NkErr_Ok)
        goto lbl_ONERROR;
    /* Format timestamp. */
    __NkInternal_LogFormatMessageAndTimestamp(NULL, NULL);
    /* Make logging facility ready. */
    gl_logContext.m_isInit = NK_TRUE;

#if (!defined NK_CONFIG_DEPLOY)
    /* Register built-in log sinks. */
    NkLogSinkProperties const dbgconLogProps = {
        .m_structSize = sizeof dbgconLogProps,
        .m_sinkIdent  = NK_MAKE_STRING_VIEW(u8"nk_dbgcon"),
        .m_minLevel   = NkLogLvl_Trace,
        .m_maxLevel   = NkLogLvl_Critical,
        .mp_extraCxt  = NULL,
        .m_sinkFlags  = NkLogSF_None,
        .mp_fnOnRegister = &__NkInternal_ConLogOnInit,
        .mp_fnOnUnregister = &__NkInternal_ConOnUninit,
        .mp_fnOnLog = &__NkInternal_ConOnLog
    };
    NkLogSinkHandle dummyHandle;
    NK_IGNORE_RETURN_VALUE(NkLogRegisterSink(&dbgconLogProps, &dummyHandle));
#endif

    NK_LOG_INFO("init: logging");
    return NkErr_Ok;

lbl_ONERROR:
    NkFreeMemory(gl_logContext.mp_msgBuffer);
    NkFreeMemory(gl_logContext.mp_tsBuffer);

    return errorCode;
}

NkVoid NK_CALL NkLogUninitialize(NkVoid) {
    NK_ASSERT(gl_logContext.m_isInit == NK_TRUE, NkErr_ComponentState);

    NK_LOG_INFO("shutdown: logging");

    /* Update the state. */
    gl_logContext.m_isInit = NK_FALSE;

    /*
     * Traverse the entire sink array and run sink-specific uninitialization on all of
     * them.
     */
    for (NkSize i = 0, j = 0; i < gl_logContext.m_logCxt.m_nSinks && j < gl_logContext.m_nOfSinks; i++) {
        NkLogSinkProperties *sinkProps = (NkLogSinkProperties *)&gl_logContext.mp_sinkArray[i];
        if (sinkProps->m_structSize == 0 || sinkProps->mp_fnOnUnregister == NULL)
            continue;

        /* Call 'onUninit' handler. */
        (*sinkProps->mp_fnOnUnregister)((NkLogSinkHandle)i, sinkProps->mp_extraCxt);
    }
    NkFreeMemory(gl_logContext.mp_sinkArray);

    NkFreeMemory(gl_logContext.mp_msgBuffer);
    NkFreeMemory(gl_logContext.mp_tsBuffer);
    NkTimerDestroy((NkTimer **)&gl_logContext.mp_tsFmtTimer);
}

NkVoid NK_CALL NkLogQueryContext(_Out_ NkLogContext *cxtStructPtr) {
    NK_ASSERT(gl_logContext.m_isInit == NK_TRUE, NkErr_ComponentState);

    /* Copy current state into result pointer. */
    memcpy(
        (NkVoid *)cxtStructPtr,
        (NkVoid const *)&gl_logContext.m_logCxt,
        NK_MIN(cxtStructPtr->m_structSize, gl_logContext.m_logCxt.m_structSize)
    );
}

_Return_ok_ NkErrorCode NK_CALL NkLogRegisterSink(
    _In_  NkLogSinkProperties const *sinkPropsPtr,
    _Out_ NkLogSinkHandle *sinkHandlePtr
) {
    NK_ASSERT(sinkPropsPtr != NULL, NkErr_InParameter);
    NK_ASSERT(sinkHandlePtr != NULL, NkErr_OutParameter);
    NK_ASSERT(gl_logContext.m_isInit == NK_TRUE, NkErr_ComponentState);
    
    /* Check if we can even add a sink. If we cannot, return invalid sink ID. */
    if (gl_logContext.m_nOfSinks >= gl_logContext.m_logCxt.m_nSinks) {
        *sinkHandlePtr = -1;

        return NkErr_CapLimitExceeded;
    }
    __NkInternal_LogValidateSinkProperties(sinkPropsPtr);

    /*
     * Insert the sink into the first free slot. Free slots are marked with their
     * m_structSize member being set to 0.
     */
    for (NkSize i = 0; i < gl_logContext.m_logCxt.m_nSinks; i++)
        if (gl_logContext.mp_sinkArray[i].m_structSize == 0) {
            *gl_logContext.mp_sinkArray = *sinkPropsPtr;

            /* Make sure the size field is set correctly. */
            gl_logContext.mp_sinkArray[i].m_structSize = sizeof * sinkPropsPtr;
            /* Update registered sink count. */
            ++gl_logContext.m_nOfSinks;

            /* Call sink-specific 'onInit()' slot if possible. */
            if (sinkPropsPtr->mp_fnOnRegister != NULL)
                (*sinkPropsPtr->mp_fnOnRegister)((NkLogSinkHandle)i, sinkPropsPtr->mp_extraCxt);
            /* Return slot ID for use as sink handle. */
            *sinkHandlePtr = (NkLogSinkHandle)i;
            return NkErr_Ok;
        }

    /* This should theoretically never happen. */
    NK_ASSERT_EXTRA(
        NK_FALSE,
        NkErr_ObjectState,
        "The sink array is marked as non-full but no sink slot is marked free. Weird."
    );
    *sinkHandlePtr = -1;
    return NkErr_Unknown;
}

_Return_ok_ NkErrorCode NK_CALL NkLogUnregisterSink(_Inout_ NkLogSinkHandle *sinkHandlePtr) {
    NK_ASSERT(sinkHandlePtr != NULL, NkErr_InOutParameter);
    NK_ASSERT(gl_logContext.m_isInit == NK_TRUE, NkErr_ComponentState);
    NK_ASSERT(gl_logContext.m_nOfSinks > 0, NkErr_ComponentState);

    /* If the sink ID is invalid, do nothing. */
    if (!NK_INRANGE_INCL(*sinkHandlePtr, 0, (NkInt32)gl_logContext.m_logCxt.m_nSinks - 1))
        return NkErr_InOutParameter;

    /* Get pointer to sink slot in question. */
    NkLogSinkProperties *sProps = &gl_logContext.mp_sinkArray[*sinkHandlePtr];

    /* Call sink-specific 'onUninit()' handler is possible. */
    if (sProps->mp_fnOnUnregister != NULL)
        (*sProps->mp_fnOnUnregister)(*sinkHandlePtr, sProps->mp_extraCxt);
    /* Remove the sink from the table. */
    sProps->m_structSize = 0;
    /*
     * Extra service: Set handle to -1 so that nobody will use the sink handle again for
     * shenanigans if they fail to set it to something invalid on their own.
     */
    *sinkHandlePtr = 0;
    return NkErr_Ok;
}

NkVoid NK_CALL NkLog(
    _In_opt_     NkLogFrame const *framePtr,
    _In_         NkLogLevel lvlId,
    _Format_str_ char const *fmtStr,
    ...
) {
    NK_ASSERT(fmtStr != NULL, NkErr_InParameter);
    NK_ASSERT(NK_INRANGE_INCL(lvlId, 0, __NkLogLvl_Count__ - 1), NkErr_InParameter);
    NK_ASSERT(gl_logContext.m_isInit == NK_TRUE, NkErr_ComponentState);

    /* If the log level is disabled, do nothing. */
    NkBoolean const isEnabled = NK_INRANGE_INCL(
        lvlId,
        gl_logContext.m_logCxt.m_glMinLevel,
        gl_logContext.m_logCxt.m_glMaxLevel
    );
    if (!isEnabled)
        return;
    /* If there are no sinks registered, do nothing. */
    if (gl_logContext.m_nOfSinks == 0)
        return;

    /* Format message and timestamp (timestamp only if needed). */
    va_list vlArgs;
    va_start(vlArgs, fmtStr);
    __NkInternal_LogFormatMessageAndTimestamp(fmtStr, vlArgs);
    va_end(vlArgs);

    /*
     * Propagate formatted message and additional context data to sinks. Stop when we
     * reach the end of the sink array or we have processed all registered sinks,
     * whatever comes first.
     */
    for (NkSize i = 0, j = 0; i < gl_logContext.m_logCxt.m_nSinks && j < gl_logContext.m_nOfSinks; i++) {
        /* Get pointer to sink slot and check if the slot is used. If not, ignore. */
        NkLogSinkProperties *sinkProps = (NkLogSinkProperties *)&gl_logContext.mp_sinkArray[i];
        if (sinkProps->m_structSize == 0)
            continue;

        /* Call the sink's 'onLog' handler. */
        (*sinkProps->mp_fnOnLog)(
            lvlId,
            framePtr,
            &(NkStringView){ gl_logContext.mp_tsBuffer, gl_logContext.m_tsLen },
            &(NkStringView){ gl_logContext.mp_msgBuffer, gl_logContext.m_msgLen },
            sinkProps
            );
        ++j;
    }
}


#undef NK_NAMESPACE


