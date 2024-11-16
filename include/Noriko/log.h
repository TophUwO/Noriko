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
 * \file  log.h
 * \brief defines the interface to Noriko's logging facility
 *
 * The logger is designed as a singleton that is fully modular. Rather than implementing
 * all features directly, the logger defers the implementation of the "sinks" to modules
 * that use its API.
 * \todo make segmented logging possible.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/nkom.h>


/**
 * \def   NK_MAKE_LOG_FRAME()
 * \brief creates an inline log frame for use with the logging functions
 */
#define NK_MAKE_LOG_FRAME()                \
    &(NkLogFrame){                         \
        sizeof(NkLogFrame),                \
        NK_MAKE_STRING_VIEW(__FILE__),     \
        NK_MAKE_STRING_VIEW(NK_NAMESPACE), \
        NK_MAKE_STRING_VIEW(__func__)      \
    }

/**
 * \defgroup Macros
 * \brief    contains shortcuts for conveniently logging messages without having to
 *           specify the log level every time
 * \note     The infix \c C means 'context'. That is, when using <tt>C</tt> macros, a
 *           caller context will be created. This can make logging slightly slower when
 *           high throughput is desired. Use <tt>C</tt>-macros primarily for debugging,
 *           or for other messages where additional context such as file, function, etc.
 *           is desirable.
 */
/** @{ */
#define NK_LOG_NONE(msg, ...)      NkLogMessage(NULL, NkLogLvl_None, msg, ##__VA_ARGS__)
#define NK_LOG_TRACE(msg, ...)     NkLogMessage(NULL, NkLogLvl_Trace, msg, ##__VA_ARGS__)
#define NK_LOG_DEBUG(msg, ...)     NkLogMessage(NULL, NkLogLvl_Debug, msg, ##__VA_ARGS__)
#define NK_LOG_INFO(msg, ...)      NkLogMessage(NULL, NkLogLvl_Info, msg, ##__VA_ARGS__)
#define NK_LOG_WARNING(msg, ...)   NkLogMessage(NULL, NkLogLvl_Warn, msg, ##__VA_ARGS__)
#define NK_LOG_ERROR(msg, ...)     NkLogMessage(NULL, NkLogLvl_Error, msg, ##__VA_ARGS__)
#define NK_LOG_CRITICAL(msg, ...)  NkLogMessage(NULL, NkLogLvl_Critical, msg, ##__VA_ARGS__)

#define NK_LOG_CNONE(msg, ...)     NkLogMessage(NK_MAKE_LOG_FRAME(), NkLogLvl_None, msg, ##__VA_ARGS__)
#define NK_LOG_CTRACE(msg, ...)    NkLogMessage(NK_MAKE_LOG_FRAME(), NkLogLvl_Trace, msg, ##__VA_ARGS__)
#define NK_LOG_CDEBUG(msg, ...)    NkLogMessage(NK_MAKE_LOG_FRAME(), NkLogLvl_Debug, msg, ##__VA_ARGS__)
#define NK_LOG_CINFO(msg, ...)     NkLogMessage(NK_MAKE_LOG_FRAME(), NkLogLvl_Info, msg, ##__VA_ARGS__)
#define NK_LOG_CWARNING(msg, ...)  NkLogMessage(NK_MAKE_LOG_FRAME(), NkLogLvl_Warn, msg, ##__VA_ARGS__)
#define NK_LOG_CERROR(msg, ...)    NkLogMessage(NK_MAKE_LOG_FRAME(), NkLogLvl_Error, msg, ##__VA_ARGS__)
#define NK_LOG_CCRITICAL(msg, ...) NkLogMessage(NK_MAKE_LOG_FRAME(), NkLogLvl_Critical, msg, ##__VA_ARGS__)
/** @} */


/**
 * \typedef NkNativeTime
 * \brief   represents detailed timestamp suitable for formatting timestamps with 
 */
NK_NATIVE typedef struct tm NkNativeTime;

/**
 * \enum  NkLogLevel
 * \brief represents the numeric log level identifier
 * 
 * This identifier is used for specifying the log level when logging messages, when
 * querying log level properties, etc.
 */
NK_NATIVE typedef _In_range_(0, __NkLogLvl_Count__ - 1) enum NkLogLevel {
    NkLogLvl_None,     /**< no log level, used for plain messages */

    NkLogLvl_Trace,    /**< trace log level (for logging API calls, etc.) */
    NkLogLvl_Debug,    /**< log level for debug messages (variable states, decisions, ...) */
    NkLogLvl_Info,     /**< general info log level; used for logging general info etc. */
    NkLogLvl_Warn,     /**< warning log level; used for problematic events that are recovered from */
    NkLogLvl_Error,    /**< error log level; used when an operation was cancelled (recoverable) */
    NkLogLvl_Critical, /**< fatal log level; used when the system cannot continue working */

    __NkLogLvl_Count__ /**< *only used internally* */
} NkLogLevel;

/**
 * \struct NkLogMessageContext
 * \brief  represents the caller's context for a log message
 *
 * This structure can optionally be passed to the 'OnMessage()' method of <tt>NkILogDevice</tt>,
 * allowing the device to print out extra context information which can be useful for
 * debugging. Generally, this should not be required.
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkLogMessageContext {
    NkSize        m_structSize; /**< size of this struct, in bytes */
    NkStringView  m_filePath;  /**< file path where the log message came from */
    NkStringView  m_nsIdent;   /**< namespace identifier, define with \c NK_NAMESPACE */
    NkStringView  m_funcName;  /**< name of the function the log message originated from */
    NkNativeTime  m_timestamp;  /**< native timestamp at the time of log call */
} NkLogMessageContext;


/**
 * \interface NkILogDevice
 * \brief     represents a device that can be used to display log messages, e.g., a log
 *            file, console window, QWidget, etc.
 */
NKOM_DECLARE_INTERFACE(NkILogDevice) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt> 
     */
    NkErrorCode (NK_CALL *QueryInterface)(
        _Inout_  NkILogDevice *self,
        _In_     NkUuid const *iId,
        _Outptr_ NkVoid **resPtr
    );
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkILogDevice *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkILogDevice *self);

    /**
     */
    NkErrorCode (NK_CALL *OnInstall)(_Inout_ NkILogDevice *self);
    /**
     */
    NkErrorCode (NK_CALL *OnUninstall)(_Inout_ NkILogDevice *self);
    /**
     */
    NkVoid (NK_CALL *OnMessage)(
        _Inout_      NkILogDevice *self,
        _In_         NkLogLevel lvlId,
        _In_         NkLogMessageContext const *msgCxtPtr,
        _In_         NkStringView tsStr,
        _Format_str_ NkStringView fmtMsgStr
    );
};


/**
 * \brief   installs a new log device
 * \param   [in,out] devRef pointer to the device
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \warning \li Calling this function without having called <tt>NkLogStartup()</tt>
 *              before is undefined behavior.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLogInstallDevice(_Inout_ NkILogDevice *devRef);
/**
 * \brief   uninstalls a previously installed log device
 * \param   [in,out] devRef pointer to the device
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \warning \li Calling this function without having called <tt>NkLogStartup()</tt>
 *              before is undefined behavior.
 * \warning \li Calling this function without having called <tt>NkILogInstallDevice()</tt>
 *              before is undefined behavior.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLogUninstallDevice(_Inout_ NkILogDevice *devRef);

/**
 * \brief   propagates the given message to all installed devices
 * \param   [in,out] msgCxtPtr pointer to an instance of \c NkLogMessageContext that
 *                   contains additional information about the current message, such as
 *                   location of origin and timestamp
 * \param   [in] lvlId ID of the log level
 * \param   [in] fmtStr format template of the message that is to be logged
 * \note    If this function is called while no device is installed, this function does
 *          nothing.
 * \warning \li If \c fmtStr is \c NULL or \c lvlId is invalid, the behavior is undefined.
 * \warning \li Calling this function without having called <tt>NkLogStartup()</tt>
 *              before or after having called <tt>NkLogShutdown()</tt> results in
 *              undefined behavior.
 * 
 * \par Remarks
 *   It is possible to pass \c NULL for the parameter <tt>msgCxtPtr</tt>, however, the
 *   <tt>NkILogDevice::OnMessage()</tt> method of the log devices will still be passed a
 *   non-<tt>NULL</tt> pointer to a statically-allocated instance of
 *   <tt>NkLogMessageContext</tt>, holding only the structure size and the timestamp. If
 *   \c msgCxtPtr is not NULL, do not set the <tt>NkLogMessageContext::m_timestamp</tt>
 *   member as it will be overwritten by this function. <tt>NkILogDevice::OnMessage()</tt>
 *   is not allowed to modify the instance further.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkLogMessage(
    _Inout_opt_  NkLogMessageContext *msgCxtPtr,
    _In_         NkLogLevel lvlId,
    _Format_str_ char const *fmtStr,
    ...
);


