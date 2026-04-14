#include "dos.h"

#include <stdio.h>


uint16_t fwrite(char* buf, uint16_t size) __naked
{
	buf;		// HL
	size;		// DE
	__asm
		push ix
		ex   de,hl

		push  hl
		ld    c,#SETDTA               ; Set Disk transfer address (DTA)
		DOSCALL

		ld hl,#1                      ; Set FCB Record size to 1 byte
		ld (#SYSFCB+14),hl
		pop   hl

		ld    de,#SYSFCB
		ld    c,#WRBLK
		DOSCALL

		ld d,#0
		ld e,a
		or a
		jr z,write_noerror$
		ld de, #0xffff

		write_noerror$:
		pop ix
		ret
	__endasm;
}

