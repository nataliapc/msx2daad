#include "dos.h"


uint8_t kbhit(void) __naked
{
	__asm
		push bc

		ld c,#CONST
		DOSCALL
		
		pop bc
		ret
	__endasm;
}
