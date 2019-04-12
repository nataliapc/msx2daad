#include "dos.h"


uint16_t fcreate(char *fn, char mode, char attributes) __naked
{
  fn;
  mode;
  attributes;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld e,0(ix)
    ld d,1(ix)
    ld a,2(ix)
    ld b,3(ix)
    ld c, CREATE
    DOSCALL

    or a
    jr z, create_noerror$
    ld h, #0xff
    ld l, a
    jp create_error$
  create_noerror$:
    ld h, #0x00
    ld l, b
  create_error$:
    pop ix
    ret
  __endasm;
}
