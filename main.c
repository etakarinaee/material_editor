#include <stdio.h>
#include <stdlib.h>

#include "gl.h"
#include "platform.h"
#include "text_editor.h"

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

    struct text_editor text_editor;
    text_editor_initialize(&text_editor);

    while (platform_update(&platform)) {
        for (int i = 0; i < platform.text_input_length; i++) {
            text_editor_insert_char(&text_editor, platform.text_input[i]);
        }

        if (platform.key_pressed[KEY_ENTER]) {
            text_editor_insert_char(&text_editor, '\n');
        }
        if (platform.key_pressed[KEY_BACKSPACE]) {
            text_editor_remove_char(&text_editor);
        }
        if (platform.key_pressed[KEY_TAB]) {
            text_editor_insert_char(&text_editor, ' ');
            text_editor_insert_char(&text_editor, ' ');
            text_editor_insert_char(&text_editor, ' ');
            text_editor_insert_char(&text_editor, ' ');
        }

        gl_clear(0.1f, 0.1f, 0.1f, GL_OPAQUE);

        // preview has its own part of the screen
        const int x = platform.width / 2;
        const int width = platform.width / 2;
        const float aspect = (float) width / (float) platform.height;

        gl_set_viewport(x, 0, width, platform.height);
        gl_begin(vector3(0, 0, 3), vector3(0, 0, 0), aspect);
        gl_draw_mesh1(vector3(0, 0, 0), 0.5f, vector3(0.4f, 0.6f, 1.0f));

        gl_set_viewport(0, 0, platform.width, platform.height);

        const int editor_width = platform.width / 2;
        gl_set_scissor(0, 0, editor_width, platform.height);

        const float text_x = 25.0f;
        const float text_y = (float) platform.height - 50.0f;

        gl_font_draw(&font, text_editor.buffer, text_x, text_y, 0.3125f,
                     vector3(1.0f, 1.0f, 1.0f), platform.width, platform.height);

        gl_clear_scissor();

        platform_swap_buffers(&platform);
    }

    // TODO: draw cursor
    // TODO: cursor movement
    // TODO: line numbers
    // TODO: mouse interaction

    return 0;
}
