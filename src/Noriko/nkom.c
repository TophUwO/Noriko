/**************************************************************************
 * NkOM - Noriko Object Model                                             *
 *  lightweight cross-platform COM-like object model providing a base     *
 *  framework for extendable APIs                                         *
 *                                                                        *
 * (c) 2024 TophUwO <tophuwo01@gmail.com>. All rights reserved.           *
 *                                                                        *
 * The source code is licensed under the BSD (3-clause) License. Refer to *
 * the LICENSE file in the root directory of this project. If this file   *
 * is not present, visit                                                  *
 *     https://opensource.org/license/bsd-3-clause                        *
 **************************************************************************/

/**
 * \file  nkom.c
 * \brief implements the public API for the Noriko Object Model (NkOM), aka. the NkOM
 *        runtime
 * 
 * The Noriko Object Model (NkOM) is a cross-platform object model designed to imitate
 * what's informally known as the "lightweight COM-approach". COM (Component Object
 * Model) is a proprietary Microsoft technology for inter-process communication by the
 * means of a unified binary standard. Being designed with the "lightweight COM-approach"
 * in mind, NkOM has, unlike the original COM, no concept of \e Marshalling and
 * <em>Apartments</em>. The idea of NkOM is to provide a stable base framework for a
 * variety of object-oriented APIs so that they are cross-platform, easily extendable
 * without breaking old software, and easily embeddable within existing applications.
 */
#define NK_NAMESPACE "nk::om"

/* Windows includes */
#if (defined _WIN32)
    #pragma warning (disable: 4668) /* macro not defined; replacing with '0' */

    #include <windows.h>
#endif

/* Noriko includes */
#include <include/Noriko/noriko.h>
#include <include/Noriko/comp.h>

/* NkOM includes */
#include <include/Noriko/nkom.h>


/*
 * This section defines the interface- and class IDs for the interfaces and classes that
 * come shipped with the NkOM runtime.
 */
#pragma region Default Interface- and Class IDs
// { 00000000-0000-0000-0000-000000000000 }
static NKOM_DEFINE_IID(NkINull, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });
// { 00000000-0000-0000-0000-000000000000 }
static NKOM_DEFINE_CLSID(NkINull, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });

// { A2C6D745-F05F-4053-BB39-907BAAEA6E6D }
NKOM_DEFINE_IID(NkIBase, { 0xa2c6d745, 0xf05f, 0x4053, 0xbb39907baaea6e6d });
// { A30AAF35-B95D-422B-BB1B-B76A0D6F6EDE }
NKOM_DEFINE_IID(NkIInitializable, { 0xa30aaf35, 0xb95d, 0x422b, 0xbb1bb76a0d6f6ede });
// { 5FAC13C3-AD8E-4830-8FDC-998823AAFD44 }
NKOM_DEFINE_IID(NkIClassFactory, { 0x5fac13c3, 0xad8e, 0x4830, 0x8fdc998823aafd44 });

// { 00000000-0000-0000-0000-000000000000 }
NKOM_DEFINE_CLSID(NkIBase, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });
// { 00000000-0000-0000-0000-000000000000 }
NKOM_DEFINE_CLSID(NkIInitializable, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });
// { 00000000-0000-0000-0000-000000000000 }
NKOM_DEFINE_CLSID(NkIClassFactory, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });
#pragma endregion (Default Interface- and Class IDs)


/** \cond INTERNAL */
/**
 * \struct __NkOM_StaticContext
 * \brief  represents the global (process-wide) NkOM context
 */
NK_NATIVE typedef struct __NkOM_StaticContext {
    NkBoolean    m_isInitialized;  /**< whether or not NkOM is initialized */
    NkBoolean    m_isDebugEnabled; /**< whether or not integrated debugging facilities are enabled */

    NK_DECL_LOCK(m_clsRegLock);    /**< synchronization object for class registry */
    NkHashtable *mp_classReg;      /**< global class registry */
} __NkOM_StaticContext;

/**
 * \brief actual instance of the static global NkOM context 
 */
NK_INTERNAL __NkOM_StaticContext gl_NkOMContext;


/**
 */
NK_INTERNAL NkVoid NK_CALL __NkOM_DestroyClsFacFn(_Inout_opt_ NkHashtableKey *keyPtr, _Inout_opt_ NkVoid *valPtr) {
    NK_UNREFERENCED_PARAMETER(keyPtr);

    /*
     * No need to do anything to the key or the value itself; just decrement the
     * ref-count of the associated factory. This is to balance the calls to the
     * 'AddRef()' function when we installed the class factory.
     * Keys do not have to be destroyed since they are required to be pointers to static
     * memory as per NkOM conventions.
     */
    if (valPtr != NULL)
        ((NkIClassFactory *)valPtr)->VT->Release((NkIClassFactory *)valPtr);
}

/**
 * \brief  initializes the NkOM runtime
 * 
 * Before some of NkOM's public functions can be used safely, the NkOM runtime has to be
 * initialized. This happens through starting up the component once per process at
 * application startup.
 * 
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li The parameters passed to this function are immutable for the duration of
 *             the session.
 */
_Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(NkOM)(NkVoid) {
    if (InterlockedExchange8((CHAR volatile *)&gl_NkOMContext.m_isInitialized, NK_TRUE) == NK_FALSE) {
        /* Query application specification for debugging flag. */
        NkApplicationSpecification const *appSpecs = NkApplicationQuerySpecification();

        /* Initialize the static context. */
        gl_NkOMContext = (__NkOM_StaticContext){
            .m_isInitialized  = NK_TRUE,
            .m_isDebugEnabled = appSpecs->m_enableDbgTools,
            .mp_classReg      = NULL
        };

        /* Initialize synchronization primitive for the class registry. */
        NK_INITLOCK(gl_NkOMContext.m_clsRegLock);

        /* Initialize the class registry. */
        NkErrorCode errCode = NkHashtableCreate(
            &(NkHashtableProperties const){
            .m_structSize  = sizeof(NkHashtableProperties),
                .m_initCap     = 16,
                .m_keyType     = NkHtKeyTy_Uuid,
                .m_minCap      = 16,
                .m_maxCap      = UINT32_MAX,
                .mp_fnElemFree = (NkHashtableFreeFn)&__NkOM_DestroyClsFacFn
            },
            &gl_NkOMContext.mp_classReg
        );
        if (errCode != NkErr_Ok) {
            InterlockedAnd8((CHAR volatile *)&gl_NkOMContext.m_isInitialized, NK_FALSE);

            return errCode;
        }

        return NkErr_Ok;
    }

    return NkErr_NoOperation;
}

/**
 */
_Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(NkOM)(NkVoid) {
    if (InterlockedExchange8((CHAR volatile *)&gl_NkOMContext.m_isInitialized, NK_FALSE) == NK_TRUE) {
        /* Destroy synchonization primitive for the class registry. */
        NK_DESTROYLOCK(gl_NkOMContext.m_clsRegLock);

        /* Destroy the class registry. */
        NkHashtableDestroy(&gl_NkOMContext.mp_classReg);
        return NkErr_Ok;
    }

    return NkErr_NoOperation;
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkOMCreateInstance(
    _In_        NkUuid const *clsId,
    _Inout_opt_ NkIBase *ctrlInst,
    _In_        NkUuid const *iId,
    _Inout_opt_ NkVoid *initParam,
    _Outptr_    NkIBase **resPtr
) {
    NK_ASSERT(
        InterlockedOr8((CHAR volatile *)&gl_NkOMContext.m_isInitialized, NK_FALSE) == NK_TRUE,
        NkErr_ComponentState
    );
    NK_ASSERT(clsId != NULL, NkErr_InParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);
    *resPtr = NULL;

    /* Query the class factory required for instantiating the desired class. */
    NkIClassFactory *reqClsFac;
    NkErrorCode errCode = NkOMQueryFactoryForClass(clsId, &reqClsFac);
    if (errCode != NkErr_Ok)
        return errCode;
    /*
     * Instantiate the class. After this call, the newly-created object's ref-count must
     * exactly one higher than the ref-count that would trigger a destroy (usually that
     * would be 0).
     */
    NkIBase *tmpResPtr;
    errCode = reqClsFac->VT->CreateInstance(reqClsFac, clsId, ctrlInst, &tmpResPtr);
    /*
     * Since NkOMQueryFactoryForClass() increments the factory's ref-count, we must
     * decrement it after we are done with the factory.
     */
    reqClsFac->VT->Release(reqClsFac);
    if (errCode != NkErr_Ok)
        return errCode;

    /*
     * Check if the created class implements the NkIInitializable interface, and if yes,
     * run its initialize method. If this fails, it is not an error.
     */
    NkIInitializable *initRef;
    if (tmpResPtr->VT->QueryInterface(tmpResPtr, NKOM_IIDOF(NkIInitializable), (NkVoid **)&initRef) == NkErr_Ok) {
        /*
         * If initialization fails, decrement the object's ref-count. It must now be one.
         * In the next step, it will be decremented again, causing it to hit zero,
         * thereby destroying the instance. If it does not fail, it will simply decrement
         * the ref-count in the next step, only releasing the NkIInitializable instance.
         */
        if ((errCode = initRef->VT->Initialize(initRef, initParam)) != NkErr_Ok) {
            initRef->VT->Release(initRef);

            *resPtr = NULL;
        }

        /* QueryInterface() did increment ref-count. */
        tmpResPtr->VT->Release(tmpResPtr);
        if (errCode != NkErr_Ok)
            return errCode;
    }

    /* Finally, query the interface that we will use to communicate with the object. */
    if (!errCode && (errCode = tmpResPtr->VT->QueryInterface(tmpResPtr, iId, (NkVoid **)resPtr)) != NkErr_Ok) {
        /*
         * Interface is not implemented or something else went wrong. Ref-count not
         * increased; release once to destroy the object altogether.
         */
        tmpResPtr->VT->Release(tmpResPtr);
    
        *resPtr = NULL;
        return errCode;
    }

    /*
     * Reference count should now be 2 after having queried the desired interface.
     * Decrement by one to return an object that has a reference count of 1 and as such
     * can be destroyed by a single call to 'Release()'.
     */
#pragma warning (suppress: 6001) /* False alarm due to SAL not working properly with function pointers. */
    (*resPtr)->VT->Release(*resPtr);
    return errCode;
}

_Return_ok_ NkErrorCode NK_CALL NkOMQueryFactoryForClass(
    _In_     NkUuid const *clsidPtr,
    _Outptr_ NkIClassFactory **clsFacPtr
) {
    NK_ASSERT(
        InterlockedOr8((CHAR volatile *)&gl_NkOMContext.m_isInitialized, NK_FALSE) == NK_TRUE,
        NkErr_ComponentState
    );
    NK_ASSERT(clsidPtr != NULL, NkErr_InParameter);
    NK_ASSERT(clsFacPtr != NULL, NkErr_OutptrParameter);

    NK_LOCK(gl_NkOMContext.m_clsRegLock);
    NkErrorCode const errCode = NkHashtableAt(
        gl_NkOMContext.mp_classReg,
        &(NkHashtableKey){ .mp_ptrKey = (NkVoid *)clsidPtr },
        clsFacPtr
    );
    if (errCode == NkErr_Ok) {
        NK_UNLOCK(gl_NkOMContext.m_clsRegLock);

        /* Add a reference to the class factory. */
        (*clsFacPtr)->VT->AddRef(*clsFacPtr);
        return NkErr_Ok;
    }
    
    NK_UNLOCK(gl_NkOMContext.m_clsRegLock);
    return NkErr_ClassNotReg;
}

_Return_ok_ NkErrorCode NK_CALL NkOMInstallClassFactory(_Inout_ NkIClassFactory *clsFac) {
    NK_ASSERT(
        InterlockedOr8((CHAR volatile *)&gl_NkOMContext.m_isInitialized, NK_FALSE) == NK_TRUE,
        NkErr_ComponentState
    );
    NK_ASSERT(clsFac != NULL, NkErr_InOutParameter);

    NkErrorCode errCode;
    /*
     * Go through all of the instantiable classes and add a class entry of the given
     * class ID associated with the current factory.
     */
    NkUuid const **clsidArr = clsFac->VT->QueryInstantiableClasses(clsFac);
    for (NkSize i = 0; clsidArr[i] != NULL; i++) {
        /** \todo check if class already exists */
        NK_SYNCHRONIZED(gl_NkOMContext.m_clsRegLock,
            errCode = NkHashtableInsert(
                gl_NkOMContext.mp_classReg,
                &(NkHashtablePair){
                    .m_keyVal    = { .mp_ptrKey = (NkVoid *)clsidArr[i] },
                    .mp_valuePtr = clsFac
                }
            );
        );

        if (errCode != NkErr_Ok) {
            /* If we couldn't add all entries, roll-back the current action. */
            if (i > 0)
                NK_IGNORE_RETURN_VALUE(NkOMUninstallClassFactory(clsFac));

            return errCode;
        }

        clsFac->VT->AddRef(clsFac);
    }

    /* All good. */
    return NkErr_Ok;
}

_Return_ok_ NkErrorCode NK_CALL NkOMUninstallClassFactory(_Inout_ NkIClassFactory *clsFac) {
    NK_ASSERT(
        InterlockedOr8((CHAR volatile *)&gl_NkOMContext.m_isInitialized, NK_FALSE) == NK_TRUE,
        NkErr_ComponentState
    );
    NK_ASSERT(clsFac != NULL, NkErr_InOutParameter);

    /*
     * Find all classes that are associated with the current class factory and erase the
     * entries, decrementing the reference count of the factory for each one.
     */
    NkUuid const **clsidArr = clsFac->VT->QueryInstantiableClasses(clsFac);
    for (NkSize i = 0; clsidArr[i] != NULL; i++) {
        NK_SYNCHRONIZED(gl_NkOMContext.m_clsRegLock,
            NK_IGNORE_RETURN_VALUE(NkHashtableErase(gl_NkOMContext.mp_classReg, &(NkHashtableKey){
                .mp_uuidKey = (NkVoid *)clsidArr[i]
            }));
        );

        clsFac->VT->Release(clsFac);
    }

    /* All good. */
    clsFac->VT->Release(clsFac);
    return NkErr_Ok;
}


NkBoolean NK_CALL NkOMIsPureVirtual(_In_ NkUuid const *iId) {
    NK_ASSERT(iId != NULL, NkErr_InParameter);

    return NkUuidIsEqual(iId, NKOM_IIDOF(NkINull));
}

NkSize NK_CALL NkOMQueryImplementationIndex(
    _In_ NkOMImplementationInfo const *infos,
    _In_ NkUuid const *uuidRef
) {
    NK_ASSERT(infos != NULL, NkErr_InParameter);
    NK_ASSERT(uuidRef != NULL, NkErr_InParameter);

    NkSize currInd = 0;
    while (infos[currInd].mp_uuidRef != NULL) {
        if (NkUuidIsEqual(uuidRef, infos[currInd].mp_uuidRef) == TRUE)
            return currInd;

        ++currInd;
    }

    return SIZE_MAX;
}


/**
 */
NK_COMPONENT_DEFINE(NkOM) {
    .m_compUuid     = { 0xb63776e5, 0x5fa1, 0x4d54, 0x8d833994eab26cee },
    .mp_clsId       = NULL,
    .m_compIdent    = NK_MAKE_STRING_VIEW("Noriko Object Model (NkOM)"),
    .m_compFlags    = 0,
    .m_isNkOM       = NK_FALSE,

    .mp_fnQueryInst = NULL,
    .mp_fnStartup   = &NK_COMPONENT_STARTUPFN(NkOM),
    .mp_fnShutdown  = &NK_COMPONENT_SHUTDOWNFN(NkOM)
};


#undef NK_NAMESPACE


