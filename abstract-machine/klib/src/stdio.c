#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  const char *p = fmt;
  char *dst = out;
  while (*p)
  {
    if(*p == '%')
    {
      switch (*++p)
      {
      case 'd':
        int val = va_arg(args, int);
        char buf[20];
        int i = 0;
        if (val < 0)
        {
          *dst++ = '-';
          val = -val;
        }
        do
        {
          buf[i++] = val%10 + '0';
          val /= 10;
        } while (val != 0);
        for (i = i - 1; i >=0 ; i--) *dst++ = buf[i];
        break;
      
      case 's':
        char *s = va_arg(args, char*);
        while((*dst++ = *s++));
        break;

      default:
        *dst++ = '%';
        *dst++ = *p;
        break;
      }
    } else *dst++ = *p;
    p++;
  }
  return dst-out;
  // panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
