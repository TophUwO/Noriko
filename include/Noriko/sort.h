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
 * \file  sort.h
 * \brief defines generic sorting algorithms usable with a variety of datatypes
 * 
 * What sorting algorithms are used depends on the sorting function being used. Functions
 * declared in this file never diverge from the algorithm used in the function's initial
 * implementation.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/error.h>


/**
 * \brief   sorts <tt>ptrArray[sInd : eInd]</tt> using the \c QuickSort algorithm with
 *          respect to the predicate provided by \c pred
 * \param   [in,out] ptrArray pointer to the array of pointers that is to be sorted
 * \param   [in] sInd first index of the sorting range
 * \param   [in] eInd last index of the sorting range
 * \param   [in] fnPred callback of which the return value is used to sort the elements
 * \return  \c NkErr_Ok on success, \c NkErr_NoOperation if no sorting was done (e.g., if
 *          no elements in the array were swapped AND/OR the sorting range is empty (i.e.,
 *          <tt>|ptrArray[sInd : eInd]| < 2</tt>)), or non-zero on error
 * \see     NkVectorSort
 * \note    For an example of a fully-compliant predicate callback, see the documentation
 *          for \c NkVectorSort()
 * \warning The behavior is undefined if:
 *          \li \c ptrArray is \c NULL or invalid
 *          \li <tt>[sInd, eInd]</tt> is out of bounds or invalid (that is, for example,
 *          \c sInd > <tt>eInd</tt>)
 *          \li \c fnPred() is not a valid function pointer or the function it points to
 *          uses a different signature        
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkQuicksortPointers(
    _Inout_  NkVoid **ptrArray,
    _In_opt_ NkSize sInd,
    _In_opt_ NkSize eInd,
    _In_     NkInt32 (NK_CALL *fnPred)(NkVoid const *, NkVoid const *)
);


