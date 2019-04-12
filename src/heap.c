
#include <stdint.h>
#include "heap.h"


void *malloc(uint16_t size) {
	uint8_t *ret = heap_top;
	heap_top += size;
	return (void *) ret;
}

void free(uint16_t size) {
	heap_top -= size;
}