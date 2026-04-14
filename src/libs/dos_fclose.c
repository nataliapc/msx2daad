#include "dos.h"

#include <stdio.h>


uint8_t fclose() __naked
{
	__asm
		ld   de,#SYSFCB
		ld   c,#FCLOSE
		push ix
		DOSCALL
		pop  ix

		ret
	__endasm;
}
