#include "dos.h"


uint16_t remove(char *file) __naked
{
	file;		// HL
	__asm
		push ix
		ex de, hl

		ld c,#DELETE
		DOSCALL

		ld d, #0xff
		ld e, a

		pop ix
		ret
	__endasm;
}