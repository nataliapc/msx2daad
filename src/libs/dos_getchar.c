#include "dos.h"


int getchar(void) __naked {
#ifdef MSX2
  __asm

    ld c,#INNOE
    push ix
    DOSCALL
    pop  ix
    ld h, #0x00

    ret
  __endasm;
#endif
#ifdef CPM
  __asm

    ld c,#CONIN
    DOSCALL
    ld h, #0x00

    ret
  __endasm;
#endif
}
