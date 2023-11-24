#include "error_handling.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <execinfo.h>
#include <string.h>
#include "global_consts.h"

const size_t NUM_ERRORS = 10;
extern const int MAX_PATH;
extern char *shell_location;
const char *BASE_SHELL_LOCATION = "bin/mysh";

void handle_error_message(int errnum, char *message) {
    fprintf(stderr, "ERROR: %s\n", message);
    fprintf(stderr, "Failed with errno %d\n", errnum);
    perror("Error printed by perror");
}

void handle_error_no_stacktrace(int errnum, char *message) {
    handle_error_message(errnum, message);
    exit(1);
}

char *find_line_hex(char *line) {
    char *ptr1 = strchr(line, '(') + 1; // move past the [
    char *ptr2 = strchr(line, ')');
    size_t len = ptr2 - ptr1;
    char *hex = malloc(sizeof(char) * (len + 1));
    if (hex == NULL) {
        int errnum = errno;
        handle_error_no_stacktrace(errnum, "Error with malloc of 'hex' in find_line_hex");
    }
    strncpy(hex, ptr1, len);
    hex[len] = '\0';
    return hex;
}

char *build_command(char *line) {
    char *hex = find_line_hex(line);
    char *command_p1 = "addr2line -a ";
    char *command_p2 = " -e ";
    if (shell_location == NULL) {
        shell_location = (char *) BASE_SHELL_LOCATION;
    }
    char *command = malloc(sizeof(char) * (strlen(command_p1) + strlen(command_p2) 
                                           + strlen(shell_location) + strlen(hex) + 1));
    if (command == NULL) {
        int errnum = errno;
        handle_error_no_stacktrace(errnum, "Error with malloc of 'command' in print_line_no_from_binary");
    }
    command[0] = '\0';
    strcat(command, command_p1);
    strcat(command, hex);
    strcat(command, command_p2);
    strcat(command, shell_location);
    free(hex);
    return command;
}

char *get_command_output(char *command) {
    FILE *output = popen(command, "r");
    if (output == NULL) {
        int errnum = errno;
        handle_error_no_stacktrace(errnum, "Error with getting output of error command");
    }
    char *line_human = malloc(sizeof(char) * MAX_PATH);
    fgets(line_human, MAX_PATH, output);
    char *result = fgets(line_human, MAX_PATH, output);
    if (result == NULL) {
        fprintf(stderr, "ERROR: No second line to print! \n");
        free(line_human);
        line_human = NULL;
    }
    return line_human;
}

void print_line_no_from_binary(char *line) {
    char *command = build_command(line);
    char *line_human = get_command_output(command);
    if (line_human == NULL || !strcmp(line_human, "??:?\n") || !strcmp(line_human, "??:0\n")) {
        fprintf(stderr, "%s\n", line);
    }
    else {
        fprintf(stderr, "%s", line_human);
    }
    free(command);
    free(line_human);
}

void print_backtrace() {
    printf("Printing backtrace for error\n");
    void *buffer[NUM_ERRORS];
    size_t size = backtrace(buffer, NUM_ERRORS);
    char **trace = backtrace_symbols(buffer, size);
    if (trace != NULL) {
        printf("Obtained %ld stack frames for error\n", size);
        for (size_t i = 0; i < size; i++) {
            print_line_no_from_binary(trace[i]);
        }
        free(trace);
    }
    else {
        printf("Error in obtaining stack frames for error\n");
    }
}

void handle_error(int errnum, char *message) {
    handle_error_message(errnum, message);
    fprintf(stderr, "\n");
    print_backtrace();
    exit(1);
}

void handle_recoverable_error(int errnum, char *message) {
    fprintf(stderr, "ERROR: %s\n", message);
}