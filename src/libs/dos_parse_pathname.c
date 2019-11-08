#include "dos.h"


int parse_pathname(char volume_name_flag, char* s) __naked {
  volume_name_flag;
  s;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld b,0(ix)
    ld e,1(ix)
    ld d,2(ix)

    ld c,#PARSE
    DOSCALL

    pop ix
    ret
  __endasm;
}

