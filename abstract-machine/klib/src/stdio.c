// #include <am.h>
// #include <klib.h>
// #include <klib-macros.h>
// #include <stdarg.h>

// #if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

// char *int_to_string(int num, char *ans, int zeroflag, int field_width){
//   int sign = (num >= 0);
//   int counter = 0;
//   if(!sign) num = -num;
//   char reverse[1024];
//   char *s = reverse;
//   if(num == 0) {*s++ = '0'; counter++;}
//   else while(num){
//     *s++ = num % 10 + '0';
//     counter++;
//     num /= 10;
//   }
//   *s = '\0';
//   size_t len = strlen(reverse);
//   if(!sign) *ans++ = '-';
//   if(field_width != -1){
//     for(size_t i = 0; i < field_width - counter; i++)
//       *ans++ = zeroflag? '0' : ' ';
//   }
//   for(size_t i = 0; i < len; i++) *ans++ = *(--s);
//   return ans;
// }


// int vsprintf(char *out, const char *fmt, va_list ap) {
//   char *str;
//   int num;
//   char *s;
//   int zeroflag = false, field_width = -1;
//   for(str = out; *fmt; fmt++){
//     if(*fmt != '%'){ *str++ = *fmt; continue;}
//     fmt++;
//     if(*fmt == '0'){ fmt++; zeroflag = true;}
//     if(*fmt >= '0' && *fmt <= '9'){
//       field_width = atoi(fmt);
//       while(*fmt >= '0' && *fmt <= '9') fmt++;
//     }
//     switch(*fmt){
//       case 'd':
//         num = va_arg(ap, int);
//         str = int_to_string(num, str, zeroflag, field_width);
//         continue;
//       case 's':
//       case 'c':
//         s = va_arg(ap, char*);
//         size_t len = strlen(s);
//         if(field_width != -1){
//           for(size_t i = 0; i < field_width - len; i++)
//             *str++ = zeroflag ? '0' : ' ';
//         }
//         for(size_t i = 0; i < len; i++) *str++ = *s++;
//         continue; 
//       default: break;
//     }
//   }
//   *str = '\0';
//   return str - out;
// }

// int sprintf(char *out, const char *fmt, ...) {
//   va_list args;
//   int val;
//   va_start(args, fmt);
//   val = vsprintf(out, fmt, args);
//   va_end(args);
//   return val;
// }

// int snprintf(char *out, size_t n, const char *fmt, ...) {
//   panic("Not implemented");
// }

// int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
//   panic("Not implemented");
// }

// int printf(const char *fmt, ...) {
//   char out[1024];
//   int val;
//   va_list args;
//   va_start(args, fmt);
//   val = vsprintf(out, fmt, args);
//   va_end(args);
//   putstr(out);
//   return val;
// }
// #endif


#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char sprint_buf[1024];

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int check = vsprintf(sprint_buf, fmt, ap);
  va_end(ap);
  for(int i = 0; i < check; i++)
    putch(sprint_buf[i]);
  return check;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *tmp = out;
  while(*fmt != '\0'){
    if(*fmt != '%'){
        *tmp++ = *fmt++;
        continue;
    }
    fmt ++;
    char *s;
    int len, i, pos, flag, zeroflag = false, field_width = -1;
    int64_t num;
    uint64_t unum;
    char snum[15];
    if(*fmt == '0'){
      fmt ++;
      zeroflag = true;
    }
    if(*fmt >= '0' && *fmt <= '9'){
      field_width = atoi(fmt);
      while(*fmt >= '0' && *fmt <= '9')
        fmt ++;
    }
      
    switch (*fmt)
    {
      case 's':
        s = va_arg(ap, char *);
        len = strlen(s);
        if(field_width != -1){
          for(i = 0; i < field_width - len; i++)
            *tmp++ = zeroflag?'0':' ';
        }
        for(i = 0; i < len; i++)
          *tmp++ = *s++;
        break;
      case 'd':
        num = va_arg(ap, int);
        pos = 0;
        flag = 0;
        if(num < 0){
          flag = 1;
          num *= -1;
        }
        if(num == 0){
          snum[++ pos] = '0';
        }
        while(num){
          snum[++pos] = '0' + (num % 10);
          num /= 10;
        }
        if(flag)
          *tmp++ = '-';
        if(field_width != -1){
          for(i = 0; i < field_width - pos; i++)
            *tmp++ = zeroflag?'0':' ';
        }
        for(i = 0; i < pos; i++)
          *tmp++ = snum[pos - i];
        break;
      case 'x': 
        unum = va_arg(ap, uint32_t);
        pos = 0;
        if(unum == 0){
          snum[++ pos] = '0';
        }
        while(unum){
          snum[++pos] = unum % 16;
          if(snum[pos] < 10)snum[pos] += '0';
          else snum[pos] += 'a' - 10;
          unum /= 16;
        }
        if(field_width != -1){
          for(i = 0; i < field_width - pos; i++)
            *tmp++ = zeroflag?'0':' ';
        }
        for(i = 0; i < pos; i++)
          *tmp++ = snum[pos - i];
        break;
      case 'p':
        unum = (unsigned int)va_arg(ap, void *);
        pos = 0;
        if(unum == 0){
          snum[++ pos] = '0';
        }
        while(unum){
          snum[++pos] = unum % 16;
          if(snum[pos] < 10)snum[pos] += '0';
          else snum[pos] += 'a' - 10;
          unum /= 16;
        }
        *tmp++ = '0';
        *tmp++ = 'x';
        if(field_width != -1){
          for(i = 0; i < field_width - pos; i++)
            *tmp++ = zeroflag?'0':' ';
        }
        for(i = 0; i < pos; i++)
          *tmp++ = snum[pos - i];
        break;
      case '%':
        *tmp++ = '%';
        break;
      default:
        break;
    }

    fmt ++;
  }
  *tmp = '\0';
  return tmp - out;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int check = vsprintf(out, fmt, ap);
  va_end(ap);
  return check;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
