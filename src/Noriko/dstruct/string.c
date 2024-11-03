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
 * \file    string.c
 * \brief   implements Noriko's built-in mutable run-time string type
 * 
 * Noriko uses UTF-8 as its internal encoding across all platforms, regardless of native
 * platform encoding. This means that on platforms which use a different native encoding
 * than UTF-8, like Windows, strings have to be converted to the native platform encoding
 * on the fly, for which reason they should be cached as much as possible.<br>
 * 
 * The string type defined by this file supports major basic Unicode string operations, 
 * such as <em>case-folding</em>, and general string operations that are UTF-8-aware.
 * 
 * \note    Strings are always <tt>NUL</tt>-terminated.
 * \warning Strings in Noriko are generally <em>unsafe</em> for performance reasons. This
 *          means that the library will never validate strings passed to it. The using
 *          module is responsible for ensuring validity of strings coming from external
 *          sources such as files, networks, or process-shared memory.
 * 
 * \par Remarks
 *   Advanced unicode operations such as <em>normalization</em>, or locale support are
 *   currently not implemented. Such functionality may be added later.
 */
#define NK_NAMESPACE "nk::dstruct"


/* stdlib includes */
#include <string.h>

/* Noriko includes */
#include <include/Noriko/alloc.h>

#include <include/Noriko/dstruct/string.h>


/** \cond INTERNAL */
/**
 * \struct __NkInt_String
 * \brief  represents the internal implementation of the public \c NkString type
 */
NK_NATIVE typedef struct __NkInt_String {
    NkUint32  m_currSize; /**< current size in bytes of the string buffer */
    NkUint32  m_currLen;  /**< current length of string, in bytes (incl. <tt>NUL</tt>) */

    char     *mp_charBuf; /**< raw string buffer (always <tt>NUL</tt>-terminated) */
} __NkInt_String;
NK_VERIFY_TYPE(NkString, __NkInt_String);


/**
 */
NK_INTERNAL NK_INLINE NkUint8 __NkInt_String_CharSize(_In_z_ _Utf8_ char const *encChar) {
    NK_ASSERT(encChar != NULL, NkErr_InParameter);

    /**
     */
    NK_INTERNAL NkUint8 const gl_c_Utf8CharSizes[0x100] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,  4, 4, 4, 4, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
    };

    return gl_c_Utf8CharSizes[*encChar];
}

/**
 */
NK_INTERNAL NkUint32 __NkInt_String_DecodeChar(_In_z_ _Utf8_ char const *encChar) {
    NK_ASSERT(encChar != NULL, NkErr_InParameter);

    /*
     * Decode the character, that is, transform it into its corresponding Unicode
     * codepoint.
     */
    switch (__NkInt_String_CharSize(encChar)) {
        case 1: return *encChar;
        case 2: return ((*encChar & 31) << 6  | (*++encChar & 63));
        case 3: return ((*encChar & 15) << 12 | (*++encChar & 63) << 6  | (*++encChar & 63));
        case 4: return ((*encChar & 7)  << 18 | (*++encChar & 63) << 12 | (*++encChar & 63) << 6 | (*++encChar & 63));
    }

    /* Codepoint is not of valid size; return codepoint that signifies an error. */
    return 0xFFFD; /* unicode replacement character (U+FFFD) */
}

/**
 */
NK_INTERNAL char const *__NkInt_String_Goto(_In_z_ _Utf8_ char const *u8Str, _In_ NkUint32 off) {
    NK_ASSERT(u8Str != NULL, NkErr_InParameter);

    while (*u8Str ^ 0 && off > 0) {
        /* Skip the first character. */
        u8Str += __NkInt_String_CharSize(u8Str);

        --off;
    }

    return u8Str;
}

/**
 */
NK_INTERNAL NkStringView __NkInt_String_CalcSubstr(
    _In_z_ _Utf8_ char const *strPtr,
    _In_          NkUint32 start,
    _In_opt_      NkUint32 maxCount
) {
    NK_ASSERT(strPtr != NULL, NkErr_InParameter);
    NK_ASSERT(maxCount > 0, NkErr_InParameter);

    char const *startPtr = __NkInt_String_Goto(strPtr, start);
    char const *endPtr   = __NkInt_String_Goto(startPtr, maxCount - 1);

    return (NkStringView) {
        .mp_dataPtr    = (char *)startPtr,
        .m_sizeInBytes = (NkSize)(endPtr - startPtr)
    };
}

/**
 */
NK_INTERNAL NkUint32 __NkInt_String_Strlen(_In_z_ _Utf8_ char const *strPtr) {
    NK_ASSERT(strPtr != NULL, NkErr_InParameter);

    NkUint32 currLen = 0;
    while (*strPtr ^ '\0') {
        strPtr += __NkInt_String_CharSize(strPtr);

        ++currLen;
    }

    return currLen;
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkStringCreate(
    _In_opt_z_ _Utf8_ char const *fromStr,
    _In_              NkUint32 count,
    _Out_             NkString *resPtr
) {
    NK_ASSERT(count > 0, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    /* Allocate memory for the string. */
    NkSize const bufSize = (fromStr != NULL ? strlen(fromStr) : 1) * count + 1;
    char *charBuf;
    NkErrorCode errCode = NkGPAlloc(NK_MAKE_ALLOCATION_CONTEXT(), bufSize, 0, NK_TRUE, &charBuf);
    if (errCode != NkErr_Ok)
        return errCode;

    /* Initialize the character buffer if necessary. */
    for (NkUint32 i = 0; fromStr != NULL && i < count; i++)
        memcpy(&charBuf[i * count], fromStr, (bufSize - 1) / count);
    charBuf[bufSize] = '\0';

    /* Initialize string. */
    *(__NkInt_String *)resPtr = (__NkInt_String){
        .m_currSize = bufSize,
        .m_currLen  = fromStr != NULL ? bufSize : 0,
        .mp_charBuf = charBuf
    };
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkStringCreateView(
    _In_     NkString const *srcPtr,
    _In_     NkUint32 start,
    _In_opt_ NkUint32 maxCount,
    _Out_    NkStringView *resPtr
) {
    NK_ASSERT(srcPtr != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    *resPtr = __NkInt_String_CalcSubstr(((__NkInt_String *)srcPtr)->mp_charBuf, start, maxCount);
    return NkErr_Ok;
}

NkVoid NK_CALL NkStringDestroy(NkString *strPtr) {
    if (strPtr == NULL)
        return;

    NkGPFree(((__NkInt_String *)strPtr)->mp_charBuf);
}


_Return_ok_ NkErrorCode NK_CALL NkStringJoin(_Inout_ NkString *strPtr, _In_z_ _Utf8_ char const *elemStr) {
    NK_ASSERT(strPtr != NULL, NkErr_InOutParameter);
    NK_ASSERT(elemStr != NULL, NkErr_InParameter);
    if (*elemStr == '\0')
        return NkErr_NoOperation;

    /* Get pointer to internal string type. */
    __NkInt_String *intStr = (__NkInt_String *)strPtr;

    /* Check if the current string + element string exceeds the current buffer. */
    NkUint32 reqBufSize;
    NkUint32 const elemStrlen = (NkUint32)strlen(elemStr);
    if (!NkCheckedUint32Add(intStr->m_currLen, elemStrlen, &reqBufSize))
        return NkErr_UnsignedWrapAround;
    if (reqBufSize > intStr->m_currSize) {
        /* If the current buffer is exceeded, we must resize it. */
        char *newBuf = intStr->mp_charBuf;
        NkErrorCode errCode = NkGPRealloc(NK_MAKE_ALLOCATION_CONTEXT(), (NkUint32)(reqBufSize * 1.5f), &newBuf);

        if (newBuf == NULL)
            return errCode;
        intStr->m_currSize = (NkUint32)(reqBufSize * 1.5f);
    }
    /* Copy the new buffer. */
    memcpy((NkVoid *)&intStr->mp_charBuf[intStr->m_currLen], (NkVoid const *)elemStr, (elemStrlen + 1) * sizeof(char));

    /* All good. */
    intStr->m_currLen += elemStrlen;
    return NkErr_Ok;
}


NkUint32 NK_CALL NkStringGetLength(_In_ NkString const *strPtr) {
    NK_ASSERT(strPtr != NULL, NkErr_InOutParameter);

    return __NkInt_String_Strlen(((__NkInt_String *)strPtr)->mp_charBuf);
}

char const *NK_CALL NkStringAt(_In_ NkString const *strPtr, _In_ NkUint32 off) {
    NK_ASSERT(strPtr != NULL, NkErr_InOutParameter);
    
    char const *currIter = ((__NkInt_String *)strPtr)->mp_charBuf;
    while (off--) {
        currIter += __NkInt_String_CharSize(currIter);

        if (currIter == '\0')
            break;
    }

    return off > 0 ? NULL : currIter;
}


char const *NK_CALL NkStringIterate(_In_z_ _Utf8_ char const *strPtr) {
    NK_ASSERT(strPtr != NULL, NkErr_InParameter);

    return *strPtr == '\0' ? NULL : __NkInt_String_Goto(strPtr, 1);
}


#undef NK_NAMESPACE


