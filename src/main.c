#include "builtin.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termios.h>
#include <unistd.h>

struct termios orig_termios;

void disable_raw_mode() { tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios); }

void enable_raw_mode() {
  // Get current terminal attributes
  tcgetattr(STDIN_FILENO, &orig_termios);

  // Register cleanup function to restore terminal on exit
  atexit(disable_raw_mode);

  // Copy original settings to modify
  struct termios raw = orig_termios;

  /* INPUT FLAGS (c_iflag) - Control input processing */

  // IXON: Disable Ctrl+S (stop transmission) and Ctrl+Q (resume)
  // By default, Ctrl+S pauses output, Ctrl+Q resumes
  raw.c_iflag &= ~(IXON);

  // ICRNL: Disable translation of carriage return (CR/13) to newline (NL/10)
  // Allows us to read Ctrl+M as 13 instead of 10
  raw.c_iflag &= ~(ICRNL);

  // BRKINT: Disable break condition causing SIGINT
  // INPCK: Disable parity checking
  // ISTRIP: Disable stripping 8th bit of input bytes
  // These are often already off, but we disable for "true" raw mode
  raw.c_iflag &= ~(BRKINT | INPCK | ISTRIP);

  /* OUTPUT FLAGS (c_oflag) - Control output processing */

  // OPOST: Disable output processing (like \n to \r\n translation)
  // Without this, newlines won't move cursor to left edge
  raw.c_oflag &= ~(OPOST);

  /* CONTROL FLAGS (c_cflag) - Control hardware settings */

  // CS8: Set character size to 8 bits per byte
  // Not strictly raw mode, but ensures we can read full bytes
  raw.c_cflag |= (CS8);

  /* LOCAL FLAGS (c_lflag) - Control terminal local processing */

  // ECHO: Disable echoing input characters
  // You won't see what you type
  raw.c_lflag &= ~(ECHO | ICANON);

  // ICANON: Disable canonical mode (line-by-line input)
  // Read input byte-by-byte instead of line-by-line

  // ISIG: Disable signal generation from Ctrl+C, Ctrl+Z, etc.
  // These become regular input characters instead
  raw.c_lflag &= ~(ISIG);

  // IEXTEN: Disable Ctrl+V (literal next) and other implementation-defined
  // input processing
  raw.c_lflag &= ~(IEXTEN);

  /* CONTROL CHARACTERS (c_cc) - Set read() behavior */

  // VMIN: Minimum number of bytes read() must return
  // Setting to 1 means read() returns as soon as 1 byte is available
  raw.c_cc[VMIN] = 1;

  // VTIME: Timeout in deciseconds (tenths of a second)
  // Setting to 0 means no timeout - read() waits indefinitely
  raw.c_cc[VTIME] = 0;

  // Apply the modified attributes
  // TCSAFLUSH: Apply changes after flushing input/output
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

  int status;
  char *line;
  char **args;

  while (1) {
    write(STDOUT_FILENO, "$ ", 3);

    line = read_input_raw();
    args = tokenize(line);
    fflush(stdout);

    if ((status = execute_builtin(args)) == -1) {
      status = launch_process(args);
    }

    free(line);
    free(args);
  }
}
