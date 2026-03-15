#include <string.h>
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
        .event_mask = ExposureMask | KeyPressMask | StructureNotifyMask | ButtonPressMask,
    };

    const int width = 1280, height = 720;
    const int x = 0, y = 0;
    const int border = 0;

    platform->window = XCreateWindow(display, root, x, y, width, height, border, visual_info->depth, InputOutput,
                                     visual_info->visual, CWColormap | CWEventMask, &set_window_attributes);

    XStoreName(display, platform->window, "Material Editor");
    XSelectInput(display, platform->window, ExposureMask | KeyPressMask | StructureNotifyMask | ButtonPressMask);
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
    platform->text_input_length = 0;
    platform->mouse = false;
    platform->alt = false;
    memset(platform->key_pressed, 0, sizeof(platform->key_pressed));

    while (XPending(platform->display)) {
        XEvent event;
        XNextEvent(platform->display, &event);

        switch (event.type) {
            case ConfigureNotify: {
                platform->width = event.xconfigure.width;
                platform->height = event.xconfigure.height;

                const int x = 0, y = 0;
                gl_set_viewport(x, y, platform->width, platform->height);
            }
            break;

            case ClientMessage: {
                if ((Atom) event.xclient.data.l[0] == platform->wm_delete_window) {
                    return false;
                }
            }
            break;

            case ButtonPress: {
                if (event.xbutton.button == Button1) {
                    platform->mouse = true;
                    platform->mx = event.xbutton.x;
                    platform->my = event.xbutton.y;
                }
            }
            break;

            case KeyPress: {
                platform->alt = (event.xkey.state & Mod1Mask) != 0;

                const KeySym sym = XLookupKeysym(&event.xkey, 0);

                switch (sym) {
                    case XK_BackSpace:
                        platform->key_pressed[KEY_BACKSPACE] = true;
                        break;
                    case XK_Return:
                        platform->key_pressed[KEY_ENTER] = true;
                        break;
                    case XK_Tab:
                        platform->key_pressed[KEY_TAB] = true;
                        break;
                    case XK_Left:
                        platform->key_pressed[KEY_LEFT] = true;
                        break;
                    case XK_Right:
                        platform->key_pressed[KEY_RIGHT] = true;
                        break;
                    case XK_Up:
                        platform->key_pressed[KEY_UP] = true;
                        break;
                    case XK_Down:
                        platform->key_pressed[KEY_DOWN] = true;
                        break;

                    default: {
                        // get text input
                        char buffer[8];
                        const int length = XLookupString(&event.xkey, buffer, sizeof(buffer), NULL, NULL);

                        for (int i = 0; i < length; i++) {
                            const char c = buffer[i];

                            if (c >= 32 && c < 127) {
                                if (platform->text_input_length < MAX_TEXT_INPUT) {
                                    platform->text_input[platform->text_input_length++] = c;
                                }
                            }
                        }
                    }
                    break;
                }
            }
            break;

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
