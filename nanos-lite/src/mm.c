#include <memory.h>

#define BITMASK(bits) ((1ull << (bits)) - 1)
#define BITS(x, hi, lo) (((uintptr_t)(x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog

/*
-------------------------------------------------------------------
|31        20|19        10|9     8| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|   PPN[1]   |   PPN[0]   |  RSW  | D | A | G | U | X | W | R | V |
|    12      |    10      |   2   | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
-------------------------------------------------------------------

              Riscv32 Sv32 PTE(page-table entry)

*/

#define VPN_0(x)  BITS(x, 21, 12)
#define VPN_1(x)  BITS(x, 31, 22)
#define OFFSET(x) BITS(x, 11, 0)
#define PPN(x)    BITS(x, 31, 10)



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
