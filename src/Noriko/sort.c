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
 * \file  sort.c
 * \brief implements generic sorting algorithms usable with a variety of datatypes
 *
 * What sorting algorithms are used depends on the sorting function being used. Functions
 * declared in this file never diverge from the algorithm used in the function's initial
 * implementation.
 */
#define NK_NAMESPACE u8"nk::sort"


/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/sort.h>
#include <include/Noriko/platform.h>


/**
 * \internal
 * \brief  implements the splitting algorithm for \c QuickSort
 * \param  [in,out] ptrArray
 * \param  [in] left first index in the sorting range (relative to left-most element)
 * \param  [in] right last index in the sorting range (relative to left-most element)
 * \param  [in] fnPred predicate callback used for sorting
 * \return index of the pivot element, <tt>[left <= x <= right]</tt>
 * \endinternal
 */
NK_INTERNAL NkSize NK_CALL __NkInternal_QuicksortSplit(
    _Inout_ NkVoid **ptrArray,
    _In_    NkSize left,
    _In_    NkSize right,
    _In_    NkInt32(NK_CALL *fnPred)(NkVoid const *, NkVoid const *)
) {
    NkSize i      = left;
    NkSize j      = right - 1;
    NkVoid *pivot = ptrArray[right];

    while (i < j) {
        /* Search for an element for which e > pivot is true, starting from the left. */
        while (i < j && (*fnPred)(ptrArray[i], pivot) <= 0)
            ++i;
        /*
         * Search for an element for which e <= pivot is true, starting from the
         * right.
         */
        while (j > i && (*fnPred)(ptrArray[j], pivot) > 0)
            --j;

        if ((*fnPred)(ptrArray[i], ptrArray[j]) > 0) {
            NkVoid *tmpPtr = ptrArray[i];

            ptrArray[i] = ptrArray[j];
            ptrArray[j] = tmpPtr;
        }
    }

    /* Swap current pivot element with new pivot element. */
    if ((*fnPred)(ptrArray[i], pivot) > 0) {
        NkVoid *tmpPtr = ptrArray[i];

        ptrArray[i]     = ptrArray[right];
        ptrArray[right] = tmpPtr;
    } else i = right;

    return i;
}


_Return_ok_ NkErrorCode NK_CALL NkQuicksortPointers(
    _Inout_  NkVoid **ptrArray,
    _In_opt_ NkSize sInd,
    _In_opt_ NkSize eInd,
    _In_     NkInt32(NK_CALL *fnPred)(NkVoid const *, NkVoid const *)
) {
    NK_ASSERT(ptrArray != NULL, NkErr_InParameter);
    NK_ASSERT(fnPred != NULL, NkErr_CallbackParameter);

    /* If the range is too empty or just one element, do nothing. */
    if (eInd - sInd < 2)
        return NkErr_NoOperation;

    /* Sort the array. */
    if (sInd < eInd) {
        NkSize split = __NkInternal_QuicksortSplit(ptrArray, sInd, eInd, fnPred);

        NK_IGNORE_RETURN_VALUE(NkQuicksortPointers(ptrArray, sInd, split - 1, fnPred));
        NK_IGNORE_RETURN_VALUE(NkQuicksortPointers(ptrArray, split + 1, eInd, fnPred));
    } else return NkErr_InvalidRange;

    return NkErr_Ok;
}


