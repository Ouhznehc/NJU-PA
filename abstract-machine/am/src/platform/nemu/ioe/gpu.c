#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)
int W, H;

void __am_gpu_init() {
  W = inw(VGACTL_ADDR + 2);
  H = inw(VGACTL_ADDR);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = inw(VGACTL_ADDR + 2), .height = inw(VGACTL_ADDR),
    .vmemsz = 0
  };
  W = cfg->width;
  H = cfg->height;
}
void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *pixels = ctl->pixels;
  for(int i = 0; i < w; i ++)
    for(int j = 0; j < h; j ++)
      if(x + i < W && y + j < H)
        outl(FB_ADDR + (W * (y + j) + x + i) * 4, pixels[w * j + i]);
      
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
