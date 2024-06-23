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


/* stdlib includes */
#include <string.h>

/* Noriko includes */
#include <include/Noriko/platform.h>
#include <include/Noriko/alloc.h>


/**
 * \brief represents the internal state for the general-purpose memory allocator 
 */
static NkAllocatorState gl_GPAlloc = {
    .m_structSize        = sizeof gl_GPAlloc,
    .mp_allocatorName    = &(NkStringView)NK_MAKE_STRING_VIEW("gp-alloc"),
    .m_nAllocSucceeded   = 0,
    .m_nAllocFailed      = 0,
    .m_nReallocSucceeded = 0,
    .m_nReallocFailed    = 0,
    .m_nFreeSucceeded    = 0,
    .m_nFreeFailed       = 0,
    .m_minAllocBytes     = 0,
    .m_maxAllocBytes     = 0,
    .m_nBytesAllocated   = 0,
    .m_nBytesFreed       = 0
};


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
 * \return a pointer to the newly-allocated block, or *NULL* on failure
 * \see    NkFreeMemoryUnaligned_Impl
 */
NK_INTERNAL void *NK_CALL NkAllocateMemoryUnaligned_Impl(
    _In_opt_         NkAllocationContext const *const allocCxt,
    _In_ _Pre_valid_ size_t sizeInBytes
) {
#if (defined NK_USE_MSVC_MEMORY_LEAK_DETECTOR)
    return _malloc_dbg(
        sizeInBytes,
        _NORMAL_BLOCK,
        allocCxt ? allocCxt->mp_filePath->mp_dataPtr : u8"n/a",
        allocCxt ? allocCxt->m_lineInFile : 0
    );
#else
    return malloc(sizeInBytes);
#endif
}

/**
 * \brief  invokes the configuration-specific native reallocation function
 * 
 * This function chooses the function similarly to its counterpart *NkAllocateMemoryUnaligned_Impl*.
 * 
 * \param  [in] allocCxt allocation context supplied with the requested allocation
 * \param  [in] newSizeInBytes new size of the memory block, in bytes
 * \param  [in,out] ptr pointer to the memory block that is to be reallocated
 * \return pointer to the (possibly) moved memory block
 */
NK_INTERNAL void *NK_CALL NkReallocateMemoryUnaligned_Impl(
    _In_opt_         NkAllocationContext const *const allocCxt,
    _In_ _Pre_valid_ size_t newSizeInBytes,
    _In_ _Pre_valid_ void *ptr 
) {
#if (defined NK_USE_MSVC_MEMORY_LEAK_DETECTOR)
    return _realloc_dbg(
        ptr,
        newSizeInBytes,
        _NORMAL_BLOCK,
        allocCxt ? allocCxt->mp_filePath->mp_dataPtr : u8"n/a",
        allocCxt ? allocCxt->m_lineInFile : 0
    );
#else
    return realloc(ptr, newSizeInBytes);
#endif
}

/**
 * \brief invokes the correct configuration-specific native memory free function
 *
 * This function chooses the function similarly to its counterpart *NkAllocateMemoryUnaligned_Impl*.
 * 
 * \param [in] ptr raw pointer to the memory that is to be freed
 * \see   NkAllocateMemoryUnaligned_Impl
 */
NK_INTERNAL void NkFreeMemoryUnaligned_Impl(_In_ _Pre_valid_ void *ptr) {
#if (defined NK_USE_MSVC_MEMORY_LEAK_DETECTOR)
    _free_dbg(ptr, _NORMAL_BLOCK);
#else
    free(ptr);
#endif
}


_Return_ok_ NkErrorCode NK_CALL NkAllocateMemory(
    _In_opt_                      NkAllocationContext const *const allocCxt,
    _In_ _Pre_valid_              size_t sizeInBytes,
    _In_opt_                      size_t alignInBytes,
    _Outptr_ _Deref_post_notnull_ void **memPtr
) { NK_DEFINE_ERROR_CODE
    /* Validate parameters. */
    NK_BASIC_ASSERT(sizeInBytes ^ 0, NkErr_InParameter);
    NK_BASIC_ASSERT(alignInBytes == 0, NkErr_NotImplemented);
    NK_BASIC_ASSERT(memPtr != NULL, NkErr_OutptrParameter);

    /* Allocate memory block. */
    *memPtr = NkAllocateMemoryUnaligned_Impl(allocCxt, sizeInBytes);
    NK_BASIC_ASSERT(*memPtr != NULL, NkErr_MemoryAllocation);

    /* Update allocator state. */
    gl_GPAlloc.m_nBytesAllocated += sizeInBytes;
    gl_GPAlloc.m_nAllocSucceeded += 1;
    gl_GPAlloc.m_minAllocBytes    = min(gl_GPAlloc.m_minAllocBytes, sizeInBytes);
    gl_GPAlloc.m_maxAllocBytes    = max(gl_GPAlloc.m_maxAllocBytes, sizeInBytes);
    return NkErr_Ok;

NK_ON_ERROR:
    ++gl_GPAlloc.m_nAllocFailed;

    return NK_ERROR_CODE;
};

_Return_ok_ NkErrorCode NK_CALL NkReallocateMemory(
    _In_opt_                                        NkAllocationContext const *const allocCxt,
    _In_ _Pre_valid_                                size_t newSizeInBytes,
    _Outptr_ _Deref_pre_valid_ _Deref_post_notnull_ void **memPtr
) {
    NK_DEFINE_ERROR_CODE
    /* Parameter validation. */
    NK_BASIC_ASSERT(memPtr != NULL && *memPtr != NULL, NkErr_InptrParameter);
    NK_BASIC_ASSERT(newSizeInBytes ^ 0, NkErr_InParameter);

    /* Try to reallocate memory. */
    void *newMemPtr = NkReallocateMemoryUnaligned_Impl(allocCxt, newSizeInBytes, *memPtr);
    NK_BASIC_ASSERT(newMemPtr != NULL, NkErr_MemoryReallocation);

    /* Update current pointer and allocator state. */
    *memPtr = newMemPtr;
    gl_GPAlloc.m_nReallocSucceeded += 1;
    gl_GPAlloc.m_minAllocBytes      = min(gl_GPAlloc.m_minAllocBytes, newSizeInBytes);
    gl_GPAlloc.m_maxAllocBytes      = max(gl_GPAlloc.m_maxAllocBytes, newSizeInBytes);
    return NkErr_Ok;

NK_ON_ERROR:
    ++gl_GPAlloc.m_nReallocFailed;

    return NK_ERROR_CODE;
}

_Return_ok_ NkErrorCode NK_CALL NkFreeMemory(_Pre_valid_ _Deref_post_null_ void **memPtr) { NK_DEFINE_ERROR_CODE
    /* Parameter validation. */
    NK_BASIC_ASSERT(memPtr != NULL, NkErr_InParameter);
    
    /* Free memory. */
    NkFreeMemoryUnaligned_Impl(*memPtr);
    *memPtr = NULL;

    /* Update allocator state. */
    gl_GPAlloc.m_nFreeSucceeded += 1;
    gl_GPAlloc.m_nBytesFreed    += 0;
    return NkErr_Ok;

NK_ON_ERROR:
    ++gl_GPAlloc.m_nFreeFailed;

    return NK_ERROR_CODE;
}


_Return_ok_ NkErrorCode NK_CALL NkGetAllocatorState(
    _In_ _Pre_valid_                 NkStringView const *const debugName,
    _Out_ _Pre_notnull_ _Post_valid_ NkAllocatorState *const bufferPtr
) { NK_DEFINE_ERROR_CODE
    /* Parameter validation. */
    NK_BASIC_ASSERT(debugName != NULL, NkErr_InParameter);
    NK_BASIC_ASSERT(bufferPtr != NULL, NkErr_OutParameter);

    /*
     * Copy requested allocator state into buffer. If the allocator with the given debug
     * name could not be found, return error.
     */
    if (!strcmp(debugName->mp_dataPtr, gl_GPAlloc.mp_allocatorName->mp_dataPtr))
        memcpy(bufferPtr, &gl_GPAlloc, bufferPtr->m_structSize);
    else
        NK_FAIL_WITH(NkErr_InParameter);

    /* Everything went well. */
    return NkErr_Ok;

NK_ON_ERROR:
    if (bufferPtr != NULL)
        memset(bufferPtr, 0, bufferPtr->m_structSize);

    return NK_ERROR_CODE;
}


