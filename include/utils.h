#include <stdlib.h>
#include <stdio.h>

#ifndef UTILS_H
#define UTILS_H

extern const int MAX_PATH;

char *make_heap_string(char *stack_string);

void set_curr_dir(char *curr_dir);

char *create_global_error_handling_variable(char *executable_name);

FILE *open_file(char *file_name, char *mode);

void close_file(FILE *f, char *file_name);

// also works to create a new empty file with this name
void clear_file(char *file_name);

size_t convert_string_to_int_base_ten(char *str);

void wait_for_child();

#endif