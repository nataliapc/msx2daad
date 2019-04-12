#include "dos.h"


int putchar(int c) __naked {
  c;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld a,(ix)
    cp #0x0a
    ld e,a
    ld c,CONOUT
    jp nz,jumpPutchar$


#ifdef DEBUG
    push af            ; to use with openmsx and '-ext debugdevice' extension
    ld  a, #0x63
    out (#0x2e),a
    ld  a,e
    out (#0x2f),a
    pop af
#endif

    DOSCALL
    ld e,#0x0d
jumpPutchar$:
    DOSCALL

#ifdef DEBUG
    push af            ; to use with openmsx and '-ext debugdevice' extension
    ld  a, #0x63
    out (#0x2e),a
    ld  a,e
    out (#0x2f),a
    pop af
#endif

    pop ix
    ret
  __endasm;
}
