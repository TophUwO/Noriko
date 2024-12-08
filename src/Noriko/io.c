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
 * \file  io.c
 * \brief implements platform-independent functionality related to the I/O services
 */
#define NK_NAMESPACE "nk::io"


/* stdlib includes */
#include <stdio.h>
#include <sys/stat.h>

/* Noriko includes */
#include <include/Noriko/io.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/comp.h>


/** \cond INTERNAL */
/**
 */
NK_NATIVE typedef struct __NkInt_File {
    NKOM_IMPLEMENTS(NkIFile);

    NkOMRefCount   m_refCount; /**< reference count */
    NkStreamIOMode m_strMode;  /**< stream mode */
    FILE          *mp_fileDsc; /**< file descriptor */
} __NkInt_File;


/* Define IIDs and CLSIDs for the interfaces and classes implemented here. */
// { 7268E0CB-376C-4019-964A-8550FB3D8D9C }
NKOM_DEFINE_IID(NkIStream, { 0x7268e0cb, 0x376c, 0x4019, 0x964a8550fb3d8d9c });
// { F3DD1340-2CAC-4D00-937E-23A58056E13F }
NKOM_DEFINE_IID(NkIFile, { 0xf3dd1340, 0x2cac, 0x4d00, 0x937e23a58056e13f });
// { B191AB16-42BC-4AAC-959F-BD95EF8A0461 }
NKOM_DEFINE_IID(NkIFilesystem, { 0xb191ab16, 0x42bc, 0x4aac, 0x959fbd95ef8a0461 });
// { 00000000-0000-0000-0000-000000000000 }
NKOM_DEFINE_CLSID(NkIStream, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });
// { 5D954D25-55FF-4976-8AFA-A9DDCA54F573 }
NKOM_DEFINE_CLSID(NkIFile, { 0x5d954d25, 0x55ff, 0x4976, 0x8afaa9ddca54f573 });
// { 7323AD44-F5F4-4D08-ACBB-EF71EF4B3695 }
NKOM_DEFINE_CLSID(NkIFilesystem, { 0x7323ad44, 0xf5f4, 0x4d08, 0xacbbef71ef4b3695 });


/**
 * \ingroup VirtFn 
 * \brief   retrieves the platform-dependent filesystem instance
 * \return  pointer to the instance
 * \note    The reference count of the returned instance is incremented. Please call
 *          <tt>Release()</tt> on the returned instance after you are done with it.
 */
NK_EXTERN NkIBase *NK_CALL __NkVirt_Filesys_QueryInstance(NkVoid);


/**
 */
NK_INTERNAL _Return_true_ NkBoolean __NkInt_File_CompileIOModeIdent(
    _In_             NkStreamIOMode mode,
    _Out_writes_(16) char resPtr[16]
) {
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    /* Handle basic I/O mode. */
    memset(resPtr, 0, 16);
    switch (mode & (NkStrMd_Read | NkStrMd_Write | NkStrMd_Append)) {
        case NkStrMd_Read:   *resPtr++ = 'r'; break;
        case NkStrMd_Write:  *resPtr++ = 'w'; break;
        case NkStrMd_Append: *resPtr++ = 'a'; break;
        default:
            return NK_FALSE;
    }
    if (mode & NkStrMd_Read && mode & NkStrMd_Write) *resPtr++ = '+';
    if (mode & NkStrMd_Binary)                       *resPtr++ = 'b';
    
    /*
     * Handle "new" (yeah, looking at you MSVC ^^) C11 feature that allows a file opened
     * with the 'w' mode to fail if the file does already exist.
     */
    if (mode & NkStrMd_MustNotExist) {
        *resPtr++ = 'x';

        if ((mode & NkStrMd_Write) == 0)
            return NK_FALSE;
    }

    /* All good. */
    return NK_TRUE;
}

/**
 */
NK_INTERNAL NK_INLINE int NK_CALL __NkInt_File_MapFromSeekOrigin(_In_ NkSeekOrigin ori) {
    switch (ori) {
        case NkSeekOri_Set: return SEEK_SET;
        case NkSeekOri_Cur: return SEEK_CUR;
        case NkSeekOri_End: return SEEK_END;
    }

    return INT_MAX;
}


/**
 * \brief implements <tt>NkIFile::AddRef()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_File_AddRef(_Inout_ NkIFile *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    return ++((__NkInt_File *)self)->m_refCount;
}

/**
 * \brief implements <tt>NkIFile::Release()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_File_Release(_Inout_ NkIFile *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    __NkInt_File *actSelf = (__NkInt_File *)self;

    if (--actSelf->m_refCount <= 0) {
        /* Reference count is 0; destroy. Close first if needed. */
        self->VT->Close(self);

        NkPoolFree(self);
        return 0;
    }

    /* Return new reference count. */
    return actSelf->m_refCount;
}

/**
 * \brief implements <tt>NkIFile::QueryInterface()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_File_QueryInterface(
    _Inout_  NkIFile *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    NK_INTERNAL NkOMImplementationInfo const gl_c_ImplInfos[] = {
        { NKOM_IIDOF(NkIBase)   },
        { NKOM_IIDOF(NkIStream) },
        { NKOM_IIDOF(NkIFile)   },
        { NULL                  }
    };
    if (NkOMQueryImplementationIndex(gl_c_ImplInfos, iId) != SIZE_MAX) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_File_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 * \brief implements <tt>NkIFile::GetStat()</tt>
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_File_GetStat(_Inout_ NkIFile *self, _Out_ NkStreamStat *statPtr) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(statPtr != NULL, NkErr_OutParameter);

    __NkInt_File *actSelf = (__NkInt_File *)self;

    /*
     * If there is no file open, zero the result array and return a corresponding error
     * code.
     */
    if (actSelf->mp_fileDsc == NULL) {
        memset(statPtr, 0, sizeof *statPtr);

        return NkErr_ObjectState;
    }

    /* Retrieve stat info. */
    struct _stat64 fStat;
    if (_fstat64(_fileno(actSelf->mp_fileDsc), &fStat) != 0) {
        memset(statPtr, 0, sizeof *statPtr);

        return NkErr_OpenFile;
    }

    /* Populate structure with info. */
    *statPtr = (NkStreamStat){
        .m_structSize = sizeof *statPtr,
        .m_type       = NkStrTy_DiskFile,
        .m_ioMode     = actSelf->m_strMode,
        .m_crTime     = (NkUint64)fStat.st_ctime,
        .m_mdTime     = (NkUint64)fStat.st_mtime,
        .m_aTime      = (NkUint64)fStat.st_atime,
        .m_size       = (NkSize)fStat.st_size,
        .m_currOff    = (NkOffset)_ftelli64(actSelf->mp_fileDsc)
    };
    return NkErr_Ok;
}

/**
 * \brief implements <tt>NkIFile::Read()</tt>
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_File_Read(
    _Inout_      NkIFile *self,
    _In_         NkSize s,
    _O_bytes_(s) NkVoid *bufPtr,
    _Out_        NkSize *br
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(s > 0, NkErr_InParameter);
    NK_ASSERT(bufPtr != NULL, NkErr_OutParameter);
    NK_ASSERT(br != NULL, NkErr_OutParameter);

    __NkInt_File *actSelf = (__NkInt_File *)self;

    /* If no file is open or we cannot read from it, do nothing and return. */
    if (actSelf->mp_fileDsc == NULL || (actSelf->m_strMode & NkStrMd_Read) == 0)
        return actSelf->mp_fileDsc == NULL ? NkErr_ObjectState : NkErr_InvStreamMode;

    /*
     * Read the block of bytes from the file. Currently, we only support binary streams.
     */
    *br = fread_s(bufPtr, s, 1, s, actSelf->mp_fileDsc);
    if (*br ^ s) {
        /* Not all bytes could be read; either file not large enough or I/O error. */
        return NkErr_ErrorDuringDiskIO;
    }

    /* All good. */
    return NkErr_Ok;
}

/**
 * \brief implements <tt>NkIFile::Write()</tt>
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_File_Write(
    _Inout_      NkIFile *self,
    _In_         NkSize s,
    _I_bytes_(s) NkVoid const *bufPtr,
    _Out_        NkSize *bw
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(s > 0, NkErr_InParameter);
    NK_ASSERT(bufPtr != NULL, NkErr_OutParameter);
    NK_ASSERT(bw != NULL, NkErr_OutParameter);

    __NkInt_File *actSelf = (__NkInt_File *)self;

    /* If no file is open or we cannot write to it, do nothing and return. */
    if (actSelf->mp_fileDsc == NULL || (actSelf->m_strMode & NkStrMd_Write) == 0)
        return actSelf->mp_fileDsc == NULL ? NkErr_ObjectState : NkErr_InvStreamMode;

    /*
     * Write to the file; if the file was opened with the 'append' mode, this will
     * automatically append to the end of the file. 
     */
    *bw = fwrite(bufPtr, 1, s, actSelf->mp_fileDsc);
    if (*bw ^ s) {
        /*
         * There was an error writing the requested byte count to the file. Possibly no
         * more space on disk or the device was removed during the writing.
         */
        return NkErr_ErrorDuringDiskIO;
    }

    /* All good. */
    return NkErr_Ok;
}

/**
 * \brief implements <tt>NkIFile::Open()</tt>
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_File_Open(
    _Inout_       NkIFile *self,
    _In_z_ _Utf8_ char const *pathStr,
    _In_          NkStreamIOMode mode
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(pathStr != NULL && *pathStr ^ '\0', NkErr_InParameter);

    __NkInt_File *actSelf = (__NkInt_File *)self;

    /*
     * If we currently already have a file open, we cannot open another one with the
     * current handle.
     */
    if (actSelf->mp_fileDsc != NULL)
        return NkErr_ObjectState;

    NkErrorCode errCode = NkErr_Ok;
    /*
     * If we are supposed to open a temporary file, open it and ignore all other flags.
     * The file name is implementation-defined.
     */
    if (mode & NkStrMd_TempFile) {
        /* Open the temporary file. */
        if (tmpfile_s(&actSelf->mp_fileDsc) != 0) {
            errCode = NkErr_OpenFile;

            goto lbl_ONERROR;
        }

        /* All good. */
        actSelf->m_strMode = NkStrMd_TempFile;
        return NkErr_Ok;
    }

    /* "Compile" the open mode identifier from the numeric I/O mode. */
    char oModeId[16] = { 0 };
    if (!__NkInt_File_CompileIOModeIdent(mode, oModeId)) {
        errCode = NkErr_InvStreamMode;

        goto lbl_ONERROR;
    }

    /* Open the file. */
    if (fopen_s(&actSelf->mp_fileDsc, pathStr, oModeId) != 0) {
        errCode = NkErr_OpenFile;

        goto lbl_ONERROR;
    }
    actSelf->m_strMode = mode;

    /* All good. */
    return NkErr_Ok;

lbl_ONERROR:
    actSelf->mp_fileDsc = NULL;
    actSelf->m_strMode  = NkStrMd_Unknown;

    return errCode;
}

/**
 * \brief implements <tt>NkIFile::Close()</tt> 
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_File_Close(_Inout_ NkIFile *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    __NkInt_File *actSelf = (__NkInt_File *)self;

    /* Close the file if this handle represents an open file handle. */
    if (actSelf->mp_fileDsc != NULL) {
        fclose(actSelf->mp_fileDsc);

        actSelf->mp_fileDsc = NULL;
        actSelf->m_strMode  = NkStrMd_Unknown;
    }
}

/**
 * \brief implements <tt>NkIFile::Seek()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_File_Seek(
    _Inout_ NkIFile *self,
    _In_    NkSeekOrigin origin,
    _In_    NkOffset offset
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(origin >= NkSeekOri_Set && origin < __NkSeekOri_Count__, NkErr_InParameter);

    __NkInt_File *actSelf = (__NkInt_File *)self;

    if (actSelf->mp_fileDsc != NULL) {
        /* Convert the Noriko seek origin to the stdc seek origin. */
        int const sOri = __NkInt_File_MapFromSeekOrigin(origin);
        if (sOri == INT_MAX)
            return NkErr_InvSeekOrigin;

        /* Move the stream indicator. */
        if (_fseeki64(actSelf->mp_fileDsc, (long long)offset, sOri) != 0)
            return NkErr_StreamSeek;
        return NkErr_Ok;
    }

    /* No file open. */
    return NkErr_ObjectState;
}

/**
 * \brief implements <tt>NkIFile::Flush()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_File_Flush(_Inout_ NkIFile *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    __NkInt_File *actSelf = (__NkInt_File *)self;

    if (actSelf->mp_fileDsc != NULL) {
        if (fflush(actSelf->mp_fileDsc) != 0)
            return NkErr_StreamFlush;

        return NkErr_Ok;
    }

    /* No file open. */
    return NkErr_ObjectState;
}


/**
 * \brief global VTable for the default implementation of the NkIFile interface 
 */
NKOM_DEFINE_VTABLE(NkIFile) {
    .QueryInterface = &__NkInt_File_QueryInterface,
    .AddRef         = &__NkInt_File_AddRef,
    .Release        = &__NkInt_File_Release,
    .GetStat        = &__NkInt_File_GetStat,
    .Read           = &__NkInt_File_Read,
    .Write          = &__NkInt_File_Write,
    .Open           = &__NkInt_File_Open,
    .Close          = &__NkInt_File_Close,
    .Seek           = &__NkInt_File_Seek,
    .Flush          = &__NkInt_File_Flush
};


/**
 * \brief implements <tt>NkIClassFactory::AddRef()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_IoFac_AddRef(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 * \brief implements <tt>NkIClassFactory::Release()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_IoFac_Release(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 * \brief implements <tt>NkIClassFactory::QueryInterface()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_IoFac_QueryInterface(
    _Inout_  NkIClassFactory *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    if (NkUuidIsEqual(iId, NKOM_IIDOF(NkIBase)) || NkUuidIsEqual(iId, NKOM_IIDOF(NkIClassFactory))) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_IoFac_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    return NkErr_InterfaceNotImpl;
}

/**
 * \brief implements <tt>NkIClassFactory::QueryInstantiableClasses()</tt>
 */
NK_INTERNAL NkUuid const **NK_CALL __NkInt_IoFac_QueryInstantiableClasses(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /**
     * \brief lists all classes that this factory can instantiate 
     */
    NK_INTERNAL NkUuid const *gl_c_ImplCls[] = {
        NKOM_CLSIDOF(NkIFile),
        NULL
    };

    return gl_c_ImplCls;
}

/**
 * \brief implements <tt>NkIClassFactory::CreateInstance()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_IoFac_CreateInstance(
    _Inout_     NkIClassFactory *self,
    _In_        NkUuid const *clsId,
    _Inout_opt_ NkIBase *ctrlInst,
    _Outptr_    NkIBase **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(clsId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    /**
     * \brief list all classes instantiable by this factory 
     */
NK_DISABLE_WARNING(NK_WARN_DIFFERENT_CONST_QUALIFIERS,
    NK_INTERNAL NkOMImplementationInfo const gl_c_ImplInfos[] = {
        { NKOM_CLSIDOF(NkIFile), sizeof(__NkInt_File), NK_FALSE, &NKOM_VTABLEOF(NkIFile) },
        { NULL                                                                           }
    };
);
    
    NkSize implIndex;
    if ((implIndex = NkOMQueryImplementationIndex(gl_c_ImplInfos, clsId)) == SIZE_MAX)
        return NkErr_UnknownClass;
    /* Get implementation entry. */
    NkOMImplementationInfo const *iInfo = &gl_c_ImplInfos[implIndex];
    
    /* Instantiate the class. */
    NkErrorCode eCode = NkPoolAlloc(NK_MAKE_ALLOCATION_CONTEXT(), (NkUint32)iInfo->m_structSize, 1, (NkVoid **)resPtr);
    if (eCode != NkErr_Ok)
        return eCode;
    /* Initialize instance memory. */
    memset((NkVoid *)*resPtr, 0, iInfo->m_structSize);
    (*resPtr)->VT = (struct __NkIBase_VTable__ *)iInfo->mp_vtabPtr;
    
    /* All went well. */
    (*resPtr)->VT->AddRef(*resPtr);
    return NkErr_Ok;
}


/**
 * \brief represents the actual instance of the basic stream factory which is void of
 *        any internal state (see <tt>gl_c_RendererFactory</tt>) 
 */
NK_INTERNAL NkIClassFactory const gl_c_IoFactory = {
    .VT = &(struct __NkIClassFactory_VTable__) {
        .AddRef                   = &__NkInt_IoFac_AddRef,
        .Release                  = &__NkInt_IoFac_Release,
        .QueryInterface           = &__NkInt_IoFac_QueryInterface,
        .QueryInstantiableClasses = &__NkInt_IoFac_QueryInstantiableClasses,
        .CreateInstance           = &__NkInt_IoFac_CreateInstance
    }
};


/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(IoSrv)(NkVoid) {
    return NkOMInstallClassFactory((NkIClassFactory *)&gl_c_IoFactory);
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(IoSrv)(NkVoid) {
    return NkOMUninstallClassFactory((NkIClassFactory *)&gl_c_IoFactory);
}


/**
 * \brief info for the I/O services component 
 */
NK_COMPONENT_DEFINE(IoSrv) {
    .m_compUuid     = { 0x32c74849, 0x8c97, 0x41d5, 0x88db7cc8d75dad61 },
    .mp_clsId       = NKOM_CLSIDOF(NkIFilesystem),
    .m_compIdent    = NK_MAKE_STRING_VIEW("I/O services"),
    .m_compFlags    = 0,
    .m_isNkOM       = NK_TRUE,

    .mp_fnQueryInst = &__NkVirt_Filesys_QueryInstance,
    .mp_fnStartup   = &NK_COMPONENT_STARTUPFN(IoSrv),
    .mp_fnShutdown  = &NK_COMPONENT_SHUTDOWNFN(IoSrv)
};
/** \endcond */


#undef NK_NAMESPACE


