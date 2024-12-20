#include <am.h>
#include <nemu.h>

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  // int i;
  // AM_GPU_CONFIG_T info = io_read(AM_GPU_CONFIG);
  // int w = info.width, h = info.height;
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i ++) fb[i] = i;
  // outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  int width = inw(VGACTL_ADDR + 2);
  int height = inw(VGACTL_ADDR);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = width, .height = height,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) {
  AM_GPU_CONFIG_T info = io_read(AM_GPU_CONFIG);
  int width = info.width;
  int w = ctl->w, h = ctl->h,
      x = ctl->x, y = ctl->y;
  uint32_t *pixels = ctl->pixels;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  int n = 0;
  for (int j = y; j < y+h; j++)
  {
    for (int i = x ; i < x+w; i++)
    {
      fb[i+j*width] = pixels[n++];
    }
  }
  if (ctl->sync) {
    outl(SYNC_ADDR, 1);
  }
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
