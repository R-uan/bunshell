#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void insert_char(char *buffer, int *len, int *cursorPos, char c) {
  memmove(&buffer[*cursorPos + 1], &buffer[*cursorPos], *len - *cursorPos);
  buffer[*cursorPos] = c;
  (*len)++;

  write(STDOUT_FILENO, &buffer[*cursorPos], *len - *cursorPos);
  (*cursorPos)++;
  printf("\033[%dD", *len - *cursorPos); // Move left
  fflush(stdout);
}

void delete_char(char *buffer, int *index, int *cursor) {
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

char *read_input_raw() {
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
    if (c == '\r' || c == '\n') {
      write(STDOUT_FILENO, "\r\n", 2);
      break;
    }

    if (c == 127 || c == 8) {
      delete_char(buffer, &index, &cursorPosition);
      continue;
    }

    if (c == 3) {
      exit(1);
    }

    if (c == 27) {
      char seq[2];
      if (read(STDIN_FILENO, &seq[0], 1) != 1)
        continue;
      if (read(STDIN_FILENO, &seq[1], 1) != 1)
        continue;

      if (seq[0] == '[') {
        if (seq[1] == 'C' && cursorPosition < index) {
          cursorPosition++;
          write(STDOUT_FILENO, "\033[C", 3);
        } else if (seq[1] == 'D' && cursorPosition > 0) {
          cursorPosition--;
          write(STDOUT_FILENO, "\033[D", 3);
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
      insert_char(buffer, &index, &cursorPosition, c);
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
