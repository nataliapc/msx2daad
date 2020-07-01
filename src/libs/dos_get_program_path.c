#include "dos.h"
#include "heap.h"


char* get_program_path()
{
	char *result = malloc(MAX_PATH);
	if (get_env("PROGRAM", result, MAX_PATH)) {
		free(MAX_PATH);
		return NULL;
	}
	return result;
}
