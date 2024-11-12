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
 * \file  util.h
 * \brief auxiliary utility functions and **static** data-structures used by many of
 *        Noriko's components
 * \todo  Add notes for every function regarding undefined behavior.
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>


/**
 * \def   NK_ESC(...)
 * \brief escapes all parameters passed into this macro at compile-time
 */
#define NK_ESC(...)                  #__VA_ARGS__
/**
 * \def   NK_MESC(x)
 * \brief expands a macro parameter and escapes its expanded value
 * \param x macro expression to escape
 */
#define NK_MESC(x)                   NK_ESC(x)

/**
 * \def   NK_DEFINE_PROTOTYPE(t, a, s)
 * \brief defines a semi-opaque prototype which is a type that is statically instantiable
 *        while exposing the least amount of implementation details
 * \param t type name
 * \param a alignment requirement; either a number, a type, or <tt>NK_ALIGNOF(NkAlign*)</tt>
 * \param s size of the type, in bytes
 */
#define NK_DEFINE_PROTOTYPE(t, a, s)                                                                       \
    static_assert(s != 0, "Invalid size for type \"" #t "\": " #s);                                        \
    static_assert(a != 0 && (a & (a - 1)) == 0, "Invalid alignment requirement for type \"" #t "\": " #a); \
    NK_NATIVE typedef struct t { alignas(a) NkByte __placeholder__[s]; } t;
/**
 * \def   NK_VERIFY_TYPE(pub, priv)
 * \brief verifies size and alignment between public prototype defined with
 *        <tt>NK_DEFINE_PROTOTYPE()</tt> and the implementation structure, denoted by
 *        \c priv
 * \param pub name of the public prototype
 * \param priv name of the implementation struct
 */
#define NK_VERIFY_TYPE(pub, priv)                                                                          \
    static_assert(                                                                                         \
        sizeof(pub) == sizeof(priv) && alignof(pub) == alignof(priv),                                      \
        "Size and/or alignment mismatch between public type \"" #pub "\" and private type \"" #priv "\". " \
        "Check definitions."                                                                               \
    )
/**
 * \def   NK_VERIFY_LUT(lut, e, s)
 * \brief validates the size of a lookup table against a controlling enum and an element
 *        count
 * \param lut name of the lookup table symbol
 * \param e name of the controlling enum
 * \param s must-size of the LUT, should normally be the <tt>\_\_\*\_Count\_\_</tt>
 *          member of \c e
 */
#define NK_VERIFY_LUT(lut, e, s)                                                                            \
    static_assert(                                                                                          \
        NK_ARRAYSIZE(lut) == s,                                                                             \
        "Mismatch between LUT \"" #lut "\" and controlling enum \"" #e "\" (s: " #s "). Check definitions." \
    )

/**
 * \def   NK_INRANGE_INCL(x, lo, hi)
 * \brief checks whether <tt>lo <= x <= hi</tt> is *true*
 * \param x numeric value to compare against *lo* and *hi*
 * \param lo lower bound of the comparison
 * \param hi higher bound of the comparison
 */
#define NK_INRANGE_INCL(x, lo, hi)   (NkBoolean)((((hi) - (x)) * ((x) - (lo))) >= 0)
/**
 * \def   NK_INRANGE_EXCL(x, lo, hi)
 * \brief checks whether <tt>lo < x < hi</tt> is *true*
 * \param x numeric value to compare against *lo* and *hi*
 * \param lo lower bound of the comparison
 * \param hi higher bound of the comparison
 */
#define NK_INRANGE_EXCL(x, lo, hi)   (NkBoolean)(NK_INRANGE_INCL(x, (lo) + 1, (hi) - 1))
/**
 * \def   NK_MIN(x, y)
 * \brief expands to the maximum of the numeric values \c x and \c y
 * \param x first value
 * \param y second value
 */
#define NK_MIN(x, y)                 (((x) > (y)) ? (y) : (x))
/**
 * \def   NK_MAX(x, y)
 * \brief expands to the minimum of the numeric values \c x and \c y
 * \param x first value
 * \param y second value
 */
#define NK_MAX(x, y)                 (((x) > (y)) ? (x) : (y))
/**
 * \def   NK_CLAMP(elem, lo, hi)
 * \brief clamps \c elem to the range <tt>[lo, hi]</tt> (inclusive)
 * \param elem element to clamp
 * \param lo lower boundary
 * \param hi higher boundary
 * \note  This macro only works with numeric values.
 */
#define NK_CLAMP(elem, lo, hi)       (NK_MAX(NK_MIN(elem, hi), lo))
/**
 * \def   NK_ISBITFLAG(n)
 * \brief tests if \c n is a <tt>bitflag</tt>, that is, a <em>power of two</em> or
 *        <em>zero</em>
 * \param n value to test (numeric)
 */
#define NK_ISBITFLAG(n)              ((NkBoolean)((n & (n - 1)) == 0))

/**
 * \def   NK_ARRAYSIZE(arr)
 * \brief calculates the size in elements of a static compile-time array
 * \param arr array to calculate size of
 */
#define NK_ARRAYSIZE(arr)            (NkSize)(sizeof (arr) / (sizeof *(arr)))


/**
 * \def   NK_OFFSETOF(st, m)
 * \brief calculates the offset (in bytes) of the given member relative to the address of
 *        the first byte of its parent structure
 * \param st structure type name
 * \param m name of member variable of which the offset is to be calculated
 * \see   https://en.wikipedia.org/wiki/Offsetof
 */
#define NK_OFFSETOF(st, m)           ((NkSize)((char *)&((st *)0)->m - (char *)0))
/**
 * \def   NK_SIZEOF(st, m)
 * \brief returns the size in bytes of the member \c m of data-structure <tt>st</tt>, i.e.
 *        <tt>sizeof st.m</tt>
 * \param st type name of the data-structure
 * \param m identifier of the member
 */
#define NK_SIZEOF(st, m)             ((NkSize)(sizeof ((st *)0)->m))
/**
 * \def   NK_ALIGNOF(e)
 * \brief returns the <em>natural</em> alignment requirement of the given type
 * \param e name of the type of which the alignment requirement is to be retrieved
 * \note  This macro returns the <em>minimum alignment requirement</em>; \c e can also
 *        be validly aligned more stringently (that is, with greater alignment
 *        requirement)
 */
#define NK_ALIGNOF(e)                ((NkSize)(alignof(e)))


/**
 * \def   NK_TRUE
 * \brief use *NK_TRUE* instead of just plain *1*
 */
#define NK_TRUE                      ((NkBoolean)(1))
/**
 * \def   NK_FALSE
 * \brief use *NK_FALSE* instead of just plain *0*
 */
#define NK_FALSE                     ((NkBoolean)(0))


/**
 * \def   NK_UUIDLEN
 * \brief minimum number of bytes required to encode a UUID as a string, incl.
 *        <tt>NUL</tt>-terminator
 */
#define NK_UUIDLEN                   ((NkSize)(37))


/**
 * \def   NK_UNREFERENCED_PARAMETER(p)
 * \brief suppress "unreferenced parameter 'x'" warnings
 * \param p name of the parameter
 */
#define NK_UNREFERENCED_PARAMETER(p) ((NkVoid)(p))
/**
 * \def   NK_IGNORE_RETURN_VALUE(e)
 * \brief macro to wrap function calls in of which the return value is intentionally not
 *        examined
 * \param e expression to ignore return value of
 */
#define NK_IGNORE_RETURN_VALUE(e)    NK_UNREFERENCED_PARAMETER(e)


#if (defined NK_TARGET_MULTITHREADED)
    /**
     * \def   NK_DECL_LOCK(id)
     * \brief declares a new mutex variable
     * \param id variable name of the mutex
     * \note  If multithreading is not enabled, this macro expands to a no-op.
     */
    #define NK_DECL_LOCK(id)            mtx_t id
    /**
     * \def   NK_LOCK(lock)
     * \brief locks the given mutex
     * \param lock mutex object (**not** pointer-to)
     * \note  If multithreading is not enabled, this macro expands to a no-op.
     */
    #define NK_LOCK(lock)               mtx_lock(&(lock))
    /**
     * \def   NK_UNLOCK(lock)
     * \brief unlocks the given mutex
     * \param lock mutex object (**not** pointer-to)
     * \note  If multithreading is not enabled, this macro expands to a no-op.
     */
    #define NK_UNLOCK(lock)             mtx_unlock(&(lock))
    /**
     * \def   NK_INITLOCK(lock)
     * \brief initializes the given mutex
     * \param lock mutex object (**not** pointer-to)
     * \note  If multithreading is not enabled, this macro expands to a no-op.
     */
    #define NK_INITLOCK(lock)           mtx_init(&(lock), mtx_plain)
    /**
     * \def   NK_DESTROYLOCK(lock)
     * \brief destroys the given mutex
     * \param lock mutex object (**not** pointer-to)
     * \note  If multithreading is not enabled, this macro expands to a no-op.
     */
    #define NK_DESTROYLOCK(lock)        mtx_destroy(&(lock))
    /**
     * \def   NK_SYNCHRONIZED(lock, body)
     * \brief provides a simple macro to wrap critical section code into an environment
     *        that automatically locks and unlocks the given lock
     * \param lock lock object (non-pointer)
     * \param body code block that is to be executed between lock and unlock
     * \note  If \c NK_TARGET_MULTITHREADED is not defined, this macro simply expands to
     *        <tt>{ body }</tt>.
     */
    #define NK_SYNCHRONIZED(lock, body) NK_LOCK(lock); do { body; } while (0); NK_UNLOCK(lock);
#else
    #define NK_DECL_LOCK(id)
    #define NK_LOCK(lock)               (0)
    #define NK_UNLOCK(lock)             (0)
    #define NK_INITLOCK(lock)           (0)
    #define NK_DESTROYLOCK(lock)        (0)
    #define NK_SYNCHRONIZED(lock, body) do { body; } while (0)
#endif


/**
 * \brief represents a compile-time constant string view
 * 
 * This utility structure allows for extra information about the string to be stored
 * directly alongside it, making simple processing simpler and less error-prone.
 * 
 * \note  The \c m_sizeInBytes field represents the length in bytes of the string value
 *        WITHOUT the <tt>NUL</tt>-terminator.
 */
NK_NATIVE typedef struct NkStringView {
    char   *mp_dataPtr;    /**< pointer to static string buffer */
    NkSize  m_sizeInBytes; /**< size of buffer, in bytes */
} NkStringView;

/**
 * \def   NK_MAKE_STRING_VIEW(str)
 * \brief generates a compile-time string view to a static string
 * \param str string literal to create string view for
 */
#define NK_MAKE_STRING_VIEW(str)     { (char *)str, sizeof str - 1 }
/**
 * \def   NK_MAKE_STRING_VIEW_PTR(str)
 * \brief like NK_MAKE_STRING_VIEW but it includes the code for turning the string view
 *        into an inline pointer
 * \param str string literal to create string view for
 */
#define NK_MAKE_STRING_VIEW_PTR(str) &(NkStringView)NK_MAKE_STRING_VIEW(str)

/**
 * \brief  initializes a string view from a raw C-string
 * \param  [in] strPtr pointer to the source (<tt>NUL</tt>-terminated) C-string
 * \param  [out] resPtr pointer to the NkStringView instance that will be initialized
 *               with the C-string
 * \return the given pointer, a.k.a. \c resPtr
 * \note   \li This function is UTF-8-aware.
 * \note   \li If \c strPtr is not <tt>NUL</tt>-terminated, the behavior is undefined.
 */
NK_NATIVE NK_API NK_INLINE NkStringView *NK_CALL NkStringViewSet(_In_z_ char const *strPtr, _Out_ NkStringView *resPtr);
/**
 * \brief  compares two string views
 * \param  [in] sv1Ptr pointer to the left NkStringView instance
 * \param  [in] sv2Ptr pointer to the right NkStringView instance
 * \return zero if both string views are equal, otherwise non-zero
 * \note   Two string views are equivalent if
 *          (1) their pointers match
 *          (2) their length is equal AND all bytes from [0] up to the length are equal
 */
NK_NATIVE NK_API NK_INLINE NkInt32 NK_CALL NkStringViewCompare(
    _In_ NkStringView const *sv1Ptr,
    _In_ NkStringView const *sv2Ptr
);
/**
 * \brief copies the string view into another one
 * \param [in] srcPtr pointer to the NkStringView instance that is to be duplicated
 * \param [out] dstPtr pointer to an NkStringView instance that will receive the
 *              duplicated string view value
 * \note  The raw string value is not duplicated.
 */
NK_NATIVE NK_API NK_INLINE NkVoid NK_CALL NkStringViewCopy(_In_ NkStringView const *srcPtr, _Out_ NkStringView *dstPtr);


/**
 * \struct NkBufferView
 * \brief  represents a view into a raw memory buffer holding any data
 * \note   This structure is largely equivalent to <tt>NkStringView</tt>. However, it is
 *         not safe to use <tt>NkStringView*()</tt> functions with this type. The reason
 *         why this type exists is to clear up confusions about the relationship between
 *         string views and raw memory ranges.
 */
NK_NATIVE typedef struct NkBufferView {
    NkByte *mp_dataPtr;    /**< pointer to the first byte in the buffer */
    NkSize  m_sizeInBytes; /**< size of the buffer view, in bytes */
} NkBufferView;

/**
 * \def   NK_MAKE_BUFFER_VIEW(b, s)
 * \brief generates a compile-time view to a buffer
 * \param b pointer to the first byte of the buffer view
 * \param s size of the buffer view, in bytes
 */
#define NK_MAKE_BUFFER_VIEW(b, s)     { (NkByte *)b, (NkSize)s }
/**
 * \def   NK_MAKE_BUFFER_VIEW_PTR(b, s)
 * \brief like NK_MAKE_BUFFER_VIEW but it includes the code for turning the buffer view
 *        into an inline pointer
 * \param b pointer to the first byte of the buffer view
 * \param s size of the buffer view, in bytes
 */
#define NK_MAKE_BUFFER_VIEW_PTR(b, s) &(NkBufferView)NK_MAKE_BUFFER_VIEW(b, s)


/**
 * \struct NkRgbaColor
 * \brief  represents a 4-tuple, each component signifying one color component of the
 *         RGBA color models
 */
NK_NATIVE typedef struct NkRgbaColor {
    NkByte m_rVal; /**< red component <tt>[0, 255]</tt> */
    NkByte m_gVal; /**< green component <tt>[0, 255]</tt> */
    NkByte m_bVal; /**< blue component <tt>[0, 255]</tt> */ 
    NkByte m_aVal; /**< alpha component <tt>[0, 255]</tt> */
} NkRgbaColor;

/**
 * \def   NK_MAKE_RGB(r, g, b)
 * \brief generates an RGBA tuple with the given values, setting the alpha channel to 255
 *        (so fully solid) by default
 * \param r red component <tt>[0, 255]</tt>
 * \param g green component <tt>[0, 255]</tt>
 * \param b blue component <tt>[0, 255]</tt>
 */
#define NK_MAKE_RGB(r, g, b)     { .m_rVal = r, .m_gVal = g, .m_bVal = b, .m_aVal = 0xFF }
/**
 * \def   NK_MAKE_RGBA(r, g, b, a)
 * \brief similar to <tt>NK_MAKE_RGB(r, g, b)</tt> but allows specifying a custom alpha
 *        value
 * \param r red component <tt>[0, 255]</tt>
 * \param g green component <tt>[0, 255]</tt>
 * \param b blue component <tt>[0, 255]</tt>
 * \param a alpha component <tt>[0, 255]</tt>
 */
#define NK_MAKE_RGBA(r, g, b, a) { .m_rVal = r, .m_gVal = g, .m_bVal = b, .m_aVal = a }


/**
 * \struct NkPoint2D
 * \brief  represents a point in 2D-space
 * 
 * \par Remarks
 *   This structure does not imply a unit on its value, e.g., it can mean a point in
 *   <em>pixel space</em> or a point in <em>tile space</em>.
 */
NK_NATIVE typedef struct NkPoint2D {
    NkInt64 m_xCoord; /**< x-coordinate */
    NkInt64 m_yCoord; /**< y-coordinate */
} NkPoint2D;


/**
 * \struct NkSize2D
 * \brief  represents a size (i.e., non-negative extents) in 2D-space
 * 
 * \par Remarks
 *   This structure does not imply a unit on its value, e.g., it can mean a size in
 *   <em>pixel space</em> or a point in <em>tile space</em>.
 */
NK_NATIVE typedef struct NkSize2D {
    NkUint64 m_width;  /**< width value */
    NkUint64 m_height; /**< height value */
} NkSize2D;


/**
 * \brief   adds two signed 32-bit integers while employing an overflow check
 * \param   [in] s1 first summand
 * \param   [in] s2 second summand
 * \param   [out] resPtr pointer to the NkInt32 variable that will hold the result of the
 *                operation
 * \return  zero if the operation 'failed', that is, an overflow occurred, non-zero if
 *          the operation succeeded
 * \warning Since the value of \c resPtr is indeterminate if an overflow occurred, only
 *          use the value after having examined the return value of the function call.
 */
NK_NATIVE NK_API NK_INLINE _Return_false_ NkBoolean NK_CALL NkCheckedInt32Add(
              NkInt32 s1,
              NkInt32 s2,
    _Ret_opt_ NkInt32 *resPtr
) {
    *resPtr = s1 + s2;

    return s1 >= 0 && INT32_MAX - s1 >= s2 || s1 < 0 && INT32_MIN - s1 <= s2;
}
/**
 * \brief   adds two signed 64-bit integers while employing an overflow check
 * \param   [in] s1 first summand
 * \param   [in] s2 second summand
 * \param   [out] resPtr pointer to the NkInt64 variable that will hold the result of the
 *                operation
 * \return  zero if the operation 'failed', that is, an overflow occurred, non-zero if
 *          the operation succeeded
 * \warning Since the value of \c resPtr is indeterminate if an overflow occurred, only
 *          use the value after having examined the return value of the function call.
 */
NK_NATIVE NK_API NK_INLINE _Return_false_ NkBoolean NK_CALL NkCheckedInt64Add(
              NkInt64 s1,
              NkInt64 s2,
    _Ret_opt_ NkInt64 *resPtr
) {
    *resPtr = s1 + s2;

    return s1 >= 0 && INT64_MAX - s1 >= s2 || s1 < 0 && INT64_MIN - s1 <= s2;
}
/**
 * \brief   adds two unsigned 32-bit integers while employing an overflow check
 * \param   [in] s1 first summand
 * \param   [in] s2 second summand
 * \param   [out] resPtr pointer to the NkUint32 variable that will hold the result of
 *                the operation
 * \return  zero if the operation 'failed', that is, an overflow occurred, non-zero if
 *          the operation succeeded
 * \warning Since the value of \c resPtr is indeterminate if an overflow occurred, only
 *          use the value after having examined the return value of the function call.
 */
NK_NATIVE NK_API NK_INLINE _Return_false_ NkBoolean NK_CALL NkCheckedUint32Add(
              NkUint32 s1,
              NkUint32 s2, 
    _Ret_opt_ NkUint32 *resPtr
) {
    *resPtr = s1 + s2;

    return UINT32_MAX - s1 >= s2;
}
/**
 * \brief   adds two unsigned 64-bit integers while employing an overflow check
 * \param   [in] s1 first summand
 * \param   [in] s2 second summand
 * \param   [out] resPtr pointer to the NkUint64 variable that will hold the result of
 *                the operation
 * \return  zero if the operation 'failed', that is, an overflow occurred, non-zero if
 *          the operation succeeded
 * \warning Since the value of \c resPtr is indeterminate if an overflow occurred, only
 *          use the value after having examined the return value of the function call.
 */
NK_NATIVE NK_API NK_INLINE _Return_false_ NkBoolean NK_CALL NkCheckedUint64Add(
              NkUint64 s1,
              NkUint64 s2,
    _Ret_opt_ NkUint64 *resPtr
) {
    *resPtr = s1 + s2;

    return UINT64_MAX - s1 >= s2;
}


/**
 * \brief  initializes the random number generator
 * \return \c NkErr_Ok on success, non-zero on failure
 * \see    NkPRNGNext
 * \note   This function must be run once per session before the first call to
 *         <tt>NkPRNGNext()</tt>.
 */
NK_NATIVE NK_API _Return_ok_ enum NkErrorCode NK_CALL NkPRNGInitialize(NkVoid);
/**
 * \brief  uninitializes the random number generator
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li After this call, it is no longer safe to call <tt>NkPRNGNext()</tt>.
 * \note   \li Call this function once from the main thread just before the application
 *             exits.
 */
NK_NATIVE NK_API _Return_ok_ enum NkErrorCode NK_CALL NkPRNGUninitialize(NkVoid);
/**
 * \brief  retrieves the next number in the random number sequence
 * \param  [in] outPtr pointer to an NkUint64 variable that will receive the random
 *              number
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   This function is thread-safe.
 */
NK_NATIVE NK_API _Return_ok_ enum NkErrorCode NK_CALL NkPRNGNext(_Out_ NkUint64 *outPtr);


/**
 * \struct NkUuid
 * \brief represents the UUID data-structure
 */
NK_NATIVE typedef struct NkUuid {
    NkUint32 m_fBlock;  /**< first block of the UUID (8 hex digits) */
    NkUint16 m_sBlock;  /**< second block (4 hex digits) */
    NkUint16 m_tBlock;  /**< third block (4 hex digits) */
    NkUint64 m_ffBlock; /**< fourth and fifth block (4 + 12 hex digits) */
} NkUuid;

/**
 * \brief  generates a new UUID
 * \param  [out] uuidPtr pointer to an NkUuid instance that will receive the generated
 *               bytes
 * \note   \li This implementation generates exclusively version 4 UUIDs (random or
 *             pseudo-random).
 * \note   \li The UUID generator depends on the pseudo-random number generator. It must
 *             be initialized for the UUID generator to work. Use \c NkPRNGInit in order
 *             to make the PRNG ready.
 * \see    NkPRNGInit
 */
NK_NATIVE NK_API NkVoid NK_CALL NkUuidGenerate(_Out_ NkUuid *uuidPtr);
/**
 * \brief  compares two UUIDs
 * \param  [in] fUuid left UUID
 * \param  [in] sUuid right UUID
 * \return non-zero if both UUIDs are equal, zero if they are not
 * \note   \li As per definition, two UUIDs are equal if their bytes are equal.
 * \note   \li It is not required for this function to initialize the PRNG beforehand.
 */
NK_NATIVE NK_API NK_INLINE NkBoolean NK_CALL NkUuidIsEqual(_In_ NkUuid const *fUuid, _In_ NkUuid const *sUuid);
/**
 * \brief  generates an UUID from a normalized UUID string
 * \param  [in] uuidAsStr string representation of the UUID
 * \param  [out] uuidPtr pointer to an NkUuid instance that will receive the result of
 *               the conversion
 * \return \c NkErr_Ok on success, non-zero on failure.
 * \note   \li If the function fails, the contents of \c uuidPtr are undefined.
 * \note   \li There is generally no validation carried out on the input string. If the
 *             input is invalidly encoded, the contents of \c uuidPtr are indeterminate.
 * \note   \li The term \e normalized refers to the usual \c 8-4-4-4-12 normal form.
 * \note   \li It is not required for this function to initialize the PRNG beforehand.
 */
NK_NATIVE NK_API NK_INLINE _Return_ok_ enum NkErrorCode NK_CALL NkUuidFromString(
    _I_bytes_(NK_UUIDLEN) char const *uuidAsStr,
    _Out_                 NkUuid *uuidPtr
);
/**
 * \brief  converts a 128-bit UUID to its normalized string representation
 * \param  [in] uuidPtr pointer to an NkUuid instance that is to be converted to its
 *              string representation
 * \param  [out] strBuf pointer to a <tt>char *</tt> buffer that is to receive the result
 *               of the conversion
 * \return \c strBuf
 * \note   \li If the function fails, the contents of \c strBuf are undefined.
 * \note   \li There is generally no validation carried out on the input UUID. If the
 *             input is invalidly encoded, the contents of \c strBuf are indeterminate.
 * \note   \li The term \e normalized refers to the usual \c 8-4-4-4-12 normal form.
 * \note   \li It is not required for this function to initialize the PRNG beforehand.
 */
NK_NATIVE NK_API NK_INLINE char *NK_CALL NkUuidToString(
    _In_                  NkUuid const *uuidPtr,
    _O_bytes_(NK_UUIDLEN) char *strBuf
);
/**
 * \brief   copies a source UUID into the destination buffer
 * \param   [in] srcPtr pointer to the source \c NkUuid instance
 * \param   [out] resPtr pointer to the destination buffer; must be at least <tt>16</tt>
 *                bytes in size
 * \warning If either \c srcPtr or \c resPtr are \c NULL or point to invalid locations,
 *          the behavior is undefined. 
 */
NK_NATIVE NK_API NK_INLINE NkVoid NK_CALL NkUuidCopy(_In_ NkUuid const *srcPtr, _Out_ NkUuid *resPtr);


/**
 * \enum  NkVariantType
 * \brief contains numeric variant type IDs
 */
NK_NATIVE typedef _In_range_(NkVarTy_None, __NkVarTy_Count__ - 1) enum NkVariantType {
    NkVarTy_None,       /**< no type saved (= not initialized/empty/null) */

    NkVarTy_Boolean,
    NkVarTy_Char,
    NkVarTy_Int8,
    NkVarTy_Int16,
    NkVarTy_Int32,
    NkVarTy_Int64,
    NkVarTy_Uint8,
    NkVarTy_Uint16,
    NkVarTy_Uint32,
    NkVarTy_Uint64,
    NkVarTy_Float,
    NkVarTy_Double,
    NkVarTy_ErrorCode,
    NkVarTy_StringView,
    NkVarTy_BufferView,
    NkVarTy_Uuid,
    NkVarTy_Pointer,
    NkVarTy_Vector,
    NkVarTy_Hashtable,
    NkVarTy_Timer,
    NkVarTy_NkOMObject,

    __NkVarTy_Count__   /**< *only used internally* */
} NkVariantType;

/**
 * \struct  NkVariant
 * \brief   represents the container for a data-structure that can hold a host of
 *          different types
 * \warning The state of \c m_reserved is implementation-defined. Thus, do never access
 *          or manipulate it directly.
 * \todo    use NK_DEFINE_PROTOTYPE() macro
 */
NK_NATIVE typedef struct NkVariant {
#if (defined __cplusplus)
    alignas(NkInt64)
#else
    _Alignas(NkInt64)
#endif
        NkByte const m_reserved[24]; /**< placeholder for the internal structure */
} NkVariant;

/**
 * \brief retrieves a copy of the value currently held by the variant
 * \param [in] varPtr pointer to the NkVariant instance of which the value is to be
 *             retrieved
 * \param [out] tyPtr pointer to a variable that will receive type information of the
 *              contained value
 * \param [out] valPtr pointer to a variable that will receive the copy of the contained
 *              value
 * \note  The memory pointed to by \c valPtr and \c tyPtr need to satisfy the alignment
 *        requirements of the underlying type. Passing misaligned memory is undefined
 *        behavior.
 */
NK_NATIVE NK_API NK_INLINE NkVoid NK_CALL NkVariantGet(
    _In_      NkVariant const *varPtr,
    _Out_opt_ NkVariantType *tyPtr,
    _Out_opt_ NkVoid *valPtr
);
/**
 * \brief   sets the variant's internal type to the given type and updates the underlying
 *          value with the one given as the first variadic parameter
 * \param   [in,out] varPtr pointer to the NkVariant instance that is to be updated
 * \param   [in] valType numeric type ID of the new underlying type
 * \note    The function expects exactly one variadic parameter that satisfies the type
 *          requirements imposed by <tt>valType</tt>.
 * \warning If no variadic parameter is given, the type ID is invalid, or the actual type
 *          of the provided variadic parameter does not adhere to the requirements
 *          imposed by <tt>valType</tt>, the behavior is undefined.
 */
NK_NATIVE NK_API NK_INLINE NkVoid NK_CALL NkVariantSet(
    _Pre_maybevalid_ _Out_ NkVariant *varPtr,
    _In_                   NkVariantType valType,
                           ...
);
/**
 * \brief   makes a (possibly shallow) copy of the given variant
 * \param   [in] srcPtr pointer to the NkVariant a copy is to be made of
 * \param   [out] dstPtr pointer to the NkVariant instance that will receive the copy of
 *                \c srcPtr
 * \warning If \c srcPtr holds a non-pointer value, the copy is deep, otherwise shallow
 *          as the only the pointer is copied, not the memory pointed to by it.
 */
NK_NATIVE NK_API NK_INLINE NkVoid NK_CALL NkVariantCopy(_In_ NkVariant const *srcPtr, _Out_ NkVariant *dstPtr);
/**
 * \brief  determines whether or not the given \c NkVariant instance is <em>empty</tt>,
 *         that is, whether its internal type corresponds to \c NkVarTy_None
 * \param  [in] varPtr pointer to the \c NkVariant instance which is to be examined
 * \return \c NK_TRUE if the variant is empty, \c NK_FALSE if not
 */
NK_NATIVE NK_API NK_INLINE NkBoolean NK_CALL NkVariantIsNull(_In_ NkVariant const *varPtr);
/**
 */
NK_NATIVE NK_API NkStringView const *NK_CALL NkVariantQueryTypeStr(_In_ NkVariantType typeId);


/**
 * \brief trims the given characters from the string, both left and right
 * \param [in] strPtr pointer to the C-string that is to be trimmed
 * \param [in] maxChars maximum number (relative to <tt>strPtr</tt>) of characters to
 *             consider
 * \param [in] keyPtr collection of characters that are to be trimmed
 * \param [out] resPtr pointer to an NkStringView instance that will receive the pointer
 *              to trimmed portion
 * \note  This function is not UTF-8-aware.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkRawStringTrim(
    _In_z_ char const *strPtr,
    _In_   NkSize maxChars,
    _In_z_ char const *keyPtr,
    _Out_  NkStringView *resPtr
);
/**
 * \brief splits the given C-string into two sections separated by a collection of
 *        characters
 * \param [in] strPtr pointer to a (<tt>NUL</tt>-terminated) C-string instance
 * \param [in] ctrlChs list of possible delimiter characters
 * \param [out] str1Ptr pointer to an NkStringView instance that will receive the portion
 *              of the string before the delimiter
 * \param [out] str2Ptr pointer to an NkStringView instance that will receive the portion
 *              of the string after the delimiter
 * \note  \li This function is not UTF-8-aware.
 * \note  \li The string will be split in accordance with the first delimiter found.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkRawStringSplit(
    _In_z_ char const *strPtr,
    _In_z_ char const *ctrlChs,
    _Out_  NkStringView *str1Ptr,
    _Out_  NkStringView *str2Ptr
);


/**
 * \brief  gets the number of elements in the given array of pointers until the first
 *         \c NULL is encountered
 * \param  [in] ptrArray array of pointers
 * \return size in elements
 * \note   If \c ptrArray is <tt>NULL</tt>, the function returns <tt>0</tt>.
 */
NK_NATIVE NK_API NkSize NK_CALL NkArrayGetDynCount(_In_to_null_ NkVoid const **ptrArray);


/**
 * \brief  calculates the origin of the renderer viewport based on window dimensions and
 *         viewport alignment
 * \param  [in] vpAlignment alignment of the viewport; a combination of fields of the
 *              \c NkViewportAlignment enumeration
 * \param  [in] vpExtents extents of the viewport, in tile space
 * \param  [in] tileSize size of one tile, in pixels
 * \param  [in] clExtents size of the client area, in pixels
 * \return coordinates of the upper-left corner of the viewport, in client space (i.e.,
 *         relative to the upper-left corner of the client area)
 * 
 * \par Remarks
 *   This function calculates the point of origin (that is, the top-left corner) of the
 *   viewport that a renderer will render to. The viewport is positioned inside the
 *   client area of the target device according to the specified <em>viewport alignment</em>.
 */
NK_NATIVE NK_API NkPoint2D NK_CALL NkCalculateViewportOrigin(
    _In_ enum NkViewportAlignment vpAlignment,
    _In_ NkSize2D vpExtents,
    _In_ NkSize2D tileSize,
    _In_ NkSize2D clExtents
);


