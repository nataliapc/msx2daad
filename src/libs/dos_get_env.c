#include "dos.h"

char get_env(char* name, char* buffer, char buffer_size) __naked {
  name;
  buffer;
  buffer_size;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld l,0(ix)
    ld h,1(ix)
    ld e,2(ix)
    ld d,3(ix)
    ld b,4(ix)

    ld c,#GENV
    DOSCALL

    ld 0(ix),l
    ld 1(ix),h
    ld 2(ix),d
    ld 3(ix),e
    ld 4(ix),a

    pop ix
    ret
  __endasm;
}
