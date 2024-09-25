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
        .m_vpAlignment     = NkVpAlign_Default,
        .m_vpExtents       = { 60, 40 },
        .m_glTileSize      = { 16, 16 },
        .m_allowedWndModes = NkWndMode_All,
        .m_initialWndMode  = NkWndMode_Normal | NkWndMode_Visible,
        .m_wndFlags        = NkWndFlag_Default,
        .mp_nativeHandle   = NULL,
        .m_wndTitle        = NK_MAKE_STRING_VIEW("Noriko Sandbox"),
        .m_wndPos          = { INT64_MAX, INT64_MAX },
        .m_argc            = argc,
        .mp_argv           = argv,
        .mp_envp           = envp,
        .m_workingDir      = NK_MAKE_STRING_VIEW("$(appDir)")
    });
    if (errCode != NkErr_Ok)
        goto lbl_END;

    /* Start the main loop. */
    errCode = NkApplicationRun();

lbl_END:
    NK_IGNORE_RETURN_VALUE(NkApplicationShutdown());
    return errCode;
}


