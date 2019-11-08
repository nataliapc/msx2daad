#include "dos.h"


char get_current_drive(void) __naked {
  __asm
    push ix

    ld c,#CURDRV
    DOSCALL

    ld h, #0x00
    ld l, a

    pop ix
    ret
  __endasm;
}
