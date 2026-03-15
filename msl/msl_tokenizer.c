#include "msl_tokenizer.h"

#include <string.h>

static int is_alpha(const char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int is_digit(const char c) {
    return c >= '0' && c <= '9';
}

static int is_whitespace(const char c) {
    return c == ' ' || c == '\t' || c == '\r';
}

static void skip_whitespace_and_comments(struct MSL_tokenizer *tokenizer) {
    for (;;) {
        if (is_whitespace(*tokenizer->at)) {
            tokenizer->at++;
        } else if (tokenizer->at[0] == '/' && tokenizer->at[1] == '/') {
            tokenizer->at += 2;

            while (*tokenizer->at && *tokenizer->at != '\n') {
                tokenizer->at++;
            }
        } else {
            break;
        }
    }
}

static struct MSL_token MSL_token(const enum MSL_token_kind kind, char *string, const int32_t length,
                                  const int32_t line) {
    return (struct MSL_token){
        .kind = kind,
        .string = string,
        .length = length,
        .line = line
    };
}

static enum MSL_token_kind check_keyword_token(const char *string, const int32_t length) {
    if (length == 4 && strncmp(string, "true", 4) == 0) {
        return TOKEN_KIND_TRUE;
    }

    if (length == 5 && strncmp(string, "false", 5) == 0) {
        return TOKEN_KIND_FALSE;
    }

    return TOKEN_KIND_IDENTIFIER;
}

void MSL_tokenizer_initialize(struct MSL_tokenizer *tokenizer, char *source) {
    tokenizer->at = source;
    tokenizer->line = 1;
}

struct MSL_token MSL_tokenizer_get_token(struct MSL_tokenizer *tokenizer) {
    skip_whitespace_and_comments(tokenizer);

    char *start = tokenizer->at;
    const int32_t line = tokenizer->line;
    const char c = *tokenizer->at;

    if (c == '\0') {
        const int length = 0;
        return MSL_token(TOKEN_KIND_EOF, start, length, line);
    }

    if (c == '\n') {
        tokenizer->at++;
        tokenizer->line++;
        return MSL_token(TOKEN_KIND_NEWLINE, start, 1, line);
    }
    if (c == '(') {
        tokenizer->at++;
        return MSL_token(TOKEN_KIND_LPAREN, start, 1, line);
    }
    if (c == ')') {
        tokenizer->at++;
        return MSL_token(TOKEN_KIND_RPAREN, start, 1, line);
    }
    if (c == '+') {
        tokenizer->at++;
        return MSL_token(TOKEN_KIND_PLUS, start, 1, line);
    }
    if (c == '-') {
        tokenizer->at++;
        return MSL_token(TOKEN_KIND_MINUS, start, 1, line);
    }
    if (c == '*') {
        tokenizer->at++;
        return MSL_token(TOKEN_KIND_STAR, start, 1, line);
    }
    if (c == '/') {
        tokenizer->at++;
        return MSL_token(TOKEN_KIND_SLASH, start, 1, line);
    }
    if (c == '=') {
        tokenizer->at++;
        return MSL_token(TOKEN_KIND_EQUALS, start, 1, line);
    }
    if (c == ',') {
        tokenizer->at++;
        return MSL_token(TOKEN_KIND_COMMA, start, 1, line);
    }

    if (is_alpha(c)) {
        while (is_alpha(*tokenizer->at) || is_digit(*tokenizer->at) || *tokenizer->at == '_') {
            tokenizer->at++;
        }

        const int32_t length = (int32_t) (tokenizer->at - start);
        const enum MSL_token_kind kind = check_keyword_token(start, length);

        return MSL_token(kind, start, length, line);
    }

    if (is_digit(c) || (c == '.' && is_digit(tokenizer->at[1]))) {
        enum MSL_token_kind kind = TOKEN_KIND_INTEGER;

        while (is_digit(*tokenizer->at)) {
            tokenizer->at++;
        }

        if (*tokenizer->at == '.' && is_digit(tokenizer->at[1])) {
            kind = TOKEN_KIND_FLOAT;
            tokenizer->at++; // consume '.'

            while (is_digit(*tokenizer->at)) {
                tokenizer->at++;
            }
        }

        const int32_t length = (int32_t) (tokenizer->at - start);
        return MSL_token(kind, start, length, line);
    }

    tokenizer->at++;
    return MSL_token(TOKEN_KIND_ERROR, start, 1, line);
}

char *MSL_token_kind_stringify(const enum MSL_token_kind token_kind) {
    switch (token_kind) {
        case TOKEN_KIND_EOF: return "eof";
        case TOKEN_KIND_ERROR: return "error";
        case TOKEN_KIND_IDENTIFIER: return "identifier";
        case TOKEN_KIND_INTEGER: return "integer";
        case TOKEN_KIND_FLOAT: return "float";
        case TOKEN_KIND_TRUE: return "true";
        case TOKEN_KIND_FALSE: return "false";
        case TOKEN_KIND_PLUS: return "plus";
        case TOKEN_KIND_MINUS: return "minus";
        case TOKEN_KIND_STAR: return "star";
        case TOKEN_KIND_SLASH: return "slash";
        case TOKEN_KIND_EQUALS: return "equals";
        case TOKEN_KIND_COMMA: return "comma";
        case TOKEN_KIND_LPAREN: return "lparen";
        case TOKEN_KIND_RPAREN: return "rparen";
        case TOKEN_KIND_NEWLINE: return "newline";
        default: return "unknown";
    }
}
