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
#include "dos.h"

//=========================================================
// Constants
//=========================================================

//Optional file containing changes to default filename for FONT and DDB files.
const char FILES_BIN[] = "FILES.BIN";

//Default FILES.BIN content (overwrited if a FILES.BIN exists)
const char FILES[] = "FONT.IM"STRINGIFY(SCREEN)"\0    DAAD.DDB\0    ";

//Buffer to fill with the image to load
const char IMG_NAME[] = "000.IM"STRINGIFY(SCREEN)"\0     ";	// Image filename

//MSX special chars (DAAD chars 16-31)
const char CHARS_MSX[]  = "\xA6\xAD\xA8\xAE\xAF\xA0\x82\xA1\xA2\xA3\xA4\xA5\x87\x80\x81\x9A";	//ª¡¿«»áéíóúñÑçÇüÜ

//Function keys redefinitions
#ifdef LANG_ES
	const char FUNC_KEYS[5][9]  = { "mirar \0 ","coger \0 ","dejar \0 ","buscar \0","lanzar \0" };
#endif
#ifdef LANG_EN
	const char FUNC_KEYS[5][10]  = { "examine \0","get \0    ","drop \0   ","search \0 ","throw \0  " };
#endif

//=========================================================
// Variables
//=========================================================

// Start position in VRAM where draw a image file
uint32_t posVRAM;

#ifdef MAPPER_CACHE
	uint8_t max_mapper_pages;
#endif

//=========================================================
// SYSTEM FUNCTIONS
//=========================================================

/*
 * Function: checkPlatformSystem
 * --------------------------------
 * Check the system looking for incompatibilities.
 * 
 * @return			Return a bool if all is OK or not.
 */
bool checkPlatformSystem()
{
   	// Check for MSX2 with at least 128Kb VRAM
	if ((ADDR_POINTER_BYTE(MODE) & 0x06) < 0x04) {
		die("MSX2 with 128Kb VRAM is needed to run!\n");
	}
	// If compiled with MSXDOS2 lib check for DOS version
	#ifdef MSXDOS2
		if (dosver()<VER_MSXDOS2x) {
			die("MSXDOS 2.x or better is needed!\n");
		}
	#endif
	// If compiled with RAM Mapper Cache initialize it
	#ifdef MAPPER_CACHE
		initializeMapper();
		initializeCache();
	#endif
    return true;
}

#ifdef MAPPER_CACHE
void initializeMapper();
void setMapperPage(uint8_t page);
void restoreMapperPage2();
void initializeCache();
uint8_t getEmptyCacheEntry();
uint8_t getCacheEntryFromLoc(uint8_t loc);
uint16_t getEmptyCacheFragment();
void releaseCacheEntry(uint8_t idx);
uint16_t getFreeCacheMemory();

void initializeMapper() __naked
{
	__asm
		xor a
		ld  hl,#0x8000
		ld  a,#4				; Set first byte of pages with their number
	loop0$:
		out (SET_RAMSEG_P2),a
		ld  (hl),a
		inc a
		jr  nz,loop0$

		ld  a,#4				; Check if the page exists reading their number
		ld  ix,#_max_mapper_pages
		ld  (ix),#0
	loop1$:
		out (SET_RAMSEG_P2),a
		cp  (hl)
		jr  nz,cont0$
		inc (ix)
	cont0$:
		inc a
		jr  nz,loop1$

		ld  a,#1				; Restore page 2
		out (SET_RAMSEG_P2),a
		ret
	__endasm;
}

void setMapperPage(uint8_t page) __naked
{
	page;
	__asm
		pop  af
		pop  bc
		push bc
		push af

		ld   a,c
		out  (SET_RAMSEG_P2),a
		ret
	__endasm;
}

void restoreMapperPage2() __naked
{
	__asm
		ld   a,#1
		out  (SET_RAMSEG_P2),a
		ret
	__endasm;
}

#define CACHE_MAX       20
#define CACHE_FRAGMENTS 28
typedef struct {
	uint8_t location;
	uint8_t uses;
	uint8_t pages[CACHE_FRAGMENTS];
	uint8_t fragments[CACHE_FRAGMENTS];
} CACHE_ENTRY;

CACHE_ENTRY *cacheIndex;
uint8_t *fragmentsUsed;

void initializeCache()
{
	cacheIndex = malloc(sizeof(CACHE_ENTRY)*CACHE_MAX);
	fragmentsUsed = malloc(max_mapper_pages);
	memset(cacheIndex, 0, sizeof(CACHE_ENTRY)*CACHE_MAX + max_mapper_pages);
}

uint8_t getEmptyCacheEntry()
{
	uint8_t idx = 255, uses = 255;

	for (uint8_t i=0; i<CACHE_MAX; i++) {
		if (cacheIndex[i].uses < uses) {
			idx = i;
			uses = cacheIndex[i].uses;
		}
	}
	releaseCacheEntry(idx);
	return idx;
}

uint8_t getCacheEntryFromLoc(uint8_t loc)
{
	for (uint8_t i=0; i<CACHE_MAX; i++) {
		if (cacheIndex[i].location == loc) return i;
	}
	return 255;
}

uint16_t getEmptyCacheFragment()
{
	for (int i; i<max_mapper_pages; i++) {
		if (fragmentsUsed[i]!=0x0f) break;
	}
	if (i==max_mapper_pages) return 0xffff;
	//TODO
}

void releaseCacheEntry(uint8_t idx)
{
	uint8_t page, frag;
	for (uint8_t i=0; i<CACHE_FRAGMENTS; i++) {
		page = cacheIndex[idx].pages[i];			// range [0...255]
		frag = cacheIndex[idx].fragments[i];		// range [0...8]	2k blocks
		fragmentsUsed[page] &= ~(1<<frag);
	}
	memset(&cacheIndex[idx], 0, sizeof(CACHE_ENTRY));
}

uint16_t getFreeCacheMemory()
{
	uint16_t size = 0;
	for (uint8_t i=0; i<max_mapper_pages; i++) {
		for (uint8_t m=0b10000000; m!=0; m>>=1) {
			if ((fragmentsUsed[i] & m)==0) size ++;
		}
	}
	return size;
}
#endif

/*
 * Function: getFreeMemory
 * --------------------------------
 * Return the free TPA in bytes.
 * 
 * @return			Return the free memory in bytes.
 */
uint16_t getFreeMemory()
{
    return ADDR_POINTER_WORD(0x0006)-heap_top+hdr->fileLength;
}

/*
 * Function: getCharsTranslation
 * --------------------------------
 * Return the MSX chars for DAAD chars range 16-31.
 * 
 * @return			Return the MSX chars for DAAD chars range 16-31.
 */
char* getCharsTranslation()
{
	return CHARS_MSX;
}

/*
 * Function: setTime
 * --------------------------------
 * Change the system time.
 * 
 * @param time   	System time in 1/50 sec fragments.
 * @return			none.
 */
void setTime(uint16_t time)
{
	ADDR_POINTER_WORD(JIFFY) = time;
}

/*
 * Function: getTime
 * --------------------------------
 * Return the system time.
 * 
 * @return			Return the system time in 1/50 sec fragments.
 */
uint16_t getTime()
{
	return ADDR_POINTER_WORD(JIFFY);
}

void waitForPrompt()
{
}

//=========================================================
// FILESYSTEM
//=========================================================

/*
 * Function: loadFile
 * --------------------------------
 * Change the system time.
 * 
 * @param filename 		File name to read.
 * @param destaddress 	Destination address.
 * @param size			Size to read.
 * @return				Return >= 0xff00 if error.
 */
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

/*
 * Function: loadFilesBin
 * --------------------------------
 * Load optional file containing alternate names for 
 * FONT and DDB files, and read them.
 * 
 * @return			none.
 */
void loadFilesBin()
{
	char *aux;
	uint16_t size = 0;

	//Load name replacements for files if exists
	loadFile(FILES_BIN, FILES, 26);

	//Load image FONT (old DMG)
	aux = (char*)malloc(13);
	posVRAM = 0;
	memcpy(aux, IMG_NAME, 13);
	memcpy(IMG_NAME, FILES, 13);
	
	disableVDP();
	gfxPictureShow();
	// Copy FONT from VRAM page0 to page1
	bitBlt(0,0, 0, 256, SCREEN_WIDTH,64, 0, 0, CMD_HMMM);
	gfxClearLines(0, 64);
	waitVDPready();
	enableVDP();

	memcpy(IMG_NAME, aux, 13);
	free(13);

	//Load DDB file
	size = filesize(&FILES[13]);
	ddb = malloc(size);
	loadFile(&FILES[13], ddb, size);
}


//=========================================================
// GRAPHICS (GFX)
//=========================================================

#if SCREEN==8
	uint8_t COLOR_INK;		// 255
	uint8_t COLOR_PAPER;	// 0
#elif SCREEN==6
	#define COLOR_INK		3
	#define COLOR_PAPER		0
#else
	#define COLOR_INK		15
	#define COLOR_PAPER		0
#endif

#if SCREEN==8		//SCREEN8 fixed colors
	const uint16_t colorTranslation[] = {	// EGA Palette -> MSX SC8 Palette (GGGRRRBB)
		0x00, 0x03, 0xc0, 0xc3, 0x18, 0x1b, 0x58, 0xdb, 0x49, 0x6f, 0xe9, 0x5f, 0x5d, 0x5f, 0xfd, 0xff
	};
#else				//Paletted colors
	const uint16_t colorTranslation[] = {	// EGA Palette -> MSX grb
		0x000, // 0: 000 black
		0x006, // 1: 006 blue
		0x600, // 2: 060 green
		0x606, // 3_ 066 dark cyan
		0x060, // 4: 600 red
		0x066, // 5: 606 dark purple
		0x260, // 6: 620 orange
		0x666, // 7: 666 light gray
		0x222, // 8: 222 dark gray
		0x337, // 9: 337 light blue
		0x722, //10: 272 light green
		0x727, //11: 277 light cyan
		0x272, //12: 722 light red
		0x277, //13: 727 light purple
		0x772, //14: 772 yellow
		0x777  //15: 777 white
	};
#endif

/*
 * Function: gfxSetScreen
 * --------------------------------
 * Change and initialize the screen to graphical mode.
 * 
 * @return				none.
 */
void gfxSetScreen()
{
	//Set Color 15,0,0
	setColor(15, 0, 0);
	//Set SCREEN mode with interslot BIOS call
	__asm
		ld   a,#SCREEN
		ld   iy,(#EXPTBL)
		ld   ix,#0x5f
		call CALSLT
	__endasm;
	
	//Change screen settings
	disableInterlacedLines();
	enable212lines();
	enable50Hz();
	disableSPR();
	ADDR_POINTER_BYTE(CLIKSW) = 0;	// Disable keys typing sound
	
	//Set Function keys with basic orders
	uint8_t *fk = (void*)FNKSTR;
	memset(fk, 0, 160);

	//Define new function keys with basic orders
	for (int i=0; i<5; i++,fk+=16) {
		memcpy(fk, FUNC_KEYS[i], 9);
	}

	#if SCREEN==8
		COLOR_INK	=   255;
		COLOR_PAPER	=	0;
	#endif
}

/*
 * Function: gfxClearLines
 * --------------------------------
 * Clear several lines filling with PAPER color.
 * 
 * @param start 	Coord-Y to start the clear.
 * @param lines 	Number of lines to clear.
 * @return			none.
 */
void gfxClearLines(uint16_t start, uint16_t lines)
{
	bitBlt(0, 0, 0, start, SCREEN_WIDTH, lines, COLOR_PAPER, 0, CMD_HMMV);
}

/*
 * Function: gfxClearScreen
 * --------------------------------
 * Clear the whole screen filling with PAPER color.
 * 
 * @return			none.
 */
void gfxClearScreen()
{
	gfxClearLines(0, 212);
}

/*
 * Function: gfxClearWindow
 * --------------------------------
 * Clear the current active DAAD Window.
 * 
 * @return			none.
 */
void gfxClearWindow()
{
	bitBlt(0, 0, cw->winX*FONTWIDTH, cw->winY*8, (cw->winW+1)*FONTWIDTH, cw->winH*8, COLOR_PAPER, 0, CMD_HMMV);
}

/*
 * Function: gfxSetPaperCol
 * --------------------------------
 * Set PAPEL colour from a EGA like palette.
 * 
 * @param col		EGA color to set.
 * @return			none.
 */
void gfxSetPaperCol(uint8_t col)
{
	col;
	#if SCREEN==8
		COLOR_PAPER = colorTranslation[col];
	#else
		setColorPal(COLOR_PAPER, colorTranslation[col]);
	#endif
}

/*
 * Function: gfxSetInkCol
 * --------------------------------
 * Set INK colour from a EGA like palette.
 * 
 * @param col		EGA color to set.
 * @return			none.
 */
void gfxSetInkCol(uint8_t col)
{
	col;
	#if SCREEN==8
		COLOR_INK = colorTranslation[col];
	#else
		setColorPal(COLOR_INK, colorTranslation[col]);
	#endif
}

/*
 * Function: gfxSetBorderCol
 * --------------------------------
 * Set BORDER colour from a EGA like palette.
 * 
 * @param col		EGA color to set.
 * @return			none.
 */
void gfxSetBorderCol(uint8_t col)
{
	gfxSetPaperCol(col);
}

/*
 * Function: gfxPutChPixels
 * --------------------------------
 * Write a char to a pixel coords in the graphical screen.
 * 
 * @param c			Char to write.
 * @param dx		Coord-X to write the char.
 * @param dy		Coord-Y to write the char.
 * @return			none.
 */
void gfxPutChPixels(uint8_t c, uint16_t dx, uint16_t dy)
{
	c -= 16;
	uint16_t sx = (c*8)%SCREEN_WIDTH,
	         sy = (c/(SCREEN_WIDTH/8)*FONTHEIGHT)+256;
	#if SCREEN==8
		bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_HMMM);
		bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_INK, 0, CMD_LMMV|LOG_TAND);
		//TODO Add PAPER color
		//bitBlt( 0,  0, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_PAPER, 0, CMD_LMMV|LOG_TIMP);
	#else
		bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_LMMM|LOG_IMP);
	#endif
}

/*
 * Function: gfxPutChWindow
 * --------------------------------
 * Write a char determining place in the current DAAD 
 * Window.
 * 
 * @param c			Char to write.
 * @return			none.
 */
void gfxPutChWindow(uint8_t c)
{
	uint16_t dx = (cw->cursorX+cw->winX)*FONTWIDTH,
	         dy = (cw->cursorY+cw->winY)*FONTHEIGHT;
	gfxPutChPixels(c, dx, dy);
}

/*
 * Function: gfxPutCh
 * --------------------------------
 * Write a char looking for escape chars and managing 
 * DAAD Window.
 * 
 * @param c			Char to write.
 * @return			none.
 */
void gfxPutCh(char c)
{
	switch (c) {
		case 11:		// \b    Clear screen
			do_CLS(); return;
		case 12:		// \k    Wait for a key
			do_INKEY(); return;
		case 14:		// \g    Enable graphical charset (128-255)
			offsetText = 128; return;
		case 15:		// \t    Enable text charset (0-127)
			offsetText = 0; return;
	}
	if (c=='\r') {						// Carriage return
		cw->cursorX = 0;
		cw->cursorY++;
	} else
	if (!(c==' ' && cw->cursorX==0)) {	// If not a SPACE in column 0 then print char
		#ifdef VERBOSE
		printf("%c", c);
		#endif
		gfxPutChWindow(offsetText + c);
		cw->cursorX++;
		if (cw->cursorX >= cw->winW) {
			cw->cursorX = 0;
			cw->cursorY++;
		}
	}
	if (cw->cursorY >= cw->winH) {		// Check for needed scroll
		gfxScrollUp();
		cw->cursorX = 0;
		cw->cursorY--;
	}
	//TODO: Check for a full screen text and show "More..." message
	/*
		if (printedLines >= cw->winH-1) {	// Must show "More..." SYS32?
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

/*
 * Function: gfxPuts
 * --------------------------------
 * Write a string in current DAAD Window.
 * 
 * @param str		String to write.
 * @return			none.
 */
void gfxPuts(char *str)
{
	char *aux = NULL, c;

	while ((c = *str)) {
		if (c==' ' || !aux) {
			aux = str+1;
			while (*aux && *aux!=' ' && *aux!='\n' && *aux!='\r') {
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

/*
 * Function: gfxPutsln
 * --------------------------------
 * Write a string+carriage in current DAAD Window.
 * 
 * @param str		String to write.
 * @return			none.
 */
void gfxPutsln(char *str)
{
	gfxPuts(str);
	gfxPutCh('\r');
}

/*
 * Function: gfxScrollUp
 * --------------------------------
 * Scroll up text in current DAAD Window.
 * 
 * @return			none.
 */
void gfxScrollUp()
{
	if (cw->winH > 1) {
		bitBlt(0, (cw->winY+1)*8, cw->winX*FONTWIDTH, cw->winY*8, cw->winW*FONTWIDTH, (cw->winH-1)*8, COLOR_PAPER, 0, CMD_YMMM);
	}
	bitBlt(0, 0, cw->winX*FONTWIDTH, (cw->winY+cw->winH-1)*8, cw->winW*FONTWIDTH, 8, COLOR_PAPER, 0, CMD_HMMV);
}

/*
 * Function: gfxPicturePrepare
 * --------------------------------
 * Prepare filename to read image and check if 
 * exists.
 * 
 * @param location	DAAD location number.
 * @return			none.
 */
bool gfxPicturePrepare(uint8_t location)
{
	char *pic = IMG_NAME;
	
	pic[2] = location%10 + '0';
	location /= 10;
	pic[1] = location%10 + '0';
	location /= 10;
	pic[0] = location%10 + '0';
	
	posVRAM = cw->winY * 8 * SCREEN_WIDTH;

	return fileexists(IMG_NAME);
}

/*
 * Function: gfxPictureShow
 * --------------------------------
 * Read the image file and shown it at graphical 
 * screen.
 * 
 * @return			none.
 */
void gfxPictureShow()
{
	uint16_t fp;
	IMG_CHUNK *chunk = (IMG_CHUNK*)heap_top;

	do_CLS();
	fp = fopen(IMG_NAME, O_RDONLY);
	if (fp<0xff00) {
		uint16_t size;

		setVDP_Write(0x00000);
		fread((char*)chunk, 4, fp);						// Skip IMAGE_MAGIC "IMGx" TODO: check it!
		do {
			size = fread((char*)chunk, 5, fp);			// Read next chunk type
			if (size & 0xff00) continue;

			if (chunk->type==IMG_CHUNK_REDIRECT) {		// Redirect to another picture
				fclose(fp);
				gfxPicturePrepare(chunk->chunkSize);
				gfxPictureShow();
				return;
			} else
			if (chunk->type==IMG_CHUNK_PALETTE) {		// Load image palette
					size = fread(chunk->data, 32, fp);
				#if SCREEN!=8
					if (!(size & 0xff00))
						setPalette(chunk->data);
				#endif
			} else {
				fread(chunk->data, chunk->chunkSize, fp);
				if (chunk->type==IMG_CHUNK_RAW) {		// Show RAW data
					//TODO: check if works after IMG format change
					__asm
						push hl
						push bc
						ld hl,(#_heap_top)
						ld de,#5
						add hl,de
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
				} else
				if (chunk->type==IMG_CHUNK_RLE) {		// Show RLE data
					unRLE_vram(chunk->data, posVRAM);
				} else
				if (chunk->type==IMG_CHUNK_PLETTER) {	// Show Pletter5 data
					pletter2vram(chunk->data, posVRAM);
				}
				posVRAM += chunk->outSize;
			}
		} while (!(size & 0xff00));

		fclose(fp);
	}
}


//=========================================================
// SOUND (SFX)
//=========================================================

/*
 * Function: sfxSound
 * --------------------------------
 * Sound a tone for a time.
 * 
 * @param value1
 * @param value2
 * @return			none.
 */
void sfxSound(uint8_t value1, uint8_t value2)
{
	value1, value2;
	//TODO: look for a good DAAD emulation. Now just a simple BEEP
	__asm
		ld   iy,(#EXPTBL-1)
		ld   ix,#0xc0
		call CALSLT
	__endasm;
}
