#include <memory.h>

#define PPN_MASK 0xfffffc00u
#define PPN(x) (((uintptr_t)x & PPN_MASK) >> 10)
#define OFFSET(x) ((uintptr_t)x & 0x00000fffu)
#define m




static void *pf = NULL;

void* new_page(size_t nr_page) {
  //void *head = pf;
  pf += nr_page * PGSIZE; // malloc nr_page * 4KB
  return pf;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  assert(n % PGSIZE == 0);
  void *page = new_page(n / PGSIZE);
  memset(page, 0, n);
  return page;
}
#endif

void free_page(void *p) {
  panic("not implement yet");
}

/* The brk() system call handler. */
int mm_brk(uintptr_t brk) {
  return 0;
}

void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
