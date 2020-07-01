#include "dos.h"

uint8_t get_env(char* name, char* buffer, char buffer_size) __naked {
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
    pop ix

    ld c,#GENV
    DOSCALL

    ld l,a

    ret
  __endasm;
}
