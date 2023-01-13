#include <common.h>
#include <proc.h>

void do_syscall(Context *c);

static Context* do_event(Event e, Context* c) {
  switch (e.event) {
    case EVENT_YIELD:
      //color_yellow("=== event  yield  triggered ===\n"); 
      return schedule(c);
      break;
    case EVENT_SYSCALL:
      //color_yellow("=== event syscall triggered ===\n");
      do_syscall(c);
      break;
    case EVENT_IRQ_TIMER:
      //color_yellow("=== event timer interupt triggered ===\n");
      return schedule(c);
      break;
    default: panic("Unhandled event ID = %d", e.event);
  }

  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
