#include "dos.h"


int cgetchar(void) __naked {
  __asm
    push ix

    ld c,CONIN
    DOSCALL
    ld h, #0x00

    pop ix
    ret
  __endasm;
}