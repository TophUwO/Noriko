/**********************************************************************
 * Noriko   - cross-platform 2-D role-playing game (RPG) game engine  *
 *            for desktop and mobile console platforms                *
 * NorikoRt - game launcher and runtime component of the Noriko game  *
 *            engine ecosystem                                        *
 *                                                                    *
 * (c) 2024 TophUwO <tophuwo01@gmail.com>. All rights reserved.       *
 *                                                                    *
 * The source code is licensed under the Apache License 2.0. Refer    *
 * to the LICENSE file in the root directory of this project. If this *
 * file is not present, visit                                         *
 *     https://www.apache.org/licenses/LICENSE-2.0                    *
 **********************************************************************/

/**
 * \file  main.c
 * \brief entrypoint of Noriko's runtime application
 */


/* stdlib includes */
#include <stdio.h>

/* Noriko includes */
#include <include/Noriko/noriko.h>


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    /* Enable Visual Studio memory leak detector. */
#if (defined _DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    if (NkLogInitialize() != NkErr_Ok)
        return -1;
    NkPRNGInitialize();

    /* Query target platform properties. */
    NkPlatformInformation targetPlatformInfo = { .m_structSize = sizeof targetPlatformInfo };
    if (NkQueryPlatformInformation(&targetPlatformInfo)) {
        NK_LOG_ERROR("Could not query target platform information.");

        return -1;
    }

    /* Print target platform information. */
    //NkFatalTerminate(&(NkFatalErrorContext){ NkErr_ArrayElemOutOfBounds, __LINE__, NK_MAKE_STRING_VIEW(u8"fuck you"), NK_MAKE_STRING_VIEW(""), NK_MAKE_STRING_VIEW(__FILE__), NK_MAKE_STRING_VIEW("nk"), NK_MAKE_STRING_VIEW(__func__)  });
    NK_LOG_INFO("NorikoRt powered by %s", targetPlatformInfo.m_prodFullInfoStr.mp_dataPtr);

    NkUuid uuid1, uuid2;
    for (int i = 0; i < 32; i++) {
        NkUuidGenerate(&uuid1);

        char buf[100];
        NkUuidToString(&uuid1, buf);
        NK_LOG_DEBUG("%s", buf);
    }

    char buffer[100];
    char *uuid = "8e6f941b-7d40-4b91-8752-cce70d22671f";
    NkUuidFromString(uuid, &uuid2);
    NkUuidToString(&uuid2, buffer);
    NK_LOG_CRITICAL("%s", buffer);

    system("pause");
    NkPRNGUninitialize();
    NkLogUninitialize();
    return 0;
}


