#ifndef BUILTIN_H
#define BUILTIN_H

int shell_cd(char **args);
int shell_exit(char **args);
int shell_help(char **args);

typedef struct {
  char *name;
  int (*func)(char **args);
} builtin_cmd;

int num_builtins();
int execute_builtin(char **args);

#endif // !BUILTIN_H
