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
#define NK_NAMESPACE "nk::timer"


/* Noriko includes */
#include <include/Noriko/timer.h>
#include <include/Noriko/platform.h>
#include <include/Noriko/alloc.h>
#include <include/Noriko/log.h>
#include <include/Noriko/comp.h>


/** \cond INTERNAL */
/**
 * \struct __NkInt_Timer
 * \brief  represents the timing device's state
 */
NK_NATIVE typedef struct __NkInt_Timer {
    NkTimerType m_type;      /**< type ID of the timing device */
    NkInt32     m_isRunning; /**< whether or not the timer is currently running */

    /**
     * \union __NkInt_TimerState
     * \brief represents type-specific state for each timer type
     * \note  There are multiple types of timing devices in Noriko. Wrapping them all
     *        into a single type allows us to easily reuse timers, i.e., for initializing
     *        an old and expired timer with new properties. This may reduce the number of
     *        allocations.
     */
    union __NkInt_TimerState {
        /**
         * \struct __NkInt_ElapsedTimerState
         * \brief  represents the internal state for the \c NkTiType_Elapsed timing
         *         device type
         */
        struct __NkInt_ElapsedTimerState {
            NkUint64 m_startTime; /**< start time, implementation-defined unit */
            NkUint64 m_endTime;   /**< end time, implementation-defined unit */
        } m_elTiState;
    } m_tiState;
} __NkInt_Timer;
/*
 * Since the public type is just a placeholder for the internal type defined here, we
 * must make sure that the internal type and the public type are compatible, that is,
 * they adhere to the same size and alignment requirements.
 */
NK_VERIFY_TYPE(NkTimer, __NkInt_Timer);

/**
 * \struct __NkInt_TimingDeviceContext
 * \brief  represents the global context shared by all timing devices implemented
 * \note   \li After initialization, the data held by this data-structure is constant;
 *             thus, it can be shared across threads without problems.
 * \note   \li This context is automatically initialized when the first timer is created.
 */
NK_NATIVE typedef struct __NkInt_TimingDeviceContext {
    struct {
        NkUint64  m_timerFrequency; /**< frequency of the underlying native timing device */
        NkUint64  m_globalBias;     /**< overhead of the timing functions themselves */
    };
} __NkInt_TimingDeviceContext;
/**
 * \brief actual instance of the global timer context
 */
NK_INTERNAL __NkInt_TimingDeviceContext gl_tdContext;


/**
 * \ingroup VirtFn
 * \brief   retrieves the current tick count of the high-precision timer
 * \return  current tick count
 */
NK_EXTERN NK_VIRTUAL NkUint64 NK_CALL __NkVirt_Timer_GetCurrentTicks(NkVoid);
/**
 * \ingroup VirtFn
 * \brief   retrieves the frequency in 1/s of the high-precision timer
 * \return  current timer frequency
 */
NK_EXTERN NK_VIRTUAL NkUint64 NK_CALL __NkVirt_Timer_GetFrequency(NkVoid);


/**
 * \brief  retrieves the global timer overhead
 * \return overhead in the same unit as the underlying performance counter
 * \note   \li The unit this value is returned in is implementation-defined.
 * \note   \li If there is no timer support implemented for the current target platform,
 *         this function will return 0.
 */
NK_INTERNAL NkUint64 __NkInt_Timer_GetOverhead(NkVoid) {
    /**
     * \brief number of iterations to find average timer bias
     */
    NK_INTERNAL NkUint64 const gl_TimerBiasIterCount = 1000ULL;

    NkUint64 diffSum = 0;
    for (NkUint64 i = 0; i < gl_TimerBiasIterCount; i++) {
        NkUint64 startTime = NkTimerGetCurrentTicks();
        NkUint64 endTime   = NkTimerGetCurrentTicks();

        diffSum += endTime - startTime;
    }

    return diffSum / gl_TimerBiasIterCount;
}

/**
 * \brief  initializes the static timing device context
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   \li This function is automatically invoked upon the first call to the
 *         \c NkTimerCreate() function.
 * \note   \li Since this function does not acquire any resources that need cleanup, a
 *         matching \c __NkInt_Timer_UninitializeStaticContext() function does not
 *         exist.
 */
NK_INTERNAL NkErrorCode __NkInt_Timer_InitializeStaticContext(NkVoid) {
    /* Determine timing device overhead. */
    gl_tdContext.m_globalBias = __NkInt_Timer_GetOverhead();
    return NkErr_Ok;
}

/**
 * \brief  initializes the global timing device context
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   This function should be run once from the main thread before any child thread
 *         has been started.
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(TimingDevCxt)(NkVoid) {
    return __NkInt_Timer_InitializeStaticContext();
}

/**
 * \brief  uninitializes the global timing device context
 * \return \c NkErr_Ok on success, non-zero on failure
 * \note   This function should be run once from the main thread after all child threads
 *         have been killed.
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(TimingDevCxt)(NkVoid) {
    return NkErr_Ok;
}
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkTimerCreate(
    _In_     NkTimerType tiType,
    _In_opt_ NkBoolean isAutoStart,
    _Out_    NkTimer *tiPtr
) {
    NK_ASSERT(NK_INRANGE_INCL(tiType, 0, __NkTiType_Count__ - 1), NkErr_InParameter);
    NK_ASSERT(tiPtr != NULL, NkErr_OutParameter);

    __NkInt_Timer *intTimerPtr = (__NkInt_Timer *)tiPtr;

     /* Initialize timer of the given type. */
    *intTimerPtr = (__NkInt_Timer){
        .m_type      = tiType,
        .m_isRunning = NK_FALSE
    };
    /* Set type-specific properties. */
    switch (tiType) {
        case NkTiType_Elapsed:
            intTimerPtr->m_tiState.m_elTiState = (struct __NkInt_ElapsedTimerState){
                .m_startTime = 0,
                .m_endTime   = 0
           };

            break;
        default:
            return NkErr_NotImplemented;
    }

    /* Start timer if requested. */
    if (isAutoStart)
        NkTimerStart(tiPtr);
    return NkErr_Ok;
}

NkVoid NK_CALL NkTimerDestroy(_Inout_ NkTimer *tiPtr) {
    if (tiPtr == NULL)
        return;
    __NkInt_Timer *intTimerPtr = (__NkInt_Timer *)tiPtr;

    /* Stop timer if necessary. */
    if (intTimerPtr->m_isRunning)
        NkTimerStop(tiPtr);
    /* Run any timer-specific uninitialization. */
    switch (intTimerPtr->m_type) { default:; }
}

NkVoid NK_CALL NkTimerStart(_Inout_ NkTimer *tiPtr) {
    NK_ASSERT(tiPtr != NULL, NkErr_InParameter);
    NK_ASSERT(((__NkInt_Timer *)tiPtr)->m_isRunning == NK_FALSE, NkErr_ObjectState);

    /* Do any start-up routines for the given timer depending on type. */
    __NkInt_Timer *intTimerPtr = (__NkInt_Timer *)tiPtr;
    switch (intTimerPtr->m_type) {
        case NkTiType_Elapsed:
            intTimerPtr->m_tiState.m_elTiState.m_startTime = NkTimerGetCurrentTicks();

            break;
    }

    /* Update state. */
    intTimerPtr->m_isRunning = NK_TRUE;
}

NkVoid NK_CALL NkTimerStop(_Inout_ NkTimer *tiPtr) {
    NK_ASSERT(tiPtr != NULL, NkErr_InParameter);
    NK_ASSERT(((__NkInt_Timer *)tiPtr)->m_isRunning == NK_TRUE, NkErr_ObjectState);

    /* Do any stopping routines for the given timer depending on type. */
    __NkInt_Timer *intTimerPtr = (__NkInt_Timer *)tiPtr;
    switch (intTimerPtr->m_type) {
        case NkTiType_Elapsed:
            intTimerPtr->m_tiState.m_elTiState.m_endTime = NkTimerGetCurrentTicks();

            break;
    }

    /* Update state. */
    intTimerPtr->m_isRunning = NK_FALSE;
}

NkVoid NK_CALL NkTimerRestart(_Inout_ NkTimer *tiPtr) {
    NK_ASSERT(tiPtr != NULL, NkErr_InParameter);
    NK_ASSERT(((__NkInt_Timer *)tiPtr)->m_isRunning == NK_TRUE, NkErr_ObjectState);

    NkTimerStop(tiPtr);
    NkTimerStart(tiPtr);
}


NkDouble NK_CALL NkElapsedTimerGetAs(_In_ NkTimer const *tiPtr, _In_ NkTimerPrecision precId) {
    NK_ASSERT(tiPtr != NULL, NkErr_InParameter);
    NK_ASSERT(((__NkInt_Timer *)tiPtr)->m_type == NkTiType_Elapsed, NkErr_ObjectType);

    /*
     * If timer is still running, calculate the offset based on the difference between
     * the current time and the starting time. Otherwise, use the timer's stop time.
     */
    NkUint64 endTime = 0;
    __NkInt_Timer *intTimerPtr = (__NkInt_Timer *)tiPtr;
    /* Determine timer end time. */
    endTime = intTimerPtr->m_isRunning == NK_TRUE
        ? NkTimerGetCurrentTicks()
        : intTimerPtr->m_tiState.m_elTiState.m_endTime
    ;

    /* Calculate difference. */
    NkUint64 const timeDiff = endTime - intTimerPtr->m_tiState.m_elTiState.m_startTime - gl_tdContext.m_globalBias;

    /* Convert to requested precision. */
    return timeDiff / (gl_tdContext.m_timerFrequency / (NkDouble)precId);
}


NkUint64 NK_CALL NkTimerGetCurrentTicks(NkVoid) {
    return __NkVirt_Timer_GetCurrentTicks();
}

NkUint64 NK_CALL NkTimerGetFrequency(NkVoid) {
    return __NkVirt_Timer_GetFrequency();
}


/** \cond INTERNAL */
/**
 * \brief info for the <em>timing device context</em> component 
 */
NK_COMPONENT_DEFINE(TimingDevCxt) {
    .m_compUuid     = { 0x546af15e, 0x9965, 0x46e2, 0xa6d8db2ababb00eb },
    .mp_clsId       = NULL,
    .m_compIdent    = NK_MAKE_STRING_VIEW("timing device context"),
    .m_compFlags    = 0,
    .m_isNkOM       = NK_FALSE,

    .mp_fnQueryInst = NULL,
    .mp_fnStartup   = &NK_COMPONENT_STARTUPFN(TimingDevCxt),
    .mp_fnShutdown  = &NK_COMPONENT_SHUTDOWNFN(TimingDevCxt)
};
/** \endcond */


#undef NK_NAMESPACE


