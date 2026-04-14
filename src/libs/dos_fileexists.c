#include "dos.h"


char fileexists(char *filename)
{
	uint16_t fp = fopen(filename);
	if (fp<0xff00) {
		fclose();
		return 1;
	}
	return 0;
}