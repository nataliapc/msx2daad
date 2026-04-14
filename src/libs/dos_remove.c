#include "dos.h"


uint16_t remove(char *file) __naked
{
	file;		// HL
	__asm
		push ix
		push hl
		call dos_initializeFCB

		pop  de
		call dos_copyFilenameToFCB

		ld   de,#SYSFCB
		ld   c,#FDEL
		DOSCALL

		ld de, #0					; Default return value OK
		or a
		jr z,remove_cont$

		dec d						; Return ERR result
		ld e, a

	remove_cont$:
		pop ix
		ret
	__endasm;
}