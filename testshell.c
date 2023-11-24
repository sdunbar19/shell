#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include "error_handling.h"
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "global_consts.h"
#include "utils.h"
#include <string.h>
#include <time.h>
#include <signal.h>

#define _XOPEN_SOURCE 700

const char *COMMAND = "bin/mysh";
const char *MY_STDIN = "testfiles/test_stdin";
const char *MY_STDOUT = "testfiles/test_stdout";
const char *MY_STDERR = "testfiles/test_stderr";
const char *EXPECTED_STDIN = "testfiles/expected_stdin";
const char *EXPECTED_STDOUT_FOLDER = "testfiles/expected_stdout";
const char *EXECUTABLE = "testshell";
char *shell_location = NULL;
extern const size_t MAX_BUFFER;
extern const int MAX_PATH;

static void handler(int signum) {
    // empty - we dont want to do anything on receiving a SIGTERM
}

void write_next_stdin(FILE *stdin_exp) {
    char buffer[MAX_BUFFER];
    buffer[0] = '\0';

    FILE *stdin_to_write = open_file((char *) MY_STDIN, "w");
    if (fgets(buffer, MAX_BUFFER, stdin_exp) == NULL) {
        handle_error(errno, "fgets failed for stdin. check your number of cases!");
    }
    while (strcmp(buffer, "-\n") != 0) {
        if (fputs(buffer, stdin_to_write) < 0) {
            handle_error(errno, "fputs failed for stdin");
        }
        if (fgets(buffer, MAX_BUFFER, stdin_exp) == NULL) {
            handle_error(errno, "fgets failed for stdin. check your number of cases!");
        }
    }   
    if (fputs("exit\n", stdin_to_write) < 0) {
        handle_error(errno, "fputs failed for stdin");
    }
    close_file(stdin_to_write, (char *) MY_STDIN);
}

void run_shell() {
    // redirect stdin
    int in_fd = open((char *) MY_STDIN, O_RDONLY);
    if (in_fd == -1) {
        handle_error(errno, "failed to open input file");
    }
    if (dup2(in_fd, STDIN_FILENO) == -1) {
        handle_error(errno, "failed to set input file");
    }
    close(in_fd);

    // redirect stdout
    int out_fd = open((char *) MY_STDOUT, O_WRONLY);
    if (out_fd == -1) {
        handle_error(errno, "failed to open output file");
    }
    if (dup2(out_fd, STDOUT_FILENO) == -1) {
        handle_error(errno, "failed to set output file");
    }
    close(out_fd);

    // make the execve call
    char **args = malloc(sizeof(char *) * 2);
    args[0] = (char *) COMMAND;
    args[1] = NULL;
    if (execve(COMMAND, args, NULL) == -1) {
        handle_error(errno, "error with execve command");
    }
}

bool check_file_equality(FILE *expected, FILE *actual) {
    char expBuffer[MAX_BUFFER];
    char actBuffer[MAX_BUFFER];
    char *ptr1 = fgets(expBuffer, MAX_BUFFER, expected);
    char *ptr2 = fgets(actBuffer, MAX_BUFFER, actual);
    while (ptr1 != NULL && ptr2 != NULL) {
        if (strcmp(expBuffer, actBuffer) != 0) {
            return false;
        }
        ptr1 = fgets(expBuffer, MAX_BUFFER, expected);
        ptr2 = fgets(actBuffer, MAX_BUFFER, actual);
    }
    return ptr1 == NULL && ptr2 == NULL;
}

bool compare_test_case_to_output(size_t case_num) {
    FILE *shell_stdout = open_file((char *) MY_STDOUT, "r");
    char case_num_str[MAX_BUFFER];
    if (sprintf(case_num_str, "%zu", case_num) < 0) {
        handle_error(errno, "could not get sprintf value");
    }
    char expected_output_file[MAX_PATH];
    expected_output_file[0] = '\0';
    strcat(expected_output_file, EXPECTED_STDOUT_FOLDER);
    strcat(expected_output_file, "/");
    strcat(expected_output_file, case_num_str);
    FILE *expected_stdout = open_file(expected_output_file, "r");
    return check_file_equality(expected_stdout, shell_stdout);
}

int main() {
    // set the signal handling
    // https://pubs.opengroup.org/onlinepubs/007904875/functions/sigaction.html
    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask); // set the mask to no signals (no blocked signals)
    sa.sa_flags = SA_RESTART; // allow interruptable system calls to restart
    if (sigaction(SIGTERM, &sa, NULL) == -1) {
        handle_error(errno, "Invalid signal setting");
    }

    FILE *stdin_exp = open_file((char *) EXPECTED_STDIN, "r");
    char buffer[MAX_BUFFER];
    fgets(buffer, MAX_BUFFER, stdin_exp);
    if (buffer[strlen(buffer) - 1] == '\n') {
        buffer[strlen(buffer) - 1] = '\0';
    }
    size_t num_cases = convert_string_to_int_base_ten(buffer);
    shell_location = create_global_error_handling_variable((char *) EXECUTABLE);
    for (size_t i = 0; i < num_cases; i++) {
        // clear the input and output files
        clear_file((char *) MY_STDIN);
        clear_file((char *) MY_STDOUT);
        write_next_stdin(stdin_exp);
        //time_t time_before = time(0);
        pid_t pid = fork();
        if (pid == 0) {
            run_shell();
        }
        else if (pid > 0) {
            wait_for_child();
            //time_t time_taken = time(0) - time_before;
            if (!compare_test_case_to_output(i + 1)) {
                printf("FAILED case %zu\n", i + 1);
            }
            else {
                printf("PASSED case %zu\n", i + 1);
            }
        }
    }
    free(shell_location);
    close_file(stdin_exp, (char *) EXPECTED_STDIN);
}
