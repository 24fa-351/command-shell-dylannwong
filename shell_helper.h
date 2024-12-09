#ifndef SHELL_HELPERS_H
#define SHELL_HELPERS_H

#include <stdbool.h>
#include <string.h>

struct variable {
    char *name;
    char *value;
};

extern size_t set_variable_count;
extern struct variable set_variables[1000];
extern char working_directory[1000];

void add_character_to_string(char *str, char c);
void split(char *cmd, char *words[], char delimiter);
bool find_absolute_path(char *cmd, char *absolute_path);
bool is_command_implemented(char **words, int words_length);
int command_helper(char **words, char *input, int words_length);

#endif  // SHELL_HELPERS_H