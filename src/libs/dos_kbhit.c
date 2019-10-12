#include "dos.h"


int kbhit(void) __naked
{
  __asm
	ld c, CONST
	DOSCALL
	ld h,#0
	ret
  __endasm;
}
