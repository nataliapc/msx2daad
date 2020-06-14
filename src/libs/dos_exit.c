#include "dos.h"


void exit(int code) __naked {
  code;
#ifdef MSXDOS2

  __asm
    push ix
    ld ix,#4
    add ix,sp
    ld b,(ix)
    pop ix

    ld c,#TERM
    DOSCALL

    ret
  __endasm;

#else //MSXDOS1 (FCB)

  __asm
    jp _exit0
  __endasm;

#endif
}
