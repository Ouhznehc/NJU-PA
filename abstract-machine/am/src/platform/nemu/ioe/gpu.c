#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
#define W 400
#define H 300

extern void *memcpy(void *out, const void *in, size_t n);

void __am_gpu_init() {
}

uint32_t screen_width(){
  return inl(VGACTL_ADDR)>>16;
}

uint32_t screen_height(){
  return inl(VGACTL_ADDR)&0xffff;
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = (int)screen_width(), .height = (int)screen_height(),
    .vmemsz = (int)(screen_width()*screen_height()*sizeof(uint32_t))
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  int minw = (W-x < w)? W-x : w;
  int copy_bits = sizeof(uint32_t) * minw;
  for(int i = 0; i < h && y + i < H; i++){
    memcpy(&fb[(y + i) * W + x], pixels, copy_bits);
    pixels += w;
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
