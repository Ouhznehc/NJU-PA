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

#define REGS(f) \
      f( 1)       f( 3) f( 4) f( 5) f( 6) f( 7) f( 8) f( 9) \
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

  // set sp from mscratch (kernel sp)
  // store original sp in mscratch
  csrrw sp, mscratch, sp
  
  // if sp is valid, do as user state
  bnez sp, user_save_sp
  // if it is kernel state
  
  // restore sp
  csrr sp, mscratch

  // push the context structure
  addi sp, sp, -CONTEXT_SIZE

  // push gprs other than sp
  MAP(REGS, PUSH)


  // tell the trap to restore in a 'kernel state' way
  li t0, KERNEL
  STORE t0, OFFSET_NP(sp)

  j end_set_state

  // if it is user state
user_save_sp:

  // push the context structure
  addi sp, sp, -CONTEXT_SIZE

  // push gprs other than sp
  MAP(REGS, PUSH)

  // read original sp from mscratch and store original sp
  csrr t0, mscratch
  STORE t0, OFFSET_SP(sp)

  // tell the trap to restore in a 'user state' way
  li t0, USER
  STORE t0, OFFSET_NP(sp)

end_set_state:

  // set mscratch in kernel state
  csrw mscratch, x0

  // save more registers
  csrr t0, mcause
  csrr t1, mstatus
  csrr t2, mepc

  STORE t0, OFFSET_CAUSE(sp)
  STORE t1, OFFSET_STATUS(sp)
  STORE t2, OFFSET_EPC(sp)

  // set mstatus.MPRV to pass difftest
  li a0, (1 << 17)
  or t1, t1, a0
  csrw mstatus, t1

  // call the handler 
  mv a0, sp
  jal __am_irq_handle

  // get next context address
  mv sp, a0

  // load mstatus and epc
  LOAD t1, OFFSET_STATUS(sp)
  LOAD t2, OFFSET_EPC(sp)
  csrw mstatus, t1
  csrw mepc, t2

  // read the way to restore ksp and sp
  LOAD t0, OFFSET_NP(sp)

  // select by t0 to restore by a 'user state' or 'kernel state' way
  bnez t0, user_restore_sp
  
  // restore gprs
  MAP(REGS, POP)

  addi sp, sp, CONTEXT_SIZE
  // no need to restore kernel sp and sp
  mret

user_restore_sp:

  // calculate and restore kernel sp
  addi t0, sp, CONTEXT_SIZE
  csrw mscratch, t0

  // restore gprs
  MAP(REGS, POP)
  
  // restore original sp
  LOAD sp, OFFSET_SP(sp)
  
  mret
