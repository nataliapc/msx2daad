#include "dos.h"

/*
    sdcccall(1)
    IN:  none
    OUT: DE
*/
int getchar(void) __naked
{
  __asm
    push ix

    ld c,#INNOE
    DOSCALL

    pop  ix
    ld d, #0x00
    ld e, l

    ret
  __endasm;
}
