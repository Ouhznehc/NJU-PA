#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>

static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;
static int canvas_w = 0, canvas_h = 0;

uint32_t NDL_GetTicks() {
  struct timeval time;
  gettimeofday(&time, NULL);
  uint32_t time_ms = time.tv_sec * 1000 + time.tv_usec / 1000;
  return time_ms;
}

int NDL_PollEvent(char *buf, int len) {
  int events = open("/dev/events", O_RDONLY);
  int res = read(events, buf, len);
  return (res != 0);
}

void NDL_OpenCanvas(int *w, int *h) {
  char dispinfo_buf[64];
  int dispinfo = open("/proc/dispinfo", O_RDONLY);
  read(dispinfo, dispinfo_buf, 64);
  sscanf(dispinfo_buf, "WIDTH : %d\nHEIGHT : %d\n", &canvas_w, &canvas_h);
  if(*w == 0 && *h == 0){*w = canvas_w, *h = canvas_h;}
  screen_w = *w; screen_h = *h;
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    write(fbctl, buf, len);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
}

void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  int fb = open("/dev/fb", O_WRONLY);
  int center_w = (canvas_w - screen_w) / 2;
  int center_h = (canvas_h - screen_h) / 2;
  for (int i = 0; i < h; i++){
    size_t offset = (center_h + y + i) * canvas_w + center_w + x;
    lseek(fb,  offset * sizeof(uint32_t), SEEK_SET);
    write(fb, pixels + w * i, w * sizeof(uint32_t));
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  return 0;
}

void NDL_Quit() {
}
