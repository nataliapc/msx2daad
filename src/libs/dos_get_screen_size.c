#include "dos.h"


char get_screen_size(void) __naked
{
  __asm
    push ix

    ld a, #0x04

    ld c,#IOCTL
    DOSCALL

    ld a, e
    pop ix
    ret
  __endasm;
}
