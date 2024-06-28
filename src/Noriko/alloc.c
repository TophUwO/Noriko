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
#define NK_NAMESPACE u8"nk::alloc"


/* stdlib includes */
#include <string.h>

/* Noriko includes */
#include <include/Noriko/alloc.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/util.h>


/**
 * \brief represents the internal state for the general-purpose memory allocator 
 */
NK_NATIVE typedef struct NkGeneralPurposeAllocator {
    NkAllocatorState m_allocState; /**< allocator state */
} NkGeneralPurposeAllocator;


/**
 * \brief global general-purpose allocator state 
 */
NK_INTERNAL NK_NATIVE NkGeneralPurposeAllocator gl_GPAllocator = {
    .m_allocState = {
        .m_structSize        = sizeof gl_GPAllocator,
        .mp_allocatorName    = &(NkStringView)NK_MAKE_STRING_VIEW("gp-alloc"),
        .m_currMemUsage      = 0,
        .m_minAllocBytes     = 0,
        .m_maxAllocBytes     = 0,
        .m_nBytesAllocated   = 0,
        .m_nBytesFreed       = 0
    }
};
NK_INTERNAL NK_NATIVE NkAllocatorState *const gl_GPAState = &gl_GPAllocator.m_allocState;


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
 * \see    NkInternalFreeMemoryUnaligned
 * \see    NkInternalReallocateMemoryUnaligned
 */
NK_INTERNAL NkVoid *NK_CALL NkInternalAllocateMemoryUnaligned(
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
 * This function chooses the actual memory management function call similarly to its
 * counterpart *NkInternalAllocateMemoryUnaligned*.
 * 
 * \param  [in] allocCxt allocation context supplied with the requested allocation
 * \param  [in] newSizeInBytes new size of the memory block, in bytes
 * \param  [in,out] ptr pointer to the memory block that is to be reallocated
 * \return pointer to the (possibly) moved memory block
 * \see    NkInternalAllocateMemoryUnaligned
 * \see    NkInternalFreeMemoryUnaligned
 */
NK_INTERNAL NkVoid *NK_CALL NkInternalReallocateMemoryUnaligned(
    _In_opt_         NkAllocationContext const *const allocCxt,
    _In_ _Pre_valid_ size_t newSizeInBytes,
    _In_ _Pre_valid_ NkVoid *ptr 
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
 * This function chooses the actual memory management function call similarly to its
 * counterpart *NkInternalAllocateMemoryUnaligned*.
 * 
 * \param [in] ptr raw pointer to the memory that is to be freed
 * \see   NkInternalAllocateMemoryUnaligned
 * \see   NkInternalReallocateMemoryUnaligned
 */
NK_INTERNAL NkVoid NkInternalFreeMemoryUnaligned(NkVoid *ptr) {
#if (defined NK_USE_MSVC_MEMORY_LEAK_DETECTOR)
    _free_dbg((void *)ptr, _NORMAL_BLOCK);
#else
    free(ptr);
#endif
}


_Return_ok_ NkErrorCode NK_CALL NkAllocateMemory(
    _In_opt_         NkAllocationContext const *const allocCxt,
    _In_ _Pre_valid_ size_t sizeInBytes,
    _In_opt_         size_t alignInBytes,
    _Init_ptr_       NkVoid **memPtr
) {
    /* Validate parameters. */
    NK_ASSERT(sizeInBytes ^ 0, NkErr_InParameter);
    NK_ASSERT(alignInBytes == 0, NkErr_NotImplemented);
    NK_ASSERT(memPtr != NULL, NkErr_OutptrParameter);

    /* Allocate memory block. */
    return (*memPtr = NkInternalAllocateMemoryUnaligned(allocCxt, sizeInBytes)) != NULL
        ? NkErr_Ok
        : NkErr_MemoryAllocation
    ;
};

_Return_ok_ NkErrorCode NK_CALL NkReallocateMemory(
    _In_opt_         NkAllocationContext const *const allocCxt,
    _In_ _Pre_valid_ size_t newSizeInBytes,
    _Reinit_ptr_     NkVoid **memPtr
) {
    /* Parameter validation. */
    NK_ASSERT(memPtr != NULL && *memPtr != NULL, NkErr_InptrParameter);
    NK_ASSERT(newSizeInBytes ^ 0, NkErr_InParameter);

    /* Try to reallocate memory. */
    NkVoid *newMemPtr = NkInternalReallocateMemoryUnaligned(allocCxt, newSizeInBytes, *memPtr);
    if (newMemPtr == NULL)
        return NkErr_MemoryReallocation;

    /* Update current pointer. */
    *memPtr = newMemPtr;
    return NkErr_Ok;
}

NkVoid NK_CALL NkFreeMemory(NkVoid *memPtr) {
    /* Parameter validation. */
    if (memPtr == NULL)
        return;

    /* Free memory. */
    NkInternalFreeMemoryUnaligned(memPtr);
}


_Return_ok_ NkErrorCode NK_CALL NkGetAllocatorState(
    _In_ _Pre_valid_                 NkStringView const *const debugName,
    _Out_ _Pre_notnull_ _Post_valid_ NkAllocatorState *const bufferPtr
) {
    /* Parameter validation. */
    NK_ASSERT(debugName != NULL, NkErr_InParameter);
    NK_ASSERT(bufferPtr != NULL, NkErr_OutParameter);

    /*
     * Copy requested allocator state into buffer. If the allocator with the given debug
     * name could not be found, return error.
     */
    if (!strcmp(debugName->mp_dataPtr, gl_GPAState->mp_allocatorName->mp_dataPtr))
        memcpy(bufferPtr, gl_GPAState, bufferPtr->m_structSize);
    else {
        memset(bufferPtr, 0, bufferPtr->m_structSize);

        return NkErr_NamedItemNotFound;
    }

    /* Everything went well. */
    return NkErr_Ok;
}


#undef NK_NAMESPACE


