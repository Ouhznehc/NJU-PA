#include <common.h>
#include "syscall.h"
#include <proc.h>
#include <fs.h>
#include <sys/time.h>

void yield();
void naive_uload(PCB *pcb, const char *filename);
void halt(int code);
int mm_brk(uintptr_t brk);

size_t fs_write(int fd, const void *buf, size_t len);

void sys_yield (Context *c) {yield(); c->GPRx = 0;}

void sys_exit (Context *c) { 
  //naive_uload(NULL, "/bin/nterm");
  //halt(c->GPRx); c->GPRx = c->GPR2;
  c->GPRx = execve("/bin/nterm", NULL, NULL);
}

void sys_write (Context *c) {c->GPRx = fs_write(c->GPR2, (void *)c->GPR3, c->GPR4);}

void sys_read (Context *c) {c->GPRx = fs_read(c->GPR2, (void *)c->GPR3, c->GPR4);}

void sys_brk (Context *c) { c->GPRx = mm_brk((uintptr_t)c->GPR2);}// always success

void sys_open (Context *c) {c->GPRx = fs_open((char *)c->GPR2, c->GPR3, c->GPR4);}

void sys_close (Context *c) {c->GPRx = fs_close(c->GPR2);}

void sys_lseek (Context *c) {c->GPRx = fs_lseek(c->GPR2, c->GPR3, c->GPR4);}

void sys_gettimeofday(Context *c) {
  uint64_t us = io_read(AM_TIMER_UPTIME).us;
  ((struct timeval *)c->GPR2)->tv_sec  = us / 1000000;
  ((struct timeval *)c->GPR2)->tv_usec = us % 1000000;
  c->GPRx = 0;
}

void sys_execve (Context *c) {
  char *filename = (char *) c->GPR2;
  char **argv    = (char **)c->GPR3;
  char **envp    = (char **)c->GPR4;
  c->GPRx = execve(filename, argv, envp); 
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_yield:        sys_yield(c);        break;
    case SYS_exit:         sys_exit(c);         break;
    case SYS_write:        sys_write(c);        break;
    case SYS_read:         sys_read(c);         break;
    case SYS_brk:          sys_brk(c);          break;
    case SYS_open:         sys_open(c);         break;
    case SYS_close:        sys_close(c);        break;
    case SYS_lseek:        sys_lseek(c);        break;
    case SYS_gettimeofday: sys_gettimeofday(c); break; 
    case SYS_execve:       sys_execve(c);       break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

