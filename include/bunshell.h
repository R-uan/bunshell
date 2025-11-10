#ifndef BUNSHELL_H
#define BUNSHELL_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  size_t size;
  char *holder;
  size_t cursor;
  size_t capacity;
  char **commands;
} cmd_history;

typedef struct {
  cmd_history history;
} Bunshell_t;

Bunshell_t init_bunshell();
void init_history(cmd_history *cmdh);
void free_history(cmd_history *cmdh);
void add_command(cmd_history *cmdh, const char *command);
void hold_command(cmd_history *cmdh, const char *command);

#endif // !BUNSHELL_H
