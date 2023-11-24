#include <stdlib.h>
#include <stdio.h>
#include "token.h"

#ifndef PREDEFINED_FNS_H
#define PREDEFINED_FNS_H

typedef void (*internal_func_t)(token_t *);

bool execute_predefined(token_t *token);

#endif
