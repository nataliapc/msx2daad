#include "dos.h"

/*
     Parameters:    C = 41H (_FNEXT) 
                   IX = Pointer to fileinfo block from previous find first function. 
     Results:       A = Error. If there are no more matching entries then a NOFIL error is returned
                 (IX) = Filled in with next matching entry 
*/
uint16_t findnext(FFBLK *ffblk ) __naked
{
	ffblk;
	__asm
		push ix
		ld   ix,#4
		add  ix,sp

		ld l,0(ix)			; pathname
		ld h,1(ix)
		push hl
		pop  ix

		ld c,#FNEXT
		DOSCALL

		or a
		ld h, #0xff
		ld l, a
		jr z, fnext_noerror$
		jr fnext_error$
	fnext_noerror$:
		inc h
	fnext_error$:
		pop  ix
		ret
	__endasm;
}
