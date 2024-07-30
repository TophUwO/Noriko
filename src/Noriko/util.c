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
 * \file  util.c
 * \brief implementation of various utilities used by Noriko but do not deserve their own
 *        module because having a gazillion files of 100 bytes each is asinine, guys
 */
#define NK_NAMESPACE "nk::util"


/* Noriko includes */
#include <include/Noriko/component.h>
#include <include/Noriko/util.h>
#include <include/Noriko/log.h>


/** \cond INTERNAL */
/**
 * \struct __NkInt_RandomNumberGeneratorContext
 * \brief  represents the pseudo-random number context
 */
NK_NATIVE typedef struct __NkInt_RandomNumberGeneratorContext {
    NkComponent;             /**< component structure */
    NK_DECL_LOCK(m_mtxLock); /**< mutex lock instance */

    /**
     * \brief random seed array
     * \note  This fields needs to be aligned so that its pointer can be cast to a
     *        pointer of a type of an alignment requirement of 4 safely.
     */
    _Alignas(_Alignof(NkUint64)) uint8_t m_seedArr[4 * sizeof(NkUint64)];
} __NkInt_RandomNumberGeneratorContext;
/**
 * \brief actual instance of the random number generator context 
 */
NK_INTERNAL __NkInt_RandomNumberGeneratorContext gl_RandContext;


/**
 * \brief runs the \c Xoshiro256 algorithm to generate a pseudo-random number
 * \param [out] dstPtr pointer to an NkUint64 variable that will receive the generated
 *              random number
 * \see   https://de.wikipedia.org/wiki/Xorshift#Xoroshiro_und_Xoshiro
 */
NK_INTERNAL NK_INLINE NkVoid __NkInt_PRNGXoshiro256(_Out_ NkUint64 *dstPtr) {
    NK_INTERNAL NkUint64 *s0 = (NkUint64 *)&gl_RandContext.m_seedArr[0 * sizeof(NkUint64)];
    NK_INTERNAL NkUint64 *s1 = (NkUint64 *)&gl_RandContext.m_seedArr[1 * sizeof(NkUint64)];
    NK_INTERNAL NkUint64 *s2 = (NkUint64 *)&gl_RandContext.m_seedArr[2 * sizeof(NkUint64)];
    NK_INTERNAL NkUint64 *s3 = (NkUint64 *)&gl_RandContext.m_seedArr[3 * sizeof(NkUint64)];

    /* Get next number in the sequence. */
    *dstPtr = *s0 + *s3;

    /* Update state. */
    NkUint64 const t = *s1 << 17;
    *s2 ^= *s0;
    *s3 ^= *s1;
    *s1 ^= *s2;
    *s0 ^= *s3;
    *s2 ^= t;
    *s3 = *s3 << 45 | *s3 >> (64 - 45);
}

/**
 * \brief carries out the generation of the random number, without locking
 * \param [out] dstPtr pointer to the memory that will receive the random bytes
 */
NK_INTERNAL NK_INLINE NkVoid __NkInt_PRNGNextNoLock(_Out_ NkUint64 *dstPtr) {
    __NkInt_PRNGXoshiro256(dstPtr);
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkPRNGInit(NkVoid) {
    NK_ENSURE_NOT_INITIALIZED(gl_RandContext);
    
    /* Initialize mutex. */
    NK_INITLOCK(gl_RandContext.m_mtxLock);

#if (defined NK_TARGET_WINDOWS)
    for (NkInt32 i = 0; i < sizeof gl_RandContext.m_seedArr / sizeof(unsigned int); i++)
        rand_s((unsigned int *)&gl_RandContext.m_seedArr + i);
#else
    NK_LOG_WARNING("For this platform, random number generation is not supported.");

    return NkErr_NotImplemented;
#endif
    
    NK_LOG_INFO("init: PRNG");
    NK_INITIALIZE(gl_RandContext);
    return NkErr_Ok;
}

NkVoid NK_CALL NkPRNGUninit(NkVoid) {
    NK_ENSURE_INITIALIZED_VOID(gl_RandContext);
    NK_UNINITIALIZE(gl_RandContext);

    /* Destroy mutex. */
    NK_DESTROYLOCK(gl_RandContext.m_mtxLock);

    NK_LOG_INFO("shutdown: PRNG");
}

_Return_ok_ NkErrorCode NK_CALL NkPRNGNext(_Out_ NkUint64 *outPtr) {
    NK_ASSERT(outPtr != NULL, NkErr_OutParameter);
    NK_ENSURE_INITIALIZED(gl_RandContext);

    /* Get next random number. */
    NK_SYNCHRONIZED(gl_RandContext.m_mtxLock, __NkInt_PRNGNextNoLock(outPtr));
    return NkErr_Ok;
}



_Return_ok_ NkErrorCode NK_CALL NkUuidGenerate(_Out_ NkUuid *uuidPtr) {
    NK_ASSERT(uuidPtr != NULL, NkErr_OutParameter);
    NK_ENSURE_INITIALIZED(gl_RandContext);

    /* Generate two random numbers. */
    NK_SYNCHRONIZED(gl_RandContext.m_mtxLock, {
        __NkInt_PRNGNextNoLock(&uuidPtr->m_asUi64[0]);
        __NkInt_PRNGNextNoLock(&uuidPtr->m_asUi64[1]);
    });

    /* Adjust version field. This implementation generates version 4 UUIDs. */
    uuidPtr->m_asByte[6] = 0b0100 << 4 | uuidPtr->m_asByte[6] & 0x0F;
    uuidPtr->m_asByte[8] = 0b0010 << 6 | uuidPtr->m_asByte[8] & 0x3F;
    return NkErr_Ok;
}

NkBoolean NK_CALL NkUuidIsEqual(_In_ NkUuid const *fUuid, _In_ NkUuid const *sUuid) {
    /* Two UUIDs are equal if all their bits are equal. */
    return fUuid == sUuid || !memcmp(fUuid, sUuid, sizeof *fUuid);
}

_Return_ok_ NkErrorCode NK_CALL NkUuidFromString(_I_bytes_(37) char const *uuidAsStr, _Out_ NkUuid *uuidPtr) {
    NK_ASSERT(uuidAsStr != NULL, NkErr_InParameter);
    NK_ASSERT(uuidPtr != NULL, NkErr_OutParameter);

    NkInt32 i, j;
    for (i = 0, j = 0; j < 16 && *uuidAsStr ^ 0x00; i += 2, j++) {
        if (*uuidAsStr == '-')
            ++uuidAsStr;

        /**
         * \brief character to nibble conversion table
         * \note  Characters not explicitly listed map to <tt>0x0</tt>.
         */
        NK_INTERNAL NkByte const gl_NibbleTable[0x80] = {
            ['0'] = 0x0, ['1'] = 0x1, ['2'] = 0x2, ['3'] = 0x3,
            ['4'] = 0x4, ['5'] = 0x5, ['6'] = 0x6, ['7'] = 0x7,
            ['8'] = 0x8, ['9'] = 0x9, ['a'] = 0xA, ['A'] = 0xA,
            ['b'] = 0xB, ['B'] = 0xB, ['c'] = 0xC, ['C'] = 0xC,
            ['d'] = 0xD, ['D'] = 0xD, ['e'] = 0xE, ['E'] = 0xE,
            ['f'] = 0xF, ['F'] = 0xF
        };
        uuidPtr->m_asByte[j] = gl_NibbleTable[*uuidAsStr] << 4 | gl_NibbleTable[*(uuidAsStr + 1)];
        uuidAsStr += 2;
    }
    if (i != 32) {
        memset(uuidPtr, 0, sizeof *uuidPtr);

        return NkErr_InParameter;
    }

    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkUuidToString(_In_ NkUuid const *uuidPtr, _O_bytes_(37) char *strBuf) {
    NK_ASSERT(uuidPtr != NULL, NkErr_InParameter);
    NK_ASSERT(strBuf != NULL, NkErr_OutParameter);

    for (NkInt32 i = 0; i < sizeof *uuidPtr; i++) {
        /* Format according to the 8-4-4-4-12 normal form. */
        if (i == 4 || i == 6 || i == 8 || i == 10)
            *strBuf++ = '-';

        *strBuf++ = "0123456789abcdef"[uuidPtr->m_asByte[i] >> 4];
        *strBuf++ = "0123456789abcdef"[uuidPtr->m_asByte[i] & 0x0F];
    }
    *strBuf = 0x00;

    return NkErr_Ok;
}


#undef NK_NAMESPACE


