#include "dos.h"


char fileexists(char *filename)
{
	uint16_t fp = fopen(filename, O_RDONLY);
	if (fp<0xff00) {
		fclose(fp);
		return 1;
	}
	return 0;
}