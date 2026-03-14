#ifndef MATERIAL_EDITOR_PLATFORM_H
#define MATERIAL_EDITOR_PLATFORM_H

#include <stdbool.h>

#include "result.h"

#ifdef __linux__
#include <X11/Xlib.h>
#include <GL/glx.h>
#endif

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
};

material_editor_result platform_initialize(struct platform *platform);

bool platform_update(struct platform *platform);

void platform_swap_buffers(const struct platform *platform);

void platform_quit(const struct platform *platform);

#endif //MATERIAL_EDITOR_PLATFORM_H
