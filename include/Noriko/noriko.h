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
 * \file  noriko.h
 * \brief main public API header of the Noriko engine component
 * 
 * This header is to be used by both the Noriko Runtime (NorikoRt) and Noriko Editor
 * (NorikoEd) component. Thus, it exposes an API that is compatible with both C and C++.
 * This header contains important exported symbols that provide access high-level access
 * to Noriko's internal components.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>
#include <include/Noriko/error.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/sort.h>
#include <include/Noriko/timer.h>
#include <include/Noriko/log.h>
#include <include/Noriko/env.h>
#include <include/Noriko/nkom.h>
#include <include/Noriko/event.h>
#include <include/Noriko/layer.h>
#include <include/Noriko/window.h>
#include <include/Noriko/helpers.h>
#include <include/Noriko/renderer.h>

#include <include/Noriko/dstruct/vector.h>
#include <include/Noriko/dstruct/htable.h>


/**
 * \struct NkApplicationSpecification
 * \brief  represents global configuration options passed to the Noriko startup routine
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkApplicationSpecification {
    NkSize                 m_structSize;      /**< size of this structure, in bytes */
    NkBoolean              m_enableDbgTools;  /**< whether or not to enable debugging tools */
    NkRendererApi          m_rendererApi;     /**< API to use for rendering */
    NkBoolean              m_isVSync;         /**< whether or not VSync is used */
    NkViewportAlignment    m_vpAlignment;     /**< viewport alignment inside the main window */
    NkSize2D               m_vpExtents;       /**< size in tiles of the main window viewport */
    NkSize2D               m_dispTileSize;    /**< tile size used in the main window viewport */
    NkWindowMode           m_allowedWndModes; /**< allowed window modes for the main window */
    NkWindowMode           m_initialWndMode;  /**< initial window mode for the main window */
    NkWindowFlags          m_wndFlags;        /**< additional (platform-dependent) window flags */
    NkNativeWindowHandle   mp_nativeHandle;   /**< optional existing window handle to create Noriko window for */
    NkStringView           m_wndTitle;        /**< main window title */
    int                    m_argc;            /**< number of command-line parameters */
    char                 **mp_argv;           /**< command-line parameters */
    char                 **mp_envp;           /**< (optional) environment variables */
    NkStringView           m_workingDir;      /**< default working directory */
} NkApplicationSpecification;


/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkApplicationStartup(_In_ NkApplicationSpecification const *specsPtr);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkApplicationShutdown(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkApplicationRun(NkVoid);
/**
 * \brief quits the application at the next possible time
 * \param [in] errCode return code to propagate to the host platform
 * \note  To exit the application immediately without cleaning up, use the
 *        <tt>NkFatalTerminate(NULL)</tt> function.
 * 
 * \par Remarks
 *   This function does not immediately exit the application, but rather posts a quit
 *   request to the main loop which will be processed at one point. Once this request
 *   is received, all engine components will be shutdown in an orderly manner and control
 *   is returned to the host platform.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkApplicationExit(_Ecode_range_ NkErrorCode errCode);

/**
 * \brief  retrieves the application specification, that is, the application settings
 *         used to initialize the Noriko engine component
 * \return pointer to the application specification
 * \note   The return value of this function can never be <tt>NULL</tt>.
 */
NK_NATIVE NK_API NkApplicationSpecification const *NK_CALL NkApplicationQuerySpecification(NkVoid);


