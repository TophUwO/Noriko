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


int main(int argc, char **argv, char **envp) {
    /* Enable Visual Studio memory leak detector. */
#if (defined _DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

    /* Startup the engine component. */
    NkErrorCode errCode = NkApplicationStartup(&(NkApplicationSpecification const){
        .m_structSize      = sizeof(NkApplicationSpecification),
        .m_enableDbgTools  = NK_TRUE,
        .m_rendererApi     = NkRdApi_Win32GDI,
        .m_isVSync         = NK_FALSE,
        .m_vpAlignment     = NkVpAlign_HCenter | NkVpAlign_VCenter,
        .m_vpExtents       = { 16, 16 },
        .m_dispTileSize    = { 32, 32 },
        .m_allowedWndModes = NkWndMode_All,
        .m_initialWndMode  = NkWndMode_Normal,
        .m_wndFlags        = NkWndFlag_DragResizable | NkWndFlag_DragMovable,
        .mp_nativeHandle   = NULL,
        .m_wndTitle        = NK_MAKE_STRING_VIEW("Noriko Sandbox"),
        .m_argc            = argc,
        .mp_argv           = argv,
        .mp_envp           = envp,
        .m_gameRootDir     = NK_MAKE_STRING_VIEW(NULL)
    });
    if (errCode != NkErr_Ok)
        goto lbl_END;

    /* Start the main loop. */
    errCode = NkApplicationRun();

lbl_END:
    NK_IGNORE_RETURN_VALUE(NkApplicationShutdown());
    return errCode;
}


