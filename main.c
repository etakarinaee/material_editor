#include <stdio.h>
#include <stdlib.h>

#include "gl.h"
#include "platform.h"

int main(void) {
    struct platform platform;

    material_editor_result result = platform_initialize(&platform);

    if (result != RESULT_SUCCESS) {
        fprintf(stderr, "%s\n", result_to_string(result));
    }

    gl_initialize();

    struct gl_font font;

    result = gl_font_initialize(&font, "JetBrainsMono-Regular.ttf", 48);

    if (result != RESULT_SUCCESS) {
        fprintf(stderr, "%s\n", result_to_string(result));

        return EXIT_FAILURE;
    }

    while (platform_update(&platform)) {
        gl_clear(0.1f, 0.1f, 0.1f, GL_OPAQUE);

        // preview has its own part of the screen
        const int x = platform.width / 2;
        const int width = platform.width / 2;
        const float aspect = (float) width / (float) platform.height;

        gl_set_viewport(x, 0, width, platform.height);
        gl_begin(vector3(0, 0, 3), vector3(0, 0, 0), aspect);
        gl_draw_mesh1(vector3(0, 0, 0), 0.5f, vector3(0.4f, 0.6f, 1.0f));

        gl_set_viewport(0, 0, platform.width, platform.height);
        gl_font_draw(&font, "Hello, World! Blah Blah kl; I hope this is fine Very good and Cool and niceee", 25.0f,
                     25.0f, 0.4f,
                     vector3(1.0f, 1.0f, 1.0f), platform.width, platform.height);

        platform_swap_buffers(&platform);
    }

    // TODO: basic text editor


    return 0;
}
