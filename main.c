#include <stdio.h>

#include "platform.h"

int main(void) {
    struct platform platform;

    const platform_result result = platform_initialize(&platform);

    if (result != PLATFORM_SUCCESS) {
        fprintf(stderr, "%s", platform_result_to_string(result));
    }

    while (platform_update(&platform)) {

    }

    // TODO: opengl context
    // TODO: clear color
    // TODO: draw a sphere
    // TODO: draw text
    // TODO: basic text editor


    return 0;
}
