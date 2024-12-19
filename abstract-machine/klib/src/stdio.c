#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define MAX_LEN 100

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char buf[100];
  int len = vsprintf(buf, fmt, ap);
  for (const char *p = buf; *p; p++) {
    putch(*p);
  }
  return len;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  const char *p = fmt;
  int len = 0;
  while (*p)
  {
    if (*p == '%')
    {
      p++;
      // 位宽实现
      int width = atoi(p);
      char padding = ' ';
      int n = 0; // 填充位数
      if (width != 0)
      {
        // 0填充和空格填充
        if(*++p == '0') padding = '0';
        while ( *p >= '0' && *p <= '9') p++;
      }
      
      switch (*p)
      {
      case 'd':
        int dval = va_arg(ap, int);
        char dbuf[20] = {0};
        int di = 0;
        bool isNeg = 0;
        if (dval < 0)
        {
          out[len++] = '-';
          isNeg = 1;
          dval = -dval;
        }
        
        do
        {
          dbuf[di++] = (dval%10) + '0';
          dval /= 10;
        } while (dval);
        
        if (isNeg)
        {
          n = width-di-1;
        } else n = width-di;
        
        if (n > 0)
        {
          while(n--) out[len++] = padding;
        }
        while(di) out[len++] = dbuf[--di];
        
        break;
      
      case 'x':
        const char hex[] = "0123456789abcdef";
        unsigned int xval = va_arg(ap, unsigned int);
        char xbuf[20];
        int xi = 0;
        if(xval == 0) xbuf[xi++] = '0';
        else {
          while (xval)
          {
            xbuf[xi++] = hex[xval & 0xf];
            xval >>= 4;
          }
        }
        n = width - xi;
        if (n > 0)
        {
          while(n--) out[len++] = padding;
        }
        while (xi) out[len++] = xbuf[--xi];
        break;

      case 's':
        const char* s = va_arg(ap, const char*);
        n = width - strlen(s);
        if (n > 0)
        {
          while(n--) out[len++] = padding;
        }
        while (*s) out[len++] = *s++;
        break;

      default:
        out[len++] = '%';
        out[len++] = *p;
        break;
      }
    } else out[len++] = *p;
    p++;
  }
  out[len] = 0;
  while (*out)
  {
    putch(*out);
    out++;
  }
  putch('\n');
  va_end(ap);
  return len;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int len = vsprintf(out, fmt, ap);
  return len;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
