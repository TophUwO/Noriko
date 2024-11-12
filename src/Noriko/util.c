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


/* stdlib includes */
#include <stdarg.h>
#include <string.h>

/* Noriko includes */
#include <include/Noriko/util.h>
#include <include/Noriko/log.h>
#include <include/Noriko/timer.h>
#include <include/Noriko/window.h>

#include <include/Noriko/dstruct/vector.h>
#include <include/Noriko/dstruct/htable.h>


/** \cond INTERNAL */
/**
 * \struct __NkInt_RandomNumberGeneratorContext
 * \brief  represents the pseudo-random number context
 */
NK_NATIVE typedef struct __NkInt_RandomNumberGeneratorContext {
    NK_DECL_LOCK(m_mtxLock); /**< mutex lock instance */

    /**
     * \brief random seed array
     * \note  This fields needs to be aligned so that its pointer can be cast to a
     *        pointer of a type of an alignment requirement of 4 safely.
     */
    _Alignas(_Alignof(NkUint64)) NkByte m_seedArr[4 * sizeof(NkUint64)];
} __NkInt_RandomNumberGeneratorContext;
/**
 * \brief actual instance of the random number generator context 
 */
NK_INTERNAL __NkInt_RandomNumberGeneratorContext gl_RandContext;

/**
 * \struct __NkInt_Variant
 * \brief  provides the internal implementation of the variant type
 */
NK_NATIVE typedef struct __NkInt_Variant {
    NkVariantType m_type;     /**< ID of the underlying type */
    NkUint32      m_reserved; /**< prevent compiler from inserting padding here */

    union {
        NkBoolean     m_boolVal;
        char          m_chVal;
        NkInt8        m_i8Val;
        NkInt16       m_i16Val;
        NkInt32       m_i32Val;
        NkInt64       m_i64Val;
        NkUint8       m_u8Val;
        NkUint16      m_u16Val;
        NkUint32      m_u32Val;
        NkUint64      m_u64Val;
        NkDouble      m_dblVal;
        NkErrorCode   m_ecVal;
        NkStringView  m_svVal;
        NkBufferView  m_bufVal;
        NkUuid        m_uuidVal;
        NkVoid       *mp_ptrVal;
        NkVector     *mp_vecVal;
        NkHashtable  *mp_htVal;
        NkTimer      *mp_tVal;
    } m_val;
} __NkInt_Variant;
/*
 * Since the public type is just a placeholder for the internal type defined here, we
 * must make sure that the internal type and the public type are compatible, that is,
 * they adhere to the same size and alignment requirements.
 */
static_assert(
    sizeof(NkVariant) == sizeof(__NkInt_Variant) && _Alignof(NkVariant) == _Alignof(__NkInt_Variant),
    "Size and/or alignment mismatch between \"NkVariant\" and \"__NkInt_Variant\". Check definitions."
);

/**
 * \union __NkInt_Uuid
 * \brief auxiliary data-structure used only internally in order to do certain operations
 *        on the UUID itself (such as generation, formatting, converting, etc.)
 * \note  While the public definition is designed to make static instantiation as easy as
 *        possible, this internal implementation is supposed to make it easier to read
 *        from or write data to the UUID structure itself.
 */
NK_NATIVE typedef union __NkInt_Uuid {
    NkUint64 m_asUi64[2];  /**< allow writing 8 bytes at the same time (used when generating UUID) */
    NkByte   m_asByte[16]; /**< allow accessing individual bytes (used when formatting and converting) */
} __NkInt_Uuid;
/*
 * Verify that the alignment and size requirements of the internal representation are
 * congruent with those of the public implementation.
 */
static_assert(
    sizeof(NkUuid) == sizeof(__NkInt_Uuid) && alignof(NkUuid) == alignof(__NkInt_Uuid),
    "Size and/or alignment mismatch between \"NkUuid\" and \"__NkInt_Uuid\". Check definitions."
);


/**
 * \brief runs the \c Xoshiro256 algorithm to generate a pseudo-random number
 * \param [out] dstPtr pointer to an NkUint64 variable that will receive the generated
 *              random number
 * \see   https://de.wikipedia.org/wiki/Xorshift#Xoroshiro_und_Xoshiro
 */
NK_INTERNAL NK_INLINE NkVoid __NkInt_PRNGXoshiro256(_Out_ NkUint64 *dstPtr) {
    NK_INTERNAL NkUint64 *const s0 = (NkUint64 *const)&gl_RandContext.m_seedArr[0 * sizeof(NkUint64)];
    NK_INTERNAL NkUint64 *const s1 = (NkUint64 *const)&gl_RandContext.m_seedArr[1 * sizeof(NkUint64)];
    NK_INTERNAL NkUint64 *const s2 = (NkUint64 *const)&gl_RandContext.m_seedArr[2 * sizeof(NkUint64)];
    NK_INTERNAL NkUint64 *const s3 = (NkUint64 *const)&gl_RandContext.m_seedArr[3 * sizeof(NkUint64)];

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

/**
 * \brief   retrieves the size in bytes of the underlying type for the given variant type
 *          ID
 * \param   [in] varType type ID of the variant
 * \return  size of the underlying type, in bytes
 * \warning Passing invalid (that is, out of range) values for \c varType results in
 *          undefined behavior.
 */
NK_INTERNAL NK_INLINE NkSize __NkInt_VariantGetTypeSize(_In_ NkVariantType varType) {
    NK_ASSERT(varType >= NkVarTy_None && varType < __NkVarTy_Count__, NkErr_InParameter);

    /** \cond INTERNAL */
    /**
     * \brief static table of variant value sizes
     */
    NK_INTERNAL NkSize const gl_c_VarTypeSizes[] = {
        [NkVarTy_None]       = 0,
        [NkVarTy_Boolean]    = sizeof(NkBoolean),
        [NkVarTy_Char]       = sizeof(char),
        [NkVarTy_Int8]       = sizeof(NkInt8),
        [NkVarTy_Int16]      = sizeof(NkInt16),
        [NkVarTy_Int32]      = sizeof(NkInt32),
        [NkVarTy_Int64]      = sizeof(NkInt64),      
        [NkVarTy_Uint8]      = sizeof(NkUint8),      
        [NkVarTy_Uint16]     = sizeof(NkUint16),     
        [NkVarTy_Uint32]     = sizeof(NkUint32),     
        [NkVarTy_Uint64]     = sizeof(NkUint64),     
        [NkVarTy_Float]      = sizeof(NkFloat),      
        [NkVarTy_Double]     = sizeof(NkDouble),     
        [NkVarTy_ErrorCode]  = sizeof(NkErrorCode),  
        [NkVarTy_StringView] = sizeof(NkStringView),
        [NkVarTy_BufferView] = sizeof(NkBufferView),
        [NkVarTy_Uuid]       = sizeof(NkUuid),       
        [NkVarTy_Pointer]    = sizeof(NkVoid *),     
        [NkVarTy_Vector]     = sizeof(NkVector *),   
        [NkVarTy_Hashtable]  = sizeof(NkHashtable *),
        [NkVarTy_Timer]      = sizeof(NkTimer *),
        [NkVarTy_NkOMObject] = sizeof(NkVoid *)
    };
    NK_VERIFY_LUT(gl_c_VarTypeSizes, NkVariantType, __NkVarTy_Count__);
    /** \endcond */

    return gl_c_VarTypeSizes[varType];
}
/** \endcond */


NkStringView *NK_CALL NkStringViewSet(_In_z_ char const *strPtr, _Out_ NkStringView *resPtr) {
    NK_ASSERT(strPtr != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);
    
    *resPtr = (NkStringView){ .mp_dataPtr = (char *)strPtr, .m_sizeInBytes = (NkSize)strlen(strPtr) };
    return resPtr;
}

NkInt32 NK_CALL NkStringViewCompare(_In_ NkStringView const *sv1Ptr, _In_ NkStringView const *sv2Ptr) {
    NK_ASSERT(sv1Ptr != NULL, NkErr_InParameter);
    NK_ASSERT(sv2Ptr != NULL, NkErr_InParameter);

    if (sv1Ptr == sv2Ptr || sv1Ptr->m_sizeInBytes == sv2Ptr->m_sizeInBytes) {
        /*
         * If two string wie pointers are equal, their data pointers will also be the
         * same. If the pointers are unequal but the sizes are the same, due to
         * short-circuit evaluation, their data pointers will differ, too.
         */
        if (sv1Ptr->mp_dataPtr == sv2Ptr->mp_dataPtr)
            return 0;

        return (NkInt32)strncmp(sv1Ptr->mp_dataPtr, sv2Ptr->mp_dataPtr, sv1Ptr->m_sizeInBytes);
    }

    return 1;
}

NkVoid NK_CALL NkStringViewCopy(_In_ NkStringView const *srcPtr, _Out_ NkStringView *dstPtr) {
    NK_ASSERT(srcPtr != NULL, NkErr_InParameter);
    NK_ASSERT(dstPtr != NULL, NkErr_OutParameter);

    memcpy_s(dstPtr, sizeof *dstPtr, srcPtr, sizeof *srcPtr);
}


_Return_ok_ NkErrorCode NK_CALL NkPRNGInitialize(NkVoid) {
    /* Initialize mutex. */
    NK_INITLOCK(gl_RandContext.m_mtxLock);

#if (defined NK_TARGET_WINDOWS)
    for (NkInt32 i = 0; i < sizeof gl_RandContext.m_seedArr / sizeof(unsigned int); i++)
        rand_s((unsigned int *)&gl_RandContext.m_seedArr + i);
#else
    NK_LOG_WARNING("For this platform, random number generation is not supported.");

    return NkErr_NotImplemented;
#endif
    
    /** \todo use exact convention of when exactly to log init/uninit infos (either before or after) */
    NK_LOG_INFO("startup: PRNG");
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkPRNGUninitialize(NkVoid) {
    /* Destroy mutex. */
    NK_DESTROYLOCK(gl_RandContext.m_mtxLock);

    NK_LOG_INFO("shutdown: PRNG");
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkPRNGNext(_Out_ NkUint64 *outPtr) {
    NK_ASSERT(outPtr != NULL, NkErr_OutParameter);

    /* Get next random number. */
    NK_SYNCHRONIZED(gl_RandContext.m_mtxLock, __NkInt_PRNGNextNoLock(outPtr));
    return NkErr_Ok;
}


NkVoid NK_CALL NkUuidGenerate(_Out_ NkUuid *uuidPtr) {
    NK_ASSERT(uuidPtr != NULL, NkErr_OutParameter);

    /* Cast UUID struct to internal representation. */
    __NkInt_Uuid *intUuid = (__NkInt_Uuid *)uuidPtr;
    /* Generate two random numbers. */
    NK_SYNCHRONIZED(gl_RandContext.m_mtxLock, {
        __NkInt_PRNGNextNoLock(&intUuid->m_asUi64[0]);
        __NkInt_PRNGNextNoLock(&intUuid->m_asUi64[1]);
    });

    /* Adjust version field. This implementation generates version 4 UUIDs. */
    intUuid->m_asByte[6] = 0b0100 << 4 | intUuid->m_asByte[6] & 0x0F;
    intUuid->m_asByte[8] = 0b0010 << 6 | intUuid->m_asByte[8] & 0x3F;
}

NkBoolean NK_CALL NkUuidIsEqual(_In_ NkUuid const *fUuid, _In_ NkUuid const *sUuid) {
    NK_ASSERT(fUuid != NULL, NkErr_InParameter);
    NK_ASSERT(sUuid != NULL, NkErr_InParameter);
    static_assert(
        sizeof(NkUint64) == 8 && alignof(NkUint64) == 8,
        "Size and alignment requirement of type \"long long unsigned\" must be 8."
    );

    return fUuid == sUuid || (NkBoolean)(
           ((NkUint64 *)fUuid)[0] == ((NkUint64 *)sUuid)[0]
        && ((NkUint64 *)fUuid)[1] == ((NkUint64 *)sUuid)[1]
    );
}

_Return_ok_ NkErrorCode NK_CALL NkUuidFromString(_I_bytes_(NK_UUIDLEN) char const *uuidAsStr, _Out_ NkUuid *uuidPtr) {
    NK_ASSERT(uuidAsStr != NULL, NkErr_InParameter);
    NK_ASSERT(uuidPtr != NULL, NkErr_OutParameter);

    NkInt32 i, j;
    for (i = 0, j = 0; j < 16 && *uuidAsStr ^ 0x00; i += 2, j++) {
        if (*uuidAsStr == '-')
            ++uuidAsStr;

        /** \cond INTERNAL */
        /**
         * \brief character to nibble conversion table
         * \note  Characters not explicitly listed map to <tt>0x0</tt>.
         */
        NK_INTERNAL NkByte const gl_c_NibbleTable[0x80] = {
            ['0'] = 0x0, ['1'] = 0x1, ['2'] = 0x2, ['3'] = 0x3,
            ['4'] = 0x4, ['5'] = 0x5, ['6'] = 0x6, ['7'] = 0x7,
            ['8'] = 0x8, ['9'] = 0x9, ['a'] = 0xA, ['A'] = 0xA,
            ['b'] = 0xB, ['B'] = 0xB, ['c'] = 0xC, ['C'] = 0xC,
            ['d'] = 0xD, ['D'] = 0xD, ['e'] = 0xE, ['E'] = 0xE,
            ['f'] = 0xF, ['F'] = 0xF
        };
        /** \endcond */
        ((__NkInt_Uuid *)uuidPtr)->m_asByte[j] = gl_c_NibbleTable[*uuidAsStr] << 4 | gl_c_NibbleTable[*(uuidAsStr + 1)];
        uuidAsStr += 2;
    }
    if (i != 32) {
        memset(uuidPtr, 0, sizeof *uuidPtr);

        return NkErr_InParameter;
    }

    return NkErr_Ok;
}

char *NK_CALL NkUuidToString(_In_ NkUuid const *uuidPtr, _O_bytes_(NK_UUIDLEN) char *strBuf) {
    NK_ASSERT(uuidPtr != NULL, NkErr_InParameter);
    NK_ASSERT(strBuf != NULL, NkErr_OutParameter);

    /* Save starting address for later return. */
    char *startBuf = strBuf;

    for (NkInt32 i = 0; i < sizeof *uuidPtr; i++) {
        /* Format according to the 8-4-4-4-12 normal form. */
        if (i == 4 || i == 6 || i == 8 || i == 10)
            *strBuf++ = '-';

        *strBuf++ = "0123456789abcdef"[((__NkInt_Uuid *)uuidPtr)->m_asByte[i] >> 4];
        *strBuf++ = "0123456789abcdef"[((__NkInt_Uuid *)uuidPtr)->m_asByte[i] & 0x0F];
    }
    *strBuf = 0x00;

    return startBuf;
}

NkVoid NK_CALL NkUuidCopy(_In_ NkUuid const *srcPtr, _Out_ NkUuid *resPtr) {
    NK_ASSERT(srcPtr != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    memcpy((void *)(__NkInt_Uuid *)resPtr, (void const *)(__NkInt_Uuid *)srcPtr, sizeof(__NkInt_Uuid));
}


NkVoid NK_CALL NkVariantGet(_In_ NkVariant const *varPtr, _Out_opt_ NkVariantType *tyPtr, _Out_opt_ NkVoid *valPtr) {
    NK_ASSERT(varPtr != NULL, NkErr_InParameter);

    __NkInt_Variant const *intVarPtr = (__NkInt_Variant const *)varPtr;
    NK_ASSERT(NK_INRANGE_INCL(intVarPtr->m_type, NkVarTy_None, __NkVarTy_Count__ - 1), NkErr_ObjectState);

    if (tyPtr != NULL) *tyPtr = intVarPtr->m_type;
    if (valPtr != NULL)
        memcpy(valPtr, &intVarPtr->m_val, __NkInt_VariantGetTypeSize(intVarPtr->m_type));
}

NkVoid NK_CALL NkVariantSet(_Pre_maybevalid_ _Out_ NkVariant *varPtr, _In_ NkVariantType valType, ...) {
    NK_ASSERT(varPtr != NULL, NkErr_InOutParameter);
    NK_ASSERT(NK_INRANGE_INCL(valType, NkVarTy_None, __NkVarTy_Count__ - 1), NkErr_InvalidRange);

    __NkInt_Variant *vPtr = (__NkInt_Variant *)varPtr;

    va_list vlArg;
    va_start(vlArg, valType);

    switch (valType) {
        case NkVarTy_None:
            memset((NkVoid *)&vPtr->m_val, 0, sizeof vPtr->m_val);

            break;
        case NkVarTy_Boolean:    vPtr->m_val.m_boolVal = va_arg(vlArg, NkBoolean);   break;
        case NkVarTy_Char:       vPtr->m_val.m_chVal   = va_arg(vlArg, char);        break;
        case NkVarTy_Int8:       vPtr->m_val.m_i8Val   = va_arg(vlArg, NkInt8);      break;
        case NkVarTy_Int16:      vPtr->m_val.m_i16Val  = va_arg(vlArg, NkInt16);     break;
        case NkVarTy_Int32:      vPtr->m_val.m_i32Val  = va_arg(vlArg, NkInt32);     break;
        case NkVarTy_Int64:      vPtr->m_val.m_i64Val  = va_arg(vlArg, NkInt64);     break;
        case NkVarTy_Uint8:      vPtr->m_val.m_u8Val   = va_arg(vlArg, NkUint8);     break;
        case NkVarTy_Uint16:     vPtr->m_val.m_u16Val  = va_arg(vlArg, NkUint16);    break;
        case NkVarTy_Uint32:     vPtr->m_val.m_u32Val  = va_arg(vlArg, NkUint32);    break;
        case NkVarTy_Uint64:     vPtr->m_val.m_u64Val  = va_arg(vlArg, NkUint64);    break;
        case NkVarTy_Float:
        case NkVarTy_Double:     vPtr->m_val.m_dblVal  = va_arg(vlArg, NkDouble);    break;
        case NkVarTy_ErrorCode:  vPtr->m_val.m_ecVal   = va_arg(vlArg, NkErrorCode); break;
        case NkVarTy_StringView:
        case NkVarTy_BufferView:
            /* Differentiate to prevent issues when ABI of either type changes. */
            memcpy(
                (NkVoid *)&vPtr->m_val.m_svVal,
                va_arg(vlArg, NkVoid const *),
                valType == NkVarTy_StringView
                    ? sizeof(NkStringView)
                    : sizeof(NkBufferView)
            );

            break;
        case NkVarTy_Uuid:
            memcpy(&vPtr->m_val.m_uuidVal, va_arg(vlArg, NkUuid *), sizeof(NkUuid));

            break;
        case NkVarTy_Pointer:
        case NkVarTy_Vector:
        case NkVarTy_Hashtable:
        case NkVarTy_Timer:
        case NkVarTy_NkOMObject: vPtr->m_val.mp_ptrVal = va_arg(vlArg, NkVoid *);    break;
    }

    vPtr->m_type = valType;
    va_end(vlArg);
}

NkVoid NK_CALL NkVariantCopy(_In_ NkVariant const *srcPtr, _Out_ NkVariant *dstPtr) {
    NK_ASSERT(srcPtr != NULL, NkErr_InParameter);
    NK_ASSERT(dstPtr != NULL, NkErr_OutParameter);

    memcpy_s(dstPtr, sizeof *dstPtr, srcPtr, sizeof *srcPtr);
}

NkBoolean NK_CALL NkVariantIsNull(_In_ NkVariant const *varPtr) {
    NK_ASSERT(varPtr != NULL, NkErr_InParameter);

    return ((__NkInt_Variant *)varPtr)->m_type == NkVarTy_None;
}

NkStringView const *NK_CALL NkVariantQueryTypeStr(_In_ NkVariantType typeId) {
    NK_ASSERT(typeId >= NkVarTy_None && typeId < __NkVarTy_Count__, NkErr_InParameter);

    /** \cond INTERNAL */
    /**
     */
    NK_INTERNAL NkStringView const gl_c_VarTypeStrs[] = {
        [NkVarTy_None]       = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_None)),
        [NkVarTy_Boolean]    = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Boolean)),
        [NkVarTy_Char]       = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Char)),
        [NkVarTy_Int8]       = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Int8)),
        [NkVarTy_Int16]      = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Int16)),
        [NkVarTy_Int32]      = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Int32)),
        [NkVarTy_Int64]      = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Int64)),
        [NkVarTy_Uint8]      = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Uint8)),
        [NkVarTy_Uint16]     = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Uint16)),
        [NkVarTy_Uint32]     = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Uint32)),
        [NkVarTy_Uint64]     = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Uint64)),
        [NkVarTy_Float]      = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Float)),
        [NkVarTy_Double]     = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Double)),
        [NkVarTy_ErrorCode]  = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_ErrorCode)),
        [NkVarTy_StringView] = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_StringView)),
        [NkVarTy_BufferView] = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_BufferView)),
        [NkVarTy_Uuid]       = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Uuid)),
        [NkVarTy_Pointer]    = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Pointer)),
        [NkVarTy_Vector]     = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Vector)),
        [NkVarTy_Hashtable]  = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Hashtable)),
        [NkVarTy_Timer]      = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_Timer)),
        [NkVarTy_NkOMObject] = NK_MAKE_STRING_VIEW(NK_ESC(NkVarTy_NkOMObject))
    };
    NK_VERIFY_LUT(gl_c_VarTypeStrs, NkVariantType, __NkVarTy_Count__);
    /** \endcond */

    return &gl_c_VarTypeStrs[typeId];
}


NkVoid NK_CALL NkRawStringTrim(
    _In_z_ char const *strPtr,
    _In_   NkSize maxChars,
    _In_z_ char const *keyPtr,
    _Out_  NkStringView *resPtr
) {
    NK_ASSERT(strPtr != NULL, NkErr_InParameter);
    NK_ASSERT(keyPtr != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    /* Skip leading characters that are to be trimmed. */
    char *modPtr = (char *)strPtr;
    resPtr->mp_dataPtr = modPtr;
    while (strpbrk(resPtr->mp_dataPtr, keyPtr) == resPtr->mp_dataPtr)
        ++resPtr->mp_dataPtr;
    resPtr->mp_dataPtr = NK_MIN(resPtr->mp_dataPtr, modPtr + maxChars);

    /* Trim characters from the end. */
    char *endPtr = strchr(resPtr->mp_dataPtr, '\0');
    if (endPtr == NULL) {
        *resPtr = (NkStringView){ .mp_dataPtr = NULL, .m_sizeInBytes = 0 };

        return;
    }
    endPtr = NK_MIN(endPtr, modPtr + maxChars);

    NkSize const keyLen = (NkSize const)strlen(keyPtr);
lbl_NEXTCHAR:
    while (endPtr >= resPtr->mp_dataPtr) {
        /*
         * Implement a reverse 'strpbrk' clone that searches until none of the characters
         * in *keyStr* match the current end pointer.
         */
        for (NkSize i = 0; i < keyLen; i++)
            if (*endPtr == keyPtr[i]) {
                --endPtr;

                goto lbl_NEXTCHAR;
            }

        break;
    }

    /* Calculate size of string (excl. NUL). */
    *resPtr = (NkStringView){ .mp_dataPtr = resPtr->mp_dataPtr, .m_sizeInBytes = endPtr - resPtr->mp_dataPtr };
}

NkVoid NK_CALL NkRawStringSplit(
    _In_z_ char const *strPtr,
    _In_z_ char const *ctrlChs,
    _Out_  NkStringView *str1Ptr,
    _Out_  NkStringView *str2Ptr
) {
    NK_ASSERT(strPtr != NULL, NkErr_InParameter);
    NK_ASSERT(ctrlChs != NULL, NkErr_InParameter);
    NK_ASSERT(str1Ptr != NULL, NkErr_OutParameter);
    NK_ASSERT(str2Ptr != NULL, NkErr_OutParameter);

    /* Determine delimiter and string end position. */
    char   *modPtr   = (char *)strPtr;
    NkSize  strSz    = strlen(strPtr);
    char   *delimPos = strpbrk(strPtr, ctrlChs);
    char   *endPtr   = strchr(strPtr, '\0');
    endPtr           = NK_CLAMP(endPtr, modPtr, modPtr + strSz);
    delimPos         = delimPos == NULL ? endPtr : delimPos;

    /* Calculate ranges. */
    *str1Ptr = (NkStringView){ modPtr, (NkSize)(NK_MAX(delimPos, modPtr) - modPtr) };
    *str2Ptr = (NkStringView){ NK_MIN(delimPos + 1, endPtr), (NkSize)(endPtr - NK_MIN(delimPos + 1, endPtr)) };
}


NkSize NK_CALL NkArrayGetDynCount(_In_to_null_ NkVoid const **ptrArray) {
    if (ptrArray == NULL)
        return 0;

    NkSize elemCount = 0;
    while (ptrArray[elemCount] != NULL && ++elemCount);

    return elemCount;
}


NkPoint2D NK_CALL NkCalculateViewportOrigin(
    _In_ NkViewportAlignment vpAlign,
    _In_ NkSize2D vpExtents,
    _In_ NkSize2D tileSize,
    _In_ NkSize2D clExtents
) {
    /* Calculate size of viewport in pixel space. */
    NkSize2D const vpExtPx = { vpExtents.m_width * tileSize.m_width, vpExtents.m_height * tileSize.m_height };

    /* Calculate coordinates of upper-left corner of viewport, in client space. */
    return (NkPoint2D){
        vpAlign & NkVpAlign_Left ? 0 : (clExtents.m_width  - vpExtPx.m_width)  / (vpAlign & NkVpAlign_Right  ? 1 : 2),
        vpAlign & NkVpAlign_Top  ? 0 : (clExtents.m_height - vpExtPx.m_height) / (vpAlign & NkVpAlign_Bottom ? 1 : 2)
    };
};


#undef NK_NAMESPACE


