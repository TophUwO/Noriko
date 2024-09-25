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


/**
 * \def   NK_MAKE_LOG_FRAME()
 * \brief creates an inline log frame for use with the logging functions
 */
#define NK_MAKE_LOG_FRAME()                    \
    &(NkLogFrame){                             \
        sizeof(NkLogFrame),                    \
        NK_MAKE_STRING_VIEW_PTR(__FILE__),     \
        NK_MAKE_STRING_VIEW_PTR(NK_NAMESPACE), \
        NK_MAKE_STRING_VIEW_PTR(__func__)      \
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
#define NK_LOG_NONE(msg, ...)      NkLogWrite(NULL, NkLogLvl_None, msg, ##__VA_ARGS__)
#define NK_LOG_TRACE(msg, ...)     NkLogWrite(NULL, NkLogLvl_Trace, msg, ##__VA_ARGS__)
#define NK_LOG_DEBUG(msg, ...)     NkLogWrite(NULL, NkLogLvl_Debug, msg, ##__VA_ARGS__)
#define NK_LOG_INFO(msg, ...)      NkLogWrite(NULL, NkLogLvl_Info, msg, ##__VA_ARGS__)
#define NK_LOG_WARNING(msg, ...)   NkLogWrite(NULL, NkLogLvl_Warn, msg, ##__VA_ARGS__)
#define NK_LOG_ERROR(msg, ...)     NkLogWrite(NULL, NkLogLvl_Error, msg, ##__VA_ARGS__)
#define NK_LOG_CRITICAL(msg, ...)  NkLogWrite(NULL, NkLogLvl_Critical, msg, ##__VA_ARGS__)

#define NK_LOG_CNONE(msg, ...)     NkLogWrite(NK_MAKE_LOG_FRAME(), NkLogLvl_None, msg, ##__VA_ARGS__)
#define NK_LOG_CTRACE(msg, ...)    NkLogWrite(NK_MAKE_LOG_FRAME(), NkLogLvl_Trace, msg, ##__VA_ARGS__)
#define NK_LOG_CDEBUG(msg, ...)    NkLogWrite(NK_MAKE_LOG_FRAME(), NkLogLvl_Debug, msg, ##__VA_ARGS__)
#define NK_LOG_CINFO(msg, ...)     NkLogWrite(NK_MAKE_LOG_FRAME(), NkLogLvl_Info, msg, ##__VA_ARGS__)
#define NK_LOG_CWARNING(msg, ...)  NkLogWrite(NK_MAKE_LOG_FRAME(), NkLogLvl_Warn, msg, ##__VA_ARGS__)
#define NK_LOG_CERROR(msg, ...)    NkLogWrite(NK_MAKE_LOG_FRAME(), NkLogLvl_Error, msg, ##__VA_ARGS__)
#define NK_LOG_CCRITICAL(msg, ...) NkLogWrite(NK_MAKE_LOG_FRAME(), NkLogLvl_Critical, msg, ##__VA_ARGS__)
/** @} */


/**
 * \brief handle used for sink management
 */
NK_NATIVE typedef        NkInt32             NkLogSinkHandle;
NK_NATIVE typedef struct NkLogSinkProperties NkLogSinkProperties;
NK_NATIVE typedef struct NkLogFrame          NkLogFrame;

/**
 * \brief  is invoked after the sink was registered
 * \param  [in] sinkHandle handle of the new sink
 * \param  [in,out] sinkPropsPtr pointer to the sink properties
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li Returning non-zero will result in the sink not being added.
 * \note   \li Only implement this function if you need to some something special
 *             directly before the sink is added.
 */
NK_NATIVE typedef NkErrorCode (NK_CALL *NkLogSinkInitFn)(
    _In_    NkLogSinkHandle sinkHandle,
    _Inout_ NkLogSinkProperties *sinkPropsPtr
);
/**
 * \brief  is invoked directly before the sink is unregistered
 * \param  [in] sinkHandle handle of the sink
 * \param  [in,out] sinkPropsPtr pointer to the sink properties
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li Returning non-zero will result in the sink not being removed.
 * \note   \li Only implement this function if you need to some something special
 *             directly before the sink is unregistered.
 */
NK_NATIVE typedef NkErrorCode (NK_CALL *NkLogSinkUninitFn)(
    _In_    NkLogSinkHandle sinkHandle,
    _Inout_ NkLogSinkProperties *sinkPropsPtr
);
/**
 * \brief is invoked for each log message and sink individually
 * \param [in] sinkHandle numeric sink handle
 * \param [in] lvlId numeric ID of the logging level used for this message
 * \param [in] tsPtr pointer to the formatted timestamp string
 * \param [in] fmtMsgPtr pointer to the formatted log message
 * \param [in] framePtr pointer to additional log context data; may be \c NULL
 * \param [in,out] sinkPropsPtr pointer to the sink this callback belongs to; can be
 *                 modified from this callback on each log message event
 * \note  The \c sinkPropsPtr is not constant. This can be used to alter sink
 *        configuration. The config will take effect automatically when the next
 *        message is logged.
 */
NK_NATIVE typedef NkVoid (NK_CALL *NkLogSinkWriteFn)(
    _In_     NkLogSinkHandle sinkHandle, 
    _In_     enum NkLogLevel lvlId,
    _In_z_   char const *tsPtr,
    _In_z_   char const *fmtMsgPtr,
    _In_opt_ NkLogFrame const *framePtr,
    _Inout_  NkLogSinkProperties *sinkPropsPtr
);

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
    NkLogLvl_Info,     /**< general info log level; used for logging events, like system start-up, shutdown, etc. */
    NkLogLvl_Warn,     /**< warning log level; used for events that may cause problems but are recovered from automatically */
    NkLogLvl_Error,    /**< error log level; used when an operation was cancelled by an error but the system is still alright */
    NkLogLvl_Critical, /**< fatal log level; used when the system cannot continue working due to a catastophic failure */

    __NkLogLvl_Count__ /**< *only used internally* */
} NkLogLevel;

/**
 * \struct NkLogFrame
 * \brief  represents the caller's context for a log message
 *
 * This structure can optionally be passed to the logger function, allowing the sinks
 * to print out extra context information which can be useful for debugging. Generally,
 * it should not be required.
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkLogFrame {
    NkSize        m_structSize; /**< size of this struct, in bytes */
    NkStringView *mp_filePath;  /**< file path where the log message came from */
    NkStringView *mp_nsIdent;   /**< namespace identifier, define with \c NK_NAMESPACE */
    NkStringView *mp_funcName;  /**< name of the function the log message originated from */
} NkLogFrame;

/**
 * \struct NkLogSinkProperties
 * \brief  represents properties for a new log sink
 * 
 * The logger uses the data in this struct to propagate the correct log messages to the
 * respective sinks.
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkLogSinkProperties {
    NkSize             m_structSize;      /**< size of this struct, in bytes */
    NkStringView       m_sinkIdent;       /**< string identifier for this sink */
    NkLogLevel         m_minLevel;        /**< global minimum log level */
    NkLogLevel         m_maxLevel;        /**< global maximum log level */
    NkVoid            *mp_extraCxt;       /**< extra context to be passed to the callbacks */
    NkLogSinkInitFn    mp_fnOnSinkInit;   /**< virtual \c OnInit callback */
    NkLogSinkUninitFn  mp_fnOnSinkUninit; /**< virtual \c onUninit callback */
    NkLogSinkWriteFn   mp_fnOnSinkWrite;  /**< virtual \c OnLog callback */
} NkLogSinkProperties;

/**
 * \struct NkLogLevelProperties
 * \brief  represents the properties for the individual log levels
 */
NK_NATIVE typedef struct NkLogLevelProperties {
    NkInt32       m_nSpace;     /**< number of spaces needed for proper alignment */
    NkRgbaColor   m_rgbaCol;    /**< color as RGB(A) value */
    NkStringView *mp_lvlStrRep; /**< string representation of the level */
    NkStringView *mp_lvlFmtStr; /**< format used for level (for virtual terminal) */
} NkLogLevelProperties;

/**
 * \struct NkLogContext
 * \brief  represents the static log context, i.e. some constant properties that can or
 *         may have to be queried by applications or modules that wish to use the logging
 *         facilities built into Noriko
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkLogContext {
    NkSize        m_structSize; /**< size of this struct, in bytes */
    NkSize        m_maxMsgSize; /**< maximum length of a log message */
    NkLogLevel    m_glMinLevel; /**< global minimum log level */
    NkLogLevel    m_glMaxLevel; /**< global maximum log level */
    NkStringView *mp_defFmtStr; /**< default format (for virtual terminal) */
    NkStringView *mp_rstFmtStr; /**< reset format (for virtual terminal) */
    NkStringView *mp_tsFmtStr;  /**< format pattern for timestamp */
    NkSize        m_maxSinkCnt; /**< total number of sink slots */

    /**
     * \struct NkLogLevelProperties
     * \brief  represents the static properties for each log level
     */
    NkLogLevelProperties m_lvlProps[__NkLogLvl_Count__]; /**< level properties array */
} NkLogContext;


/**
 * \brief initializes the logging facility
 * \note  Call this function only once during application startup as early as possible
 *        from the main thread.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLogInitialize(NkVoid);
/**
 * \brief   uninitializes the logging facility
 * \note    Call this function only once during application shutdown as late as possible
 *          from the main thread.
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \warning Calling this function without having called \c NkLogInitialize() before is
 *          undefined behavior.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLogUninitialize(NkVoid);
/**
 * \brief   queries the static log context
 * \param   [in] cxtStructPtr pointer to a NkLogContext instance that will receive the
 *               static log context
 * \note    The data returned in \c cxtStructPtr is **not** going to change throughout
 *          the application's lifetime. Thus, only do this once before you log the first
 *          message if necessary. You can call this function right after you call
 *          <tt>NkLogInitialize()</tt>.
 * \warning Calling this function without having called \c NkLogInitialize() before is
 *          undefined behavior.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkLogQueryContext(_Out_ NkLogContext *cxtStructPtr);

/**
 * \brief   registers a new log sink
 * \param   [in,out] sinkPropsPtr properties of the new sink
 * \param   [out] sinkHandlePtr pointer to a NkLogSinkHandle variable that will receive
 *                the handle to the new sink
 * \return  \c NkErr_Ok on success, non-zero on failure.
 * \note    If the function fails, \c sinkHandlePtr will be initialized to <tt>-1</tt>.
 * \warning \li If <tt>sinkPropsPtr</tt> or <tt>sinkHandlePtr</tt> are <tt>NULL</tt>, the
 *              behavior is undefined.
 * \warning \li Calling this function without having called \c NkLogInitialize() before
 *              is undefined behavior.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLogRegisterSink(
    _Inout_ NkLogSinkProperties *sinkPropsPtr,
    _Out_   NkLogSinkHandle *sinkHandlePtr
);
/**
 * \brief   unregisters a previously registered sink
 * \param   [in,out] sinkHandlePtr pointer to the log sink handle
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \note    \li If the function succeeds, \c sinkHandlePtr will be set to <tt>-1</tt>.
 * \note    \li If the function fails, \c sinkHandlePtr will not be modified.
 * \note    \li If \c sinkHandlePtr holds an invalid sink handle, the function does
 *              nothing. **However, the behavior is undefined if the sink handle points
 *              to a sink that does no longer exist.**
 * \warning \li Calling this function without having called \c NkLogInitialize() before
 *              is undefined behavior.
 * \warning \li Calling this function without having called \c NkLogRegisterSink() before
 *              is undefined behavior.
 * \warning \li If \c sinkHandlePtr is <tt>NULL</tt>, the behavior is undefined.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkLogUnregisterSink(_Inout_ NkLogSinkHandle *sinkHandlePtr);

/**
 * \brief   propagates the given message to all registered sinks
 * \param   [in] framePtr pointer to an NkLogFrame instance that provided additional local
 *              context for the log message
 * \param   [in] lvlId ID of the log level
 * \param   [in] fmtStr format template of the message that is to be logged
 * \note    If this function is called while no sink is registered, this function does
 *          nothing.
 * \warning \li If \c fmtStr is \c NULL or \c lvlId is invalid, the behavior is undefined.
 * \warning \li \c framePtr can be <tt>NULL</tt>, so log handlers must take special care
 *              if they wish to use the context provided by it.
 * \warning \li Calling this function without having called \c NkLogInitialize() before
 *              is undefined behavior.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkLogWrite(
    _In_opt_     NkLogFrame const *framePtr,
    _In_         NkLogLevel lvlId,
    _Format_str_ char const *fmtStr,
    ...
);


