#include "dos.h"


uint16_t fread(char* buf, uint16_t size, char fp) __naked
{
  buf, size, fp;
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
    ld c,#READ
    DOSCALL

    or a
    jr z, read_noerror$
    ld h, #0xff
    ld l, #0xff
  read_noerror$:
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
    ld l,2(ix)  ; Num. bytes to read
    ld h,3(ix)

    push  hl
    ld    c,#SETDTA               ; Set Disk transfer address (DTA)
    DOSCALL

    ld hl,#1                      ; Set FCB Record size to 1 byte
    ld (#SYSFCB+14),hl
    pop   hl

    ld    de,#SYSFCB
    ld    c,#RDBLK
    DOSCALL

    or a
    jr z, read_noerror$
    ld hl, #0xffff

  read_noerror$:
    pop ix
    ret

  __endasm;

#endif
}
