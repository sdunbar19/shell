#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

typedef void (*free_func_t)(void *);

typedef struct test_return {
    void *expected;
    void *actual;
    free_func_t freer_expected;
    free_func_t freer_actual;
} test_return_t;

typedef test_return_t (*tester_t)(void);

typedef bool (*equality_tester_t)(test_return_t);

void do_test(tester_t tester, char *desc, equality_tester_t eq_tester);

#endif
