#include "dos.h"


void cputs(char *s)
{
	while (*s != 0) {
		putchar(*s);
		s++;
	}
	putchar('\n');
}
