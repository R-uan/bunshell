#include "builtin.h"
#include "bunshell.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disable_raw_mode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enable_raw_mode() {  tcgetattr(STDIN_FILENO, &orig_termios);
  atexit(disable_raw_mode);
  
  struct termios raw = orig_termios;

  raw.c_iflag &= ~(IXON);
  raw.c_iflag &= ~(ICRNL);
  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP);

  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);

  raw.c_lflag &= ~(ECHO | ICANON);
  raw.c_lflag &= ~(ISIG);
  raw.c_lflag &= ~(IEXTEN);

  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;

  tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

int launch_process(char **args) {
  int pid = fork();
  if (pid == 0) {
    // child process
    disable_raw_mode();
    execvp(args[0], args);
    printf("bunshell: `%s` command not found\r\n", args[0]);

    _exit(1);
  } else if (pid > 0) {
    waitpid(pid, NULL, WUNTRACED);
    enable_raw_mode();
    return 1;
  }

  return -1;
}

int main(int argc, char **argv) {
  enable_raw_mode();
  Bunshell_t bunshell = init_bunshell();

  int status;
  char *line;
  char **args;

  while (1) {
    write(STDOUT_FILENO, "$ ", 3);

    line = shell_read_input(&bunshell.history);
    args = tokenize(line);

    add_command(&bunshell.history, line);

    fflush(stdout);

    if ((status = execute_builtin(args)) == -1) {
      status = launch_process(args);
    }

    free(line);
    free(args);
  }
}
