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
    #define NK_MAKE_ALLOCATION_CONTEXT()                     \
        &(NkAllocationContext){                              \
            sizeof(NkAllocationContext),                     \
            (NkStringView)NK_MAKE_STRING_VIEW(__FILE__),     \
            (NkStringView)NK_MAKE_STRING_VIEW(NK_NAMESPACE), \
            (NkStringView)NK_MAKE_STRING_VIEW(__func__),     \
            __LINE__,                                        \
            0                                                \
        }
#else
    #define NK_MAKE_ALLOCATION_CONTEXT() (NULL)
#endif


/**
 * \struct NkAllocationContext
 * \brief  represents an allocation context
 * 
 * The allocation context is used by the debugging tools to monitor allocation states. It
 * is usually provided by the requesting function and may be statically allocated. When
 * this structure is passed to a function, it is never written to. If the structure
 * passed is shared across threads, the behavior is undefined.
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkAllocationContext {
    NkSize       m_structSize;        /**< size of this structure, in bytes */
    NkStringView m_filePath;          /**< path of the file */
    NkStringView m_namespaceIdent;    /**< namespace the function belongs to */
    NkStringView m_functionName;      /**< name of the function */
    NkUint32     m_lineInFile;        /**< line the allocation originated from, relative to file */
    NkUint32     m_timestampInMillis; /**< timestamp of when the allocation was requested */
} NkAllocationContext;


/**
 * \brief  initializes the global memory allocators
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li It is not safe to call any allocation functions if this function fails.
 * \note   \li This function should be run once from the main thread before worker
 *         threads that might access the shared allocators are started.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkAllocInitialize(NkVoid);
/**
 * \brief  uninitializes the global memory allocators
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li It is not safe to call any allocation functions after this function has
 *         returned.
 * \note   \li This function should be called once from the main thread after all worker
 *         threads that may access the shared allocators have been terminated.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkAllocUninitialize(NkVoid);


/**
 * \brief  allocates a new block of memory on the heap
 * 
 * This general-purpose memory allocator allocates directly from the heap provided by the
 * host platform. Use this only for book-keeping memory or other memory that is only very
 * infrequently (re-)allocated. For frequent allocations, use the pool-allocator.
 * 
 * \param  [in] allocCxt allocation context for debugging
 * \param  [in] sizeInBytes size of the memory block, in bytes
 * \param  [in] alignInBytes alignment requirement of the memory block, in bytes
 * \param  [in] isZeroed whether or not the newly-allocated memory should be
 *              pre-initialized with all zeroes
 * \param  [out] memPtr address of a pointer that will receive the starting address of
 *         the new memory block
 * \return *NkErr_Ok* on success, non-zero on failure
 * \note   \li If the function fails, no memory is allocated and *memPtr* will be set to
 *         point to NULL.
 * \note   \li \c alignInBytes must be a power of two.
 * \note   \li Aligned allocation is currently not supported.
 * \todo   Implement *aligned allocation*.
 * \see    NkAllocationContext
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkGPAlloc(
    _In_opt_         NkAllocationContext const *allocCxt,
    _In_ _Pre_valid_ NkSize sizeInBytes,
    _In_opt_         NkSize alignInBytes,
    _In_opt_         NkBoolean isZeroed,
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
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkGPRealloc(
    _In_opt_         NkAllocationContext const *allocCxt,
    _In_ _Pre_valid_ NkSize newSizeInBytes,
    _Reinit_ptr_     NkVoid **memPtr
);
/**
 * \brief  frees dynamically-allocated memory
 * \param  [in] memPtr pointer to the memory address that is to be freed
 * \note   If \c memPtr is <tt>NULL</tt>, the function does nothing.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkGPFree(_In_ NkVoid const *memPtr);

/**
 * \brief   allocates one or more blocks of memory using the pool allocator
 * 
 * Noriko implements a variety of different allocators optimized for different use-cases.
 * For small allocations, especially allocations of object types that are plentiful in
 * the current application state, a pool allocator can be used to improve memory- and
 * cache performance.
 * 
 * \param   [in] allocCxt (optional) allocation context for debugging and such
 * \param   [in] blockSize block size to use for chunking
 * \param   [in] blockCount number of (consecutive) blocks to allocate
 * \param   [out] memPtr pointer to a variable that will receive the pointer to the newly
 *                allocated block
 * \return  \c NkErr_Ok on success, non-zero on failure
 * \see     NkPoolReserve, NkPoolFree
 * \note    \li This function is thread-safe.
 * \note    \li \c blockSize must be non-zero and a multiple of eight.
 * \note    \li To return the memory back to the allocator after you are done with it,
 *          use the \c NkPoolFree() function.
 * \note    \li This function may allocate new memory pools on the heap if necessary.
 *          This may increase latency a bit for large allocations. To mitigate this, use
 *          the \c NkPoolReserve() function.
 * \warning The memory returned by this function is not guaranteed to be zeroed. Do not
 *          access the returned memory before having verified its integrity or before
 *          having reinitialized it yourself.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkPoolAlloc(
    _In_opt_   NkAllocationContext const *allocCxt,
    _In_       NkUint32 blockSize,
    _In_opt_   NkUint32 blockCount,
    _Init_ptr_ NkVoid **memPtr
);
/**
 * \brief  allocates a new memory pool with the given properties
 *
 * The name \c *reserve() may be misleading; it does not \e reserve a pool for use by a
 * specific actor, but rather allocates a new pool of the given block count. This is done
 * even if a free pool with the same properties already exists. The allocated pool can be
 * used by every component that uses the pool allocator.
 * 
 * \param  [in] blockSize size of one block, in bytes
 * \param  [in] blockCount number of block the pool will have
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   This function is thread-safe.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkPoolReserve(_In_ NkUint32 blockSize, _In_ NkUint32 blockCount);
/**
 * \brief frees the allocation at the given address
 * \param [in] memPtr memory block address as returned by \c NkPoolAlloc()
 * \note  \li This function is thread-safe.
 * \note  \li If \c memPtr is <tt>NULL</tt>, the function does nothing.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkPoolFree(_Inout_opt_ NkVoid *memPtr);
/**
 * \brief  determines the size of the block at the given address
 * \param  [in] memPtr memory block address
 * \return size of the memory block, in bytes
 * \note   This function is thread-safe.
 */
NK_NATIVE NK_API NkUint32 NK_CALL NkPoolGetBlockSize(_In_ NkVoid const *memPtr);
/**
 * \brief  determines the size of the entire application
 * \param  [in] memPtr memory block address
 * \return size of the allocation, in bytes
 * \note  \li This function is thread-safe.
 * \note   \li For single block allocations (i.e., \c blockCount=1 when <tt>NkPoolAlloc()</tt>
 *         was called), the allocation size is equal to the block size. For multi-block
 *         allocations, the allocation size is the size of all blocks that belong to the 
 *         same allocation as the block pointed to by the given block address combined.
 */
NK_NATIVE NK_API NkUint32 NK_CALL NkPoolGetAllocSize(_In_ NkVoid const *memPtr);


