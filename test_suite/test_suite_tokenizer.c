#include "tokenizer.h"
#include "test_utils.h"
#include "token_test_utils.h"
#include "error_handling.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

const char *FOLDER_NAME = "test_suite/tokenizer_test_files";
const char *shell_location = "bin/mysh";
const size_t MAX_FILE_LINE_LENGTH = 100;
extern const size_t MAX_BUFFER;

typedef struct token_list_components {
    bool is_valid;
    size_t num_tokens;
    token_components_t **tokens;
} token_list_components_t;

void token_list_components_free(token_list_components_t *components) {
    for (size_t i = 0; i < components->num_tokens; i++) {
        token_component_free(components->tokens[i]);
    }
    free(components->tokens);
    free(components);
}

/* EQUALITY TESTERS */
/* ______________________________________________________________________________________________*/
bool token_list_equality_tester(test_return_t test_return) {
    token_list_components_t *expected = (token_list_components_t *) test_return.expected;
    token_list_t *actual = (token_list_t *) test_return.actual;
    if (expected->is_valid != token_list_get_is_valid(actual)) {
        printf("    validity differs, expected %d, got %d\n", expected->is_valid, token_list_get_is_valid(actual));
        return false;
    }
    if (expected->num_tokens != token_list_get_num_tokens(actual)) {
        printf("    num tokens differ, expected %zu, got %zu\n", expected->num_tokens, token_list_get_num_tokens(actual));
        return false;
    }
    for (size_t i = 0; i < expected->num_tokens; i++) {
        token_components_t *expected_token = expected->tokens[i];
        token_t *actual_token = token_list_get_token_from_list(actual, i);
        test_return_t to_call = (test_return_t) {.expected = expected_token,
                                                 .actual = actual_token,
                                                 .freer_expected = (free_func_t) token_component_free,
                                                 .freer_actual = (free_func_t) token_free};
        bool same_token = token_equality_tester(to_call);
        if (!same_token) {
            printf("    error occurred on token %zu\n", i);
            return false;
        }
    }
    return true;
}

bool token_list_count_validity_equality_tester(test_return_t test_return) {
    bool passed = true;
    token_list_components_t *expected = (token_list_components_t *) test_return.expected;
    token_list_t *actual = (token_list_t *) test_return.actual;
    if (expected->is_valid != token_list_get_is_valid(actual)) {
        printf("    validity differs, expected %d, got %d\n", expected->is_valid, token_list_get_is_valid(actual));
        return false;
    }
    if (expected->num_tokens != token_list_get_num_tokens(actual)) {
        printf("    num tokens differ, expected %zu, got %zu\n", expected->num_tokens, token_list_get_num_tokens(actual));
        passed = false;
    }
    return passed;
}

/* TEST UTIL FUNCTIONS */
/* ______________________________________________________________________________________________*/

test_return_t token_list_count_equality_test_util(char *input, size_t num_tokens_expected, bool is_valid_expected) {
    token_list_t *token_list = tokenize_input(input);
    token_list_components_t *token_list_components = malloc(sizeof(token_list_components_t));
    token_list_components->num_tokens = num_tokens_expected;
    token_list_components->is_valid = is_valid_expected;
    return (test_return_t) {.expected = token_list_components,
                            .actual = token_list,
                            .freer_expected = free,
                            .freer_actual = (free_func_t) token_list_free};
}

char *read_string_from_file(FILE *file, char *buffer, char *error) {
    if (fgets(buffer, MAX_FILE_LINE_LENGTH, file) == NULL) {
        handle_error(errno, error);
    }
    buffer[strlen(buffer) - 1] = '\0';
    char *string = malloc(sizeof(char) * MAX_FILE_LINE_LENGTH);
    string[0] = '\0';
    strcpy(string, buffer);
    return string;
}

token_components_t *file_read_token(FILE *file, char *buffer) {
    token_components_t *token_component = malloc(sizeof(token_components_t));

    // skip the space
    if (fgets(buffer, MAX_FILE_LINE_LENGTH, file) == NULL) {
        handle_error(errno, "could not skip space");
    }

    // get if valid
    if (fgets(buffer, MAX_FILE_LINE_LENGTH, file) == NULL) {
        handle_error(errno, "could not get token validity");
    }
    char *validity = malloc(sizeof(char) * (strlen(buffer) + 1));
    validity[0] = '\0';
    strcpy(buffer, validity);
    if (strcmp(validity, "true\n")) {
        token_component->is_valid = true;
    }
    else if (strcmp(validity, "false\n")) {
        token_component->is_valid = false;
    }
    else {
        handle_error(EIO, "unexpected token validity value");
    }
    free(validity);

    // get num args
    if (fgets(buffer, MAX_FILE_LINE_LENGTH, file) == NULL) {
        handle_error(errno, "could not get num args in token");
    }
    size_t num_args = convert_string_to_int_base_ten(buffer);
    token_component->num_args = num_args;
    
    char **argv = malloc(sizeof(char *) * num_args);
    token_component->argv = argv;

    // get input file
    token_component->input_file = read_string_from_file(file, buffer, "could not read input file");

    // get output file
    token_component->output_file = read_string_from_file(file, buffer, "could not read output file");
    
    // get command
    argv[0] = read_string_from_file(file, buffer, "could not read command from file");
    token_component->command = argv[0];

    // read arguments
    for (size_t i = 1; i < num_args; i++) {
        argv[i] = read_string_from_file(file, buffer, "could not read argument string");
    }
    return token_component;
}

test_return_t deconstruct_file_test_case(char *file_name) {
    size_t file_name_length = strlen(FOLDER_NAME) + strlen(file_name) + 2;
    char file_name_folder[file_name_length];
    strcpy(file_name_folder, FOLDER_NAME);

    strcat(file_name_folder, "/");
    strcat(file_name_folder, file_name);
    FILE *file = open_file(file_name_folder, "r");
    char buffer[MAX_FILE_LINE_LENGTH];

    // get input, actual token
    if (fgets(buffer, MAX_FILE_LINE_LENGTH, file) == NULL) {
        handle_error(errno, "could not get input");
    }
    char *input = malloc(sizeof(char) * (strlen(buffer) + 1));
    input[0] = '\0';
    strcpy(input, buffer); 
    input[strlen(input) - 1] = '\0';
    token_list_t *token_list_actual = tokenize_input(input);
    free(input);

    token_list_components_t *token_list_expected = malloc(sizeof(token_list_components_t));
    // find validity
    if (fgets(buffer, MAX_FILE_LINE_LENGTH, file) == NULL) {
        handle_error(errno, "could not get validity");
    }
    char *validity = malloc(sizeof(char) * (strlen(buffer) + 1));
    validity[0] = '\0';
    strcpy(validity, buffer);
    if (!strcmp(validity, "true\n")) {
        token_list_expected->is_valid = true;
    }
    else if (!strcmp(validity, "false\n")) {
        token_list_expected->is_valid = false;
    }
    else {
        handle_error(EIO, "unexpected validity value");
    }
    free(validity);

    // find num tokens
    if (fgets(buffer, MAX_FILE_LINE_LENGTH, file) == NULL) {
        handle_error(errno, "could not get the number of tokens");
    }
    size_t num_args = convert_string_to_int_base_ten(buffer);
    token_list_expected->num_tokens = num_args;
    token_list_expected->tokens = malloc(sizeof(token_components_t *) * num_args);

    for (size_t i = 0; i < num_args; i++) {
        token_components_t *token = file_read_token(file, buffer);
        token_list_expected->tokens[i] = token;
    }
    
    close_file(file, file_name_folder);
    return (test_return_t) {
        .expected = token_list_expected,
        .actual = token_list_actual,
        .freer_expected = (free_func_t) token_list_components_free,
        .freer_actual = (free_func_t) token_list_free
    };
}

/* TOKEN COUNT/VALIDITY TESTS */
/* ______________________________________________________________________________________________*/
test_return_t test_token_list_ceq_simple() {
    return token_list_count_equality_test_util("token one | token two", 2, true);
}

test_return_t test_token_list_ceq_single() {
    return token_list_count_equality_test_util("token one", 1, true);
}

test_return_t test_token_list_ceq_nowhitespace() {
    return token_list_count_equality_test_util("tokenone|tokentwo|tokenthree|tokenfour", 4, true);
}

test_return_t test_token_list_ceq_onlywhitespace() {
    return token_list_count_equality_test_util("             |           ", 0, false);
}

test_return_t test_token_list_ceq_simple_quotes() {
    return token_list_count_equality_test_util(" command \"| it is an interesting \" man | command \" don't | mind | these | tokens \" ", 2, true);
}

test_return_t test_token_list_ceq_even_backslash() {
    return token_list_count_equality_test_util("command \\\\\\\\\\\\ \" | \" hey ", 1, true);
}

test_return_t test_token_list_ceq_odd_backslash() {
    return token_list_count_equality_test_util("\\\\\\\\\\\" | \\\" hey ", 2, true);
}

test_return_t test_token_list_ceq_backslash_pipe() {
    return token_list_count_equality_test_util(" sup sup sup \\| hey ", 1, true);
}

test_return_t test_token_list_ceq_backslash_only() {
    return token_list_count_equality_test_util("\\\\\\\\|\\\\\\\\\\", 2, true);
}

test_return_t test_token_list_ceq_quotes_only_even() {
    return token_list_count_equality_test_util("command \"\" \"\"| command \"\" \"\" \"\"", 2, true);
}

test_return_t test_token_list_ceq_multiple_pipes() {
    return token_list_count_equality_test_util("||   |||||||    ||||  |", 0, false);
}

test_return_t test_token_list_ceq_quote_chars() {
    return token_list_count_equality_test_util("\\\"\\\"\\\"|\\\"\\\"", 2, true);
}

/* TOKENIZING TESTS */
test_return_t test_token_list_simple() {
    return deconstruct_file_test_case("simple");
}

test_return_t test_token_list_one_token() {
    return deconstruct_file_test_case("one_token");
}

test_return_t test_token_list_invalid_token() {
    return deconstruct_file_test_case("invalid_token");
}

test_return_t test_token_list_invalid_third_token() {
    return deconstruct_file_test_case("invalid_third_token");
}

/* GENERAL FUNCTIONS */
/* ______________________________________________________________________________________________*/

void do_token_list_test(tester_t tester, char *name) {
    char *test_suffix = "_token_list_test";
    char token_list_test_name[strlen(name) + strlen(test_suffix) + 1];
    token_list_test_name[0] = '\0';
    strcat(token_list_test_name, name);
    strcat(token_list_test_name, test_suffix); 
    do_test(tester, token_list_test_name, token_list_equality_tester);
}

void do_token_list_count_validity_test(tester_t tester, char *name) {
    char *test_suffix = "_token_list_count_validity_test";
    char token_list_test_name[strlen(name) + strlen(test_suffix) + 1];
    token_list_test_name[0] = '\0';
    strcat(token_list_test_name, name);
    strcat(token_list_test_name, test_suffix); 
    do_test(tester, token_list_test_name, token_list_count_validity_equality_tester);
}

int main(int argc, char **argv) {
    do_token_list_count_validity_test(test_token_list_ceq_simple, "simple");
    do_token_list_count_validity_test(test_token_list_ceq_single, "single");
    do_token_list_count_validity_test(test_token_list_ceq_nowhitespace, "nowhitespace");
    do_token_list_count_validity_test(test_token_list_ceq_simple_quotes, "simple_quotes");
    do_token_list_count_validity_test(test_token_list_ceq_even_backslash, "even_backslash");
    do_token_list_count_validity_test(test_token_list_ceq_odd_backslash, "odd_backslash");
    do_token_list_count_validity_test(test_token_list_ceq_backslash_pipe, "backslash_pipe");
    do_token_list_count_validity_test(test_token_list_ceq_backslash_only, "backslash_only");
    do_token_list_count_validity_test(test_token_list_ceq_quotes_only_even, "quotes_only_even");
    do_token_list_count_validity_test(test_token_list_ceq_multiple_pipes, "multiple_pipes");
    do_token_list_count_validity_test(test_token_list_ceq_quote_chars, "quote_chars");
    do_token_list_test(test_token_list_simple, "simple");
    do_token_list_test(test_token_list_one_token, "one_token");
    do_token_list_test(test_token_list_invalid_token, "invalid_token");
    do_token_list_test(test_token_list_invalid_third_token, "invalid_third_token");
}