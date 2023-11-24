#include <stdlib.h>
#include <stdio.h>

#ifndef ERROR_HANDLING_H
#define ERROR_HANDLING_H

/* Handle semantic errors in a consistent, appropriate way */
/* Does not display a stacktrace */
void handle_error_no_stacktrace(int errnum, char *message);

/* For the girl who wants to have it all */
/* (She can!) */
/* Will display a useful stacktrace with line numbers */
void handle_error(int errnum, char *message);

void handle_recoverable_error(int errnum, char *message);

#endif
