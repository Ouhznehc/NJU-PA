#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *str;
  for(str = out; *fmt; fmt++){
    if(*fmt != '%'){
      *str++ = *fmt;
      continue;
    }
    fmt++;
    switch(*fmt){
      case 'd':
        int num = va_arg(ap, int);
        while(num){
          *str++ = (char)(num % 10 + '0');
          num /= 10;
        }
        continue;
      case 's':
        char *s = va_arg(ap, char*);
        size_t len = strlen(s);
        //for(size_t i = 0; i < len; i++) *str++ = *s++;
        strncpy(str, s, len);
        str += len;
        continue; 
      default: panic("Not implemented");
    }
  }
  *str = '\0';
  return str - out;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  int val;
  va_start(args, fmt);
  val = vsprintf(out, fmt, args);
  va_end(args);
  return val;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
