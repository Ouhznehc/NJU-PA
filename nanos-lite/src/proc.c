#include <proc.h>

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;

extern void naive_uload(PCB *pcb, const char *filename);
void context_kload(PCB *pcb, void (*entry)(void *), void *arg);
void context_uload(PCB *pcb, const char *filename);

void switch_boot_pcb() {
  current = &pcb_boot;
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%s' for the %dth time!", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  // switch_boot_pcb();

  // Log("Initializing processes...");

  // naive_uload(NULL, "/bin/hello");

  context_kload(&pcb[0], hello_fun, "zero");
  context_kload(&pcb[1], hello_fun, "one ");
  switch_boot_pcb();
}

Context* schedule(Context *prev) {
  // save the context pointer
current->cp = prev;

// always select pcb[0] as the new process
//current = &pcb[0];
current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);

// then return the new context
return current->cp;
}
