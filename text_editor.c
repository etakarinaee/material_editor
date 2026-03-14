#include "text_editor.h"

#include <string.h>

void text_editor_initialize(struct text_editor *text_editor) {
    text_editor->length = 0;
    text_editor->cursor = 0;
    text_editor->buffer[0] = '\0';
}

void text_editor_insert_char(struct text_editor *text_editor, const char c) {
    if (text_editor->length < TEXT_EDITOR_MAX_SIZE - 1) {
        /// shift everything from cursor onward one position to the right
        memmove(&text_editor->buffer[text_editor->cursor + 1],
                &text_editor->buffer[text_editor->cursor],
                text_editor->length - text_editor->cursor + 1);

        text_editor->buffer[text_editor->cursor] = c;
        text_editor->length++;
        text_editor->cursor++;
    }
}

void text_editor_remove_char(struct text_editor *text_editor) {
    if (text_editor->length > 0) {
        memmove(&text_editor->buffer[text_editor->cursor - 1],
                &text_editor->buffer[text_editor->cursor],
                text_editor->length - text_editor->cursor + 1);

        text_editor->length--;
        text_editor->cursor--;
    }
}

void text_editor_move_left(struct text_editor *text_editor) {
    if (text_editor->cursor > 0) {
        text_editor->cursor--;
    }
}

void text_editor_move_right(struct text_editor *text_editor) {
    if (text_editor->cursor < text_editor->length) {
        text_editor->cursor++;
    }
}

// find the column offset of the cursor within its current line
static int cursor_column(const struct text_editor *text_editor) {
    int column = 0;

    for (int i = text_editor->cursor - 1; i >= 0; i--) {
        if (text_editor->buffer[i] == '\n') {
            break;
        }

        column++;
    }

    return column;
}

void text_editor_move_up(struct text_editor *text_editor) {
    int column = cursor_column(text_editor);

    int line_start = text_editor->cursor;
    while (line_start > 0 && text_editor->buffer[line_start - 1] != '\n') {
        line_start--;
    }

    // if already on the first line
    if (line_start == 0) {
        // move cursor to the start
        text_editor->cursor = 0;

        return;
    }

    const int previous_line_end = line_start - 1; // index of \n ending the previous line
    int previous_line_start = previous_line_end;

    while (previous_line_start > 0 && text_editor->buffer[previous_line_start - 1] != '\n') {
        previous_line_start--;
    }

    const int previous_line_length = previous_line_end - previous_line_start;

    if (column > previous_line_length) {
        column = previous_line_length;
    }

    text_editor->cursor = previous_line_start + column;
}

void text_editor_move_down(struct text_editor *text_editor) {
    int column = cursor_column(text_editor);

    int position = text_editor->cursor;
    while (position < text_editor->length && text_editor->buffer[position] != '\n') {
        position++;
    }

    // if on the last line
    if (position >= text_editor->length) {
        // move cursor to the end
        text_editor->cursor = text_editor->length;

        return;
    }

    // skip over the \n to reach the start of the next line
    const int next_line_start = position + 1;

    int next_line_end = next_line_start;
    while (next_line_end < text_editor->length && text_editor->buffer[next_line_end] != '\n') {
        next_line_end++;
    }

    const int next_line_length = next_line_end - next_line_start;

    if (column > next_line_length) {
        column = next_line_length;
    }

    text_editor->cursor = next_line_start + column;
}
