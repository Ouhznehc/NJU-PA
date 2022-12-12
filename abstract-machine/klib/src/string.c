#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  size_t len = 0;
  while(*(s + len) != '\0')
    len ++;
  return len;
}

char *strcpy(char *dst, const char *src) {
  char *tmp1 = dst;
  const char *tmp2 = src;
  while((*tmp1++ = *tmp2++) != '\0');
  return dst;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *tmp1 = dst;
  const char *tmp2 = src;
  while(n-- && ((*tmp1++ = *tmp2++) != '\0'));
  while(n--){
    *tmp1 = '\0';
    tmp1 ++;
  }
  return dst;
}

char *strcat(char *dst, const char *src) {
  char *tmp1 = dst;
  const char *tmp2 = src;
  while(*tmp1)tmp1 ++;
  while((*tmp1++ = *tmp2++) != '\0');
  return dst;
}

int strcmp(const char *s1, const char *s2) {
  const char *tmp1 = s1;
  const char *tmp2 = s2;
  while(1){
    if(*tmp1 != *tmp2)
      return ((*tmp1 < *tmp2) ? -1 : 1);
    if(*tmp2 == '\0')
      return 0;
    tmp1 ++;
    tmp2 ++;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  const char *tmp1 = s1;
  const char *tmp2 = s2;
  while(n--){
    if(*tmp1 != *tmp2)
      return ((*tmp1 < *tmp2) ? -1 : 1);
    if(*tmp2 == '\0')
      return 0;
    tmp1 ++;
    tmp2 ++;
  }
  return 0;
}

void *memset(void *s, int c, size_t n) {
  char *tmp = s;
  while(n--){
    *tmp++ = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  char *tmp1 = dst;
  const char *tmp2 = src;
  if(tmp2 < tmp1 && tmp1 < tmp2 + n){
    tmp1 += n;
    tmp2 += n;
    while(n--){
      tmp1 --;
      tmp2 --;
      *tmp1 = *tmp2;
    }
  }
  else 
    while(n--){
      *tmp1++ = *tmp2++;
    }
  return dst;
}

void *memcpy(void *out, const void *in, size_t n) {
  char *tmp1 = out;
  const char *tmp2 = in;
  while(n--){
    *tmp1++ = *tmp2++;
  }
  return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *tmp1 = s1;
  const char *tmp2 = s2;
  while(n--){
    if(*tmp1 != *tmp2)
      return ((*tmp1 < *tmp2) ? -1 : 1);
    tmp1 ++;
    tmp2 ++;
  }
  return 0;
}

#endif
