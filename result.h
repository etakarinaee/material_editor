#ifndef MATERIAL_EDITOR_RESULT_H
#define MATERIAL_EDITOR_RESULT_H

typedef int material_editor_result;

#define RESULT_SUCCESS 0

#define RESULT_ERROR_UNKNOWN (-1)
#define RESULT_ERROR_OUT_OF_MEMORY (-2)

#define RESULT_GL_ERROR_NO_FRAMEBUFFER_CONFIG (-100)
#define RESULT_GL_ERROR_CREATE_CONTEXT (-101)
#define RESULT_GL_ERROR_GLEW_INIT (-102)
#define RESULT_GL_ERROR_NO_VISUAL (-103)

#define RESULT_X11_ERROR_OPEN_DISPLAY (-200)

static inline const char *result_to_string(const material_editor_result result) {
    switch (result) {
        case RESULT_SUCCESS: {
            return "";
        }

        case RESULT_ERROR_OUT_OF_MEMORY: {
            return "out of memory";
        }

        case RESULT_X11_ERROR_OPEN_DISPLAY: {
            return "could not open X11 display";
        }

        case RESULT_GL_ERROR_NO_FRAMEBUFFER_CONFIG: {
            return "no suitable framebuffer config available";
        }

        case RESULT_GL_ERROR_CREATE_CONTEXT: {
            return "could not create GL context";
        }

        case RESULT_GL_ERROR_GLEW_INIT: {
            return "could not initialize GLEW";
        }

        case RESULT_GL_ERROR_NO_VISUAL: {
            return "could not find visual info";
        }

        default: return "an unknown error has occurred";
    }
}

#endif //MATERIAL_EDITOR_RESULT_H
