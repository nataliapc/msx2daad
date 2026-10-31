#include "dos.h"
#include "utils.h"
#if SCREEN==16
	#include "gfx9000.h"
#endif


void die(const char *s)
{
#if SCREEN==16
	// GFX9000: blank its display, so the MSX VDP (text mode below) is what is
	// shown when the program ends
	g9kDisplayOff();
#endif
	// Set SCREEN 0 (text mode)
	__asm
		xor  a
		ld   iy,(#0xfcc1-1)		; EXPTBL
		ld   ix,#0x5f
		call 0x1c				; CALSTL
	__endasm;

	cputs(s);

#if SCREEN==16
	// Back to MSX-DOS (the other builds stop here, as they always have)
	exit0();
#else
	__asm
		di
		halt
	__endasm;
	//exit0();
#endif
}