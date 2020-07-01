#include "dos.h"

/*
     Parameters:    C = 40H (_FFIRST) 
                   DE = Drive/path/file ASCIIZ string or fileinfo block pointer
                   HL = filename ASCIIZ string (only if DE = fileinfo pointer)
                    B = Search attributes
                   IX = Pointer to new fileinfo block
     Results:       A = Error
                 (IX) = Filled in with matching entry
*/
uint16_t findfirst(const char *pathname, FFBLK *ffblk, uint8_t attrib) __naked
{
	pathname, ffblk, attrib;
	__asm
		push ix
		ld   ix,#4
		add  ix,sp

		ld e,0(ix)			; pathname
		ld d,1(ix)
		ld l,2(ix)			; ffblk
		ld h,3(ix)
		ld b,4(ix)			; attrib
		push hl
		pop  ix

		ld c,#FFIRST
		DOSCALL

		ld h, #0xff
		ld l, a
		or a
		jr z, ffirst_noerror$
		jr ffirst_error$
	ffirst_noerror$:
		inc h
	ffirst_error$:
		pop  ix
		ret
	__endasm;
}