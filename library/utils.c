#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdbool.h>
#include "error_handling.h"
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

extern const int MAX_PATH;
extern const size_t MAX_BUFFER;

char *make_heap_string(char *stack_string) {
    char *heap = malloc(sizeof(char) * (strlen(stack_string) + 1));
    heap[0] = '\0';
    strcpy(heap, stack_string);
    return heap;
}

void set_curr_dir(char *curr_dir) {
    char *ptr = getcwd(curr_dir, MAX_PATH);
    if (ptr == NULL) {
        handle_error(errno, "getcwd returned NULL parameter");
    }
}

char *create_global_error_handling_variable(char *executable_name) {
    char curr_dir[MAX_PATH];
    set_curr_dir(curr_dir);
    char *full_path = malloc(sizeof(char) * MAX_PATH);
    full_path[0] = '\0';
    strcpy(full_path, curr_dir);
    strcat(full_path, "/");
    strcat(full_path, executable_name);
    return full_path;
}

FILE *open_file(char *file_name, char *mode) {
    FILE *f = fopen(file_name, mode);
    if (f == NULL) {
        char str[MAX_BUFFER];
        if (sprintf(str, "file %s failed to open", file_name) < 0) {
            handle_error(errno, "sprintf failed in file open");
        }
        handle_error(errno, str);
    }
    return f;
}

void close_file(FILE *f, char *file_name) {
    if (fclose(f) == EOF) {
        char str[MAX_BUFFER];
        if (sprintf(str, "file %s failed to close", file_name) < 0) {
            handle_error(errno, "sprintf failed in file close");
        }
        handle_error(errno, str);
    }
}

void clear_file(char *file_name) {
    FILE *f = open_file(file_name, "w");
    close_file(f, file_name);
}

size_t convert_string_to_int_base_ten(char *str) {
    size_t num_cases = strtol(str, NULL, 10);
    if (num_cases == 0 && errno != 0) {
        handle_error(errno, "error reading number of cases");
    }
    return num_cases;
}

void wait_for_child() {
    int status;
    pid_t child_pid = wait(&status);
    if (!(WIFSIGNALED(status) && WTERMSIG(status) == SIGTERM)) {
        if (!WIFEXITED(status)) {
            char str[MAX_BUFFER];
                if (sprintf(str, "child %d failed to terminate", child_pid) < 0) {
                    handle_error(errno, "could not get sprintf value");
                }
            handle_recoverable_error(EIO, str);
        }
        else {
            int exit_status = WEXITSTATUS(status); 
            if (exit_status != 0) {
                char str[MAX_BUFFER];
                if (sprintf(str, "child %d failed with exit code %d", child_pid, exit_status) < 0) {
                    handle_error(errno, "could not get sprintf value");
                }
                handle_recoverable_error(EIO, str);
            }
        }
    }
}