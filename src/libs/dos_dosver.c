#include "dos.h"


char dosver(void) __naked {
  __asm
    push ix

    ld b, #0x5A       ; magic numbers to detect Nextor
    ld hl, #0x1234
    ld de, #0xABCD
    ld ix, #0

    ld c, DOSVER
    DOSCALL

    ld a,i            ; Nextor: IXh must contain '1'
    or a
    jr nz,no_nextor$
    ld b, #3

  no_nextor$:
    ld h, #0x00
    ld l, b

    pop ix
    ret
  __endasm;
}

