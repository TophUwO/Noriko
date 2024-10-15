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
 * \file  bmp.c
 * \brief implements the public API for the Windows BMP file reader and writer
 * 
 * The purpose of this module is to be able to provide basic support for reading,
 * modifying, and writing Windows Bitmap (BMP) files. At least the reader-portion of the
 * API will probably not find much use in production code since image loading will be
 * handled by a dedicated asset loading pipeline with custom file formats and shiii...
 * The writer-portion, however, may persist in primitive 'content creation' features such
 * as screenshots.
 */
#define NK_NAMESPACE "nt::bmp"


/* stdlib includes */
#include <string.h>
#include <stdio.h>

/* Noriko includes */
#include <include/Noriko/bmp.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/log.h>


/** \cond INTERNAL */
/**
 * \struct __NkInt_DIBitmap
 * \brief  represents the internal structure of an \c NkDIBitmap object
 */
NK_NATIVE typedef struct __NkInt_DIBitmap {
    NkBitmapSpecification  m_bSpec;    /**< bitmap specification */
    NkByte                *mp_pxArray; /**< pointer to the raw pixel array */
} __NkInt_DIBitmap;
/* Verify integrity between definitions. */
static_assert(
    sizeof(NkDIBitmap) == sizeof(__NkInt_DIBitmap) && alignof(NkDIBitmap) == alignof(__NkInt_DIBitmap),
    "Size and/or alignment mismatch between \"NkDIBitmap\" and \"__NkInt_DIBitmap\". Check definitions."
);

#pragma pack(push, 1)
/**
 * \struct __NkInt_BitmapFileHeader
 * \brief  represents the file header used by the bitmap file
 */
NK_NATIVE typedef struct __NkInt_BitmapFileHeader {
    NkUint16 m_bfType;     /**< <tt>'BM'</tt> to identify what file type it is */
    NkUint32 m_bfSize;     /**< size of the image file (unreliable and unused) */
    NkUint32 m_bfReserved; /**< reserved; must be 0 */
    NkUint32 m_bfOffBytes; /**< offset of the pixel array, in bytes */
} __NkInt_BitmapFileHeader;
#pragma pack(pop)

/**
 */
NK_NATIVE typedef struct __NkInt_BitmapInfoHeader {
    NkUint32 m_biSize;          /**< size of the DIB header structure */
    NkInt32  m_biWith;          /**< width of the image, in pixels */
    NkInt32  m_biHeight;        /**< height of the image, in pixels */
    NkUint16 m_biPlanes;        /**< not used in Windows Bitmaps */
    NkUint16 m_biBitCount;      /**< size of a pixel, in bits */
    NkUint32 m_biCompression;   /**< compression method or pixel format description */
    NkUint32 m_biSizeImage;     /**< size of the pixel array, in bytes (often 0) */
    NkInt32  m_biXPelsPerMeter; /**< horizontal resolution (unused here) */
    NkInt32  m_biYPelsPerMeter; /**< vertical resolution (unused here) */
    NkUint32 m_blClrUsed;       /**< number of colors used (unused here) */
    NkUint32 m_blClrImportant;  /**< number of important colors (unused here) */
} __NkInt_BitmapInfoHeader;

/**
 */
NK_NATIVE typedef struct __NkInt_BitmapV4InfoHeader {
    NkUint32 m_biSize;          /**< size of the DIB header structure */
    NkInt32  m_biWidth;         /**< width of the image, in pixels */
    NkInt32  m_biHeight;        /**< height of the image, in pixels */
    NkUint16 m_biPlanes;        /**< not used in Windows Bitmaps */
    NkUint16 m_biBitCount;      /**< size of a pixel, in bits */
    NkUint32 m_biCompression;   /**< compression method or pixel format description */
    NkUint32 m_biSizeImage;     /**< size of the pixel array, in bytes (often 0) */
    NkInt32  m_biXPelsPerMeter; /**< horizontal resolution (unused here) */
    NkInt32  m_biYPelsPerMeter; /**< vertical resolution (unused here) */
    NkUint32 m_blClrUsed;       /**< number of colors used (unused here) */
    NkUint32 m_blClrImportant;  /**< number of important colors (unused here) */
    NkUint32 m_redMask;         /**< position of the red component */
    NkUint32 m_greenMask;       /**< position of the green component */
    NkUint32 m_blueMask;        /**< position of the blue component */
    NkUint32 m_alphaMask;       /**< position of the alpha component */
    NkUint32 m_csType;          /**< color space type (unused here) */ 
    NkByte   m_colorEndPt[36];  /**< color endpoints (unused here) */
    NkUint32 m_gammaRed;        /**< gamma correction for red channel (unused here) */
    NkUint32 m_gammaGreen;      /**< gamma correction for green channel (unused here) */
    NkUint32 m_gammaBlue;       /**< gamma correction for blue channel (unused here) */
} __NkInt_BitmapV4InfoHeader;


/**
 */
NK_INTERNAL NK_INLINE NkUint32 __NkInt_DIBitmap_CalculateStride(_In_ NkInt32 bmpWidth, _In_ NkUint16 bitsPerPx) {
    /*
     * See: https://learn.microsoft.com/en-us/windows/win32/api/wingdi/ns-wingdi-bitmapinfoheader#calculating-surface-stride
     */
    return (bmpWidth * (NkUint32)bitsPerPx + 31 & ~31) >> 3;
}

/**
 */
NK_INTERNAL NK_INLINE NkInt32 __NkInt_DIBitmap_GetAdjustedHeight(_In_ NkUint32 bmpHeight, _In_ NkUint32 bmpFlags) {
    return bmpHeight * (bmpFlags & ~NkBmpFlag_Flipped ? -1 : 1);
}

/**
 */
NK_INTERNAL NK_INLINE NkUint32 __NkInt_DIBitmap_MapToCompressionMethod(_In_ NkUint32 bmpFlags) {
    return bmpFlags & NkBmpFlag_UseBitmasks ? BI_BITFIELDS : BI_RGB;
}

/**
 */
NK_INTERNAL NK_INLINE NkUint32 __NkInt_DIBitmap_CalculateRawArraySize(
    _In_ NkInt32 bmpWidth,
    _In_ NkInt32 bmpHeight,
    _In_ NkUint16 bitsPerPx
) {
    return (NkUint32)(__NkInt_DIBitmap_CalculateStride(bmpWidth, bitsPerPx) * bmpHeight);
}

/**
 */
NK_INTERNAL NkUint32 __NkInt_DIBitmap_CvtColor(_In_ NkRgbaColor cCol, _In_ NkBitmapSpecification const *bmpSpecs) {
    NK_ASSERT(bmpSpecs != NULL, NkErr_InParameter);

    /*
     * Build 32-bit integers where every byte in the number is equal to the current color
     * component. This way, we can later easily combine them using the bitmasks provided
     * by *bmpSpecs*.
     */
    NkUint32 const rPat = cCol.m_rVal << 24 | cCol.m_rVal << 16 | cCol.m_rVal << 8 | cCol.m_rVal;
    NkUint32 const gPat = cCol.m_gVal << 24 | cCol.m_gVal << 16 | cCol.m_gVal << 8 | cCol.m_gVal;
    NkUint32 const bPat = cCol.m_bVal << 24 | cCol.m_bVal << 16 | cCol.m_bVal << 8 | cCol.m_bVal;
    NkUint32 const aPat = cCol.m_aVal << 24 | cCol.m_aVal << 16 | cCol.m_aVal << 8 | cCol.m_aVal;

    /*
     * Build the resulting color. Write RGB channels first. If we do not have bitmasks,
     * we cannot have an alpha channel so we write a dummy 255 (0xFF). The following code
     * assumes little endian.
     * If we do not have bitmasks, use the ARGB pixel format which is (because little 
     * endian being expected) saved as BGRA (lower byte offset -> higher byte offset).
     */
    NkUint32 resCol = bmpSpecs->m_bmpFlags & NkBmpFlag_UseBitmasks
        ? rPat & bmpSpecs->m_redMask | gPat & bmpSpecs->m_greenMask | bPat & bmpSpecs->m_blueMask
        : 0xFF000000 | cCol.m_rVal << 16 |  cCol.m_gVal << 8 | cCol.m_bVal
    ;
    /* If we have an alpha channel, write it. */
    return resCol | (bmpSpecs->m_bmpFlags & NkBmpFlag_SuppAlpha ? aPat & bmpSpecs->m_alphaMask : 0);
}

/**
 */
NK_INTERNAL NkVoid __NkInt_DIBitmap_Fill(
    _In_  NkBitmapSpecification const *bmpSpecs,
    _In_  NkUint32 rowSize,
    _In_  NkRgbaColor clearCol,
    _Out_ NkByte *pxArray
) {
    NK_ASSERT(pxArray != NULL, NkErr_OutParameter);

    NkUint32 const pixelWidth  = bmpSpecs->m_bitsPerPx >> 3;
    NkUint32 const cvtClearCol = __NkInt_DIBitmap_CvtColor(clearCol, bmpSpecs);

    /*
     * Fill the first row. For this to work, we first write our first pixel, then we
     * 'append' what has been previously written. This copies chunks that are increasing
     * in size, speeding up the copying process. We do this until the entire row is
     * filled.
     * It must be noted that it doesn't matter if half of a pixel is written into the
     * padding as the padding is never meant to be read and also, as per specification,
     * is not required to be all zeroes.
     * The way this is done is based on this post: https://stackoverflow.com/a/44251649.
     */
    memcpy(pxArray, (NkVoid const *)&cvtClearCol, (NkSize)pixelWidth);
    for (NkUint32 i = pixelWidth; i < rowSize; i += i)
        memcpy(&pxArray[i], (NkVoid const *)pxArray, NK_MIN(rowSize - i, i));

    /*
     * Now, we fill all the subsequent rows by just copying the first row into them. We
     * do this in the same fashion as we did when we filled the first row.
     */
    for (NkInt32 i = 1; i < bmpSpecs->m_bmpHeight; i += i)
        memcpy(&pxArray[i * rowSize], (NkVoid const *)pxArray, rowSize * NK_MIN(bmpSpecs->m_bmpHeight - i, i));
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_DIBitmap_ValidateSpecification(_In_ NkBitmapSpecification const *bmSpecs) {
    NK_ASSERT(bmSpecs != NULL, NkErr_InParameter);

    NkErrorCode errCode = NkErr_Ok;

    /* Validate dimensions. */
    NK_WEAK_ASSERT(errCode, NkErr_InvImageDimensions, bmSpecs->m_bmpWidth > 0, ERROR, "Bitmap width must be > 0!");
    NK_WEAK_ASSERT(errCode, NkErr_InvImageDimensions, bmSpecs->m_bmpHeight > 0, ERROR, "Bitmap height must be > 0!");
    /* Validate bits per pixel. */
    NK_WEAK_ASSERT(
        errCode,
        NkErr_InvBitDepth,
        bmSpecs->m_bitsPerPx == 24 || bmSpecs->m_bitsPerPx == 32,
        ERROR,
        "Bitmap bit depth must be one of the following values: 24, 32! Value: %i",
        (NkInt32)bmSpecs->m_bitsPerPx
    );

    return errCode;
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkDIBitmapCreate(
    _In_     NkBitmapSpecification const *bmpSpecs,
    _In_opt_ NkRgbaColor *clearCol,
    _Out_    NkDIBitmap *resPtr
) {
    NK_ASSERT(bmpSpecs != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    /*
     * Validate bitmap specification. Note that some options, while valid for the BMP
     * file format, are not valid as the implementation does not support it (at least as
     * of now).
     */
    NkErrorCode errCode;
    if ((errCode = __NkInt_DIBitmap_ValidateSpecification(bmpSpecs)) != NkErr_Ok)
        return errCode;

    /* Calculate pixel array size. */
    NkSize rowSize = __NkInt_DIBitmap_CalculateStride(bmpSpecs->m_bmpWidth, bmpSpecs->m_bitsPerPx);
    /* Allocate the pixel array. */
    NkByte *pixelArray;
    errCode = NkGPAlloc(NK_MAKE_ALLOCATION_CONTEXT(), bmpSpecs->m_bmpHeight * rowSize, 0, NK_FALSE, &pixelArray);
    if (errCode != NkErr_Ok)
        return errCode;

    /* If *clearCol* is not NULL, fill the entire surface with that color. */
    if (clearCol != NULL)
        __NkInt_DIBitmap_Fill(bmpSpecs, (NkUint32)rowSize, *clearCol, pixelArray);

    /* Initialize the object. */
    *(__NkInt_DIBitmap *)resPtr = (__NkInt_DIBitmap){
        .m_bSpec = {
            .m_structSize = bmpSpecs->m_structSize,
            .m_bmpWidth   = bmpSpecs->m_bmpWidth,
            .m_bmpHeight  = bmpSpecs->m_bmpHeight,
            .m_bmpStride  = (NkUint32)rowSize,
            .m_bitsPerPx  = bmpSpecs->m_bitsPerPx,
            .m_bmpFlags   = bmpSpecs->m_bmpFlags,
            .m_redMask    = bmpSpecs->m_redMask,
            .m_greenMask  = bmpSpecs->m_greenMask,
            .m_blueMask   = bmpSpecs->m_blueMask,
            .m_alphaMask  = bmpSpecs->m_alphaMask
        },
        .mp_pxArray = pixelArray
    };
    /* All good. */
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkDIBitmapLoad(_In_z_ char const *filePath, _Out_ NkDIBitmap *resPtr) {
    NK_ASSERT(filePath != NULL && *filePath != '\0', NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    /* Open the file for reading. */
    FILE *fStream;
    if (fopen_s(&fStream, filePath, "rb") != 0)
        return NkErr_OpenFile;

    /* Read the headers to get the information on the pixel buffer. */
    __NkInt_BitmapFileHeader   fileHead;
    __NkInt_BitmapV4InfoHeader dibHead;
    fread_s(&fileHead, sizeof fileHead, sizeof fileHead, 1, fStream);
    fread_s(&dibHead.m_biSize, sizeof dibHead.m_biSize, sizeof dibHead.m_biSize, 1, fStream);
    fseek(fStream, (long)sizeof fileHead, SEEK_SET);
    /* Check if the header is supported. */
    if (dibHead.m_biSize == sizeof(__NkInt_BitmapInfoHeader) || dibHead.m_biSize == sizeof(__NkInt_BitmapV4InfoHeader)) {
        /* Header is supported. */
        fread_s(&dibHead, sizeof dibHead, sizeof dibHead, 1, fStream);
    } else {
        /* Header is not supported. */
        NK_LOG_ERROR("DIB headers of size %u are currently not supported.", dibHead.m_biSize);

        fclose(fStream);
        return NkErr_UnsupportedFileFormat;
    }
    fseek(fStream, (long)fileHead.m_bfOffBytes, SEEK_SET);

    /* Allocate memory for the pixel buffer. */
    NkByte *pxBuf;
    NkUint32 const pxBufSize = dibHead.m_biSizeImage
        ? dibHead.m_biSizeImage
        : __NkInt_DIBitmap_CalculateRawArraySize(dibHead.m_biWidth, dibHead.m_biHeight, dibHead.m_biBitCount)
    ;
    NkErrorCode errCode = NkGPAlloc(NK_MAKE_ALLOCATION_CONTEXT(), (NkSize)pxBufSize, 0, NK_FALSE, &pxBuf);
    if (errCode != NkErr_Ok) {
        fclose(fStream);

        return errCode;
    }

    /* Read the pixel buffer directly into the allocated memory. */
    fread_s(pxBuf, pxBufSize, pxBufSize, 1, fStream);

    /* Cleanup and initialize bitmap structure. */
    fclose(fStream);
    *(__NkInt_DIBitmap *)resPtr = (__NkInt_DIBitmap){
        .m_bSpec = {
            .m_structSize = sizeof(NkBitmapSpecification),
            .m_bmpWidth   = dibHead.m_biWidth,
            .m_bmpHeight  = dibHead.m_biHeight,
            .m_bmpStride  = __NkInt_DIBitmap_CalculateStride(dibHead.m_biWidth, dibHead.m_biBitCount),
            .m_bitsPerPx  = dibHead.m_biBitCount,
            .m_bmpFlags   = 0
        },
        .mp_pxArray = pxBuf
    };
    /* Set masks if we are using bitmasks. */
    if (dibHead.m_biCompression == BI_BITFIELDS) {
        __NkInt_DIBitmap *actResPtr = (__NkInt_DIBitmap *)resPtr;

        /* Copy all three main bitmasks by default. */
        actResPtr->m_bSpec.m_redMask = dibHead.m_redMask;
        actResPtr->m_bSpec.m_redMask = dibHead.m_redMask;
        actResPtr->m_bSpec.m_redMask = dibHead.m_redMask;

        /* If the bitmap uses the BITMAPV4INFOHEADER, we copy all four bitmasks. */
        if (dibHead.m_biSize == sizeof(__NkInt_BitmapV4InfoHeader))
            actResPtr->m_bSpec.m_alphaMask = dibHead.m_alphaMask;
    }

    /* All good. */
    return NkErr_Ok;
}

NkVoid NK_CALL NkDIBitmapDestroy(_Inout_ NkDIBitmap *bmpPtr) {
    if (bmpPtr == NULL)
        return;

    NkGPFree(((__NkInt_DIBitmap *)bmpPtr)->mp_pxArray);
}

_Return_ok_ NkErrorCode NK_CALL NkDIBitmapSave(_In_ NkDIBitmap const *bmpPtr, _In_z_ char const *filePath) {
    NK_ASSERT(bmpPtr != NULL, NkErr_InParameter);
    NK_ASSERT(filePath != NULL && *filePath ^ '\0', NkErr_InParameter);

    /* Get pointer to internal bitmap structure. */
    __NkInt_DIBitmap *actBmpPtr = (__NkInt_DIBitmap *)bmpPtr;

    /* Prepare the bitmap file header. */
    __NkInt_BitmapFileHeader const fileHead = {
        .m_bfType     = 'MB',
        .m_bfSize     = 8294522,
        .m_bfReserved = 0,
        .m_bfOffBytes = 14 + 108
    };
    /*
     * Currently, we save our bitmaps with a BITMAPV4INFOHEADER exclusively. Additionally,
     * color space information is unused. We set 'm_csType' to a dummy value since 0
     * signifies 'LCS_CALIBRATED_RGB' which would require to specify valid color space
     * information in the other fields.
     */
    __NkInt_BitmapV4InfoHeader const infoHead = {
        .m_biSize          = sizeof infoHead,
        .m_biWidth         = actBmpPtr->m_bSpec.m_bmpWidth,
        .m_biHeight        = __NkInt_DIBitmap_GetAdjustedHeight(
            actBmpPtr->m_bSpec.m_bmpHeight,
            actBmpPtr->m_bSpec.m_bmpFlags
        ),
        .m_biPlanes        = 1,
        .m_biBitCount      = actBmpPtr->m_bSpec.m_bitsPerPx,
        .m_biCompression   = __NkInt_DIBitmap_MapToCompressionMethod(actBmpPtr->m_bSpec.m_bmpFlags),
        .m_biSizeImage     = __NkInt_DIBitmap_CalculateRawArraySize(
            actBmpPtr->m_bSpec.m_bmpWidth,
            actBmpPtr->m_bSpec.m_bmpHeight,
            actBmpPtr->m_bSpec.m_bitsPerPx
        ),
        .m_biYPelsPerMeter = 0,
        .m_biYPelsPerMeter = 0,
        .m_blClrUsed       = 0,
        .m_blClrImportant  = 0,
        .m_redMask         = actBmpPtr->m_bSpec.m_redMask,
        .m_greenMask       = actBmpPtr->m_bSpec.m_greenMask,
        .m_blueMask        = actBmpPtr->m_bSpec.m_blueMask,
        .m_alphaMask       = actBmpPtr->m_bSpec.m_alphaMask,
        .m_csType          = UINT32_MAX,
        .m_colorEndPt      = { 0 },
        .m_gammaRed        = 0,
        .m_gammaGreen      = 0,
        .m_gammaBlue       = 0
    };

    /* Open the file stream. */
    FILE *fStream;
    if (fopen_s(&fStream, filePath, "wb") != 0)
        return NkErr_OpenFile;
    /* Write file- and DIB header. */
    fwrite((NkVoid const *)&fileHead, sizeof fileHead, 1, fStream);
    fwrite((NkVoid const *)&infoHead, sizeof infoHead, 1, fStream);
    /* Write buffer. */
    fwrite((NkVoid const *)actBmpPtr->mp_pxArray, infoHead.m_biSizeImage, 1, fStream);

    /* All good. */
    fclose(fStream);
    return NkErr_Ok;
}

NkBitmapSpecification const *NK_CALL NkDIBitmapGetSpecification(_In_ NkDIBitmap const *bmpPtr) {
    NK_ASSERT(bmpPtr != NULL, NkErr_InParameter);

    return &((__NkInt_DIBitmap *)bmpPtr)->m_bSpec;
}

NkByte *NK_CALL NkDIBitmapGetPixels(_In_ NkDIBitmap const *bmpPtr, _Out_opt_ NkUint32 *sizePtr) {
    NK_ASSERT(bmpPtr != NULL, NkErr_InParameter);

    __NkInt_DIBitmap *actBmpPtr = (__NkInt_DIBitmap *)bmpPtr;
    if (sizePtr != NULL)
        *sizePtr = actBmpPtr->m_bSpec.m_bmpStride * actBmpPtr->m_bSpec.m_bmpHeight;

    return actBmpPtr->mp_pxArray;
}


#undef NK_NAMESPACE


