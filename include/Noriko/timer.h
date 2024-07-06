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
 * \file  timer.h
 * \brief defines a variety of timing devices for use in Noriko
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/error.h>


/**
 * \enum  NkTimerType
 * \brief timing device IDs used for creating timers of various types
 */
NK_NATIVE typedef _In_range_(0, __NkTiType_Count__ - 1) enum NkTimerType {
    NkTiType_Elapsed,  /**< elapsed timer, useful for measuring time distances and benchmarking */

    __NkTiType_Count__ /**< *only used internally* */
} NkTimerType;

/**
 * \enum  NkTimerPrecision
 * \brief represents the timer precisions to choose from when retrieving timer values
 */
NK_NATIVE typedef enum NkTimerPrecision {
    NkTiPrec_Nanoseconds  = (NkInt32)1e+9, /**< unit for nanoseconds */
    NkTiPrec_Microseconds = (NkInt32)1e+6, /**< unit for microseconds */
    NkTiPrec_Milliseconds = (NkInt32)1e+3, /**< unit for milliseconds */
    NkTiPrec_Seconds      = (NkInt32)1e+0, /**< unit for seconds */

    __NkTiPrec_Count__                     /**< *only used internally* */
} NkTimerPrecision;


/**
 * \struct NkTimer 
 * \brief  generic timer type, for use with the timer API functions declared in
 *         \c timer.h 
 */
NK_NATIVE typedef struct NkTimer NkTimer;
/**
 * \struct NkElapsedTimer
 * \brief  forward-declaration of Noriko's elapsed timer implementations
 */
NK_NATIVE typedef struct NkElapsedTimer NkElapsedTimer;


/**
 * \brief   creates a new timing device with the specified properties or reinitializes an
 *          an existing timing device with new properties
 * \param   [in] tiType type ID of the new timer
 * \param   [in] isAutoStart whether or not to start the timer automatically right before
 *               the function returns
 * \param   [in] tiPtr pointer to a variable holding the pointer to either an existing
 *               timer (in which case the existing timer will be halted, and reinitialized
 *               with the new properties), or a <tt>NULL</tt>-pointer (in which case a new
 *               timer with the given properties will be created)
 * \return  \c NkErr_Ok if everything goes well, non-zero on failure
 * \note    \li If the function fails, \c tiPtr will be set to <tt>NULL</tt>.
 * \note    \li The function will never fail to reinitialize an existing timer.
 * \warning If \c tiPtr is <tt>NULL</tt>, the behavior is undefined.
 */
NK_NATIVE NK_API _Return_ok_ NkErrorCode NK_CALL NkTimerCreate(
    _In_                             NkTimerType tiType,
    _In_opt_                         NkBoolean isAutoStart,
    _Init_ptr_ _Deref_pre_opt_valid_ NkTimer **tiPtr
);
/**
 * \brief   destroys the given timing device
 * \param   [in] tiPtr pointer to the variable that holds the pointer to the timing device
 * \note    \li Like all destructors, passing a <tt>NULL</tt>-pointer will result in a
 *              no-op.
 * \note    \li This function works for all timing devices.
 * \warning If \c tiPtr is <tt>NULL</tt>, the behavior is undefined.
 */
NK_NATIVE NK_API NkVoid NK_CALL NkTimerDestroy(_Uninit_ptr_ NkTimer **tiPtr);
/**
 * \brief   starts the given timer
 * \param   [in] tiPtr pointer to the timing device
 * \note    This function works for all timing devices.
 * \warning If \c tiPtr is <tt>NULL</tt>, the behavior is undefined.
 */
NK_NATIVE NK_API NK_INLINE NkVoid NK_CALL NkTimerStart(_Inout_ NkTimer *tiPtr);
/**
 * \brief   stops the given timer
 * \param   [in] tiPtr pointer to the timing device
 * \note    This function works for all timing devices.
 * \warning If \c tiPtr is <tt>NULL</tt>, the behavior is undefined.
 */
NK_NATIVE NK_API NK_INLINE NkVoid NK_CALL NkTimerStop(_Inout_ NkTimer *tiPtr);
/**
 * \brief   restarts the given timer
 * \param   [in] tiPtr pointer to the timing device
 * \note    \li This function is basically a shortcut for stopping and subsequently
 *          starting and is thus equivalent to
 *          \code{.c}
 *          NkTimer *t;
 * 
 *          ...
 * 
 *          NkTimerStop(t);
 *          NkTimerStart(t);
 *          \endcode
 * \note    \li This function works for all timing devices.
 * \warning If \c tiPtr is <tt>NULL</tt>, the behavior is undefined.
 */
NK_NATIVE NK_API NK_INLINE NkVoid NK_CALL NkTimerRestart(_Inout_ NkTimer *tiPtr);

/**
 * \brief   retrieves the elapsed time for timers of type "elapsed timer"
 * \param   [in] tiPtr pointer to the timing device; must have been created/reinitialized
 *               using \c NkTimerCreate() with the <tt>tiType = NkTiType_Elapsed</tt>
 *               property
 * \param   [in] precId unit/precision to receive the result in
 * \return  elapsed time, in the given unit, or \c NaN if there was an error
 * \note    If the timer pointed to by \c tiPtr is currently stopped, it will use the end
 *          time to calculate the duration. If not, the function will use the current time
 *          instead.
 * \warning If \c tiPtr is <tt>NULL</tt> or \c tiPtr is not a timer of type
            <tt>NkTiType_Elapsed</tt>, the behavior is undefined.
 */
NK_NATIVE NK_API NkDouble NK_CALL NkElapsedTimerGetAs(_In_ NkElapsedTimer const *tiPtr, _In_ NkTimerPrecision precId);


