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
 * \file  htable.h
 * \brief defines the public API for Noriko's internal hash table container type
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>
#include <include/Noriko/util.h>


/**
 * \brief callback definition for destroying keys and/or elements
 * \param [in,out] keyPtr pointer to the key
 * \param [in,out] valPtr pointer to the value
 */
typedef NkVoid (NK_CALL *NkHashtableFreeFn)(_Inout_opt_ union NkHashtableKey *keyPtr, _Inout_opt_ NkVoid *valPtr);
/**
 * \brief  callback definition for when the implementation iterates over a range of
 *         elements
 * \param  [in,out] pairPtr pointer to the current key-value pair
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   If the function returns non-zero, the implementation will stop iterating and
 *         propagate the return code of this callback to the caller.
 */
typedef NkErrorCode (NK_CALL *NkHashtableIterFn)(_Inout_ struct NkHashtablePair *pairPtr);

/**
 * \enum  NkHashtableKeyType
 * \brief represents the type IDs for the key type that is to be considered valid when
 *        adding and matching keys
 */
NK_NATIVE typedef _In_range_(0, __NkHtKeyTy_Count__ - 1) enum NkHashtableKeyType {
    NkHtKeyTy_Int64,      /**< type ID for 64-bit signed integer key */
    NkHtKeyTy_Uint64,     /**< type ID for 64-bit unsigned integer key */
    NkHtKeyTy_String,     /**< type ID for string key */
    NkHtKeyTy_StringView, /**< type ID for string view key */
    NkHtKeyTy_Pointer,    /**< type ID for generic pointer key */
    NkHtKeyTy_Uuid,       /**< type ID for UUID key */

    __NkHtKeyTy_Count__   /**< *only used internally* */
} NkHashtableKeyType;

/**
 * \struct NkHashtable
 * \brief  represents the type for a generic hash table implementation
 * 
 * This implementation works with pointers by default and implements Robin-Hood-Hashing.
 * Keys can be of a variety of primitive types.
 * The hash table does not by default take ownership of the contained elements. To enable
 * this feature, the user must provide a suitable \c free() function.
 */
NK_NATIVE typedef struct NkHashtable NkHashtable;

/**
 * \union   NkHashtableKey
 * \brief   contains all valid primitive and complex key values
 * \warning Only one key value can be valid at a time. What key value will be treated
 *          as valid is set when the hash table is created. Changing the valid key
 *          type during operation as well as submitting invalid keys is undefined
 *          behavior.
 */
NK_NATIVE typedef union NkHashtableKey {
    NkInt64       m_int64Key;  /**< signed 64-bit integer key */
    NkUint64      m_uint64Key; /**< unsigned 64-bit integer key */
    char         *mp_strKey;   /**< string value key */
    NkStringView *mp_svKey;    /**< string view key */
    NkVoid       *mp_ptrKey;   /**< pointer value key */
    NkUuid       *mp_uuidKey;  /**< UUID key type */
} NkHashtableKey;

/**
 * \struct NkHashtablePair
 * \brief  represents a key-value pair that is to be saved in the hash table
 */
NK_NATIVE typedef struct NkHashtablePair {
    NkHashtableKey  m_keyVal;    /**< key value */
    NkVoid         *mp_valuePtr; /**< pointer to the value */
} NkHashtablePair;

/**
 * \struct NkHashtableProperties
 * \brief  represents the configuration properties for the hash table
 * \note   If \c mp_fnElemFree is <tt>NULL</tt>, the hash table will assume that its
 *         elements are externally managed and thus not take ownership. \c mp_fnElemFree
 *         also supports destroying the key alongside the value; however, the function
 *         is not required to destroy either, so it can be used to only destroy the value
 *         and leave the key unchanged (e.g., when the keys are primitive types and thus
 *         do not require cleanup), but the values are complex (i.e., struct types with
 *         fields that do require manual cleanup).
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkHashtableProperties {
    NkUint32           m_structSize;  /**< size of this struct, in bytes */
    NkUint32           m_initCap;     /**< initial capacity of hash table, in elements */
    NkUint32           m_minCap;      /**< minimum capacity of hash table, in elements */
    NkUint32           m_maxCap;      /**< maximum capacity of hash table, in elements */
    NkHashtableKeyType m_keyType;     /**< type ID of key */
    NkHashtableFreeFn  mp_fnElemFree; /**< element free function callback (may be NULL) */
} NkHashtableProperties;


/**
 * \brief  creates a new hash table
 * \param  [in] htPropsPtr pointer to the data-structure that contains the configuration
 *              options for the new hash table
 * \param  [out] htPtr pointer to a variable that will receive the pointer to the
 *               newly-created hash table instance
 * \return \c NkErr_Ok on success, non-zero on failure
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkHashtableCreate(
    _In_       NkHashtableProperties const *htPropsPtr,
    _Init_ptr_ NkHashtable **htPtr
);
/**
 * \brief destroys a hash table instance, optionally destroying all contained objects
 * \param [in,out] htPtr pointer to a variable that contains the hash table instance that
 *                 is to be destroyed
 * \note  If the \c mp_fnElemFree member of the \c NkHashtableProperties structure that
 *        was supplied when the hash table was created is <tt>NULL</tt>, no elements are
 *        destroyed.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkHashtableDestroy(_Uninit_ptr_ NkHashtable **htPtr);
/**
 * \brief clears the hash table, optionally destroying all contained objects
 * \param [in,out] htPtr pointer to the hash table that is to be cleared
 * \note  \li If the \c mp_fnElemFree member of the \c NkHashtableProperties structure
 *        that was supplied when the hash table was created is <tt>NULL</tt>, no elements
 *        are destroyed.
 * \note  \li If possible, the element array is shrunk to the minimum specified capacity.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkHashtableClear(_Inout_ NkHashtable *htPtr);
/**
 * \brief  inserts an element into the hash table
 * \param  [in,out] htPtr pointer to the hash table instance
 * \param  [in] htPairPtr pointer to the element in a <key, val> structure
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   The implementation may necessitate resizing and rebuilding the array, causing
 *         this operation to take longer than usual. Therefore, it is not advised to call
 *         this function excessively in a tight loop.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkHashtableInsert(
    _Inout_ NkHashtable *htPtr,
    _In_    NkHashtablePair const *htPairPtr
);
/**
 * \brief  inserts multiple elements into the hash table
 * \param  [in,out] htPtr pointer to the hash table
 * \param  [in] htPairArray pointer to the element array
 * \param  [in] nElems number of elements in \c htPairArray
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li The implementation may necessitate resizing and rebuilding the array,
 *         causing this operation to take longer than usual. Therefore, it is not advised
 *         to call this function excessively in a tight loop.
 * \note   \li If the hash table does not fit and cannot be resized to fit all elements,
 *         the function fails and no elements are inserted. The hash table state is no
 *         changed.
 * \note   \li Duplicates inside the pair array are not added to the hash table. The hash
 *         table is still enlarged by the required amount as if no duplicates were
 *         submitted if such a reallocation is determined to be necessary.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkHashtableInsertMulti(
    _Inout_           NkHashtable *htPtr,
    _I_array_(nElems) NkHashtablePair const **htPairArray,
    _In_              NkUint32 nElems
);
/**
 * \brief  erases an element from the hash table
 * \param  [in,out] htPtr pointer to the hash table instance the element is to be erased
 *                  from
 * \param  [in] keyPtr pointer to the key of the element is to be searched
 * \return \c NkErr_Ok on success, or non-zero if there was an error
 * \note   The key in the hash table needs to be equal to the key at <tt>keyPtr</tt>. It
 *         is not necessary for \c keyPtr and the key candidate in the hash table to
 *         point to the same memory.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkHashtableErase(
    _Inout_ NkHashtable *htPtr,
    _In_    NkHashtableKey const *keyPtr
);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkHashtableAt(
    _In_     NkHashtable const *htPtr,
    _In_     NkHashtableKey const *keyPtr,
    _Outptr_ NkVoid **valPtr
);
/**
 * \brief  extracts (i.e., erases without deleting) the given element from the hash table
 * \param  [in,out] htPtr pointer to the hash table instance the element is to be erased
 *                  from
 * \param  [in] keyPtr pointer to the key of the element is to be searched
 * \param  [out] valuePtr pointer to a variable that will receive the pointer to the 
 *               value corresponding to \c keyPtr
 * \return \c NkErr_Ok on success, non-zero on failure.
 * \note   If the function fails, the contents of \c valuePtr are indeterminate.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkHashtableExtract(
    _Inout_  NkHashtable *htPtr,
    _In_     NkHashtableKey const *keyPtr,
    _Outptr_ NkVoid **valuePtr
);

/**
 * \brief  checks whether the hash table contains the given key
 * \param  [in] htPtr pointer to the hash table instance
 * \param  [in] keyPtr pointer to the key that is to be located
 * \return non-zero if the key could be found, zero if it could not be found
 */
NK_NATIVE NK_API NkBoolean NK_CALL NkHashtableContains(
    _In_ NkHashtable const *htPtr,
    _In_ NkHashtableKey const *keyPtr
);
/**
 * \brief  iterates over the hash table in linear (i.e., array) order
 * \param  [in] htPtr pointer to the hash table instance that is to be iterated over
 * \param  [in] fnIter iteration callback invoked once for each occupied slot
 * \return \c NkErr_Ok on success, non-zero if an error occurred or the user terminated
 *         the run by returning non-zero from \c fnIter
 * \note   If \c fnIter returns non-zero, the iteration is cancelled and the returned
 *         value is propagated to the caller.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkHashtableForEach(
    _In_ NkHashtable const *htPtr,
    _In_ NkHashtableIterFn fnIter
);
/**
 * \brief  retrieves the current number of elements in the hash table
 * \param  [in] htPtr pointer to the hash table instance of which the element count is to
 *              be retrieved
 * \return number of elements in the hash table
 */
NK_NATIVE NK_API NkUint32 NK_CALL NkHashtableCount(_In_ NkHashtable const *htPtr);


