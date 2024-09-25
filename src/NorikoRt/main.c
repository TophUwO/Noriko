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


int main(int argc, char **argv, char **env) {
    /* Enable Visual Studio memory leak detector. */
#if (defined _DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    NkLogInitialize();
    NkTimerInitialize();
    NkAllocInitialize();
    NkPRNGInitialize();
    NkOMInitialize(NK_TRUE);
    NkEnvParse(argc, argv, env);
    NkLayerstackStartup();

    /* Query target platform properties. */
    NkPlatformInformation targetPlatformInfo = { .m_structSize = sizeof targetPlatformInfo };
    NkQueryPlatformInformation(&targetPlatformInfo);
    NK_LOG_INFO("NorikoRt powered by %s", targetPlatformInfo.m_prodFullInfoStr.mp_dataPtr);

    system("pause");
    NkLayerstackShutdown();
    NkEnvCleanup();
    NkOMUninitialize();
    NkPRNGUninitialize();
    NkAllocUninitialize();
    NkTimerUninitialize();
    NkLogUninitialize();
    return 0;
}


