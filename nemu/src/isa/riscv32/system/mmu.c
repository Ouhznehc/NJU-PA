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

#define PTE_V 0x01
#define MYBITMASK(bits) ((1u << (bits)) - 1)
#define MYBITS(x, hi, lo) (((vaddr_t)(x) >> (lo)) & MYBITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
#define VPN_0(x)  MYBITS(x, 21, 12)
#define VPN_1(x)  MYBITS(x, 31, 22)
#define OFFSET(x) MYBITS(x, 11, 0)
#define PPN(x)    MYBITS(x, 31, 10)
#define PPN_MASK  (0xfffffc00u)
typedef uintptr_t PTE;

paddr_t isa_mmu_translate(vaddr_t vaddr, int len, int type) {
  paddr_t pte_addr = (cpu.satp << 12) + VPN_1(vaddr) * 4;
  PTE pte = paddr_read(pte_addr, 4);
  assert(pte & PTE_V);

  paddr_t leaf_pte_addr = PPN(pte) * 4096 + VPN_0(vaddr) * 4;
  PTE leaf_pte = paddr_read(leaf_pte_addr, 4);
  //assert((leaf_pte & PTE_V) == 1);

  paddr_t pa = PPN(leaf_pte) * 4096 + OFFSET(vaddr);
  assert(pa == vaddr);
  return pa;
}
