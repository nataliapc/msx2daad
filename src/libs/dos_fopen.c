#include "dos.h"

#include <stdio.h>


uint16_t fopen(char *fn) __naked
{
	fn;			// HL
	__asm
		push ix
		push hl
		call dos_initializeFCB

		pop  de
		call dos_copyFilenameToFCB

		ld   de,#SYSFCB
		ld   c,#FOPEN                 ; Call FOPEN Bios function
		DOSCALL

		or a                          ; Check result
		jr z,open_noerror$
		ld de, #0xffd7                ; Set return value ERR
		jp open_cont$

	open_noerror$:
		ld de,#0x0000                 ; Set return value OK

	open_cont$:
		pop ix
		ret
	__endasm;
}
