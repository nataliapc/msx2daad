/*
=========================================================
	PC TXT: Platform dependent API functions
=========================================================
*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/stat.h>
#include "daad_condacts.h"
#include "daad.h"


// Constants
const char CHARS_PC[]  = "\xA6\xAD\xA8\xAE\xAF\xA0\x82\xA1\xA2\xA3\xA4\xA5\x87\x80\x81\x9A";	//ª¡¿«»áéíóúñÑçÇüÜ

const char PIC_NAME[] = { '0','0','0','.','I','M','0'+SCREEN, '\0' };	// Picture filename
const char PLT_NAME[] = { '0','0','0','.','P','L','0'+SCREEN, '\0' };	// Palette filename

long long timeZero;


//=========================================================
// SYSTEM FUNCTIONS

bool checkPlatformSystem()
{
	//TODO
    return true;
}

uint16_t getFreeMemory()
{
	//TODO
    return 65535;
}

char* getCharsTranslation()
{
	return CHARS_PC;
}

long long current_timestamp() {
    struct timeval te; 
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // calculate milliseconds
    return milliseconds;
};

void setTime(uint16_t time)
{
	timeZero = current_timestamp();
}

uint16_t getTime()
{
	return current_timestamp() - timeZero;
}

//=========================================================
// FILESYSTEM

const char FILES[] = "FONT5   RAWDAAD    DDB           ";
const char FILES_BIN[] = "FILES.BIN";


uint16_t loadFile(char *filename, uint8_t *destaddress, uint16_t size)
{
	FILE *fp = fopen(filename, O_RDONLY);
	uint32_t len;

	while (!feof(fp) && size) {
		len = fread(destaddress, size, fp);
		size -= len;
		destaddress += len;
	}
	fclose(fp);

	return size;
}

uint16_t fileSize(char *filename)
{
	struct stat st;
	stat(filename, &st);
	return (uint16_t)st.st_size;
}

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

void loadFilesBin()
{
	uint16_t size = 0;
	char aux[256];
	char *buff=aux+33;	// [13]

	loadFile(FILES_BIN, FILES, 33);

	//DDB
	compactFilename(buff, &FILES[11]);
	size = fileSize(buff);
	ddb = malloc(size);
	loadFile(buff, ddb, size);
}


//=========================================================
// GRAPHICS (GFX)

void gfxSetScreen()
{
	//TODO
}

void gfxClearLines(uint16_t start, uint16_t lines)
{
	//TODO
}

void gfxClearScreen()
{
	//TODO
}

void gfxClearWindow()
{
	//TODO
}

void gfxSetPaperCol(uint8_t col)
{
	//TODO
}

void gfxSetInkCol(uint8_t col)
{
	//TODO
}

void gfxSetBorderCol(uint8_t col)
{
	//TODO
}

void gfxPutCh(char c)
{
	switch (c) {
		case 11:		// \b
			do_CLS(); return;
		case 12:		// \k
			do_INKEY(); return;
		case 14:		// \g
			offsetText = 128; return;
		case 15:		// \t
			offsetText = 0; return;
	}
	if (c=='\r') {
		cw->cursorX = 0;
		cw->cursorY++;
		putchar('\n');
#ifdef VERBOSE
printf("\nNEWLINE\n");
#endif
	} else 
	if (!(c==' ' && cw->cursorX==0)) {
#ifdef VERBOSE
printf("%c", c);
#endif
		putchar(offsetText + c);
		cw->cursorX++;
		if (cw->cursorX >= cw->winW) {
			putchar('\n');
			cw->cursorX = 0;
			cw->cursorY++;
		}
	}
}

void gfxPuts(char *str)
{
	char *aux = NULL, c;

	while ((c = *str)) {
		if (c==' ' || !aux) {
			aux = str+1;
			while (*aux && *aux!=' ' && *aux!='\n') {
				aux++;
			}
			if (cw->cursorX+(aux-(str+1)) >= cw->winW) {
				if (c==' ') c = '\r'; else do_NEWLINE();
			}
		}
		gfxPutCh(c);
		str++;
	}
}

void gfxPutsln(char *str)
{
	gfxPuts(str);
	gfxPutCh('\r');
}

void gfxScrollUp()
{
	//TODO
}

bool gfxPicturePrepare(uint8_t location)
{
	//TODO
	return false;
}

void gfxPictureShow()
{
	//TODO
}

void sfxSound(uint8_t value1, uint8_t value2)
{
	//TODO
}
