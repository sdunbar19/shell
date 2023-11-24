#include "predefined_commands.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <execinfo.h>
#include <string.h>
#include "token.h"
#include <signal.h>
#include <sys/types.h>
#include "error_handling.h"

const size_t NUM_COMMANDS = 2;

void execute_changedir(token_t *token) {
    size_t num_args = token_get_num_args(token);
    if (num_args > 2) {
        handle_recoverable_error(EIO, "too many arguments passed to cd command");
        return;
    }
    // change to home directory
    char *change_to = getenv("HOME");
    if (num_args == 2) {
        char **args = token_get_args(token);
        char *arg = args[1];
        if (strcmp(arg, "~") != 0) {
            change_to = arg;
        }
    }
    if (chdir(change_to) == -1) {
        handle_recoverable_error(errno, "changing directory failed");
        return;
    }
}

bool execute_predefined(token_t *token) {
    char *command = token_get_command(token);
    if(!strcmp(command, "cd")) {
        execute_changedir(token);
        return true;
    }
    if (!strcmp(command, "exit")) {
        kill(0, SIGTERM);
    }
    return false;
}