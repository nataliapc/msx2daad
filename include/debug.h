
#ifndef __OPENMSX_DEBUG_H__
#define __OPENMSX_DEBUG_H__


// To debug type at openmsx console:
//    debug set_watchpoint write_io 0x18
#define DEBUG_BREAKPOINT __asm out (0x18),a __endasm;

#define DEBUG_BORDERCOLOR __asm \
	ld  a,X \
	out (0x99),a \
	ld  a,\#0x87 \
	out (0x99),a \
__endasm;

#define DEBUG_PRINTNUM __asm \
	push af \
	ld  a,#0x62 \
	out (0x2e),a \
	ld  a,X \
	out (0x2f),a \
	pop af \
__endasm;


#endif  // __OPENMSX_DEBUG_H__
