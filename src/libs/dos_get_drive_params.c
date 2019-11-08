#include "dos.h"


char get_drive_params(char drive, DPARM_info *param) __naked {
  drive, param;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld l,0(ix)
    ld e,1(ix)
    ld d,2(ix)
    ld c,#DPARM
    DOSCALL

    ld h, #0x00
    or a
    jr z, dparm_noerrors$
    ld l, a
    jp dparm_errors$
  dparm_noerrors$:
    ld l, #0x00
  dparm_errors$:
    pop ix
    ret
  __endasm;
}
