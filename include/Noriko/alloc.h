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
 * \file  alloc.h
 * \brief Noriko's general purpose and specialized memory allocators
 * 
 * This file exposes the allocation functions used by Noriko to allocate memory in a more
 * efficient way, depending on the purpose of the memory. For example, it provides global
 * allocation functions for book-keeping and general-purpose memory and specialized pool
 * allocators for fixed object sizes requiring frequent allocation and deallocation.
 */


#pragma once

/* stdlib includes */
#include <stdint.h>

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>
#include <include/Noriko/util.h>

/* Only use allocation contexts in debug builds. */
#if (!defined NK_CONFIG_DEPLOY)
    /**
     * \def   NK_MAKE_ALLOCATION_CONTEXT(ns, p, s, a)
     * \brief constructs a new static allocation context for use with the general-purpose
     *        allocator
     * 
     * This structure carries contextual data automatically-evaluated 
     * 
     * \note  *ns* and *p* must be static string literals.
     */
    #define NK_MAKE_ALLOCATION_CONTEXT()                      \
        &(NkAllocationContext){                               \
            sizeof(NkAllocationContext),                      \
            &(NkStringView)NK_MAKE_STRING_VIEW(__FILE__),     \
            &(NkStringView)NK_MAKE_STRING_VIEW(NK_NAMESPACE), \
            &(NkStringView)NK_MAKE_STRING_VIEW(__func__),     \
            __LINE__,                                         \
            0                                                 \
        }
#else
    #define NK_MAKE_ALLOCATION_CONTEXT() (NULL)
#endif


/**
 * \brief represents an allocation context
 * 
 * The allocation context is used by the debugging tools to monitor allocation states. It
 * is usually provided by the requesting function and may be statically allocated. When
 * this structure is passed to a function, it is never written to. If the structure
 * passed is shared across threads, the behavior is undefined.
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkAllocationContext {
    size_t        m_structSize;        /**< size of this structure, in bytes */
    NkStringView *mp_filePath;         /**< path of the file */
    NkStringView *mp_namespaceIdent;   /**< namespace the function belongs to */
    NkStringView *mp_functionName;     /**< name of the function */
    uint32_t      m_lineInFile;        /**< line the allocation originated from, relative to file */
    uint32_t      m_timestampInMillis; /**< timestamp of when the allocation was requested */
} NkAllocationContext;

/**
 * \brief represents the debug state of an allocator
 * 
 * The debugging tools make extensive use of the data found in this structure. They are
 * used for visualization, profiling, and logging.
 * 
 * \note  The values in this struct show only a momentary state and will have to be
 *        requeried for up-to-date information.
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkAllocatorState {
    size_t        m_structSize;        /**< size of this structure, in bytes */
    NkStringView *mp_allocatorName;    /**< debug name for this allocator */
    NkSize        m_currMemUsage;      /**< current memory usage, in bytes */
    size_t        m_minAllocBytes;     /**< minimum allocation size so far */
    size_t        m_maxAllocBytes;     /**< maximum allocation size so far */
    size_t        m_nBytesAllocated;   /**< number of bytes ever allocated */
    size_t        m_nBytesFreed;       /**< number of bytes ever freed */
} NkAllocatorState;


/**
 * \brief  allocates a new block of memory from the heap
 * 
 * This general-purpose memory allocator allocates directly from the heap provided by the
 * host platform. Use this only for book-keeping memory or other memory that is only very
 * infrequently (re-)allocated. For frequent allocations, use the pool-allocator.
 * 
 * \param  [in] allocCxt allocation context for debugging
 * \param  [in] sizeInBytes size of the memory block, in bytes
 * \param  [in] alignInBytes alignment requirement of the memory block, in bytes
 * \param  [out] memPtr address of a pointer that will receive the starting address of
 *         the new memory block
 * \return *NkErr_Ok* on success, non-zero on failure
 * \note   If the function fails, no memory is allocated and *memPtr* will be set to
 *         point to NULL.
 * \note   *alignInBytes* must be a power of two.
 * \note   Aligned allocation is currently not supported.
 * \todo   Implement *aligned allocation*.
 * \see    NkAllocationContext
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkAllocateMemory(
    _In_opt_         NkAllocationContext const *const allocCxt,
    _In_ _Pre_valid_ size_t sizeInBytes,
    _In_opt_         size_t alignInBytes,
    _Init_ptr_       NkVoid **memPtr
);
/**
 * \brief  reallocate a previously-allocated dynamic block of memory
 * \param  [in] allocCxt allocation context for debugging
 * \param  [in] newSizeInBytes new size of the block, in bytes
 * \param  [in,out] memPtr pointer to the block of memory that is to be reallocated;
 *         the contents of the block are moved to the new location and the old block is 
 *         then freed
 * \return *NkErr_Ok* on success, non-zero on failure
 * \note   If the function fails, the memory is not moved and thus remains valid.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkReallocateMemory(
    _In_opt_         NkAllocationContext const *const allocCxt,
    _In_ _Pre_valid_ size_t newSizeInBytes,
    _Reinit_ptr_     NkVoid **memPtr
);
/**
 * \brief  frees dynamically-allocated memory
 * \param  [out] memPtr pointer to the memory address that is to be freed
 */
NK_NATIVE NK_API NkVoid NK_CALL NkFreeMemory(NkVoid *memPtr);

/**
 * \brief   retrieves the current state for the requested allocator
 * \param   [in] debugName debug identifier for the allocator
 * \param   [out] bufferPtr pointer to the buffer that will receive the momentary memory
 *          allocator state
 * \return  *NkErr_Ok* on success, non-zero on failure
 * \note    If *bufferPtr* is *NULL* or *bufferPtr* is improperly initialized, the
 *          function fails.
 * \note    If the function fails and *bufferPtr* is not NULL, *bufferPtr* is initialized
 *          with all zeroes.
 * \warning Before you run this function, initialize the *m_structSize* member variable
 *          of *bufferPtr* to the size of the buffer that is being passed to the
 *          function. To do this, use **sizeof(NkAllocatorState)**.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkGetAllocatorState(
    _In_ _Pre_valid_                 NkStringView const *const debugName,
    _Out_ _Pre_notnull_ _Post_valid_ NkAllocatorState *const bufferPtr
);


