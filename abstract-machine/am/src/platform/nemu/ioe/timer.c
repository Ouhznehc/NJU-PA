#include <am.h>
#include <nemu.h>
static uint64_t bool_time = 0;

static uint64_t __am_timer_get_time(){
  uint64_t hi = (uint64_t)inl(RTC_ADDR + 4);
  uint64_t lo = (uint64_t)inl(RTC_ADDR + 0);
  uint64_t time = (hi << 32) | lo;
  return time;
}

void __am_timer_init() {
  bool_time = __am_timer_get_time();
  return;
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = __am_timer_get_time() - bool_time;
  return;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
