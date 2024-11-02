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
 * \file    env.c
 * \brief   implements the the public API for Noriko's command-line parser
 *
 * Noriko generally runs by invoking Noriko's runtime component with certain config
 * parameters such as input file, and other runtime options. The command line arguments
 * need to be parsed once at application startup and can then be accessed throughout the
 * lifetime of the application. Synchronization is not necessary since the command-line
 * arguments are constant.
 *
 * \warning Due to the nature of the command-line arguments storage, it should not be
 *          accessed in functions that can be called after \c main() has returned such as
 *          functions registered in <tt>atexit()</tt>.
 */
#define NK_NAMESPACE "nk::env"


/* Noriko includes */
#include <include/Noriko/env.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/log.h>
#include <include/Noriko/noriko.h>

#include <include/Noriko/dstruct/vector.h>
#include <include/Noriko/dstruct/htable.h>


/** \cond INTERNAL */
/**
 * \brief global hash table that holds the key-value store to the 
 */
NK_INTERNAL NkHashtable *gl_EnvStore = NULL;


/**
 * \brief frees the key and value memory
 * \param [in] keyPtr pointer to the hashtable key
 * \param [in] varPtr pointer to the value
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_EnvFreeElem(_Inout_ NkHashtableKey *keyPtr, _Inout_ NkVariant *varPtr) {
    /* Destroy the key. It's always a pool-allocated pointer to a string view. */
    NkPoolFree(keyPtr->mp_svKey);
    NkPoolFree(varPtr);
}

/**
 * \brief  checks if the current character is equal to one of the expected characters and
 *         advances the character indicator to the next character
 * \param  [in] paramStr original raw command-line option string
 * \param  [in] expPtr collection of possible characters to accept
 * \param  [in,out] chPtr pointer to the character pointer that is to be checked
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   The \c expPtr parameter is to be formatted as a raw collection of characters,
 *         <tt>NUL</tt>-terminated, like "0123456789ABCDEFabcdef" to accept all possible
 *         hex digits.
 */
NK_INTERNAL NK_INLINE _Return_ok_ NkErrorCode __NkInt_EnvExpectChars(
    _In_z_               char const *paramStr,
    _In_z_               char const *expPtr,
    _Pre_valid_ _Outptr_ char **chPtr
) {
    if (strpbrk(*chPtr, expPtr) != *chPtr) {
        NK_LOG_ERROR(
            "Unexpected character while parsing command-line parameter \"%s\": '%c' (0x%X); "
            "expected one of the following: %s.",
            paramStr,
            **chPtr,
            (NkInt32)**chPtr,
            expPtr
        );

        return NkErr_UnexpectedCharacter;
    }

    ++*chPtr;
    return NkErr_Ok;
}

/**
 * \brief   validates the given string as a decimal (integer or floating-point) number
 *          and converts it if possible
 * \param   [in] rawVal pointer to an NkStringView instance that holds the portion that
 *               is to be validated and converted
 * \param   [out] resPtr pointer to the NkVariant instance that is to hold the converted
 *                number value
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \note    \li This function only considers the portion defined by the <tt>rawVal</tt>
 *          parameter.
 * \note    \li If the function fails, \c resPtr is not modified.
 * \warning The behavior is undefined if \c rawVal does not constitute a valid string
 *          view into a previously-defined string literal or mutable string.
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_EnvParseNumber(
    _In_  NkStringView const *rawVal,
    _Out_ NkVariant *resPtr
) {
    /* Parse the pre-decimal places. */
    NkSize charsToGo = rawVal->m_sizeInBytes;
    char const *decPtr = rawVal->mp_dataPtr;
    /* Skip sign. */
    (NkVoid)((*decPtr == '+' || *decPtr == '-') && --charsToGo && ++decPtr);
    while (isdigit(*decPtr) && --charsToGo && ++decPtr);
    
    /* If we got a decimal comma, parse the post-decimal places. */
    (NkVoid)(*decPtr == '.' && --charsToGo && ++decPtr);
    while (isdigit(*decPtr) && --charsToGo && ++decPtr);
    if (charsToGo > 0 && !isdigit(*decPtr) && *decPtr ^ '\0') {
        /* Found invalid character. */
        NK_LOG_ERROR(
            "Unexpected non-decimal character '%c' in value string \"%.*s\".",
            *decPtr,
            (NkUint32)rawVal->m_sizeInBytes,
            rawVal->mp_dataPtr
        );

        return NkErr_UnexpectedCharacter;
    }

    /* Prepare the value. */
    NkVariantSet(resPtr, NkVarTy_Double, (NkDouble)strtod(rawVal->mp_dataPtr, NULL));
    return NkErr_Ok;
}

/**
 * \brief  parses a raw command-line option string as a key-value pair, separating the
 *         identifier from the value, optionally converting the value, resulting in a
 *         form that can be used for further processing
 * \param  [in] optStr raw command-line option string (<tt>NUL</tt>-terminated)
 * \param  [in] expectPrefix whether or not to skip prefixes, for example when parsing
 *              command-line options
 * \param  [out] keyPtr pointer to a variable that will receive the pointer to the
 *               newly-allocated and initialized key
 * \param  [out] valPtr pointer to a variable that will receive the pointer to the
 *               newly-allocated and initialized value (can be unmodified if there is no
 *               value)
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li The \c valPtr parameter is not modified when there is no value to be
 *         parsed.
 * \note   \li Upon error, both \c keyPtr and \c valPtr are initialized to <tt>NULL</tt>.
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_EnvParsePair(
    _In_z_           char const *optStr,
    _In_             NkBoolean expectPrefix,
    _Init_ptr_       NkStringView **keyPtr,
    _Init_ptr_maybe_ NkVariant **valPtr
) {
    /*
     * Skip the param identifier prefix if necessary. It can be one of the following
     * characters: $ / - #
     * For environment variables, for example, a prefix is not supplied.
     */
    char const *startPtr = optStr;
    NkErrorCode errorCode;
    if (expectPrefix == NK_TRUE) {
        errorCode = __NkInt_EnvExpectChars(optStr, "$/-#", &startPtr);

        if (errorCode != NkErr_Ok)
            return errorCode;
    }

    /* Get the identifier and value strings. */
    NkVariant tmpResult;
    NkStringView paramNameStr, paramValStr, trimmedNameStr, trimmedValStr, tmpSvVal;
    /* First, split them by searching for the first delimiter. */
    NkRawStringSplit(startPtr, "=:", &paramNameStr, &paramValStr);
    /*
     * Trim all the spaces and other unwanted characters so that we are left with the raw
     * but cleaned-up identifier- and value section strings.
     */
    NkRawStringTrim(paramNameStr.mp_dataPtr, paramNameStr.m_sizeInBytes, " ", &trimmedNameStr);
    NkRawStringTrim(paramValStr.mp_dataPtr, paramValStr.m_sizeInBytes, " ", &trimmedValStr);

    /* Parse the value if needed. */
    if (trimmedValStr.m_sizeInBytes == 0)
        goto lbl_MAKERESULTS;
    switch (*trimmedValStr.mp_dataPtr) {
        case '0': case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9': case '+': case '-':
            /* Parse as decimal number. */
            if ((errorCode = __NkInt_EnvParseNumber(&trimmedValStr, &tmpResult)) != NkErr_Ok)
                return errorCode;

            break;
        case '\"':
            /*
             * Parse as escaped string literal if and only if the first and the last
             * character in the trimmed range are equal to quotation marks. Otherwise,
             * parse as a normal string.
             */
            if (*trimmedValStr.mp_dataPtr == trimmedValStr.mp_dataPtr[trimmedValStr.m_sizeInBytes - 1]) {
                /* Trim away the quotation marks. */
                tmpSvVal = (NkStringView){
                    .mp_dataPtr = trimmedValStr.mp_dataPtr + 1,
                    .m_sizeInBytes = trimmedValStr.m_sizeInBytes - 2
                };

                NkVariantSet(&tmpResult, NkVarTy_StringView, &tmpSvVal);
                break;
            }

            /* Parse as a string instead. */
            NkVariantSet(&tmpResult, NkVarTy_StringView, &trimmedValStr);
            break;
        default: {
            /* Parse as identifier non-escaped string or bool. */
            NK_INTERNAL struct { NkStringView m_strVal; NkBoolean m_strMapping; } const gl_BoolMappings[] = {
                { NK_MAKE_STRING_VIEW("true"),  NK_TRUE  }, { NK_MAKE_STRING_VIEW("yes"), NK_TRUE  },
                { NK_MAKE_STRING_VIEW("false"), NK_FALSE }, { NK_MAKE_STRING_VIEW("no"),  NK_FALSE },
                { NK_MAKE_STRING_VIEW("on"),    NK_TRUE  }, { NK_MAKE_STRING_VIEW("off"), NK_FALSE }
            };
            /*
             * Try parsing it as a boolean value first. If the value does not correspond
             * to a boolean constant, parse it as a string instead.
             */
            for (NkUint32 i = 0; i < NK_ARRAYSIZE(gl_BoolMappings); i++)
                if (NkStringViewCompare(&gl_BoolMappings[i].m_strVal, &trimmedValStr) == 0) {
                    NkVariantSet(&tmpResult, NkVarTy_Boolean, gl_BoolMappings[i].m_strMapping);

                    break;
                }

            /* Parse as a string instead. */
            NkVariantSet(&tmpResult, NkVarTy_StringView, &trimmedValStr);
        }
    }

lbl_MAKERESULTS:
    /* Allocate the memory using the pool allocator. */
    errorCode = NkPoolAlloc(NK_MAKE_ALLOCATION_CONTEXT(), sizeof trimmedNameStr, 1, keyPtr);
    if (errorCode != NkErr_Ok)
        return errorCode;
    if (trimmedValStr.m_sizeInBytes != 0) {
        errorCode = NkPoolAlloc(NK_MAKE_ALLOCATION_CONTEXT(), sizeof tmpResult, 1, valPtr);
        if (errorCode != NkErr_Ok) {
            NkPoolFree(*keyPtr);

            *(NkVoid **)keyPtr = *(NkVoid **)valPtr = NULL;
            return errorCode;
        }

        /* Copy value. */
        NkVariantCopy(&tmpResult, *valPtr);
    } else *valPtr = NULL;

    /* Copy key. */
    NkStringViewCopy(&trimmedNameStr, *keyPtr);
    return NkErr_Ok;
}

/**
 * \brief parses the given array as an array of stringified key-value option pair and
 *        adds the interpretations to the global key-value store
 * \param [in] optCount number of elements in \c optArray
 * \param [in] optArray array of C-strings (<tt>NUL</tt>-terminated) which encode key-val
 *             pairs
 * \param [in] hasPrefix whether or not the keys are prefixed; environment variables are
 *             generally not prefixed while command-line options must be prefixed
 * \note  If an error occurs while parsing a specific command-line option string, then
 *        that key-value pair is not added, whereas the parsing process of the input
 *        array is never aborted entirely.
 */
NK_INTERNAL NkVoid __NkInt_EnvParseOptionArray(
    _In_                     NkUint32 optCount,
    _In_reads_opt_(optCount) char **optArray,
    _In_                     NkBoolean hasPrefix
) {
    if (optArray == NULL)
        return;

    for (NkUint32 i = 0; i < optCount; i++) {
        NkStringView *keyPtr  = NULL;
        NkVariant    *varPtr  = NULL;
        NkErrorCode   errCode = NkErr_Ok;
        
        /* Parse the pair. */
        errCode = __NkInt_EnvParsePair(optArray[i], hasPrefix, &keyPtr, &varPtr);
        if (errCode != NkErr_Ok)
            continue;
        /*
         * Add it to the hash table. Check if such a key already exists. If yes, ignore
         * the current key and value.
         */
        NkBoolean doesExist = NkHashtableContains(gl_EnvStore, &(NkHashtableKey const){ .mp_svKey = keyPtr });
        if (doesExist) {
            /* Option already defined. */
            NK_LOG_WARNING(
                "Option [%s] \"%.*s\" (ind: %i) is already defined; ignoring redefinition.",
                hasPrefix ? "CMD" : "ENV",
                (NkUint32)keyPtr->m_sizeInBytes,
                keyPtr->mp_dataPtr,
                i
            );

            /* Delete key and value since we are not going to add it again. */
            goto lbl_DELPAIR;
        }

        /* Add to hash table. */
        errCode = NkHashtableInsert(
            gl_EnvStore,
            &(NkHashtablePair){
                .m_keyVal    = (NkHashtableKey){ .mp_svKey = keyPtr },
                .mp_valuePtr = varPtr
            }
        );
        if (errCode == NkErr_Ok)
            continue;

lbl_DELPAIR:
        NkPoolFree(keyPtr);
        NkPoolFree(varPtr);
    }
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkEnvStartup(NkVoid) {
    NK_ASSERT(gl_EnvStore == NULL, NkErr_ComponentState);
    NK_LOG_INFO("startup: command-line");

    /* Query the required information from the application specification. */
    NkApplicationSpecification const *appSpecs = NkApplicationQuerySpecification();

    /* Initialize the key-value store. */
    NkErrorCode errCode = NkErr_Ok;
    NkHashtableProperties const htProps = {
        .m_structSize  = sizeof htProps,
        .m_keyType     = NkHtKeyTy_StringView,
        .m_initCap     = 16,
        .m_minCap      = 16,
        .m_maxCap      = UINT32_MAX,
        .mp_fnElemFree = (NkHashtableFreeFn)&__NkInt_EnvFreeElem
    };
    errCode = NkHashtableCreate(&htProps, &gl_EnvStore);
    if (errCode != NkErr_Ok)
        return errCode;

    /*
     * First, if possible, parse all environment variables. That's to make sure that the
     * user cannot add command-line parameters with the same identifier as a predefined
     * environment variable.
     */
    __NkInt_EnvParseOptionArray(
        (NkUint32)NkArrayGetDynCount((NkVoid const **)appSpecs->mp_envp),
        appSpecs->mp_envp,
        NK_FALSE
    );
    /*
     * Parse all command-line arguments. Offset the argv array by one to skip the first
     * parameter which is pretty much always the path to the executable file.
     */
    __NkInt_EnvParseOptionArray(appSpecs->m_argc - 1, &appSpecs->mp_argv[1], NK_TRUE);
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkEnvShutdown(NkVoid) {
    NK_LOG_INFO("shutdown: command-line");

    NkHashtableDestroy(&gl_EnvStore);
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkEnvGetValue(_In_z_ char const *keyStr, _Out_ NkVariant *valPtr) {
    NK_ASSERT(keyStr != NULL, NkErr_InParameter);
    NK_ASSERT(valPtr != NULL, NkErr_OutParameter);

    /* Get the value associated with the parameter/environment variable. */
    NkVariant *varPtr;
    NkStringView keyAsSv;
    NkErrorCode const errCode = NkHashtableAt(
        (NkHashtable const *)gl_EnvStore,
        &(NkHashtableKey const) {
            .mp_svKey = NkStringViewSet(keyStr, &keyAsSv)
        },
        &varPtr
    );
    if (errCode != NkErr_Ok)
        return errCode;

    /* Get copy of value. */
    NkVariantCopy(varPtr, valPtr);
    return NkErr_Ok;
}


#undef NK_NAMESPACE


