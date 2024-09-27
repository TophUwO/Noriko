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
 * \file  winhelpers.c
 * \brief implements global platform-dependent helper functions for the Windows platform
 */
#define NK_NAMESPACE "nk::winhelpers"


/* Noriko includes */
#include <include/Noriko/helpers.h>
#include <include/Noriko/platform.h>


#if (defined NK_TARGET_WINDOWS)
_Return_ok_ NkErrorCode NK_CALL NkGetMaximizedWindowExtents(_Out_ NkSize2D *resPtr) {
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    /* Get size of maximized window instead. */
    *resPtr = (NkSize2D){
        (NkUint64)GetSystemMetrics(SM_CXMAXIMIZED),
        (NkUint64)GetSystemMetrics(SM_CYMAXIMIZED)
    };
    return NkErr_Ok;
}

NkPoint2D NK_CALL NkCalculateInitialWindowPos(_In_ NkSize2D const *wndSize) {
    NK_ASSERT(wndSize != NULL, NkErr_InParameter);

    RECT wkArea;
    if (!SystemParametersInfo(SPI_GETWORKAREA, 0, &wkArea, 0))
        return (NkPoint2D){ 0, 0 };

    /* Center the window in the working area. */
    return (NkPoint2D){
       ((wkArea.right  - wkArea.left) - wndSize->m_width)  / 2,
       ((wkArea.bottom - wkArea.top)  - wndSize->m_height) / 2
    };
}

NkSize2D NK_CALL NkCalculateMaximumViewportExtents(
    _In_  NkInt32 wndStyle,
    _In_  NkInt32 extWndStyle,
    _In_  NkSize2D const *dispTileSize
) {
    NK_ASSERT(dispTileSize != NULL, NkErr_InParameter);

    /* Get size of a maximized window on the main monitor. */
    NkSize2D maxWndSize;
    NK_IGNORE_RETURN_VALUE(NkGetMaximizedWindowExtents(&maxWndSize));

    /* Get the size of the non-client area. */
    RECT nclRect = (RECT){ 0, 0, 0, 0 };
    if (!AdjustWindowRectEx(&nclRect, wndStyle, FALSE, extWndStyle))
        return (NkSize2D){ 0, 0 };

    /* Calculate the maximum client area extents. */
    return (NkSize2D){
        (maxWndSize.m_width  - (nclRect.right  - nclRect.left)) / dispTileSize->m_width,
        (maxWndSize.m_height - (nclRect.bottom - nclRect.top))  / dispTileSize->m_height
    };
}
#endif /* NK_TARGET_WINDOWS */


#undef NK_NAMESPACE


