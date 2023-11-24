#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>
#include "global_consts.h"
#include "error_handling.h"
#include "tokenizer.h"
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "predefined_commands.h"
#include <string.h>
#include "utils.h"

extern const int MAX_PATH;
const size_t MAX_USER = 30;
const size_t MAX_LINE_LENGTH = 4096;
extern const size_t MAX_BUFFER;
const char *FOLDER_NAME = "/usr/bin/";
const char *TEMP_FILE_1 = "tempfiles/file1.txt";
const char *TEMP_FILE_2 = "tempfiles/file2.txt";
const char *EXECUTABLE = "bin/mysh";
const bool DEBUG_MODE = false;

char *shell_location = NULL;

char *get_command_shell_prompt() {
    char curr_dir[MAX_PATH];
    char username[MAX_USER];
    set_curr_dir(curr_dir);
    struct passwd *user = getpwuid(getuid());
    if (user == NULL) {
        handle_error(errno, "getpwuid returned NULL parameter");
    }
    strcpy(username, user->pw_name);
    char *command_shell_prompt = malloc(sizeof(char) * (strlen(curr_dir) + strlen(username) + 4));
    if (command_shell_prompt == NULL) {
        handle_error(errno, "malloc failed for command_shell_prompt");
    }
    command_shell_prompt[0] = '\0';
    strcat(command_shell_prompt, username);
    strcat(command_shell_prompt, ":");
    strcat(command_shell_prompt, curr_dir);
    strcat(command_shell_prompt, "> ");
    return command_shell_prompt;
}

char *get_full_command_name(char *command) {
    char *command_file_name = malloc(sizeof(char) * MAX_PATH);
    command_file_name[0] = '\0';
    strcpy(command_file_name, FOLDER_NAME);
    strcat(command_file_name, command);
    return command_file_name;
}

bool is_internal_command(char *command) {
    char *command_full = get_full_command_name(command);
    bool exists = access(command_full, F_OK) == 0;
    free(command_full);
    return exists;
}

bool replace_input(token_t *token) {
    char *token_input = token_get_input_file(token);
    if (!strcmp(token_input, "")) {
        return true;
    }
    bool exists = access(token_input, F_OK) == 0;
    if (!exists) {
        char str[MAX_BUFFER];
        if (sprintf(str, "file %s does not exist", token_input) < 0) {
            handle_error(errno, "could not get sprintf value");
        }
        handle_recoverable_error(errno, str);
        return false;
    }
    int in_fd = open(token_input, O_RDONLY);
    if (dup2(in_fd, STDIN_FILENO) == -1) {
        handle_error(errno, "failed to set input file");
    }
    close(in_fd);
    return true;
}

void replace_output(token_t *token) {
    char *token_output = token_get_output_file(token);
    if (!strcmp(token_output, "")) {
        return;
    }
    clear_file(token_output); // will create file if no exist
    int out_fd = open(token_output, O_WRONLY);
    if (dup2(out_fd, STDOUT_FILENO) == -1) {
        handle_error(errno, "failed to set output file");
    }
    close(out_fd);
}

void handle_internal_command(token_t *token) {
    if (!replace_input(token)) {
        return;
    }
    replace_output(token);
    char *full_command_path = get_full_command_name(token_get_command(token));
    char **args = token_get_args(token);
    free(args[0]);
    args[0] = full_command_path;
    if (execve(full_command_path, args, NULL) == -1) {
        handle_error(errno, "error with execve command");
    }
}

void handle_command(token_t *token) {
    replace_input(token);
    replace_output(token);
    if (execve(token_get_args(token)[0], token_get_args(token), NULL) == -1) {
        handle_error(errno, "error with execve command");
    }
}

void set_temp_files(char *temp_files_array[], char temp1[], char temp2[], char *base_dir_of_shell) {
    temp1[0] = '\0';
    strcat(temp1, base_dir_of_shell);
    strcat(temp1, "/");
    strcat(temp1, TEMP_FILE_1);
    temp2[0] = '\0';
    strcat(temp2, base_dir_of_shell);
    strcat(temp2, "/");
    strcat(temp2, TEMP_FILE_2);
    temp_files_array[0] = temp1;
    temp_files_array[1] = temp2;
    clear_file(temp1);
    clear_file(temp2);
}

void handle_child(char *command, token_t *token) {
    bool command_exists = access(command, F_OK) == 0;
    if (command_exists) {
        handle_command(token);
    }
    else {
        bool is_predefined = execute_predefined(token);
        if (!is_predefined) {
            if (is_internal_command(command)) {
                handle_internal_command(token);
            }
            else {
                char str[MAX_BUFFER];
                if (sprintf(str, "%s not a valid command!", command) < 0) {
                    handle_error(errno, "sprintf failed on valid command error");
                }
                handle_recoverable_error(EIO, str);
            }
        }
    }
}

int main(int argc, char **argv) {
    char *base_dir_of_shell = malloc(sizeof(char) * MAX_PATH);
    set_curr_dir(base_dir_of_shell);
    shell_location = create_global_error_handling_variable((char *) EXECUTABLE);
    char *TEMP_FILES[2];
    char TEMP_1[strlen(base_dir_of_shell) + strlen(TEMP_FILE_1) + 2];
    char TEMP_2[strlen(base_dir_of_shell) + strlen(TEMP_FILE_2) + 2];
    set_temp_files(TEMP_FILES, TEMP_1, TEMP_2, base_dir_of_shell);

    char curr_dir[MAX_PATH];
    char username[MAX_USER];
    char input[MAX_LINE_LENGTH];

    while (true) {
        char *command_shell_prompt = get_command_shell_prompt(curr_dir, username);
        printf("%s", command_shell_prompt);
        fgets(input, MAX_LINE_LENGTH, stdin);
        token_list_t *tokens = tokenize_input(input);
        if (!token_list_get_is_valid(tokens)) {
            handle_recoverable_error(EIO, "invalid input detected");
        }
        else {
            size_t num_tokens = token_list_get_num_tokens(tokens);
            size_t parity = 0;

            for (size_t i = 0; i < num_tokens; i++) {
                token_t *token = token_list_get_token_from_list(tokens, i);
                if (i > 0) {
                    token_try_input_file(token, TEMP_FILES[parity]);
                }
                if (i < num_tokens - 1) {
                    token_try_output_file(token, TEMP_FILES[(parity + 1) % 2]);
                }
                parity += 1;
                parity = parity % 2;
                char *command = token_get_command(token);
                pid_t pid = fork();
                if (pid == 0) {
                    handle_child(command, token);
                }
                else if (pid > 0) {
                    wait_for_child();
                }
                else {
                    handle_error(errno, "fork failed to create child");
                }
            }
            free(command_shell_prompt);
            token_list_free(tokens);
        }
    }
    free(base_dir_of_shell);
    free(shell_location);
    return 0;
}