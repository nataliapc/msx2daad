#include "dos.h"


void set_transfer_address(uint8_t *memaddress) __naked {
  memaddress;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld e,0(ix)
    ld d,1(ix)
    ld c,#SETDTA
    DOSCALL

    pop ix
    ret
  __endasm;
}

