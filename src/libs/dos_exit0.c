#include "dos.h"


void exit0() __naked {

  __asm
    ld c,#TERM0
    DOSCALL
    ret
  __endasm;

}
