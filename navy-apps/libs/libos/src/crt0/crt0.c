#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  printf("shit !! \n");
  int argc = *args;
  char **argv = (char **)(args + 1);
  while(*args++);
  char **envp = (char **)(args + 1);
  environ = envp;
  printf("argv is '%s'\n", *argv);
  exit(main(argc, argv, envp));
  assert(0);
}
