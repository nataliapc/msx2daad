#include "dos.h"


char get_current_directory(char drive, char *path) __naked {
  drive, path;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld b,0(ix)
    ld e,1(ix)
    ld d,2(ix)

    ld c, #GETCD
    DOSCALL

    ld h, #0x00
    ld l, a

    pop ix
    ret
  __endasm;
}
