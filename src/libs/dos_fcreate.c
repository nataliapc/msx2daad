#include "dos.h"


uint16_t fcreate(char *fn, char mode, char attributes) __naked
{
  fn;
  mode;
  attributes;
#ifdef MSXDOS2

  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld e,0(ix)
    ld d,1(ix)
    ld a,2(ix)
    ld b,3(ix)
    ld c, #CREATE
    DOSCALL

    or a
    jr z, create_noerror$
    ld h, #0xff
    ld l, a
    jp create_error$
  create_noerror$:
    ld h, #0x00
    ld l, b
  create_error$:
    pop ix
    ret
  __endasm;

#else //MSXDOS1 (FCB)

  __asm
    call    dos_initializeFCB

    push    ix                    ; Put filename in DE
    ld      ix,#4
    add     ix,sp
    ld      e,0(ix)
    ld      d,1(ix)

    call    dos_copyFilenameToFCB

    ld      de,#SYSFCB
    ld      c,#FMAKE              ; Call FOPEN Bios function
    DOSCALL

    or a                          ; Return result
    jr z,create_noerror$
    ld hl, #0xffd7                ; Set return value ERR
    jp create_cont$
  create_noerror$:
    ld hl,#0x0000                 ; Set return value OK

  create_cont$:
    pop ix
    ret
  __endasm;

#endif
}
