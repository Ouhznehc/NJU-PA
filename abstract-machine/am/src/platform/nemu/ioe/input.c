#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000
#define KEY_CODE 0x7fff

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t code = inl(KBD_ADDR);
  kbd->keydown = (bool)(code & KEYDOWN_MASK);
  kbd->keycode = (int)(code & KEY_CODE);
  return;
}
