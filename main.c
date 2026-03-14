#include <stdio.h>

#include "gl.h"
#include "platform.h"

int main(void) {
    struct platform platform;

    const material_editor_result result = platform_initialize(&platform);

    if (result != RESULT_SUCCESS) {
        fprintf(stderr, "%s", result_to_string(result));
    }

    gl_initialize();

    while (platform_update(&platform)) {
        gl_clear(0.1f, 0.1f, 0.1f, GL_OPAQUE);

        // preview has its own part of the screen
        const int x = platform.width / 2;
        const int width = platform.width / 2;
        const float aspect = (float) width / (float) platform.height;

        gl_set_viewport(x, 0, width, platform.height);
        gl_begin(vector3(0, 0, 3), vector3(0, 0, 0), aspect);
        gl_draw_mesh1(vector3(0, 0, 0), 0.5f, vector3(0.4f, 0.6f, 1.0f));

        platform_swap_buffers(&platform);
    }

    // TODO: draw text
    // TODO: basic text editor


    return 0;
}
