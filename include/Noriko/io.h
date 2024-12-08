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
 * \file  io.h
 * \brief represents structures and interfaces for interacting with filesystems, networks,
 *        and other input and output devices
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>
#include <include/Noriko/error.h>
#include <include/Noriko/nkom.h>

#include <include/Noriko/dstruct/string.h>


/**
 */
typedef NkErrorCode (NK_CALL *NkDirectoryTraverseFn)(_In_z_ _Utf8_ char const *, _Inout_opt_ NkVoid *);


/**
 */
NK_NATIVE typedef enum NkStreamType {
    NkStrTy_Unknown = 0,

    NkStrTy_Directory,
    NkStrTy_DiskFile,

    __NkStrTy_Count__
} NkStreamType;

/**
 */
NK_NATIVE typedef enum NkSeekOrigin {
    NkSeekOri_Set,
    NkSeekOri_Cur,
    NkSeekOri_End,

    __NkSeekOri_Count__
} NkSeekOrigin;

/**
 */
NK_NATIVE typedef enum NkStreamIOMode {
    NkStrMd_Unknown      = 0,
                         
    NkStrMd_Read         = 1 << 0,
    NkStrMd_Write        = 1 << 1,
    NkStrMd_Append       = 1 << 2,
    NkStrMd_Text         = 1 << 3,
    NkStrMd_Binary       = 1 << 4,
    NkStrMd_TempFile     = 1 << 5,
    NkStrMd_MustNotExist = 1 << 6,

    __NkStrMd_Count__
} NkStreamIOMode;


/**
 */
NK_NATIVE typedef _Struct_size_bytes_(m_structSize) struct NkStreamStat {
    NkSize         m_structSize;
    NkStreamType   m_type;
    NkStreamIOMode m_ioMode;
    NkUint64       m_crTime;
    NkUint64       m_mdTime;
    NkUint64       m_aTime;
    NkSize         m_size;
    NkOffset       m_currOff;
} NkStreamStat;


/**
 */
NKOM_DECLARE_INTERFACE(NkIStream) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkIStream *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIStream *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIStream *self);

    /**
     */
    NkErrorCode (NK_CALL *GetStat)(_Inout_ NkIStream *self, _Out_ NkStreamStat *statPtr);

    /**
     */
    NkErrorCode (NK_CALL *Read)(_Inout_ NkIStream *self, _In_ NkSize s, _O_bytes_(s) NkVoid *bufPtr, _Out_ NkSize *br);
    /**
     */
    NkErrorCode (NK_CALL *Write)(
        _Inout_      NkIStream *self,
        _In_         NkSize s,
        _I_bytes_(s) NkVoid const *bufPtr,
        _Out_        NkSize *bw
    );
};

/**
 */
NKOM_DECLARE_INTERFACE(NkIFile) {
    /**
     * \brief reimplements <tt>NkIStream::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkIFile *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
    /**
     * \brief reimplements <tt>NkIStream::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIFile *self);
    /**
     * \brief reimplements <tt>NkIStream::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIFile *self);

    /**
     * \brief reimplements <tt>NkIStream::GetStat()</tt>
     */
    NkErrorCode (NK_CALL *GetStat)(_Inout_ NkIFile *self, _Out_ NkStreamStat *statPtr);

    /**
     * \brief reimplements <tt>NkIStream::Read()</tt>
     */
    NkErrorCode (NK_CALL *Read)(_Inout_ NkIFile *self, _In_ NkSize s, _O_bytes_(s) NkVoid *bufPtr, _Out_ NkSize *br);
    /**
     * \brief reimplements <tt>NkIStream::Write()</tt>
     */
    NkErrorCode (NK_CALL *Write)(
        _Inout_      NkIFile *self,
        _In_         NkSize s,
        _I_bytes_(s) NkVoid const *bufPtr,
        _Out_        NkSize *bw
    );
    
    /**
     */
    NkErrorCode (NK_CALL *Open)(_Inout_ NkIFile *self, _In_z_ _Utf8_ char const *pathStr, _In_ NkStreamIOMode mode);
    /**
     */
    NkVoid (NK_CALL *Close)(_Inout_ NkIFile *self);
    /**
     */
    NkErrorCode (NK_CALL *Seek)(_Inout_ NkIFile *self, _In_ NkSeekOrigin origin, _In_ NkOffset offset);
    /**
     */
    NkErrorCode (NK_CALL *Flush)(_Inout_ NkIFile *self);
};

/**
 */
NKOM_DECLARE_INTERFACE(NkIFilesystem) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(
        _Inout_  NkIFilesystem *self,
        _In_     NkUuid const *iId,
        _Outptr_ NkVoid **resPtr
    );
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIFilesystem *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIFilesystem *self);

    /**
     */
    char const *(NK_CALL *GetWorkingDirectory)(_Inout_ NkIFilesystem *self);
    /**
     */
    NkVoid (NK_CALL *SetWorkingDirectory)(_Inout_ NkIFilesystem *self, _In_z_ _Utf8_ char const *pathStr);

    /**
     */
    NkBoolean (NK_CALL *Exists)(_Inout_ NkIFilesystem *self, _In_z_ _Utf8_ char const *pathStr);
    /**
     */
    NkBoolean (NK_CALL *IsDirectory)(_Inout_ NkIFilesystem *self, _In_z_ _Utf8_ char const *pathStr);
    /**
     */
    NkBoolean (NK_CALL *IsFile)(_Inout_ NkIFilesystem *self, _In_z_ _Utf8_ char const *pathStr);

    /**
     */
    NkErrorCode (NK_CALL *Create)(
        _Inout_           NkIFilesystem *self,
        _In_              NkStreamType strType,
        _In_opt_z_ _Utf8_ char const *pathStr,
        _In_              NkStreamIOMode mode,
        _Outptr_mb_       NkIFile **resPtr
    );
    /**
     */
    NkVoid (NK_CALL *Remove)(_Inout_ NkIFilesystem *self, _In_z_ _Utf8_ char const *pathStr);
    /**
     */
    NkErrorCode (NK_CALL *Traverse)(
        _Inout_       NkIFilesystem *self,
        _In_z_ _Utf8_ char const *rootPath,
        _In_          NkBoolean isRecursive,
        _In_          NkDirectoryTraverseFn fnTrav,
        _Inout_opt_   NkVoid *extraParam
    );
};


