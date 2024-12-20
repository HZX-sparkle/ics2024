#include <am.h>
#include <nemu.h>
#include <stdio.h>

#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t k = AM_KEY_NONE;
  k = inl(KBD_ADDR);
  kbd->keydown = (KEYDOWN_MASK & k) ? true : false;
  kbd->keycode = k & ~KEYDOWN_MASK;
}
