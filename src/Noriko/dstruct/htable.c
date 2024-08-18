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
 * \file  htable.c
 * \brief implements the hash table data-structure used by Noriko
 * \todo pass proper value to key hash fn
 */
#define NK_NAMESPACE "nk::dstruct"


/* stdlib includes */
#include <string.h>

/* Noriko includes */
#include <include/Noriko/alloc.h>
#include <include/Noriko/error.h>
#include <include/Noriko/log.h>

#include <include/Noriko/dstruct/htable.h>


/* SipHash macros */
/** \cond */
#pragma region SIPHASH-STUFF
#define ROTL(x, b) (NkUint32)(((x) << (b)) | ((x) >> (32 - (b))))

#define U32TO8_LE(p, v)            \
    (p)[0] = (uint8_t)((v));       \
    (p)[1] = (uint8_t)((v) >> 8);  \
    (p)[2] = (uint8_t)((v) >> 16); \
    (p)[3] = (uint8_t)((v) >> 24);                                

#define U8TO32_LE(p)                                          \
    (((NkUint32)((p)[0])) | ((NkUint32)((p)[1]) << 8) |       \
     ((NkUint32)((p)[2]) << 16) | ((NkUint32)((p)[3]) << 24))     

#define SIPROUND           \
    do {                   \
        v0 += v1;          \
        v1 = ROTL(v1, 5);  \
        v1 ^= v0;          \
        v0 = ROTL(v0, 16); \
        v2 += v3;          \
        v3 = ROTL(v3, 8);  \
        v3 ^= v2;          \
        v0 += v3;          \
        v3 = ROTL(v3, 7);  \
        v3 ^= v0;          \
        v2 += v1;          \
        v1 = ROTL(v1, 13); \
        v1 ^= v2;          \
        v2 = ROTL(v2, 16); \
    } while (0)
#pragma endregion
/** \endcond */


/** \cond INTERNAL */
/**
 * \struct __NkInt_HashtableExtPair
 * \brief represents the hash table pair data-structure used for slots, extended with the
 *        value's offset (i.e., "richness") from its "home" slot (that is, the slot that
 *        the pair's key component originally hashed to)
 */
NK_NATIVE typedef struct __NkInt_HashtableExtPair {
    NkHashtablePair m_regPair; /**< regular pair */
    NkUint32        m_isUsed;  /**< whether or not the slot is in use */
    NkUint32        m_offset;  /**< offset from "home key"; as unsigned 32-bit integer */
} __NkInt_HashtableExtPair;

/**
 * \struct __NkInt_HashtableContext
 * \brief  represents static hashtable context
 * \note   This data-structure is thread-safe.
 */
NK_NATIVE typedef struct __NkInt_HashtableContext {
    NkUint64 m_hashSeed; /**< random hash seed; determined at start-up */
} __NkInt_HashtableContext;
NK_INTERNAL __NkInt_HashtableContext gl_HtContext;

/**
 * \struct NkHashtable
 * \brief  represents the implementation details of the hash table data-structure
 */
NK_NATIVE struct NkHashtable {
    NkUint32                  m_elemCount;  /**< current number of elements stored */
    NkUint32                  m_currCap;    /**< current capacity, in elements */
    NkHashtableProperties     m_htProps;    /**< internal state */
    __NkInt_HashtableExtPair *mp_elemArray; /**< raw element array */
};


/**
 * \brief destroys the hashtable's elements
 * \param [in,out] htPtr pointer to the NkHashtable instance whose elements are to be
 *                 destroyed
 * \note  If no destructor callback was supplied when the hash table was created, this
 *        function does nothing.
 */
NK_INTERNAL NkVoid __NkInt_HashtableFreeElems(_Inout_ NkHashtable *htPtr) {
    if (htPtr->m_htProps.mp_fnElemFree == NULL)
        return;

    /*
     * If the element destroy function is defined, destroy all remaining elements in hash
     * table first.
     * This loop uses two indices, one that iterates over the entire array, and one
     * index that counts the number of elements processed. If j reaches its maximum
     * (that is, the number of elements stored), the loop terminates as the remainder
     * of the slots will be empty anyway.
     * 
     * It is not necessary that the \c m_isUsed field is set to \c NK_FALSE since this
     * function is only called if the hash table is
     *  (a) to be destroyed, invalidating the element array
     *  (b) to be cleared, after which the element array will be zeroed accordingly
     */
    for (NkUint32 i = 0, j = 0; i < htPtr->m_currCap && j < htPtr->m_elemCount; i++) {
        /* Retrieve pointer to entry. Skip entry if not used. */
        __NkInt_HashtableExtPair *pairPtr = (__NkInt_HashtableExtPair *)&htPtr->mp_elemArray[i];
        if (pairPtr->m_isUsed == NK_FALSE)
            continue;

        /* Invoke destructor. */
        (*htPtr->m_htProps.mp_fnElemFree)(&pairPtr->m_regPair.m_keyVal, pairPtr->m_regPair.mp_valuePtr);
        ++j;
    }
}

/**
 * \brief  retrieves the size of the current key type in bytes
 * \param  [in] keyPtr pointer to the key
 * \param  [in] kType numeric key type ID
 * \return key size in bytes
 */
NK_INTERNAL NK_INLINE NkSize __NkInt_HashtableGetKeySizeInBytes(
    _In_ NkHashtableKey const *keyPtr,
    _In_ NkHashtableKeyType kType
) {
    switch (kType) {
        case NkHtKeyTy_Int64:
        case NkHtKeyTy_Uint64:     return sizeof(NkUint64);
        case NkHtKeyTy_Pointer:    return sizeof(NkVoid *);
        case NkHtKeyTy_String:     return (NkSize)strlen(keyPtr->mp_strKey);
        case NkHtKeyTy_StringView: return keyPtr->mp_svKey->m_sizeInBytes;
        default:
            /* Should never happen. */
#pragma warning (suppress: 4127)
            NK_ASSERT_EXTRA(NK_FALSE, NkErr_ObjectState, "Invalid key type. Should not happen.");
    }
}

/**
 * \brief  computes the hash value for the given key
 * \param  [in] keyPtr pointer to the key data-structure
 * \param  [in] kType numeric key type ID
 * \param  [in] htCap capacity of the hash table (value to clamp raw hash value to)
 * \return hash value (clamped)
 * \see    https://github.com/veorq/SipHash/blob/master/halfsiphash.c
 * \note   The hash value is implemented using the \c HalfSipHash algorithm.
 */
NK_INTERNAL NkUint32 __NkInt_HashtableHash(
    _In_ NkHashtableKey const *keyPtr,
    _In_ NkHashtableKeyType kType,
    _In_ NkUint32 htCap
) {
    char const unsigned *ni = NULL;
    switch (kType) {
        case NkHtKeyTy_String:     ni = (char const unsigned *)keyPtr->mp_strKey;             break;
        case NkHtKeyTy_StringView: ni = (char const unsigned *)keyPtr->mp_svKey->mp_dataPtr;  break;
        default:
            ni = (char const unsigned *)keyPtr;
    }
    unsigned char const *kk = (unsigned char const *)&gl_HtContext.m_hashSeed;

    NkUint64 out;
    NkSize const outlen = 8;
    NkSize const inlen  = __NkInt_HashtableGetKeySizeInBytes(keyPtr, kType);
    NkUint32 v0 = 0;
    NkUint32 v1 = 0;
    NkUint32 v2 = UINT32_C(0x6c796765);
    NkUint32 v3 = UINT32_C(0x74656462);
    NkUint32 k0 = U8TO32_LE(kk);
    NkUint32 k1 = U8TO32_LE(kk + 4);
    NkUint32 m;
    unsigned char const *end = ni + inlen - (inlen % sizeof(NkUint32));
    int const left = inlen & 3;
    NkUint32 b = ((NkUint32)inlen) << 24;
    v3 ^= k1;
    v2 ^= k0;
    v1 ^= k1;
    v0 ^= k0;

    if (outlen == 8)
        v1 ^= 0xee;

    for (; ni != end; ni += 4) {
        m = U8TO32_LE(ni);
        v3 ^= m;

        SIPROUND; SIPROUND;

        v0 ^= m;
    }

    switch (left) {
        case 3: b |= ((NkUint32)ni[2]) << 16;
        case 2: b |= ((NkUint32)ni[1]) << 8;
        case 1: b |= ((NkUint32)ni[0]);
        case 0: break;
    }

    v3 ^= b;
    SIPROUND; SIPROUND;
    v0 ^= b;

    if (outlen == 8)
        v2 ^= 0xee;
    else
        v2 ^= 0xff;
    SIPROUND; SIPROUND;
    SIPROUND; SIPROUND;

    b = v1 ^ v3;
    U32TO8_LE((unsigned char *)&out, b);
    if (outlen == 4)
        return 0;

    v1 ^= 0xdd;
    SIPROUND; SIPROUND;
    SIPROUND; SIPROUND;
    b = v1 ^ v3;
    U32TO8_LE((unsigned char *)&out + 4, b);

    return ((out & 0xFFFFFFFF) ^ (out >> 32)) % htCap;
}

/**
 * \brief  inserts an element using Robin-Hood-Hashing
 * \param  [in] htPtr pointer to the NkHashtable structure where the element is to be
 *         inserted
 * \param  [in] pairPtr pointer to the NkHashtablePair structure that is to be inserted
 * \return \c NK_TRUE if the element was inserted, \c NK_FALSE if not
 */
NK_INTERNAL NkBoolean __NkInt_HashtableRobinHoodInsertSingle(
    _In_ NkHashtable *htPtr,
    _In_ NkHashtablePair const *pairPtr
) {
    /* Calculate hash value for given key. */
    NkUint32 const initialHash = __NkInt_HashtableHash(
        &pairPtr->m_keyVal,
        htPtr->m_htProps.m_keyType,
        htPtr->m_currCap
    );

    /* Use Robin-Hood-Hashing to insert elements. */
    NkBoolean       isInserted = NK_FALSE;
    NkHashtablePair tmpPair = *pairPtr;
    for (NkUint32 i = initialHash, currPsa = 0, j = 0; j < htPtr->m_currCap;) {
        /* If current slot is free, insert element. */
        if (htPtr->mp_elemArray[i].m_isUsed == NK_FALSE) {
            htPtr->mp_elemArray[i] = (__NkInt_HashtableExtPair){
                .m_isUsed  = NK_TRUE,
                .m_regPair = tmpPair,
                .m_offset  = currPsa
            };

            isInserted = NK_TRUE;
            break;
        }

        /*
         * Compare PSL. If the PSA of the element that is to be inserted is smaller or
         * equal to the PSA of the item in the current probe slot, continue. Otherwise,
         * swap and continue with the new element.
         */
        if (currPsa <= htPtr->mp_elemArray[i].m_offset)
            ++currPsa;
        else {
            NkHashtablePair tmpPair2 = htPtr->mp_elemArray[i].m_regPair;
            NkUint32        oldPsa = htPtr->mp_elemArray[i].m_offset;

            /* Insert element. */
            htPtr->mp_elemArray[i] = (__NkInt_HashtableExtPair){
                .m_isUsed  = NK_TRUE,
                .m_offset  = currPsa,
                .m_regPair = tmpPair
            };

            /* Update state. */
            currPsa = oldPsa + 1;
            tmpPair = tmpPair2;
        }

        /* Incremement i; if the capacity is reached, wrap around to 0. */
        i = i >= htPtr->m_currCap - 1 ? 0 : ++i;
    }

    return isInserted;
}

/**
 * \brief  resizes and rebuilds the hash table
 * \param  [in,out] htPtr pointer to the NkHashtable data-structure that is to be rebuilt
 * \param  [in] newCap new capacity of the internal slot array
 * \return \c NkErr_Ok on success, non-zero on failure
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_HashtableAdjustCapacity(
    _Inout_ NkHashtable *htPtr,
    _In_    NkUint32 newCap
) {
    /* Create a dummy hash table instance. */
    NkHashtable hTable = { .m_currCap = newCap, .m_elemCount = htPtr->m_elemCount, .m_htProps = htPtr->m_htProps };
    NkErrorCode eCode = NkGPAlloc(
        NK_MAKE_ALLOCATION_CONTEXT(),
        sizeof(__NkInt_HashtableExtPair) * newCap,
        0,
        NK_TRUE,
        &hTable.mp_elemArray
    );
    if (eCode ^ NkErr_Ok)
        return eCode;

    /* Insert all the elements of the old hash table into the new hash table. */
    for (NkUint32 i = 0, j = 0; i < htPtr->m_currCap && j < htPtr->m_elemCount; i++) {
        __NkInt_HashtableExtPair const *htPair = (__NkInt_HashtableExtPair const *)&htPtr->mp_elemArray[i];
        if (htPair->m_isUsed == NK_FALSE)
            continue;

        /*
         * Insert element. If it fails, simply free the dummy hash table's memory as the
         * elements in the current hash table are still valid.
         */
        if (__NkInt_HashtableRobinHoodInsertSingle(&hTable, &htPair->m_regPair) ^ NK_TRUE) {
            NkGPFree(hTable.mp_elemArray);

            return NkErr_CapLimitExceeded;
        }
        ++j;
    }
    /* All elements are transferred. Delete the old element array. */
    NkGPFree(htPtr->mp_elemArray);

    /* Copy over the dummy hash table. */
    *htPtr = hTable;
    return NkErr_Ok;
}

/**
 * \brief   compares the given keys for equality
 * \param   [in] fkPtr pointer to the left key
 * \param   [in] skPtr pointer to the right key
 * \param   [in] keyType numeric key type ID (both \c fkPtr and \c skPtr must be of the
 *               same key type)
 * \return  non-zero if the keys are equal, zero if they are not
 * \warning If either \c fkPtr or \c skPtr are <tt>NULL</tt>, \c keyType is invalid or
 *          unknown, or \c fkPtr and/or \c skPtr are not of the exact type denoted by
 *          <tt>keyType</tt>, the behavior is undefined.
 */
NK_INTERNAL NkBoolean __NkInt_HashtableCompareKeys(
    _In_ NkHashtableKey const *restrict fkPtr,
    _In_ NkHashtableKey const *restrict skPtr,
    _In_ NkHashtableKeyType keyType
) {
    switch (keyType) {
        case NkHtKeyTy_Int64:      return fkPtr->m_int64Key == skPtr->m_int64Key;
        case NkHtKeyTy_Uint64:     return fkPtr->m_uint64Key == skPtr->m_uint64Key;
        case NkHtKeyTy_String:     return !strcmp(fkPtr->mp_strKey, skPtr->mp_strKey);
        case NkHtKeyTy_StringView: return !NkStringViewCompare(fkPtr->mp_svKey, skPtr->mp_svKey);
        case NkHtKeyTy_Pointer:    return fkPtr->mp_ptrKey == skPtr->mp_ptrKey;
        default:
#pragma warning (suppress: 4127)
            NK_ASSERT_EXTRA(
                NK_FALSE,
                NkErr_ObjectState,
                "Invalid key type; weird that that s*it was never validated. Mm."
            );
    }

    /*
     * Should theoretically never happen. If it happens, well, let's hope it will not ...
     */
    return NK_FALSE;
}

/**
 * \brief  locates the given key in the hash table
 * \param  [in] htPtr hash table to search for the key
 * \param  [in] keyPtr pointer to the key that is to be located
 * \return index of the key in the hash table's array; or \c UINT32_MAX if the key could
 *         not be located
 */
NK_INTERNAL NkUint32 __NkInt_HashtableLocKey(_In_ NkHashtable const *htPtr, _In_ NkHashtableKey const *keyPtr) {
    /* Calculate hash value for given key. */
    NkUint32 const initialHash = __NkInt_HashtableHash(keyPtr, htPtr->m_htProps.m_keyType, htPtr->m_currCap);

    /* Iterate through the table and try finding the key. */
    for (NkUint32 i = initialHash, j = 0; j < htPtr->m_currCap; j++) {
        NkBoolean const isKeysEqual = (htPtr->mp_elemArray[i].m_isUsed
            && __NkInt_HashtableCompareKeys(
                keyPtr,
                &htPtr->mp_elemArray[i].m_regPair.m_keyVal,
                htPtr->m_htProps.m_keyType
            )
        );

        /* If keys match, return index. */
        if (isKeysEqual == NK_TRUE)
            return i;
        /* If not, continue with next slot. */
        i = i >= htPtr->m_currCap - 1 ? 0 : i + 1;
    }

    return UINT32_MAX;
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkHashtableCreate(
    _In_       NkHashtableProperties const *htPropsPtr,
    _Init_ptr_ NkHashtable **htPtr
) {
    /* Initialize static context. */
    if (gl_HtContext.m_hashSeed == 0) {
        NkErrorCode eCode;

        if ((eCode = NkPRNGNext(&gl_HtContext.m_hashSeed)) != NkErr_Ok)
            NK_LOG_ERROR(
                "Could not initialize hash table seed. Error: %s (%i).",
                NkGetErrorCodeStr(eCode)->mp_dataPtr,
                eCode
            );
    }

    NK_ASSERT(htPropsPtr, NkErr_InParameter);
    NK_ASSERT(htPtr != NULL, NkErr_OutptrParameter);

    /* Allocate memory for data-structure. */
    NkErrorCode errorCode = NkGPAlloc(NK_MAKE_ALLOCATION_CONTEXT(), sizeof **htPtr, 0, NK_FALSE, htPtr);
    if (errorCode != NkErr_Ok)
        return errorCode;
    /* Allocate memory for element array. */
    errorCode = NkGPAlloc(
        NK_MAKE_ALLOCATION_CONTEXT(),
        sizeof *(*htPtr)->mp_elemArray * htPropsPtr->m_initCap,
        0,
        NK_TRUE,
        &(*htPtr)->mp_elemArray
    );
    if (errorCode != NkErr_Ok) {
        NkGPFree(*htPtr);

        *htPtr = NULL;
        return errorCode;
    }

    /* Init state. */
    (*htPtr)->m_htProps   = *htPropsPtr;
    (*htPtr)->m_elemCount = 0;
    (*htPtr)->m_currCap   = htPropsPtr->m_initCap;
    return NkErr_Ok;
}

NkVoid NK_CALL NkHashtableDestroy(_Uninit_ptr_ NkHashtable **htPtr) {
    if (htPtr == NULL || *htPtr == NULL)
        return;

    /*
     * If the element destroy function is defined, destroy all remaining elements in hash
     * table first.
     */
    __NkInt_HashtableFreeElems(*htPtr);

    /* Free memory used. */
    NkGPFree((*htPtr)->mp_elemArray);
    NkGPFree(*htPtr);
    *htPtr = NULL;
}

NkVoid NK_CALL NkHashtableClear(_Inout_ NkHashtable *htPtr) {
    NK_ASSERT(htPtr != NULL, NkErr_InOutParameter);

    /* Destroy elements if possible. */
    __NkInt_HashtableFreeElems(htPtr);

    /* Shrink array. If this fails, simply use the old array and zero it. */
    NK_IGNORE_RETURN_VALUE(__NkInt_HashtableAdjustCapacity(htPtr, htPtr->m_htProps.m_minCap));
    memset(htPtr->mp_elemArray, 0, htPtr->m_currCap * sizeof *htPtr->mp_elemArray);
    htPtr->m_elemCount = 0;
}

_Return_ok_ NkErrorCode NK_CALL NkHashtableInsert(_Inout_ NkHashtable *htPtr, _In_ NkHashtablePair const *htPairPtr) {
    NK_ASSERT(htPtr != NULL, NkErr_InOutParameter);
    NK_ASSERT(htPairPtr != NULL, NkErr_InParameter);

    return NkHashtableInsertMulti(htPtr, &(NkHashtablePair const *){ htPairPtr }, 1);
}

_Return_ok_ NkErrorCode NK_CALL NkHashtableInsertMulti(
    _Inout_           NkHashtable *htPtr,
    _I_array_(nElems) NkHashtablePair const **htPairArray,
    _In_              NkUint32 nElems
) {
    NK_ASSERT(htPtr != NULL, NkErr_InOutParameter);
    NK_ASSERT(htPairArray != NULL, NkErr_InParameter);

    /* Check capacity constraints. */
    NkUint32 newElemCount;
    if (!NkCheckedUint32Add(htPtr->m_elemCount, nElems, &newElemCount) || newElemCount > htPtr->m_htProps.m_maxCap)
        return NkErr_CapLimitExceeded;

    /* Resize and rehash array if necessary. */
    if (newElemCount / (NkFloat)htPtr->m_currCap >= 0.75f) {
        /** \cond INTERNAL */
        /**
         * \brief global hash table target load-factor
         *
         * This load-factor (normalized to <tt>[0.f <= x <= 1.f]</tt>) is the load-factor
         * that will be targeted when resizing the internal array. However, if the user
         * provided capacity constraints, the hash table never grows beyond that capacity
         * regardless of the target load-factor.
         */
        NK_INTERNAL NkFloat const gl_TarLoadFactor = 0.35f;
        /** \endcond */

        /*
         * Adjust the hash table so that its new load-factor is 35%. This function,
         * however, also takes into account user-provided capacity constraints. These
         * constraints are hard limits.
         */
        NkErrorCode const errorCode = __NkInt_HashtableAdjustCapacity(
            htPtr,
            NK_CLAMP(
                (NkUint32)(newElemCount / gl_TarLoadFactor),
                htPtr->m_htProps.m_minCap,
                htPtr->m_htProps.m_maxCap
            )
        );
        if (errorCode != NkErr_Ok)
            return errorCode;
    }

    /* Insert elements. */
    NkUint32 actAdded = 0;
    for (NkUint32 i = 0; i < nElems; i++) {
        if (NkHashtableContains(htPtr, &htPairArray[i]->m_keyVal) == NK_TRUE)
            continue;

        __NkInt_HashtableRobinHoodInsertSingle(htPtr, htPairArray[i]);
        ++actAdded;
    }

    /* Update state and return. */
    htPtr->m_elemCount += actAdded;
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkHashtableErase(_Inout_ NkHashtable *htPtr, _In_ NkHashtableKey const *keyPtr) {
    NK_ASSERT(htPtr != NULL, NkErr_InOutParameter);
    NK_ASSERT(keyPtr != NULL, NkErr_InParameter);

    /*
     * Simply set the element's "valid" flag to FALSE. That will cause the element to be
     * considered free, so its data may be overwritten. The hash table implementation
     * will never read a slot before having checked its "valid" flag.
     */
    NkUint32 const where2Find = __NkInt_HashtableLocKey(htPtr, keyPtr);
    if (where2Find == UINT32_MAX)
        return NkErr_ItemNotFound;
    htPtr->mp_elemArray[where2Find].m_isUsed = NK_FALSE;

    /*
     * If the user provided a custom key and element destructor function when the hash
     * table was created, call this destructor on the key and the element that are to be
     * erased.
     */
    if (htPtr->m_htProps.mp_fnElemFree != NULL) {
        /* Determine the key pointer that is to be passed. */
        NkVoid *key2Pass = NK_INRANGE_INCL(htPtr->m_htProps.m_keyType, NkHtKeyTy_String, NkHtKeyTy_Pointer) 
            ? &htPtr->mp_elemArray[where2Find].m_regPair.m_keyVal
            : NULL
        ;
        
        /* Call the destructor. */
        (*htPtr->m_htProps.mp_fnElemFree)(key2Pass, htPtr->mp_elemArray[where2Find].m_regPair.mp_valuePtr);
    }

    --htPtr->m_elemCount;
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkHashtableAt(
    _In_     NkHashtable const *htPtr,
    _In_     NkHashtableKey const *keyPtr,
    _Outptr_ NkVoid **valPtr
) {
    NK_ASSERT(htPtr != NULL, NkErr_InParameter);
    NK_ASSERT(keyPtr != NULL, NkErr_InParameter);
    NK_ASSERT(valPtr != NULL, NkErr_OutptrParameter);

    /* Find the key. */
    NkUint32 const where2Find = __NkInt_HashtableLocKey(htPtr, keyPtr);
    if (where2Find == UINT32_MAX) {
        *valPtr = NULL;

        return NkErr_ItemNotFound;
    }
    __NkInt_HashtableExtPair *entryPtr = (__NkInt_HashtableExtPair *)&htPtr->mp_elemArray[where2Find];

    /* Get the value. */
    *valPtr = entryPtr->m_regPair.mp_valuePtr;
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkHashtableExtract(
    _Inout_  NkHashtable *htPtr,
    _In_     NkHashtableKey const *keyPtr,
    _Outptr_ NkVoid **valuePtr
) {
    NK_ASSERT(htPtr != NULL, NkErr_InOutParameter);
    NK_ASSERT(keyPtr != NULL, NkErr_InParameter);
    NK_ASSERT(valuePtr != NULL, NkErr_OutptrParameter);

    /* Find key. */
    NkUint32 const where2Find = __NkInt_HashtableLocKey(htPtr, keyPtr);
    if (where2Find == UINT32_MAX) {
        *valuePtr = NULL;

        return NkErr_ItemNotFound;
    }
    __NkInt_HashtableExtPair *entryPtr = (__NkInt_HashtableExtPair *)&htPtr->mp_elemArray[where2Find];

    /* Let user destruct the key. */
    if (htPtr->m_htProps.mp_fnElemFree)
        (*htPtr->m_htProps.mp_fnElemFree)(&entryPtr->m_regPair.m_keyVal, NULL);

    /* Set "valid" bit to 0 and return stored value pointer. */
    entryPtr->m_isUsed = NK_FALSE;
    *valuePtr = entryPtr->m_regPair.mp_valuePtr;
    --htPtr->m_elemCount;
    return NkErr_Ok;
}

NkBoolean NK_CALL NkHashtableContains(_In_ NkHashtable const *htPtr, _In_ NkHashtableKey const *keyPtr) {
    NK_ASSERT(htPtr != NULL, NkErr_InParameter);
    NK_ASSERT(keyPtr != NULL, NkErr_InParameter);

    return __NkInt_HashtableLocKey(htPtr, keyPtr) ^ UINT32_MAX;
}

_Return_ok_ NkErrorCode NK_CALL NkHashtableForEach(_In_ NkHashtable const *htPtr, _In_ NkHashtableIterFn fnIter) {
    NK_ASSERT(htPtr != NULL, NkErr_InParameter);
    NK_ASSERT(fnIter != NULL, NkErr_CallbackParameter);

    /*
     * Iterate over all valid slots, calling the provided iterator function on each of
     * them.
     */
    NkErrorCode errorCode = NkErr_NoOperation;
    for (NkUint32 i = 0, j = 0; i < htPtr->m_currCap && j < htPtr->m_elemCount; i++) {
        __NkInt_HashtableExtPair *pairPtr = (__NkInt_HashtableExtPair *)&htPtr->mp_elemArray[i];
        if (pairPtr->m_isUsed == NK_FALSE)
            continue;

        /*
         * If the iterator function returns non-zero, interpret this as the signal to
         * terminate iteration.
         */
        if ((errorCode = (*fnIter)(&pairPtr->m_regPair)) != NkErr_Ok)
            return errorCode;
    }

    return errorCode;
}


#undef NK_NAMESPACE


