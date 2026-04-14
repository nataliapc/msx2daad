#include "dos.h"


uint16_t fcreate(char *fn) __naked
{
	fn;			// HL
	__asm
		push ix
		push hl
		call dos_initializeFCB

		pop  de
		call dos_copyFilenameToFCB

		ld   de,#SYSFCB
		ld   c,#FMAKE                 ; Call FOPEN Bios function
		DOSCALL

		or a                          ; Return result
		jr z,create_noerror$
		ld de, #0xffd7                ; Set return value ERR
		jp create_cont$

	create_noerror$:
		ld de,#0x0000                 ; Set return value OK

	create_cont$:
		pop ix
		ret
	__endasm;
}
