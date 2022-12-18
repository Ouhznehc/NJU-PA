#include <unistd.h>
#include <stdio.h>

int main() {
  write(1, "Hello World!\n", 13);
  int i = 2;
  volatile int j = 100;
  printf("yzhnmb\n");
  while (j--) {
      printf("Hello World from Navy-apps for the %dth time!\n", i ++);
  }
  return 0;
}
