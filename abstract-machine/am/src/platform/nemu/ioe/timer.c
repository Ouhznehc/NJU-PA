#include <am.h>
#include <nemu.h>
//#include "riscv.h"
static AM_TIMER_UPTIME_T boot_time;
static uint64_t __am_timer_get_now_time(){
  uint64_t time_at_now;
  uint32_t high = inl(RTC_ADDR+4);
  uint32_t low = inl(RTC_ADDR);
  time_at_now = (uint64_t)low | ((uint64_t)high<<32);
  return time_at_now;
}

void __am_timer_init() {
  boot_time.us = __am_timer_get_now_time();
}

void __am_timer_uptime(AM_TIMER_UPTIME_T *uptime) {
  uptime->us = __am_timer_get_now_time() - boot_time.us;
}

void __am_timer_rtc(AM_TIMER_RTC_T *rtc) {
  rtc->second = 0;
  rtc->minute = 0;
  rtc->hour   = 0;
  rtc->day    = 0;
  rtc->month  = 0;
  rtc->year   = 1900;
}
