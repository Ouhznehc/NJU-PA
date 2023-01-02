#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[], char *envp[]);
extern char **environ;
void call_main(uintptr_t *args) {
  // int argc = *args;
  // char **argv = (char **)(args + 1);
  // while(*args++);
  // char **envp = (char **)(args + 1);
   uint32_t* argcpt=(uint32_t*)args;
  uint32_t st=(uint32_t)args;
  int argc = argcpt[0];
char** argv = (char**)(st+4);
  char** envp = (char**)(st+(argc+2)*4);
  environ = envp;
  exit(main(argc, argv, envp));
  assert(0);
}
