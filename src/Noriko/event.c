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
 * \file  event.c
 * \brief implements the API for the Noriko event system
 *
 * User input and asynchronous message handling is done through a message-pump, 'pumping
 * out' and dispatching events to the layer stack. Each layer gets the chance to process
 * the event. If the layer chooses to process the event, layers underneath it in the
 * stack will not receive the event. If the layer does not process the event, the event
 * is propagated through the layer stack until the entire stack has been traversed or the
 * event has been handled.<br>
 * 
 * \note  Note that this event system is for application- and host-system events, not for
 *        in-game events.
 */
#define NK_NAMESPACE "nk::event"


/* stdlib includes */
#include <string.h>
#include <stdarg.h>

/* Noriko includes */
#include <include/Noriko/event.h>
#include <include/Noriko/timer.h>
#include <include/Noriko/layer.h>


/** \cond INTERNAL */
/**
 * \struct __NkInt_EventTypeInfo
 * \brief  represents the event type info, that is, the categories the event type is
 *         associated, whether or not extra data is required when creating the event, and
 *         more
 */
NK_NATIVE typedef struct __NkInt_EventTypeInfo { 
    NkEventType     m_evType;     /**< numeric event type */
    NkEventCategory m_evCat;      /**< numeric event categories the event type is associated to */
    NkSize          m_dataSize;   /**< size of the (optional) additional data */
    NkBoolean       m_expectData; /**< whether or not data is required for the event type */
} __NkInt_EventTypeInfo;


/**
 * \brief represents the static event type table, containing information on the events
 *        which are used by the event system when creating and dispatching events
 */
NK_INTERNAL __NkInt_EventTypeInfo const gl_c_EvTypeTbl[] = {
    /* technical events */
    { NkEv_None,                NkEvCat_None,                     0,                       NK_FALSE },

    /* window events */                                           
    { NkEv_WindowClosed,        NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowGotFocus,      NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowLostFocus,     NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowResized,       NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowMinimized,     NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowMaximized,     NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowRestored,      NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowMoved,         NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowFullscreen,    NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowShown,         NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },
    { NkEv_WindowHidden,        NkEvCat_Window,                   sizeof(NkWindowEvent),   NK_TRUE  },

    /* keyboard events */
    { NkEv_KeyboardKeyDown,     NkEvCat_Input | NkEvCat_Keyboard, sizeof(NkKeyboardEvent), NK_TRUE  },
    { NkEv_KeyboardKeyUp,       NkEvCat_Input | NkEvCat_Keyboard, sizeof(NkKeyboardEvent), NK_TRUE  },
    { NkEv_KeyboardKeyRepeated, NkEvCat_Input | NkEvCat_Keyboard, sizeof(NkKeyboardEvent), NK_TRUE  },

    /* mouse events */
    { NkEv_MouseButtonDown,     NkEvCat_Input | NkEvCat_Mouse,    sizeof(NkMouseEvent),    NK_TRUE  },
    { NkEv_MouseButtonUp,       NkEvCat_Input | NkEvCat_Mouse,    sizeof(NkMouseEvent),    NK_TRUE  },
    { NkEv_MouseMoved,          NkEvCat_Input | NkEvCat_Mouse,    sizeof(NkMouseEvent),    NK_TRUE  },
    { NkEv_MouseScrollUp,       NkEvCat_Input | NkEvCat_Mouse,    sizeof(NkMouseEvent),    NK_TRUE  },
    { NkEv_MouseScrollDown,     NkEvCat_Input | NkEvCat_Mouse,    sizeof(NkMouseEvent),    NK_TRUE  }
};
/* Verify table integrity. */
static_assert(
    NK_ARRAYSIZE(gl_c_EvTypeTbl) == __NkEv_Count__,
    "Mismatch between event type enumeration and event type info table. Check definitions."
);
/** \endcond */


_Return_ok_ NkErrorCode NK_CALL NkEventDispatch(_In_ NkEventType evType, ...) {
    NK_ASSERT(evType > NkEv_None && evType < __NkEv_Count__, NkErr_InParameter);

    /* Construct the event. */
    NkEvent specEvent = (NkEvent){
        .m_evType    = evType,
        .m_evCat     = gl_c_EvTypeTbl[evType].m_evCat,
        .m_timestamp = NkGetCurrentTime()
    };
    /*
     * If the event requires additional data, query the parameter after the event type.
     * Then, if no such parameter was provided, the behavior is undefined.
     */
    if (gl_c_EvTypeTbl[evType].m_expectData == NK_TRUE) {
        va_list vlArgs;
        va_start(vlArgs, evType);

        /* Copy the event data from the provided parameter. */
        memcpy((void *)&specEvent.m_wndEvent, va_arg(vlArgs, void const *), gl_c_EvTypeTbl[evType].m_dataSize);
        va_end(vlArgs);
    }

    /* Finally, dispatch the event. */
    return NkLayerstackOnEvent(&specEvent);
}

NkVoid NK_CALL NkEventCopy(_In_ NkEvent const *srcPtr, _Out_ NkEvent *dstPtr) {
    NK_ASSERT(srcPtr != NULL, NkErr_InParameter);
    NK_ASSERT(dstPtr != NULL, NkErr_OutParameter);

    /*
     * Copy using memcpy is enough since the event user-data does not contain any
     * pointers, i.e., is definitive.
     */
    memcpy(dstPtr, srcPtr, sizeof *srcPtr);
}


NkStringView const *NK_CALL NkEventQueryTypeString(_In_ NkEventType evType) {
    /** \cond INTERNAL */
    /**
     * \brief contains the string representations of the numeric event type IDs for when
     *        using a debugger/logger
     */
    NK_INTERNAL NkStringView const gl_c_EventTypeStrings[] = {
        /* technical events */
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_None)),            

        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowClosed)),    
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowGotFocus)),  
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowLostFocus)),
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowResized)),      
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowMinimized)),    
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowMaximized)),    
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowRestored)),     
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowMoved)),        
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowFullscreen)),          
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowShown)),        
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_WindowHidden)),       

        /* keyboard events */
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_KeyboardKeyDown)),    
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_KeyboardKeyUp)),      
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_KeyboardKeyRepeated)),

        /* mouse events */
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_MouseButtonDown)),    
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_MouseButtonUp)),      
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_MouseMoved)),         
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_MouseScrollUp)),      
        NK_MAKE_STRING_VIEW(NK_ESC(NkEv_MouseScrollDown))    
    };
    /* Verify table integrity. */
    static_assert(
        NK_ARRAYSIZE(gl_c_EventTypeStrings) == __NkEv_Count__,
        "Mismatch between event type enumeration and event type string table. Check definitions."
    );
    /** \endcond */

    return &gl_c_EventTypeStrings[evType];
}

NkStringView const *NK_CALL NkEventQueryCategoryString(_In_ NkEventCategory evCat) {
    NK_ASSERT(evCat >= 0 && (evCat & (evCat - 1)) == 0 && evCat < __NkEvCat_Count__, NkErr_InParameter);

    /** \cond INTERNAL */
    /**
     * \brief contains the string representations for the event categories (for use with
     *        debugger/logging, ...)
     */
    NK_INTERNAL NkStringView const gl_c_EventCategoryStrings[] = {
        [NkEvCat_None]        = NK_MAKE_STRING_VIEW(NK_ESC(NkEvCat_None)),

        [NkEvCat_Application] = NK_MAKE_STRING_VIEW(NK_ESC(NkEvCat_Application)),
        [NkEvCat_Window]      = NK_MAKE_STRING_VIEW(NK_ESC(NkEvCat_Window)),     
        [NkEvCat_Input]       = NK_MAKE_STRING_VIEW(NK_ESC(NkEvCat_Input)),      
        [NkEvCat_Keyboard]    = NK_MAKE_STRING_VIEW(NK_ESC(NkEvCat_Keyboard)),   
        [NkEvCat_Mouse]       = NK_MAKE_STRING_VIEW(NK_ESC(NkEvCat_Mouse)),      
        [NkEvCat_User]        = NK_MAKE_STRING_VIEW(NK_ESC(NkEvCat_User))
    };
    /** \endcond */

    return &gl_c_EventCategoryStrings[evCat];
}

NkEventCategory NK_CALL NkEventQueryCategories(_In_ NkEventType evType) {
    NK_ASSERT(evType > NkEv_None && evType < __NkEv_Count__, NkErr_InParameter);

    return gl_c_EvTypeTbl[evType].m_evCat;
}


#undef NK_NAMESPACE


