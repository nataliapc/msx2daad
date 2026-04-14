/*
	SDCC cross-version compatibility helpers for the z80 port.

	SDCC 4.2.0+ switched the default z80 calling convention from __sdcccall(0)
	(params on stack) to __sdcccall(1) (params in registers). Naked functions
	with inline asm that read params via `ix+n`/`pop`/`push` still expect the
	old ABI, so we pin them with SDCC_STACKCALL.

	The __sdcccall attribute was introduced in SDCC 4.1.12. On older SDCC the
	macro expands to empty (those builds already use sdcccall(0) by default).
*/
#ifndef __SDCC_COMPAT_H__
#define __SDCC_COMPAT_H__

#define SDCC_STACKCALL __sdcccall(0)

#endif //__SDCC_COMPAT_H__
