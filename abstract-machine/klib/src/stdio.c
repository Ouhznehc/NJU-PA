#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

char *int_to_string(int num, char *ans){
  int sign = (num >= 0);
  if(!sign) num = -num;
  char reverse[1024];
  char *s = reverse;
  if(num == 0) *s++ = '0';
  else while(num){
    *s++ = num % 10 + '0';
    num /= 10;
  }
  *s = '\0';
  size_t len = strlen(reverse);
  if(!sign) *ans++ = '-';
  for(size_t i = 0; i < len; i++) *ans++ = *(--s);
  return ans;
}


int vsprintf(char *out, const char *fmt, va_list ap) {
  char *str;
  int num;
  char *s;
  for(str = out; *fmt; fmt++){
    if(*fmt != '%'){
      *str++ = *fmt;
      continue;
    }
    fmt++;
    switch(*fmt){
      case 'd':
        num = va_arg(ap, int);
        str = int_to_string(num, str);
        continue;
      case 's':
      case 'c':
        s = va_arg(ap, char*);
        size_t len = strlen(s);
        for(size_t i = 0; i < len; i++) *str++ = *s++;
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

int printf(const char *fmt, ...) {
  char out[1024];
  int val;
  va_list args;
  va_start(args, fmt);
  val = vsprintf(out, fmt, args);
  va_end(args);
  putstr(out);
  return val;
}
#endif
