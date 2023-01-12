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


void switch_mstatus(int mode){
  if(mode == MSTATUS_SAVE){
    (cpu.mstatus & MIE) ? (cpu.mstatus |= MPIE) : (cpu.mstatus &= ~MPIE);
    cpu.mstatus &= ~MIE;
  }
  if(mode == MSTATUS_RESTORE){
    (cpu.mstatus & MPIE) ? (cpu.mstatus |= MIE) : (cpu.mstatus &= ~MIE); 
    cpu.mstatus |= MPIE;
  }
}


word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  #ifdef CONFIG_ETRACE
    color_green(" EXCEPTION TRACE : normal");
    color_green("pc = 0x%08x   \n", epc);
  #endif
  switch_mstatus(MSTATUS_SAVE);
  cpu.mepc = epc;
  cpu.mcause = NO;


  return cpu.mtvec;
}

word_t isa_query_intr() {
  if (cpu.intr && (cpu.mstatus & MIE)) {
    cpu.intr = false;
    return IRQ_TIMER;
  }
  return INTR_EMPTY;
}
