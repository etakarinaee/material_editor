#ifndef MATERIAL_EDITOR_MSL_EVAL_H
#define MATERIAL_EDITOR_MSL_EVAL_H

#include "msl_parser.h"

enum MSL_value_kind {
    VALUE_KIND_INTEGER,
    VALUE_KIND_FLOAT,
    VALUE_KIND_BOOLEAN,
};

struct MSL_value {
    enum MSL_value_kind kind;

    union {
        int64_t integer;
        double floating;
        int boolean;
    };
};

struct MSL_variable {
    char *name;
    int32_t name_length;
    struct MSL_value value;
};

struct MSL_eval {
    struct MSL_variable variables[256];
    int32_t variables_length;

    int error;
    char error_message[256];
};

void MSL_eval_initialize(struct MSL_eval *eval);

void MSL_eval_execute(struct MSL_eval *eval, const struct MSL_node *program);

struct MSL_value *MSL_eval_get(struct MSL_eval *eval, const char *name);

float MSL_eval_get_with_fallback(struct MSL_eval *eval, const char *name, float fallback);

void MSL_eval_set(struct MSL_eval *eval, char *name, float value);

double MSL_value_as_float(struct MSL_value x);

#endif //MATERIAL_EDITOR_MSL_EVAL_H
