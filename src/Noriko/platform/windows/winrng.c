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
 * \file  winrng.c
 * \brief implements the random seed generation for the Windows platform
 */
#define NK_NAMESPACE "nk::prng"


/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>
#include <include/Noriko/platform.h>


_Return_ok_ NkErrorCode NK_CALL __NkVirt_PRNG_GenerateSeed(
    _In_                      NkSize sizeInBytes,
    _Out_writes_(sizeInBytes) NkByte *randSeedBuf
) {
    NK_ASSERT(sizeInBytes > 0, NkErr_InParameter);
    NK_ASSERT(randSeedBuf != NULL, NkErr_OutParameter);

#if (defined _MSC_VER)
    /*
     * Use 'rand_s()' (MSVC-specific) for a cryptographically-secure pseudo-random
     * number. Should be usable as a random seed.
     */
    for (NkSize i = 0; i < sizeInBytes / sizeof(unsigned int); i++)
        rand_s(&((unsigned int *)randSeedBuf)[i]);
#else
    #error Need to define "__NkVirt_PRNG_GenerateSeed()" for the current compiler.
#endif

    return NkErr_Ok;
}


#undef NK_NAMESPACE

