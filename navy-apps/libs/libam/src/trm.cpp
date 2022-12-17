#include <am.h>

Area heap;

void putch(char ch) {
    printf("%c", ch);
}

void halt(int code) {
    asm volatile ("int3" : :"a"(code));
}
