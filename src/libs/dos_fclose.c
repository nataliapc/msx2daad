#include "dos.h"

#include <stdio.h>


uint8_t fclose(char fp) __naked
{
  fp;
#ifdef MSXDOS2

  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld b,(ix)
    ld c,#CLOSE
    DOSCALL

    ld h, #0x00
    ld l,a

    pop ix
    ret
  __endasm;

#else //MSXDOS1 (FCB) or CPM

  __asm
    ld      de,#SYSFCB
    ld      c,#FCLOSE
    DOSCALL

    ld h,#0x00
    ld l,a
    ret
  __endasm;

#endif
}
