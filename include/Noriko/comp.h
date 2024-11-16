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
 * \file  comp.h
 * \brief defines the Noriko component data-structure used in the startup- and cleanup
 *        procedures
 */


#pragma once

/* Noriko includes */
#include <include/Noriko/def.h>
#include <include/Noriko/util.h>
#include <include/Noriko/nkom.h>


/**
 * \struct NkComponent
 * \brief  represents a global Noriko component which has to be started-up and shutdown
 *         during the respective application phases
 */
NK_NATIVE typedef struct NkComponent {
    NkUuid              m_compUuid;  /**< UUID of the component */
    NkUuid       const *mp_clsId;    /**< CLSID of the respective NkOM class */
    NkStringView        m_compIdent; /**< textual identifier of the component */
    NkUint32            m_compFlags; /**< additional component flags; reserved, must be 0 */
    NkBoolean           m_isNkOM;    /**< whether or not \c mp_clsId and \c mp_fnQueryInst are valid */

    /**
     */
    NkIBase     *(NK_CALL *mp_fnQueryInst)(NkVoid);
    /**
     */
    NkErrorCode  (NK_CALL *mp_fnStartup)(NkVoid);
    /**
     */
    NkErrorCode  (NK_CALL *mp_fnShutdown)(NkVoid);
} NkComponent;


/**
 */
#define NK_COMPONENT(n)               gl_c_##n##Component
/**                                   
 */                                   
#define NK_COMPONENT_STARTUPFN(n)     __NkInt_##n##_Startup
/**                                   
 */                                   
#define NK_COMPONENT_SHUTDOWNFN(n)    __NkInt_##n##_Shutdown

/**
 */
#define NK_COMPONENT_IMPORT(n)                                                   \
    NK_EXTERN NkComponent const NK_COMPONENT(n);                                 \
    NK_EXTERN _Return_ok_ NkErrorCode NK_CALL __NkInt_##n##_PreStartup(NkVoid);  \
    NK_EXTERN _Return_ok_ NkErrorCode NK_CALL __NkInt_##n##_PostStartup(NkVoid); \
    NK_EXTERN _Return_ok_ NkErrorCode NK_CALL __NkInt_##n##_PreShutdown(NkVoid); \
    NK_EXTERN _Return_ok_ NkErrorCode NK_CALL __NkInt_##n##_PostShutdown(NkVoid)
/**                                   
 */                                   
#define NK_COMPONENT_DEFINE(n)        NK_NATIVE NkComponent const NK_COMPONENT(n) =


