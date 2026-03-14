#ifndef MATERIAL_EDITOR_TEXT_EDITOR_H
#define MATERIAL_EDITOR_TEXT_EDITOR_H

#define TEXT_EDITOR_MAX_SIZE 4096

struct text_editor {
    char buffer[TEXT_EDITOR_MAX_SIZE];
    int length;
    int cursor;
};

void text_editor_initialize(struct text_editor *text_editor);

void text_editor_insert_char(struct text_editor *text_editor, char c);

void text_editor_remove_char(struct text_editor *text_editor);

#endif //MATERIAL_EDITOR_TEXT_EDITOR_H
