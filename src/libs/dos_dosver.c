#include "dos.h"


char dosver(void) __naked {
  __asm
    push ix

    ld b,  #0x5A      ; magic numbers to detect Nextor
    ld hl, #0x1234
    ld de, #0xABCD
    ld ix, #0

    ld c,#DOSVER
    DOSCALL

    or  a
    jr  z,check_dos2$
    ld  b,#0          ; unknown DOS
    jr  ret_version$

  check_dos2$:
    ld  a,b
    cp  #2
    jr  z,check_nextor$
    ld  b,#1          ; is MSX-DOS 1

  check_nextor$:
    ld  a,i           ; Nextor: IXh must contain '1'
    dec a
    jr  nz,ret_version$ 
    ld  b,#3          ; is NextorDOS

  ret_version$:
    ld h, #0x00
    ld l, b

    pop ix
    ret
  __endasm;
}

