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

#include <include/Noriko/dstruct/vector.h>
#include <include/Noriko/dstruct/htable.h>


