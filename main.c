#include <stdio.h>

#include "gl.h"
#include "platform.h"

int main(void) {
    struct platform platform;

    const material_editor_result result = platform_initialize(&platform);

    if (result != RESULT_SUCCESS) {
        fprintf(stderr, "%s", result_to_string(result));
    }

    while (platform_update(&platform)) {
        gl_clear(0.1f, 0.1f, 0.1f, GL_OPAQUE);
        gl_begin(vector3(0, 0, 3), vector3(0, 0, 0), platform.width / platform.height);

        platform_swap_buffers(&platform);
    }

    // TODO: draw a sphere
    // TODO: draw text
    // TODO: basic text editor


    return 0;
}
