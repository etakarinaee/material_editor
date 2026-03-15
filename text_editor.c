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

void text_editor_set_cursor_from_position(struct text_editor *text_editor, const float cx, const float cy,
                                          const float ox, const float oy,
                                          const float line_height, const float char_width,
                                          const float ascender) {
    const float visual_top = oy + ascender;

    int line = (int) ((visual_top - cy) / line_height);

    if (line < 0) {
        line = 0;
    }


    // which column?
    int column = (int) ((cx - ox + char_width * 0.5f) / char_width);

    if (column < 0) {
        column = 0;
    }

    int current_line = 0;
    int line_start = 0;

    for (int i = 0; i < text_editor->length; i++) {
        if (current_line == line) {
            break;
        }

        if (text_editor->buffer[i] == '\n') {
            current_line++;
            line_start = i + 1;
        }
    }

    int line_end = line_start;
    while (line_end < text_editor->length && text_editor->buffer[line_end] != '\n') {
        line_end++;
    }

    const int line_length = line_end - line_start;

    if (column > line_length) {
        column = line_length;
    }

    text_editor->cursor = line_start + column;
}

static int get_line_start(const struct text_editor *text_editor, int position) {
    while (position > 0 && text_editor->buffer[position - 1] != '\n') {
        position--;
    }

    return position;
}

static int get_line_end(const struct text_editor *text_editor, int position) {
    while (position < text_editor->length && text_editor->buffer[position] != '\n') {
        position++;
    }

    return position;
}

void text_editor_put_line_up(struct text_editor *text_editor) {
    const int line_start = get_line_start(text_editor, text_editor->cursor);

    if (line_start == 0) {
        return;
    }

    const int line_end = get_line_end(text_editor, text_editor->cursor);
    const int previous_line_start = get_line_start(text_editor, line_start - 1);

    const int cursor_offset = text_editor->cursor - line_start;

    const int previous_length = (line_start - 1) - previous_line_start;
    const int current_length = line_end - line_start;
    const int current_has_newline = (line_end < text_editor->length);

    char previous[TEXT_EDITOR_MAX_SIZE];
    char current[TEXT_EDITOR_MAX_SIZE];
    memcpy(previous, &text_editor->buffer[previous_line_start], previous_length);
    memcpy(current, &text_editor->buffer[line_start], current_length);

    int position = previous_line_start;
    memcpy(&text_editor->buffer[position], current, current_length);
    position += current_length;

    text_editor->buffer[position++] = '\n';
    memcpy(&text_editor->buffer[position], previous, previous_length);
    position += previous_length;

    if (current_has_newline) {
        text_editor->buffer[position] = '\n';
    }

    text_editor->buffer[text_editor->length] = '\0';

    text_editor->cursor = previous_line_start + cursor_offset;
}

void text_editor_put_line_down(struct text_editor *text_editor) {
    const int line_start = get_line_start(text_editor, text_editor->cursor);
    const int line_end = get_line_end(text_editor, text_editor->cursor);

    // already on last line
    if (line_end >= text_editor->length) {
        return;
    }

    const int next_line_start = line_end + 1;
    const int next_line_end = get_line_end(text_editor, next_line_start);

    const int cursor_offset = text_editor->cursor - line_start;

    const int current_length = line_end - line_start;
    const int next_length = next_line_end - next_line_start;
    const int next_has_newline = (next_line_end < text_editor->length);

    char current[TEXT_EDITOR_MAX_SIZE];
    char next[TEXT_EDITOR_MAX_SIZE];
    memcpy(current, &text_editor->buffer[line_start], current_length);
    memcpy(next, &text_editor->buffer[next_line_start], next_length);

    int position = line_start;
    memcpy(&text_editor->buffer[position], next, next_length);
    position += next_length;

    text_editor->buffer[position++] = '\n';
    memcpy(&text_editor->buffer[position], current, current_length);
    position += current_length;

    if (next_has_newline) {
        text_editor->buffer[position] = '\n';
    }

    text_editor->buffer[text_editor->length] = '\0';

    text_editor->cursor = line_start + next_length + 1 + cursor_offset;
}
