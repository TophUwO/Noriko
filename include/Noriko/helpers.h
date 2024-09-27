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
 * \file  helpers.h
 * \brief defines global platform-dependent helper functions
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>
#include <include/Noriko/error.h>


/**
 * \brief  retrieves the extents of the working area of the primary system monitor
 * 
 * Desktops have a working area, that is, the area that is not obstructed by system menus
 * such as a task bar or a top menu bar. This function retrieves the dimensions of this
 * working area, but only for the primary monitor.
 * 
 * \param  [out] resPtr pointer to an \c NkSize2D structure that will receive the
 *               dimensions of the main monitor's working area
 * \return \c NkErr_Ok on success, non-zero on failure
 * 
 * \par Remarks
 *   If the function fails, the contents of \c resPtr are indeterminate. If the working
 *   area of the primary desktop is not available, the function should return
 *   \c NkErr_NotImplemented and leave \c resPtr untouched.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkQueryMaximizedWindowExtents(_Out_ NkSize2D *resPtr);
/**
 */
NK_NATIVE NK_API NkPoint2D NK_CALL NkCalculateInitialWindowPos(_In_ NkSize2D const *wndSize);
/**
 */
NK_NATIVE NK_API NkSize2D NK_CALL NkCalculateMaximumViewportExtents(
    _In_  NkInt32 wndStyle,
    _In_  NkInt32 extWndStyle,
    _In_  NkSize2D const *dispTileSize
);


