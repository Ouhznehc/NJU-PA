#ifndef __DEBUG_H__
#define __DEBUG_H__

#include <common.h>
// #define ANSI_FG_BLACK   "\33[1;30m"
// #define ANSI_FG_RED     "\33[1;31m"
// #define ANSI_FG_GREEN   "\33[1;32m"
// #define ANSI_FG_YELLOW  "\33[1;33m"
// #define ANSI_FG_BLUE    "\33[1;34m"
// #define ANSI_FG_MAGENTA "\33[1;35m"
// #define ANSI_FG_CYAN    "\33[1;36m"
// #define ANSI_FG_WHITE   "\33[1;37m"
// #define ANSI_BG_BLACK   "\33[1;40m"
// #define ANSI_BG_RED     "\33[1;41m"
// #define ANSI_BG_GREEN   "\33[1;42m"
// #define ANSI_BG_YELLOW  "\33[1;43m"
// #define ANSI_BG_BLUE    "\33[1;44m"
// #define ANSI_BG_MAGENTA "\33[1;35m"
// #define ANSI_BG_CYAN    "\33[1;46m"
// #define ANSI_BG_WHITE   "\33[1;47m"
// #define ANSI_NONE       "\33[0m"

#define Log(format, ...) \
  printf("\33[1;35m[%s,%d,%s] " format "\33[0m\n", \
      __FILE__, __LINE__, __func__, ## __VA_ARGS__)

#undef panic
#define panic(format, ...) \
  do { \
    Log("\33[1;31msystem panic: " format, ## __VA_ARGS__); \
    halt(1); \
  } while (0)
// ================= my printf =============================
#define color_red(format, ...) \
  do { \
    printf("\33[1;31m" format "\33[0m", ## __VA_ARGS__); \
  } while (0)

#define color_yellow(format, ...) \
  do { \
    printf("\33[1;33m" format "\33[0m", ## __VA_ARGS__); \
  } while (0)

#define color_blue(format, ...) \
  do { \
    printf("\33[1;34m" format "\33[0m", ## __VA_ARGS__); \
  } while (0)

#define color_green(format, ...) \
  do { \
    printf("\33[1;32m" format "\33[0m", ## __VA_ARGS__); \
  } while (0)
// ========================================================


#ifdef assert
# undef assert
#endif

#define assert(cond) \
  do { \
    if (!(cond)) { \
      panic("Assertion failed: %s", #cond); \
    } \
  } while (0)

#define TODO() panic("please implement me")

#endif
