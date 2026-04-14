#include "dos.h"

/*
    sdcccall(1)
    IN:  HL
    OUT: DE
*/
int putchar(int c) __naked __sdcccall(1)
{
  c;
  __asm
    push hl
    ld e,l
    ld c,#CONOUT

    ld a,e
    cp #0x0a
    jp nz,jumpPutchar$

#if defined(DEBUG) || defined(TEST)
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

#if defined(DEBUG) || defined(TEST)
    push af            ; to use with openmsx and '-ext debugdevice' extension
    ld  a, #0x63
    out (#0x2e),a
    ld  a,e
    out (#0x2f),a
    pop af
#endif

    pop  de
    ret
  __endasm;
}
