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
 * \file  renderer.h
 * \brief defines the public platform-independent API for the renderer
 * 
 * In Noriko, a renderer can only exist in the context of a window, that is, the renderer
 * is managed not by the caller, but by the window. As such, the renderer can only be
 * accessed through the window. When the window is modified (e.g., when the window mode
 * is changed), changes to the client area are automatically propagated to the renderer.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/nkom.h>
#include <include/Noriko/window.h>


/**
 * \enum  NkRendererApi
 * \brief lists all implemented renderer APIs, only some of which may be implemented on
 *        the current platform
 * \note  To get a list of all implemented renderer APIs for which a renderer is
 *        implemented on the current platform, call the
 *        <tt>NkRendererQueryAvailablePlatformApis()</tt> function.
 */
NK_NATIVE typedef enum NkRendererApi {
    NkRdApi_Unknown = 0, /**< unknown/invalid API */
    NkRdApi_Default,     /**< default renderer for current platform */

    NkRdApi_Win32GDI,    /**< GDI renderer */

    __NkRdApi_Count__    /**< *only used internally* */
} NkRendererApi;

/**
 * \struct NkRendererSpecification
 * \brief  represents the configuration data used when configuring
 */
NK_NATIVE typedef struct NkRendererSpecification {
    NkSize               m_structSize;   /**< size of this structure, in bytes */
    NkIWindow           *mp_wndRef;      /**< reference to the parent window */
    NkBoolean            m_isVSync;      /**< whether VSync is enabled */
    NkRendererApi        m_renderApi;    /**< API to initialize renderer for */
    NkSize2D             m_vpExtents;    /**< viewport extents (in tiles) */
    NkSize2D             m_dispTileSize; /**< tile size (in pixels) */
    NkViewportAlignment  m_vpAlignment;  /**< viewport alignment in client area of window */
} NkRendererSpecification;


/**
 * \interface NkIRenderer
 * \brief     represents the public platform-independent API of a platform renderer;
 *            query this interface to issue general rendering commands
 */
NKOM_DECLARE_INTERFACE(NkIRenderer) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkIRenderer *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt>
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIRenderer *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt>
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIRenderer *self);

    /**
     * \brief reimplements <tt>NkIInitializable::Initialize()</tt>
     */
    NkErrorCode (NK_CALL *Initialize)(_Inout_ NkIRenderer *self, _Inout_opt_ NkVoid *initParam);

    /**
     * \brief  retrieves the renderer API identifier that the current renderer uses to
     *         render graphics
     * \param  [in,out] self pointer to the current \c NkIRenderer instance
     * \return \c numeric renderer API identifier; part of the \c NkRendererApi
     *         enumeration
     * 
     * \par Remarks
     *   The return value of this function is never \c NkRdApi_Unknown or
     *   <tt>NkRdApi_Default</tt>. Therefore, even if the renderer was initialized using
     *   the <tt>NkRdApi_Default</tt> API identifier, the returned value is the actual
     *   API chosen.
     */
    NkRendererApi (NK_CALL *QueryRendererAPI)(_Inout_ NkIRenderer *self);
    /**
     * \brief  retrieves a pointer to the original renderer specification passed when the
     *         renderer was created
     * \param  [in,out] self pointer to the current \c NkIRenderer instance
     * \return pointer to the original renderer specification
     */
    NkRendererSpecification const *(NK_CALL *QuerySpecification)(_Inout_ NkIRenderer *self);
    /**
     * \brief  retrieves the handle to the Noriko window the current renderer belongs to
     * \param  [in,out] self pointer to the current \c NkIRenderer instance
     * \return pointer to the Noriko window handle
     * \note   The return value of this function can never be <tt>NULL</tt>.
     */
    NkIWindow *(NK_CALL *QueryWindow)(_Inout_ NkIRenderer *self);

    /**
     * \brief  resizes the client area of the renderer which is usually congruent with the
     *         client area of the window
     * \param  [in,out] self current \c NkIRenderer instance
     * \param  [in] clAreaSize dimensions of the client area, in pixels
     * \return \c NkErr_Ok on success, non-zero on failure
     */
    NkErrorCode (NK_CALL *Resize)(_Inout_ NkIRenderer *self, _In_ NkSize2D const *clAreaSize);
    /**
     * \brief  starts a new batch of rendering commands
     * \param  [in,out] self current \c NkIRenderer instance
     * \return \c NkErr_Ok on success, non-zero on failure
     */
    NkErrorCode (NK_CALL *BeginRender)(_Inout_ NkIRenderer *self);
    /**
     * \brief  finishes a batch of rendering commands, flushes the output device and, if
     *         VSync is enabled, waits (i.e., blocks) for a V-Blank signal
     * \param  [in,out] self current \c NkIRenderer instance
     * \return \c NkErr_Ok on success, non-zero on failure
     */
    NkErrorCode (NK_CALL *EndRender)(_Inout_ NkIRenderer *self);
};


/**
 * \brief  does some pre-runtime initialization of some global state associated with the
 *         application
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   This function does not instantiate an actual renderer but rather enables the
 *         application to instantiate the renderer it so desires. Call this function once
 *         per process before instantiating the first renderer.
 */
NK_NATIVE NK_VIRTUAL NK_API _Return_ok_ NkErrorCode NK_CALL NkRendererStartup(NkVoid);
/**
 * \brief  uninitializes global renderer state
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   If this function is called without a corresponding call to
 *         <tt>NkRendererStartup()</tt>, this function does nothing.
 */
NK_NATIVE NK_VIRTUAL NK_API _Return_ok_ NkErrorCode NK_CALL NkRendererShutdown(NkVoid);

/**
 * \brief   retrieves a list of renderer APIs of which an implementation, that is, a
 *          renderer exists
 * 
 * This function can be used, for instance, to select a renderer at run-time that is not
 * influenced by the default setting. Another use-case could be to display a list of
 * available renderer APIs in a settings dialog, enabling the player/application user to
 * choose a rendering API on their own.
 * 
 * \param   [out] resPtr pointer to an array of \c NkRendererApi values
 * \return  number of elements in \c resPtr
 * \see     NkRendererApi
 * \warning If \c resPtr is <tt>NULL</tt>, the behavior is undefined.
 * 
 * \par Remarks
 *   <ul>
 *    <li>
 *     The array pointed to by \c resPtr is statically-allocated and, thus, does not need
 *     to be memory-managed.
 *    <li>
 *     The array pointed to by \c resPtr may be empty and the return value may be 0 as a
 *     consequence.
 *    </li>
 *   </ul>
 */
NK_NATIVE NK_VIRTUAL NK_API NkSize NK_CALL NkRendererQueryAvailablePlatformApis(_Out_ NkRendererApi *resPtr);
/**
 * \brief  retrieves the API identifier for the renderer API that will be chosen when
 *         specifying the \c NkRdApi_Default flag when instantiating the renderer
 * \return renderer API; part of the \c NkRendererApi enumeration
 * \see    NkRendererApi
 * \note   If no renderer API is implemented on the current platform, the return value is
 *         <tt>NkRdApi_Unknown</tt>.
 * 
 * \par Remarks
 *   During the development phase, on certain platforms, there may not be a single
 *   renderer API implemented. This should not be a thing in a release version of the
 *   Noriko engine component.
 */
NK_NATIVE NK_VIRTUAL NK_API NkRendererApi NK_CALL NkRendererQueryDefaultPlatformApi(NkVoid);


