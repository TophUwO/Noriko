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
 * \file  world.h
 * \brief defines the public API for the 'world layer', that is, the layer that manages
 *        and displays the world and all in-game objects
 */
#define NK_NAMESPACE "nk::world"


/* Noriko includes */
#include <include/Noriko/layer.h>
#include <include/Noriko/nkom.h>
#include <include/Noriko/renderer.h>
#include <include/Noriko/window.h>
#include <include/Noriko/layer.h>
#include <include/Noriko/log.h>
#include <include/Noriko/timer.h>
#include <include/Noriko/path.h>
#include <include/Noriko/noriko.h>
#include <include/Noriko/comp.h>

#include <include/Noriko/dstruct/string.h>

#include <math.h>
/** \cond INTERNAL */
/**
 */
#define N(x, y) ((NkUint32)((((NkUint16)(x) & 0xFFFF) | (((NkUint16)(y) & 0xFFFF)) << 16)))
/** \endcond */


/** \cond INTERNAL */
/**
 */
NK_NATIVE typedef struct __NkInt_WorldLayer {
    NKOM_IMPLEMENTS(NkILayer);

    NkIWindow          *mp_rdTarget;     /**< cached reference to the render window */
    NkIRenderer        *mp_rdRef;        /**< cached reference to the window's renderer */
    NkIInput           *mp_ialRef;       /**< cached reference to the IAL */
    NkRendererResource *mp_mainTexAtlas; /**< texture atlas for world, etc. */
    NkRendererResource *mp_texAtlasMask; /**< texture atlas transparency mask */
    NkRendererResource *mp_playerAtlas;
    NkRendererResource *mp_plAtlasMask;

    NkVec2F             m_prevPos;
    NkVec2F             m_playerPos;
    NkVec2F             m_targetPos;
    NkBoolean           m_isMoving;
    NkVec2F             m_lvel;
    NkVec2F             m_vel;
    NkUint64            m_lastAnimFrame;
    NkVec2F             m_currAnimFrame;
    NkInt32             m_dir;
    NkTimer             t;
    NkFloat             m_moveSpeed;
    NkTimer             t2;
    int                 state;
} __NkInt_WorldLayer;


// top left
NK_INTERNAL NkUint32 gl_TestMap1[16 * 16] = {
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(5, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(0, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(0, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(5, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
                                                                                                        
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(5, 0), N(2, 0), N(5, 0), N(2, 0), N(6, 3), N(7, 3), N(7, 3), N(7, 3),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(5, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(6, 3), N(7, 3), N(7, 3), N(7, 3), N(8, 2), N(7, 6), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(6, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(6, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(5, 6), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(6, 5), N(7, 5), N(7, 5), N(8, 1), N(7, 4), N(7, 4), N(7, 6), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(6, 6), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4)
};

// bottom left
NK_INTERNAL NkUint32 gl_TestMap3[16 * 16] = {
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(6, 5), N(7, 5), N(7, 5), N(8, 1), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(5, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(5, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4),

    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(6, 5), N(7, 5),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(5, 0), N(2, 0), N(2, 0), N(2, 0),   N(5, 0), N(5, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(5, 0), N(5, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(5, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
};

// top right
NK_INTERNAL NkUint32 gl_TestMap2[16 * 16] = {
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(6, 3), N(7, 3), N(7, 3),   N(7, 3), N(7, 3), N(7, 3), N(8, 3), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(6, 1), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4),   N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4),   N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4),   N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0), N(2, 0),

    N(7, 3), N(7, 3), N(7, 3), N(7, 3), N(7, 3), N(8, 2), N(7, 4), N(7, 4),   N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),   N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),   N(7, 4), N(7, 1), N(7, 5), N(8, 5), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),   N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(0, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(5, 6), N(7, 4), N(7, 4),   N(7, 4), N(8, 4), N(2, 0), N(0, 0), N(5, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),   N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),   N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(3, 6), N(7, 4), N(7, 4), N(7, 4), N(7, 4),   N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
};

// bottom right
NK_INTERNAL NkUint32 gl_TestMap4[16 * 16] = {
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 1), N(7, 5), N(7, 5), N(7, 5),   N(7, 5), N(8, 5), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),
    N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(8, 4), N(2, 0), N(2, 0), N(5, 0),   N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),

    N(7, 5), N(7, 5), N(7, 5), N(7, 5), N(8, 5), N(2, 0), N(2, 0), N(2, 0),   N(5, 0), N(2, 0), N(5, 0), N(2, 0), N(6, 3), N(7, 3), N(7, 3), N(7, 3),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(5, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(6, 3), N(7, 3), N(7, 3), N(7, 3), N(8, 2), N(7, 6), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(6, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(6, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(6, 5), N(7, 5), N(7, 5), N(8, 1), N(7, 4), N(7, 4), N(7, 6), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(6, 2), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(6, 6), N(7, 4), N(7, 4),
    N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0), N(2, 0),   N(2, 0), N(2, 0), N(2, 0), N(6, 4), N(7, 4), N(7, 4), N(7, 4), N(7, 4)
};


/**
 */
NK_INTERNAL NkVoid __NkInt_WorldLayer_DeleteResources(_Inout_ __NkInt_WorldLayer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Delete resources. */
    self->mp_rdRef->VT->DeleteResource(self->mp_rdRef, &self->mp_mainTexAtlas);
    self->mp_rdRef->VT->DeleteResource(self->mp_rdRef, &self->mp_texAtlasMask);
    self->mp_rdRef->VT->DeleteResource(self->mp_rdRef, &self->mp_playerAtlas);
    self->mp_rdRef->VT->DeleteResource(self->mp_rdRef, &self->mp_plAtlasMask);

    /* Release renderer, IAL, and window. */
    self->mp_ialRef->VT->Release(self->mp_ialRef);
    self->mp_rdRef->VT->Release(self->mp_rdRef);
    self->mp_rdTarget->VT->Release(self->mp_rdTarget);
}

/**
 */
NK_INTERNAL NkVoid __NkInt_WorldLayer_ActionScreenshot(_Inout_ __NkInt_WorldLayer *self, char const *filePath) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(filePath != NULL && *filePath ^ '\0', NkErr_InParameter);

    NkErrorCode errCode;
    NkDIBitmap newScreenshot;

    /* Grab the pixel data from the framebuffer. */
    if ((errCode = self->mp_rdRef->VT->GrabFramebuffer(self->mp_rdRef, &newScreenshot)) != NkErr_Ok) {
        /* Failed to copy pixels. */
        NK_LOG_ERROR(
            "Failed to grab current framebuffer. Reason: %s (%i)",
            NkGetErrorCodeStr(errCode)->mp_dataPtr,
            (int)errCode
        );

        NkDIBitmapDestroy(&newScreenshot);
        return;
    }

    /*
     * Save the bitmap to the file. We use one filePath for now, can only save the
     * latest screenshot.
     * TODO: Generate random number or use timestamp for screenshot file names.
     */
    if ((errCode = NkDIBitmapSave(&newScreenshot, filePath)) != NkErr_Ok)
        /* Failed to write bitmap file to disk. */
        NK_LOG_ERROR(
            "Failed to write screenshot file to \"%s\". Reason: %s (%i)",
            filePath,
            NkGetErrorCodeStr(errCode)->mp_dataPtr,
            (int)errCode
        );

    NK_LOG_INFO("Successfully wrote screenshot \"%s\".", filePath);
    /* At last, delete bitmap. */
    NkDIBitmapDestroy(&newScreenshot);
}

/**
 */
NK_INTERNAL NkVec2F __NkInt_WorldLayer_GetAnimPos(__NkInt_WorldLayer *self) {
    NkFloat x = 0, y = 0;

    //if (self->m_isMoving) {
    //    // move 
    //    if (self->m_vel.m_xVal == 0.f && self->m_vel.m_yVal == 1.f)

    //} else {
    //    // always middle row
    //    x = 1.f;

    //NK_LOG_DEBUG("v = { %g, %g }", self->m_vel.m_xVal, self->m_vel.m_yVal);

        x = 1.f;
        y = (NkFloat)self->m_dir;
        //if (self->m_vel.m_xVal == 0.f && self->m_vel.m_yVal == 1.f)
        //    y = 0.f;
        //else if (self->m_vel.m_xVal == 1.f && self->m_vel.m_yVal == 0.f)
        //    y = 2.f;
        //else if (self->m_vel.m_xVal == 0.f && self->m_vel.m_yVal == -1.f)
        //    y = 3.f;
        //else if (self->m_vel.m_xVal == -1.f && self->m_vel.m_yVal == 0.f)
        //    y = 1.f;
        //else if (self->m_vel.m_xVal == 0.f && self->m_vel.m_yVal == 0.f) {
        //    if ()
        //}
    //}

    return (NkVec2F){ x, y };
}

/**
 */
NK_INTERNAL NkVoid __NkInt_WorldLayer_UpdAnim(__NkInt_WorldLayer *wl) {
    NK_INTERNAL NkFloat frame = 0.1f;
    NK_INTERNAL NkInt32 maxFrame = 2;
    NK_INTERNAL NkInt32 minFrame = 0;

    /* Anim must update. */
    //NkUint64 currTime = NkGetCurrentTime();
    //if (((NkFloat)currTime - wl->m_lastAnimFrame) >= frame * (NkFloat)NkGetTimerFrequency()) {
    //    switch ()
    //}
}


/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WorldLayer_AddRef(_Inout_ NkILayer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Stub. */
    return 1;
}

/**
 */
NK_INTERNAL NkOMRefCount NK_CALL __NkInt_WorldLayer_Release(_Inout_ NkILayer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(self);

    /* Same as above. */
    return 1;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_QueryInterface(
    _Inout_  NkILayer *self,
    _In_     NkUuid const *iId,
    _Outptr_ NkVoid **resPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(iId != NULL, NkErr_InParameter);
    NK_ASSERT(resPtr != NULL, NkErr_OutptrParameter);

    /* Define interface table for this class. */
    NK_INTERNAL NkOMImplementationInfo const gl_ImplInfos[] = {
        { NKOM_IIDOF(NkIBase)  },
        { NKOM_IIDOF(NkILayer) },
        { NULL                 }
    };

    if (NkOMQueryImplementationIndex(gl_ImplInfos, iId) != SIZE_MAX) {
        /* Interface is implemented. */
        *resPtr = (NkVoid *)self;

        __NkInt_WorldLayer_AddRef(self);
        return NkErr_Ok;
    }

    /* Interface not implemented. */
    *resPtr = NULL;
    return NkErr_InterfaceNotImpl;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnPush(
    _Inout_  NkILayer *self,
    _In_opt_ NkILayer const *beforeRef,
    _In_opt_ NkILayer const *afterRef,
    _In_     NkSize index
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(index);
    NK_UNREFERENCED_PARAMETER(beforeRef);
    NK_UNREFERENCED_PARAMETER(afterRef);

    /* Get internal structure of world layer. */
    __NkInt_WorldLayer *actWorldLayer = (__NkInt_WorldLayer *)self;

    /* Query renderer and create resources. */
    NkIWindow   *mainWnd = (NkIWindow *)NkApplicationQueryInstance(NKOM_CLSIDOF(NkIWindow));
    NkIRenderer *mainRd  = mainWnd->VT->GetRenderer(mainWnd);

    /* Create main resources. */
    NkDIBitmap mainTs, atlasTs;
    NkRendererResource *mainTsRes = NULL;
    NkErrorCode errCode = NkDIBitmapLoad("../res/def/ts_main.bmp", &mainTs);
    if (errCode != NkErr_Ok)
        goto lbl_ONERROR;
    if ((errCode = mainRd->VT->CreateTexture(mainRd, &mainTs, &mainTsRes)) != NkErr_Ok) {
        NkDIBitmapDestroy(&mainTs);

        goto lbl_ONERROR;
    }
    NkDIBitmapDestroy(&mainTs);

    NkDIBitmapLoad("../res/def/ts_player.bmp", &atlasTs);
    NkRendererResource *mainTsMask = NULL, *plAtlas = NULL, *plAtlasMask = NULL;
    errCode = mainRd->VT->CreateTextureMask(mainRd, mainTsRes, (NkRgbaColor)NK_MAKE_RGB(255, 255, 255), &mainTsMask);
    errCode = mainRd->VT->CreateTexture(mainRd, &atlasTs, &plAtlas);
    errCode = mainRd->VT->CreateTextureMask(mainRd, plAtlas, (NkRgbaColor)NK_MAKE_RGB(255, 0, 255), &plAtlasMask);
    NkDIBitmapDestroy(&atlasTs);

    /* Initialize instance. */
    *actWorldLayer = (__NkInt_WorldLayer){
        .NkILayer_Iface = actWorldLayer->NkILayer_Iface,

        .mp_rdTarget     = mainWnd,
        .mp_rdRef        = mainRd,
        .mp_ialRef       = (NkIInput *)NkApplicationQueryInstance(NKOM_CLSIDOF(NkIInput)),
        .mp_mainTexAtlas = mainTsRes,
        .mp_texAtlasMask = mainTsMask,
        .mp_playerAtlas  = plAtlas,
        .mp_plAtlasMask  = plAtlasMask,
        .m_prevPos       = (NkVec2F){ 11.f * 32.f , 9.f * 32.f },
        .m_playerPos     = (NkVec2F){ 11.f * 32.f , 9.f * 32.f },
        .m_targetPos     = (NkVec2F){ 11.f * 32.f , 9.f * 32.f },
        .m_isMoving      = NK_FALSE,
        .m_vel           = (NkVec2F) { 0.f, 1.f },
        .m_lvel          = (NkVec2F) { 0.f, 1.f },
        .m_moveSpeed     = 3.f * 32.f,
        .state           = 0 // idle
    };
    NkTimerCreate(NkTiType_Elapsed, NK_TRUE, &actWorldLayer->t);
    NkTimerCreate(NkTiType_Elapsed, NK_TRUE, &actWorldLayer->t2);
    /* All good. */
    return NkErr_Ok;

lbl_ONERROR:
    mainRd->VT->Release(mainRd);
    mainWnd->VT->Release(mainWnd);

    return errCode;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnPop(_Inout_ NkILayer *self) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Get internal structure of world layer. */
    __NkInt_WorldLayer *actWorldLayer = (__NkInt_WorldLayer *)self;

    /* Destroy resources. */
    //actWorldLayer->mp_rdRef->VT->DeleteResource(actWorldLayer->mp_rdRef, &actWorldLayer->mp_mainTexAtlas);
    /* Release components. */
    //actWorldLayer->mp_rdRef->VT->Release(actWorldLayer->mp_rdRef);
    //actWorldLayer->mp_rdTarget->VT->Release(actWorldLayer->mp_rdTarget);

    /* All good. */
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnEvent(
    _Inout_ NkILayer *self,
    _In_    NkEvent const *evPtr
) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_ASSERT(evPtr != NULL, NkErr_InParameter);

    /* Allow taking screenshots. */
    if (evPtr->m_evType == NkEv_KeyboardKeyDown && evPtr->m_kbEvent.m_vKeyCode == NkKey_F11) {
        __NkInt_WorldLayer_ActionScreenshot((__NkInt_WorldLayer *)self, "latestScreenshot.bmp");

        /* Event was handled. */
        return NkErr_Ok;
    } else if (evPtr->m_evType == NkEv_KeyboardKeyDown && evPtr->m_kbEvent.m_vKeyCode == NkKey_F4) {
        NkApplicationExit(NkErr_Ok);

        return NkErr_Ok;
    }

    /* Event was not handled. */
    return NkErr_NoOperation;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnUpdate(_Inout_ NkILayer *self, _In_ NkFloat updTime) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);

    /* Get internal structure of world layer. */
    __NkInt_WorldLayer *actWorldLy = (__NkInt_WorldLayer *)self;

    /* Update move speed. */
    if (actWorldLy->mp_ialRef->VT->IsKeyPressed(actWorldLy->mp_ialRef, NkKey_LShift))
        actWorldLy->m_moveSpeed = 8.f * 32.f;
    else
        actWorldLy->m_moveSpeed = 4.f * 32.f;

    /* Update positions. */
    if (actWorldLy->m_isMoving == NK_FALSE) {
        /* Get axis movement. */
        NkFloat axisX = actWorldLy->mp_ialRef->VT->IsKeyPressed(actWorldLy->mp_ialRef, NkKey_AlnumA) ? -1.f : (actWorldLy->mp_ialRef->VT->IsKeyPressed(actWorldLy->mp_ialRef, NkKey_AlnumD) ? 1.f : 0.f);
        NkFloat axisY = actWorldLy->mp_ialRef->VT->IsKeyPressed(actWorldLy->mp_ialRef, NkKey_AlnumW) ? -1.f : (actWorldLy->mp_ialRef->VT->IsKeyPressed(actWorldLy->mp_ialRef, NkKey_AlnumS) ? 1.f : 0.f);
        if (axisX != 0.f) axisY = 0.f;

        if (actWorldLy->m_vel.m_xVal != axisX || actWorldLy->m_vel.m_yVal != axisY) {
            actWorldLy->m_vel = (NkVec2F){ axisX, axisY };
        }
        if (actWorldLy->m_vel.m_xVal != 0.f || actWorldLy->m_vel.m_yVal != 0.f) {
            // Update dir.
            if (actWorldLy->m_vel.m_xVal == 1.f)
                actWorldLy->m_dir = 2;
            else if (actWorldLy->m_vel.m_xVal == -1.f)
                actWorldLy->m_dir = 1;
            else if (actWorldLy->m_vel.m_yVal == 1.f)
                actWorldLy->m_dir = 0;
            else if (actWorldLy->m_vel.m_yVal == -1.f)
                actWorldLy->m_dir = 3;

            actWorldLy->m_targetPos = (NkVec2F){
                actWorldLy->m_playerPos.m_xVal + axisX * 32.f,
                actWorldLy->m_playerPos.m_yVal + axisY * 32.f
            };
            /* Fix positions so they remain in the map. */
            //actWorldLy->m_targetPos.m_xVal = NK_CLAMP(actWorldLy->m_targetPos.m_xVal, 0, 31 * 32);
            //actWorldLy->m_targetPos.m_yVal = NK_CLAMP(actWorldLy->m_targetPos.m_yVal, 0, 31 * 32);

            actWorldLy->m_isMoving = NK_TRUE;
            NkTimerRestart(&actWorldLy->t);
        }
    }

    if (actWorldLy->m_isMoving == NK_TRUE) {
        NkVec2F diffVec = {
            actWorldLy->m_targetPos.m_xVal - actWorldLy->m_playerPos.m_xVal,
            actWorldLy->m_targetPos.m_yVal - actWorldLy->m_playerPos.m_yVal
        };
        NkFloat mag = sqrtf(diffVec.m_xVal * diffVec.m_xVal + diffVec.m_yVal * diffVec.m_yVal);
        if ((diffVec.m_xVal * diffVec.m_xVal + diffVec.m_yVal * diffVec.m_yVal) > 0.001) {
            if(mag < actWorldLy->m_moveSpeed * updTime || mag == 0.f)
                goto lbl_HERE;

            actWorldLy->m_prevPos = actWorldLy->m_playerPos;
            actWorldLy->m_playerPos = (NkVec2F){
                actWorldLy->m_playerPos.m_xVal + diffVec.m_xVal / mag * (actWorldLy->m_moveSpeed * updTime),
                actWorldLy->m_playerPos.m_yVal + diffVec.m_yVal / mag * (actWorldLy->m_moveSpeed * updTime)
            };

            /* Fix positions so they remain in the map. */
            //actWorldLy->m_playerPos.m_xVal = NK_CLAMP(actWorldLy->m_playerPos.m_xVal, 0, 31 * 32);
            //actWorldLy->m_playerPos.m_yVal = NK_CLAMP(actWorldLy->m_playerPos.m_yVal, 0, 31 * 32);
        } else {
        lbl_HERE:
            actWorldLy->m_playerPos = actWorldLy->m_targetPos;
            actWorldLy->m_isMoving = NK_FALSE;

            //NK_LOG_INFO("Move took %g ms", NkElapsedTimerGetAs(&actWorldLy->t, NkTiPrec_Milliseconds));
            /* Fix positions so they remain in the map. */
            //actWorldLy->m_playerPos.m_xVal = NK_CLAMP(actWorldLy->m_playerPos.m_xVal, 0, 31 * 32);
            //actWorldLy->m_playerPos.m_yVal = NK_CLAMP(actWorldLy->m_playerPos.m_yVal, 0, 31 * 32);
        }
    }

    __NkInt_WorldLayer_UpdAnim(actWorldLy);
    return NkErr_Ok;
}

/**
 */
NK_INTERNAL _Return_ok_ NkErrorCode NK_CALL __NkInt_WorldLayer_OnRender(_Inout_ NkILayer *self, _In_ NkFloat aheadBy) {
    NK_ASSERT(self != NULL, NkErr_InOutParameter);
    NK_UNREFERENCED_PARAMETER(aheadBy);

    /* Get internal structure of world layer. */
    __NkInt_WorldLayer *actWorldLy = (__NkInt_WorldLayer *)self;
    /* Get viewport dimensions. */
    NkSize2D vpDim = actWorldLy->mp_rdTarget->VT->GetClientDimensions(actWorldLy->mp_rdTarget);

    /* Interpolate player position. */
    //NkVec2F actPlPos = (NkVec2F){
    //    actWorldLy->m_playerPos.m_xVal * aheadBy + actWorldLy->m_prevPos.m_xVal * (1.f - aheadBy),
    //    actWorldLy->m_playerPos.m_yVal * aheadBy + actWorldLy->m_prevPos.m_yVal * (1.f - aheadBy)
    //};
    NkVec2F actPlPos = (NkVec2F){
        actWorldLy->m_playerPos.m_xVal,
        actWorldLy->m_playerPos.m_yVal
    };

    int orix = -(int)actPlPos.m_xVal % 32 - 8 * 32;
    int oriy = -(int)actPlPos.m_yVal % 32 - 8 * 32;
    int xdim = NK_MIN(16 * 32, vpDim.m_width) - 8 * 32;
    int ydim = NK_MIN(16 * 32, vpDim.m_height) - 8 * 32;

    /* Draw world. */
    for (int x = orix, tsx = (int)actPlPos.m_xVal / 32 * 32 - 8 * 32; x < xdim; x += 32, tsx += 32)
        for (int y = oriy, tsy = (int)actPlPos.m_yVal / 32 * 32 - 8 * 32; y < ydim; y += 32, tsy += 32) {
            /* Skip tiles that are out of range. */
            if (tsx < 0 || tsx > 31 * 32 || tsy < 0 || tsy > 31 * 32)
                continue;

            /* Get chunk. */
            int ix = tsx / 32 / 16;
            int iy = tsy / 32 / 16;
            NkUint32 *c = ix == 0 ? (iy == 0 ? gl_TestMap1 : gl_TestMap3) : (iy == 0 ? gl_TestMap2 : gl_TestMap4);
                
            /* Draw a tile. */
            actWorldLy->mp_rdRef->VT->DrawTexture(
                actWorldLy->mp_rdRef,
                &(NkRectF){
                    .m_xCoord = x + 8 * 32,
                    .m_yCoord = y + 8 * 32,
                    .m_width  = 32,
                    .m_height = 32 
                },
                actWorldLy->mp_mainTexAtlas,
                &(NkRectF){ 
                    .m_xCoord = 32 * (c[16 * ((tsy / 32) % 16) + (tsx / 32) % 16] & 0xFFFF),
                    .m_yCoord = 32 * (c[16 * ((tsy / 32) % 16) + (tsx / 32) % 16] >> 16),
                    .m_width  = 32,
                    .m_height = 32
                }
            );
        }

    NkVec2F charFrame = __NkInt_WorldLayer_GetAnimPos(actWorldLy);
    actWorldLy->mp_rdRef->VT->DrawMaskedTexture(actWorldLy->mp_rdRef, &(NkRectF){ 8 * 32, 8 * 32, 32, 32 }, actWorldLy->mp_playerAtlas, (NkVec2F){ charFrame.m_xVal * 32.f, charFrame.m_yVal * 32.f }, actWorldLy->mp_plAtlasMask, (NkVec2F){ charFrame.m_xVal * 32.f, charFrame.m_yVal * 32.f });

    /* All good. */
    return NkErr_Ok;
}


/**
 * \brief actual world layer instance
 */
NK_INTERNAL __NkInt_WorldLayer gl_WorldLayer = {
    .NkILayer_Iface = {
        .VT = &(struct __NkILayer_VTable__){
            .AddRef         = &__NkInt_WorldLayer_AddRef,
            .Release        = &__NkInt_WorldLayer_Release,
            .QueryInterface = &__NkInt_WorldLayer_QueryInterface,
            .OnPush         = &__NkInt_WorldLayer_OnPush,
            .OnPop          = &__NkInt_WorldLayer_OnPop,
            .OnEvent        = &__NkInt_WorldLayer_OnEvent,
            .OnUpdate       = &__NkInt_WorldLayer_OnUpdate,
            .OnRender       = &__NkInt_WorldLayer_OnRender
        }
    },
    .mp_rdTarget     = NULL,
    .mp_rdRef        = NULL,
    .mp_mainTexAtlas = NULL
};


_Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_STARTUPFN(WorldLayer)(NkVoid) {
    /*
     * Push the world layer to the end of the layer stack so that it's updated last but
     * rendered first.
     */
    return NkLayerstackPush((NkILayer *)&gl_WorldLayer, NK_AS_NORMAL);
}

_Return_ok_ NkErrorCode NK_CALL NK_COMPONENT_SHUTDOWNFN(WorldLayer)(NkVoid) {
    /* Retrieve the layer index. */
    NkSize worldLayerIndex = NkLayerstackQueryIndex((NkILayer *)&gl_WorldLayer);
    if (worldLayerIndex == SIZE_MAX)
        return NkErr_ArrayElemOutOfBounds;

    /* Delete all resources. */
    __NkInt_WorldLayer_DeleteResources((__NkInt_WorldLayer *)NkLayerstackPop(worldLayerIndex));
    return NkErr_Ok;
}


/**
 */
NK_COMPONENT_DEFINE(WorldLayer) {
    .m_compUuid     = { 0x6e6c7be8, 0xedf7, 0x494d, 0x8f61fb1abf5aa80b },
    .mp_clsId       = NULL,
    .m_compIdent    = NK_MAKE_STRING_VIEW("world layer"),
    .m_compFlags    = 0,
    .m_isNkOM       = NK_FALSE,

    .mp_fnQueryInst = NULL,
    .mp_fnStartup   = &NK_COMPONENT_STARTUPFN(WorldLayer),
    .mp_fnShutdown  = &NK_COMPONENT_SHUTDOWNFN(WorldLayer)
};
/** \endcond */


#undef NK_NAMESPACE


