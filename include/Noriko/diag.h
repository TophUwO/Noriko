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
 * \file  diag.h
 * \brief global definitions for Noriko's diagnosis component
 *
 * This header defines the public API for Noriko's built-in debugging and diagnosis tools.
 * Throughout the application's lifetime, this component will collect all data from logs,
 * over allocations, events, errors, etc. and generate a HTML document from it that can
 * be viewed by a browser for easier traversing and filtering.
 * 
 * \note This component is active by default in all build configurations - even in
 *       \c deploy builds. That behavior can be toggled with the <tt>--diag=off</tt>
 *       option. Besides turning it off entirely, there are options to turn off only some
 *       features such as allocation tracking (can be disabled by passing the option
 *       <tt>--diagatrack=off</tt> to the executable).
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>


/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NkDiagInitialize(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NkDiagUninitialize(NkVoid);


