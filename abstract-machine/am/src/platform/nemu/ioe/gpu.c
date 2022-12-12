#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

uint32_t screen_width() {
  return (uint32_t)inw(VGACTL_ADDR + 2);
}

uint32_t screen_height() {
  return (uint32_t)inw(VGACTL_ADDR);
}

uint32_t screen_size() {
  return screen_width() * screen_height() * sizeof(uint32_t);
}

void __am_gpu_init() {
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = (int)screen_width(), .height = (int)screen_height(),
    .vmemsz = (int)screen_size()
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *pixels = ctl->pixels;
  uint32_t width = screen_width(), height = screen_height();
  for(int i = 0; i < w; i++)
    for(int j = 0; j < h; j++)
      if(x + i < width && y + j < height)
        outl(FB_ADDR + (width * (y + j) + x + i) * 4, pixels[i + w * j]);
  if(ctl->sync) outl(SYNC_ADDR, 1);
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
