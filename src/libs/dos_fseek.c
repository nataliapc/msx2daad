#include "dos.h"
#include "utils.h"


uint32_t fseek (char fp, uint32_t offset, char origin) __naked
{
  fp;
  offset;
  origin;
#ifdef MSXDOS2

  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld b,0(ix)
    ld l,1(ix)
    ld h,2(ix)
    ld e,3(ix)
    ld d,4(ix)
    ld a,5(ix)
    ld c, SEEK
    DOSCALL

    or a
    jr z, seek_noerror$
    ld de, #0xffff
    ld h, #0xff
    ld l, a
  seek_noerror$:
    pop ix
    ret
  __endasm;

#else //MSXDOS1 (FCB)

    die("fseek fail!");

#endif
}
