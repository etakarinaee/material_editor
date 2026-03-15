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

    result = gl_font_initialize(&font, "JetBrainsMono-Regular.ttf", 15);

    if (result != RESULT_SUCCESS) {
        fprintf(stderr, "%s\n", result_to_string(result));

        return EXIT_FAILURE;
    }

    struct text_editor text_editor;
    text_editor_initialize(&text_editor);

    // TODO: this is messed up, clean up
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

        if (platform.key_pressed[KEY_LEFT]) {
            text_editor_move_left(&text_editor);
        }
        if (platform.key_pressed[KEY_RIGHT]) {
            text_editor_move_right(&text_editor);
        }
        if (platform.key_pressed[KEY_UP]) {
            if (platform.alt) {
                text_editor_put_line_up(&text_editor);
            } else {
                text_editor_move_up(&text_editor);
            }
        }
        if (platform.key_pressed[KEY_DOWN]) {
            if (platform.alt) {
                text_editor_put_line_down(&text_editor);
            } else {
                text_editor_move_down(&text_editor);
            }
        }

        if (platform.mouse) {
            if (platform.mx < platform.width / 2) {
                const float line_number_width = 40.0f;
                const float text_x = 25.0f;
                const float code_x = text_x + line_number_width;
                const float text_y = (float) platform.height - 50.0f;

                const float line_height = font.line_height;
                const float char_width = (float) font.glyphs['A'].advance;
                const float ascender = (float) font.glyphs['A'].bearing_y;

                const float y = (float) (platform.height - platform.my);

                text_editor_set_cursor_from_position(&text_editor, (float) platform.mx, y,
                                                     code_x, text_y, line_height, char_width,
                                                     ascender);
            }
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
        const float line_number_width = 40.0f;
        const float code_x = text_x + line_number_width;

        int line_count = 1;
        for (int i = 0; i < text_editor.length; i++) {
            if (text_editor.buffer[i] == '\n') {
                line_count++;
            }
        }

        char line_number[16];
        float line_y = text_y;
        for (int i = 1; i <= line_count; i++) {
            snprintf(line_number, sizeof(line_number), "%3d", i);

            gl_font_draw(&font, line_number, text_x, line_y, 1.0f,
                         vector3(0.5f, 0.5f, 0.5f), platform.width, platform.height);

            line_y -= font.line_height;
        }

        gl_font_draw(&font, text_editor.buffer, code_x, text_y, 1.0f,
                     vector3(1.0f, 1.0f, 1.0f), platform.width, platform.height);

        gl_font_draw_cursor(&font, text_editor.buffer, text_editor.cursor,
                            code_x, text_y, 1.0f,
                            vector3(1.0f, 1.0f, 1.0f), platform.width, platform.height);

        gl_clear_scissor();

        platform_swap_buffers(&platform);
    }

    // TODO: selection (with cursor)
    // TODO: selection (with shortcuts)
    // TODO: selection (with cursor by lines)

    return 0;
}
