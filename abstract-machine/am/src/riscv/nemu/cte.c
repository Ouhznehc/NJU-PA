#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>
void __am_get_cur_as(Context *c);
void __am_switch(Context *c);
static Context* (*user_handler)(Event, Context*) = NULL;
#define EVENT 0
Context* __am_irq_handle(Context *c) {
   __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case EVENT:
        if(!~c->GPR1) ev.event = EVENT_YIELD;
        else ev.event = EVENT_SYSCALL;
        break;
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
    c->mepc += 4;
  }
  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  Context *context = kstack.end - sizeof(Context);
  context->mstatus = 0x1800;
  context->mepc    = (uintptr_t)entry;
  context->GPR2    = (uintptr_t)arg;
  return context;
}

void yield() {
  asm volatile("li a7, -1; ecall");
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
