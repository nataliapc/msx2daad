#include "asm.h"
#include "dos.h"
#include "utils.h"


extern Z80_registers regs;


void die(const char *s)
{
	regs.Bytes.A = 0;
	BiosCall(0x5f, &regs, REGS_ALL);

	cputs(s);
	exit0();
}