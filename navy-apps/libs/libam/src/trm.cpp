#include <am.h>

Area heap;

void putch(char ch) {
    printf("%c", ch);
}

void halt(int code) {
    asm volatile("mv a0, %0; ebreak" : :"r"(code));
    //should not reach here
    while(1);
}
