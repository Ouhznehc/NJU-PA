#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};
void switch_program_index(int index);

size_t serial_write(const void *buf, size_t offset, size_t len){
  char *now = (char *)buf;
  size_t res = len;
  while(*now && len--) putch(*now++);
  return res;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T kbd = io_read(AM_INPUT_KEYBRD);
  size_t res = 0;
  if(kbd.keycode == AM_KEY_NONE) return 0;
  if(kbd.keydown){
    res = sprintf(buf, "key down %s\n", keyname[kbd.keycode]);
    switch (kbd.keycode){
      case AM_KEY_F1:
        switch_program_index(1);
        break;
      case AM_KEY_F2:
        switch_program_index(2);
        break;
      case AM_KEY_F3:
        switch_program_index(3);
        break;
      case AM_KEY_F4:
        switch_program_index(4);
        break;
      default:break;
    }
  }
  else res = sprintf(buf, "key up   %s\n", keyname[kbd.keycode]);
  return res;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  AM_GPU_CONFIG_T config = io_read(AM_GPU_CONFIG);
  int width  = config.width;
  int height = config.height;
  sprintf(buf, "WIDTH : %d\nHEIGHT : %d\n", width, height);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int width  = io_read(AM_GPU_CONFIG).width * sizeof(uint32_t);
  int h = offset / width;
  int w = (offset - h * width) / sizeof(uint32_t);
  io_write(AM_GPU_FBDRAW, w, h, (uint32_t *)buf, len / sizeof(size_t), 1, true);
  return len;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
