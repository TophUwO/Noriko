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


/** \cond INTERNAL */
/**
 * \def   NK_LOG_PADDING(x)
 * \brief calculates the padding required for aligning log message severities based on a
 *        given log level
 * \param x string representation of log level
 */
#define NK_LOG_PADDING(x) ((NkSize)(sizeof "FATAL" - sizeof x))

/**
 * \brief NK_LOG_TSSIZE
 * \brief size of the message timestamp buffer, in bytes (incl. <tt>NUL</tt>-terminator)
 */
#define NK_LOG_TSSIZE     ((NkSize)(1 << 5))
/**
 * \def   NK_LOG_MSGSIZE
 * \brief size of the message buffer, in bytes (incl. <tt>NUL</tt>-terminator)
 */
#define NK_LOG_MSGSIZE    ((NkSize)(1 << 12))
/**
 * \def   NK_LOG_NSINKS
 * \brief maximum number of sinks that can be registered at a time
 */
#define NK_LOG_NSINKS     ((NkSize)(1 << 6))


/**
 * \brief forward-declaration of the log level properties type for internal use 
 */
NK_NATIVE typedef struct NkLogLevelProperties NkLogLevelProperties;

/**
 * \struct __NkInt_LogSinkExtProps
 * \brief  represents the log sink properties enhanced with synchronization facilities
 */
NK_NATIVE typedef struct __NkInt_LogSinkExtProps {
    NkLogSinkProperties m_regProps; /**< public properties */

    NK_DECL_LOCK(m_mtxLock);        /**< synchronization object for sink */
} __NkInt_LogSinkExtProps;

/**
 * \struct __NkInt_LogExtContext
 * \brief  represents the internal log context
 */
NK_NATIVE typedef struct __NkInt_LogExtContext {
    NK_DECL_LOCK(m_mtxLock);

    NkSize                   m_nOfSinks;                 /**< current number of registered sinks */
    __NkInt_LogSinkExtProps  m_sinkArray[NK_LOG_NSINKS]; /**< sink array */
    NkLogContext             m_logCxt;                   /**< global logger settings */
} __NkInt_LogExtContext;


/**
 * \brief represents the global log context instance
 * 
 * This instance is not constant since it may be altered at startup by command-line args
 * and such. However, after initialization, the data in \c m_logCxt is not touched, so it
 * may be accessed by multiple threads simultaneously without problems.
 */
NK_INTERNAL __NkInt_LogExtContext gl_LogContext = {
    .m_nOfSinks = 0,
    .m_logCxt = {
        .m_structSize = sizeof gl_LogContext.m_logCxt,
        .m_maxMsgSize = NK_LOG_MSGSIZE,
        .m_glMinLevel = NkLogLvl_None,
        .m_glMaxLevel = NkLogLvl_Critical,
        .mp_defFmtStr = NK_MAKE_STRING_VIEW_PTR("\033[97m"),
        .mp_rstFmtStr = NK_MAKE_STRING_VIEW_PTR("\033[0m"),
        .mp_tsFmtStr  = NK_MAKE_STRING_VIEW_PTR("%m-%d-%y %H:%M:%S"),
        .m_maxSinkCnt = NK_LOG_NSINKS,

        .m_lvlProps = { { 0, NK_MAKE_RGBA(0, 0, 0, 0), NULL, NULL },
            { NK_LOG_PADDING("TRACE"), NK_MAKE_RGB(120, 120, 120), NK_MAKE_STRING_VIEW_PTR("TRACE"), NK_MAKE_STRING_VIEW_PTR("\033[90;40m") },
            { NK_LOG_PADDING("DEBUG"), NK_MAKE_RGB(120, 120, 120), NK_MAKE_STRING_VIEW_PTR("DEBUG"), NK_MAKE_STRING_VIEW_PTR("\033[90;40m") },
            { NK_LOG_PADDING("INFO"),  NK_MAKE_RGB(0, 255, 0),     NK_MAKE_STRING_VIEW_PTR("INFO"),  NK_MAKE_STRING_VIEW_PTR("\033[92;40m") },
            { NK_LOG_PADDING("WARN"),  NK_MAKE_RGB(255, 255, 0),   NK_MAKE_STRING_VIEW_PTR("WARN"),  NK_MAKE_STRING_VIEW_PTR("\033[33;40m") },
            { NK_LOG_PADDING("ERROR"), NK_MAKE_RGB(255, 0, 0),     NK_MAKE_STRING_VIEW_PTR("ERROR"), NK_MAKE_STRING_VIEW_PTR("\033[91;40m") },
            { NK_LOG_PADDING("FATAL"), NK_MAKE_RGB(150, 0, 0),     NK_MAKE_STRING_VIEW_PTR("FATAL"), NK_MAKE_STRING_VIEW_PTR("\033[97;41m") }
        }
    }
};
/* Make sure that the level table aligns with the numeric log level IDs. */
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
    _In_     char const *tsPtr,
    _In_     char const *fmtMsgPtr,
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
        fputs(tsPtr, stdout);
        fputs("] <", stdout);
        fputs(logLvlPropsPtr->mp_lvlFmtStr->mp_dataPtr, stdout);
        fputs(logLvlPropsPtr->mp_lvlStrRep->mp_dataPtr, stdout);
        fputs(gl_LogContext.m_logCxt.mp_rstFmtStr->mp_dataPtr, stdout);
        fputs(">", stdout);
        for (NkInt32 i = 0; i <= logLvlPropsPtr->m_nSpace; i++)
            fputc(' ', stdout);
    }
    fputs(fmtMsgPtr, stdout);
    fputc('\n', stdout);
}
/** \endcond */
#pragma endregion


/**
 * \brief formats the new log message and also updates the timestamp if necessary
 * \param [in] fmtStr pointer to the format template of the current log message
 * \param [in] vlArgs extra arguments to format the message with
 * \param [out] msgPtr pointer to the message buffer
 * \param [out] tsPtr pointer to the timestamp buffer
 * \param [out] msgSzPtr variable that will receive the size in bytes (that is, incl.
 *              <tt>NUL</tt>-terminator) of the message buffer
 * \param [out] tsSzPtr variable that will receive the size in bytes (that is, incl.
 *              <tt>NUL</tt>-terminator) of the timestamp buffer
 * \note  This function never writes beyond the hard boundaries of the buffers.
 */
NK_INTERNAL NkVoid __NkInt_LogFormatMessageAndTimestamp(
    _In_z_ _Format_str_          char const *fmtStr,
    _In_opt_                     va_list vlArgs,
    _Out_writes_(NK_LOG_MSGSIZE) char *msgPtr,
    _Out_writes_(NK_LOG_TSSIZE)  char *tsPtr,
    _Out_                        NkSize *msgSzPtr,
    _Out_                        NkSize *tsSzPtr
) {
    /* Format string. */
    *msgSzPtr = (NkSize)vsnprintf(msgPtr, NK_LOG_MSGSIZE, fmtStr, vlArgs);

    /* Format timestamp. */
    NkInt64 currLTime;
    struct tm currTime;
    /* Get current time. */
    _time64(&currLTime);
    localtime_s(&currTime, &currLTime);
    *tsSzPtr = strftime(
        tsPtr,
        NK_LOG_TSSIZE,
        gl_LogContext.m_logCxt.mp_tsFmtStr->mp_dataPtr,
        &currTime
    );
}

/**
 * \brief  checks whether the given log level is enabled for a sink
 * \param  [in] sProps pointer to the properties for the sink
 * \param  [in] lvlId numeric level ID
 * \return non-zero if the log level is enabled for the sink, zero if it is not
 */
NK_INTERNAL NkBoolean __NkInt_LogIsLevelEnabledForSink(
    _In_ __NkInt_LogSinkExtProps const *sProps,
    _In_ NkLogLevel lvlId
) {
    return NK_INRANGE_INCL(lvlId, sProps->m_regProps.m_minLevel, sProps->m_regProps.m_maxLevel);
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkLogInitialize(NkVoid) {
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
    NkErrorCode errorCode = NkLogRegisterSink(&dbgconLogProps, &dummyHandle);
    if (errorCode != NkErr_Ok)
        return errorCode;
#endif

    NK_LOG_INFO("startup: logging");
    return NkErr_Ok;
}

NkVoid NK_CALL NkLogUninitialize(NkVoid) {
    NK_LOG_INFO("shutdown: logging");

    /*
     * Traverse the entire sink array and run sink-specific uninitialization on all of
     * them.
     */
    for (NkSize i = 0, j = 0; i < gl_LogContext.m_logCxt.m_maxSinkCnt && j < gl_LogContext.m_nOfSinks; i++) {
        __NkInt_LogSinkExtProps *sinkProps = (__NkInt_LogSinkExtProps *)&gl_LogContext.m_sinkArray[i];
        if (sinkProps->m_regProps.m_structSize == 0 || sinkProps->m_regProps.mp_fnOnSinkUninit == NULL)
            continue;

        /* Call 'onUninit' handler. */
        (*sinkProps->m_regProps.mp_fnOnSinkUninit)((NkLogSinkHandle)i, sinkProps->m_regProps.mp_extraCxt);
    }

    NK_DESTROYLOCK(gl_LogContext.m_mtxLock);
}

NkVoid NK_CALL NkLogQueryContext(_Out_ NkLogContext *cxtStructPtr) {
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
    NkErrorCode errorCode = NkErr_Ok;

    /* Check if we can even add a sink. If we cannot, return invalid sink ID. */
    NK_LOCK(gl_LogContext.m_mtxLock);
    if (gl_LogContext.m_nOfSinks >= gl_LogContext.m_logCxt.m_maxSinkCnt) {
        *sinkHandlePtr = -1;

        errorCode = NkErr_CapLimitExceeded;
        goto lbl_CLEANUP;
    }

    /*
     * Insert the sink into the first free slot. Free slots are marked with their
     * m_structSize member being set to 0.
     */
    for (NkSize i = 0; i < gl_LogContext.m_logCxt.m_maxSinkCnt; i++) {
        /* As soon as we reach an empty slot, attempt to insert sink in there. */
        if (gl_LogContext.m_sinkArray[i].m_regProps.m_structSize == 0) {
            /* Call sink-specific 'onInit()' slot if possible. */
            if (sinkPropsPtr->mp_fnOnSinkInit != NULL) {
                NK_UNLOCK(gl_LogContext.m_mtxLock);
                errorCode = (*sinkPropsPtr->mp_fnOnSinkInit)((NkLogSinkHandle)i, sinkPropsPtr);
                NK_LOCK(gl_LogContext.m_mtxLock);

                if (errorCode != NkErr_Ok)
                    goto lbl_CLEANUP;
            }

            /* Copy sink properties. */
            memcpy(&gl_LogContext.m_sinkArray[i], sinkPropsPtr, sizeof * sinkPropsPtr);
            /* Make sure the size field is set correctly. */
            gl_LogContext.m_sinkArray[i].m_regProps.m_structSize = sizeof * sinkPropsPtr;
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
    NkErrorCode errorCode = NkErr_Ok;

    /* If the sink ID is invalid, do nothing. */
    NK_LOCK(gl_LogContext.m_mtxLock);
    if (NK_INRANGE_INCL(*sinkHandlePtr, 0, (NkInt32)gl_LogContext.m_logCxt.m_maxSinkCnt - 1) == NK_FALSE) {
        errorCode = NkErr_InOutParameter;

        goto lbl_CLEANUP;
    }

    /* Get pointer to sink slot in question. */
    __NkInt_LogSinkExtProps *sProps = &gl_LogContext.m_sinkArray[*sinkHandlePtr];

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

NkVoid NK_CALL NkLogWrite(
    _In_opt_     NkLogFrame const *framePtr,
    _In_         NkLogLevel lvlId,
    _Format_str_ char const *fmtStr,
    ...
) {
    /* Format message and timestamp (timestamp only if needed). */
    char msgBuf[NK_LOG_MSGSIZE], tsBuf[NK_LOG_TSSIZE];
    NkSize tsSize, msgSize;
    va_list vlArgs;
    va_start(vlArgs, fmtStr);
    __NkInt_LogFormatMessageAndTimestamp(fmtStr, vlArgs, msgBuf, tsBuf, &msgSize, &tsSize);
    va_end(vlArgs);

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

    /*
     * Propagate formatted message and additional context data to sinks. Stop when we
     * reach the end of the sink array or we have processed all registered sinks,
     * whatever comes first.
     */
    for (NkSize i = 0, j = 0; i < NK_ARRAYSIZE(gl_LogContext.m_sinkArray) && j < gl_LogContext.m_nOfSinks; i++) {
        /*
         * Get pointer to sink slot and check if the slot is used. If not, ignore. Also
         * ignore if the current sink is set to ignore the log level of the current
         * message.
         */
        __NkInt_LogSinkExtProps *sinkProps = (__NkInt_LogSinkExtProps *)&gl_LogContext.m_sinkArray[i];
        if (sinkProps->m_regProps.m_structSize == 0 || __NkInt_LogIsLevelEnabledForSink(sinkProps, lvlId) == NK_FALSE)
            continue;

        /* Call the sink's 'onLog' handler. */
        NK_UNLOCK(gl_LogContext.m_mtxLock);
        (*sinkProps->m_regProps.mp_fnOnSinkWrite)(
            (NkLogSinkHandle)i,
            lvlId,
            tsBuf,
            msgBuf,
            framePtr,
            &sinkProps->m_regProps
        );
        NK_LOCK(gl_LogContext.m_mtxLock);

        ++j;
    }

lbl_CLEANUP:
    NK_UNLOCK(gl_LogContext.m_mtxLock);
}


#undef NK_NAMESPACE


