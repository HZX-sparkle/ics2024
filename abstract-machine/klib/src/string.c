#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

size_t strlen(const char *s) {
  panic("Not implemented");
}

char *strcpy(char *dst, const char *src) {
  char *p = dst;
  while ((*p++ = *src++)!=0);
  return dst;
  // panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
  panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  char *p = dst;
  while(!*p) p++;
  while((*p++ = *src++)!=0);
  return dst;
  // panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  while (1)
  {
    char c1 = *s1++;
    char c2 = *s2++;
    if(c1 != c2) return c1 > c2 ? 1 : -1;
    if(!c1) break;
  }
  return 0;
}

int strncmp(const char *s1, const char *s2, size_t n) {
  panic("Not implemented");
}

void *memset(void *s, int c, size_t n) {
  unsigned char* p = s;
  unsigned char uc = (unsigned char)c;
  for (size_t i = 0; i < n; i++) *p++ = uc;
  return s;
}

void *memmove(void *dst, const void *src, size_t n) {
  panic("Not implemented");
}

void *memcpy(void *out, const void *in, size_t n) {
  panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  while (n--)
  {
    unsigned char c1 = *(unsigned char*)s1;
    unsigned char c2 = *(unsigned char*)s2;
    if(c1 != c2) return c1 > c2 ? 1 : -1;
    if(!c1) break;
  }
  return 0;
}

#endif
