.globl  _start
_start:
  move s0, zero
  mv sp, a0
  mv a0, a1
  jal call_main
