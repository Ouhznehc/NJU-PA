#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <NDL.h>

int main() {
  uint32_t time = NDL_GetTicks();
  while(1){
    if(NDL_GetTicks() - time >= 500){
      printf("now time is %dms\n", time);
      time = NDL_GetTicks();
    }
  }
  return 0;
}
