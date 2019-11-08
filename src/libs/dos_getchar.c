#include "dos.h"


int getchar(void) __naked {
#ifdef MSX2
  __asm
    push ix

    ld c,#INNOE
    DOSCALL
    ld h, #0x00

    pop ix
    ret
  __endasm;
#endif
#ifdef CPM
  __asm
    push ix

    ld c,#CONIN
    DOSCALL
    ld h, #0x00

    pop ix
    ret
  __endasm;
#endif
}
