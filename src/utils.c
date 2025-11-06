#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

char **tokenize(char *input) {
  char *token;
  int index = 0;
  int bufSize = 10;
  char **tokens = malloc(bufSize * sizeof(char *));

  if (tokens == NULL) {
    fprintf(stderr, "bunshell: allocation failed\n");
    exit(1);
  }

  token = strtok(input, " \t\n");
  while (token != NULL) {
    tokens[index++] = token;

    if (index >= bufSize) {
      bufSize += 10;
      tokens = realloc(tokens, bufSize * sizeof(char *));
      if (tokens == NULL) {
        printf("bunshell: realloc failed\n");
        exit(1);
      }
    }

    token = strtok(NULL, " \t\n");
  }

  tokens[index] = NULL;
  return tokens;
}
