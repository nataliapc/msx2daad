#include "dos.h"


uint16_t fread(char* buf, unsigned int size, char fp) __naked
{
  buf;
  size;
  fp;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld e,0(ix)
    ld d,1(ix)
    ld l,2(ix)
    ld h,3(ix)
    ld b,4(ix)
    ld c, READ
    DOSCALL

    or a
    jr z, read_noerror$
    ld h, #0xff
    ld l, #0xff
  read_noerror$:
    pop ix
    ret
  __endasm;
}
