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
#include <include/Noriko/platform.h>
#include <include/Noriko/bmp.h>


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
 * \enum  NkRendererResourceType
 * \brief represents a renderer resource type
 */
NK_NATIVE typedef enum NkRendererResourceType {
    NkRdResTy_None = 0, /**< invalid resource type */

    NkRdResTy_Texture,  /**< texture */

    __NkRdResTy_Count__ /**< *only used internally* */
} NkRendererResourceType;

/**
 * \enum  NkRendererResourceFlags
 * \brief represents boolean renderer resource flags which may allow the renderer to
 *        optimize drawing operation involving the resource or let the resource owner
 *        know whether or not the resource needs special attention in case of certain
 *        events
 */
NK_NATIVE typedef enum NkRendererResourceFlags {
    NkRdResFlag_None = 0,        /**< no flags */

    NkRdResFlag_DeviceDependent, /**< resource is <em>device-dependent</em> */

    __NkRdResFlag_Count__        /**< *only used internally* */
} NkRendererResourceFlags;

/**
 * \enum  NkTextureInterpolationMode
 * \brief defines available texture interpolation modes to use when scaling textures
 * 
 * \par Remarks
 *   \c NkTexIMd_NearestNeighbor is generally faster and does not produce halftones, but 
 *   results in jagged edges and pixelated areas around color gradients; best for when
 *   rendering pixel art like tiles, sprites, etc.<br>
 *   <tt>NkTexIMd_Bilinear</tt>, on the other hand, produces smooth color gradients and
 *   generally results in a higher quality output, but is slower and may not exactly be
 *   a good fit for the pixel art style; use when rendering images that are not pixel
 *   art.
 */
NK_NATIVE typedef enum NkTextureInterpolationMode {
    NkTexIMd_Default = 0,     /**< default interpolation mode */

    NkTexIMd_NearestNeighbor, /**< uses the color of the nearest neighbor */
    NkTexIMd_Bilinear,        /**< uses bilinear filtering which results in smooth color changes */

    __NkTexIMd_Count__        /**< *only used internally* */
} NkTextureInterpolationMode;


/**
 * \typedef NkRendererResourceHandle
 * \brief   represents an abstract resource handle
 * 
 * \par Remarks
 *   Generally, this handle can represent a implementation-(that is, renderer-) defined
 *   value and thus should not be touched ever by an outside component.
 */
NK_NATIVE typedef NkInt64 NkRendererResourceHandle;

/**
 * \struct NkRendererResource
 * \brief  represents an abstract Noriko renderer resource instance
 * 
 * \par Description
 *   Noriko supports multiple renderer APIs out of the box, each with their own ways of
 *   representing resources of various types such as, but not limited to: textures,
 *   brushes, paths, etc. This data-structure serves as an abstraction over all the
 *   renderer APIs' resource representations so that code using the renderers can do so
 *   in a mostly platform-agnostic way. Of course, this comes at a cost of both a bit of
 *   memory overhead as well as additional management cost since the created resources
 *   are managed not by the renderer but by the component invoking the renderer.
 *   Therefore, if a component uses the renderer and requests resources, that component
 *   must carry-out all duties of resource management such as deletion and (potential)
 *   recreation.
 * 
 * \par Remarks
 *   In some APIs, resources are bound to devices (like in Direct2D/3D, for example). If
 *   a device loss happens, if, for example, the device driver resets or the device is
 *   physically removed from the system, all resources created by the renderer that can
 *   only exist in the context of that device context (i.e., are <em>device-dependent</em>),
 *   must be recreated by the resource owner, that is, the component that requested it.
 */
NK_NATIVE typedef struct NkRendererResource {
    struct NkIRenderer              *mp_rdRef;    /**< reference to the renderer that created the resource */
           NkRendererResourceType    m_resType;   /**< numeric type ID of the resource */
           NkRendererResourceHandle  m_resHandle; /**< implementation-defined resource handle (don't touch!) */
           NkRendererResourceFlags   m_resFlags;  /**< miscellaneous resource flags */
} NkRendererResource;

/**
 * \struct NkRectF
 * \brief  represents a rectangular area in the renderer, relative to viewport space
 */
NK_NATIVE typedef struct NkRectF {
    NkFloat m_xCoord; /**< x-coordinate of upper-left corner */
    NkFloat m_yCoord; /**< y-coordinate of upper-left corner */
    NkFloat m_width;  /**< width of the rectangle, in pixels */
    NkFloat m_height; /**< height of the rectangle, in pixels */
} NkRectF;

/**
 * \struct NkRendererSpecification
 * \brief  represents the configuration data used when configuring
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkRendererSpecification {
    NkSize                      m_structSize;   /**< size of this structure, in bytes */
    NkIWindow                  *mp_wndRef;      /**< reference to the parent window */
    NkBoolean                   m_isVSync;      /**< whether VSync is enabled */
    NkRendererApi               m_rendererApi;  /**< API to initialize renderer for */
    NkSize2D                    m_vpExtents;    /**< viewport extents (in tiles) */
    NkSize2D                    m_dispTileSize; /**< tile size (in pixels) */
    NkViewportAlignment         m_vpAlignment;  /**< viewport alignment in client area of window */
    NkRgbaColor                 m_clearCol;     /**< clear color to use for background */
    NkTextureInterpolationMode  m_texInterMode; /**< texture interpolation mode */
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
     * \param  [in, out] self pointer to the current \c NkIRenderer instance
     * \return \c numeric renderer API identifier; part of the \c NkRendererApi
     *         enumeration
     * 
     * \par Remarks
     *   The return value of this function is never \c NkRdApi_Unknown or
     *   <tt>NkRdApi_Default</tt>. Therefore, even if the renderer was initialized using
     *   the <tt>NkRdApi_Default</tt> API identifier, the returned value is the actual
     *   API chosen.
     */
    NkRendererApi (NK_CALL *QueryRendererApi)(_Inout_ NkIRenderer *self);
    /**
     * \brief  retrieves a pointer to the *original* renderer specification passed when
     *         the renderer was created
     * \param  [in, out] self pointer to the current \c NkIRenderer instance
     * \return pointer to the original renderer specification
     */
    NkRendererSpecification const *(NK_CALL *QuerySpecification)(_Inout_ NkIRenderer *self);
    /**
     * \brief  retrieves the handle to the Noriko window the current renderer belongs to
     * \param  [in, out] self pointer to the current \c NkIRenderer instance
     * \return pointer to the Noriko window handle
     * \note   The return value of this function can never be <tt>NULL</tt>.
     *
     * \par Remarks
     *   Like with all functions that return an NkOM object, they increment the reference
     *   count of the returned object.
     */
    NkIWindow *(NK_CALL *QueryWindow)(_Inout_ NkIRenderer *self);

    /**
     * \brief  resizes the client area of the renderer which is usually congruent with
     *         the client area of the window
     * \param  [in, out] self current \c NkIRenderer instance
     * \param  [in] clAreaSize dimensions of the client area, in pixels
     * \return \c NkErr_Ok on success, non-zero on failure
     */
    NkErrorCode (NK_CALL *Resize)(_Inout_ NkIRenderer *self, _In_ NkSize2D clAreaSize);
    /**
     * \brief  starts a new batch of rendering commands
     * \param  [in, out] self current \c NkIRenderer instance
     * \return \c NkErr_Ok on success, non-zero on failure
     */
    NkErrorCode (NK_CALL *BeginDraw)(_Inout_ NkIRenderer *self);
    /**
     * \brief  finishes a batch of rendering commands, flushes the output device and, if
     *         VSync is enabled, waits (i.e., blocks) for a V-Blank signal
     * \param  [in, out] self current \c NkIRenderer instance
     * \return \c NkErr_Ok on success, non-zero on failure
     */
    NkErrorCode (NK_CALL *EndDraw)(_Inout_ NkIRenderer *self);
    /**
     * \brief   draws a portion of a texture at the given viewport position
     * \param   [in, out] self current \c NkIRenderer instance
     * \param   [in] dstRect rectangle describing the destination of where the texture
     *               (-portion) is to be rendered
     * \param   [in] texPtr pointer to the \c NkRendererResource instance that represents
     *               the texture
     * \param   [in] srcRect rectangle that describes the texture portion that is to be
     *               rendered; can be <tt>NULL</tt> if the entire texture is to be drawn
     *               into the destination rectangle
     * \return  \c NkErr_Ok on success, non-zero on failure
     * \warning If \c texPtr does not identify a valid texture handle, then the behavior
     *          is undefined.
     * 
     * \par Remarks
     *   If \c dstRect anf \c srcRect do not match in their respective extents, then the
     *   texture (-portion) is stretched/compressed so that it fits the destination
     *   rectangle.<br>
     *   If you want to draw the entire bitmap contents from a certain point, configure
     *   \c srcRect so that <tt>(m_xCoord, m_yCoord)</tt> is the upper-left corner of the
     *   the region you want to draw and set both \c m_width and \c m_height to <tt>-1</tt>.
     */
    NkErrorCode (NK_CALL *DrawTexture)(
        _Inout_  NkIRenderer *self,
        _In_     NkRectF const *dstRect,
        _In_     NkRendererResource const *texPtr,
        _In_opt_ NkRectF const *srcRect
    );

    /**
     * \brief  creates a new resource representing a texture, that is, a 2D array of
     *         pixels, from an existing <em>device-independent</em> bitmap
     * \param  [in, out] self current \c NkIRenderer instance
     * \param  [in] resourcePtr pointer to the <em>device-independent</em> bitmap that is
     *              to be used to create the resource
     * \param  [out] resourcePtr pointer to a variable that will receive the pointer to
     *               the newly-created resource instance; an existing instance may be
     *               passed which will cause the old resource to be deleted and the new
     *               resource to be created in-place
     * \return \c NkErr_Ok on success, non-zero on failure
     * \note   If the function succeeds, the \c mp_rdRef member's reference count will be
     *         incremented.
     * 
     * \par Remarks
     *   If the function fails, th contents of both <tt>resourcePtr</tt> and
     *   <tt>*resourcePtr</tt> are indeterminate.
     */
    NkErrorCode (NK_CALL *CreateTexture)(
        _Inout_        NkIRenderer *self,
        _In_           NkDIBitmap const *dibPtr,
        _Maybe_reinit_ NkRendererResource **resourcePtr
    );
    /**
     * \brief   deletes the given resource and frees all memory used by it, including the
     *          memory used to store the instance itself
     * \param   [in, out] self current \c NkIRenderer instance
     * \param   [in, out] resourcePtr pointer to a variable holding the pointer to the
     *                    resource instance that is to be deleted
     * \return  \c NkErr_Ok on success, non-zero on failure
     * \note    \li Use this function to delete any resource created by the
     *              <tt>NkRenderer::Create*()</tt>-family of methods.
     * \note    \li If <tt>(*)resourcePtr</tt> is <tt>NULL</tt>, the function does
     *              nothing.
     * \note    \li If the function succeeds, the reference count of the \c mp_rdRef
     *              member is decremented.
     * \warning The behavior is undefined if the resource was either not created by the
     *          current renderer instance identified by <tt>self</tt>, or if
     *          <tt>resourcePtr</tt> does not identify a valid \c NkRendererResource
     *          instance.
     * 
     * \par Remarks
     *   After the function returns and does so not reporting an error,
     *   <tt>*resourcePtr</tt> will be <tt>NULL</tt>.
     */
    NkErrorCode (NK_CALL *DeleteResource)(_Inout_ NkIRenderer *self, _Uninit_ptr_ NkRendererResource **resourcePtr);
};


/**
 * \brief  does some pre-runtime initialization of some global state associated with the
 *         application
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   This function does not instantiate an actual renderer but rather enables the
 *         application to instantiate the renderer of its own choice. Call this function
 *         once per process before instantiating the first renderer.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkRendererStartup(NkVoid);
/**
 * \brief  uninitializes global renderer state
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   If this function is called without a corresponding call to
 *         <tt>NkRendererStartup()</tt>, this function does nothing.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkRendererShutdown(NkVoid);

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
 *     to be manually memory-managed.
 *    <li>
 *     The array pointed to by \c resPtr may be empty and the return value may be 0 as a
 *     consequence.
 *    </li>
 *   </ul>
 */
NK_NATIVE NK_API NkSize NK_CALL NkRendererQueryAvailablePlatformApis(_Outptr_ NkRendererApi const **resPtr);
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
NK_NATIVE NK_API NkRendererApi NK_CALL NkRendererQueryDefaultPlatformApi(NkVoid);
/**
 * \brief  retrieves the ID (CLSID) of the class that implements the renderer based on
 *         the API identified by \c apiIdent
 * \param  [in] apiIdent identifier of the rendering API
 * \return class ID of the renderer, or \c NULL if the API is unknown or the current
 *         platform does not implement a renderer based on the given API
 */
NK_NATIVE NK_API NkUuid const *NK_CALL NkRendererQueryCLSIDFromApi(_In_ NkRendererApi apiIdent);

/**
 * \brief   checks if both \c r1Ptr and \c r2Ptr are equal in size, that is not just
 *          whether their area is the same but also their side lengths
 * \param   [in] r1Ptr pointer to the first rectangle
 * \param   [in] r2Ptr pointer to the second rectangle
 * \return  non-zero if both rectangles are equal in area and side lengths, zero if not
 * \warning If either \c r1Ptr or \c r2Ptr are \c NULL or in some other way invalid, then
 *          the behavior is undefined.
 */
NK_NATIVE NK_API NkBoolean NK_CALL NkRendererCompareRectangles(_In_ NkRectF const *r1Ptr, _In_ NkRectF const *r2Ptr);


/* Define renderers for the Windows platform. */
#if (defined NK_TARGET_WINDOWS)
/**
 * \interface NkIGdiRenderer
 * \brief     represents a renderer based on Windows' GDI (**G**raphics **D**evice
 *            **I**nterface) technology
 */
NKOM_DECLARE_INTERFACE_ALIAS(NkIRenderer, NkIGdiRenderer);
#endif


