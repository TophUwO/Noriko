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
 * \file  renderer.c
 * \brief implements some global renderer helper functions
 * 
 * In Noriko, a renderer can only exist in the context of a window, that is, the renderer
 * is managed not by the caller, but by the window. As such, the renderer can only be
 * accessed through the window. When the window is modified (e.g., when the window mode
 * is changed), changes to the client area are automatically propagated to the renderer.
 */
#define NK_NAMESPACE "nk::renderer"


/* Noriko includes */
#include <include/Noriko/renderer.h>
#include <include/Noriko/alloc.h>


/** \cond INTERNAL */
/**
 * \struct __NkInt_ClassImplEntry
 * \brief  represents an implementation entry for a specific NkOM class
 */
NK_NATIVE typedef struct __NkInt_ClassImplEntry {
    NkUuid                 const *mp_clsId;    /**< class ID of the implementation */
    NkOMImplementationInfo const *mp_implInfo; /**< implementation details */
} __NkInt_ClassImplEntry;
/** \endcond */


/* Define IID and CLSID of the generic NkIRenderer interface. */
// { B5CD4AFE-0227-4B56-9BAE-2241F3AE3126 }
NKOM_DEFINE_IID(NkIRenderer, { 0xb5cd4afe, 0x0227, 0x4b56, 0x9bae2241f3ae3126 });
// { 00000000-0000-0000-0000-000000000000 }
NKOM_DEFINE_CLSID(NkIRenderer, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });


/** \cond INTERNAL */
/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_RendererFactory_AddRef(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub because static object. */
    return 1;
}

/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_RendererFactory_Release(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub because static object. */
    return 1;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_RendererFactory_QueryInterface(
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

        __NkInt_RendererFactory_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 */
NK_INTERNAL NkUuid **NK_CALL __NkInt_RendererFactory_QueryInstantiableClasses(_Inout_ NkIClassFactory *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /**
     * \brief lists all classes instantiable by the current factory
     */
    NK_INTERNAL NkUuid const *gl_InstClasses[] = { NKOM_CLSIDOF(NkIGdiRenderer) };

    return (NkUuid **)gl_InstClasses;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_RendererFactory_CreateInstance(
    _Inout_     NkIClassFactory *self,
    _In_        NkUuid const *clsId,
    _Inout_opt_ NkIBase *ctrlInst,
    _Outptr_    NkIBase **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(clsId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);
    NK_UNREFERENCED_PARAMETER(self);
    NK_UNREFERENCED_PARAMETER(ctrlInst);

#if (defined NK_TARGET_WINDOWS)
    /**
     * \brief implementation details for the GDI-based renderer
     */
    NK_EXTERN NkOMImplementationInfo const __gl_GdiRdImplInfo__;
#endif

    /**
     */
    NK_INTERNAL __NkInt_ClassImplEntry const gl_InstClsTable[] = {
#if (defined NK_TARGET_WINDOWS)
        { NKOM_CLSIDOF(NkIGdiRenderer), &__gl_GdiRdImplInfo__ },
#endif

        { NULL }
    };
    NK_INTERNAL NkSize const gl_InstClsTblSize = NK_ARRAYSIZE(gl_InstClsTable);

    /* Retrieve the entry for the given class ID. */
    NkOMImplementationInfo const *implInfo = NULL;
    for (NkSize i = 0; i < gl_InstClsTblSize; i++)
        if (NkUuidIsEqual(gl_InstClsTable[i].mp_clsId, clsId)) {
            implInfo = gl_InstClsTable[i].mp_implInfo;

            break;
        }
    if (implInfo == NULL) {
        /* Class entry not found; class is not implemented. */
        *resPtr = NULL;

        return NkErr_UnknownClass;
    }

    /* Instantiate the class. */
    NkErrorCode eCode = NkGPAlloc(NK_MAKE_ALLOCATION_CONTEXT(), implInfo->m_structSize, 0, NK_TRUE, (NkVoid **)resPtr);
    if (eCode != NkErr_Ok) {
        *resPtr = NULL;

        return eCode;
    }
    (*resPtr)->VT = (struct __NkIBase_VTable__ *)implInfo->mp_vtabPtr;

    /* All went well. */
    (*resPtr)->VT->AddRef(*resPtr);
    return NkErr_Ok;
}


/**
 * \brief define the actual instance of the renderer factory
 * 
 * \par Remarks
 *   It's generally unsafe to directly declare a variable of our interface type and mark
 *   it as <tt>static const</tt> while passing it as normal to functions of the NkOM
 *   runtime and other components. However, since no member function of the class factory
 *   references any instance-specific memory or state, the instance itself virtually only
 *   consists of a VTable and as such is never modified and may, therefore, be declared
 *   <tt>static const</tt>. It's technically unsafe but given the circumstances, we're
 *   actually fine. Crazy world, isn't it?
 */
NK_INTERNAL NkIClassFactory const gl_RendererFactory = {
    .VT = &(struct __NkIClassFactory_VTable__){
        .QueryInterface           = &__NkInt_RendererFactory_QueryInterface,
        .AddRef                   = &__NkInt_RendererFactory_AddRef,
        .Release                  = &__NkInt_RendererFactory_Release,
        .QueryInstantiableClasses = &__NkInt_RendererFactory_QueryInstantiableClasses,
        .CreateInstance           = &__NkInt_RendererFactory_CreateInstance
    }
};
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkRendererStartup(NkVoid) {
    /* See comment regarding *gl_RendererFactory* above. */
    return NkOMInstallClassFactory((NkIClassFactory *)&gl_RendererFactory);
}

_Return_ok_ NkErrorCode NK_CALL NkRendererShutdown(NkVoid) {
    /* See comment regarding *gl_RendererFactory* above. */
    return NkOMUninstallClassFactory((NkIClassFactory *)&gl_RendererFactory);
}


NkSize NK_CALL NkRendererQueryAvailablePlatformApis(_Outptr_ NkRendererApi const **resPtr) {
    NK_ASSERT(resPtr != NULL, NkErr_OutParameter);

    /**
     * \brief list of available renderer APIs on the windows platform 
     */
    NK_INTERNAL NkRendererApi const gl_AvailRdApis[] = { NkRdApi_Win32GDI };

    *resPtr = gl_AvailRdApis;
    return NK_ARRAYSIZE(gl_AvailRdApis);
}

NkRendererApi NK_CALL NkRendererQueryDefaultPlatformApi(NkVoid) {
    return NkRdApi_Win32GDI;
}

NkUuid const *NK_CALL NkRendererQueryCLSIDFromApi(_In_ NkRendererApi apiIdent) {
    switch (apiIdent) {
#if (defined NK_TARGET_WINDOWS)
        case NkRdApi_Win32GDI: return NKOM_CLSIDOF(NkIGdiRenderer);
#endif /* NK_TARGET_WINDOWS */
    }

    return NULL;
}


NkBoolean NK_CALL NkRendererCompareRectangles(_In_ NkRectF const *r1Ptr, _In_ NkRectF const *r2Ptr) {
    return r1Ptr->m_width == r2Ptr->m_width && r1Ptr->m_height == r2Ptr->m_height;
}


#undef NK_NAMESPACE


