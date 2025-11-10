#include "utils.h"
#include "bunshell.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void shell_insert_char(char *buffer, int *len, int *cursorPos, char c) {
  memmove(&buffer[*cursorPos + 1], &buffer[*cursorPos], *len - *cursorPos);
  buffer[*cursorPos] = c;
  (*len)++;

  write(STDOUT_FILENO, &buffer[*cursorPos], *len - *cursorPos);
  (*cursorPos)++;
  printf("\033[%dD", *len - *cursorPos); // Move left
  fflush(stdout);
}

void shell_backspace(char *buffer, int *index, int *cursor) {
  if (*cursor == 0 || *index == 0)
    return;

  if (*cursor != *index) {
    memmove(&buffer[*cursor - 1], &buffer[*cursor], *index - *cursor);
    (*cursor)--;
    (*index)--;

    int charsAfter = *index - *cursor;
    write(STDOUT_FILENO, "\033[D", 3);
    write(STDOUT_FILENO, &buffer[*cursor], charsAfter);
    write(STDOUT_FILENO, " ", 1);

    char move_seq[16];
    snprintf(move_seq, sizeof(move_seq), "\033[%dD", charsAfter + 1);
    write(STDOUT_FILENO, move_seq, strlen(move_seq));

    fflush(stdout);
  } else {
    (*index)--;
    (*cursor)--;
    write(STDOUT_FILENO, "\b \b", 3);
  }
}

void shell_delete(char *buffer, int *index, int *cursor) {
  if (*cursor >= *index)
    return;
  memmove(&buffer[*cursor], &buffer[*cursor + 1], *index - *cursor);
  (*index)--;

  int charsAfter = *index - *cursor;
  write(STDOUT_FILENO, &buffer[*cursor], *index - *cursor);
  write(STDOUT_FILENO, " ", 1);
  char move_seq[16];
  snprintf(move_seq, sizeof(move_seq), "\033[%dD", charsAfter + 1);
  write(STDOUT_FILENO, move_seq, strlen(move_seq));

  fflush(stdout);
}

char *shell_read_input(cmd_history *history) {
  int index = 0;
  int bufferSize = 1024;
  int cursorPosition = 0;
  char *buffer = malloc(bufferSize * sizeof(char));

  if (buffer == NULL) {
    perror("bunshell: malloc failed\n");
    exit(1);
  }

  char c;

  while (read(STDIN_FILENO, &c, 1) == 1) {
    // ENTER
    if (c == '\r' || c == '\n') {
      write(STDOUT_FILENO, "\r\n", 2);
      break;
    }

    if (c == 127 || c == 8) {
      shell_backspace(buffer, &index, &cursorPosition);
      continue;
    }

    if (c == 3) {
      free_history(history);
      free(buffer);
      exit(0);
    }

    if (c == 27) {
      char seq[3];
      if (read(STDIN_FILENO, &seq[0], 1) != 1)
        continue;
      if (read(STDIN_FILENO, &seq[1], 1) != 1)
        continue;

      if (seq[0] == '[') {
        // Command History
        if (seq[1] == 'A' || seq[1] == 'B') {
          char *command = NULL;
          // Checks if there's a history
          if (history->size > 0) {
            if (seq[1] == 'A') { // UP ARROW
              // If the cursor is the same as the size then the
              // current buffer is not in history and should be held.
              if (history->cursor == history->size) {
                buffer[index] = '\0';
                hold_command(history, buffer);
              }
              if (history->cursor > 0)
                history->cursor--;
              command = history->commands[history->cursor];
            } else if (seq[1] == 'B') { // DOWN ARROW
              if (history->cursor < history->size)
                history->cursor++;

              if (history->cursor == history->size) {
                command = history->holder;
              } else {
                command = history->commands[history->cursor];
              }
            }
          }

          if (command != NULL) {
            if (index != 0 && cursorPosition == index) {
              char seq[32];
              snprintf(seq, sizeof(seq), "\033[%dD\033[%dP", index, index);
              write(STDOUT_FILENO, seq, strlen(seq));
            }
            size_t size = strlen(command);
            write(STDOUT_FILENO, command, size);
            strcpy(buffer, command);
            index = size;
            cursorPosition = size;
          }
          continue;
        }
        switch (seq[1]) {
        case 'C':
          if (cursorPosition < index) {
            cursorPosition++;
            write(STDOUT_FILENO, "\033[C", 3);
          }
          break;
        case 'D':
          if (cursorPosition > 0) {
            cursorPosition--;
            write(STDOUT_FILENO, "\033[D", 3);
          }
          break;
        case '3':
          read(STDIN_FILENO, &seq[2], 1);
          if (seq[2] == '~') {
            shell_delete(buffer, &index, &cursorPosition);
          }
          break;
        }
      }
      continue;
    }

    if (iscntrl(c) && c != '\t') {
      continue;
    }

    if (index >= bufferSize - 1) {
      bufferSize *= 2;
      char *tempBuffer = realloc(buffer, bufferSize * sizeof(char));
      if (tempBuffer == NULL) {
        printf("bunshell: realloc failed\n");
        free(buffer);
        exit(1);
      }
      buffer = tempBuffer;
    }

    if (cursorPosition != index) {
      shell_insert_char(buffer, &index, &cursorPosition, c);
    } else {
      write(STDOUT_FILENO, &c, 1);
      buffer[index++] = c;
      cursorPosition++;
    }
  }

  buffer[index] = '\0';
  return buffer;
}

char **tokenize(char *input) {
  char *token;
  int index = 0;
  int bufSize = 10;
  char **tokens = malloc(bufSize * sizeof(char *));

  if (tokens == NULL) {
    fprintf(stderr, "bunshell: allocation failed\r\n");
    exit(1);
  }

  token = strtok(input, " \t\n");
  while (token != NULL) {
    tokens[index++] = token;

    if (index >= bufSize) {
      bufSize += 10;
      tokens = realloc(tokens, bufSize * sizeof(char *));
      if (tokens == NULL) {
        printf("bunshell: realloc failed\r\n");
        exit(1);
      }
    }

    token = strtok(NULL, " \t\n");
  }

  tokens[index] = NULL;
  return tokens;
}
