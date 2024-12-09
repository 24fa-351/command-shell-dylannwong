#include "shell_helper.h"

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

// int output_fd = 0;
size_t set_variable_count = 0;
struct variable set_variables[1000];
char working_directory[1000];

void add_character_to_string(char *str, char c) {
    int len = strlen(str);
    str[len] = c;
    str[len + 1] = '\0';
}

void split(char *cmd, char *words[], char delimiter) {
    int word_count = 0;
    char *next_char = cmd;
    char current_word[10000];
    strcpy(current_word, "");

    while (*next_char != '\0') {
        if (*next_char == delimiter) {
            words[word_count++] = strdup(current_word);
            strcpy(current_word, "");
        } else {
            add_character_to_string(current_word, *next_char);
        }
        ++next_char;
    }

    words[word_count++] = strdup(current_word);
    words[word_count] = NULL;
}
bool find_absolute_path(char *cmd, char *absolute_path) {
    char *directories[1000];

    split(getenv("PATH"), directories, ':');

    // break path up by colon and look in array until i find path plus cmd

    for (int i = 0; directories[i] != NULL; i++) {
        char path[1000];
        strcpy(path, directories[i]);
        add_character_to_string(path, '/');
        strcat(path, cmd);
        if (access(path, X_OK) == 0) {
            strcpy(absolute_path, path);
            return true;
        }
    }
    return false;
}

bool is_command_implemented(char **words, int length) {
    int words_length = length;
    if (words_length > 1) {
        if (strcmp(words[1], "<") == 0) {
            int input_fd = 0;
            input_fd = open(words[0], O_RDONLY);
            if (input_fd == -1) {
                printf("Error: File not found\n");
                return false;
            }
            char input[1000];
            ssize_t bytes_read = read(input_fd, input, sizeof(input) - 1);

            input[bytes_read] = '\0';
            split(input, words, ' ');
            words_length = 0;
            while (words[words_length] != NULL) {
                words_length++;
            }
        }
    }

    if (strcmp(words[0], "exit") == 0) {
        exit(0);
    }

    if (strncmp(words[0], "./", 2) == 0 && words_length == 1) {
        pid_t pid = fork();
        if (pid == 0) {
            execve(words[0], words, NULL);
            perror("execve failed");
            exit(EXIT_FAILURE);
        } else if (pid > 0) {
            int status;
            waitpid(pid, &status, 0);
        } else {
            perror("fork failed");
        }
        return true;
    }

    if (strcmp(words[0], "cd") == 0) {
        if (words_length < 2) {
            fprintf(stderr, "cd: missing argument\n");
        } else if (strcmp(words[1], "..") == 0) {
            char *files[1000];
            split(working_directory, files, '/');
            char new_directory[1000] = "";
            for (int i = 1; files[i + 1] != NULL; i++) {
                strcat(new_directory, "/");
                strcat(new_directory, files[i]);
            }
            if (chdir(new_directory) != 0) {
                perror("cd failed");
                return 0;
            } else {
                strcpy(working_directory, new_directory);
                return 1;
            }
        } else {
            if (chdir(words[1]) != 0) {
                perror("cd failed");
            } else {
                char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    strcpy(working_directory, cwd);
                    return 1;
                } else {
                    perror("cd failed");
                }
            }
        }
    }

    if (strcmp(words[0], "echo") == 0) {
        char output[10000] = "";
        for (int i = 1; words[i] != NULL; i++) {
            // prints output up to > character
            if (strcmp(words[i], ">") == 0 || strcmp(words[i], ">>") == 0) {
                break;
            }
            // checks if first char is $, if so, it is a variable
            if (words[i][0] == '$') {
                for (int j = 0; j < set_variable_count; j++) {
                    if (strcmp(words[i] + 1, set_variables[j].name) == 0) {
                        strcat(output, set_variables[j].value);
                    }
                }
            } else {  // else add it to string
                strcat(output, words[i]);
                strcat(output, " ");
            }
        }
        // echo output like normal
        printf("%s\n", output);
        return true;
    }

    if (strcmp(words[0], "set") == 0) {
        int already_set = 0;
        for (int i = 0; i < set_variable_count; i++) {
            if (strcmp(words[1], set_variables[i].name) == 0) {
                set_variables[i].value = strdup(words[2]);
                printf("Variable %s set to %s\n", words[1], words[2]);
            }
        }
        if (already_set == 1) {
            return "";
        }
        struct variable new_variable;
        new_variable.name = strdup(words[1]);
        new_variable.value = strdup(words[2]);
        set_variables[set_variable_count] = new_variable;
        set_variable_count++;

        printf("Variable %s set to %s\n", words[1], words[2]);
        return true;
    }

    if (strcmp(words[0], "unset") == 0) {
        for (int i = 0; i < set_variable_count; i++) {
            if (strcmp(words[1], set_variables[i].name) == 0) {
                set_variables[i].name = NULL;
                set_variables[i].value = "";
                set_variable_count--;
            }
        }
        return true;
    }

    return false;
}