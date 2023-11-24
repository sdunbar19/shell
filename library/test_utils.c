#include "test_utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>

void do_test(tester_t tester, char *desc, equality_tester_t eq_tester) {
    printf("Running test %s\n", desc);
    test_return_t return_val = tester();
    bool passed = eq_tester(return_val);
    if (passed) {
        printf("    Test %s passed\n", desc);
    }
    else {
        printf("    Test %s failed\n", desc);
    }
    return_val.freer_expected(return_val.expected);
    return_val.freer_actual(return_val.actual);
}
