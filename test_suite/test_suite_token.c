#include "test_utils.h"
#include "token.h"
#include "token_test_utils.h"
#include "utils.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

const char *shell_location = "bin/mysh";

test_return_t token_test_util(char *input, bool is_valid_expected, char *command_expected,
                              char *input_expected, char *output_expected, size_t num_args_expected, char **args) {
    token_t *token_actual = token_init(&input);
    token_components_t *token_components = malloc(sizeof(token_components_t));
    token_components->is_valid = is_valid_expected;
    token_components->command = command_expected;
    token_components->input_file = input_expected;
    token_components->output_file = output_expected;
    token_components->num_args = num_args_expected;
    token_components->argv = args;
    return (test_return_t) {.expected = token_components,
                            .actual = token_actual,
                            .freer_expected = (free_func_t) token_component_free,
                            .freer_actual = (free_func_t) token_free};
}

test_return_t test_token_simple() {
    size_t num_args = 3;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("arg1");
    args[2] = make_heap_string("arg2");
    char *input_string = make_heap_string("");
    char *output_string = make_heap_string("");
    return token_test_util("grep arg1 arg2 | more", true, args[0], input_string, output_string, num_args, args);
} 

test_return_t test_token_input() {
    size_t num_args = 2;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("arg1");
    char *input_string = make_heap_string("input.txt");
    char *output_string = make_heap_string("");
    return token_test_util("grep arg1 < input.txt | more", true, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_output() {
    size_t num_args = 2;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("arg1");
    char *output_string = make_heap_string("output.txt");
    char *input_string = make_heap_string("");
    return token_test_util("grep arg1 > output.txt | more", true, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_input_output() {
    size_t num_args = 2;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("arg1");
    char *output_string = make_heap_string("output.txt");
    char *input_string = make_heap_string("input.txt");
    return token_test_util("grep arg1 > output.txt < input.txt | more", true, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_solo() {
    size_t num_args = 2;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("arg1");
    char *output_string = make_heap_string("output.txt");
    char *input_string = make_heap_string("input.txt");
    return token_test_util("grep arg1 > output.txt < input.txt", true, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_no_whitespace() {
    size_t num_args = 2;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("arg1");
    char *output_string = make_heap_string("output.txt");
    char *input_string = make_heap_string("input.txt");
    return token_test_util("grep arg1>output.txt<input.txt", true, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_lots_whitespace() {
    size_t num_args = 2;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("arg1");
    char *output_string = make_heap_string("output.txt");
    char *input_string = make_heap_string("input.txt");
    return token_test_util("grep arg1        >      output.txt\n   <\finput.txt", true, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_cant_quotes_unquotes() {
    size_t num_args = 1;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    char *input_string = make_heap_string("");
    char *output_string = make_heap_string("");
    return token_test_util("grep arg1\"arg2\" | more", false, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_quoted_strings() {
    size_t num_args = 3;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("     arg1");
    args[2] = make_heap_string("  arg2");
    char *input_string = make_heap_string("");
    char *output_string = make_heap_string("");
    return token_test_util("grep \"     arg1\" \"  arg2\" | more", true, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_quoted_specials() {
    size_t num_args = 3;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("arg1|<>");
    args[2] = make_heap_string("arg2\\\\");
    char *input_string = make_heap_string("");
    char *output_string = make_heap_string("");
    return token_test_util("grep \"arg1|<>\" \"arg2\\\\\" | more", true, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_backslash_chars() {
    size_t num_args = 4;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("grep");
    args[1] = make_heap_string("\\|");
    args[2] = make_heap_string("\\<");
    args[3] = make_heap_string("\\>");
    char *input_string = make_heap_string("");
    char *output_string = make_heap_string("");
    return token_test_util("grep \\| \\< \\> | more", true, args[0], input_string, output_string, num_args, args);
}

test_return_t test_token_open_quotes() {
    size_t num_args = 1;
    char **args = malloc(sizeof(char *) * num_args);
    args[0] = make_heap_string("echo");
    char *input_string = make_heap_string("");
    char *output_string = make_heap_string("");
    return token_test_util("echo \"cat", false, args[0], input_string, output_string, num_args, args);
}

void do_token_test(tester_t tester, char *name) {
    char token_test_name[strlen(name) + 12];
    token_test_name[0] = '\0';
    strcat(token_test_name, name);
    strcat(token_test_name, "_token_test"); 
    do_test(tester, token_test_name, token_equality_tester);
}

int main(int argc, char **argv) {
    do_token_test(test_token_simple, "simple");
    do_token_test(test_token_input, "input");
    do_token_test(test_token_output, "output"); 
    do_token_test(test_token_input_output, "input_output"); 
    do_token_test(test_token_solo, "solo"); 
    do_token_test(test_token_no_whitespace, "no_whitespace"); 
    do_token_test(test_token_lots_whitespace, "lots_whitespace"); 
    do_token_test(test_token_cant_quotes_unquotes, "cant_quotes_unquotes"); 
    do_token_test(test_token_quoted_strings, "quoted_strings"); 
    do_token_test(test_token_quoted_specials, "quoted_specials");
    do_token_test(test_token_backslash_chars, "backslash_chars");
    do_token_test(test_token_open_quotes, "open_quotes");
}