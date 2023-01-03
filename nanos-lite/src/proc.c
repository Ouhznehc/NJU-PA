#include <proc.h>

#define MAX_NR_PROC 4
#define pcb_select (current == &pcb[0] ? &pcb[1] : &pcb[0])

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

extern void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
void context_uload(PCB *pcb, const char *filename, char *const argv[], char *const envp[]);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    //Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  Log("Initializing processes...");


   context_kload(&pcb[0], hello_fun, "zero");
  //char *argv[] = {"--skip", "--splash", NULL};
  //context_uload(&pcb[1], "/bin/pal", argv, NULL);
  //char *argv[] = {"/bin/exec-test", "0", NULL};
  context_uload(&pcb[1], "/bin/bmp-test", NULL, NULL);
  switch_boot_pcb();
}

Context* schedule(Context *prev) {
  // save the context pointer
  current->cp = prev;

  current = pcb_select;

  // then return the new context
  return current->cp;
}

int execve(const char *filename, char *const argv[], char *const envp[]){
  context_uload(&pcb[1], filename, argv, envp);
  switch_boot_pcb();
  yield();
  return 0;
}
