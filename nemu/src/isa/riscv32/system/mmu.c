/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <memory/paddr.h>
#include <memory/vaddr.h>

// #define PTE_V 0x01
// #define PTE_R 0x02
// #define PTE_W 0x04
// #define PTE_X 0x08
// #define PTE_U 0x10
// #define PTE_A 0x40
// #define PTE_D 0x80
// #define MYBITMASK(bits) ((1u << (bits)) - 1)
// #define MYBITS(x, hi, lo) (((vaddr_t)(x) >> (lo)) & MYBITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
// #define VPN_0(x)  MYBITS(x, 21, 12)
// #define VPN_1(x)  MYBITS(x, 31, 22)
// #define OFFSET(x) MYBITS(x, 11, 0)
// #define PPN(x)    MYBITS(x, 31, 10)
// #define pde_addr cpu.satp * 4096 + VPN_1(vaddr) * 4
// #define pte_addr PPN(pde) * 4096 + VPN_0(vaddr) * 4

// paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
//   uint32_t pde = paddr_read(pde_addr, 4);
//   if((pde & PTE_V) == 0) panic("page directory entry: 0x%08x is invalid, vaddr is 0x%08x", pde, vaddr);

//   uint32_t pte = paddr_read(pte_addr, 4);
//   if((pte & PTE_V) == 0) panic("page table     entry: 0x%08x is invalid, vaddr is 0x%08x", pte, vaddr); 

//   uint32_t paddr = PPN(pte) * 4096 + OFFSET(vaddr);
//   return paddr;
// }

#define exp2(x)         (1u << (x))
#define PGW             (12)
#define PGSIZE          exp2(PGW)
#define PGMASK          BITMASK(PGW)
#define PGPN(addr)      ((addr) >> (PGW))
#define PGOFF(addr)     ((addr) & (PGMASK))
#define PGSTART(pn)     ((paddr_t) ((pn) << (PGW)))

#define T0SZ            (10)
#define T1SZ            (10)

#define VPN_1(vaddr)  BITS(vaddr, (PGW + T0SZ + T1SZ - 1), (PGW + T0SZ))
#define VPN_0(vaddr)  BITS(vaddr, (PGW + T0SZ - 1), PGW)

typedef uint32_t PTE;

#define PTE_V 0x01
#define PTE_R 0x02
#define PTE_W 0x04
#define PTE_X 0x08
#define PTE_U 0x10
#define PTE_A 0x40
#define PTE_D 0x80

#define PTE_PPN(ppn)    ((uint32_t) (ppn) << 10)
#define PPN(pte)        ((uint32_t) (pte) >> 10)

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  //check state
  if (isa_mmu_check(vaddr, len, type) == MMU_DIRECT) return vaddr; 
  // Log("Paging Opened!");

  //L1 page table
  paddr_t p_target_pde = PGSTART(cpu.satp);
  p_target_pde += sizeof(PTE) * VPN_1(vaddr);

  word_t target_pde = paddr_read(p_target_pde, sizeof(PTE));

  //L2 page table
  paddr_t p_target_pte;
  if (target_pde & PTE_V)
    p_target_pte = PGSTART(PPN(target_pde));
  else {panic("L1 miss: 0x%08x", vaddr);}

  p_target_pte += sizeof(PTE) * VPN_0(vaddr);

  word_t target_pte = paddr_read(p_target_pte, sizeof(PTE));

  //Return physical address
  if (target_pte & PTE_V)
    return PGSTART(PPN(target_pte)) | PGOFF(vaddr);
  else {panic("L2 miss: 0x%08x", vaddr);}
}
