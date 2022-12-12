#include <common.h>
#include <fs.h>
#include "syscall.h"

void sys_yield(Context *c) { yield(); c->GPRx = 0; }

void sys_exit (Context *c) { halt(c->GPRx); }

size_t s_write(const void *buf, size_t offset, size_t len) {
  size_t res = len;
  char *now = (char *) buf;
  while(*now++ && len--) {putch(*now);}
  return res;
}

void sys_write(Context *c) {
  //  c->GPRx = fs_write(c->GPR2, (void*)c->GPR3, c->GPR4);
    c->GPRx = s_write((void*)c->GPR2, c->GPR3, c->GPR4);
  }

void sys_read (Context *c) { c->GPRx = fs_read(c->GPR2, (void*)c->GPR3, c->GPR4); }



void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;

  switch (a[0]) {
    case SYS_yield: sys_yield(c); break;
    case SYS_exit : sys_exit(c);  break;
    case SYS_write: sys_write(c); break;
    case SYS_read : sys_read(c);  break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
