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
 * \file  vector.h
 * \brief global definitions for Noriko's vector data-structure
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/util.h>
#include <include/Noriko/error.h>

/**
 * \defgroup Static Vector Indices
 * \brief    defines some constant vector indices pointing to the first (NK_VECTOR_BEGIN)
 *           and after the last element (NK_VECTOR_END)
 */
/** @{ */
#define NK_VECTOR_BEGIN(v) ((NkSize)(0))
#define NK_VECTOR_END(v)   ((NkSize)(NkVectorGetElementCount(v)))
/** @} */


/**
 * \struct NkVector
 * \brief  forward-declaration of opaque vector type
 */
typedef struct NkVector NkVector;

/**
 * \struct NkVectorProperties
 * \brief  holds configuration properties for the vector container type
 * \note   A fixed-size array can be simulated by setting, for example,
 *         <tt>m_initialCapacity = m_minCapacity = m_maxCapacity</tt>.
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkVectorProperties {
    NkSize  m_structSize;      /**< [\c x > 0] size of this structure, in bytes */
    NkSize  m_initialCapacity; /**< [\c m_minCapacity <= \c x <= <tt>m_maxCapacity</tt>] initial capacity of vector [def: <tt>16</tt>] */
    NkSize  m_minCapacity;     /**< [\c < \c x <= <tt>m_maxCapacity</tt>] minimum capacity, in elements (cannot shrink below) [def: <tt>8</tt>] */
    NkSize  m_maxCapacity;     /**< [\c m_minCapacity <= \c x < <tt>SIZE_MAX</tt>] maximum capacity, in elements (cannot grow beyond) [def: <tt>SIZE_MAX - 1</tt>] */
    NkFloat m_growFactor;      /**< [\c x > <tt>1.0</tt>] resize factor when growing array [def: <tt>1.5</tt>] */
} NkVectorProperties;


/**
 * \brief   creates and initializes a new vector data-structure
 * \param   [in] vecPropsPtr pointer to a NkVectorProperties structure holding config
 *               data
 * \param   [in] fnElemDest (optional) element destructor function
 * \param   [out] vecPtr pointer to a variable that will receive the pointer to the
 *                newly-created data-structure
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \see     NkVectorDestroy
 * \note    \li Below is an example of a fully-compliant element destructor.
 * \note    \li Use NkVectorDestroy to destroy instances created by this function.
 * \warning The implementation allows adding <tt>NULL</tt>-pointers to the data-struture
 *          as elements and must therefore be handled properly by the element destructor.
 * 
 * \code{.c}
 * // A callback for destroying a vector element could look like the following.
 * NkVoid NK_CALL NkExampleFreeCallback(NkVoid *elemPtr) {
 *     // Handle NULL pointers. The library does not forbid adding NULL-pointers to the
 *     // data-structure as values. Thus, they must be properly handled by all callbacks
 *     // passed to the data-structure.
 *     if (elemPtr == NULL)
 *         return;
 *     // Convert generic pointer to required datatype.
 *     NkString *strPtr = (NkString *)elemPtr;
 * 
 *     // Destroy any nested pointers.
 *     NkFreeMemory(strPtr->mp_strBuffer);
 *     NkFreeMemory(strPtr->mp_tmpBuffer);
 *     NkVectorDestroy(strPtr->mp_chVec);
 * 
 *     // Destroy parent structure itself.
 *     NkFreeMemory(strPtr);
 * }
 * \endcode
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorCreate(
    _In_       NkVectorProperties const *vecPropsPtr,
    _In_opt_   NkVoid (NK_CALL *fnElemDest)(NkVoid *),
    _Init_ptr_ NkVector **vecPtr
);
/**
 * \brief destroys all contained elements if possible and deallocates all memory used by
 *        the data-structure itself
 * \param [in,out] vecPtr pointer to a variable that stores the pointer of the vector
 * \note  \li <tt>*vecPtr</tt> will be set to <tt>NULL</tt>.
 * \note  \li If the \c fnElemDest parameter of the \c NkVectorCreate function call that
 *        created the instance pointed to by \c vecPtr was <tt>NULL</tt>, this function
 *        assumes the vector's elements to be managed by an external component and thus
 *        won't try to destruct them.
 * \todo  Specify destructors to do a no-op if NULL ptr
 */
NK_NATIVE NK_API NkVoid NK_CALL NkVectorDestroy(_Uninit_ptr_ NkVector **vecPtr);
/**
 * \brief  destroys all elements if possible and resets the vector's capacity to the
 *         specified minimum
 * \param  [in,out] vecPtr pointer to the NkVector instance that is to be cleared
 * \return \c NkErr_Ok on success, non-zero if there was an error
 * \note   If the \c fnElemDest parameter of the \c NkVectorCreate function call that
 *         created the instance pointed to by \c vecPtr was <tt>NULL</tt>, this function
 *         assumes the vector's elements to be managed by an external component and thus
 *         won't try to destruct them.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorClear(_Inout_ NkVector *vecPtr);
/**
 * \brief   inserts the given element \c elemPtr at \c index
 * \param   [in,out] vecPtr pointer to the NkVector instance the element is to be
 *                   inserted into
 * \param   [in] elemPtr pointer of the element that is to be inserted
 * \param   [in] index index where the element is to be inserted
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \see     NkVectorInsertMulti
 * \note    As this data-structure only saves pointers to elements, the element pointed
 *          to by \c elemPtr is not copied.
 * \note    To insert multiple elements as a batch, consider using NkVectorInsertMulti
 * \warning \li If \c index is out of bounds, the behavior is undefined.
 * \warning \li It is possible to add <tt>NULL</tt>-pointers to the data-structure.
 *          Callbacks must handle this on their own.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorInsert(
    _Inout_  NkVector *vecPtr,
    _In_     NkVoid const *elemPtr,
    _In_opt_ NkSize index
);
/**
 * \brief   inserts multiple elements into the given vector at \c index
 * \param   [in,out] vecPtr pointer to the NkVector instance the elements are to be
 *                   inserted into
 * \param   [in] index position where the first element is to be inserted
 * \param   [in] elemArray array of elements
 * \param   [in] nElems number of elements to be inserted from \c elemArray
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \see     NkVectorInsert
 * \note    As this data-structure only saves pointers to elements, the elements in
 *          \c elemArray are not copied.
 * \warning \li If \c index is out of bounds, the behavior is undefined.
 * \warning \li If the elements exceed the vector's current capacity, it is reallocated.
 *          If this reallocation fails, no elements are copied and the vector is
 *          unchanged. Similarly, if the number of elements exceeds the maximum capacity,
 *          no elements are copied.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorInsertMulti(
    _Inout_           NkVector *vecPtr,
    _In_opt_          NkSize index,
    _I_array_(nElems) NkVoid const **elemArray,
    _In_              NkSize nElems
);
/**
 * \brief   erases the element at \c index and destroys it if possible
 * \param   [in,out] vecPtr pointer to the NkVector instance where the element is to be
 *                   deleted from
 * \param   [in] index index of the item that is to be deleted
 * \param   [out] elemPtr pointer to receive the element if it was not destroyed
 * \return  \c NkErr_Ok on success, non-zero on error
 * \note    If the element was destroyed, \c elemPtr will be set to <tt>NULL</tt>,
 *          otherwise \c elemPtr will be set to the element at <tt>index</tt>.
 * \warning If \c index is out of bounds, the behavior is undefined.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorErase(
    _Inout_           NkVector *vecPtr,
    _In_opt_          NkSize index,
    _Init_ptr_mbnull_ NkVoid **elemPtr
);
/**
 * \brief   erases multiple elements from the vector, destroying them if possible
 * \param   [in,out] vecPtr pointer to the NkVector instance where the elements are to be
 *                   deleted from
 * \param   [in] sInd index of the first element in the deletion range
 * \param   [in] maxN maximum number of elements to be erased
 * \param   [out] elemArray pointer to an array that will receive the elements that were
 *                erased from the vector if they were not destroyed
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \note    If the elements were destroyed, \c elemArray will not be changed, otherwise
 *          it will receive up to \c maxN elements that were erased from the vector.
 * \warning If <tt>[sInd, sInd + maxN]</tt> is out of bounds, the behavior is undefined.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorEraseMulti(
    _Inout_             NkVector *vecPtr,
    _In_opt_            NkSize sInd,
    _In_opt_            NkSize maxN,
    _O_array_opt_(maxN) NkVoid **elemArray
);
/**
 * \brief   iterates over the entire or a part of the vector and erases all elements that
 *          satisfy the provided predicate
 * \param   [in,out] vecPtr pointer to the NkVector instance where the elements are to be
 *                   deleted from
 * \param   [in] sInd index of the first element in the deletion range
 * \param   [in] maxN maximum number of elements to be erased
 * \param   [out] elemArray pointer to an array that will receive the elements that were
 *                erased from the vector if they were not destroyed
 * \param   [in] fnPred predicate to use for determining whether the element is to be
 *               erased
 * \param   [in,out] extraParam extra parameter to be passed into \c fnPred()
 * \return  \c NkErr_Ok on success, \c NkErr_NoOperation if no elements were erased, or
 *           non-zero if there was an error
 * \see     NkVectorFindIf
 * \note    For an example of a compliant predicate, see <tt>NkVectorFindIf</tt>'s
 *          \c fnPred parameter.
 * \warning \li If <tt>[sInd, eInd]</tt> is out of bounds, the behavior is undefined.
 * \warning \li If \c fnPred() is invalid (not a function or diverting signature), the
 *          behavior is undefined.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorEraseIf(
    _Inout_         NkVector *vecPtr,
    _In_opt_        NkSize sInd,
    _In_opt_        NkSize maxN,
    _O_array_(maxN) NkVoid **elemArray,
    _In_            NkBoolean (NK_CALL *fnPred)(NkVoid const *, NkVoid *, NkSize),
    _Inout_opt_     NkVoid *extraParam
);
/**
 * \brief   finds the first occurrence of the pointer \c elemPtr in \c vecPtr
 * \param   [in] vecPtr pointer to the NkVector instance that is to be searched
 * \param   [in] elemPtr element that is to be searched for
 * \param   [in] sInd sInd first index of the searching range (seen from the left)
 * \param   [in] eInd last index of the searching range (seen from the left)
 * \param   [in] isLeftToRight whether to iterate from left-to-right (\c sInd being the
 *               first index searched) or right-to-left (\c eInd being the first index
 *               searched)
 * \return  \c elemPtr if it is contained in <tt>[sInd, eInd]</tt>, or \c NULL if it
 *          could not be found
 * \warning If <tt>[sInd, eInd]</tt> is out of bounds, the behavior is undefined.
 */
NK_NATIVE NK_API NkVoid *NK_CALL NkVectorFind(
    _In_     NkVector const *vecPtr,
    _In_     NkVoid const *elemPtr,
    _In_opt_ NkSize sInd,
    _In_opt_ NkSize eInd,
    _In_     NkBoolean isLeftToRight
);
/**
 * \brief   searches the given range for an element that satisfies the provided predicate
 * \param   [in] vecPtr pointer to the NkVector instance that is to be searched
 * \param   [in] sInd first index of the searching range (seen from the left)
 * \param   [in] eInd last index of the searching range (seen from the left)
 * \param   [in] isLeftToRight whether to iterate from left-to-right (\c sInd being the
 *               first index searched) or right-to-left (\c eInd being the first index
 *               searched)
 * \param   [in] fnPred predicate to use for searching the element range
 * \param   [in,out] extraParam extra parameter to be passed into \c fnPred()
 * \return  pointer to the first element that satisfies the given predicate, or \c NULL
 *          if no element in the provided interval satisfies the predicate
 * \see     NkVectorFind
 * \warning \li If <tt>[sInd, eInd]</tt> is out of bounds, the behavior is undefined.
 * \warning \li If \c fnPred() is invalid (not a function or diverting signature), the
 *          behavior is undefined.
 */
NK_NATIVE NK_API NkVoid *NK_CALL NkVectorFindIf(
    _In_        NkVector const *vecPtr,
    _In_opt_    NkSize sInd,
    _In_opt_    NkSize eInd,
    _In_        NkBoolean isLeftToRight,
    _In_        NkBoolean (NK_CALL *fnPred)(NkVoid const *, NkVoid *, NkSize),
    _Inout_opt_ NkVoid *extraParam
);
/**
 * \brief   filters the vector for elementss satisfying the provided predicate
 * \param   [in] vecPtr pointer to the NkVector instance of which the elements are to be
 *               filtered
 * \param   [in] sInd first index of the filter range
 * \param   [in] maxN maximum number of elements to filter
 * \param   [out] subVecArrayPtr pointer to an array that will receive the results of the
 *                filter operation, at least \c maxN elements in capacity
 * \param   [in] fnPred criterion to filter elements for 
 * \param   [in,out] extraParam extra parameter to be passed into \c fnPred()
 * \return  \c NkErr_Ok if everything went according to plan, \c NkErr_NoOperation if no
 *          elements were filtered, or non-zero if there was an error
 * \warning \li If \c fnPred is not a valid predicate (not a function/not a function with
 *          the proper signature), the behavior is undefined.
 * \warning \li If <tt>[sInd, sInd + maxN]</tt> is out of bounds, the behavior is
 *          undefined.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorFilter(
    _In_            NkVector const *vecPtr,
    _In_opt_        NkSize sInd,
    _In_opt_        NkSize maxN,
    _O_array_(maxN) NkVoid **subVecArrayPtr,
    _In_            NkBoolean (NK_CALL *fnPred)(NkVoid const *, NkVoid *, NkSize),
    _Inout_opt_     NkVoid *extraParam
);
/**
 * \brief   reverses the order of elements in the given vector
 * \param   [in,out] vecPtr pointer to the NkVector instance of which the order of
 *                   elements is to be reversed
 * \param   [in] sInd first index of the interval that is to be reversed
 * \param   [in] eInd last index of the interval that is to be reversed
 * \warning If <tt>[sInd, eInd]</tt> is out of bounds, the behavior is undefined.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkVectorReverse(_Inout_ NkVector *vecPtr, _In_opt_ NkSize sInd, _In_opt_ NkSize eInd);
/**
 * \brief   sorts the vector's elements with respect to the given predicate
 * \param   [in,out] vecPtr pointer to the NkVector instance that is to be sorted
 * \param   [in] sInd first index of the sorting interval
 * \param   [in] eInd last index of the sorting interval
 * \param   [in] fnPred pointer to the predicate function
 * \note    **0.0.1-1**: The current implementation of this function uses the
 *          \c QuickSort sorting algorithm.
 * 
 * \warning If <tt>[sInd, eInd]</tt> is out of bounds, the behavior is undefined.
 * \warning If \c fnPred is not a valid predicate (not a function/not a function with the
 *          proper signature), the behavior is undefined.
 *
 * \code{.c}
 * // data-structure held by the vector
 * struct data {
 *     void *x;
 *     int   y;
 * }
 * 
 * // A candidate for the predicate callback is as follows:
 * NkInt32 NK_CALL NkExampleSortPredicate(NkVoid const *lPtr, NkVoid const *rPtr) {
 *     // Do the usual checking pointers first. I want NULL-pointers to be shifted to the
 *     // end of the array because they are annoying and I don't like them. Refer to the
 *     // explanation on the return values slightly below.
 *     if (lPtr == NULL || rPtr == NULL) {
 *         if (lPtr == NULL && rPtr != NULL) return 1;
 *         if (lPtr != NULL && rPtr == NULL) return -1;
 *         if (lPtr == NULL && rPtr == NULL) return 0;
 *     }
 * 
 *     // Cast generic pointers to the respective type.
 *     struct data const *left  = (struct data const *)lPtr;
 *     struct data const *right = (struct data const *)rPtr;
 * 
 *     // Sort items in ascending order with respect to data.y.
 *     // Return > 0 if left > right.
 *     // Return == 0 if left == right.
 *     // Return < 0 if left < right.
 *     return left->y > right->y ? 1 : (left->y < right->y ? -1 : 0);
 * }
 * \endcode
 */
NK_NATIVE NK_API NkVoid NK_CALL NkVectorSort(
    _Inout_  NkVector *vecPtr,
    _In_opt_ NkSize sInd,
    _In_opt_ NkSize eInd,
    _In_     NkInt32 (NK_CALL *fnPred)(NkVoid const *, NkVoid const *)
);
/**
 * \brief   retrieves the element at index \c index
 * \param   [in] vecPtr pointer to the NkVector structure
 * \param   [in] index index of the requested element
 * \return  element at index \c index
 * \warning If \c index is out of bounds, the behavior is undefined.
 */
NK_NATIVE NK_API NK_INLINE NkVoid *NK_CALL NkVectorAt(_In_ NkVector const *vecPtr, _In_ NkSize index);
/**
 * \brief   copies a sub-array from the passed NkVector instance into the provided buffer
 * \param   [in] vecPtr pointer to the NkVector instance
 * \param   [in] sInd starting index of the sub-array
 * \param   [in] maxN maximum number of elements to copy
 * \param   [out] subVecArrayPtr pointer to an array that will receive the pointers to
 *                the copied elements, at least \c maxN in size (in elements)
 * \return  \c NkErr_Ok on success, \c NkErr_NoOperation if no elements were copied but
 *          no real error occurred, or non-zero in case of an error
 * \note    The function stops copying once \c maxN items have been copied or the end of
 *          the array has been reached (that is, the last element has been copied),
 *          whichever happens first.
 * \warning If \c sInd is out of bounds, the behavior is undefined. However, if
 *          <tt>[sInd, sInd + maxN]</tt> is out of bounds, the function stops after the
 *          last element, not going out of bounds.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorAtMulti(
    _In_            NkVector const *vecPtr,
    _In_opt_        NkSize sInd,
    _In_opt_        NkSize maxN,
    _O_array_(maxN) NkVoid **subVecArrayPtr
);
/**
 * \brief   iterates through the array in the given direction, invoking the given
 *          callback on each item individually
 * \param   [in] vecPtr pointer to the NkVector instance that is to be traversed
 * \param   [in] sInd starting index of the iteration procedure
 * \param   [in] maxN maximum number of elements to iterate over
 * \param   [in] fnCallback() pointer to the function that is to be called on each
 *               element; see below for an example of a fully-compliant candidate
 * \param   [in,out] extraParam extra parameter to be passed to \c fnCallback()
 * \return  \c NkErr_Ok, or value of \c fnCallback() if the callback returned non-zero at
 *          any point throughout the iteration procedure
 * \note    The function stops iterating once \c maxN items have been iterated over or
 *          the end of the array has been reached (that is, the last element has been
 *          iterated over), whichever happens first.
 * \warning If \c fnCallback() is not valid (not a function/not a function with the
 *          proper signature), the behavior is undefined.
 * 
 * \code{.c}
 * // data-structure held by the vector
 * struct data {
 *     int32_t  a;
 *     int32_t  b;
 *     char    *c;
 * }
 * 
 * // data-structure passed into the 'NkVectorForEach()' function as 'extraParam'
 * struct context {
 *     int32_t n;
 *     int32_t k;
 * }
 * 
 * // A candidate for a callback function suitable for this function is the following:
 * NkErrorCode NK_CALL NkExampleForeachCallback(NkVoid *elemPtr, NkVoid *extraPtr, NkSize index) {
 *     // The user could push NULL-pointers to the vector, so handle them appropriately.
 *     if (elemPtr == NULL)
 *         return NkErr_Ok;
 *
 *     // Do some stuff with the element, like printing, for example.
 *     struct data const *ptr = (struct data const *)elemPtr;
 *     printf("v[%zu] = { %i, %i, \"%s\" }\n", index, ptr->a, ptr->b, ptr->c);
 * 
 *     // Update context with whatever you want.
 *     if (ptr->a == ptr->b) {
 *         struct context *cxt = (struct context *)extraPtr;
 *         
 *         // Does not make a whole lot of sense, but whatever ...
 *         cxt->n = index;
 *         cxt->k = n + index << 2;
 *     } else if (!strcmp(ptr->c, u8"end")) return NkErr_ManuallyAborted;
 * 
 *     return NkErr_Ok;
 * }
 * \endcode
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkVectorForEach(
    _In_        NkVector const *vecPtr,
    _In_opt_    NkSize sInd,
    _In_opt_    NkSize maxN,
    _In_        NkErrorCode (NK_CALL *fnCallback)(NkVoid *, NkVoid *, NkSize),
    _Inout_opt_ NkVoid *extraParam
);
/**
 * \brief   retrieves the pointer to a part of the vector's internal buffer
 * \param   [in] vecPtr pointer to an NkVector data-structure of which a pointer to the
 *               internal buffer is to be retrieved
 * \param   [in] sInd offset of where the sub-buffer starts, in elements
 * \return  pointer to the internal buffer shifted by \c sInd
 * \warning If \c sInd is out of bounds, the behavior is undefined.
 */
NK_NATIVE NK_API NK_INLINE NkVoid **NK_CALL NkVectorGetBuffer(_In_ NkVector const *vecPtr, _In_opt_ NkSize sInd);
/**
 * \brief   retrieves the current number of elements currently stored in the vector
 * \param   [in] vecPtr pointer to a NkVector structure of which the element count is to
 *               be retrieved
 * \return  current number of elements stored, or \c SIZE_MAX if there was an error
 * \warning If \c vecPtr is \c NULL, the behavior is undefined.
 */
NK_NATIVE NK_API NK_INLINE NkSize NK_CALL NkVectorGetElementCount(_In_ NkVector const *vecPtr);
/**
 * \brief   retrieves the current container capacity; that is, the maximum number of
 *          elements that can be stored before being forced to grow the container
 * \param   [in] vecPtr pointer to a NkVector structure of which the container capacity
 *               is to be retrieved
 * \return  current container capacity, or \c SIZE_MAX if there was an error
 * \warning If \c vecPtr is \c NULL, the behavior is undefined.
 */
NK_NATIVE NK_API NK_INLINE NkSize NK_CALL NkVectorGetCapacity(_In_ NkVector const *vecPtr);

/**
 * \brief  retrieves the default properties for the vector data-structure
 * \return pointer to a data-structure that holds the default configuration values for
 *         dynamic arrays (vectors)
 * \note   The pointer returned is a pointer to static and constant memory.
 */
NK_NATIVE NK_API NK_INLINE NkVectorProperties const *NkVectorDefaultProperties(NkVoid);


