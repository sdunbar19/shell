#include "token.h"
#include "error_handling.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include "test_utils.h"
#include "utils.h"
#include <string.h>

struct token {
    bool is_valid;
    char *input_file;
    char *output_file;
    char *command;
    size_t num_args;
    char **argv;
};

typedef bool (*is_done_t)(char, size_t, token_t *);

bool is_whitespace(char c) {
    return c == ' ' || c == '\n' || c == '\t' || c == '\n' || c == '\f';
}

bool is_special_character(char c) {
    return c == '|' || c == '>' || c == '<';
}

bool is_done_quoted(char c, size_t num_backslashes, token_t *token) {
    if (c == '\0') {
        token->is_valid = false;
        return true;
    }
    return (c == '"' && num_backslashes % 2 == 0);
}

bool is_token_feeder_done(char c, size_t num_backslashes) {
    return (c == '|' && num_backslashes % 2 == 0) || c == '\0';
}

bool is_done_unquoted(char c, size_t num_backslashes, token_t *token) {
    if (c == '"' && num_backslashes % 2 == 0) {
        token->is_valid = false;
        return true;
    }
    return is_token_feeder_done(c, num_backslashes) || (is_special_character(c) && num_backslashes % 2 == 0) || is_whitespace(c);
}

char *get_valid_string(char **token_feeder, is_done_t is_done, token_t *token) {
    size_t num_consecutive_backslashes = 0;
    char *string = malloc(sizeof(char) * (strlen(*token_feeder) + 1));
    if (string == NULL) {
        handle_error(errno, "malloc failed on string");
    }
    size_t token_loc = 0;
    while (!is_done(**token_feeder, num_consecutive_backslashes, token)) {
        if (**token_feeder == '\\') {
            num_consecutive_backslashes++;
        }
        else {
            num_consecutive_backslashes = 0;
        }
        string[token_loc] = **token_feeder;
        (*token_feeder)++;
        token_loc++;
    }
    string[token_loc] = '\0';
    return string;
}

char *get_valid_quoted_string(char **token_feeder, token_t *token) {
    (*token_feeder)++;
    char *string_to_return = get_valid_string(token_feeder, is_done_quoted, token);
    if (**token_feeder != '\0' && token->is_valid) {
        (*token_feeder)++;
    }
    if (!is_whitespace(**token_feeder) && !is_special_character(**token_feeder) && **token_feeder != '\0') {
        token->is_valid = false;
    }
    return string_to_return;
}

char *get_valid_unquoted_string(char **token_feeder, token_t *token) {
    return get_valid_string(token_feeder, is_done_unquoted, token);
}

void skip_whitespace(char **token_feeder) {
    while (!is_token_feeder_done(**token_feeder, 0) && is_whitespace(**token_feeder)) {
        (*token_feeder)++;
    }
}

char *extract_required_string(char **token_feeder, token_t *token) {
    (*token_feeder)++;
    skip_whitespace(token_feeder);
    if (is_token_feeder_done(**token_feeder, 0)) {
        token->is_valid = false;
        char * dummy = malloc(sizeof(char)); // return a dummy pointer to be freed
        if (dummy == NULL) {
            handle_error(errno, "malloc failed on dummy pointer");
        }
    }
    char *input;
    if (**token_feeder == '"') {
        input = get_valid_quoted_string(token_feeder, token);
    }
    else {
        input = get_valid_unquoted_string(token_feeder, token);
    }
    return input;
}

token_t *token_init(char **token_feeder) {
    token_t *token = malloc(sizeof(token_t));
    if (token == NULL) {
        handle_error(errno, "malloc failed on token");
    }
    token->is_valid = true;
    token->input_file = malloc(sizeof(char));
    if (token->input_file == NULL) {
        handle_error(errno, "malloc failed on input file");
    }
    token->input_file[0] = '\0';
    token->output_file = malloc(sizeof(char));
    if (token->output_file == NULL) {
        handle_error(errno, "malloc failed on output file");
    }
    token->output_file[0] = '\0';
    token->command = malloc(sizeof(char));
    if (token->command == NULL) {
        handle_error(errno, "malloc failed on command");
    }
    token->command[0] = '\0';
    size_t num_args = 1;
    token->num_args = num_args;
    char **argv = malloc(sizeof(char *) * num_args);
    if (argv == NULL) {
        handle_error(errno, "malloc failed on argv array");
    }
    
    argv[0] = token->command;
    token->argv = argv;

    skip_whitespace(token_feeder);

    // get command
    if (**token_feeder == '"' || is_special_character(**token_feeder)) {
        token->is_valid = false;
        return token;
    }
    char *command = get_valid_unquoted_string(token_feeder, token);
    free(token->command);
    argv[0] = command;
    token->command = command;

    skip_whitespace(token_feeder);
    while (!is_token_feeder_done(**token_feeder, 0)) {
        if (**token_feeder == '<') {
            if (strlen(token->input_file) > 0) {
                token->is_valid = false;
                return token;
            }
            char *input = extract_required_string(token_feeder, token);
            if (token->is_valid == false) {
                free(input);
                return token;
            }
            free(token->input_file);
            token->input_file = input;
        }
        else if (**token_feeder == '>') {
            if (strlen(token->output_file) > 0) {
                token->is_valid = false;
                return token;
            }
            char *output = extract_required_string(token_feeder, token);
            if (token->is_valid == false) {
                free(output);
                return token;
            }
            free(token->output_file);
            token->output_file = output;
        }
        else {
            char *argument;
            if (**token_feeder == '"') {
                argument = get_valid_quoted_string(token_feeder, token);
            }
            else {
                argument = get_valid_unquoted_string(token_feeder, token);
            }
            if (!token->is_valid) {
                free(argument);
                return token;
            }
            num_args++;
            argv = realloc(argv, sizeof(char *) * num_args);
            token->argv = argv;
            token->num_args = num_args;
            argv[num_args - 1] = argument;
        }
        skip_whitespace(token_feeder);
    }
    if (**token_feeder == '|') {
        (*token_feeder)++;
    }
    skip_whitespace(token_feeder);
    token->num_args = num_args;
    token->argv = realloc(argv, sizeof(char *) * (num_args + 1));
    token->argv[num_args] = '\0';
    return token;
}

void token_free(token_t *token) {
    free(token->input_file);
    free(token->output_file);

    // don't need to free token->command because it will be an argument
    for (size_t i = 0; i < token->num_args; i++) {
        free(token->argv[i]);
    }
    free(token->argv);
    free(token);
}

bool token_get_is_valid(token_t *token) {
    return token->is_valid;
}

char *token_get_input_file(token_t *token) {
    return token->input_file;
}

void token_try_input_file(token_t *token, char *input_file) {
    if (!strcmp(token->input_file, "")) {
        free(token->input_file);
        token->input_file = make_heap_string(input_file);
    }
}

void token_try_output_file(token_t *token, char *output_file) {
    if (!strcmp(token->output_file, "")) {
        free(token->output_file);
        token->output_file = make_heap_string(output_file);
    }
}


char *token_get_output_file(token_t *token) {
    return token->output_file;
}

char *token_get_command(token_t *token) {
    return token->command;
}

size_t token_get_num_args(token_t *token) {
    return token->num_args;
}

char **token_get_args(token_t *token) {
    return token->argv;
}

void token_print(token_t *token) {
    printf("Token validity: %d\n", token->is_valid);
    printf("Token command: %s\n", token->command);
    printf("Token input file: %s\n", token->input_file);
    printf("Token output file: %s\n", token->output_file);
    printf("Token num args (excluding command): %ld\n", token->num_args - 1);
    for (size_t i = 1; i < token->num_args; i++) {
        printf("Token arg %zu: %s\n", i, token->argv[i]);
    }
}   