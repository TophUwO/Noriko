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
 * \file  nkom.h
 * \brief defines the public API for the Noriko Object Model (NkOM)
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


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>
#include <include/Noriko/error.h>


/**
 * \typedef NkOMRefCount
 * \brief   represents the type that is used for holding the reference count of any NkOM
 *          class instance
 */
NK_NATIVE typedef int NkOMRefCount;


/**
 * \struct NkOMImplementationInfo
 * \brief  an auxiliary data-structure which can be used to define implementation details
 *         of interface and/or classes specific to the current instance; using this may
 *         reduce spaghetti code for complex objects
 * \see    NkOMQueryImplementationIndex
 * \note   Use the \c NkOMQueryImplementationIndex() function to dynamically retrieve the
 *         info associated with the given IID/CLSID.
 */
NK_NATIVE typedef struct NkOMImplementationInfo {
    NkUuid    *mp_uuidRef;       /**< IID/CLSID */

    /* The following are for use as class infos only. */
    NkSize     m_structSize;     /**< size of the internal structure */
    NkBoolean  m_isAggSupported; /**< whether or not aggregation is supported */
} NkOMImplementationInfo;


/**
 * \def   NKOM_IIDOF
 * \brief can be used to retrieve the interface ID (<tt>IID</tt>) of the given plain
 *        interface name
 * \param ifaceName interface name
 * \see   NKOM_CLSIDOF
 * \note  Use this macro when you need to use the interface identifier. The class
 *        identifier (<tt>CLSID</tt>) is not required to be (and also should not be)
 *        identical to the interface identifier (<tt>IID</tt>).
 */
#define NKOM_IIDOF(ifaceName) ((NkUuid const *)&__##ifaceName##_IID__)
/**
 * \def   NKOM_CLSIDOF
 * \brief can be used to retrieve the class ID (<tt>CLSID</tt>) of the given plain class
 *        name
 * \param className interface name
 * \see   NKOM_IIDOF
 * \note  Use this macro when you need to use a class identifier. The interface
 *        identifier (<tt>IID</tt>) is not required to be (and also should not be)
 *        identical to the class identifier (<tt>CLSID</tt>).
 */
#define NKOM_CLSIDOF(className) ((NkUuid const *)&__##className##_CLSID__)
/**
 * \def   NKOM_OFFSETOF(self, ifaceName)
 * \brief calculates the offset of a specific interface's VTable in a struct definition
 * \param self name of the structure
 * \param ifaceName name of the interface
 * \note  Use the name of the interface that was used when the interface was declared,
 *        i.e., when using <tt>NKOM_DECLARE_INTERFACE()</tt>.
 */
#define NKOM_OFFSETOF(self, ifaceName) ((NkSize)((char *)&((self *)0)->ifaceName##_Iface - (char *)0))
/**
 * \def   NKOM_VTABLEOF(ifaceName)
 * \brief evaluates to the canonical name of the VTable structure for the given interface
 *        name
 * \param ifaceName name of the interface
 * \note  Use the name of the interface that was used when the interface was declared,
 *        i.e., when using <tt>NKOM_DECLARE_INTERFACE()</tt>.
 */
#define NKOM_VTABLEOF(ifaceName) __gl_##ifaceName##_VTable__
/**
 * \def   NKOM_DECLARE_INTERFACE
 * \brief auxiliary macro used to define new interfaces
 *
 * An interface is a data-structure that consists only of function pointers. Use this
 * macro to insert the interface declaration as well as necessary meta-information on the
 * new interface.
 * When creating new interfaces, a few rules must be abided by:
 *  \li Every derived interface must contain all methods exposed by <tt>NkIBase</tt>.
 *  \li Every derived interface's first three methods must exactly reflect those found in
 *      <tt>NkIBase</tt>. That means, the order of declaration, the name, and the
 *      prototypes must be identical barring the pointer type of the \c self parameters.
 *  \li Two separate interfaces can have the same name and declaration, but must differ
        in their respective interface IDs.
 *  \li The interface's ID (or <tt>IID</tt> for short) must be non-zero for it to be
 *      valid.
 */
#define NKOM_DECLARE_INTERFACE(ifaceName)                                      \
    NK_NATIVE typedef struct ifaceName ifaceName;                              \
    NK_NATIVE NK_API NkUuid const __##ifaceName##_IID__;                       \
    NK_NATIVE NK_API NkUuid const __##ifaceName##_CLSID__;                     \
    NK_NATIVE struct ifaceName { struct __##ifaceName##_VTable__ const *VT; }; \
    NK_NATIVE struct __##ifaceName##_VTable__
/**
 * \def   NKOM_DEFINE_VTABLE(ifaceName)
 * \brief convenience macro for defining the static internal VTable for an interface
 *        of which the name is provided
 * \param ifaceName name of the interface
 */
#define NKOM_DEFINE_VTABLE(ifaceName) static struct __##ifaceName##_VTable__ const NKOM_VTABLEOF(ifaceName) =
/**
 * \def   NKOM_IMPLEMENTS(ifaceName)
 * \brief auxiliary macro which inserts a pointer to the VTable of the given interface
 *        into a structure definition
 * 
 * This macro is useful when creating classes that implement interfaces.
 */
#define NKOM_IMPLEMENTS(ifaceName) struct ifaceName ifaceName##_Iface
/**
 * \def   NKOM_EXTENDS(className)
 * \brief auxiliary macro which inserts a pointer to an object by class name
 * 
 * This macro is useful when extending existing class instances or when you want a
 * sub-object to be dynamically instantiated in an NkOM object implementing multiple
 * interfaces.
 */
#define NKOM_EXTENDS(className) struct clsName *##className##_Obj
/**
 * \def   NKOM_DEFINE_IID(ifaceName, ...)
 * \brief sets the constant interface ID for the given interface
 * \param ifaceName name of the interface that will be associated with the given UUID
 * 
 * \par Remarks
 *   The following is an example use of <tt>NKOM_DEFINE_IID</tt>:
 *   \code{.c}
 *   // in myinterface.h
 *   #include <nkom.h>
 * 
 *   
 *   NKOM_DECLARE_INTERFACE(NkIMyInterface) {
 *       ...
 *   };
 * 
 *   ...
 * 
 *   // in myinterface.c
 *   #include <nkom.h>
 * 
 * 
 *   NKOM_DEFINE_IID(NkIMyInterface, { 0x1bb7e7bb, 0xcdaf, 0x47c0, 0x8dfca82c09bf30bb });
 *   \endcode
 */
#define NKOM_DEFINE_IID(ifaceName, ...) NK_NATIVE NkUuid const __##ifaceName##_IID__ = __VA_ARGS__
/**
 * \def   NKOM_DEFINE_CLSID(className, ...)
 * \brief sets the constant class ID for a specific implementation of an interface
 * \param className name of the class that will be associated with the given UUID
 *
 * \par Remarks
 *   It is not strictly necessary for an interface to have a default implementation. As
 *   specific implementations are identified by their class ID (CLSID), to signify that
 *   no default implementation exists, set the CLSID of the interface itself to all
 *   zeroes like so:
 *   \code{.c}
 *   NKOM_DEFINE_CLSID(NkIMyInterface, { 0x00000000, 0x0000, 0x0000, 0x0000000000000000 });
 *   \endcode
 *   If there actually is no default implementation but the CLSID is not all zeroes, then
 *   that poses a violation of NkOM conventions. If the CLSID is not all zeroes, then
 *   there must be a default implementation queryable through the class factory
 *   instantiated by the module the interface is declared in.
 * 
 * \note  \li Class names are usually identical to their interface names.
 * \note  \li If you want to define multiple classes based on exactly the same interface
 *            and you want these classes to be instantiable from the outside, then you
 *            must generate and expose respective class IDs in the public API of your
 *            module. In such a case, it may be helpful to mark the interface itself as
 *            <em>pure virtual</em> by settings its own CLSID to all zeroes in order to
 *            avoid confusion if none of the classes can be considered the default
 *            implementation of that interface. This way, all classes semantically appear
 *            as <em>specializations</em>.
 */
#define NKOM_DEFINE_CLSID(className, ...) NK_NATIVE NkUuid const __##className##_CLSID__ = __VA_ARGS__


/**
 * \interface NkIBase
 * \brief     represents the most fundamental interface that any NkOM class must
 *            implement in order to guarantee consistency across the entire architecture
 * 
 * The \c NkIBase interface implements the most basic functions required by NkOM. It
 * covers nothing but the basic lifetime management functionality. All NkOM objects must
 * directly or indirectly derive from this interface. However, there is no hard
 * requirement as to whether or not the class actually implements reference-counting.
 */
NKOM_DECLARE_INTERFACE(NkIBase) {
    /**
     * \brief   exposes other interfaces implemented by the current NkOM object
     * 
     * In general, classes and their instances can implement multiple interfaces. The
     * behavior of this function must reflect that by never failing when provided with an
     * interface ID of which the interface is implemented by the current instance.
     * 
     * \param   [in,out] self pointer to the current NkIBase instance
     * \param   [in] iidPtr pointer to the interface identifier of the interface that is
     *               to be queried
     * \param   [in] resPtr pointer to a variable that will receive the pointer to the
     *               queried interface
     * \return  \c NkOM_Ok on success, non-zero on failure<br>
     *          Besides \c NkOM_Ok signifying success, the function may return the
     *          following error codes signifying specific error conditions:
     * 
     * <table>
     *  <tr>
     *   <th>Error Code Identifier</th>
     *   <th>Error Condition</th>
     *  </tr>
     *  <tr>
     *   <td>\c NkErr_UnknownInterface</td>
     *   <td>The given IID does not identify an interface known to the NkOM runtime.</td>
     *  </tr>
     *  <tr>
     *   <td>\c NkErr_InterfaceNotImpl</td>
     *   <td>
     *    The current class instance does not implement the interface of which the
     *    IID is specified.
     *   </td>
     *  </tr>
     *  <tr>
     *   <td>\c NkErr_InvalidParameter</td>
     *   <td>
     *    If the function validates the arguments, then it can return this error code as
     *    an alternative to simply leaving the behavior undefined.
     *   </td>
     *  </tr>
     * </table>
     * 
     * \par Remarks
     *   If the function succeeds, the function returns \c NkErr_Ok and \c *resPtr will
     *   be initialized to a non-<tt>NULL</tt> value that points to the queried interface
     *   VTable.<br>
     *   If the function fails, the function returns non-zero and \c *resPtr will be
     *   initialized with <tt>NULL</tt>.<br>
     *   The function must return \c NkOM_Ok for all interfaces in the derivation tree.
     *   That is, if the class implements an interface \c NkIList which derives from two
     *   interfaces called <tt>NkICollection</tt> and <tt>NkIOrdered</tt>, which both
     *   derive directly from <tt>NkIBase</tt>, then \c QueryInterface() must succeed for
     *   all queries that query one of the aforementioned interfaces.<br>
     *   The behavior when queried for interfaces must be fixed. This means that if the
     *   first query for an interface of a special type (expressed through IID) succeeds,
     *   all subsequent queries for the same interface must also succeed. Similarly, if
     *   it fails, all subsequent queries for the same interface must fail.<br>
     *   Aside from the basic requirements outlined above, \c QueryInterface() also needs
     *   to satisfy the requirements of an <em>equivalency relation</em>.
     *   In essence,
     *   <ol>
     *    <li>
     *     It must be <em>reflexive</em>. This means that if you query an object for
     *     interface an <tt>A</tt>, this succeeds, and use the queried interface to query
     *     for <tt>A</tt> again, then the second query must succeed as well.
     *    </li>
     *    <li>
     *     It must be <em>symmetric</em>. This means if you query an object for an
     *     interface <tt>B</tt> using the object's <tt>A</tt>, this query succeeds, and
     *     you use the queried <tt>B</tt> interface to query for <tt>A</tt>, then this
     *     second query must succeed, too.
     *    </li>
     *    <li>
     *     It must be <em>transitive</em>. That is, if you successfully query for an
     *     interface <tt>A</tt>, use <tt>A</tt> to successfully query for <tt>B</tt>, and
     *     use <tt>B</tt> to successfully query for <tt>C</tt>, then using <tt>A</tt> to
     *     query for <tt>C</tt> must also succeed.
     *    </li>
     *   </ol>
     * 
     * \note    If this function succeeds, it must call the queried interface's
     *          <tt>AddRef()</tt> method.
     * \warning Passing invalid parameters, that is, \c NULL for any of the pointer
     *          parameters generally invokes undefined behavior unless otherwise
     *          specified in the documentation for a concrete interface.
     */
    NkErrorCode (NK_CALL *QueryInterface)(_Inout_ NkIBase *self, _In_ NkUuid const *iId, _Outptr_ NkVoid **resPtr);
    /**
     * \brief   increments the reference count of the current instance
     * 
     * The lifetime NkOM objects is controlled via reference-counting. The reference
     * count is a numeric value that holds the number of outstanding references to the
     * current object. If this value is greater than zero, this means we have at least
     * one piece of code that still holds a reference to the current instance; we cannot
     * destroy the object in such a case. If the reference count reaches zero in response
     * to a call to <tt>Release()</tt>, the object can be safely destroyed.
     * 
     * \param   [in,out] self pointer to the current NkIBase instance
     * \return  the new (incremented) reference count
     * 
     * \par Remarks
     *   <tt>AddRef()</tt> and <tt>Release()</tt> are not required to return accurate
     *   values. This enables the user to create static NkOM objects that do not
     *   implement the reference-counting mechanism.
     * 
     * \warning The return value of this function should not be relied on for any
     *          meaningful client code. Use it for auxiliary purposes like debugging.
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIBase *self);
    /**
     * \brief   decrements the reference count of the current instance
     * 
     * The lifetime NkOM objects is controlled via reference-counting. The reference
     * count is a numeric value that holds the number of outstanding references to the
     * current object. If this value is greater than zero, this means we have at least
     * one piece of code that still holds a reference to the current instance; we cannot
     * destroy the object in such a case. If the reference count reaches zero in response
     * to a call to <tt>Release()</tt>, the object can be safely destroyed.<br>
     * Once the reference count reaches zero, the object must be immediately destroyed
     * before <tt>Release()</tt> returns. When the reference count reaches zero and
     * <tt>Release()</tt> returns, the pointer to the instance will be invalid.
     * 
     * \param   [in,out] self pointer to the current NkIBase instance
     * \return  the new (decremented) reference count
     * 
     * \par Remarks
     *   <tt>AddRef()</tt> and <tt>Release()</tt> are not required to return accurate
     *   values. This enables the user to create static NkOM objects that do not
     *   implement the reference-counting mechanism.
     * 
     * \warning The return value of this function should not be relied on for any
     *          meaningful client code. Use it for auxiliary purposes like debugging.
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIBase *self);
};

/**
 * \interface NkIInitializable
 * \brief     represents an NkOM interface that adds basic RAII capabilities to an object
 * 
 * RAII stands for <em>resource acquisition is initialization</em> which means that the
 * object must be initialized when the resource (i.e., the memory) is allocated. This
 * happens through the use of a <em>constructor</em>. When the user is done with the
 * resource, the <em>destructor</em> is called. This happens semi-automatically when
 * <tt>Release()</tt> is called. The addition of an <tt>Initialize()</tt> method
 * completes the RAII support. It has to be noted that this isn't "proper" RAII since
 * that would involve the resource being automatically uninitialized and destroyed when
 * the resource goes out of scope. Since there is no automatic destruction of resources
 * upon going out of scope in C, this is pretty much as good as we can do without more
 * advanced technologies.
 * 
 * \note      This interface inherits from <tt>NkIBase</tt>.
 */
NKOM_DECLARE_INTERFACE(NkIInitializable) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt>
     */
    NkErrorCode (NK_CALL *QueryInterface)(
        _Inout_  NkIInitializable *self,
        _In_     NkUuid const *iId,
        _Outptr_ NkVoid **resPtr
    );
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIInitializable *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIInitializable *self);

    /**
     * \brief  initializes the object directly after it is created
     * 
     * The purpose of this function is to represent a "constructor", that is, a function
     * that is called directly after the resource is allocated. This can be seen as a
     * semi-real version of RAII.
     * 
     * \param  [in] self pointer to the current \c NkIInitializable instance
     * \param  [in,out] initParam pointer to a caller-supplied data-structure that can be
     *                  used to supply the constructor with parameters that may be used
     *                  for initialization purposes; may be \c NULL
     * \return \c NkOM_ErrOk on success, non-zero on failure
     * 
     * \par Remarks
     *   When an object is created using the <tt>NkOMCreateInstance()</tt> function or
     *   <tt>NkIClassFactory::CreateInstance()</tt> function, the newly-created object is
     *   queried for the <tt>NkIInitializable</tt> interface to check whether or not the
     *   instance supports initialization. If it does, the <tt>Initialize()</tt> method
     *   is invoked with the supplied <tt>initParam</tt> parameter.<br>
     *   If the call to <tt>Initialize()</tt> succeeds, then the instance creation
     *   succeeded. If <tt>Initialize()</tt> returns non-zero, then the initialization
     *   is considered failed and the object instantiation will be aborted. The memory
     *   allocated for that instance is freed automatically by the NkOM runtime.
     * 
     * \warning \li Do not change (that is, call <tt>AddRef()</tt> or <tt>Release()</tt>)
     *              the reference count of the instance from within this function.
     * \warning \li If your implementation uses the <tt>initParam</tt> parameter, you
     *              must be able to properly handle <tt>initParam</tt> being
     *              <tt>NULL</tt>.
     */
    NkErrorCode (NK_CALL *Initialize)(_Inout_ NkIInitializable *self, _Inout_opt_ NkVoid *initParam);
};

/**
 * \interface NkIClassFactory
 * \brief     represents an NkOM object that is capable of instantiating other NkOM
 *            objects
 * \note      This interface inherits from <tt>NkIBase</tt>.
 * \warning   When you register your implementation as a global object so that the object
 *            is usable by <tt>NkOMCreateInstance()</tt>, it should make sure that either
 * <ul>
 *  <li>all functions are reentrant, or</li>
 *  <li>
 *   proper synchronization routines are implemented to protect factory's internal data.
 *  </li>
 * </ul>
 */
NKOM_DECLARE_INTERFACE(NkIClassFactory) {
    /**
     * \brief reimplements <tt>NkIBase::QueryInterface()</tt> 
     */
    NkErrorCode (NK_CALL *QueryInterface)(
        _Inout_  NkIClassFactory *self,
        _In_     NkUuid const *iId,
        _Outptr_ NkVoid **resPtr
    );
    /**
     * \brief reimplements <tt>NkIBase::AddRef()</tt> 
     */
    NkOMRefCount (NK_CALL *AddRef)(_Inout_ NkIClassFactory *self);
    /**
     * \brief reimplements <tt>NkIBase::Release()</tt> 
     */
    NkOMRefCount (NK_CALL *Release)(_Inout_ NkIClassFactory *self);

    /**
     * \brief  retrieves a list of class IDs (CLSIDs) of which the current factory
     *         instance is capable of instantiating
     * \param  [in,out] self pointer to the current \c NkIClassFactory instance
     * \param  [out] clsidArr pointer to a variable that will receive the pointer to an
     *              array of CLSIDs
     * \return \c NkOM_ErrOk on success, non-zero on failure<br>
     *         Aside from the error code mentioned above, the function can also return
     *         the following error codes:
     * 
     * <table>
     *  <tr>
     *   <th>Error Code Identifier</th>
     *   <th>Error Condition</th>
     *  </tr>
     *  <tr>
     *   <td>\c NkOM_ErrInvalidParameter</td>
     *   <td>
     *    This is an optional return value that may be returned if you decide to validate
     *    the parameters.
     *   </td>
     *  </tr>
     *  <tr>
     *   <td>\c NkOM_ErrOutOfMemory</td>
     *   <td>
     *    If your implementation allocates dynamic memory, it can return this error code
     *    if the memory allocation failed.
     *   </td>
     *  </tr>
     * </table>
     * 
     * \note   Although not strictly required, it is recommended that the internal CLSID
     *         array be static and constant. If memory is dynamically allocated, the
     *         documentation of your implementation must make that clear.
     */
    NkUuid const **(NK_CALL *QueryInstantiableClasses)(_Inout_ NkIClassFactory *self);
    /**
     * \brief  creates a new instance of the given class identified by the CLSID,
     *         optionally initializing it
     * \param  [in,out] self pointer to the current \c NkIClassFactory instance
     * \param  [in] clsId the class ID of which an instance is to be created
     * \param  [in] ctrlInst pointer to the controlling instance if the object supports
     *              aggregation
     * \param  [out] resPtr pointer to a variable that will receive the pointer to the
     *               freshly-created instance
     * \return \c NkOM_ErrOk on success, non-zero on failure<br>
     *         Aside from the error code mentioned above, the function can also return
     *         the following error codes:
     * 
     * <table>
     *  <tr>
     *   <th>Error Code Identifier</th>
     *   <th>Error Condition</th>
     *  </tr>
     *  <tr>
     *   <td>\c NkOM_ErrPureVirtual</td>
     *   <td>
     *    This error code should be returned if the interface is marked as
     *    <em>pure virtual</em>, that is, if the CLSID of that class is all zeroes.
     *   </td>
     *  </tr>
     *  <tr>
     *   <td>\c NkOM_ErrOutOfMemory</td>
     *   <td>
     *    This error should be returned if the factory could not allocate the memory
     *    required to instantiate the requested class.
     *   </td>
     *  </tr>
     *  <tr>
     *   <td>\c NkOM_ErrUnknownInterface</td>
     *   <td>Should be returned if the interface is unknown to the current factory.</td>
     *  </tr>
     *  <tr>
     *   <td>\c NkOM_ErrInvalidParameter</td>
     *   <td>
     *    This is an optional return value that may be returned if you decide to validate
     *    the parameters.
     *   </td>
     *  </tr>
     *  <tr>
     *   <td>\c NkOM_ErrNoAggregation</td>
     *   <td>
     *    If the class that is to be instantiated does not support aggregation, but \c
     *    ctrlInst is not <tt>NULL</tt>, then the function should return this value.
     *   </td>
     *  </tr>
     * </table>
     * 
     * \par Remarks
     *   The purpose of this function is to allocate the memory required for the object's
     *   internal state. This can be done by either implementing an own allocator, or by
     *   using the default allocator of the NkOM runtime, the latter of which is
     *   recommended.<br>
     *   To use the global NkOM allocator, the following can be done:
     *   \code{.c}
     *   // Allocate memory for new object.
     *   NkIBase *newObj;
     *   NkErrorCode errCode = NkOMAllocateMemory(sizeof(NkIBaseImpl), (void **)&newObj);
     *   if (errCode == NkOM_ErrOk) {
     *       // Allocation succeeded. Run other initialization if needed.
     *       ...
     *   }
     *   \endcode
     *   The above example is recommended since it automatically releases the allocator 
     *   before <tt>NkOMAllocateMemory()</tt> returns. Furthermore, using
     *   <tt>NkOMAllocateMemory()</tt> allows a global <tt>NkIAllocatorObserver</tt> object to
     *   log the allocation or add debug data to the allocation block.<br>
     *   The following example queries the global allocator object for direct use, but
     *   this does not trigger any installed instances of <tt>NkIAllocatorObserver</tt>.
     *   \code{.c}
     *   // Query the global NkOM allocator object.
     *   NkIAllocator *glAlloc;
     *   NkErrorCode errCode = NkOMQueryGlobalObject(NkOM_CatAllocator, (NkIBase **)&glAlloc);
     *   if (errCode == NkOM_ErrOk) {
     *       // Use the queried allocator.
     *       ...
     *       // Release the allocator when you are done with it.
     *       glAlloc->VT->Release(glAlloc);
     *   }
     *   \endcode
     *   Whatever allocation strategy is used, there must be consistency in API usage.
     *   For example, if you use the global <tt>NkOMAllocateMemory()</tt> function, you
     *   must use the <tt>NkOMFreeMemory()</tt> function to deallocate it. Similarly,
     *   if you use the allocator directly, you must use its functions directly. Mixing
     *   APIs results in undefined behavior.
     */
    NkErrorCode (NK_CALL *CreateInstance)(
        _Inout_     NkIClassFactory *self,
        _In_        NkUuid const *clsId,
        _Inout_opt_ NkIBase *ctrlInst,
        _Outptr_    NkIBase **resPtr
    );
};


/**
 * \brief  initializes the NkOM runtime
 * 
 * Before some of NkOM's public functions can be used safely, the NkOM runtime has to be
 * initialized. This happens through calling <tt>NkOMInitialize()</tt> once per process
 * at startup.
 * 
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li The parameters passed to this function are immutable for the duration of
 *             the session. A session is marked by a pair of
 *             <tt>NkOMInitialize()</tt>/<tt>NkOMUninitialize()</tt>. A new session can
 *             be started by calling <tt>NkOMInitialize()</tt> again after having called
 *             <tt>NkOMUninitialize()</tt>.
 * \note   \li Calling this function more than once without having called
 *             <tt>NkOMUninitialize()</tt> in between results in a no-op.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkOMInitialize(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkOMUninitialize(NkVoid);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkOMCreateInstance(
    _In_        NkUuid const *clsId,
    _Inout_opt_ NkIBase *ctrlInst,
    _In_        NkUuid const *iId,
    _Inout_opt_ NkVoid *initParam,
    _Outptr_    NkIBase **resPtr
);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkOMQueryFactoryForClass(
    _In_     NkUuid const *clsidPtr,
    _Outptr_ NkIClassFactory **clsFacPtr
);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkOMInstallClassFactory(_Inout_ NkIClassFactory *clsFac);
/**
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkOMUninstallClassFactory(_Inout_ NkIClassFactory *clsFac);

/**
 */
NK_NATIVE NK_API NkBoolean NK_CALL NkOMIsPureVirtual(_In_ NkUuid const *iId);
/**
 */
NK_NATIVE NK_API NkSize NK_CALL NkOMQueryImplementationIndex(
    _In_ NkOMImplementationInfo const *infos,
    _In_ NkUuid const *uuidRef
);


/**
 * \page NkOMIntro NkOM introduction
 * 
 * The <em>Noriko Object Model</em> (NkOM) is a cross-platform object model designed to
 * imitate what's informally known as the "lightweight COM-approach". COM (Component
 * Object Model) is a proprietary Microsoft technology for inter-process communication by
 * the means of a unified binary standard. Being designed with the "lightweight
 * COM-approach" in mind, NkOM has, unlike the original COM, no concept of \e Marshalling
 * and <em>Apartments</em>. The idea of NkOM is to provide a stable base framework for a
 * variety of object-oriented APIs so that they are cross-platform, easily extendable
 * without breaking old software, and easily embeddable within existing applications. As
 * such, NkOM focuses on the binary standard aspect of COM.
 * 
 * \section NkOMConcepts Basic Concepts and Terminology
 * The most fundamental concept of NkOM is an <em>interface</em>. An interface is in the
 * context of the C programming language nothing more than a <tt>struct</tt> holding
 * nothing but function pointers. <em>classes</em> are implementations of one or more
 * <em>interfaces</em>, and <em>instances</em> are actual objects created from
 * <em>classes</em>.
 * \subsection NkOMConceptsIface Interfaces
 *  As previously mentioned, an interface is a collection of function pointers. In order
 *  to uniquely identify interfaces across module borders, each interface has a globally
 *  unique identifier (called IID when used in this context) associated with it. The most
 *  basic interface is <tt>NkIBase</tt>, implementing exactly three methods:
 *   <ol>
 *    <li>
 *     <tt>QueryInterface()</tt> for querying of other interfaces the current instance
 *     may implement
 *    </li>
 *    <li>
 *     <tt>AddRef()</tt> and <tt>Release()</tt> for reference-counting
 *    </li>
 *   </ol>
 *  Every interface must have these three methods, and by that, every class needs to
 *  provide custom implementations for these three methods.
 * 
 * \subsection NkOMConceptsClasses Classes
 *  Classes, also called <em>implementations</em>, provide implementations for the
 *  functions exposed by interfaces. An NkOM class can implement multiple interfaces,
 *  all of which must be queryable by <tt>QueryInterface()</tt>. The NkOM runtime comes
 *  with a couple of interfaces predefined, some of which have default implementations.
 *  Just like interfaces, classes are also uniquely identified by a UUID/GUID, called a
 *  class ID (CLSID) in this context. It abides by the same rules as the interface ID
 *  (IID).
 * 
 * \subsection NkOMConceptsInst Instances
 *  Lastly, instances are concrete objects that are created from classes using class
 *  factories or static instantiation. Unlike interfaces and classes, instances are not
 *  uniquely identified by an ID. To be able to dynamically instantiate classes, a class
 *  factory object has to be created (either statically or also dynamically by another
 *  class factory) which implements the necessary routines to create the object. Objects
 *  are default-initialized upon instantiation; custom initialization can be carried out
 *  by implementing the pre-defined \c NkIInitializable interface.
 */


