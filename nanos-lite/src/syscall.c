#include <common.h>
#include "syscall.h"
#include <proc.h>
#include <fs.h>
void yield();
void naive_uload(PCB *pcb, const char *filename);
void halt(int code);
size_t fs_write(int fd, const void *buf, size_t len);

void sys_yield (Context *c) {yield(); c->GPRx = 0;}

void sys_exit (Context *c) {halt(0);}

void sys_write (Context *c) {c->GPRx = fs_write(c->GPR2, (void *)c->GPR3, c->GPR4);}

void sys_read (Context *c) {c->GPRx = fs_read(c->GPR2, (void *)c->GPR3, c->GPR4);}

void sys_brk (Context *c) {c->GPRx = 0;}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_yield: sys_yield(c);  break;
    case SYS_exit:  sys_exit(c);   break;
    case SYS_write: sys_write(c);  break;
    case SYS_read:  sys_read(c);   break;
    case SYS_brk:   sys_brk(c);    break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}

// #include <common.h>
// #include "syscall.h"
// void do_syscall(Context *c) {
//   uintptr_t a[4];
//   a[0] = c->GPR1;

//   switch (a[0]) {
//     default: panic("Unhandled syscall ID = %d", a[0]);
//   }
// }

