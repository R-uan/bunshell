#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int launch_process(char **args) {
  int pid = fork();
  if (pid == 0) {
    // child process
    execvp(args[0], args);
    printf("bunshell: `%s` command not found\n", args[0]);
    exit(1);
  } else if (pid > 0) {
    waitpid(pid, NULL, WUNTRACED);
    return 1;
  }

  return -1;
}

int main(int argc, char **argv) {
  char *line;
  char **args;
  int status;

  while (1) {
    printf("$ ");
    line = read_input();
    args = tokenize(line);
    status = launch_process(args);

    free(line);
    free(args);
  }
}
