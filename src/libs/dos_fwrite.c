#include "dos.h"

#include <stdio.h>


uint16_t fwrite(char* buf, uint16_t size, char fp) __naked
{
  buf;
  size;
  fp;
#ifdef MSXDOS2

  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld e,0(ix)
    ld d,1(ix)
    ld l,2(ix)
    ld h,3(ix)
    ld b,4(ix)
    ld c,#WRITE
    DOSCALL

    or a
    jr z, write_noerror$
    ld hl, #0xffff
  write_noerror$:
    pop ix
    ret
  __endasm;

#else //MSXDOS1 (FCB)

  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld e,0(ix)  ; Disk trasfer address
    ld d,1(ix)
    ld l,2(ix)  ; Num. bytes to write
    ld h,3(ix)

    push  hl
    ld    c,#SETDTA               ; Set Disk transfer address (DTA)
    DOSCALL

    ld hl,#1                      ; Set FCB Record size to 1 byte
    ld (#SYSFCB+14),hl
    pop   hl

    ld    de,#SYSFCB
    ld    c,#WRBLK
    DOSCALL

    or a
    jr z,write_noerror$
    ld hl, #0xffff
  write_noerror$:
    pop ix
    ret
  __endasm;

#endif
}

