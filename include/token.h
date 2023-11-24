#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifndef TOKEN_H
#define TOKEN_H

typedef struct token token_t;

bool is_whitespace(char c);

token_t *token_init(char **token_feeder);

void token_free(token_t *token);

bool token_get_is_valid(token_t *token);

char *token_get_input_file(token_t *token);

void token_try_input_file(token_t *token, char *input_file);

void token_try_output_file(token_t *token, char *output_file);

char *token_get_output_file(token_t *token);

char *token_get_command(token_t *token);

size_t token_get_num_args(token_t *token);

char **token_get_args(token_t *token);

void skip_whitespace(char **token_feeder);

void token_print(token_t *token);

#endif