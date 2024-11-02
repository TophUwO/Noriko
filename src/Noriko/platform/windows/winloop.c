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
 * \file  winloop.c
 * \brief implements the platform-dependent portion of the main loop
 *
 * While <tt>NkApplicationRun()</tt> contains the full implementation of the main-loop,
 * some parts of it - such as platform event handling - are delegated to
 * platform-dependent implementations. This file handles the platform-dependent portion
 * of the main-loop for Windows systems.
 */
#define NK_NAMESPACE "nk::winloop"


/* Noriko includes */
#include <include/Noriko/noriko.h>


/**
 */
_Return_ok_ NkErrorCode NK_CALL __NkInt_Application_PlatformLoop(
    _Out_       NkBoolean *isLeave,
    _Inout_opt_ NkVoid *extraCxt
) {
    NK_ASSERT(isLeave != NULL, NkErr_OutParameter);
    NK_UNREFERENCED_PARAMETER(extraCxt);

    MSG currMsg;
    while (PeekMessage(&currMsg, NULL, 0, 0, PM_REMOVE) ^ 0) {
        if (currMsg.message == WM_QUIT) {
            /*
             * Check if the message was actually the Windows 'WM_QUIT' message.
             * The 'WM_QUIT' message is not sent to any window procedures. When
             * this message is received, we leave the main loop because the app
             * was instructed to quit in an orderly manner.
             */
            *isLeave = NK_TRUE;

            return (NkErrorCode)currMsg.wParam;
        }

        TranslateMessage(&currMsg);
        DispatchMessage(&currMsg);
    }

    /*
     * All messages since the last frame have been handled. No WM_QUIT was received, so
     * we continue.
     */
    *isLeave = NK_FALSE;
    return NkErr_Ok;
}


#undef NK_NAMESPACE


