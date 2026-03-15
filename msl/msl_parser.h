#ifndef MATERIAL_EDITOR_MSL_PARSER_H
#define MATERIAL_EDITOR_MSL_PARSER_H

#include "msl_tokenizer.h"

enum MSL_node_kind {
    NODE_KIND_PROGRAM,
    NODE_KIND_ASSIGNMENT,
    NODE_KIND_BINARY,
    NODE_KIND_UNARY,
    NODE_KIND_CALL,
    NODE_KIND_INTEGER,
    NODE_KIND_FLOAT,
    NODE_KIND_BOOLEAN,
    NODE_KIND_IDENTIFIER,
};

struct MSL_node {
    enum MSL_node_kind kind;
    int32_t line;

    char *name;
    int32_t name_length;

    enum MSL_token_kind op;

    union {
        int64_t integer;
        double floating;
        int boolean;
    };

    struct MSL_node *left;
    struct MSL_node *right;

    struct MSL_node **children;
    int32_t children_length;
};

struct MSL_parser {
    struct MSL_tokenizer tokenizer;
    struct MSL_token current;
    // whether there was an error or not
    int error;
    char error_message[256];
};

void MSL_parser_initialize(struct MSL_parser *parser, char *source);

struct MSL_node *MSL_parser_parse(struct MSL_parser *parser);

void MSL_node_stringify(const struct MSL_node *node, int indentation);

void MSL_node_free(struct MSL_node *node);

#endif //MATERIAL_EDITOR_MSL_PARSER_H
