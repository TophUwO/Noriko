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
 * \file  component.h
 * \brief global API for the Noriko's component system
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/platform.h>


/**
 * \def   NK_ENSURE_INITIALIZED(cxt)
 * \brief checks the component state and returns an error if the component is not
 *        initialized
 * \param cxt pointer to the component
 */
#define NK_ENSURE_INITIALIZED(cxt)                                \
    if (NkComponentGetInit((NkComponent const *)&cxt) != NK_TRUE) \
        { return NkErr_ComponentState; }
/**
 * \def   NK_ENSURE_INITIALIZED_VOID(cxt)
 * \brief similar to NK_ENSURE_INITIALIZED but without returning an error; suitable for
 *        functions returning <tt>NkVoid</tt>
 * \param cxt pointer to the component
 * \see   NK_ENSURE_INITIALIZED
 */
#define NK_ENSURE_INITIALIZED_VOID(cxt)                           \
    if (NkComponentGetInit((NkComponent const *)&cxt) != NK_TRUE) \
        { return; }
/**
 * \def   NK_ENSURE_NOT_INITIALIZED(cxt)
 * \brief checks the component state and returns an error if the component is initialized
 * \param cxt pointer to the component
 */
#define NK_ENSURE_NOT_INITIALIZED(cxt)                             \
    if (NkComponentGetInit((NkComponent const *)&cxt) != NK_FALSE) \
        { return NkErr_ComponentState; }
/**
 * \def   NK_ENSURE_NOT_INITIALIZED_VOID(cxt)
 * \brief similar to NK_ENSURE_NOT_INITIALIZED but without returning an error; suitable
 *        for functions returning <tt>NkVoid</tt>
 * \param cxt pointer to the component
 * \see   NK_ENSURE_NOT_INITIALIZED
 */
#define NK_ENSURE_NOT_INITIALIZED_VOID(cxt)                        \
    if (NkComponentGetInit((NkComponent const *)&cxt) != NK_FALSE) \
        { return; }

/**
 * \def   NK_INITIALIZE(cxt)
 * \brief initializes the component denoted by \c cxt
 * \param cxt component (as non-pointer)
 * \see   NK_UNINITIALIZE
 * \note  This function call is thread-safe.
 */
#define NK_INITIALIZE(cxt)   NkComponentSetInit((NkComponent *)(&cxt), NK_TRUE);
/**
 * \def   NK_UNINITIALIZE(cxt)
 * \brief uninitializes the component denoted by \c cxt
 * \param cxt component (as non-pointer)
 * \note  This function call is thread-safe.
 */
#define NK_UNINITIALIZE(cxt) NkComponentSetInit((NkComponent *)(&cxt), NK_FALSE);


/**
 * \struct NkComponent
 * \brief  represents the static init flag mechanism used by singletons and regular
 *         components
 * \note   \li A value of \c non-zero signifies that the component is initialized while a
 *             value of \c zero signifies that initialization has not yet happened or the
 *             component has already been uninitialized.
 * \note   \li Functions that use and/or modify components should verify that the
 *             component is ready for use before the function begins its work. This check
 *             should be done at least in non-deploy builds.
 * \note   \li The initialization flag mechanism is thread-safe.
 */
NK_NATIVE typedef struct NkComponent {
#if (defined NK_TARGET_MULTITHREADED)
    _Atomic NkInt64 m_isInit; /**< (atomic) init flag for component */
#else
    NkInt64 m_isInit;
#endif
} NkComponent;

/**
 * \brief  atomically retrieves the init flag for the given component
 * \param  [in] compPtr pointer to the NkComponent instance of which the init flag value
 *              is to be retrieved
 * \return value of init flag
 * \note   This function is thread-safe.
 */
NK_NATIVE NK_INLINE NkInt64 NK_CALL NkComponentGetInit(_In_ NkComponent const *compPtr) {
#if (defined NK_TARGET_MULTITHREADED)
    return atomic_load(&compPtr->m_isInit);
#else
    return compPtr->m_isInit;
#endif
}
/**
 * \brief  atomically updates the init flag for the given component
 * \param  [in,out] compPtr pointer to the NkComponent instance of which the init flag value
 *                  is to be updated
 * \param  [in] initVal new initialization state
 * \note   This function is thread-safe.
 */
NK_NATIVE NK_INLINE NkVoid NK_CALL NkComponentSetInit(_Inout_ NkComponent *compPtr, _In_ NkInt64 initVal) {
#if (defined NK_TARGET_MULTITHREADED)
    atomic_store(&compPtr->m_isInit, initVal);
#else
    compPtr->m_isInit = initVal;
#endif
}


