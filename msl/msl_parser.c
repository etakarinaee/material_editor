#include "msl_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void advance(struct MSL_parser *parser) {
    parser->current = MSL_tokenizer_get_token(&parser->tokenizer);
}

static void skip_newlines(struct MSL_parser *parser) {
    while (parser->current.kind == TOKEN_KIND_NEWLINE) {
        advance(parser);
    }
}

static int match(struct MSL_parser *parser, const enum MSL_token_kind kind) {
    if (parser->current.kind == kind) {
        advance(parser);

        return 1;
    }

    return 0;
}

static void error(struct MSL_parser *parser, const char *message) {
    if (parser->error) {
        return;
    }

    parser->error = 1;
    snprintf(parser->error_message, sizeof(parser->error_message), "%d: %s", parser->current.line, message);
}

static struct MSL_node *MSL_node_alloc(const enum MSL_node_kind kind, const int32_t line) {
    struct MSL_node *node = calloc(1, sizeof(struct MSL_node));

    node->kind = kind;
    node->line = line;

    return node;
}

static int64_t parse_integer(const char *string, const int32_t length) {
    int64_t integer = 0;

    for (int32_t i = 0; i < length; i++) {
        integer = integer * 10 + (string[i] - '0');
    }

    return integer;
}

static double parse_floating(const char *string, const int32_t length) {
    char buffer[64];

    const int32_t copy_length = length < 63 ? length : 63;
    memcpy(buffer, string, copy_length);
    buffer[copy_length] = '\0';

    return atof(buffer);
}

enum {
    PRECEDENCE_NONE = 0,
    PRECEDENCE_TERM = 1,
    PRECEDENCE_FACTOR = 2,
    PRECEDENCE_UNARY = 3,
};

static int precedence_get(const enum MSL_token_kind token_kind) {
    switch (token_kind) {
        case TOKEN_KIND_PLUS: return PRECEDENCE_TERM;
        case TOKEN_KIND_MINUS: return PRECEDENCE_TERM;
        case TOKEN_KIND_STAR: return PRECEDENCE_FACTOR;
        case TOKEN_KIND_SLASH: return PRECEDENCE_FACTOR;
        default: return PRECEDENCE_NONE;
    }
}

static struct MSL_node *parse_expression(struct MSL_parser *parser, const int min_precedence) {
    struct MSL_node *left;
    const struct MSL_token token = parser->current;

    if (parser->current.kind == TOKEN_KIND_MINUS) {
        advance(parser);

        struct MSL_node *operand = parse_expression(parser, PRECEDENCE_UNARY);
        if (!operand) {
            return NULL;
        }

        left = MSL_node_alloc(NODE_KIND_UNARY, token.line);
        left->op = TOKEN_KIND_MINUS;
        left->left = operand;
    } else if (match(parser, TOKEN_KIND_INTEGER)) {
        left = MSL_node_alloc(NODE_KIND_INTEGER, token.line);
        left->integer = parse_integer(token.string, token.length);
    } else if (match(parser, TOKEN_KIND_FLOAT)) {
        left = MSL_node_alloc(NODE_KIND_FLOAT, token.line);
        left->floating = parse_floating(token.string, token.length);
    } else if (match(parser, TOKEN_KIND_TRUE)) {
        left = MSL_node_alloc(NODE_KIND_BOOLEAN, token.line);
        left->boolean = 1;
    } else if (match(parser, TOKEN_KIND_FALSE)) {
        left = MSL_node_alloc(NODE_KIND_BOOLEAN, token.line);
        left->boolean = 0;
    } else if (match(parser, TOKEN_KIND_IDENTIFIER)) {
        if (parser->current.kind == TOKEN_KIND_LPAREN) {
            advance(parser);

            left = MSL_node_alloc(NODE_KIND_CALL, token.line);
            left->name = token.string;
            left->name_length = token.length;

            int32_t capacity = 4;
            left->children = malloc(capacity * sizeof(struct MSL_node *));
            left->children_length = 0;

            if (parser->current.kind != TOKEN_KIND_RPAREN) {
                for (;;) {
                    if (left->children_length >= capacity) {
                        capacity *= 2;
                        left->children = realloc(left->children, capacity * sizeof(struct MSL_node *));
                    }

                    struct MSL_node *argument = parse_expression(parser, PRECEDENCE_NONE);
                    if (!argument) {
                        return NULL;
                    }

                    left->children[left->children_length++] = argument;

                    if (!match(parser, TOKEN_KIND_COMMA)) {
                        break;
                    }
                }
            }

            if (!match(parser, TOKEN_KIND_RPAREN)) {
                error(parser, "expected ')' after arguments");

                return NULL;
            }
        } else {
            left = MSL_node_alloc(NODE_KIND_IDENTIFIER, token.line);
            left->name = token.string;
            left->name_length = token.length;
        }
    } else if (match(parser, TOKEN_KIND_LPAREN)) {
        left = parse_expression(parser, PRECEDENCE_NONE);
        if (!left) {
            return NULL;
        }

        if (!match(parser, TOKEN_KIND_RPAREN)) {
            error(parser, "expected ')' after expressions");

            return NULL;
        }
    } else {
        error(parser, "expected expression");

        return NULL;
    }

    for (;;) {
        const int precedence = precedence_get(parser->current.kind);
        if (precedence == PRECEDENCE_NONE || precedence < min_precedence) {
            break;
        }

        const struct MSL_token op = parser->current;
        advance(parser);

        struct MSL_node *right = parse_expression(parser, precedence + 1);
        if (!right) {
            return NULL;
        }

        struct MSL_node *binary = MSL_node_alloc(NODE_KIND_BINARY, token.line);
        binary->op = op.kind;
        binary->left = left;
        binary->right = right;
        left = binary;
    }

    return left;
}

static struct MSL_node *parse_assignment(struct MSL_parser *parser) {
    if (parser->current.kind != TOKEN_KIND_IDENTIFIER) {
        error(parser, "expected identifier");

        return NULL;
    }

    const struct MSL_token name = parser->current;
    advance(parser);

    if (!match(parser, TOKEN_KIND_EQUALS)) {
        error(parser, "expected '=' after identifier");

        return NULL;
    }

    struct MSL_node *value = parse_expression(parser, PRECEDENCE_NONE);
    if (!value) {
        return NULL;
    }

    struct MSL_node *node = MSL_node_alloc(NODE_KIND_ASSIGNMENT, name.line);
    node->name = name.string;
    node->name_length = name.length;
    node->left = value;

    return node;
}

static struct MSL_node *parse_program(struct MSL_parser *parser) {
    struct MSL_node *program = MSL_node_alloc(NODE_KIND_PROGRAM, 1);

    int32_t capacity = 8;
    program->children = malloc(capacity * sizeof(struct MSL_node *));
    program->children_length = 0;

    skip_newlines(parser);

    while (parser->current.kind != TOKEN_KIND_EOF) {
        if (program->children_length >= capacity) {
            capacity *= 2;
            program->children = realloc(program->children, capacity * sizeof(struct MSL_node *));
        }

        struct MSL_node *statement = parse_assignment(parser);

        if (!statement) {
            return NULL;
        }

        program->children[program->children_length++] = statement;

        if (parser->current.kind != TOKEN_KIND_EOF && parser->current.kind != TOKEN_KIND_NEWLINE) {
            error(parser, "expected newline after statement");
        }

        skip_newlines(parser);
    }

    return program;
}

void MSL_parser_initialize(struct MSL_parser *parser, char *source) {
    MSL_tokenizer_initialize(&parser->tokenizer, source);

    parser->error = 0;
    parser->error_message[0] = '\0';

    advance(parser);
}

struct MSL_node *MSL_parser_parse(struct MSL_parser *parser) {
    return parse_program(parser);
}

void MSL_node_stringify(const struct MSL_node *node, const int indentation) {
    for (int i = 0; i < indentation; i++) {
        printf("  ");
    }

    switch (node->kind) {
        case NODE_KIND_PROGRAM: {
            printf("program\n");

            for (int32_t i = 0; i < node->children_length; i++) {
                MSL_node_stringify(node->children[i], indentation + 1);
            }
        }
        break;

        case NODE_KIND_ASSIGNMENT: {
            printf("assignment %.*s\n", node->name_length, node->name);

            MSL_node_stringify(node->left, indentation + 1);
        }
        break;

        case NODE_KIND_BINARY: {
            printf("binary %s\n", MSL_token_kind_stringify(node->op));

            MSL_node_stringify(node->left, indentation + 1);
            MSL_node_stringify(node->right, indentation + 1);
        }
        break;

        case NODE_KIND_UNARY: {
            printf("unary %s\n", MSL_token_kind_stringify(node->op));
            MSL_node_stringify(node->left, indentation + 1);
        }
        break;

        case NODE_KIND_CALL: {
            printf("call %.*s\n", node->name_length, node->name);

            for (int32_t i = 0; i < node->children_length; i++) {
                MSL_node_stringify(node->children[i], indentation + 1);
            }
        }
        break;

        case NODE_KIND_INTEGER: {
            printf("integer %lld\n", (long long) node->integer);
        }
        break;

        case NODE_KIND_FLOAT: {
            printf("float %f\n", node->floating);
        }
        break;

        case NODE_KIND_BOOLEAN: {
            printf("bool %s\n", node->boolean ? "true" : "false");
        }
        break;

        case NODE_KIND_IDENTIFIER: {
            printf("identifier %.*s\n", node->name_length, node->name);
        }
        break;
    }
}

void MSL_node_free(struct MSL_node *node) {
    if (!node) {
        return;
    }

    MSL_node_free(node->left);
    MSL_node_free(node->right);

    for (int32_t i = 0; i < node->children_length; i++) {
        MSL_node_free(node->children[i]);
    }

    free(node->children);
    free(node);
}
