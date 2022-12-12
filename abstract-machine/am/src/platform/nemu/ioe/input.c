#include <am.h>
#include <nemu.h>

#define KEYDOWN_MASK 0x8000
#define KEYCODE_MASK 0x7fff

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint16_t kbd_value = inw(KBD_ADDR);
  kbd->keydown = kbd_value & KEYDOWN_MASK;
  kbd->keycode = (int)(kbd_value & KEYCODE_MASK);
}
