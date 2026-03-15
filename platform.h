#ifndef MATERIAL_EDITOR_PLATFORM_H
#define MATERIAL_EDITOR_PLATFORM_H

#include <stdbool.h>

#include "result.h"

#ifdef __linux__
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>
#endif

#define MAX_TEXT_INPUT 32

enum key {
    KEY_BACKSPACE,
    KEY_ENTER,
    KEY_TAB,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
    // this always must be last in the enum
    KEY_COUNT,
};

struct gl {
#ifdef __linux__
    GLXContext context;
#endif
};

struct platform {
#ifdef __linux__
    Display *display;
    Window window;
    // to handle the polite quit request properly and not be killed by SIGTERM
    Atom wm_delete_window;
    Colormap colormap;
#endif

    struct gl gl;

    int width;
    int height;

    char text_input[MAX_TEXT_INPUT];
    int text_input_length;
    bool key_pressed[KEY_COUNT];

    bool mouse;
    int mx, my;
};

material_editor_result platform_initialize(struct platform *platform);

bool platform_update(struct platform *platform);

void platform_swap_buffers(const struct platform *platform);

void platform_quit(const struct platform *platform);

#endif //MATERIAL_EDITOR_PLATFORM_H
