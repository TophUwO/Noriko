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
 * \file  winfilesys.c
 * \brief implements the file system manager for the windows platform
 */
#define NK_NAMESPACE "nk::filesys"


/* Windows includes */
#include <shlwapi.h>

/* stdlib includes */
#include <sys/stat.h>

/* Noriko includes */
#include <include/Noriko/io.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/path.h>
#include <include/Noriko/alloc.h>


/** \cond INTERNAL */
/**
 * \brief implements <tt>NkIFilesystem::AddRef()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WinFilesys_AddRef(_Inout_ NkIFilesystem *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 * \brief implements <tt>NkIFilesystem::Release()</tt> 
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WinFilesys_Release(_Inout_ NkIFilesystem *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 * \brief implements <tt>NkIFilesystem::QueryInterface()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WinFilesys_QueryInterface(
    _Inout_  NkIFilesystem *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    if (NkUuidIsEqual(iId, NKOM_IIDOF(NkIBase)) || NkUuidIsEqual(iId, NKOM_IIDOF(NkIFilesystem))) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_WinFilesys_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface is not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 * \brief implements <tt>NkIFilesystem::GetWorkingDirectory()</tt> 
 */
NK_INTERNAL char const *NK_CALL __NkInt_WinFilesys_GetWorkingDirectory(_Inout_ NkIFilesystem *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Allocate a new string for the current working directory. */
    DWORD reqSize = GetCurrentDirectoryA(0, NULL);
    if (reqSize != 0) {
        char *retBuf;
        NkErrorCode errCode = NkGPAlloc(NK_MAKE_ALLOCATION_CONTEXT(), (NkSize)reqSize, 0, NK_FALSE, &retBuf);
        if (errCode != NkErr_Ok)
            return NULL;

        /* Copy the current directory in the buffer. */
        GetCurrentDirectoryA(reqSize, (LPSTR)retBuf);
        return retBuf;
    }

    /* Could not retrieve current directory. */
    return NULL;
}

/**
 * \brief implements <tt>NkIFilesystem::SetWorkingDirectory()</tt>
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_WinFilesys_SetWorkingDirectory(
    _Inout_       NkIFilesystem *self,
    _In_z_ _Utf8_ char const *pathStr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(pathStr != NULL && *pathStr ^ '\0', NkErr_InParameter);
    NK_UNREFERENCED_PARAMETER(self);

    SetCurrentDirectoryA((LPCSTR)pathStr);
}

/**
 * \brief implements <tt>NkIFilesystem::Exists()</tt> 
 */
NK_INTERNAL NkBoolean NK_CALL __NkInt_WinFilesys_Exists(_Inout_ NkIFilesystem *self, _In_z_ _Utf8_ char const *pathStr) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(pathStr != NULL, NkErr_InParameter);
    NK_UNREFERENCED_PARAMETER(self);

    return PathFileExistsA((LPCSTR)pathStr) == TRUE;
}

/**
 * \brief implements <tt>NkIFilesystem::IsDirectory()</tt> 
 */
NK_INTERNAL NkBoolean NK_CALL __NkInt_WinFilesys_IsDirectory(
    _Inout_       NkIFilesystem *self,
    _In_z_ _Utf8_ char const *pathStr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(pathStr != NULL && *pathStr ^ '\0', NkErr_InParameter);
    NK_UNREFERENCED_PARAMETER(self);

    return GetFileAttributesA((LPCSTR)pathStr) & FILE_ATTRIBUTE_DIRECTORY;
}

/**
 * \brief implements <tt>NkIFilesystem::IsFile()</tt> 
 */
NK_INTERNAL NkBoolean NK_CALL __NkInt_WinFilesys_IsFile(
    _Inout_       NkIFilesystem *self,
    _In_z_ _Utf8_ char const *pathStr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(pathStr != NULL && *pathStr ^ '\0', NkErr_InParameter);
    NK_UNREFERENCED_PARAMETER(self);

    return GetFileAttributesA((LPCSTR)pathStr) & FILE_ATTRIBUTE_NORMAL;
}

/**
 * \brief implements <tt>NkIFilesystem::Create()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WinFilesys_Create(
    _Inout_           NkIFilesystem *self,
    _In_              NkStreamType strType,
    _In_opt_z_ _Utf8_ char const *pathStr,
    _In_              NkStreamIOMode mode,
    _Outptr_mb_       NkIFile **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(strType > NkStrTy_Unknown && strType < __NkStrTy_Count__, NkErr_InParameter);
    NK_ASSERT(mode > NkStrMd_Unknown && mode < __NkStrMd_Count__, NkErr_InParameter);

    switch (strType) {
        case NkStrTy_DiskFile: {
            NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

            /* Create the file stream object. */
            NkErrorCode errCode = NkOMCreateInstance(NKOM_CLSIDOF(NkIFile), NULL, NKOM_IIDOF(NkIFile), NULL, resPtr);
            if (errCode != NkErr_Ok)
                return errCode;
            NkIFile *fileObj = *resPtr;

            /* Open the file path if the path is not NULL or empty. */
            if (pathStr != NULL && *pathStr ^ '\0') {
                errCode = fileObj->VT->Open(fileObj, pathStr, mode);

                if (errCode != NkErr_Ok) {
                    /* Release the file object; this will destroy it. */
                    fileObj->VT->Release(fileObj);

                    *resPtr = NULL;
                    return NkErr_OpenFile;
                }
            }

            /* All good. */
            return errCode;
        }
        case NkStrTy_Directory: {
            NK_ASSERT(pathStr != NULL, NkErr_InParameter);

            BOOL res = CreateDirectoryA((LPCSTR)pathStr, NULL);

            (NkVoid)(resPtr != NULL ? (*resPtr = NULL) : NULL);
            return res == TRUE ? NkErr_Ok : NkErr_CreateDirectory;
        }
    }

    /* Unknown stream type. */
    (NkVoid)(resPtr != NULL ? (*resPtr = NULL) : NULL);
    return NkErr_InvalidStreamType;
}

/**
 * \brief implements <tt>NkIFilesystem::Remove()</tt> 
 */
NK_INTERNAL NkVoid NK_CALL __NkInt_WinFilesys_Remove(_Inout_ NkIFilesystem *self, _In_z_ _Utf8_ char const *pathStr) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(pathStr != NULL && *pathStr ^ '\0', NkErr_InParameter);

    /* Check if file even exists. */
    if (!self->VT->Exists(self, pathStr))
        return;
    /* Get type of entity. */
    struct stat fStat;
    if (stat(pathStr, &fStat) != 0)
        return;

    /* Carry out the action. */
    switch (fStat.st_mode) {
        case _S_IFREG: DeleteFileA((LPCSTR)pathStr);      break;
        case _S_IFDIR: RemoveDirectoryA((LPCSTR)pathStr); break;
    }
}

/**
 * \brief implements <tt>NkIFilesystem::Traverse()</tt> 
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WinFilesys_Traverse(
    _Inout_       NkIFilesystem *self,
    _In_z_ _Utf8_ char const *rootPath,
    _In_          NkBoolean isRecursive,
    _In_          NkDirectoryTraverseFn fnTrav,
    _Inout_opt_   NkVoid *extraParam
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(rootPath != NULL && *rootPath ^ '\0', NkErr_InParameter);
    NK_ASSERT(fnTrav != NULL, NkErr_CallbackParameter);
    NK_UNREFERENCED_PARAMETER(self);
    NK_UNREFERENCED_PARAMETER(rootPath);
    NK_UNREFERENCED_PARAMETER(isRecursive);
    NK_UNREFERENCED_PARAMETER(fnTrav);
    NK_UNREFERENCED_PARAMETER(extraParam);

    return NkErr_NotImplemented;
}


/**
 * \brief static filesystem tools instance; got no state so we can simply have it be
 *        constant
 */
NK_INTERNAL NkIFilesystem const gl_c_WinFilesysTools = {
    .VT = &(struct __NkIFilesystem_VTable__ const){
        .QueryInterface      = &__NkInt_WinFilesys_QueryInterface,
        .AddRef              = &__NkInt_WinFilesys_AddRef,
        .Release             = &__NkInt_WinFilesys_Release,
        .GetWorkingDirectory = &__NkInt_WinFilesys_GetWorkingDirectory,
        .SetWorkingDirectory = &__NkInt_WinFilesys_SetWorkingDirectory,
        .Exists              = &__NkInt_WinFilesys_Exists,
        .IsDirectory         = &__NkInt_WinFilesys_IsDirectory,
        .IsFile              = &__NkInt_WinFilesys_IsFile,
        .Create              = &__NkInt_WinFilesys_Create,
        .Remove              = &__NkInt_WinFilesys_Remove,
        .Traverse            = &__NkInt_WinFilesys_Traverse
    }
};


NkIBase *NK_CALL __NkVirt_Filesys_QueryInstance(NkVoid) {
    /* Got no state, so no need to increment the instance's reference count. */
    return (NkIBase *)&gl_c_WinFilesysTools;
}
/** \endcond */


#undef NK_NAMESPACE


