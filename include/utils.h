#ifndef UTILS_H
#define UTILS_H

#include "bunshell.h"
char **tokenize(char *input);
char *shell_read_input(cmd_history *history);

#endif // !UTILS_H
