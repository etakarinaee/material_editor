#ifndef MATERIAL_EDITOR_PLATFORM_H
#define MATERIAL_EDITOR_PLATFORM_H

#include <stdbool.h>

#ifdef __linux__
#include <X11/Xlib.h>
#endif

struct platform {
#ifdef __linux__
    Display *display;
    Window window;
    // to handle the polite quit request properly and not be killed by SIGTERM
    Atom wm_delete_window;
#endif

    int width;
    int height;
};

#define PLATFORM_SUCCESS 0

#define PLATFORM_ERROR_UNKNOWN (-1)
#define PLATFORM_ERROR_OUT_OF_MEMORY (-2)

typedef int platform_result;

const char *platform_result_to_string(platform_result result);

platform_result platform_initialize(struct platform *platform);

bool platform_update(struct platform *platform);

void platform_quit(const struct platform *platform);

#endif //MATERIAL_EDITOR_PLATFORM_H
