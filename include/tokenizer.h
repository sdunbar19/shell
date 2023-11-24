#include "token.h"
#include <stdio.h>
#include <stdlib.h>

#ifndef TOKENIZER_H
#define TOKENIZER_H

typedef struct token_list token_list_t;

token_list_t *tokenize_input(char *input);

size_t token_list_get_num_tokens(token_list_t *token_list);

token_t *token_list_get_token_from_list(token_list_t *token_list, size_t index);

bool token_list_get_is_valid(token_list_t *token_list);

void token_list_free(token_list_t *token_list);

void token_list_print(token_list_t *token_list);

#endif
