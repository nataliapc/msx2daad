#include <string.h>
#include "dos.h"

uint16_t fputs(char *str, char fp)
{
	uint16_t len = strlen(str);
	return fwrite(str, len, fp);
}