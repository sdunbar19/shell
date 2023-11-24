#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include "test_utils.h"

#ifndef TOKEN_TEST_UTILS_H
#define TOKEN_TEST_UTILS_H

typedef struct token_components {
    bool is_valid;
    char *input_file;
    char *output_file;
    char *command;
    size_t num_args;
    char **argv;
} token_components_t;

bool token_equality_tester(test_return_t test_return);

void token_component_free(token_components_t *token_component);

#endif