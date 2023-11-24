#include "token_test_utils.h"
#include "token.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "test_utils.h"

bool token_equality_tester(test_return_t test_return) {
    bool passed = true;
    token_components_t *expected = (token_components_t *) test_return.expected;
    token_t *actual = (token_t *) test_return.actual;
    if (expected->is_valid != token_get_is_valid(actual)) {
        printf("    validity differs, expected %d, got %d\n", expected->is_valid, token_get_is_valid(actual));
        return false;
    }
    if (strcmp(expected->command, token_get_command(actual)) != 0) {
        printf("    command differs, expected %s, got %s\n", expected->command, token_get_command(actual));
        passed = false;
    }
    if (strcmp(expected->input_file, token_get_input_file(actual)) != 0) {
        printf("    input file differs, expected %s, got %s\n", expected->input_file, token_get_input_file(actual));
        passed = false;
    }
    if (strcmp(expected->output_file, token_get_output_file(actual)) != 0) {
        printf("    output file differs, expected %s, got %s\n", expected->output_file, token_get_output_file(actual));
        passed = false;
    }
    if (expected->num_args != token_get_num_args(actual)) {
        printf("    num args differ, expected %zu, got %zu\n", expected->num_args, token_get_num_args(actual));
        return false;
    }
    for (size_t i = 0; i < expected->num_args; i++) {
        char *arg_expected = expected->argv[i];
        char *arg_actual = token_get_args(actual)[i];
        if (strcmp(arg_expected, arg_actual) != 0) {
            printf("    argument %zu differs, expected %s got %s\n", i, arg_expected, arg_actual);
            passed = false;
        }
    }
    return passed;
}

void token_component_free(token_components_t *token_component) {
    free(token_component->input_file);
    free(token_component->output_file);
    for (size_t i = 0; i < token_component->num_args; i++) {
        free(token_component->argv[i]);
    }
    free(token_component->argv);
    free(token_component);
}