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
 * \file  bmp.h
 * \brief defines the public API for the Windows BMP file reader and writer
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>


/**
 * \enum  NkBitmapFlags
 * \brief represents various bitmap settings representable by a bit flag
 */
NK_NATIVE typedef enum NkBitmapFlags {
    NkBmpFlag_None,        /**< no flags set */

    NkBmpFlag_Flipped,     /**< bitmap is 'upside down' (last row of pixels first) */
    NkBmpFlag_UseBitmasks, /**< whether or not the bitmask fields are valid */
    NkBmpFlags_SuppAlpha   /**< whether or not the alpha channel is valid */
} NkBitmapFlags;

/**
 * \struct NkBitmapSpecification
 * \brief  represents bitmap properties used for creation and to describe the pixel
 *         format
 */
NK_NATIVE typedef struct NkBitmapSpecification {
    NkUint32 m_bmpWidth;    /**< width of the bitmap, in pixels */
    NkUint32 m_bmpHeight;   /**< height of the bitmap, in pixels */
    NkUint32 m_bmpRowWidth; /**< size of a row, in bytes (incl. padding) */
    NkUint16 m_bitsPerPx;   /**< size of one pixel, in bits */
    NkUint16 m_bmpFlags;    /**< boolean-representable bitmap settings */
    NkUint32 m_redMask;     /**< position of the pixel's 'red component' */
    NkUint32 m_greenMask;   /**< position of the pixel's 'green component' */
    NkUint32 m_blueMask;    /**< position of the pixel's 'blue component' */
    NkUint32 m_alphaMask;   /**< position of the pixel's 'alpha component' */
} NkBitmapSpecification;

/**
 * \struct NkDIBitmap
 * \brief  represents a device-independent bitmap (i.e., a bitmap that uses a pixel
 *         format that is standardized and can be used to transfer between devices)
 */
NK_NATIVE typedef struct NkDIBitmap {
    alignas(NkInt64) NkByte __placeholder__[40]; /**< placeholder for internal data */
} NkDIBitmap;


/**
 * \brief  creates a new device-independent blank bitmap with the given properties
 * \param  [in] bmpSpecs pointer to the instance of \c NkBitmapSpecification that
 *              contains the bitmap options
 * \param  [in] clearCol pointer to an instance of \c NkRgbaColor which holds the clear
 *              color that the pixel array will be filled with; can be \c NULL if no
 *              filling should be done
 * \param  [out] resPtr pointer to the \c NkBitmap instance that will receive the
 *               initialized instance
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li If the function fails, no bitmap will be created and the contents of
 *             \c resPtr are indeterminate.
 * \note   \li If \c clearCol is <tt>NULL</tt>, the contents of the pixel array upon 
 *             function return are indeterminate.
 * \note   \li If the \c UseBitmasks option is set in the \c m_bmpFlags member of
 *             \c bmpSpecs, \c clearCol (if not <tt>NULL</tt>) is adjusted to that it
 *             reflects the layout as specified by the bitmasks.
 * 
 * \par Remarks
 *   This function creates a blank bitmap, optionally initialized with the given clear
 *   color. This bitmap has the given dimensions and abides by the bitmap specification
 *   provided.<br>
 *   As of now, only 24- or 32-bit RGB(A) bitmaps are supported, optionally with
 *   bitfields describing the exact pixel layout. If the alpha channel is valid, then the
 *   bitfields must be used as per BMP file format specification.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkDIBitmapCreate(
    _In_     NkBitmapSpecification const *bmpSpecs,
    _In_opt_ NkRgbaColor *clearCol,
    _Out_    NkDIBitmap *resPtr
);
/**
 * \brief  loads a bitmap from a file path
 * \param  [in] filePath path to the bitmap file, <tt>NUL</tt>-terminated
 * \param  [out] resPtr pointer to the \c NkDIBitmap instance that will receive the
 *               representation of the bitmap
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   If the function fails, no bitmap will be created and the contents of \c resPtr
 *         are indeterminate.
 * \see    https://en.wikipedia.org/wiki/BMP_file_format
 * 
 * \par Remarks
 *   As of now, only 24- or 32-bit RGB(A) bitmaps are supported, optionally with
 *   bitfields describing the pixel layout. In technical terms, that corresponds to the
 *   bitmaps of which the DIB-header corresponds to \c BITMAPINFOHEADER or
 *   <tt>BITMAPV4INFOHEADER</tt>.<br>
 *   For a description on the BMP file format and the DIB headers, refer to
 *   <tt>https://en.wikipedia.org/wiki/BMP_file_format</tt>.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkDIBitmapLoad(_In_z_ char const *filePath, _Out_ NkDIBitmap *resPtr);
/**
 * \brief destroys the given device-independent bitmap, freeing all its resources
 * \param [in,out] bmpPtr pointer to the \c NkDIBitmap instance that is to be freed
 * \note  If \c bmpPtr is <tt>NULL</tt>, the function does nothing.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkDIBitmapDestroy(_Inout_ NkDIBitmap *bmpPtr);
/**
 * \brief  saves the given bitmap instance to a file specified by the provided path
 * \param  [in] bmpPtr pointer to the \c NkDIBitmap instance that is to be saved
 * \param  [in] filePath path to the destination file
 * \return \c NkErr_Ok on success, non-zero on failure
 * 
 * \par Remarks
 *   The file is saved to the disk according to the current internal bitmap
 *   specification. The pixels are saved without regards as to whether they were
 *   initialized or not. The saving operation is <tt>atomic</tt>, that is, if it fails,
 *   the changes which might have already been made to the persistent storage device are
 *   rolled back.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkDIBitmapSave(
    _In_   NkDIBitmap const *bmpPtr,
    _In_z_ char const *filePath
);
/**
 * \brief   retrieves the current specification of the current \c NkDIBitmap instance
 * \param   [in] bmpPtr pointer to the \c NkDIBitmap instance of which the specification
 *               is to be retrieved
 * \return  the pointer to the bitmap's specification; cannot be \c NULL
 * \warning If \c bmpPtr is <tt>NULL</tt> or does not point to a valid \c NkDIBitmap
 *          instance, then the behavior is undefined.
 * 
 * \par Remarks
 *   The lifetime of the returned pointer is tied to the lifetime of the bitmap object
 *   itself. Thus, do not call <tt>NkDIBitmapDestroy()</tt> while using the returned
     pointer.
 */
NK_NATIVE NK_API NkBitmapSpecification const *NkDIBitmapGetSpecification(_In_ NkDIBitmap const *bmpPtr);
/**
 * \brief   retrieves a pointer to the bitmap's pixel array
 * \param   [in] bmpPtr pointer to the \c NkDIBitmap instance of which the pixel array
 *               pointer is to be retrieved
 * \param   [out] sizePtr (optional) pointer to a variable of type \c NkSize which will
 *                receive the total (writable) array size, in bytes
 * \return  pointer to the bitmap's pixel array; cannot be \c NULL
 * \warning If \c bmpPtr is <tt>NULL</tt> or does not point to a valid \c NkDIBitmap
 *          instance, then the behavior is undefined.
 * 
 * \par Remarks
 *   The function returns a pointer to the pixel array that can be written to in order to
 *   modify the contents of the bitmap. This should, however, only be done after having
 *   queried the necessary information (such as image dimensions, pixel format, ...). The
 *   implementation never validates the pixel array.<br>
 *   The lifetime of the returned pointer is tied to the lifetime of the bitmap object
 *   itself. Thus, do not call <tt>NkDIBitmapDestroy()</tt> while using the buffer.
 *   Additionally, no inter-thread synchronization is provided by the implementation; if
 *   the bitmap is written to from multiple threads, appropriate synchronization measures
 *   must be employed by the caller.
 */
NK_NATIVE NK_API NkByte *NK_CALL NkDIBitmapGetPixels(_In_ NkDIBitmap const *bmpPtr, _Out_opt_ NkSize *sizePtr);


