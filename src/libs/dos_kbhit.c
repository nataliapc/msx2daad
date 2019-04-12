#include "dos.h"


int kbhit(void) __naked
{
  __asm
	ld c, CONST
	DOSCALL
	ret
  __endasm;
}
