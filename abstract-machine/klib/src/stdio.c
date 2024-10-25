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
  int len = 0;
  while (*p)
  {
    if (*p == '%')
    {
      switch (*++p)
      {
      case 'd':
        int val = va_arg(args, int);
        char buf[20] = {0};
        int i = 0;
        if (val<0)
        {
          out[len++] = '-';
          val = -val;
        }
        
        do
        {
          buf[i++] = val%10 + '0';
          val /= 10;
        } while (val);

        while (i) out[len++] = buf[--i];

        break;
      
      case 's':
        const char* s = va_arg(args, const char*);
        while (*s) out[len++] = *s++;
        break;

      default:
        out[len++] = '%';
        out[len++] = *p;
        break;
      }
    } else out[len] = *p;
    p++;
  }
  out[len] = 0;
  va_end(args);
  return len;

  // panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
