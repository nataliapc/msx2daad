#include "dos.h"
#include "utils.h"
#include <stdio.h>


uint16_t filesize(char *fn) __naked
{
	fn;		// HL
	__asm
		push ix
		push hl
		call dos_initializeFCB

		pop  de
		call dos_copyFilenameToFCB

		ld   de,#SYSFCB
		ld   c,#FOPEN                 ; Call FOPEN Bios function
		DOSCALL

		or a                          ; Return result
		jr z,fsize_noerror$
		ld hl, #0xffd7                ; Set return value ERR
		jp fsize_cont$
	fsize_noerror$:
		ld hl,(SYSFCB+16)             ; Set return value OK with filesize
		push hl
		call _fclose
		pop  de

	fsize_cont$:
		pop ix
		ret
	__endasm;
}
