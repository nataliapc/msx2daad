#include "dos.h"


uint8_t kbhit(void) __naked
{
	__asm
		ld c,#CONST
		DOSCALL
		ret
	__endasm;
}
