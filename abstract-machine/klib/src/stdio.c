#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char out[1024];

char* itoa(int value, char *string, int radix, bool zeroflag, int field_width)
{
	char str[37]="0123456789abcde";
	char temp[100]={0};
  int count = 0;
  
	int i=0;
  if(value==0){
    *string++ = '0';
    *string = '\0';
    return string;
  }

  if(value<0){
    *string++ = '-';
    value = -value;
  }

	while(value)
	{
		temp[i++] = str[value % radix];
		value /= radix;
    count++;
	}

  if(field_width != -1){
      for(size_t i = 0; i < field_width - count; i++)
      *string++ = zeroflag? '0' : ' ';
  }
 
	for(int j=i-1;j>=0;j--)
	{
		*string++ = temp[j];
	}

	*string = '\0';
	return string;
}

char* uitoa(uint32_t value, char *string, int radix, bool zeroflag, int field_width)
{
	char str[37]="0123456789abcdef";
	char temp[100]={0};
  int count = 0;
  
	int i=0;
  if(value==0){
    *string++ = '0';
    *string = '\0';
    return string;
  }

	while(value)
	{
		temp[i++] = str[value % radix];
		value /= radix;
    count++;
	}

  if(field_width != -1){
      for(size_t i = 0; i < field_width - count; i++)
      *string++ = zeroflag? '0' : ' ';
  }
 
	for(int j=i-1;j>=0;j--)
	{
		*string++ = temp[j];
	}

	*string = '\0';
	return string;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int res = vsprintf(out, fmt, ap);
  va_end(ap);
  putstr(out);
  return res;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  char *out_copy = out;
  const char *p = NULL;
  int n = 0, field_width = -1;
  bool zeroflag = false;
  while(*fmt){
    if(*fmt != '%'){
      *out_copy++ = *fmt;
    }else{
      fmt++;
      if(*fmt == '0'){ fmt++; zeroflag = true;}
      if(*fmt > '0' && *fmt <= '9'){
          field_width = atoi(fmt);
          while(*fmt >= '0' && *fmt <= '9') fmt++;
      }
      switch (*fmt)
      { 
        case 'c':
        case 's':
          p = va_arg(ap, char*);
          n = strlen(p);
          for(size_t i = 0; i < n; i++) *out_copy++ = *p++;
          break;
        
        case 'd':
          out_copy = itoa(va_arg(ap, int), out_copy, 10, zeroflag, field_width);
          break;

        case 'p':
          *out_copy++ = '0';
          *out_copy++ = 'x';
          out_copy = uitoa(va_arg(ap, uint32_t), out_copy, 16, zeroflag, field_width);
        default:
          break;
      }
    }
    fmt++;
  }
  *(out_copy) = '\0';
  return out_copy - out;
}

int sprintf(char *out, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int res = vsprintf(out, fmt, ap);
    va_end(ap);
    return res;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif




