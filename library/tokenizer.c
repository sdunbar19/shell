#include "tokenizer.h"
#include "token.h"
#include "error_handling.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <string.h>

struct token_list {
    bool is_valid;
    size_t num_tokens;
    token_t **tokens;
};

size_t count_num_tokens(char *input, bool *success) {
    size_t num_tokens = 0;
    bool in_double_quotes = false;
    size_t consecutive_backslash = 0;
    bool found_chars_after_pipe = false;
    for (int i = 0; i < strlen(input); i++) {
        char c = input[i];
        if (c == '\\') {
            found_chars_after_pipe = true;
            consecutive_backslash += 1;
        }
        else {
            if (c == '"' && consecutive_backslash % 2 == 0) {
                in_double_quotes = !in_double_quotes;
                found_chars_after_pipe = true;
            }
            else if (c == '|' && consecutive_backslash % 2 == 0) {
                if (!in_double_quotes && found_chars_after_pipe) {
                    num_tokens += 1;
                    found_chars_after_pipe = false;
                }
                else if (!in_double_quotes && !found_chars_after_pipe) {
                    *success = false;
                }
            }
            else if (!is_whitespace(c)) {
                found_chars_after_pipe = true;
            }
            consecutive_backslash = 0;
        }
    }
    if (found_chars_after_pipe) {
        num_tokens += 1;
    }
    else {
        *success = false;
    }
    if (in_double_quotes) {
        *success = false;
    }
    return num_tokens;
}

size_t token_list_get_num_tokens(token_list_t *token_list) {
    return token_list->num_tokens;
}

token_t *token_list_get_token_from_list(token_list_t *token_list, size_t index) {
    if (index >= token_list->num_tokens) {
        handle_error(EIO, "token index exceeds number of tokens");
    }
    return token_list->tokens[index];
}

bool token_list_get_is_valid(token_list_t *token_list) {
    return token_list->is_valid;
}

token_list_t *tokenize_input(char *input) {
    bool *success = malloc(sizeof(bool));
    *success = true;
    size_t num_tokens = count_num_tokens(input, success);
    token_list_t *my_token_list = malloc(sizeof(token_list_t));
    if (my_token_list == NULL) {
        handle_error(errno, "my_token_list malloc failure");
    }
    my_token_list->num_tokens = 0;
    my_token_list->is_valid = *success;
    my_token_list->tokens = malloc(sizeof(token_t *) * num_tokens);
    free(success);
    if (!my_token_list->is_valid) {
        return my_token_list;
    }
    size_t counted_tokens = 0;
    while (counted_tokens < num_tokens && *input != '\0') {
        token_t *token = token_init(&input);
        my_token_list->tokens[counted_tokens] = token;
        if (!token_get_is_valid(token)) {
            token_free(token);
            my_token_list->is_valid = false;
            return my_token_list;
        }
        counted_tokens++;
        my_token_list->num_tokens = counted_tokens;
    }
    if (*input != '\0' || counted_tokens < num_tokens) {
        handle_error(ENOTRECOVERABLE, "mismatch between end of string, counted tokens");
    }
    return my_token_list;
}

void token_list_free(token_list_t *token_list) {
    for (size_t i = 0; i < token_list->num_tokens; i++) {
        token_free(token_list->tokens[i]);
    }
    free(token_list->tokens);
    free(token_list);
}

void token_list_print(token_list_t *token_list) {
    printf("\n");
    printf("Printing token list\n");
    printf("Token list is valid: %d\n", token_list->is_valid);
    printf("Token list num tokens: %zu\n", token_list->num_tokens);
    for (size_t i = 0; i < token_list->num_tokens; i++) {
        printf("\n");
        printf("Printing token %zu\n", i);
        token_print(token_list->tokens[i]);
    }
    printf("\n");
}