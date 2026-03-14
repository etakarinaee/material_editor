#include <GL/glew.h>

#include "platform.h"

#include <X11/Xlib.h>

#include "gl.h"

material_editor_result platform_initialize(struct platform *platform) {
    const char *name = 0;
    Display *display = XOpenDisplay(name);

    if (!display) {
        return RESULT_X11_ERROR_OPEN_DISPLAY;
    }

    platform->display = display;

    const int screen = DefaultScreen(display);
    const Window root = RootWindow(display, screen);

    static const int visual_attributes[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        GLX_SAMPLE_BUFFERS, 1,
        GLX_SAMPLES, 4,
        None
    };

    int framebuffers;
    GLXFBConfig *framebuffer_configs = glXChooseFBConfig(display, screen, visual_attributes, &framebuffers);

    if (!framebuffer_configs || framebuffers == 0) {
        XCloseDisplay(display);

        return RESULT_GL_ERROR_NO_FRAMEBUFFER_CONFIG;
    }

    const GLXFBConfig framebuffer_config = framebuffer_configs[0];
    XFree(framebuffer_configs);

    XVisualInfo *visual_info = glXGetVisualFromFBConfig(display, framebuffer_config);

    if (!visual_info) {
        XCloseDisplay(display);

        return RESULT_GL_ERROR_NO_VISUAL;
    }

    platform->colormap = XCreateColormap(display, root, visual_info->visual, AllocNone);

    XSetWindowAttributes set_window_attributes = {
        .colormap = platform->colormap,
        .event_mask = ExposureMask | KeyPressMask | StructureNotifyMask,
    };

    const int width = 1280, height = 720;
    const int x = 0, y = 0;
    const int border = 0;

    platform->window = XCreateWindow(display, root, x, y, width, height, border, visual_info->depth, InputOutput,
                                     visual_info->visual, CWColormap | CWEventMask, &set_window_attributes);

    XStoreName(display, platform->window, "Material Editor");
    XSelectInput(display, platform->window, ExposureMask | KeyPressMask | StructureNotifyMask);
    XMapWindow(display, platform->window);

    const int only_if_exists = False;
    platform->wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", only_if_exists);

    const int count = 1;
    XSetWMProtocols(display, platform->window, &platform->wm_delete_window, count);

    typedef GLXContext (*glXCreateContextAttribsARBProc)(
        Display *, GLXFBConfig, GLXContext, Bool, const int *
    );

    const glXCreateContextAttribsARBProc glXCreateContextAttribsARB =
            (glXCreateContextAttribsARBProc) glXGetProcAddressARB(
                (const GLubyte *) "glXCreateContextAttribsARB"
            );

    const GLXContext share = NULL;
    const int direct = True;

    if (glXCreateContextAttribsARB) {
        static const int context_attributes[] = {
            GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
            GLX_CONTEXT_MINOR_VERSION_ARB, 3,
            GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
            None
        };

        platform->gl.context = glXCreateContextAttribsARB(
            display, framebuffer_config, share, direct, context_attributes
        );
    } else {
        const int render_type = GLX_RGBA_TYPE;

        platform->gl.context = glXCreateNewContext(
            display, framebuffer_config, render_type, share, direct
        );
    }

    if (!platform->gl.context) {
        XDestroyWindow(display, platform->window);
        XFreeColormap(display, platform->colormap);
        XCloseDisplay(display);

        return RESULT_GL_ERROR_CREATE_CONTEXT;
    }

    glXMakeCurrent(display, platform->window, platform->gl.context);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        glXDestroyContext(display, platform->gl.context);
        XDestroyWindow(display, platform->window);
        XFreeColormap(display, platform->colormap);
        XCloseDisplay(display);

        return RESULT_GL_ERROR_GLEW_INIT;
    }

    // clear all errors GLEW produced
    while (glGetError() != GL_NO_ERROR) {
    }

    glEnable(GL_DEPTH_TEST);

    platform->width = width;
    platform->height = height;

    return RESULT_SUCCESS;
}

bool platform_update(struct platform *platform) {
    while (XPending(platform->display)) {
        XEvent event;
        XNextEvent(platform->display, &event);

        switch (event.type) {
            case ConfigureNotify: {
                platform->width = event.xconfigure.width;
                platform->height = event.xconfigure.height;
                gl_set_viewport(platform->width, platform->height);
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

void platform_swap_buffers(const struct platform *platform) {
    glXSwapBuffers(platform->display, platform->window);
}

void platform_quit(const struct platform *platform) {
    XDestroyWindow(platform->display, platform->window);
    XCloseDisplay(platform->display);
}
