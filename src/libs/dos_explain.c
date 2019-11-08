#include "dos.h"


void explain(char* buffer, char error_code) __naked {
  error_code;
  buffer;
#ifdef MSXDOS2
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld e,0(ix)
    ld d,1(ix)
    ld b,2(ix)

    ld c,#EXPLAIN
    DOSCALL

    ld 0(ix),d
    ld 1(ix),e

    pop ix
    ret
  __endasm;
#else
    *buffer = '\0';
#endif
}
