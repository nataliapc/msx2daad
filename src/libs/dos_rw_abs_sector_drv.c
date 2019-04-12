#include "dos.h"


char _abs_sector_drv(uint8_t drv, uint32_t startsec, uint8_t nsec, uint8_t doscall) __naked {
  doscall, drv, nsec, startsec;
  __asm
    push ix
    ld ix,#4
    add ix,sp

    ld a,0(ix)
    ld e,1(ix)
    ld d,2(ix)
    ld l,3(ix)
    ld h,4(ix)
    ld b,5(ix)
    ld c,6(ix)
    DOSCALL

    ld h, #0
    ld l, a

    pop ix
    ret
  __endasm;
}

char read_abs_sector_drv(uint8_t drv, uint32_t startsec, uint8_t nsec) {
  return _abs_sector_drv(drv, startsec, nsec, RDDRV);
}

char write_abs_sector_drv(uint8_t drv, uint32_t startsec, uint8_t nsec) {
  return _abs_sector_drv(drv, startsec, nsec, WRDRV);
}
