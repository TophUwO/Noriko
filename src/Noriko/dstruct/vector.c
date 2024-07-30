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
 * \file  vector.c
 * \brief implementation of Noriko's dynamic array (vector) data-structure
 */
#define NK_NAMESPACE "nk::dstruct"


/* stdlib includes */
#include <memory.h>
#include <math.h>

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/util.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/sort.h>

#include <include/Noriko/dstruct/vector.h>


/**
 * \struct NkVector
 * \brief  internal definition of the vector data-structure 
 */
struct NkVector {
    NkSize             m_elemCount;                    /**< current number of elements stored */
    NkSize             m_elemCap;                      /**< current maximum element Cap */
    NkVectorProperties m_vecProps;                     /**< vector properties */
    NkVoid             (NK_CALL *mp_fnDest)(NkVoid *); /**< custom element destructor */

    NkVoid **mp_dataPtr; /**< raw pointer to the internal array */
};


/** \cond INTERNAL */
/**
 * \brief  default predicate for the NkVectorFind method
 * \param  [in] elemPtr pointer to the current element
 * \param  [in] extraParam extra parameter to be passed to the predicate callback
 * \param  [in] currIndex index of the current item in the internal array
 * \return NK_TRUE if the predicate is satisfied by *elemPtr*, NK_FALSE if not
 */
NK_INTERNAL NK_INLINE NkBoolean __NkInt_VectorDefaultPred(
    _In_     NkVoid const *elemPtr,
    _In_opt_ NkVoid *extraParam,
    _In_     NkSize currIndex
) {
    NK_UNREFERENCED_PARAMETER(extraParam);
    NK_UNREFERENCED_PARAMETER(currIndex);

    return elemPtr == extraParam;
}

/**
 * \brief  runs validation on the passed vector properties
 * \param  [in] vecPropsPtr pointer to the NkVectorProperties structure that is to be
 *              validated
 * \return NK_TRUE if the given vector properties are valid, NK_FALSE if not
 * \note   This function only validates the properties in **non-deploy** builds.
 */
NK_INTERNAL NK_INLINE NkBoolean __NkInt_VectorValidateProperties(_In_ NkVectorProperties const *vecPropsPtr) {
/** \cond */
/**
 * \def   NK_VEC_P(p)
 * \brief shorten vector parameter access code for better legibility
 * \param p param name, without \c m_ prefix
 */
#define NK_VEC_P(p) (vecPropsPtr->m_##p)
/** \endcond */

    /* Validate each property according to its specifications. */
    NK_ASSERT(NK_VEC_P(structSize) > 0, NkErr_InParameter);
    NK_ASSERT(
        NK_VEC_P(initialCap) >= NK_VEC_P(minCap) && NK_VEC_P(initialCap) <= NK_VEC_P(maxCap),
        NkErr_InParameter
    );
    NK_ASSERT(NK_VEC_P(minCap) > 0 && NK_VEC_P(minCap) <= NK_VEC_P(maxCap), NkErr_InParameter);
    NK_ASSERT(NK_VEC_P(maxCap) < SIZE_MAX - 1, NkErr_InParameter);
    NK_ASSERT(NK_VEC_P(growFactor) > 1.f, NkErr_InParameter);

    /* All good. */
    return NK_TRUE;
}

/**
 * \brief  attempts to destroy all objects in the ranged denoted by <tt>[sInd, eInd]</tt>
 *
 * This function "fails" when the range is empty, that is, <tt>sInd == eInd</tt>, or if
 * the provided destructor callback is \c NULL.
 * 
 * \param  [in] vecPtr pointer to the NkVector structure of which the internal buffer's
 *              elements are to be destroyed
 * \param  [in] sInd starting index <tt>[0 <= sInd <= eInd]</tt>
 * \param  [in] eInd end index <tt>[sInd <= eInd <= vecPtr->m_elemCount - 1]</tt>
 * \return \c NkErr_Ok if everything went according to plan; if the function did nothing,
 *         \c NkErr_NoOperation is returned
 * \see    NkErrorCode
 */
NK_INTERNAL NkErrorCode __NkInt_VectorTryFreeRange(
    _In_     NkVector const *vecPtr,
    _In_opt_ NkSize sInd,
    _In_opt_ NkSize eInd
) {
    /* If the parameters are somehow invalid, do nothing. */
    if (vecPtr->mp_fnDest == NULL || sInd == eInd)
        return NkErr_NoOperation;

    /* Destroy elements in the given range. */
    for (NkSize i = sInd; i < eInd; i++)
        (*vecPtr->mp_fnDest)(vecPtr->mp_dataPtr[i]);

    return NkErr_Ok;
}

/**
 * \brief  resizes the internal buffer
 * \param  [in,out] vecPtr pointer to the NkVector data-structure of which the internal
 *                  buffer is to be reallocated
 * \param  [in] newCap new capacity of the internal buffer
 * \return see return codes of NkReallocateMemory
 * \see    NkReallocateMemory
 */
NK_INTERNAL NkErrorCode __NkInt_VectorResizeBuffer(_Inout_ NkVector *vecPtr, _In_ NkSize newCap) {
    return NkReallocateMemory(NK_MAKE_ALLOCATION_CONTEXT(), newCap * sizeof(NkVoid *), (NkVoid *)&vecPtr->mp_dataPtr);
}

/**
 * \brief  shifts the internal buffer's contents at a given index by \c positions to make
 *         space for elements or to erase an existing element
 * \param  [in,out] vecPtr pointer to the NkVector data-structure of which the internal
 *                  buffer is to be modified
 * \param  [in] offset offset of where to move the elements from
 * \param  [in] dist shift distance in elements 
 * \param  [in] isLeft direction of the shift operation (\c true for left direction (to
 *              lower index) or \c false for right direction (to higher index)
 * \return \c NkErr_Ok on success, \c NkErr_UnsignedWrapAround if the operation could not
 *         be carried out due to an unsigned wrap-around having occurred (i.e., shifting
 *         distance exceeded array boundaries)
 */
NK_INTERNAL NkErrorCode __NkInt_VectorShiftBuffer(
    _Inout_ NkVector *vecPtr,
    _In_    NkSize offset,
    _In_    NkSize dist,
    _In_    NkBoolean isLeft
) {
    NkSize const destIndex = offset + (isLeft ? -1 : 1) * dist;
    if (isLeft && destIndex > offset || !isLeft && offset > destIndex)
        return NkErr_UnsignedWrapAround;

    memmove(
        (NkVoid *)&vecPtr->mp_dataPtr[destIndex],
        (NkVoid const *)&vecPtr->mp_dataPtr[offset],
        (vecPtr->m_elemCount - offset - 1) * sizeof(NkVoid *)
    );
    return NkErr_Ok;
}

/**
 * \brief  copy array of elements into internal buffer
 * \param  [in,out] vecPtr pointer to the NkVector data-structure of which the internal
 *                  buffer is to be modified
 * \param  [in] index index of where the elements are to be copied to
 * \param  [in] elemArray array with elements that are to be stored in the buffer
 * \param  [in] nElems number of elements stored in \c elemArray
 * \note   This function is most often used in conjunction with the internal function
 *         __NkInt_VectorShiftBuffer
 * \see    __NkInt_VectorShiftBuffer
 */
NK_INTERNAL NkVoid __NkInt_VectorCopyBuffer(
    _In_ NkVector *vecPtr,
    _In_ NkSize index,
    _In_ NkVoid const **elemArray,
    _In_ NkSize nElems
) {
    memcpy((NkVoid *)&vecPtr->mp_dataPtr[index], (NkVoid const *)elemArray, nElems);
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkVectorCreate(
    _In_       NkVectorProperties const *vecPropsPtr,
    _In_opt_   NkVoid (NK_CALL *fnElemDest)(NkVoid *),
    _Init_ptr_ NkVector **vecPtr
) {
    NK_ASSERT(vecPropsPtr != NULL, NkErr_InParameter);
    NK_ASSERT(vecPtr != NULL, NkErr_OutptrParameter);
    NK_ASSERT(__NkInt_VectorValidateProperties(vecPropsPtr), NkErr_InParameter);

    /* Allocate memory for parent structure. */
    NkErrorCode errorCode = NkErr_Ok;
    if ((errorCode = NkAllocateMemory(NK_MAKE_ALLOCATION_CONTEXT(), sizeof **vecPtr, 0, NK_FALSE, vecPtr)) != NkErr_Ok)
        return errorCode;
    /* Allocate memory for internal buffer. */
    NkSize const initCap = sizeof *vecPtr * vecPropsPtr->m_initialCap;
    errorCode = NkAllocateMemory(NK_MAKE_ALLOCATION_CONTEXT(), initCap, NK_FALSE, 0, (NkVoid **)&(*vecPtr)->mp_dataPtr);
    if (errorCode != NkErr_Ok) {
        NkFreeMemory(*vecPtr);

        *vecPtr = NULL;
        return errorCode;
    }

    /* Copy properties. */
    (*vecPtr)->m_vecProps  = *vecPropsPtr;
    (*vecPtr)->mp_fnDest   = fnElemDest;
    (*vecPtr)->m_elemCount = 0;
    (*vecPtr)->m_elemCap   = vecPropsPtr->m_initialCap;

    /* All good. */
    return NkErr_Ok;
}

NkVoid NK_CALL NkVectorDestroy(_Uninit_ptr_ NkVector **vecPtr) {
    NK_ASSERT(vecPtr != NULL && *vecPtr != NULL, NkErr_InptrParameter);

    /* Destroy elements if possible. */
    __NkInt_VectorTryFreeRange(*vecPtr, NK_VECTOR_BEGIN(*vecPtr), NK_VECTOR_END(*vecPtr) - 1);

    /* Free parent structure and buffer memory. */
    NkFreeMemory((*vecPtr)->mp_dataPtr);
    NkFreeMemory(*vecPtr);
    *vecPtr = NULL;
}

_Return_ok_ NkErrorCode NK_CALL NkVectorClear(_Inout_ NkVector *vecPtr) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);

    /* Destroy elements if possible. */
    __NkInt_VectorTryFreeRange(vecPtr, NK_VECTOR_BEGIN(vecPtr), NK_VECTOR_END(vecPtr));

    /* Update properties and resize buffer to the specified smallest size. */
    vecPtr->m_elemCount    = 0;
    vecPtr->m_elemCap = vecPtr->m_vecProps.m_minCap;

    /* Resize buffer. */
    return __NkInt_VectorResizeBuffer(vecPtr, vecPtr->m_vecProps.m_minCap);
}

_Return_ok_ NkErrorCode NK_CALL NkVectorInsert(
    _Inout_  NkVector *vecPtr,
    _In_     NkVoid const *elemPtr,
    _In_opt_ NkSize index
) {
    return NkVectorInsertMulti(vecPtr, index, &(NkVoid const *){ elemPtr }, 1);
}

_Return_ok_ NkErrorCode NK_CALL NkVectorInsertMulti(
    _Inout_           NkVector *vecPtr,
    _In_opt_          NkSize index,
    _I_array_(nElems) NkVoid const **elemArray,
    _In_              NkSize nElems
) {
    NkErrorCode errorCode = NkErr_Ok;
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(elemArray != NULL && *elemArray != NULL, NkErr_InParameter);
    NK_ASSERT(index <= vecPtr->m_elemCount, NkErr_ArrayElemOutOfBounds);

    /*
     * Calculate requested size (new number of elements) and check whether an unsigned
     * wrap-around occurred.
     */
    NkSize const reqSize = vecPtr->m_elemCount + nElems;
    if (reqSize <= vecPtr->m_elemCount)
        return NkErr_UnsignedWrapAround;

    /*
     * Resize internal array if the current size plus *nElems* exceeds the current
     * capacity.
     */
    if (reqSize > vecPtr->m_elemCap) {
        /*
         * Calculate the theoretical capacity that the array would have if it were grown
         * by its 'natural' grow factor.
         */
        NkSize const capAfterGrowing = (NkSize)ceilf(vecPtr->m_elemCap * vecPtr->m_vecProps.m_growFactor);
        /*
         * Calculate possible capacity in elements, taking into account possible hard
         * limits set at initialization.
         */
        NkSize const newCap = NK_MAX(vecPtr->m_vecProps.m_maxCap, NK_MAX(reqSize, capAfterGrowing));
        if (newCap < reqSize)
            return NkErr_CapLimitExceeded;

        /* Finally, resize the buffer and adjust internal state. */
        if ((errorCode = __NkInt_VectorResizeBuffer(vecPtr, newCap)) != NkErr_Ok)
            return errorCode;
        vecPtr->m_elemCap = newCap;
    }
    /* Shift buffer right by the needed number of slots. */
    if ((errorCode = __NkInt_VectorShiftBuffer(vecPtr, index, nElems, NK_FALSE)) != NkErr_Ok)
        return errorCode;

    /* Copy pointers into buffer. */
    __NkInt_VectorCopyBuffer(vecPtr, index, elemArray, nElems);
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkVectorErase(
    _Inout_           NkVector *vecPtr,
    _In_opt_          NkSize index,
    _Init_ptr_mbnull_ NkVoid **elemPtr
) {
    /* If the element was deleted, *elemPtr will be set to NULL automatically. */
    return NkVectorEraseMulti(vecPtr, index, 1, elemPtr);
}

_Return_ok_ NkErrorCode NK_CALL NkVectorEraseMulti(
    _Inout_             NkVector *vecPtr,
    _In_opt_            NkSize sInd,
    _In_opt_            NkSize maxN,
    _O_array_opt_(maxN) NkVoid **elemArray
) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(sInd < vecPtr->m_elemCount, NkErr_ArrayElemOutOfBounds);
    NK_ASSERT(elemArray != NULL, NkErr_OutptrParameter);

    /*
     * Get pointer that is to be erased. If it can be freed, free it, else write it to
     * **elemPtr**.
     */
    NkSize const lenToDel = NK_MIN(maxN, vecPtr->m_elemCount - sInd);
    if (lenToDel > 0) {
        if (__NkInt_VectorTryFreeRange(vecPtr, sInd, sInd + lenToDel) == NkErr_NoOperation) {
            /*
             * Copy range into output buffer for the caller to (possibly and hopefully)
             * free them.
             */
            memcpy(*elemArray, (NkVoid *)&vecPtr->mp_dataPtr[sInd], lenToDel * sizeof(NkVoid *));
            return NkErr_Ok;
        }

        /*
         * Shift buffer by one to the left to remove the element from the buffer. This
         * function can also not fail because sInd + lenToDel > lenToDel is always true.
         */
        __NkInt_VectorShiftBuffer(vecPtr, sInd + lenToDel, lenToDel, NK_TRUE);
        vecPtr->m_elemCount -= lenToDel;
        /*
         * Set the first element of the result buffer to NULL to adhere to the
         * constraints posed on the return value by NkVectorErase().
         */
        *elemArray = NULL;
    }  else return NkErr_NoOperation;

    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkVectorEraseIf(
    _Inout_         NkVector *vecPtr,
    _In_opt_        NkSize sInd,
    _In_opt_        NkSize maxN,
    _O_array_(maxN) NkVoid **elemArray,
    _In_            NkBoolean (NK_CALL *fnPred)(NkVoid const *, NkVoid *, NkSize),
    _Inout_opt_     NkVoid *extraParam
) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(fnPred != NULL, NkErr_CallbackParameter);

    /*
     * Go through range and erase all elements for which the provided predicate returns
     * **true**.
     */
    NkSize const realCount = NK_MIN(maxN, vecPtr->m_elemCount - sInd);
    for (NkSize i = sInd, j = 0; i < sInd + realCount;) {
        NkVoid *elemPtr = vecPtr->mp_dataPtr[i];

        if ((*fnPred)(vecPtr, extraParam, i) == NK_TRUE) {
            if (vecPtr->mp_fnDest != NULL)
                (*vecPtr->mp_fnDest)(elemPtr);
            else
                elemArray[j] = elemPtr;

            __NkInt_VectorShiftBuffer(vecPtr, i, 1, NK_TRUE);
            ++j;
        } else ++i;

        vecPtr->m_elemCount -= j;
    }

    return NkErr_Ok;
}

NkVoid *NK_CALL NkVectorFind(
    _In_     NkVector const *vecPtr,
    _In_     NkVoid const *elemPtr,
    _In_opt_ NkSize sInd,
    _In_opt_ NkSize eInd,
    _In_     NkBoolean isLeftToRight
) {
    return NkVectorFindIf(
        vecPtr,
        sInd,
        eInd,
        isLeftToRight,
        &__NkInt_VectorDefaultPred,
        (NkVoid *)elemPtr
    );
}

NkVoid *NK_CALL NkVectorFindIf(
    _In_        NkVector const *vecPtr,
    _In_opt_    NkSize sInd,
    _In_opt_    NkSize eInd,
    _In_        NkBoolean isLeftToRight,
    _In_        NkBoolean (NK_CALL *fnPred)(NkVoid const *, NkVoid *, NkSize),
    _Inout_opt_ NkVoid *extraParam
) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(fnPred != NULL, NkErr_CallbackParameter);

    /* Calculate starting and end index. */
    NkSize const startIndex = isLeftToRight ? sInd : eInd;
    NkSize const endIndex   = isLeftToRight ? eInd : sInd;

    for (NkSize i = startIndex; isLeftToRight ? i < endIndex : i > endIndex;) {
        NkVoid const *elemPtr = vecPtr->mp_dataPtr[i];

        /*
         * Run predicate on *elemPtr* and return it if it satisfies the predicate's
         * condition.
         */
        if ((*fnPred)(elemPtr, extraParam, i) == NK_TRUE)
            return (NkVoid *)elemPtr;
    }

    /*
     * If an element could not be found that satisfies the condition modeled by the
     * provided predicate, return NULL.
     */
    return NULL;
}

_Return_ok_ NkErrorCode NK_CALL NkVectorFilter(
    _In_            NkVector const *vecPtr,
    _In_opt_        NkSize sInd,
    _In_opt_        NkSize maxN,
    _O_array_(maxN) NkVoid **subVecArrayPtr,
    _In_            NkBoolean (NK_CALL *fnPred)(NkVoid const *, NkVoid *, NkSize),
    _Inout_opt_     NkVoid *extraParam
) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(sInd < vecPtr->m_elemCount, NkErr_InParameter);
    NK_ASSERT(subVecArrayPtr != NULL, NkErr_OutptrParameter);
    NK_ASSERT(fnPred != NULL, NkErr_CallbackParameter);

    /* Find all elements satisfying the predicate in the given range. */
    NkSize outIndex = 0;
    for (NkSize i = sInd; i < NK_MIN(maxN, vecPtr->m_elemCount); i++) {
        NkVoid const *elemPtr = vecPtr->mp_dataPtr[i];

        if ((*fnPred)(elemPtr, extraParam, i) == NK_TRUE)
            subVecArrayPtr[outIndex++] = (NkVoid *)elemPtr;
    }
    /* Zero the remainder of the buffer to mark the remaining slots as empty. */
    memset(
        (void *)(((char *)subVecArrayPtr) + outIndex * sizeof *subVecArrayPtr),
        0,
        (maxN - outIndex) * sizeof *subVecArrayPtr
    );
    
    return NkErr_Ok;
}

NkVoid NK_CALL NkVectorReverse(_Inout_ NkVector *vecPtr, _In_opt_ NkSize sInd, _In_opt_ NkSize eInd) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(sInd <= eInd, NkErr_InvalidRange);
    NK_ASSERT(eInd < vecPtr->m_elemCount, NkErr_ArrayElemOutOfBounds);

    for (NkSize i = sInd, j = eInd; i ^ j; i++, j--) {
        NkVoid *tmpPtr = vecPtr->mp_dataPtr[i];

        vecPtr->mp_dataPtr[i] = vecPtr->mp_dataPtr[j];
        vecPtr->mp_dataPtr[j] = tmpPtr;
    }
}

NkVoid NK_CALL NkVectorSort(
    _Inout_  NkVector *vecPtr,
    _In_opt_ NkSize sInd,
    _In_opt_ NkSize eInd,
    _In_     NkInt32 (NK_CALL *fnPred)(NkVoid const *, NkVoid const *)
) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(sInd <= eInd, NkErr_InvalidRange);
    NK_ASSERT(eInd < vecPtr->m_elemCount, NkErr_ArrayElemOutOfBounds);
    NK_ASSERT(fnPred != NULL, NkErr_CallbackParameter);

    NK_IGNORE_RETURN_VALUE(NkQuicksortPointers(vecPtr->mp_dataPtr, sInd, eInd, fnPred));
}

NkVoid *NK_CALL NkVectorAt(_In_ NkVector const *vecPtr, _In_ NkSize index) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(index < vecPtr->m_elemCount, NkErr_ArrayElemOutOfBounds);

    return vecPtr->mp_dataPtr[index];
}

_Return_ok_ NkErrorCode NK_CALL NkVectorAtMulti(
    _In_            NkVector const *vecPtr,
    _In_opt_        NkSize sInd,
    _In_opt_        NkSize maxN,
    _O_array_(maxN) NkVoid **subVecArrayPtr
) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(sInd < vecPtr->m_elemCount, NkErr_InParameter);
    NK_ASSERT(subVecArrayPtr != NULL, NkErr_OutptrParameter);

    /* Copy buffer into array. */
    memcpy(
        (void *)subVecArrayPtr,
        &vecPtr->mp_dataPtr[sInd],
        NK_MIN(vecPtr->m_elemCount - sInd, maxN) * sizeof *vecPtr->mp_dataPtr
    );

    /*
     * Indicate that no operation was carried out (i.e., no elements were copied) if that
     * was the case.
     */
    return maxN == 0 ? NkErr_NoOperation : NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkVectorForEach(
    _In_        NkVector const *vecPtr,
    _In_opt_    NkSize sInd,
    _In_opt_    NkSize maxN,
    _In_        NkErrorCode (NK_CALL *fnCallback)(NkVoid *, NkVoid *, NkSize),
    _Inout_opt_ NkVoid *extraParam
) {
    NkErrorCode errorCode = NkErr_Ok;
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(sInd < vecPtr->m_elemCount, NkErr_InvalidRange);
    NK_ASSERT(fnCallback != NULL, NkErr_CallbackParameter);

    NkSize const realCount = NK_MIN(maxN, vecPtr->m_elemCount - sInd);
    for (NkSize i = sInd; i <= sInd + realCount; i++) {
        NkVoid *elemPtr = vecPtr->mp_dataPtr[i];

        /* Run callback on each element; return if the callback returns non-zero. */
        if ((errorCode = (*fnCallback)(elemPtr, extraParam, i)) != NkErr_Ok)
            return errorCode;
    }

    /* Everything went well. (no callback returned non-zero) */
    return NkErr_Ok;
}

NkVoid **NK_CALL NkVectorGetBuffer(_In_ NkVector const *vecPtr, _In_opt_ NkSize sInd) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);
    NK_ASSERT(sInd < vecPtr->m_elemCap, NkErr_ArrayOutOfBounds);

    return &vecPtr->mp_dataPtr[sInd];
}

NkSize NK_CALL NkVectorGetElementCount(_In_ NkVector const *vecPtr) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);

    return vecPtr->m_elemCount;
}

NkSize NK_CALL NkVectorGetCapacity(_In_ NkVector const *vecPtr) {
    NK_ASSERT(vecPtr != NULL, NkErr_InParameter);

    return vecPtr->m_elemCap;
}


NkVectorProperties const *NK_CALL NkVectorDefaultProperties(NkVoid) {
    /** \cond */
    /**
     * \brief static data-structure holding default vector properties
     * 
     * The pointer to this instance can be directly passed into NkVectorCreate().
     */
    /** \endcond */
    NK_INTERNAL NkVectorProperties const gl_DefaultVectorProperties = {
        .m_structSize = sizeof gl_DefaultVectorProperties,
        .m_initialCap = 16,
        .m_minCap     = 8,
        .m_maxCap     = SIZE_MAX - 1,
        .m_growFactor = 1.5f
    };

    return &gl_DefaultVectorProperties;
}


#undef NK_NAMESPACE


