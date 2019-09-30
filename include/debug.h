
#ifndef __OPENMSX_DEBUG_H__
#define __OPENMSX_DEBUG_H__


// To debug type at openmsx console:
//    debug set_watchpoint write_io 0x18
#define DEBUG_BREAKPOINT \
	out (0x18),a

#define call_BORDERCOLOR \
	out (0x99),a\
	ld a,#0x87\
	out (0x99),a\


#endif  // __OPENMSX_DEBUG_H__
