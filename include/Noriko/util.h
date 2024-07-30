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
 * \def   NK_INRANGE_INCL(x, lo, hi)
 * \brief checks whether lo <= x <= hi is *true*
 * \param x numeric value to compare against *lo* and *hi*
 * \param lo lower bound of the comparison
 * \param hi higher bound of the comparison
 */
#define NK_INRANGE_INCL(x, lo, hi)   (NkBoolean)((((hi) - (x)) * ((x) - (lo))) >= 0)
/**
 * \def   NK_INRANGE_EXCL(x, lo, hi)
 * \brief checks whether lo < x < hi is *true*
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
 * \brief returns the size in bytes of the member \c m of data-structure \c st, i.e.
 *        <tt>sizeof st.m</tt>
 * \param st type name of the data-structure
 * \param m identifier of the member
 */
#define NK_SIZEOF(st, m)             ((NkSize)(sizeof ((st *)0)->m))


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
 * \def   NK_UNREFERENCED_PARAMETER(p)
 * \brief suppress "unreferenced parameter 'x'" warnings
 * \param p name of the parameter
 */
#define NK_UNREFERENCED_PARAMETER(p) ((void)(p))
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
#define NK_MAKE_STRING_VIEW(str)     { .mp_dataPtr = str, .m_sizeInBytes = sizeof str }
/**
 * \def   NK_MAKE_STRING_VIEW_PTR(str)
 * \brief like NK_MAKE_STRING_VIEW but it includes the code for turning the string view
 *        into an inline pointer
 * \param str string literal to create string view for
 */
#define NK_MAKE_STRING_VIEW_PTR(str) &(NkStringView)NK_MAKE_STRING_VIEW(str)


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
NK_NATIVE NK_API _Return_ok_ enum NkErrorCode NK_CALL NkPRNGInit(NkVoid);
/**
 * \brief uninitializes the random number generator
 * \note  \li After this call, it is no longer safe to call <tt>NkPRNGNext()</tt>.
 * \note  \li Call this function once from the main thread just before the application
 *            exits.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkPRNGUninit(NkVoid);
/**
 * \brief  retrieves the next number in the random number sequence
 * \param  [in] outPtr pointer to an NkUint64 variable that will receive the random number
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   This function is thread-safe.
 */
NK_NATIVE NK_API _Return_ok_ enum NkErrorCode NK_CALL NkPRNGNext(_Out_ NkUint64 *outPtr);


/**
 * \union NkUuid
 * \brief represents the UUID data-structure
 */
NK_NATIVE typedef union NkUuid {
    NkByte   m_asByte[16]; /**< access UUID per byte */
    NkUint32 m_asUi32[4];  /**< access UUID per word */
    NkUint64 m_asUi64[2];  /**< access UUID per doubleword */
} NkUuid;

/**
 * \brief  generates a new UUID
 * \param  [out] uuidPtr pointer to an NkUuid instance that will receive the generated
 *               bytes
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li This implementation generates exclusively version 4 UUIDs (random or
 *             pseudo-random).
 * \note   \li The UUID generator depends on the pseudo-random number generator. It must
 *             be initialized for the UUID generator to work. Use \c NkPRNGInit in order
 *             to make the PRNG ready.
 * \see    NkPRNGInit
 */
NK_NATIVE NK_API _Return_ok_ enum NkErrorCode NK_CALL NkUuidGenerate(_Out_ NkUuid *uuidPtr);
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
    _I_bytes_(37) char const *uuidAsStr,
    _Out_         NkUuid *uuidPtr
);
/**
 * \brief  converts a 128-bit UUID to its normalized string representation
 * \param  [in] uuidPtr pointer to an NkUuid instance that is to be converted to its
 *              string representation
 * \param  [out] strBuf pointer to a <tt>char *</tt> buffer that is to receive the result
 *               of the conversion
 * \return \c NkErr_Ok on success, non-zero on failure.
 * \note   \li If the function fails, the contents of \c strBuf are undefined.
 * \note   \li There is generally no validation carried out on the input UUID. If the
 *             input is invalidly encoded, the contents of \c strBuf are indeterminate.
 * \note   \li The term \e normalized refers to the usual \c 8-4-4-4-12 normal form.
 * \note   \li It is not required for this function to initialize the PRNG beforehand.
 */
NK_NATIVE NK_API NK_INLINE _Return_ok_ enum NkErrorCode NK_CALL NkUuidToString(
    _In_          NkUuid const *uuidPtr,
    _O_bytes_(37) char *strBuf
);


