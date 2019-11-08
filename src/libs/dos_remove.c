#include "dos.h"


uint16_t remove(char *file) __naked
{
  file;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld e,0(ix)
    ld d,1(ix)

    ld c,#DELETE
    DOSCALL

    ld h, #0xff
    ld l, a

    pop ix
    ret
  __endasm;
}