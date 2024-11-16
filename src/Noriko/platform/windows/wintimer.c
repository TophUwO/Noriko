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
 * \file  wintimer.c
 * \brief implements Noriko's timing functionality for the Windows platform
 */
#define NK_NAMESPACE "nk::wintimer"


/* Noriko includes */
#include <include/Noriko/timer.h>
#include <include/Noriko/platform.h>


NkUint64 NK_CALL __NkVirt_Timer_GetCurrentTicks(NkVoid) {
    LARGE_INTEGER currTicks;
    QueryPerformanceCounter(&currTicks);

    return (NkUint64)currTicks.QuadPart;
}

NkUint64 NK_CALL __NkVirt_Timer_GetFrequency(NkVoid) {
    LARGE_INTEGER currTicks;
    QueryPerformanceFrequency(&currTicks);

    return (NkUint64)currTicks.QuadPart;
}


#undef NK_NAMESPACE


