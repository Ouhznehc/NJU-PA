#include <proc.h>
#include <fs.h>

#define MAX_NR_PROC 4
#define pcb_select (current == &pcb[0] ? &pcb[program_index] : &pcb[0]);
int program_index = 1;
void switch_program_index(int index){
  program_index = index;
}
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
    printf("Hello World from Nanos-lite with arg '%s' for the %dth time!\n", (uintptr_t)arg, j);
    j ++;
    yield();
  }
}

void init_proc() {
  Log("Initializing processes...");
  char *argv1[] = {"/bin/nterm", NULL};
  char *argv2[] = {"/bin/pal", "--skip", "--splash", NULL};
  char *argv3[] = {"/bin/nslider", NULL};
  context_kload(&pcb[0], hello_fun, "fuck");
  //context_uload(&pcb[0], "/bin/hello",   NULL,  NULL);
  context_uload(&pcb[1], "/bin/nterm",   argv1, NULL);
  context_uload(&pcb[2], "/bin/pal",     argv2, NULL);
  context_uload(&pcb[3], "/bin/nslider", argv3, NULL);
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
  if(fs_open(filename, 0, 0) == -2) return -2;
  context_uload(current, filename, argv, envp);
  switch_boot_pcb();
  yield();
  return 0;
}
