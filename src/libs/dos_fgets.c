#include "dos.h"


uint16_t fgets(char *str, uint16_t n, uint16_t fh)
{
	char *p = str;
	uint16_t ret = 0;

	n++;
	while (n) {
		ret = fread(p, 1, fh);
		if (ret>=0xff00 || *p=='\n') break;	// CR
		if (*p!='\r') { p++; n--; }			// LF
	}
	if (ret==0xff00|ERR_EOF && p!=str) ret = 0;
	*p = '\0';
	return ret;
}
