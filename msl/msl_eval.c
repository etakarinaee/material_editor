#include "msl_eval.h"

#include <math.h>
#include <stdio.h>
#include <string.h>

static void error(struct MSL_eval *eval, const int32_t line, const char *message) {
    if (eval->error) {
        return;
    }

    eval->error = 1;
    snprintf(eval->error_message, sizeof(eval->error_message), "%d: %s", line, message);
}

static int name_equals(const char *a, const int32_t a_length, const char *b, const int32_t b_length) {
    if (a_length != b_length) {
        return 0;
    }

    return strncmp(a, b, a_length) == 0;
}

static struct MSL_value *variable_get(struct MSL_eval *eval, const char *name, const int32_t length) {
    for (int32_t i = 0; i < eval->variables_length; i++) {
        if (name_equals(eval->variables[i].name, eval->variables[i].name_length, name, length)) {
            return &eval->variables[i].value;
        }
    }

    return NULL;
}

void variable_set(struct MSL_eval *eval, char *name, const int32_t length, const struct MSL_value value) {
    struct MSL_value *existing = variable_get(eval, name, length);
    if (existing) {
        *existing = value;

        return;
    }

    if (eval->variables_length >= 256) {
        eval->error = 1;
        snprintf(eval->error_message, sizeof(eval->error_message), "too many variables");

        return;
    }

    eval->variables[eval->variables_length].name = name;
    eval->variables[eval->variables_length].name_length = length;
    eval->variables[eval->variables_length].value = value;

    eval->variables_length++;
}

double MSL_value_as_float(const struct MSL_value x) {
    switch (x.kind) {
        case VALUE_KIND_INTEGER: return (double) x.integer;
        case VALUE_KIND_FLOAT: return x.floating;
        case VALUE_KIND_BOOLEAN: return (double) x.boolean;
    }
    return 0.0;
}

static struct MSL_value MSL_value_integer(const int64_t x) {
    return (struct MSL_value){.kind = VALUE_KIND_INTEGER, .integer = x};
}

static struct MSL_value MSL_value_float(const double x) {
    return (struct MSL_value){.kind = VALUE_KIND_FLOAT, .floating = x};
}

static struct MSL_value MSL_value_boolean(const int x) {
    return (struct MSL_value){.kind = VALUE_KIND_BOOLEAN, .boolean = x};
}

static struct MSL_value eval_node(struct MSL_eval *eval, const struct MSL_node *node) {
    if (eval->error) {
        return MSL_value_float(0.0);
    }

    switch (node->kind) {
        case NODE_KIND_INTEGER: {
            return MSL_value_integer(node->integer);
        }

        case NODE_KIND_FLOAT: {
            return MSL_value_float(node->floating);
        }

        case NODE_KIND_BOOLEAN: {
            return MSL_value_boolean(node->boolean);
        }

        case NODE_KIND_IDENTIFIER: {
            struct MSL_value *var = variable_get(eval, node->name, node->name_length);

            if (!var) {
                error(eval, node->line, "undefined variable");
                return MSL_value_float(0.0);
            }

            return *var;
        }

        case NODE_KIND_CALL: {
            const char *name = node->name;
            const int32_t len = node->name_length;
            const int32_t argc = node->children_length;

            if (len == 3 && strncmp(name, "sin", 3) == 0) {
                if (argc != 1) {
                    error(eval, node->line, "sin expects 1 argument");
                    return MSL_value_float(0.0);
                }

                const double x = MSL_value_as_float(eval_node(eval, node->children[0]));

                return MSL_value_float(sin(x));
            }

            if (len == 4 && strncmp(name, "lerp", 4) == 0) {
                if (argc != 3) {
                    error(eval, node->line, "lerp expects 3 arguments");
                    return MSL_value_float(0.0);
                }

                const double a = MSL_value_as_float(eval_node(eval, node->children[0]));
                const double b = MSL_value_as_float(eval_node(eval, node->children[1]));
                const double t = MSL_value_as_float(eval_node(eval, node->children[2]));

                return MSL_value_float(a + (b - a) * t);
            }

            error(eval, node->line, "undefined function");
            return MSL_value_float(0.0);
        }

        case NODE_KIND_BINARY: {
            const struct MSL_value left = eval_node(eval, node->left);
            const struct MSL_value right = eval_node(eval, node->right);

            if (left.kind == VALUE_KIND_INTEGER && right.kind == VALUE_KIND_INTEGER) {
                const int64_t lhs = left.integer;
                const int64_t rhs = right.integer;

                switch (node->op) {
                    case TOKEN_KIND_PLUS: return MSL_value_integer(lhs + rhs);
                    case TOKEN_KIND_MINUS: return MSL_value_integer(lhs - rhs);
                    case TOKEN_KIND_STAR: return MSL_value_integer(lhs * rhs);
                    case TOKEN_KIND_SLASH: {
                        if (rhs == 0) {
                            error(eval, node->line, "division by zero");

                            return MSL_value_integer(0);
                        }

                        return MSL_value_integer(lhs / rhs);
                    }
                    default: break;
                }
            }

            const double lhs = MSL_value_as_float(left);
            const double rhs = MSL_value_as_float(right);

            switch (node->op) {
                case TOKEN_KIND_PLUS: return MSL_value_float(lhs + rhs);
                case TOKEN_KIND_MINUS: return MSL_value_float(lhs - rhs);
                case TOKEN_KIND_STAR: return MSL_value_float(lhs * rhs);
                case TOKEN_KIND_SLASH: {
                    if (rhs == 0.0) {
                        error(eval, node->line, "division by zero");

                        return MSL_value_float(0.0);
                    }

                    return MSL_value_float(lhs / rhs);
                }
                default: break;
            }

            error(eval, node->line, "undefined operator");
            return MSL_value_float(0.0);
        }

        case NODE_KIND_UNARY: {
            const struct MSL_value operand = eval_node(eval, node->left);

            if (node->op == TOKEN_KIND_MINUS) {
                if (operand.kind == VALUE_KIND_INTEGER) {
                    return MSL_value_integer(-operand.integer);
                }

                return MSL_value_float(-MSL_value_as_float(operand));
            }

            error(eval, node->line, "undefined unary operator");
            return MSL_value_float(0.0);
        }

        default: {
            error(eval, node->line, "unexpected node");
            return MSL_value_float(0.0);
        }
    }
}

void MSL_eval_initialize(struct MSL_eval *eval) {
    eval->variables_length = 0;
    eval->error = 0;
    eval->error_message[0] = '\0';
}

void MSL_eval_execute(struct MSL_eval *eval, const struct MSL_node *program) {
    if (program->kind != NODE_KIND_PROGRAM) {
        error(eval, program->line, "expected program node");
        return;
    }

    for (int32_t i = 0; i < program->children_length; i++) {
        const struct MSL_node *statement = program->children[i];

        if (statement->kind == NODE_KIND_ASSIGNMENT) {
            const struct MSL_value value = eval_node(eval, statement->left);
            variable_set(eval, statement->name, statement->name_length, value);
        } else {
            error(eval, statement->line, "unexpected statement");
        }

        if (eval->error) {
            break;
        }
    }
}

struct MSL_value *MSL_eval_get(struct MSL_eval *eval, const char *name) {
    const int32_t length = (int32_t) strlen(name);
    return variable_get(eval, name, length);
}

float MSL_eval_get_with_fallback(struct MSL_eval *eval, const char *name, const float fallback) {
    const struct MSL_value *x = MSL_eval_get(eval, name);

    if (x) {
        return (float) MSL_value_as_float(*x);
    }

    return fallback;
}

void MSL_eval_set(struct MSL_eval *eval, char *name, const float value) {
    const int32_t length = (int32_t) strlen(name);
    variable_set(eval, name, length, MSL_value_float(value));
}
