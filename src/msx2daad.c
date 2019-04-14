#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "msx_const.h"
#include "asm.h"
#include "dos.h"
#include "utils.h"
#include "vdp.h"
#include "heap.h"
#include "daad.h"


#if SCREEN==6
	const char FILES[] = "FONT6   RAWDAAD    DDB           ";
#endif
#if SCREEN==7
	const char FILES[] = "FONT7   RAWDAAD    DDB           ";
#endif
#if SCREEN==8
	const char FILES[] = "FONT8   RAWDAAD    DDB           ";
#endif
#if SCREEN==5 || !defined(SCREEN)
	const char FILES[] = "FONT5   RAWDAAD    DDB           ";
	#warning "No screen mode defined for output, assumed SC5."
#endif

const char FILES_BIN[] = "FILES.BIN";



void compactFilename(char *dst, char *src)
{
	char pos=0;
	memset(dst, 0, 13);
	memcpy(dst, src, 8);
	while (src[pos]!=' ' && pos<8) pos++;
	dst[pos++]='.';
	memcpy(&dst[pos], &src[8], 3);
	while (dst[pos]!=' ' && pos<13) pos++;
	dst[pos]='\0';
}

uint16_t loadFile(char *filename, uint8_t *destaddress, uint16_t size)
{
	uint16_t fp = fopen(filename, O_RDONLY);
	uint16_t len;
	char *error;
	if (fp & 0xff00) {
		len = 0;
		if ((uint8_t)fp != 0xd7) {
			explain(error, (uint8_t)fp);
			printf("Error %u\n",(uint8_t)fp);
			die(error);
		}
	} else {
		len = fread(destaddress, size, fp);
		fclose(fp);
	}
	return len;
}

uint16_t fileSize(char *filename)
{
	uint16_t fp = fopen(filename, O_RDONLY);
	uint32_t len;
	char *error;
	if (fp & 0xff00) {
		len = 0;
		if ((uint8_t)fp != 0xd7) {
			explain(error, (uint8_t)fp);
			die(error);
		}
	} else {
		len = fseek(fp, 0, SEEK_END);
		fclose(fp);
	}
	return (uint16_t)len;
}

void loadFilesBin()
{
	uint16_t size = 0;
	char aux[256];
	char *buff=aux+33;	// [13]

	loadFile(FILES_BIN, FILES, 33);

	//DMG
	compactFilename(buff, &FILES[0]);
	size = fileSize(buff);
	loadFile(buff, heap_top, size);
	unRLE_vram(heap_top, VRAM_PAGE1);

	//DDB
	compactFilename(buff, &FILES[11]);
	size = fileSize(buff);
	ddb = malloc(size);
	loadFile(buff, ddb, size);
}


int main()
{
	// Check for MSX2 with 128Kb VRAM
	if ((ADDR_POINTER(ADDR_MODE) & 0x06) < 0x04) {
		die("MSX2 with 128Kb VRAM is needed to run!\n");
	}

	gfxSetScreen();

	//Load files (DDB, Font, ...)
	loadFilesBin();
	if (!initDDB()) {
		die("Bad DDB file!\n");
		exit(1);
	}

	mainLoop();

	die("Done");
}
