#include <memory.h>
#include <proc.h>

static void *pf = NULL;

void* new_page(size_t nr_page) {
  void *head = pf;
  pf += nr_page * PGSIZE; // malloc nr_page * 4KB
  return head;
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
  if(brk <= current->max_brk) return 0;
  //printf("===============\n");
  uint32_t max_nr_page = (current->max_brk - 1) / PGSIZE;
  uint32_t now_nr_page = (brk - 1) / PGSIZE;
  int nr_page = now_nr_page - max_nr_page;
  void *page = new_page(nr_page);
  //printf("begin malloc: max_brk = %08p, brk = %08p, pages = %d\n", current->max_brk, brk, nr_page);
  void *vaddr = (void *)current->max_brk;
  for(int i = 0; i < nr_page; i++) map(&current->as, vaddr + i * PGSIZE, page + i * PGSIZE, MMAP_READ | MMAP_WRITE);
  current->max_brk = ROUNDUP(brk, PGSIZE);
  //printf("end   malloc: max_brk = %08p\n", current->max_brk);
  //printf("===============\n");
  return 0;
}

extern PCB *current;
void init_mm() {
  pf = (void *)ROUNDUP(heap.start, PGSIZE);
  Log("free physical pages starting from %p", pf);

#ifdef HAS_VME
  vme_init(pg_alloc, free_page);
#endif
}
