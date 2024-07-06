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
 * \file  timer.c
 * \brief implements a variety of timing devices for use in Noriko
 */
#define NK_NAMESPACE u8"nk::timer"


/* Noriko includes */
#include <include/Noriko/timer.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/alloc.h>


/**
 * \struct NkTimer
 * \brief  represents the timer's state
 */
struct NkTimer {
    NkTimerType m_type;      /**< type ID of the timing device */
    NkBoolean   m_isRunning; /**< whether or not the timer is currently running */
    NkUint64    m_tiBias;    /**< bias to subtract from the elapsed time (timer overhead) */

    /**
     * \union NkInternalTimerState
     * \brief represents type-specific state for each timer type
     * \note  There are multiple types of timing devices in Noriko. Wrapping them all
     *        into a single type allows us to easily reuse timers, i.e., for initializing
     *        an old and expired timer with new properties. This may reduce the number of
     *        allocations.
     */
    union NkInternalTimerState {
        /**
         * \struct NkInternalElapsedTimerState
         * \brief  represents the internal state for the \c NkTiType_Elapsed timing
         *         device type
         */
        struct NkInternalElapsedTimerState {
            NkUint64 m_startTime; /**< start time, implementation-defined unit */
            NkUint64 m_endTime;   /**< end time, implementation-defined unit */
        } m_elTiState;
    } m_tiState;
};

/**
 * \struct NkInternalTimingDeviceContext
 * \brief  represents the global context shared by all timing devices implemented
 * \note   \li After initialization, the data held by this data-structure is constant;
 *             thus, it can be shared across threads without problems.
 * \note   \li This context is automatically initialized when the first timer is created.
 */
struct NkInternalTimingDeviceContext {
    NkBoolean m_isInit;         /**< whether or not the timing facility is ready */
    NkUint64  m_timerFrequency; /**< frequency of the underlying native timing device */
};
/**
 * \internal 
 * \brief actual instance of the global timer context
 * \endinternal
 */
NK_INTERNAL struct NkInternalTimingDeviceContext gl_tdContext = {
    .m_isInit         = NK_FALSE,
    .m_timerFrequency = 0
};


/**
 * \brief  retrieves the current timestamp in a platform-independent way
 * \param  [out] retPtr pointer to an NkUint64 variable which will receive the current
 *              timestamp
 * \return \c NkErr_Ok on success, non-zero on failure.
 * \note   If no timing support is implemented on the current target platform, this
 *         function sets \c retPtr to \c 0 and returns <tt>NkErr_NotImplemented</tt>.
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInternal_TimerGetClock(_Out_ NkUint64 *retPtr) {
#if (defined NK_TARGET_WINDOWS)
    QueryPerformanceCounter((LARGE_INTEGER *)retPtr);

    return NkErr_Ok;
#else
    *retPtr = 0ULL;
    return NkErr_NotImplemented;
#endif
}

/**
 * \brief  retrieves the underlying timer's frequency a platform-independent way
 * \param  [out] retPtr pointer to an NkUint64 variable which will receive the frequency
 *               value
 * \return \c NkErr_Ok on success, non-zero on failure.
 * \note   If no timing support is implemented on the current target platform, this
 *         function sets \c retPtr to \c 0 and returns <tt>NkErr_NotImplemented</tt>.
 */
NK_INTERNAL _Return_ok_ NkErrorCode __NkInternal_TimerGetFrequency(_Out_ NkUint64 *retPtr) {
#if (defined NK_TARGET_WINDOWS)
    QueryPerformanceFrequency((LARGE_INTEGER *)retPtr);

    return NkErr_Ok;
#else
    *retPtr = 0ULL;
    return NkErr_NotImplemented;
#endif
}


_Return_ok_ NkErrorCode NK_CALL NkTimerCreate(
    _In_                             NkTimerType tiType,
    _In_opt_                         NkBoolean isAutoStart,
    _Init_ptr_ _Deref_pre_opt_valid_ NkTimer **tiPtr
) {
    /* If global timing device context is not yet initialized, do it here. */
    if (gl_tdContext.m_isInit == NK_FALSE) {
        NK_IGNORE_RETURN_VALUE(__NkInternal_TimerGetFrequency(&gl_tdContext.m_timerFrequency));

        gl_tdContext.m_isInit = NK_TRUE;
    }
    NK_ASSERT(NK_INRANGE_INCL(tiType, 0, __NkTiType_Count__ - 1), NkErr_InParameter);
    NK_ASSERT(tiPtr != NULL, NkErr_OutParameter);

    /*
     * If the given timer has been previously initialized, reinitialize it with the new
     * properties. Otherwise, allocate memory for the data-structure.
     */
    if (*tiPtr == NULL) {
        /* Allocate memory for the timer. */
        NkErrorCode errorCode = NkAllocateMemory(
            NK_MAKE_ALLOCATION_CONTEXT(),
            sizeof * *tiPtr,
            0,
            NK_FALSE,
            tiPtr
        );

        if (errorCode != NkErr_Ok)
            return errorCode;
    }

     /* Initialize timer of the given type. */
    **tiPtr = (NkTimer){
        .m_type      = tiType,
        .m_isRunning = NK_FALSE,
        .m_tiBias    = 0
    };

    /* Set type-specific properties. */
    switch (tiType) {
        case NkTiType_Elapsed:
           (*tiPtr)->m_tiState.m_elTiState = (struct NkInternalElapsedTimerState){
                .m_startTime = 0,
                .m_endTime   = 0
           };

            break;
        default:
            return NkErr_NotImplemented;
    }

    /* Start timer if requested. */
    if (isAutoStart)
        NkTimerStart(*tiPtr);
    return NkErr_Ok;
}

NkVoid NK_CALL NkTimerDestroy(_Uninit_ptr_ NkTimer **tiPtr) {
    if (tiPtr == NULL || *tiPtr == NULL)
        return;

    /* Stop timer if necessary. */
    if ((*tiPtr)->m_isRunning)
        NkTimerStop(*tiPtr);
    /* Run any timer-specific uninitialization. */
    switch ((*tiPtr)->m_type) { default:; }

    NkFreeMemory(*tiPtr);
    *tiPtr = NULL;
}

NkVoid NK_CALL NkTimerStart(_Inout_ NkTimer *tiPtr) {
    NK_ASSERT(tiPtr != NULL, NkErr_InParameter);
    NK_ASSERT(tiPtr->m_isRunning == NK_FALSE, NkErr_ObjectState);

    /* Do any start-up routines for the given timer depending on type. */
    switch (tiPtr->m_type) {
        case NkTiType_Elapsed:
            NK_IGNORE_RETURN_VALUE(
                __NkInternal_TimerGetClock(&tiPtr->m_tiState.m_elTiState.m_startTime)
            );

            break;
    }

    /* Update state. */
    tiPtr->m_isRunning = NK_TRUE;
}

NkVoid NK_CALL NkTimerStop(_Inout_ NkTimer *tiPtr) {
    NK_ASSERT(tiPtr != NULL, NkErr_InParameter);
    NK_ASSERT(tiPtr->m_isRunning == NK_TRUE, NkErr_ObjectState);

    /* Do any stopping routines for the given timer depending on type. */
    switch (tiPtr->m_type) {
        case NkTiType_Elapsed:
            NK_IGNORE_RETURN_VALUE(
                __NkInternal_TimerGetClock(&tiPtr->m_tiState.m_elTiState.m_endTime)
            );

            break;
    }

    /* Update state. */
    tiPtr->m_isRunning = NK_FALSE;
}

NkVoid NK_CALL NkTimerRestart(_Inout_ NkTimer *tiPtr) {
    NK_ASSERT(tiPtr != NULL, NkErr_InParameter);
    NK_ASSERT(tiPtr->m_isRunning == NK_TRUE, NkErr_ObjectState);

    NkTimerStop(tiPtr);
    NkTimerStart(tiPtr);
}

NkDouble NK_CALL NkElapsedTimerGetAs(_In_ NkElapsedTimer const *tiPtr, _In_ NkTimerPrecision precId) {
    NkTimer const *actualTimer = (NkTimer const *)tiPtr;
    NK_ASSERT(tiPtr != NULL, NkErr_InParameter);
    NK_ASSERT(actualTimer->m_type == NkTiType_Elapsed, NkErr_ObjectType);

    /*
     * If timer is still running, calculate the offset based on the difference between
     * the current time and the starting time. Otherwise, use the timer's stop time.
     */
    NkUint64 endTime = 0;
    if (actualTimer->m_isRunning == NK_TRUE)
        NK_IGNORE_RETURN_VALUE(__NkInternal_TimerGetClock(&endTime));
    else 
        endTime = actualTimer->m_tiState.m_elTiState.m_endTime;

    /* Calculate difference. */
    NkUint64 const timeDiff = endTime - actualTimer->m_tiState.m_elTiState.m_startTime;

    /* Convert to requested precision. */
    return timeDiff / (gl_tdContext.m_timerFrequency / (NkDouble)precId);
}


#undef NK_NAMESPACE


