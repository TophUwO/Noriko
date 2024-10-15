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

/* Noriko includes */
#include <include/Noriko/platform.h>
#include <include/Noriko/log.h>


/** \cond INTERNAL */
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
 * \def   NK_LOG_NDEV
 * \brief maximum number of devices that can be registered at a time
 */
#define NK_LOG_NDEV       ((NkSize)(1 << 6))


/**
 * \struct __NkInt_LogContext
 * \brief  represents the internal log context
 */
NK_NATIVE typedef struct __NkInt_LogContext {
    NK_DECL_LOCK(m_mtxLock);               /**< synchronization primitive */

    NkSize        m_nOfDev;                /**< current number of registered devices */
    NkLogLevel    m_glMinLevel;            /**< global minimum log level */
    NkLogLevel    m_glMaxLevel;            /**< global maximum log level */
    NkStringView  m_defTsFmt;              /**< default ts format string */
    NkILogDevice *m_devArray[NK_LOG_NDEV]; /**< device array */
} __NkInt_LogContext;

/**
 * \brief represents the global log context instance
 */
NK_INTERNAL __NkInt_LogContext gl_LogContext = {
    .m_nOfDev     = 0,
    .m_glMinLevel = NkLogLvl_None,
    .m_glMaxLevel = NkLogLvl_Critical,
    .m_defTsFmt   = NK_MAKE_STRING_VIEW("%m-%d-%y %H:%M:%S"),
    .m_devArray   = { NULL }
};

/* Define IID and CLSID of the default NkILogDevice interface. */
// { 3B93159F-704B-4B51-A19C-31D505D2729A }
NKOM_DEFINE_IID(NkILogDevice, { 0x3b93159f, 0x704b, 0x4b51, 0xa19c31d505d2729a });
// { 00000000-0000-0000-0000-000000000000 }
NKOM_DEFINE_CLSID(NkILogDevice, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });


/*
 * implementation of the log callbacks for the built-in debug console logger (host
 * console window)
 * 
 * This logger is not used in deploy builds.
 */
#pragma region Debug Console Logger
/**
 * \def   NK_LOG_PADDING(x)
 * \brief calculates the padding required for aligning log message severities based on a
 *        given log level
 * \param x string representation of log level
 */
 #define NK_LOG_PADDING(x) ((NkSize)(sizeof "FATAL" - sizeof x))


/**
 * \struct __NkInt_LogLevelStatic
 * \brief  contains static per-level information
 */
NK_NATIVE typedef struct __NkInt_LogLevelStatic {
    NkStringView m_lvlRep;   /**< textual representation of the level */
    NkStringView m_vt100Seq; /**< VT-100 formatting sequence */
    NkSize       m_padding;  /**< padding for alignment of log level string reps */
} __NkInt_LogLevelStatic;

/**
 * \brief actual instance of the static level infos 
 */
NK_INTERNAL __NkInt_LogLevelStatic const gl_LogLevelStaticInfo[] = {
    { NK_MAKE_STRING_VIEW(""),      NK_MAKE_STRING_VIEW("\033[97m"),    0                       },

    { NK_MAKE_STRING_VIEW("TRACE"), NK_MAKE_STRING_VIEW("\033[90;40m"), NK_LOG_PADDING("TRACE") },
    { NK_MAKE_STRING_VIEW("DEBUG"), NK_MAKE_STRING_VIEW("\033[90;40m"), NK_LOG_PADDING("DEBUG") },
    { NK_MAKE_STRING_VIEW("INFO"),  NK_MAKE_STRING_VIEW("\033[92;40m"), NK_LOG_PADDING("INFO")  },
    { NK_MAKE_STRING_VIEW("WARN"),  NK_MAKE_STRING_VIEW("\033[33;40m"), NK_LOG_PADDING("WARN")  },
    { NK_MAKE_STRING_VIEW("ERROR"), NK_MAKE_STRING_VIEW("\033[91;40m"), NK_LOG_PADDING("ERROR") },
    { NK_MAKE_STRING_VIEW("FATAL"), NK_MAKE_STRING_VIEW("\033[97;41m"), NK_LOG_PADDING("FATAL") }
};
/* Validate integrity. */
static_assert(NK_ARRAYSIZE(gl_LogLevelStaticInfo) == __NkLogLvl_Count__, "");

/**
 * \brief VT-100 reset format directive
 */
NK_INTERNAL NkStringView const gl_ResetFmtCmd = NK_MAKE_STRING_VIEW("\033[0m");
/**
 * \brief string containing only spaces to use for padding
 */
NK_INTERNAL NkStringView const gl_PaddingTempl = NK_MAKE_STRING_VIEW("                ");


/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_ConoutDev_AddRef(_Inout_ NkILogDevice *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub because static object. */
    return 1;
}

/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_ConoutDev_Release(_Inout_ NkILogDevice *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Same thing as in 'AddRef()'. */
    return 1;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_ConoutDev_QueryInterface(
    _Inout_  NkILogDevice *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);
    NK_UNREFERENCED_PARAMETER(self);

    if (NkUuidIsEqual(iId, NKOM_IIDOF(NkIBase)) || NkUuidIsEqual(iId, NKOM_IIDOF(NkILogDevice))) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_ConoutDev_OnInstall(_Inout_ NkILogDevice *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

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

NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_ConoutDev_OnUninstall(_Inout_ NkILogDevice *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Destroy console window. */
#if (defined NK_TARGET_WINDOWS)
    /*
    * Reset format right before exiting so that subsequent debug messages, etc.
    * generated by the debugger are not looking funny.
    */
    fputs(gl_ResetFmtCmd.mp_dataPtr, stdout);

    FreeConsole();
#endif

    return NkErr_Ok;
}

NK_INTERNAL NkVoid NK_CALL __NkInt_ConoutDev_OnMessage(
    _Inout_      NkILogDevice *self,
    _In_         NkLogLevel lvlId,
    _In_         NkLogMessageContext const *msgCxtPtr,
    _In_         NkStringView tsStr,
    _Format_str_ NkStringView fmtMsgStr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(0 <= lvlId && lvlId < __NkLogLvl_Count__, NkErr_InParameter);
    NK_ASSERT(msgCxtPtr != NULL, NkErr_InParameter);
    NK_UNREFERENCED_PARAMETER(self);
    NK_UNREFERENCED_PARAMETER(msgCxtPtr);

    /* Retrieve pointer to the static info for the current log level. */
    __NkInt_LogLevelStatic const *lvlInfo = &gl_LogLevelStaticInfo[lvlId];

    /* Compose log message from the "compiled" components. */
    if (lvlId ^ NkLogLvl_None) {
        /* If the logging level is "none", do not print the level or timestamp information. */
        fputs("[", stdout);
        fputs(tsStr.mp_dataPtr, stdout);
        fputs("] <", stdout);
        fputs(lvlInfo->m_vt100Seq.mp_dataPtr, stdout);
        fputs(lvlInfo->m_lvlRep.mp_dataPtr, stdout);
        fputs(gl_ResetFmtCmd.mp_dataPtr, stdout);
        fputs(">", stdout);
        fprintf(stdout, "%.*s", (int)lvlInfo->m_padding + 1, gl_PaddingTempl.mp_dataPtr);
    }
    fputs(fmtMsgStr.mp_dataPtr, stdout);
    fputc('\n', stdout);
}


/**
 * \brief define the internal debug device
 * 
 * \par Remarks
 *   Just like with the internal definition of the \c gl_RendererFactory object in file
 *   <tt>renderer.c</tt>, internal state of the object is not needed, so we can define
 *   our object to just be the VTable which by itself can be static. This is not going to
 *   cause us any issues.
 */
NK_INTERNAL NkILogDevice const gl_ConoutDevice = {
    .VT = &(struct __NkILogDevice_VTable__){
        .QueryInterface = &__NkInt_ConoutDev_QueryInterface,
        .AddRef         = &__NkInt_ConoutDev_AddRef,
        .Release        = &__NkInt_ConoutDev_Release,
        .OnInstall      = &__NkInt_ConoutDev_OnInstall,
        .OnUninstall    = &__NkInt_ConoutDev_OnUninstall,
        .OnMessage      = &__NkInt_ConoutDev_OnMessage
    }
};
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
    _Out_                        NkSize *tsSzPtr,
    _Out_                        NkNativeTime *currTimePtr
) {
    /* Format string. */
    *msgSzPtr = (NkSize)vsnprintf(msgPtr, NK_LOG_MSGSIZE, fmtStr, vlArgs);

    /* Format timestamp. */
    NkInt64 currLTime;
    _time64(&currLTime);
    localtime_s(currTimePtr, &currLTime);
    *tsSzPtr = strftime(
        tsPtr,
        NK_LOG_TSSIZE,
        gl_LogContext.m_defTsFmt.mp_dataPtr,
        currTimePtr
    );
}

/**
 */
NK_INTERNAL NkSize NK_CALL __NkInt_LogFindDevice(_In_ NkILogDevice *devRef) {
    NK_ASSERT(devRef != NULL, NkErr_InParameter);

    for (NkSize i = 0; i < NK_ARRAYSIZE(gl_LogContext.m_devArray); i++)
        if (gl_LogContext.m_devArray[i] == devRef)
            return i;

    return SIZE_MAX;
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkLogStartup(NkVoid) {
    NK_INITLOCK(gl_LogContext.m_mtxLock);

    NkErrorCode errCode = NkErr_Ok;

#if (!defined NK_CONFIG_DEPLOY)
    /* Register built-in debug logger. */
    errCode = NkLogInstallDevice((NkILogDevice *)&gl_ConoutDevice);
#endif

    NK_LOG_INFO("startup: logging");
    return errCode;
}

_Return_ok_ NkErrorCode NK_CALL NkLogShutdown(NkVoid) {
    NK_LOG_INFO("shutdown: logging");

    /*
     * Traverse the device array, run their 'NkILogDevice::OnUninstall()' method and
     * release them. 
     */
    for (NkSize i = 0, j = 0; i < NK_ARRAYSIZE(gl_LogContext.m_devArray) && j < gl_LogContext.m_nOfDev; i++) {
        NkILogDevice *currDev = gl_LogContext.m_devArray[i];
        if (currDev == NULL)
            continue;

        /* Call 'NkILogDevice::OnUninstall()' method. */
        NK_IGNORE_RETURN_VALUE(currDev->VT->OnUninstall(currDev));
        /*
         * Release device. This balances the 'NkILogDevice::AddRef()'-call we made in
         * 'NkLogInstallDevice()'.
         */
        currDev->VT->Release(currDev);
        ++j;
    }

    NK_DESTROYLOCK(gl_LogContext.m_mtxLock);
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkLogInstallDevice(_Inout_ NkILogDevice *devRef) {
    NK_ASSERT(devRef != NULL, NkErr_InOutParameter);

    NkErrorCode errorCode = NkErr_Ok;

    /* Check if we can even add a sink. If we cannot, return invalid sink ID. */
    NK_LOCK(gl_LogContext.m_mtxLock);
    if (gl_LogContext.m_nOfDev >= NK_ARRAYSIZE(gl_LogContext.m_devArray)) {
        errorCode = NkErr_CapLimitExceeded;

        goto lbl_CLEANUP;
    }

    /* Get pointer to device array. */
    NkILogDevice **devArr = gl_LogContext.m_devArray;
    /* Insert the sink into the first free slot. */
    for (NkSize i = 0; i < NK_ARRAYSIZE(gl_LogContext.m_devArray); i++) {
        /*
         * As soon as we reach an empty slot, we run the 'NkILogDevice::OnInstall' method
         * which also lets us ask if we can actually install the device.
         */
        if (devArr[i] == NULL) {
            NK_UNLOCK(gl_LogContext.m_mtxLock);
            errorCode = devRef->VT->OnInstall(devRef);
            NK_LOCK(gl_LogContext.m_mtxLock);
            if (errorCode != NkErr_Ok)
                goto lbl_CLEANUP;

            /* Install the new device. */
            devArr[i] = devRef;
            devRef->VT->AddRef(devRef);
            ++gl_LogContext.m_nOfDev;
            goto lbl_CLEANUP;
        }
    }

lbl_CLEANUP:
    NK_UNLOCK(gl_LogContext.m_mtxLock);

    return errorCode;
}

_Return_ok_ NkErrorCode NK_CALL NkLogUninstallDevice(_Inout_ NkILogDevice *devRef) {
    NK_ASSERT(devRef != NULL, NkErr_InOutParameter);
    NK_LOCK(gl_LogContext.m_mtxLock);

    /* Check if the device is installed. */
    NkSize devIndex;
    if ((devIndex = __NkInt_LogFindDevice(devRef)) == SIZE_MAX)
        return NkErr_ItemNotFound;

    /* Call 'NkILogDevice::OnUninstall()' method. */
    NK_UNLOCK(gl_LogContext.m_mtxLock);
    NK_IGNORE_RETURN_VALUE(devRef->VT->OnUninstall(devRef));
    NK_IGNORE_RETURN_VALUE(devRef->VT->Release(devRef));
    NK_LOCK(gl_LogContext.m_mtxLock);

    /* Erase device from array. */
    gl_LogContext.m_devArray[devIndex] = NULL;
    --gl_LogContext.m_nOfDev;

    NK_UNLOCK(gl_LogContext.m_mtxLock);
    return NkErr_Ok;
}

NkVoid NK_CALL NkLogMessage(
    _Inout_opt_  NkLogMessageContext *msgCxtPtr,
    _In_         NkLogLevel lvlId,
    _Format_str_ char const *fmtStr,
    ...
) {
    NK_ASSERT(0 <= lvlId && lvlId < __NkLogLvl_Count__, NkErr_InParameter);
    NK_ASSERT(fmtStr != NULL, NkErr_InParameter);

    /*
     * If we got no message context provided, redirect our context modification efforts
     * to our statically-allocated context.
     */
    NkLogMessageContext msgContext = { .m_structSize = sizeof msgContext };
    msgCxtPtr = msgCxtPtr != NULL ? msgCxtPtr : &msgContext;

    /* Format message and timestamp (timestamp only if needed). */
    char msgBuf[NK_LOG_MSGSIZE], tsBuf[NK_LOG_TSSIZE];
    NkSize tsSize, msgSize;
    va_list vlArgs;
    va_start(vlArgs, fmtStr);
    __NkInt_LogFormatMessageAndTimestamp(fmtStr, vlArgs, msgBuf, tsBuf, &msgSize, &tsSize, &msgCxtPtr->m_timestamp);
    va_end(vlArgs);

    NK_LOCK(gl_LogContext.m_mtxLock);
    /*
     * If the log level is globally disabled or there are currently no sinks registered,
     * do nothing.
     */
    NkBoolean const isEnabled = NK_INRANGE_INCL(lvlId, gl_LogContext.m_glMinLevel, gl_LogContext.m_glMaxLevel);
    if (!isEnabled || gl_LogContext.m_nOfDev == 0)
        goto lbl_CLEANUP;

    /*
     * Propagate formatted message and additional context data to sinks. Stop when we
     * reach the end of the sink array or we have processed all registered sinks,
     * whatever comes first.
     */
    for (NkSize i = 0, j = 0; i < NK_ARRAYSIZE(gl_LogContext.m_devArray) && j < gl_LogContext.m_nOfDev; i++) {
        /* Get pointer to device. If the slot is empty, ignore. */
        NkILogDevice *currDev = gl_LogContext.m_devArray[i];
        if (currDev == NULL)
            continue;

        /* Call the device's 'NkILogDevice::OnMessage()' method. */
        NK_UNLOCK(gl_LogContext.m_mtxLock);
        currDev->VT->OnMessage(
            currDev,
            lvlId,
            msgCxtPtr,
            (NkStringView){ tsBuf,  msgSize },
            (NkStringView){ msgBuf, msgSize }
        );
        NK_LOCK(gl_LogContext.m_mtxLock);
        ++j;
    }

lbl_CLEANUP:
    NK_UNLOCK(gl_LogContext.m_mtxLock);
}


#undef NK_NAMESPACE


