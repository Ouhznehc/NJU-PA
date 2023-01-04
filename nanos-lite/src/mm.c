#include <memory.h>

#define page_size 4096

static void *pf = NULL;

void* new_page(size_t nr_page) {
  //void *head = pf;
  pf += nr_page * page_size; // malloc nr_page * 4KB
  return pf;
}

#ifdef HAS_VME
static void* pg_alloc(int n) {
  assert(n % page_size == 0);
  void *page = new_page(n / page_size);
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
