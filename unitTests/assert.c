#include "../include/dos.h"
#include "assert.h"


void _ASSERT(bool succeedCondition, const char *failMsg, char *file, char *func, int line)
{
	if (!succeedCondition) {
		cprintf("### Assert failed at: %s :: %s :: %d\n\r    by \"%s\"\n\r\x07", file, func, line, failMsg);
		exit0();
	}
}

void _FAIL(const char *failMsg, char *file, char *func, int line)
{
	cprintf("Fail by '%s'\n\r  at %s :: %s :: %d\n\r\x07", failMsg, file, func, line);
	exit0();
}

void _SUCCEED(char *file, char *func)
{
	cprintf("%s:%s ... OK\n\r", file, func);
}
