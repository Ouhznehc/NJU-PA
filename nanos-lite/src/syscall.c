#include <common.h>
#include "syscall.h"
#include <proc.h>

void yield();
void naive_uload(PCB *pcb, const char *filename);
void halt(int code);
size_t fs_write(int fd, const void *buf, size_t len);

void sys_yield(Context *c){
  yield();
  c->GPRx = 0;
}

void sys_exit(Context *c){
  halt(0);
}

size_t s_write(const void *buf, size_t len){
  char *temp = (char *)buf;
  for(int i = 0; i < len; i++){
    putch(temp[i]);
  }
  return len;
}

void sys_write(Context *c){
  if(c->GPR2 == 1 || c->GPR2 == 2){
    c->GPRx = s_write((void *)c->GPR3, c->GPR4);
  }else{
    c->GPRx = fs_write(c->GPR2, (void *)c->GPR3, c->GPR4);
  }
}

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  switch (a[0]) {
    case SYS_yield:
      sys_yield(c);
      break;
    case SYS_exit:
      sys_exit(c);
      break;
    case SYS_write:
      sys_write(c);
      break;
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

