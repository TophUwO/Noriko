#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>



/* Demonstrating nested preprocessor directives. */         /* 11. */
#if (defined NK_LEVEL_1_PREPROC)
    #if (defined NK_LEVEL_2_PREPROC)                        /* 7. */
        #define NK_LEVEL (3)                                /* 14. */

        #include <windows.h>
    #elif (defined _DEBUG)
        #error Cannot compile in debug mode.
    #endif
#endif
#define NK_ANNOTATION


/* Demonstrating global variables. */
NK_ANNOTATION static int const gl_c_displayWidth = 1080;    /* 15. */


/* Demonstrating types. */
typedef int NkErrorCode;

/**
 * \brief This is a DOXYGEN-style comment.
 *
 * This is the detailed description for this declaration.
 *
 * \note  This is a note.
 */
typedef struct NkStaticApplicationContext {                 /* 2. */
    uint32_t  m_uintVal;                                    /* 16. */
    uint32_t *mp_uintPtrVal;                                /* 4. & 16. */

    void (__cdecl *getObject)(char const *const id);        /* 4. & 15. */
};


/* Demonstrating enumerations. */
typedef enum NkErrorCode {                                  /* 2. */
    NkErr_Ok,
    NkErr_Unknown,

    NkErr_NotImplemented,
    NkErr_OutOfMemory,
    NkErr_DeviceLost
} NkErrorCode;


/* Demonstrating functions. */
NK_ANNOTATION inline NkErrorCode NkCreateWindow(            /* 1. & 15. */
    uint32_t width,
    uint32_t height,
    uint8_t bbp,
    uint32_t twidth,
    uint32_t theight,
    float scale,
    bool vsync
) {
    /* Validate parameters. */
    if (!width || !height)                                  /* 3. & 8. */
        return NkErr_NotImplemented;

    /* Do stuff. */
    void *ptr = calloc(1, sizeof width * 32);               /* 4. & 9. & 10. & 13. */
    if (!ptr)
        return NkErr_OutOfMemory;
    else if (ptr == 0x01) {                                 /* 5. */
        printf("Pointer was 0x01.\n");

        goto lbl_ERR;                                       /* 9. */
    }

    /* Clean up and return. */
    free(ptr);
    return NkErr_Ok;

    /* Clean up in case of error. */
lbl_ERR:                                                    /* 17. */
    free(ptr);
    return NkErr_Unknown;
}
