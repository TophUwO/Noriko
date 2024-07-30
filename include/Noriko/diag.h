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
 * \file  diag.h
 * \brief global definitions for Noriko's diagnosis component
 *
 * This header defines the public API for Noriko's built-in debugging and diagnosis tools.
 * Throughout the application's lifetime, this component will collect all data from logs,
 * over allocations, events, errors, etc. and generate a HTML document from it that can
 * be viewed by a browser for easier traversing and filtering.
 * 
 * \note This component is active by default in all build configurations - even in
 *       \e deploy builds. That behavior can be toggled with the <tt>--diag=off</tt>
 *       option. Besides turning it off entirely, there are options to use more "lean"
 *       versions of the diagnosis tools (e.g., via --diag=lean).
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>
#include <include/Noriko/log.h>


/**
 * \enum  NkDiagEntryType
 * \brief represents all numeric type IDs processable by the diagnosis tools
 */
NK_NATIVE typedef _In_range_(0, __NkDiTy_Count__ - 1) enum NkDiagEntryType {
    NkDiType_Alloc,   /**< new allocation entry */
    NkDiType_Realloc, /**< reallocation entry (must have a corresponding \c *Alloc entry) */
    NkDiType_Free,    /**< memory free entry (must have a corresponding <tt>*[Rea-/A]lloc</tt> entry) */
    NkDiType_Log,     /**< log message entry */

    __NkDiTy_Count__  /**< *only used internally* */
} NkDiagEntryType;


/**
 * \struct NkDiagEntryAllocContext
 * \brief  represents the context for an allocation entry; i.e., additional information
 *         about the allocation to simplify debugging and logging
 */
NK_NATIVE typedef struct NkDiagEntryAllocContext {
    NkInt32 m_allocId;      /**< numeric ID of the allocator */
    NkSize  m_allocSize;    /**< requested size of the block */
    NkSize  m_blSize;       /**< actual size of the block */
    NkSize  m_alignInBytes; /**< alignment in bytes */
    NkVoid *mp_memPtr;      /**< pointer to the allocated memory block */
} NkDiagEntryAllocContext;

/**
 * \struct NkDiagEntryLogContext
 * \brief  holds the data for the current log message
 * \note   The data pointers are expected not to change until the processing function
 *         returns.
 */
NK_NATIVE typedef struct NkDiagEntryLogContext {
    NkLogLevel   m_lvlId;      /**< numeric log level ID */
    NkStringView m_fmtMessage; /**< pointer to the formatted message */
} NkDiagEntryLogContext;

/**
 * \struct NkDiagEntryContext
 * \brief  represents additional entry context used for logging, etc.
 */
NK_NATIVE typedef struct NkDiagEntryContext {
    NkStringView m_filePath;   /**< file path of where the entry originated */
    NkStringView m_nsName;     /**< namespace */
    NkStringView m_funcName;   /**< name of the function */
    NkUint64     m_lineInFile; /**< line relative to the origin file */
    NkFlags      m_entryFlags; /**< additional entry flags */
    NkUint64     m_tsValue;    /**< value of the timestamp (UNIX epoch) */

    /**
     * \brief represents the additional type-specific context
     * \note  Only one member of the following union is valid. What member it is depends
     *        on the value of the \c NkDiagEntryProperties::m_entryType field.
     */
    union {
        NkDiagEntryAllocContext m_allocCxt; /**< allocation context */
        NkDiagEntryLogContext   m_logCxt;   /**< log message context */
    };
} NkDiagEntryContext;

/**
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkDiagEntryProperties {
    NkSize             m_structSize;
    NkDiagEntryType    m_entryType;
    NkDiagEntryContext m_cxtStruct;
} NkDiagEntryProperties;


/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkDiagInitialize(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkDiagUninitialize(NkVoid);
/**
 */
NK_NATIVE NK_API NkVoid NK_CALL NkDiagFlush(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkDiagSubmitEntry(NkDiagEntryProperties const *entryPropsPtr);


