#include <string.h>
#include "dos.h"

uint16_t fputs(char *str)
{
	uint16_t len = strlen(str);
	return fwrite(str, len);
}