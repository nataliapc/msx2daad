/*
=========================================================
	MSX2: Platform dependent API functions
=========================================================
*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "daad_condacts.h"
#include "daad_platform_msx2.h"
#include "daad.h"
#include "vdp.h"
#include "asm.h"


#define ADDR_POINTER_BYTE(X)	(*((uint8_t*)X))
#define ADDR_POINTER_WORD(X)	(*((uint16_t*)X))

#define MODE	0xfafc		/*Flag for screen mode. (1B/R)
								bit 7: 1 = conversion to Katakana; 0 = conversion to Hiragana. (MSX2+~)
								bit 6: 1 if Kanji ROM level 2. (MSX2+~)
								bit 5: 0/1 to draw in RGB / YJK mode SCREEN 10 or 11. (MSX2+~)
								bit 4: 0/1 to limit the Y coordinate to 211/255. (MSX2+~)
								bit 3: 1 to apply the mask in SCREEN 0~3.
								bits 1-2: VRAM size
											00 for 16kB
											01 for 64kB
											10 for 128kB
											11 for 192kB
								bit 0: 1 if the conversion of Romaji to Kana is possible. (MSX2~)*/
#define JIFFY   0xfc9e		// (WORD) Contains value of the software clock, each interrupt of the VDP 
							//        it is increased by 1 (50/60Hz)

// Constants
const char CHARS_MSX[]  = "\xA6\xAD\xA8\xAE\xAF\xA0\x82\xA1\xA2\xA3\xA4\xA5\x87\x80\x81\x9A";	//ª¡¿«»áéíóúñÑçÇüÜ

const char PIC_NAME[] = { '0','0','0','.','I','M','0'+SCREEN, '\0' };	// Picture filename
const char PLT_NAME[] = { '0','0','0','.','P','L','0'+SCREEN, '\0' };	// Palette filename


Z80_registers regs;


//=========================================================
// SYSTEM FUNCTIONS

bool checkPlatformSystem()
{
   	// Check for MSX2 with 128Kb VRAM
	if ((ADDR_POINTER_BYTE(MODE) & 0x06) < 0x04) {
		die("MSX2 with 128Kb VRAM is needed to run!\n");
	}
    return true;
}

uint16_t getFreeMemory()
{
    return ADDR_POINTER_WORD(0x0006)-heap_top+hdr->fileLength;
}

char* getCharsTranslation()
{
	return CHARS_MSX;
}

void setTime(uint16_t time)
{
	ADDR_POINTER_WORD(JIFFY) = time;
}

uint16_t getTime()
{
	return ADDR_POINTER_WORD(JIFFY);
}

//=========================================================
// FILESYSTEM

#if SCREEN==5
	const char FILES[] = "FONT5   RAWDAAD    DDB           ";
#elif SCREEN==6
	const char FILES[] = "FONT6   RAWDAAD    DDB           ";
#elif SCREEN==7
	const char FILES[] = "FONT7   RAWDAAD    DDB           ";
#elif SCREEN==8
	const char FILES[] = "FONT8   RAWDAAD    DDB           ";
 #elif !defined(SCREEN)
	#error "No screen mode defined for output!"
 #endif

const char FILES_BIN[] = "FILES.BIN";


uint16_t loadFile(char *filename, uint8_t *destaddress, uint16_t size)
{
	uint16_t fp = fopen(filename, O_RDONLY);
	uint16_t len;
	char *error;
	if (fp & 0xff00) {
		len = 0;
		if ((uint8_t)fp != 0xd7) {
			explain(error, (uint8_t)fp);
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

	//DMG/FONT
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


//=========================================================
// GRAPHICS (GFX)

#define MAX_PICFILE_READ	2048

#if SCREEN==8
	uint8_t COLOR_INK	=   255;
	uint8_t COLOR_PAPER	=	0;
#elif SCREEN==6
	#define COLOR_INK		3
	#define COLOR_PAPER		0
#else
	#define COLOR_INK		15
	#define COLOR_PAPER		0
#endif

#if SCREEN==8		//SCREEN8 fixed colors
	const uint16_t colorTranslation[] = {	// EGA rgb -> MSX SC8
		0x00, 0x03, 0xc0, 0xc3, 0x18, 0x1b, 0x58, 0xdb, 0x49, 0x6f, 0xe9, 0x5f, 0x5d, 0x5f, 0xfd, 0xff
	};
#else				//Paletted colors
	const uint16_t colorTranslation[] = {	// EGA rgb -> MSX grb
		0x000, //000 black
		0x006, //006 blue
		0x600, //060 green
		0x606, //066 dark cyan
		0x060, //600 red
		0x066, //606 dark purple
		0x260, //620 orange
		0x666, //666 light gray
		0x222, //222 dark gray
		0x337, //337 light blue
		0x722, //272 light green
		0x727, //277 light cyan
		0x272, //722 light red
		0x277, //727 light purple
		0x772, //772 yellow
		0x777  //777 white
	};
#endif


void gfxSetScreen()
{
	//Set Color 15,0,0
	setColor(15, 0, 0);
	//Set SCREEN mode
	regs.Bytes.A = SCREEN;
	BiosCall(0x5f, &regs, REGS_ALL);
	disableInterlacedLines();
	enable212lines();
	enable50Hz();
	disableSPR();
}

void gfxClearLines(uint16_t start, uint16_t lines)
{
	bitBlt(0, 0, 0, start, SCREEN_WIDTH, lines, COLOR_PAPER, 0, CMD_HMMV);
}

void gfxClearScreen()
{
	gfxClearLines(0, 212);
}

void gfxClearWindow()
{
	bitBlt(0, 0, cw->winX*FONTWIDTH, cw->winY*8, (cw->winW+1)*FONTWIDTH, cw->winH*8, COLOR_PAPER, 0, CMD_HMMV);
}

void gfxSetPaperCol(uint8_t col)
{
	#if SCREEN==8
		COLOR_PAPER = colorTranslation[col];
	#else
		setColorPal(COLOR_PAPER, colorTranslation[col]);
	#endif
}

void gfxSetInkCol(uint8_t col)
{
	#if SCREEN==8
		COLOR_INK = colorTranslation[col];
	#else
		setColorPal(COLOR_INK, colorTranslation[col]);
	#endif
}

void gfxSetBorderCol(uint8_t col)
{
	gfxSetPaperCol(col);
}

void gfxPutChPixels(uint8_t c)		// c = ASCII-0x10
{
	uint16_t sx = (c*8)%SCREEN_WIDTH,
	         sy = (c/(SCREEN_WIDTH/8)*FONTHEIGHT)+256,
			 dx = (cw->cursorX+cw->winX)*FONTWIDTH,
			 dy = (cw->cursorY+cw->winY)*FONTHEIGHT;
	#if SCREEN==8
		bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_HMMM);
		//TODO Add INK color
		//bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_INK, 0, CMD_LMMV|LOG_TAND);
		bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_PAPER, 0, CMD_LMMV|LOG_TOR);
	#else
		bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_LMMM|LOG_IMP);
	#endif
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
#ifdef VERBOSE
printf("\nNEWLINE\n");
#endif
	} else 
	if (!(c==' ' && cw->cursorX==0)) {
#ifdef VERBOSE
printf("%c", c);
#endif
		c -= 16;
		gfxPutChPixels(offsetText + c);
		cw->cursorX++;
		if (cw->cursorX >= cw->winW) {
			cw->cursorX = 0;
			cw->cursorY++;
		}
	}
	if (cw->cursorY >= cw->winH) {
		gfxScrollUp();
		cw->cursorX = 0;
		cw->cursorY--;
	}
	/*if (printedLines >= cw->winH-1) {	// Must be show "More..." SYS32?
		char *oldTmpMsg = tmpMsg;
		tmpMsg += TEXT_BUFFER_LEN/2;
		printSystemMsg(32);
		waitForTimeout(TIME_MORE);
		getchar();
		tmpMsg = oldTmpMsg;
		cw->cursorX = 0;
		cw->cursorY++;
		printedLines=0;
	}*/
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
	if (cw->winH > 1) {
		bitBlt(0, (cw->winY+1)*8, cw->winX*FONTWIDTH, cw->winY*8, cw->winW*FONTWIDTH, (cw->winH-1)*8, COLOR_PAPER, 0, CMD_YMMM);
	}
	bitBlt(0, 0, cw->winX*FONTWIDTH, (cw->winY+cw->winH-1)*8, cw->winW*FONTWIDTH, 8, COLOR_PAPER, 0, CMD_HMMV);
}

bool gfxPicturePrepare(uint8_t location)
{
	char *pic = PIC_NAME, *plt = PLT_NAME;

	pic[2] = plt[2] = location%10 + '0';
	location /= 10;
	pic[1] = plt[1] = location%10 + '0';
	location /= 10;
	pic[0] = plt[0] = location%10 + '0';

	return fileexists(PIC_NAME);
}

uint16_t chunkSize;
uint16_t outSize;

void gfxPictureShow()
{
	uint16_t fp;
	uint8_t i = cw->winH;
	char *buffer = heap_top;
	char *buffer2 = buffer + MAX_PICFILE_READ;

	do_CLS();	//TODO clear only lines read from file

	#if SCREEN!=8
		fp = fopen(PLT_NAME, O_RDONLY);
		if (!(fp&0xff00)) {
			fread(buffer, 32, fp);
			setPalette(buffer);
			fclose(fp);
		}
	#endif

	fp = fopen(PIC_NAME, O_RDONLY);
	if (fp<0xff00) {
		uint16_t size;
		uint32_t posVRAM = 0;
		setVDP_Write(0);
		do {
			size = fread(&chunkSize, 2, fp);
			if (!(size & 0xff00)) {
				fread(&outSize, 2, fp);
				fread(buffer, chunkSize, fp);
#ifdef DEBUG
printf("size:%u\tchunk:%u\toutSize:%u\n",size,chunkSize,outSize);
#endif
				pletter2ram(buffer, buffer2);
				__asm
					di
					push hl
					push bc
					ld hl,(#_heap_top)
					ld de,#MAX_PICFILE_READ
					add hl,de
					ld de,(#_outSize)
					ld bc,#0x0098
				0001$:
					otir
					dec d
					jr nz, 0001$
					ld b,e
					otir
					pop bc
					pop hl
					ei
				__endasm;
			}
		} while (!(size & 0xff00));
/*
		setVDP_Write(0x00000);
		while (fread(buffer, MAX_PICFILE_READ, fp)==MAX_PICFILE_READ) {
			__asm
				push hl
				push bc
				ld hl,(#_heap_top)
				ld bc,#0x0098
				otir
				otir
				otir
				otir
				otir
				otir
				otir
				otir
				pop bc
				pop hl
			__endasm;
		}
*/
		fclose(fp);
	}
}

void sfxSound(uint8_t value1, uint8_t value2)
{
	value1, value2;
	//TODO
	BiosCall(0xc0, NULL, REGS_NONE);
}
