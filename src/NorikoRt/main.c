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


int main(int argc, char **argv) {
    /* Query target platform properties. */
    NkPlatformInformation targetPlatformInfo = {
        .m_structSize = sizeof targetPlatformInfo
    };
    if (NkQueryPlatformInformation(&targetPlatformInfo)) {
        printf("Could not query target platform information.\n");

        return -1;
    }

    /* Print target platform information. */
    printf("NorikoRt powered by %s\n", targetPlatformInfo.mp_prodFullInfoStr->mp_dataPtr);
    return 0;
}


