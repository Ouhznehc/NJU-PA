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

size_t serial_write(const void *buf, size_t offset, size_t len){
  Log("%d", len);
  char *now = (char *)buf;
  size_t res = len;
  while(*now && len--) putch(*now++);
  return res;
}

size_t events_read(void *buf, size_t offset, size_t len) {
  AM_INPUT_KEYBRD_T kbd = io_read(AM_INPUT_KEYBRD);
  size_t res = 0;
  if(kbd.keycode == AM_KEY_NONE) return 0;
  if(kbd.keydown) res = sprintf(buf, "key down %s\n", keyname[kbd.keycode]);
  else res = sprintf(buf, "key up   %s\n", keyname[kbd.keycode]);
  return res;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  int width  = io_read(AM_GPU_CONFIG).width;
  int height = io_read(AM_GPU_CONFIG).height;
  sprintf(buf, "WIDTH : %d\nHEIGHT : %d\n", width, height);
  return len;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  int width  = io_read(AM_GPU_CONFIG).width * sizeof(uint32_t);
  int h = offset / width;
  int w = (offset - h * width) / sizeof(uint32_t);
  io_write(AM_GPU_FBDRAW, w, h, (uint32_t *)buf, len / sizeof(size_t), 1, false);
  io_write(AM_GPU_FBDRAW, 0, 0, NULL, 0, 0, true);
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
