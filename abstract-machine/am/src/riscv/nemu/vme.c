#include <am.h>
#include <nemu.h>
#include <klib.h>

static AddrSpace kas = {};
static void* (*pgalloc_usr)(int) = NULL;
static void (*pgfree_usr)(void*) = NULL;
static int vme_enable = 0;

static Area segments[] = {      // Kernel memory mappings
  NEMU_PADDR_SPACE
};

#define USER_SPACE RANGE(0x40000000, 0x80000000)

static inline void set_satp(void *pdir) {
  uintptr_t mode = 1ul << (__riscv_xlen - 1);
  asm volatile("csrw satp, %0" : : "r"(mode | ((uintptr_t)pdir >> 12)));
}

static inline uintptr_t get_satp() {
  uintptr_t satp;
  asm volatile("csrr %0, satp" : "=r"(satp));
  return satp << 12;
}

bool vme_init(void* (*pgalloc_f)(int), void (*pgfree_f)(void*)) {
  pgalloc_usr = pgalloc_f;
  pgfree_usr = pgfree_f;

  kas.ptr = pgalloc_f(PGSIZE);

  int i;
  for (i = 0; i < LENGTH(segments); i ++) {
    void *va = segments[i].start;
    for (; va < segments[i].end; va += PGSIZE) {
      map(&kas, va, va, 0);
    }
  }

  set_satp(kas.ptr);
  vme_enable = 1;

  return true;
}

void protect(AddrSpace *as) {
  PTE *updir = (PTE*)(pgalloc_usr(PGSIZE));
  as->ptr = updir;
  as->area = USER_SPACE;
  as->pgsize = PGSIZE;
  // map kernel space
  memcpy(updir, kas.ptr, PGSIZE);
}

void unprotect(AddrSpace *as) {
}

void __am_get_cur_as(Context *c) {
  c->pdir = (vme_enable ? (void *)get_satp() : NULL);
}

void __am_switch(Context *c) {
  if (vme_enable && c->pdir != NULL) {
    set_satp(c->pdir);
  }
}

/*
-------------------------------------------------------------------
|31        20|19        10|9     8| 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |
|   PPN[1]   |   PPN[0]   |  RSW  | D | A | G | U | X | W | R | V |
|    12      |    10      |   2   | 1 | 1 | 1 | 1 | 1 | 1 | 1 | 1 |
-------------------------------------------------------------------

              Riscv32 Sv32 PTE(page-table entry)

*/
#define BITMASK(bits) ((1u << (bits)) - 1)
#define BITS(x, hi, lo) (((uint32_t)(x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
#define VPN_0(x)  BITS(x, 21, 12)
#define VPN_1(x)  BITS(x, 31, 22)
#define PPN(x)    BITS(x, 31, 10)

void map(AddrSpace *as, void *va, void *pa, int prot) {
  uint32_t *pde = as->ptr + VPN_1(va) * 4;
  if((*pde & PTE_V) == 0) {
    uint32_t *new_page = pgalloc_usr(PGSIZE);
    *pde = (uint32_t)new_page >> 2;
    *pde |= PTE_V;
  }
  uint32_t *pte = (uint32_t *)(PPN(*pde) * 4096 + VPN_0(va) * 4);
  *pte |= (uint32_t)pa >> 2;
  *pte |= PTE_V;
}

bool check_map(AddrSpace *as, void *va){
  uint32_t *pde = as->ptr + VPN_1(va) * 4;
  if((*pde & PTE_V) == 0) return false;
  uint32_t *pte = (uint32_t *)(PPN(*pde) * 4096 + VPN_0(va) * 4);
  if((*pte & PTE_V) == 0) return false;
  return true;
}

Context *ucontext(AddrSpace *as, Area kstack, void *entry) {
  Context *context = kstack.end - sizeof(Context);
  context->mstatus = 0x1800 | 0x0080 ;
  context->mepc    = (uintptr_t)entry;
  context->pdir    = as->ptr;
  context->np      = USER;
  return context;
}
