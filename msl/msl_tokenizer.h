#ifndef MATERIAL_EDITOR_MSL_TOKENIZE_H
#define MATERIAL_EDITOR_MSL_TOKENIZE_H
#include <stdint.h>

enum MSL_token_kind {
    TOKEN_KIND_EOF = 0,
    TOKEN_KIND_ERROR,

    TOKEN_KIND_IDENTIFIER,
    TOKEN_KIND_INTEGER,
    TOKEN_KIND_FLOAT,
    TOKEN_KIND_TRUE,
    TOKEN_KIND_FALSE,

    TOKEN_KIND_PLUS,
    TOKEN_KIND_MINUS,
    TOKEN_KIND_STAR,
    TOKEN_KIND_SLASH,
    TOKEN_KIND_EQUALS,
    TOKEN_KIND_COMMA,
    TOKEN_KIND_LPAREN,
    TOKEN_KIND_RPAREN,

    TOKEN_KIND_NEWLINE,
};

struct MSL_token {
    enum MSL_token_kind kind;
    char *string;
    int32_t length;
    int32_t line;
};

struct MSL_tokenizer {
    char *at;
    int32_t line;
};

void MSL_tokenizer_initialize(struct MSL_tokenizer *tokenizer, char *source);

struct MSL_token MSL_tokenizer_get_token(struct MSL_tokenizer *tokenizer);

char *MSL_token_kind_stringify(enum MSL_token_kind token_kind);

#endif //MATERIAL_EDITOR_MSL_TOKENIZE_H
