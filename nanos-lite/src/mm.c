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
  // if(brk < current->max_brk) return 0;
  // uint32_t max_nr_page = ROUNDUP(current->max_brk, PGSIZE) / PGSIZE - 1;
  // uint32_t now_nr_page = ROUNDUP(brk,              PGSIZE) / PGSIZE - 1;
  // int nr_page = now_nr_page - max_nr_page;
  // void *page = new_page(nr_page);
  // void *vaddr = (void *)ROUNDUP(current->max_brk, PGSIZE);
  // for(int i = 0; i < nr_page; i++) map(&current->as, vaddr + i * PGSIZE, page + i * PGSIZE, MMAP_READ | MMAP_WRITE);
  // return 0;
  uintptr_t max_page_end = current->max_brk; 
  uintptr_t max_page_pn = (max_page_end >> 12) - 1;
  uintptr_t brk_pn = brk >> 12;//12
  if (brk >= max_page_end){
    void *allocted_page =  new_page(brk_pn - max_page_pn + 1);
    for (int i = 0; i < brk_pn - max_page_pn + 1; ++i){
      map(&current->as, (void *)(max_page_end + i * 0xfff),
       (void *)(allocted_page + i * 0xfff), 1);
    }

    current->max_brk = (brk_pn + 1) << 12;
    assert(current->max_brk > brk);
}
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
