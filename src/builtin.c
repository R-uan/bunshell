#include "builtin.h"
#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int shell_cd(char **args) {
  if (args[1] == NULL) {
    fprintf(stderr, "cd: expected argument\n");
    return 1;
  }

  int cd = chdir(args[1]);
  if (cd != 0) {
    switch (cd) {
    case EACCES:
      fprintf(stderr, "cd: permission denied: %s\n", args[1]);
      break;
    case ENOENT:
      fprintf(stderr, "cd: no such file or directory: %s\n", args[1]);
      break;
    case ENOTDIR:
      fprintf(stderr, "cd: not a directory: %s\n", args[1]);
      break;
    case ENAMETOOLONG:
      fprintf(stderr, "cd: path too long: %s\n", args[1]);
      break;
    default:
      perror("cd"); // Generic error message
      break;
    }
  }
  return cd;
}

int shell_exit(char **_) {
  exit(0);
  return 0;
}
int shell_help(char **args) { return 0; }

builtin_cmd builtins[] = {
    {"cd", shell_cd},     //
    {"exit", shell_exit}, //
    {"help", shell_help}, //
    {NULL, NULL}          //
};

int num_builtins() { return sizeof(builtins) / sizeof(builtin_cmd) - 1; }

int execute_builtin(char **args) {
  if (args[0] == NULL) {
    return 1;
  }

  for (int i = 0; builtins[i].name != NULL; i++) {
    if (strcmp(args[0], builtins[i].name) == 0) {
      return builtins[i].func(args);
    }
  }

  return -1;
}
