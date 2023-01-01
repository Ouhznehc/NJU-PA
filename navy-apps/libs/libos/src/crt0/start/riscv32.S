.globl  _start
_start:
  move s0, zero
  //set right stack pointer
  //mv sp, a0
  jal call_main
