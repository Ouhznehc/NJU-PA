#define concat_temp(x, y) x ## y
#define concat(x, y) concat_temp(x, y)
#define MAP(c, f) c(f)

#if __riscv_xlen == 32
#define LOAD  lw
#define STORE sw
#define XLEN  4
#else
#define LOAD  ld
#define STORE sd
#define XLEN  8
#endif

#define REGS1(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9)

#define REGS2(f) \
f(10) f(11) f(12) f(13) f(14) f(15) f(16) f(17) f(18) f(19) \
f(20) f(21) f(22) f(23) f(24) f(25) f(26) f(27) f(28) f(29) \
f(30) f(31)

#define PUSH(n) STORE concat(x, n), (n * XLEN)(sp);
#define POP(n)  LOAD  concat(x, n), (n * XLEN)(sp);

#define CONTEXT_SIZE  ((32 + 3 + 1 + 1) * XLEN)
#define OFFSET_SP     ( 2 * XLEN)
#define OFFSET_CAUSE  (32 * XLEN)
#define OFFSET_STATUS (33 * XLEN)
#define OFFSET_EPC    (34 * XLEN)
#define OFFSET_NP     (35 * XLEN)

#define KERNEL    0
#define USER      3

.align 3
.globl __am_asm_trap
__am_asm_trap:
  csrrw sp, mscratch, sp
  bnez sp, a1
  csrr sp, mscratch
  addi sp, sp, -CONTEXT_SIZE
  MAP(REGS1, PUSH)
  li t0, KERNEL
  STORE t0, OFFSET_NP(sp)
  j a2
a1:
  addi sp, sp, -CONTEXT_SIZE
  MAP(REGS1, PUSH)
  csrr t0, mscratch
  STORE t0, OFFSET_SP(sp)
  li t0, USER
  STORE t0, OFFSET_NP(sp)
a2:
  MAP(REGS2, PUSH)
  csrw mscratch, x0
  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc
  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1
  mv a0, sp
  jal __am_irq_handle
  mv sp, a0
  MAP(REGS2, POP)
  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  csrw mstatus, t1
  csrw mepc, t2
  LOAD t0, OFFSET_NP(sp)
  bnez t0, a3
  MAP(REGS1, POP)
  addi sp, sp, CONTEXT_SIZE
  mret
a3:
  addi t0, sp, CONTEXT_SIZE
  csrw mscratch, t0
  MAP(REGS1, POP)
  LOAD sp, OFFSET_SP(sp)
  mret