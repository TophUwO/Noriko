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
 * \file  alloc.c
 * \brief implementation of Noriko's general purpose and specialized memory allocators
 *
 * This file implements the allocation functions used by Noriko to allocate memory in a
 * more efficient way, depending on the purpose of the memory. For example, it provides
 * global allocation functions for book-keeping and general-purpose memory and various
 * specialized pool-allocators for fixed object sizes requiring frequent allocation and
 * deallocation.
 */
#define NK_NAMESPACE "nk::alloc"


/* stdlib includes */
#include <string.h>
#include <math.h>

/* Noriko includes */
#include <include/Noriko/alloc.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/util.h>
#include <include/Noriko/log.h>


/** \cond INTERNAL */
/**
 * \enum  __NkInt_AllocType
 * \brief type identifier for the block
 */
NK_NATIVE typedef enum __NkInt_AllocType {
    NkIntAllocTy_Single       = (1 << 0), /**< single block allocation */
    NkIntAllocTy_StartOfMulti = (1 << 1), /**< start of a multi-block allocation */
    NkIntAllocTy_PartOfMulti  = (1 << 2)  /**< block that is part of a multi-block allocation */
} __NkInt_AllocType;

/**
 * \struct __NkInt_PoolAllocMemoryPool
 * \brief  represents the metadata saved with each block
 */
NK_NATIVE typedef struct __NkInt_PoolAllocMemoryPool {
    NkUint32  m_blockSize;    /**< size of one block, in bytes */
    NkUint32  m_blockCount;   /**< number of slots */
    NkUint32  m_nAllocBlocks; /**< number of currently allocated blocks */
    NkUint32  m_fFreeInd;     /**< index of the first free block */
    NkVoid   *mp_blockPtr;    /**< pointer to the block memory */
} __NkInt_PoolAllocMemoryPool;

/**
 * \struct __NkInt_PoolAllocBlockHead
 * \brief  represents the header for a single block of memory inside a pool
 */
NK_NATIVE typedef struct __NkInt_PoolAllocBlockHead {
    NkUint32 m_blockFlags;  /**< block flags (allocated, type, ...) */
    NkUint32 m_nMultiBlock; /**< number of blocks if this block is the start of a multi-block allocation */
} __NkInt_PoolAllocBlockHead;

/**
 * \struct __NkInt_PoolAllocContext
 * \brief  represents the global pool allocator context
 */
NK_NATIVE typedef struct __NkInt_PoolAllocContext {
    NK_DECL_LOCK(m_mtxLock); /**< synchronization object */

    NkUint32                    m_nAllocPools;    /**< number of currently allocated pools */
    NkUint32                    m_firstFreePool;  /**< offset of the first free block in the pool */
    __NkInt_PoolAllocMemoryPool m_memPools[8192]; /**< static array of memory pools */
} __NkInt_PoolAllocContext;


/**
 * \brief actual instance of the global pool allocator 
 */
NK_INTERNAL __NkInt_PoolAllocContext gl_PoolAllocCxt;
/**
 * \brief maximum number of memory pools allocatable at the same time
 */
NK_INTERNAL NkUint32 const gl_MaxPools = sizeof gl_PoolAllocCxt.m_memPools / sizeof *gl_PoolAllocCxt.m_memPools;
/**
 * \brief block alignment requirement for the pool allocator 
 */
NK_INTERNAL NkUint32 const gl_BlockAlign = 16U;
/**
 * \brief default block count per memory pool 
 */
NK_INTERNAL NkUint32 const gl_DefBlockCount = 128U;


/**
 * \brief  invokes the configuration-specific native allocation function
 * 
 * This function chooses the right allocation function based on current configuration
 * (e.g. *debug*, *debug optimized*, or *deploy* and target platform. This is to make it
 * possible to use native debugging tools that rely on specific functions to be invoked
 * in order to work properly.
 * 
 * \param  [in] allocCxt allocation context supplied with the requested allocation
 * \param  [in] sizeInBytes size in bytes of the requested allocation
 * \param  [in] isZeroed whether or not the newly-allocated memory should be
 *              pre-initialized with all zeroes
 * \return a pointer to the newly-allocated block, or *NULL* on failure
 * \see    __NkInt_FreeMemoryUnaligned
 * \see    __NkInt_ReallocateMemoryUnaligned
 */
NK_INTERNAL NkVoid *__NkInt_AllocateMemoryUnaligned(
    _In_opt_         NkAllocationContext const *allocCxt,
    _In_ _Pre_valid_ NkSize sizeInBytes,
    _In_opt_         NkBoolean isZeroed
) {
    NK_UNREFERENCED_PARAMETER(allocCxt);

#if (defined NK_USE_MSVC_MEMORY_LEAK_DETECTOR)
    if (isZeroed)
        return _calloc_dbg(1, sizeInBytes, _NORMAL_BLOCK,
            allocCxt ? allocCxt->m_filePath.mp_dataPtr : "n/a",
            allocCxt ? allocCxt->m_lineInFile : 0
        );
    else
        return _malloc_dbg(
            sizeInBytes,
            _NORMAL_BLOCK,
            allocCxt ? allocCxt->m_filePath.mp_dataPtr : "n/a",
            allocCxt ? allocCxt->m_lineInFile : 0
        );
#else
    return isZeroed ? calloc(1, sizeInBytes) : malloc(sizeInBytes);
#endif
}

/**
 * \brief  invokes the configuration-specific native reallocation function
 * 
 * This function chooses the actual memory management function call similarly to its
 * counterpart *__NkInt_AllocateMemoryUnaligned*.
 * 
 * \param  [in] allocCxt allocation context supplied with the requested allocation
 * \param  [in] newSizeInBytes new size of the memory block, in bytes
 * \param  [in,out] ptr pointer to the memory block that is to be reallocated
 * \return pointer to the (possibly) moved memory block
 * \see    __NkInt_AllocateMemoryUnaligned
 * \see    __NkInt_FreeMemoryUnaligned
 */
NK_INTERNAL NkVoid *__NkInt_ReallocateMemoryUnaligned(
    _In_opt_         NkAllocationContext const *allocCxt,
    _In_ _Pre_valid_ NkSize newSizeInBytes,
    _In_ _Pre_valid_ NkVoid *ptr 
) {
    NK_UNREFERENCED_PARAMETER(allocCxt);

#if (defined NK_USE_MSVC_MEMORY_LEAK_DETECTOR)
    return _realloc_dbg(
        ptr,
        newSizeInBytes,
        _NORMAL_BLOCK,
        allocCxt ? allocCxt->m_filePath.mp_dataPtr : "n/a",
        allocCxt ? allocCxt->m_lineInFile : 0
    );
#else
    return realloc(ptr, newSizeInBytes);
#endif
}

/**
 * \brief invokes the correct configuration-specific native memory free function
 *
 * This function chooses the actual memory management function call similarly to its
 * counterpart <tt>__NkInt_AllocateMemoryUnaligned()</tt>.
 * 
 * \param [in] ptr raw pointer to the memory that is to be freed
 * \see   __NkInt_AllocateMemoryUnaligned
 * \see   __NkInt_ReallocateMemoryUnaligned
 */
NK_INTERNAL NkVoid __NkInt_FreeMemoryUnaligned(_In_ NkVoid const *ptr) {
#if (defined NK_USE_MSVC_MEMORY_LEAK_DETECTOR)
    _free_dbg((NkVoid *)ptr, _NORMAL_BLOCK);
#else
    free(ptr);
#endif
}

/**
 * \brief  calculates the size of the pool block header section
 * \param  [in] blockCount number of memory blocks in the pool
 * \param  [in] blAlign alignment of the first memory block, in bytes
 * \return block header section size, in bytes
 */
NK_INTERNAL NK_INLINE NkSize __NkInt_PoolAllocCalcPoolHeadSize(
    _In_ NkUint32 blockCount,
    _In_ NkSize blAlign
) {
    NkSize poolHeadSize = 0;

    /* Add the block header section size. */
    poolHeadSize += blockCount * sizeof(__NkInt_PoolAllocBlockHead);
    /* Add the optional padding so that the actual memory blocks are aligned properly. */
    poolHeadSize += blAlign - poolHeadSize % blAlign;

    return poolHeadSize;
}

/**
 * \brief  calculates the size of the entire memory pool
 * \param  [in] blockCount number of blocks
 * \param  [in] blockSize size of one block, in bytes
 * \param  [in] blAlign alignment of the first memory block, in bytes
 * \return total size of the memory pool, in bytes
 * \note   \c blAlign must be a power of two.
 */
NK_INTERNAL NK_INLINE NkSize __NkInt_PoolAllocCalcPoolSize(
    _In_ NkUint32 blockCount,
    _In_ NkUint32 blockSize,
    _In_ NkSize blAlign
) {
    NK_ASSERT(blAlign ^ 0 && (blAlign & (blAlign - 1)) == 0, NkErr_MemoryAlignment);

    NkSize poolSize = 0;
    
    /* Calculate pool head size. */
    poolSize += __NkInt_PoolAllocCalcPoolHeadSize(blockCount, blAlign);
    /* Add the size of the actual memory blocks. */
    poolSize += blockSize * blockCount;

    return poolSize;
}

/**
 * \brief  allocates a new memory pool with the given properties
 * \param  [in] blockCount number of blocks in the pool
 * \param  [in] blockSize size of one block, in bytes
 * \param  [out] poolPtr pointer to a variable that will receive the pointer to the pool
 *               that just got allocated
 * \return \c NkErr_Ok on success, non-zero on failure
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInt_PoolAllocRequestNewPool(
    _In_     NkUint32 blockCount,
    _In_     NkUint32 blockSize,
    _Outptr_ __NkInt_PoolAllocMemoryPool **poolPtr
) {
    /*
     * If there are no free pools, we cannot request a new one. In this case, the
     * pool allocation procedure fails.
     */
    if (gl_PoolAllocCxt.m_firstFreePool == UINT32_MAX)
        return NkErr_MemoryAllocation;

    /* Allocate the new pool by using the general-purpose allocator. */
    NkVoid *blockPtr = NULL;
    NkErrorCode errorCode = NkGPAlloc(
        NK_MAKE_ALLOCATION_CONTEXT(),
        __NkInt_PoolAllocCalcPoolSize(blockCount, blockSize, gl_BlockAlign),
        0,
        NK_FALSE,
        &blockPtr
    );
    if (errorCode ^ NkErr_Ok)
        return NkErr_MemoryAllocation;
    /* Zero the block headers. */
    memset(blockPtr, 0, blockCount * sizeof(__NkInt_PoolAllocBlockHead));

    /* Setup the pool header. */
     *poolPtr = &gl_PoolAllocCxt.m_memPools[gl_PoolAllocCxt.m_firstFreePool];
    **poolPtr = (__NkInt_PoolAllocMemoryPool){
        .m_blockCount   = blockCount,
        .m_fFreeInd     = 0,
        .m_blockSize    = blockSize,
        .m_nAllocBlocks = 0,
        .mp_blockPtr    = blockPtr
    };
    ++gl_PoolAllocCxt.m_nAllocPools;

    /*
     * Find the next free pool. There are two cases:
     *  (1) We just allocated the last available block.
     *  (2) We still have free blocks available.
     */
    if (gl_PoolAllocCxt.m_nAllocPools == gl_MaxPools)
        gl_PoolAllocCxt.m_firstFreePool = UINT32_MAX;
    else {
        for (NkUint32 i = gl_PoolAllocCxt.m_firstFreePool; i < gl_MaxPools; i++)
            if (gl_PoolAllocCxt.m_memPools[i].m_blockSize == 0) {
                gl_PoolAllocCxt.m_firstFreePool = i;

                break;
            }
    }

    NK_LOG_TRACE(
        "Allocated new pool 0x%p of %u elements with a block size of %u.",
        *poolPtr,
        blockCount,
        blockSize
    );
    return NkErr_Ok;
}

/**
 * \brief  checks whether a memory block address is in the specified memory pool
 * \param  [in] memPtr memory block address
 * \param  [in] poolPtr memory pool that is to be searched
 * \return \c non-zero if the block address is located inside the specified pool, or 0 if
 *         not
 */
NK_INTERNAL NK_INLINE NkBoolean __NkInt_PoolAllocIsInsidePool(
    _In_ NkVoid const *memPtr,
    _In_ __NkInt_PoolAllocMemoryPool const *poolPtr
) {
    /*
     * If the offset is in the range of the current pool, the memory address lies
     * within the current pool, so return its index.
     */
    NkSize const poolSz = __NkInt_PoolAllocCalcPoolSize(
        poolPtr->m_blockCount,
        poolPtr->m_blockSize,
        gl_BlockAlign
    );

    return memPtr >= poolPtr->mp_blockPtr && (NkSize)((NkByte *)memPtr - (NkByte *)poolPtr->mp_blockPtr) < poolSz;
}

/**
 * \brief  determines the memory pool the given block address is located in
 * \param  [in] memPtr memory block address
 * \return index of the memory pool the block address could be found in, or \c UINT32_MAX
 *         if the block address could not be located
 */
NK_INTERNAL NK_INLINE NkUint32 __NkInt_PoolAllocLocateBlock(_In_ NkVoid const *memPtr) {
    for (NkUint32 i = 0, j = 0; i < gl_MaxPools && j < gl_PoolAllocCxt.m_nAllocPools; i++) {
        /* Skip empty pools. */
        __NkInt_PoolAllocMemoryPool const *poolPtr = &gl_PoolAllocCxt.m_memPools[i];
        if (poolPtr->m_blockSize == 0)
            continue;

        if (__NkInt_PoolAllocIsInsidePool(memPtr, poolPtr) == NK_TRUE)
            return i;
        ++j;
    }

    /*
     * Went through all pools without getting a match. Either the address used to be
     * allocated or its invalid.
     */
    return UINT32_MAX;
}

/**
 * \brief   calculates the header address of the given memory block address
 * \param   [in] memPtr pointer of the memory block
 * \param   [out] poolPtr pointer to a variable that will receive the pointer to the
 *                memory pool the block address could be located in
 * \return  the address of the respective memory block header, or \c NULL if the block
 *          could not be located
 * \warning If the function fails (that is, the return value is <tt>NULL</tt>), the
 *          contents of \c poolPtr are indeterminate.
 */
NK_INTERNAL _Return_notnull_ __NkInt_PoolAllocBlockHead *__NkInt_PoolAllocGetHeaderForBlock(
    _In_     NkVoid const *memPtr,
    _Outptr_ __NkInt_PoolAllocMemoryPool **poolPtr
) {
    /* Locate the memory pool of the given block pointer. */
    NkUint32 const poolInd = __NkInt_PoolAllocLocateBlock(memPtr);
    if (poolInd == UINT32_MAX)
        return NULL;
    *poolPtr = &gl_PoolAllocCxt.m_memPools[poolInd];

    /* Calculate the address of the first block in the pool. */
    NkSize const headSize = __NkInt_PoolAllocCalcPoolHeadSize((*poolPtr)->m_blockCount, gl_BlockAlign);
    NkByte const *blStart = (NkByte *)(*poolPtr)->mp_blockPtr + headSize;
    NkByte const *bytePtr = (NkByte const *)memPtr;
    NkSize const  blIndex = (bytePtr - blStart) / (*poolPtr)->m_blockSize;
    
    /* Calculate the address of the block head associated with the memory address. */
    __NkInt_PoolAllocBlockHead *blHeadPtr = &((__NkInt_PoolAllocBlockHead *)(*poolPtr)->mp_blockPtr)[blIndex];
    /*
     * If the block is part of a multiblock allocation, go back in the pool head until
     * the nearest 'start of allocation' block is found. Return that block.
     */
    while (blHeadPtr->m_blockFlags & NkIntAllocTy_PartOfMulti && (NkVoid *)blHeadPtr > (*poolPtr)->mp_blockPtr) {
        if (blHeadPtr->m_blockFlags & NkIntAllocTy_StartOfMulti)
            break;

        --blHeadPtr;
    }

    /*
     * Validate that the returned block indeed has the desired properties and the search
     * did not just abort because we had reached the start of the pool. If this happens,
     * it possibly indicates a bug with the block flags.
     */
    NK_ASSERT_EXTRA(
        blHeadPtr->m_blockFlags & NkIntAllocTy_StartOfMulti || (NkVoid *)blHeadPtr >= (*poolPtr)->mp_blockPtr,
        NkErr_ComponentState,
        "Could not find start of multiblock allocation. Check block flags."
    );
    return blHeadPtr;
}

/**
 * \brief  locates the first free block range of at least \c blockCount blocks
 * \param  [in] poolPtr pointer to the memory pool that is to be traversed
 * \param  [in] blockCount number of blocks to allocate
 * \return the index of the first block in the allocation if the function could find a
 *         suitable block, or \c UINT32_MAX if no such range could be found
 */
NK_INTERNAL NkUint32 __NkInt_PoolAllocFindRangeInPool(
    _In_ __NkInt_PoolAllocMemoryPool const *poolPtr,
    _In_ NkUint32 blockCount
) {
    NkUint32 startIndex = poolPtr->m_fFreeInd;
    if (startIndex == UINT32_MAX)
        return UINT32_MAX;

lbl_FINDBLOCK:
    /* If there is not enough space left to hold the allocation, abort. */
    if (poolPtr->m_blockCount - startIndex < blockCount)
        return UINT32_MAX;
    /*
     * Scan the current range for used blocks. If there is a used block in the range,
     * search for the next free block and continue from there.
     */
    NkUint32 k = startIndex;
    __NkInt_PoolAllocBlockHead *blockPtr;
    do {
        blockPtr = &((__NkInt_PoolAllocBlockHead *)poolPtr->mp_blockPtr)[k];

        if (blockPtr->m_blockFlags ^ 0) {
            startIndex = k;

            goto lbl_FINDNEXT;
        }
    } while (++k < startIndex + blockCount);

    return startIndex;
lbl_FINDNEXT:
    while (startIndex < poolPtr->m_blockCount && blockPtr->m_blockFlags ^ 0)
        ++startIndex;

    /* Found the next free block. Scan the range from there. */
    goto lbl_FINDBLOCK;
}
/** \endcond */



_Return_ok_ NkErrorCode NK_CALL NkAllocInitialize(NkVoid) {
    NK_LOG_INFO("init: allocators");

    NK_INITLOCK(gl_PoolAllocCxt.m_mtxLock);
    return NkErr_Ok;
}

NkVoid NK_CALL NkAllocUninitialize(NkVoid) {
    NK_LOG_INFO("uninit: allocators");

    /* Free all memory pools. */
    for (NkUint32 i = 0, j = 0; i < gl_MaxPools && j < gl_PoolAllocCxt.m_nAllocPools; i++) {
        /* Skip unallocated pools. */
        if (gl_PoolAllocCxt.m_memPools[i].m_blockSize == 0)
            continue;

        NkGPFree(gl_PoolAllocCxt.m_memPools[i].mp_blockPtr);
        ++j;
    }
    /* Destroy lock. */
    NK_DESTROYLOCK(gl_PoolAllocCxt.m_mtxLock);
}


_Return_ok_ NkErrorCode NK_CALL NkGPAlloc(
    _In_opt_         NkAllocationContext const *allocCxt,
    _In_ _Pre_valid_ NkSize sizeInBytes,
    _In_opt_         NkSize alignInBytes,
    _In_opt_         NkBoolean isZeroed,
    _Init_ptr_       NkVoid **memPtr
) {
    NK_ASSERT(sizeInBytes ^ 0, NkErr_InParameter);
    NK_ASSERT(alignInBytes == 0, NkErr_NotImplemented);
    NK_ASSERT(memPtr != NULL, NkErr_OutptrParameter);

    /* Allocate memory block. */
    return (*memPtr = __NkInt_AllocateMemoryUnaligned(allocCxt, sizeInBytes, isZeroed)) != NULL
        ? NkErr_Ok
        : NkErr_MemoryAllocation
    ;
};

_Return_ok_ NkErrorCode NK_CALL NkGPRealloc(
    _In_opt_         NkAllocationContext const *allocCxt,
    _In_ _Pre_valid_ NkSize newSizeInBytes,
    _Reinit_ptr_     NkVoid **memPtr
) {
    NK_ASSERT(memPtr != NULL && *memPtr != NULL, NkErr_InptrParameter);
    NK_ASSERT(newSizeInBytes ^ 0, NkErr_InParameter);

    /* Try to reallocate memory. */
    NkVoid *newMemPtr = __NkInt_ReallocateMemoryUnaligned(allocCxt, newSizeInBytes, *memPtr);
    if (newMemPtr == NULL)
        return NkErr_MemoryReallocation;

    /* Update current pointer. */
    *memPtr = newMemPtr;
    return NkErr_Ok;
}

NkVoid NK_CALL NkGPFree(_In_ NkVoid const *memPtr) {
    if (memPtr == NULL)
        return;

    __NkInt_FreeMemoryUnaligned(memPtr);
}



_Return_ok_ NkErrorCode NK_CALL NkPoolAlloc(
    _In_opt_   NkAllocationContext const *allocCxt,
    _In_       NkUint32 blockSize,
    _In_opt_   NkUint32 blockCount,
    _Init_ptr_ NkVoid **memPtr
) {
    NK_UNREFERENCED_PARAMETER(allocCxt);
    NK_ASSERT(blockSize != 0, NkErr_InParameter);
    NK_ASSERT(blockCount != 0, NkErr_InParameter);
    NK_ASSERT(memPtr != NULL, NkErr_OutptrParameter);

    NK_LOCK(gl_PoolAllocCxt.m_mtxLock);

    /*
     * Try to find a pool for the requested block size that has enough space for the
     * allocation.
     */
    __NkInt_PoolAllocMemoryPool *memPool;
    NkUint32 blockIndex = 0;
    for (NkUint32 i = 0, j = 0; i < gl_MaxPools && j < gl_PoolAllocCxt.m_nAllocPools; i++) {
        if ((memPool = &gl_PoolAllocCxt.m_memPools[i])->m_blockSize == 0)
            continue;

        /*
         * Found a block with a suitable block size; check if it contains a free range
         * that is large enough to hold the allocation.
         */
        if (memPool->m_blockSize == blockSize)
            if ((blockIndex = __NkInt_PoolAllocFindRangeInPool(memPool, blockCount)) ^ UINT32_MAX)
                goto lbl_ALLOCBLOCK;
        ++j;
    }
    memPool = NULL;
    
lbl_ALLOCBLOCK:
    if (memPool != NULL) {
        /* Get a pointer to the block header section of each pool. */
        __NkInt_PoolAllocBlockHead *basePtr = (__NkInt_PoolAllocBlockHead *)memPool->mp_blockPtr;
        __NkInt_PoolAllocBlockHead *specPtr;

        /* Set the allocation flag. */
        (specPtr = &basePtr[blockIndex])->m_blockFlags |= blockCount ^ 1 ? NkIntAllocTy_StartOfMulti : NkIntAllocTy_Single;
        /* Set additional fields if a multiblock allocation is being requested. */
        if (blockCount > 1) {
            /* Set block count on first block of allocation. */
            specPtr->m_nMultiBlock = blockCount;

            /* Go through all blocks and set the 'part of multiblock allocation' flag. */
            for (NkUint32 i = blockIndex; i < blockIndex + blockCount; i++)
                (specPtr = &basePtr[i])->m_blockFlags |= NkIntAllocTy_PartOfMulti;
        }
        /* Update the number of allocated blocks in the respective pool. */
        memPool->m_nAllocBlocks += blockCount;

        /*
         * Update the first free index of the pool if the index lies in the allocated
         * range.
         */
        if (blockIndex == memPool->m_fFreeInd) {
            /* Search for the next free block to update the 'first-free'-index. */
            for (NkUint32 i = memPool->m_fFreeInd; i < memPool->m_blockCount; i++) {
                if (basePtr[i].m_blockFlags == 0) {
                    memPool->m_fFreeInd = i;

                    goto lbl_RETBLOCKPTR;
                }
            }

            /* No free block could be found. -> Mark the current pool as 'full'. */
            memPool->m_fFreeInd = UINT32_MAX;
        }

lbl_RETBLOCKPTR:
        *memPtr = (NkByte *)(basePtr + memPool->m_blockCount)
            + gl_BlockAlign - (memPool->m_blockCount * sizeof *specPtr) % gl_BlockAlign
            + memPool->m_blockSize * (&basePtr[blockIndex] - basePtr);

        NK_LOG_TRACE(
            "Allocated %u memory block(s) [%u ->] (0x%p) [base: 0x%p] in pool [%u] (0x%p).",
            blockCount,
            blockIndex,
            *memPtr,
            basePtr,
            (NkUint32)(memPool - &gl_PoolAllocCxt.m_memPools[0]),
            memPool
        );
        NK_UNLOCK(gl_PoolAllocCxt.m_mtxLock);
        return NkErr_Ok;
    }

    /*
     * If no such memory pool could be found, allocate a new pool if possible and
     * allocate the new block at its firstindex.
     */
    NkErrorCode errorCode = __NkInt_PoolAllocRequestNewPool(gl_DefBlockCount, blockSize, &memPool);
    blockIndex = 0;
    if (errorCode != NkErr_Ok) {
        NK_UNLOCK(gl_PoolAllocCxt.m_mtxLock);

        return errorCode;
    }

    /*
     * Run the block allocation procedure again. This time, no new pool allocation will
     * be necessary. *memPool* is now initialized with the pointer to the pool header of
     * the newly-allocated pool so we do not have to search for it again.
     */
    goto lbl_ALLOCBLOCK;
}

_Return_ok_ NkErrorCode NK_CALL NkPoolReserve(_In_ NkUint32 blockSize, _In_ NkUint32 blockCount) {
    NK_ASSERT(blockSize != 0, NkErr_InParameter);
    NK_ASSERT(blockCount != 0, NkErr_InParameter);

    NkErrorCode errorCode;
    NK_SYNCHRONIZED(gl_PoolAllocCxt.m_mtxLock, {
        errorCode = __NkInt_PoolAllocRequestNewPool(
            blockCount,
            blockSize,
            &(__NkInt_PoolAllocMemoryPool *){ NULL }
        );
    });

    return errorCode;
}

NkVoid NK_CALL NkPoolFree(_Inout_opt_ NkVoid *memPtr) {
    if (memPtr == NULL)
        return;

    NK_LOCK(gl_PoolAllocCxt.m_mtxLock);

    /* Get the block header for the given memory address. */
    __NkInt_PoolAllocMemoryPool *poolPtr;
    __NkInt_PoolAllocBlockHead *blockPtr = __NkInt_PoolAllocGetHeaderForBlock(memPtr, &poolPtr);
    if (poolPtr == NULL || blockPtr == NULL) {
        NK_LOG_WARNING("Passed out of range memory address 0x%p to pool allocator.", memPtr);

        NK_UNLOCK(gl_PoolAllocCxt.m_mtxLock);
        return;
    }

    /* Mark all blocks that are part of the allocation as 'free'. */
    NkUint32 const blockCount = blockPtr->m_blockFlags & NkIntAllocTy_Single ? 1 : blockPtr->m_nMultiBlock;
    for (NkUint32 i = 0; i < blockCount; i++)
        blockPtr[i] = (__NkInt_PoolAllocBlockHead){ .m_blockFlags = 0, .m_nMultiBlock = 0 };

    /* Adjust the 'first free' index if needed. */
    if ((poolPtr->m_nAllocBlocks -= blockCount) > 0) {
        NkUint32 const blockIndex = (NkUint32)(blockPtr - (__NkInt_PoolAllocBlockHead *)poolPtr->mp_blockPtr);

        poolPtr->m_fFreeInd = poolPtr->m_fFreeInd > blockIndex ? blockIndex : poolPtr->m_fFreeInd;
        NK_LOG_TRACE(
            "Freed memory %u memory blocks [starting from %u] (0x%p) [blsz=%u] in pool index [%u] (0x%p).",
            blockCount,
            blockIndex,
            memPtr,
            poolPtr->m_blockSize,
            (NkUint32)(poolPtr - &gl_PoolAllocCxt.m_memPools[0]),
            poolPtr
        );
    } else {
        /*
         * If the pool is now empty, free the pool for reuse. First, calculate the pool
         * index.
         */
        NkUint32 const poolIndex = (NkUint32)(poolPtr - &gl_PoolAllocCxt.m_memPools[0]);

        /* Search for the next free index if needed. */
        for (NkUint32 i = poolIndex; i < gl_MaxPools; i++)
            if (gl_PoolAllocCxt.m_memPools[i].m_blockSize == 0)
                gl_PoolAllocCxt.m_firstFreePool = i;
        /* Free the pool memory. */
        NkGPFree(poolPtr->mp_blockPtr);

        /* Lastly, reset the pool header. */
        NK_LOG_TRACE(
            "Freed memory pool [%u] (0x%p) [sz=%u, count=%u]; %u pools still allocated.",
            poolIndex,
            poolPtr,
            poolPtr->m_blockSize,
            poolPtr->m_blockCount,
            gl_PoolAllocCxt.m_nAllocPools - 1
        );
        *poolPtr = (__NkInt_PoolAllocMemoryPool){
            .m_blockSize    = 0,
            .m_blockCount   = 0,
            .m_fFreeInd     = 0,
            .m_nAllocBlocks = 0,
            .mp_blockPtr    = NULL
        };
        --gl_PoolAllocCxt.m_nAllocPools;
    }

    NK_UNLOCK(gl_PoolAllocCxt.m_mtxLock);
}

NkUint32 NK_CALL NkPoolGetBlockSize(_In_ NkVoid const *memPtr) {
    NK_ASSERT(memPtr != NULL, NkErr_InParameter);

    NkUint32 res;
    NK_SYNCHRONIZED(gl_PoolAllocCxt.m_mtxLock, {
        /* Get the memory pool the memory block pointed to by *memPtr* is located in. */
        NkUint32 poolInd = __NkInt_PoolAllocLocateBlock(memPtr);
        if (poolInd == UINT32_MAX)
            return 0;

        res = gl_PoolAllocCxt.m_memPools[poolInd].m_blockSize;
    });

    return res;
}

NkUint32 NK_CALL NkPoolGetAllocSize(_In_ NkVoid const *memPtr) {
    NK_ASSERT(memPtr != NULL, NkErr_InParameter);

    NkUint32 res;
    NK_SYNCHRONIZED(gl_PoolAllocCxt.m_mtxLock, {
        /*
         * Get the pointer to the block header. If the given block is part of a
         * multiblock allocation, the returned block header is the header of the first
         * block in the allocation.
         */
        __NkInt_PoolAllocMemoryPool *poolHead;
        __NkInt_PoolAllocBlockHead *blHead = __NkInt_PoolAllocGetHeaderForBlock(memPtr, &poolHead);
        if (blHead == NULL || poolHead == NULL)
            return 0;
        
        /* Calculate the size of the allocation based on the allocation's type. */
        res = (NkUint32)(
            poolHead->m_blockSize * (blHead->m_blockFlags & NkIntAllocTy_Single
                ? 1
                : blHead->m_nMultiBlock
            )
        );
    });
    
    return res;
}


#undef NK_NAMESPACE


