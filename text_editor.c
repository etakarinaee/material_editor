#include "text_editor.h"

void text_editor_initialize(struct text_editor *text_editor) {
    text_editor->length = 0;
    text_editor->buffer[0] = '\0';
}

void text_editor_insert_char(struct text_editor *text_editor, char c) {
    if (text_editor->length < TEXT_EDITOR_MAX_SIZE - 1) {
        text_editor->buffer[text_editor->length++] = c;
        text_editor->buffer[text_editor->length] = '\0';
    }
}

void text_editor_remove_char(struct text_editor *text_editor) {
    if (text_editor->length > 0) {
        text_editor->buffer[--text_editor->length] = '\0';
    }
}
