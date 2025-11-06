#include "utils.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

char *read_input() {
  char c;
  int index = 0;
  int bufferSize = 1024;
  char *buffer = malloc(bufferSize * sizeof(char));

  while ((c = getchar()) != EOF && c != '\n') {
    if (index >= bufferSize) {
      bufferSize *= 2;
      char *tempBuffer = realloc(buffer, bufferSize * sizeof(char));
      if (tempBuffer == NULL) {
        printf("bunshell: realloc failed\n");
        free(buffer);
        exit(1);
      }
      buffer = tempBuffer;
    }
    buffer[index++] = c;
  }

  buffer[index] = '\0';
  return buffer;
}

char *read_input_raw() {
  int index = 0;
  int bufferSize = 1024;
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
      if (index > 0) {
        index--;
        write(STDOUT_FILENO, "\b \b", 3);
      }
      continue;
    }

    if (c == 3) {
      exit(1);
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

    write(STDOUT_FILENO, &c, 1);
    buffer[index++] = c;
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
