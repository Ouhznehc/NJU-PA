#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  const char *temp;
  size_t len = 0;
  for ( temp = s; *temp != '\0'; temp++)
  {
    len++;
  }
  return len;
}

char *strcpy(char *dst, const char *src) {
  char *temp = dst;
  while((*dst++ = *src++) != '\0'){
    ;
  }
  return temp;
}

char *strncpy(char *dst, const char *src, size_t n) {
  char *temp = dst;
  while(n){
    if ((*temp = *src) != 0)
        src++;
    temp++;
    n--;
  }
  return dst;  
}

char *strcat(char *dst, const char *src) {
  char *temp = dst;
  while(*dst){
    dst ++;
  }
  while((*dst++ = *src++) != '\0'){
    ;
  }
  return temp;  
}

int strcmp(const char *s1, const char *s2) {
  while(*s1 == *s2){
    if(*s1 == '\0'){
      return 0;
    }
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  while(*s1 == *s2 && n--){
    if (*s1 == '\0')
    {
      return 0;
    }
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

void *memset(void *s, int c, size_t n) {
  char *temp = (char*)s;
  while(n--){
    *temp++ = c;
  }
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
	char *ret=(char *)dst;
	char *dest_t=dst;
	char *src_t=(char *)src;
	assert( NULL != src && NULL != dst);
	
	if (dest_t<=src_t || dest_t>=src_t+n)
	{
        while(n--)
		{
			*dest_t++ = *src_t++;
		}
	}
	else
	{
		dest_t += n-1;
		src_t += n-1;
		while(n--)
		{
			*dest_t-- = *src_t--;
		}
	}
	return ret;
}

void *memcpy(void *out, const void *in, size_t n) {
 char *d;
 const char *s;
 if ((out > (in + n)) || (out < in)){
    d = out;
    s = in;
    while (n--)
        *d++ = *s++;        
 }else{
    d = (char *)(out + n - 1); /* offset of pointer is from 0 */
    s = (char *)(out + n - 1);
    while (n --)
        *d-- = *s--;
 }
 return out;
}

int memcmp(const void *s1, const void *s2, size_t n) {
  const char *su1=s1, *su2=s2;
  int res = 0;
  for (; n; su1++, su2++, n--)
  {
    if((res = *su1-*su2) != 0){
      break;
    }
  }
  return res;
}

#endif
