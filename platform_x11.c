#include "platform.h"

#include <X11/Xlib.h>

#define PLATFORM_ERROR_OPEN_DISPLAY (-100)

const char *platform_result_to_string(const platform_result result) {
    switch (result) {
        case PLATFORM_ERROR_OPEN_DISPLAY: {
            return "PLATFORM_ERROR_OPEN_DISPLAY: could not open X11 display";
        }
        default: return "unknown error";
    }
}

platform_result platform_initialize(struct platform *platform) {
    const char *name = 0;
    Display *display = XOpenDisplay(name);

    if (!display) {
        return PLATFORM_ERROR_OPEN_DISPLAY;
    }

    platform->display = display;

    const int screen = DefaultScreen(display);
    const Window root = RootWindow(display, screen);

    const int width = 1280, height = 720;
    const int x = 0, y = 0;
    const int border = 0;

    platform->window = XCreateSimpleWindow(display, root, x, y, width, height, border, BlackPixel(display, screen),
                                           BlackPixel(display, screen));

    XStoreName(display, platform->window, "Material Editor");
    XSelectInput(display, platform->window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(display, platform->window);

    const int only_if_exists = False;
    platform->wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", only_if_exists);

    const int count = 1;
    XSetWMProtocols(display, platform->window, &platform->wm_delete_window, count);

    platform->width = width;
    platform->height = height;

    return PLATFORM_SUCCESS;
}

bool platform_update(struct platform *platform) {
    while (XPending(platform->display)) {
        XEvent event;
        XNextEvent(platform->display, &event);

        switch (event.type) {
            case ConfigureNotify: {
                platform->width = event.xconfigure.width;
                platform->height = event.xconfigure.height;
            }
            break;

            case ClientMessage: {
                if ((Atom) event.xclient.data.l[0] == platform->wm_delete_window) {
                    return false;
                }
            }

            default: break;
        }
    }

    return true;
}

void platform_quit(const struct platform *platform) {
    XDestroyWindow(platform->display, platform->window);
    XCloseDisplay(platform->display);
}
