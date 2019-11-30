#include "dos.h"

#include <stdio.h>


uint16_t fopen(char *fn, char mode) __naked
{
  fn, mode;

#ifdef MSXDOS2

  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld e,0(ix)
    ld d,1(ix)
    ld a,2(ix)
    ld c, #OPEN
    DOSCALL

    or a
    jr z, open_noerror$
    ld h, #0xff
    ld l, a
    jp open_error$
  open_noerror$:
    ld h, #0x00
    ld l, b
  open_error$:
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
    ld      c,#FOPEN              ; Call FOPEN Bios function
    DOSCALL

    or a                          ; Check result
    jr z,open_noerror$
    ld hl, #0xffd7                ; Set return value ERR
    jp open_cont$
  open_noerror$:
    ld hl,#0x0000                 ; Set return value OK

  open_cont$:
    pop ix
    ret
  __endasm;

#endif
}
