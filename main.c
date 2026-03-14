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
        platform_swap_buffers(&platform);
        gl_clear(0.1f, 0.1f, 0.1f, GL_OPAQUE);
    }

    // TODO: opengl context
    // TODO: clear color
    // TODO: draw a sphere
    // TODO: draw text
    // TODO: basic text editor


    return 0;
}
