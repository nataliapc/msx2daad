#include "dos.h"


char dosver(void) __naked
{
	__asm
		push ix

		ld b,  #0x5A			; magic numbers to detect Nextor
		ld hl, #0x1234
		ld de, #0xABCD
		ld ix, #0

		ld c,#DOSVER
		call 0xF37D				; BDOS (upper memory DOSCALL)

		or  a
		jr  z,check_dos1$
		xor a					; BDOS (upper memory DOSCALL)
		jr  ret_version$

	check_dos1$:
		ld  a,b					; B<2 --> MSX-DOS 1
		cp  #2
		jr  nc,check_dos2nextor$
		ld  b,#VER_MSXDOS1x		; A = VER_MSXDOS1x (is MSX-DOS 1)
		jr  ret_version$

	check_dos2nextor$:
		push ix					; Nextor: IXh must contain '1'
		pop  hl
		ld   a, h
		dec a
		jr  z,is_nextor$
		ld  a,#VER_MSXDOS2x		; A = VER_MSXDOS2x (is MSXDOS 2)
		jr  ret_version$

	is_nextor$:
		ld  a,#VER_NextorDOS	; A = VER_NextorDOS (is NextorDOS)

	ret_version$:
		pop ix
		ret
	__endasm;
}

