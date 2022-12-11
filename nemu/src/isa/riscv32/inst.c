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

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write



//============================ ENMU =========================
enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_B, TYPE_R,
  TYPE_N, // none
};

enum {
    EVENT_NULL = 0,
    EVENT_YIELD, EVENT_SYSCALL, EVENT_PAGEFAULT, EVENT_ERROR,
    EVENT_IRQ_TIMER, EVENT_IRQ_IODEV,
  };

enum {
    MSTATUS = 0x300,
    MTVEC   = 0x305,
    MEPC    = 0x341,
    MCAUSE  = 0x342
  };
//==============================================================





static word_t csr_read(word_t csr){
  word_t value = 0;
  switch (csr) {
      case MSTATUS: value = cpu.mstatus;  break;
      case MTVEC:   value = cpu.mtvec;    break;
      case MEPC:    value = cpu.mepc;     break;
      case MCAUSE:  value = cpu.mcause;   break;
    } 
  return value;
}

static void csr_write(word_t csr, word_t value){
      switch (csr) {
      case MSTATUS: cpu.mstatus = value; break;
      case MTVEC:   cpu.mtvec = value;   break;
      case MEPC:    cpu.mepc = value;    break;
      case MCAUSE:  cpu.mcause = value;  break;
    } 
}

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
//! begin my code
#define immJ() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 20) |\
                                    (BITS(i, 30, 21)) << 1  |\
                                    (BITS(i, 20, 20)) << 11 |\
                                    (BITS(i, 19, 12)) << 12 ; } while(0)

#define immB() do { *imm = (SEXT(BITS(i, 31, 31), 1) << 12) |\
                                    (BITS(i, 30, 25)) << 5  |\
                                    (BITS(i, 11, 8)) << 1   |\
                                    (BITS(i, 7, 7)) << 11   ; } while(0)
//! end my code

static void decode_operand(Decode *s, int *dest, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rd  = BITS(i, 11, 7);
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *dest = rd;
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
//! begin my code    
    case TYPE_J:                   immJ(); break;
    case TYPE_B: src1R(); src2R(); immB(); break;
    case TYPE_R: src1R(); src2R();         break;
//! end my code
  }
}

static int decode_exec(Decode *s) {
  int dest = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &dest, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(dest) = imm);
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(dest) = Mr(src1 + imm, 4));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw     , S, Mw(src1 + imm, 4, src2));
  //! begin my code
  // type_R
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(dest) = src1 < src2);
  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(dest) = src1 + src2);
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(dest) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(dest) = src1 | src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(dest) = src1 - src2);
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(dest) = (int)src1 / (int)src2);
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(dest) = src1 * src2);
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(dest) = (int)src1 % (int)src2);
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(dest) = (int)src1 < (int)src2);
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(dest) = src1 << src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(dest) = src1 & src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, R(dest) = (SEXT(src1, 32) * SEXT(src2, 32)) >> 32);
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(dest) = (int)src1 >> (src2 & 0x1f));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(dest) = src1 >> (src2 & 0x1f));
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(dest) = src1 / src2);
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(dest) = src1 % src2);
  INSTPAT("0000001 ????? ????? 011 ????? 01100 11", mulhu  , R, R(dest) = ((uint64_t)src1 * (uint64_t)src2) >> 32);
  INSTPAT("0011000 00010 00000 000 00000 11100 11", mret   , R, s->dnpc = csr_read(MEPC));



  // type_U
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(dest) = s->pc + imm);


  // type_S
  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));


  // type_J
  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal    , J, s->dnpc = s->pc + imm; R(dest) = s->snpc);


  // type_B
  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, if(src1 == src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, if(src1 != src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, if((int)src1 < (int)src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, if((int)src1 >= (int)src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, if(src1 < src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, if(src1 >= src2) s->dnpc = s->pc + imm);


  // type_I
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(dest) = imm + src1);
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, s->dnpc = imm + src1; R(dest) = s->snpc);
  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(dest) = src1 < imm);
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(dest) = src1 & imm);
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(dest) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(dest) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(dest) = Mr(src1 + imm, 2));
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(dest) = src1 ^ imm);
  INSTPAT("010000? ????? ????? 101 ????? 00100 11", srai   , I, if(((imm >> 5) & 1) == 0) R(dest) = (int)src1 >> (imm & 0x1f));
  INSTPAT("0000000 ????? ????? 001 ????? 00100 11", slli   , I, if(((imm >> 5) & 1) == 0) R(dest) = src1 << (imm & 0x1f));
  INSTPAT("0000000 ????? ????? 101 ????? 00100 11", srli   , I, if(((imm >> 5) & 1) == 0) R(dest) = src1 >> (imm & 0x1f));
  INSTPAT("??????? ????? ????? 000 ????? 00000 11", lb     , I, R(dest) = SEXT((Mr(src1 + imm, 1)),8));
  INSTPAT("??????? ????? ????? 110 ????? 00100 11", ori    , I, R(dest) = src1 | imm);
  INSTPAT("??????? ????? ????? 010 ????? 00100 11", slti   , I, R(dest) = ((int)src1 < (int)imm));
  INSTPAT("0000000 00000 00000 000 00000 11100 11", ecall  , I, s->dnpc = isa_raise_intr(EVENT_YIELD, s->pc));
  INSTPAT("??????? ????? ????? 010 ????? 11100 11", csrrs  , I, R(dest) = csr_read(imm); csr_write(imm, csr_read(imm) | src1););
  INSTPAT("??????? ????? ????? 001 ????? 11100 11", csrrw  , I, R(dest) = csr_read(imm); csr_write(imm, src1););





  //! end my code
  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}


