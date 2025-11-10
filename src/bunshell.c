#include "bunshell.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void init_history(cmd_history *cmdh) {
  cmdh->size = 0;
  cmdh->capacity = 10;
  cmdh->commands = malloc(cmdh->capacity * sizeof(char *));
}

void add_command(cmd_history *cmdh, const char *command) {
  if (cmdh->size >= cmdh->capacity) {
    cmdh->capacity += 10;
    cmdh->commands = realloc(cmdh->commands, cmdh->capacity * sizeof(char *));
  }
  cmdh->commands[cmdh->size++] = strdup(command);
  cmdh->cursor = cmdh->size;
}

void hold_command(cmd_history *cmdh, const char *command) {
  free(cmdh->holder);
  cmdh->holder = strdup(command);
}

void free_history(cmd_history *cmdh) {
  for (size_t i = 0; i < cmdh->size; i++) {
    free(cmdh->commands[i]);
  }
  free(cmdh->commands);
}

Bunshell_t init_bunshell() {
  cmd_history history;
  init_history(&history);
  Bunshell_t bunshell = {history};
  return bunshell;
}
