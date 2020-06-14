/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio

	=========================================================
		MSX2: System dependent API functions
	=========================================================
*/
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "daad_condacts.h"
#include "daad_platform_msx2.h"
#include "daad.h"
#include "vdp.h"
#include "dos.h"
#if defined(TEST) || defined(DEBUG)
	#include "debug.h"
#endif


//=========================================================
// Constants
//=========================================================

//Optional file containing changes to default filename for FONT and DDB files.
const char FILES_BIN[] = "FILES.BIN\0  ";

//Default FILES.BIN content (overwrited if a FILES.BIN exists)
const char FILES[] = 
	"FONT    IM"SCREEN_CHAR"\n"		// Filename for Font file
	"DAAD    DDB\n"					// Filename for DDB file
	"TEXTS   XDB\n"					// Filename with externalized texts (XMES/XMESSAGE)
	"LOADING IM"SCREEN_CHAR"\n"		// Filename for loading screen
	"000     IM"SCREEN_CHAR"\n";	// Buffer to fill with the image to load (XPICTURE)

#define FILE_FONT	&FILES[0]
#define FILE_DDB	&FILES[12]
#define FILE_XDB	&FILES[24]
#define FILE_LOAD	&FILES[36]
#define FILE_IMG	&FILES[48]

//MSX special chars (DAAD chars 16-31)
const char CHARS_MSX[]  = "\xA6\xAD\xA8\xAE\xAF\xA0\x82\xA1\xA2\xA3\xA4\xA5\x87\x80\x81\x9A";	//ª¡¿«»áéíóúñÑçÇüÜ

//Function keys redefinitions
#ifdef LANG_ES
	const char FUNC_KEYS[5][10] = { "examinar ", "coger \0  ", "soltar \0 ", "buscar \0 ", "lanzar \0 " };
#endif
#ifdef LANG_EN
	const char FUNC_KEYS[5][9]  = { "examine ", "get \0   ", "drop \0  ", "search \0", "throw \0 " };
#endif


//=========================================================
// Variables
//=========================================================

// Start position in VRAM where draw a image file
uint32_t posVRAM;

// Offset to set/unset the graphical charset
bool offsetText;
// Offset to set/unset the VRAM page where load images
uint32_t gfxPictureOffet;
// Current VRAM page visible
bool currentPage;

// RAM Mapper variables [experimental]
#ifdef RAM_MAPPER
	bool    DOS2MAPPER;
	uint8_t MAX_MAPPER_PAGES;
	uint8_t PAGE2_RAMSEG;
	uint8_t FIRST_RAMSEG[4];
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
		die("MSX2 with 128Kb VRAM is needed!\n");
	}
	// If compiled with MSXDOS2 lib check for DOS version
	#ifdef MSXDOS2
		if (dosver()<VER_MSXDOS2x) {
			die("MSXDOS 2.x or better is needed!\n");
		}
	#endif

	// If compiled with RAM Mapper Cache initialize it
	#ifdef RAM_MAPPER
		initializeMapper();
		#ifdef DEBUG
			printf("\nFree RAM mapper pages detected: %u\n", MAX_MAPPER_PAGES);
			printf("First Free RAM page: %u\n", FIRST_RAMSEG[0]);
			printf("Page2 default RAM segment: %u\n\n", PAGE2_RAMSEG);
		#endif
	#endif

	//Enable R800 / Turbo CPU
//	enableR800CPU();
//	enableTurboCPU();

    return true;
}

/*
 * Function: getFreeMemory
 * --------------------------------
 * Return the free TPA in bytes.
 * 
 * @return			Return the free memory in bytes.
 */
uint16_t getFreeMemory()
{
    return ADDR_POINTER_WORD(TPA_LIMIT) - heap_top + hdr->fileLength - sizeof(IMG_CHUNK);
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
void setTime(uint16_t time) __naked
{
	time;
	__asm
		push af
		push hl
		pop  hl
		pop  af
		ld (#JIFFY),hl
		ret
	__endasm;
}

/*
 * Function: getTime
 * --------------------------------
 * Return the system time.
 * 
 * @return			Return the system time in 1/50 sec fragments.
 */
uint16_t getTime() __naked
{
	__asm
		ld hl,(#JIFFY)
		ret
	__endasm;
}

uint16_t checkKeyboardBuffer()
{
	return ADDR_POINTER_WORD(PUTPNT)-ADDR_POINTER_WORD(GETPNT);
}

void clearKeyboardBuffer()
{
	ADDR_POINTER_WORD(PUTPNT) = ADDR_POINTER_WORD(GETPNT);
}

uint8_t  getKeyInBuffer()
{
	uint8_t *pnt = (uint8_t*)ADDR_POINTER_WORD(GETPNT);
	uint8_t ret = *pnt;
	pnt++;
	if (pnt >= (uint8_t*)(KEYBUF+40)) pnt = (uint8_t*)KEYBUF;
	ADDR_POINTER_WORD(GETPNT) = (uint16_t)pnt;
	return ret;
}

void waitingForInput()
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
void loadFilesBin(int argc, char **argv)
{
	char *aux;
	size_t size = 0;

	// Commandline parameter with new FILES_BIN?
	if (argc>0) {
		memcpy(FILES_BIN, argv[0], 13);
	}

	// Load name replacements for files if exists
	loadFile(FILES_BIN, FILES, filesize(FILES));

	// Replace \n for \0 chars
	aux = FILES;
	*(aux+11) = *(aux+23) = *(aux+35) = *(aux+47) = *(aux+59) = '\0';

	aux = malloc(11);
	memcpy(aux, FILE_IMG, 11);
	//Show LOADING picture
	memcpy(FILE_IMG, FILE_LOAD, 11);
	cw = (Window*)(heap_top+IMG_MAXREAD);	//Fake current Window
	cw->winX = cw->winY = 0;
	posVRAM = 0;
	gfxPictureShow();

	// Load image FONT (old DMG) to VRAM hidden zone
	memcpy(FILE_IMG, FILE_FONT, 11);
	posVRAM = ((uint32_t)FONTINITY) * BYTESxLINE;
	gfxPictureShow();
	// Restore & free
	memcpy(FILE_IMG, aux, 11);
	free(11);

	// Load DDB file
	size = filesize(FILE_DDB);
	if (getFreeMemory() - size < 0) die("Max. DDB size exceeded.");
	ddb = malloc(size);
	loadFile(FILE_DDB, ddb, size);

	// Load External Texts DDB file (max 64kb in this version)
	#ifdef RAM_MAPPER
		// There is at least 4x16kb of paged RAM & file exists?
		if (MAX_MAPPER_PAGES>=4 && fileexists(FILE_XDB)) {
			loadExtendedTexts(FILE_XDB);
		}
	#endif
}


//=========================================================
// EXTERNAL TEXTS
//=========================================================

#ifndef DISABLE_EXTERN
#ifdef RAM_MAPPER

void loadExtendedTexts(char *filename)
{
	uint16_t toRead = 16384, 
			 size,
			 fp;
	uint8_t  page = 0;

	size = filesize(filename);
	fp = fopen(filename, O_RDONLY);

	if (fp < 0xff00) {
#ifdef DEBUG
		printf("Loading %s (%u bytes): ", filename, size);
#endif
		while (size) {
			if (size<toRead)  toRead=size;
			setMapperPage2(FIRST_RAMSEG[page++]);
			fread(PAGE2, toRead, fp);
			size -= toRead;
#ifdef DEBUG
			putchar('*');
#endif
		}
		restoreMapperPage2();
		fclose(fp);
#ifdef DEBUG
		printf("\n\n");
#endif
	}
}

#endif	//RAM_MAPPER

/*
 * Function: printXMES
 * --------------------------------
 * Print a string from externalized messages.
 * 
 * @param address	Message address in a virtual 64kb file/RAM.
 * @return			none.
 */
void printXMES(uint16_t address)
{
	#ifdef RAM_MAPPER
		if (FIRST_RAMSEG[0]) {
			//Print from RAM
			setMapperPage2(FIRST_RAMSEG[(uint8_t)(address>>14)]);
			printMsg((char*)(0x8000 + (address & 0x3FFF)), true);
			restoreMapperPage2();
		} else
	#endif //RAM_MAPPER
	{
		//Print from file
		uint16_t fp = fopen(FILE_XDB, O_RDONLY);
		if (fp < 0xff00) {
			fseek(fp, address, SEEK_SET);
			fread((char*)heap_top, 512, fp);
			fclose(fp);
			printMsg((char*)heap_top, true);
		}
	}
}

#endif //DISABLE_EXTERN


//=========================================================
// GRAPHICS (GFX)
//=========================================================

#if SCREEN==12
	#define COLOR_INK		0xff
	#define COLOR_PAPER		0x00
#else
	#define COLOR_INK		(cw->ink)
	#define COLOR_PAPER		(cw->paper)
#endif

#if SCREEN == 8
	#define COLOR_BLACK 	0x00
	#define COLOR_WHITE 	0xff
#elif SCREEN == 10
	#define COLOR_BLACK 	0x08
	#define COLOR_WHITE 	0xf8
#else
	#define COLOR_BLACK 	0x00
	#define COLOR_WHITE 	0x0f
#endif

#if SCREEN==6 || SCREEN==12
	#define AUX_HMMM		CMD_LMMM|LOG_IMP
	#define AUX_HMMV		CMD_LMMV|LOG_IMP
#else
	#define AUX_HMMM		CMD_HMMM
	#define AUX_HMMV		CMD_HMMV
#endif

#if SCREEN == 8		//SCREEN 8 fixed colors
	const uint8_t colorTranslationSC8[] = {	// EGA Palette -> MSX SC8 Palette (GGGRRRBB)
	//  000   006   600   606   060   066   260   666   222   447   733   737   373   377   773   777      (GGGRRRBBB) 9bits color guide
		0x00, 0x02, 0xc0, 0xc3, 0x18, 0x1b, 0x58, 0xdb, 0x49, 0x93, 0xed, 0xef, 0x7d, 0x7f, 0xfd, 0xff	// (GGGRRRBB) 8bits real color used
	};
#endif
#if SCREEN == 6	//SCREEN 6 paletted colors
	const uint16_t colorTranslation[] = {	// Amber Palette -> MSX grb
		0x000, // 0: 000 black
		0x230, // 1: 320 dark amber
		0x450, // 2: 540 medium amber
		0x670, // 3_ 760 light amber
		0,0,0,0, 0,0,0,0, 0,0,0,0
	};
#else										//Paletted colors
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
		0x447, // 9: 447 light blue
		0x733, //10: 373 light green
		0x737, //11: 377 light cyan
		0x373, //12: 733 light red
		0x377, //13: 727 light purple
		0x773, //14: 773 yellow
		0x777  //15: 777 white
	};
#endif

uint8_t getColor(uint8_t col)
{
	col;
	#if SCREEN == 6
		col %= 4;
		return col | (col << 2) | (col << 4) | (col << 6);
	#elif SCREEN < 8
		col %= 16;
		return col | (col << 4);
	#elif SCREEN == 8
		return colorTranslationSC8[col % 16];
	#elif SCREEN == 10
		return col << 4 | 8;
	#else
		return 0;
	#endif
}

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
	#if SCREEN <= 8
		__asm
			ld   a,#SCREEN
			ld   iy,(#EXPTBL)
			ld   ix,#0x5f
			call CALSLT
		__endasm;
	#else
		__asm
			ld   a,#8
			ld   iy,(#EXPTBL)
			ld   ix,#0x5f
			call CALSLT
		#if SCREEN==10
			ld   bc,#0x1899		; enable YJK colors + Palette mixed mode
		#elif SCREEN==12
			ld   bc,#0x0899		; enable YJK colors
		#endif
			out  (c),b
			out  (c),c
		__endasm;
	#endif

	//Change screen settings
	disableInterlacedLines();
	#if SCREEN_HEIGHT==192
		enable192lines();
	#elif SCREEN_HEIGHT==212
		enable212lines();
	#endif

	//Clear VRAM page 2
	gfxRoutines(6, 0);	// Clear Back screen

	//Disable hardware sprites
	disableSPR();

	//Set screen adjust
	setRegVDP8(18, ADDR_POINTER_BYTE(0xFFF1));

	//Set Palette
	#if SCREEN!=8 && SCREEN!=12
		setPalette(colorTranslation);
	#endif

	//Disable keys typing sound
//	ADDR_POINTER_BYTE(CLIKSW) = 0;
	
	//Set Function keys with basic orders
	uint8_t *fk = (void*)FNKSTR;
	memset(fk, 0, 160);

	//Define new function keys with basic orders
	for (int i=0; i<5; i++,fk+=16) {
		memcpy(fk, FUNC_KEYS[i], 16);
	}

	//Initialize variables
	offsetText = false;
	currentPage = false;
	gfxPictureOffet = 0;
}

/*
 * Function: gfxSetScreenModeFlags
 * --------------------------------
 * Set the value for flags [fGFlags] & [fScMode]:
 * 
 * 		[fGFlags] Flag 29:
 * 			Bit 7: Set if there are graphics available 					[ALWAYS SET IN MSX2 INTERPRETERS]
 * 			       If the flag is less than 128 then you can assume that 
 * 			       you are in 80 col text mode (because only the ST and 
 * 			       the IBM (and now MSX2 too) will not set this bit, and 
 * 			       that is when they are running in 80 column text mode.
 * 			Bit 6: Invisible draw mode for Drawstring machines 			[**NOT IMPLEMENTED**]
 * 			       Whe set this bit causes drawstring machines that have a 
 * 			       palette (i.e. CPC) to refrain from setting the colours  
 * 			       until the entire picture has been draw.
 * 			Bit 5: Pictures OFF (drawstring only) 						[**NOT IMPLEMENTED**]
 * 			Bit 4: Wait for a key after drawing picture (drawstring) 	[**NOT IMPLEMENTED**]
 * 			Bit 3: Change BORDER to picture colours (drawstring) 		[**NOT IMPLEMENTED**]
 * 			Bit 2: Undefined											[**UNDEFINED**]
 * 			Bit 1: Undefined											[**UNDEFINED**]
 * 			Bit 0: Mouse present (16 bit only) 							[**NOT IMPLEMENTED**]
 * 
 * 		[FScMode] Flag 62:
 * 			On ST and PC gives the absolute screen mode in use on the machine. 
 * 			This allows checks about the screen size etc, but to determine 
 * 			if you are in graphics mode (see [fGFlags] Flag 29).
 * 				Bit 7: Is set in VGA to indicate you have palette switching.
 *			On the ST (bits 0-3):
 * 				0 - Low Res
 * 				1 - Med Res
 * 			On the PC (bits 0-3):
 * 				4 - CGA
 * 				7 - Mono character only
 * 				13 - EGA or VGA
 * 			On MSX2:
 * 				Bit 4: Is a MSX/MSX2 mode
 * 				Bits 0-3: Screen mode
 * 					16|5  - SCREEN 5
 * 					16|6  - SCREEN 6
 * 					16|7  - SCREEN 7
 * 					16|8  - SCREEN 8
 * 					16|10 - SCREEN 10
 * 					16|12 - SCREEN 12
 * 
 * @return		Return the value to be assigned to the flag
 */
void gfxSetScreenModeFlags()
{
	// Set Graphics & ScreenMode flag
	flags[fGFlags] = 0b10000000;
	flags[fScMode] = 16|SCREEN;
}

/*
 * Function: gfxClearScreenBlock
 * --------------------------------
 * Clear a chars block filling with PAPER color.
 * 
 * @param x			Coord-X to start the clear.
 * @param y			Coord-Y to start the clear.
 * @param w 		Width to clear in pixels.
 * @param h 		Height to clear in pixels.
 * @return			none.
 */
void gfxClearScreenBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
	uint16_t w2 = w<MAX_COLUMNS ? w*FONTWIDTH : SCREEN_WIDTH;
	bitBlt(0, 0, x*FONTWIDTH, y*FONTHEIGHT, w2, h*FONTHEIGHT, COLOR_PAPER, 0, CMD_HMMV);
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
	gfxClearScreenBlock(cw->winX, cw->winY, cw->winW, cw->winH);
}

/*
 * Function: gfxClearCurrentLine
 * --------------------------------
 * Clear the current line in DAAD Window.
 * 
 * @return			none.
 */
void gfxClearCurrentLine()
{
	gfxClearScreenBlock(cw->winX, cw->winY+cw->cursorY, cw->winW, 1);
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
	ASM_HALT;
	if (cw->winH > 1) {
		bitBlt(cw->winX*FONTWIDTH, (cw->winY+1)*FONTHEIGHT, cw->winX*FONTWIDTH, cw->winY*FONTHEIGHT, cw->winW*FONTWIDTH, (cw->winH-1)*FONTHEIGHT, 0, 0, CMD_HMMM);
	}
	gfxClearScreenBlock(cw->winX, (cw->winY+cw->winH-1), cw->winW, 1);
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
	#if SCREEN < 12
		COLOR_PAPER = getColor(col);
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
	#if SCREEN < 12
		COLOR_INK = getColor(col);
	#endif
}

/*
 * Function: gfxSetBorderCol
 * --------------------------------
 * Set BORDER colour from a EGA like palette (SCR 8/12), or the current
 * Picture palette (SCR 5/6/7).
 * 
 * @param col		EGA color to set.
 * @return			none.
 */
void gfxSetBorderCol(uint8_t col)
{
	setBorder(colorTranslation[col % 16]);
}

/*
 * Function: gfxSetGraphCharset
 * --------------------------------
 * Enable/Disable the graphical charset.
 * 
 * @param value		Boolean to set/unset the charset.
 * @return			none.
 */
void gfxSetGraphCharset(bool value)
{
	offsetText = value;
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
	         sy = (c/(SCREEN_WIDTH/FONTHEIGHT)*FONTHEIGHT) + FONTINITY;

	if ((cw->mode & MODE_FORCEGCHAR) || offsetText) sy += (256+8);

	#if SCREEN <= 10
		#ifdef DISABLE_GFXCHAR_COLOR
			if (c>=128-16) {
				bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);
			} else
		#endif
		if (COLOR_PAPER==COLOR_BLACK) {
			bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);							// Paint char in white
			if (COLOR_INK!=COLOR_WHITE) {
				bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_INK, 0, CMD_LMMV|LOG_TAND);			// Paint char INK foreground
			}
		} else {
			//Use VRAM like TEMP working space to avoid glitches
			if (COLOR_INK==COLOR_BLACK) {
				bitBlt( 0,  0, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, 255, 0, AUX_HMMV);					// Paint white background destination
				bitBlt(sx, sy, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, 0, 0, CMD_LMMM|LOG_XOR);			// Paint char in black
				bitBlt( 0,  0, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, COLOR_PAPER, 0, CMD_LMMV|LOG_AND);	// Paint PAPER background destination
				bitBlt(dx, FONTTEMPY, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);				// Copy TEMP char to destination
			} else {
				bitBlt( 0,  0, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_PAPER, 0, AUX_HMMV);				// Paint PAPER background destination
				bitBlt(sx, sy, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);				// Paint TEMP char in white
				bitBlt(sx, sy, dx, FONTTEMPY, FONTWIDTH, FONTHEIGHT, COLOR_INK, 0, CMD_LMMV|LOG_TAND);	// Paint TEMP INK color foreground
				bitBlt(dx, FONTTEMPY, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_LMMM|LOG_TIMP);		// Copy TEMP char to destination
			}
		}
	#else
		bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, AUX_HMMM);								// Copy char in white
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
 * Function: gfxPutInputEcho
 * --------------------------------
 * Print a char from console input with echo.
 * 
 * @param c			Char to print.
 * @return			none.
 */
void gfxPutInputEcho(char c, bool keepPos)
{
	if (c==0x08) c=' ';
	if (keepPos)
		gfxPutChWindow(c);
	else
		printChar(c);
}

/*
 * Function: gfxSetPalette
 * --------------------------------
 * Set palette RGB to color index.
 * 
 * @param index		Color index to change.
 * @param red		Red component.
 * @param green		Green component.
 * @param blue		Blue component.
 * @return			none.
 */
void gfxSetPalette(uint8_t index, uint8_t red, uint8_t green, uint8_t blue)
{
	setColorPal(index%16, (((uint16_t)red & 0b11100000)<<3) | ((green & 0b11100000)>>1) | ((blue & 0b11100000)>>5));
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
	char *pic = FILE_IMG;
	
	pic[2] = location%10 + '0';
	location /= 10;
	pic[1] = location%10 + '0';
	location /= 10;
	pic[0] = location%10 + '0';
	
	posVRAM = cw->winX * FONTWIDTH / PIXELSxBYTE + cw->winY * FONTHEIGHT * BYTESxLINE + gfxPictureOffet;
	return fileexists(FILE_IMG);
}

/*
 * Function: gfxPictureShow
 * --------------------------------
 * Read the image file and shown it at graphical 
 * screen.
 * 
 * @return			none.
 */
bool gfxPictureShow()
{
	uint16_t fp;
	IMG_CHUNK *chunk = (IMG_CHUNK*)heap_top;

	fp = fopen(FILE_IMG, O_RDONLY);
	if (fp<0xff00) {
		uint16_t size;

		fseek(fp, 4, SEEK_SET);							// Skip IMAGE_MAGIC "IMGx"
		do {
			size = fread((char*)chunk, 5, fp);			// Read next chunk type
			if (size & 0xff00) continue;

			//=============================================
			// Redirect to another picture
			if (chunk->type==IMG_CHUNK_REDIRECT) {
				fclose(fp);
				gfxPicturePrepare(chunk->chunkSize);
				return gfxPictureShow();
			} else
			//=============================================
			// Clear Window (CLS)
			if (chunk->type==IMG_CHUNK_CLS) {
				uint8_t pageOffset = gfxPictureOffet ? 32 : 0;
				gfxClearScreenBlock(cw->winX, cw->winY+pageOffset, cw->winW, cw->winH);
			} else
			//=============================================
			// Reset VRAM write position to current Window
			if (chunk->type==IMG_CHUNK_RESET) {
				posVRAM = cw->winX * FONTWIDTH / PIXELSxBYTE + cw->winY * FONTHEIGHT * BYTESxLINE + gfxPictureOffet;
			} else
			//=============================================
			// Skip VRAM bytes
			if (chunk->type==IMG_CHUNK_SKIP) {
				posVRAM += chunk->auxData;
			} else
			//=============================================
			// Pause the image load in 1/50 sec units
			if (chunk->type==IMG_CHUNK_PAUSE) {
				setTime(0);
				while (getTime() < chunk->auxData) waitingForInput();
			} else
			//=============================================
			// Load image palette
			if (chunk->type==IMG_CHUNK_PALETTE) {
				#if SCREEN!=8 && SCREEN!=12
					size = fread(chunk->data, 32, fp);
					if (!(size & 0xff00))
						setPalette(chunk->data);
				#else
					fseek(fp, 32, SEEK_CUR);
				#endif
			} else {
				//=============================================
				// Picture data chunks
				fread(chunk->data, chunk->chunkSize, fp);
				if (chunk->type==IMG_CHUNK_RAW) {		// Show RAW data
					copyToVRAM((uint16_t)chunk->data, posVRAM, chunk->chunkSize);
				} else
				if (chunk->type==IMG_CHUNK_RLE) {		// Show RLE data
					unRLE_vram(chunk->data, posVRAM);
				} else
				if (chunk->type==IMG_CHUNK_PLETTER) {	// Show Pletter5 data
					pletter2vram(chunk->data, posVRAM);
				}
				posVRAM += chunk->auxData;	//OutSize
			}
		} while (!(size & 0xff00));

		fclose(fp);
		return true;
	}
	return false;
}

/*
 * Function: gfxRoutines
 * --------------------------------
 * Execute GFX condact routines.
 * 
 * @param routine	The routine to execute.
 * @param value		Optional value if the routine needs it.
 * @return			none.
 */
void gfxRoutines(uint8_t routine, uint8_t value)
{
	value;
	uint16_t page_offset = 0;

	switch (routine) {
#ifndef DISABLE_GFX
		//=================== BACK->PHYS
		case 0:
			bitBlt(0, 256, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00, 0, CMD_HMMM); // Copy screen
			break;
		//=================== PHYS->BACK
		case 1:
			bitBlt(0, 0, 0, 256, SCREEN_WIDTH, SCREEN_HEIGHT, 0x00, 0, CMD_HMMM); // Copy screen
			break;
		//=================== SWAP PHYS<->BACK
		case 2:
			currentPage = !currentPage;
			waitVDPready();
			if (currentPage) setVPage(1); else setVPage(0);
			break;
		//=================== Graphics Write to Phys
		case 3:
			gfxPictureOffet = 0;
			break;
		//=================== Graphics Write to Back
		case 4:
			gfxPictureOffet = 256l * BYTESxLINE;
			break;
#endif//DISABLE_GFX
		//=================== Clear Back
		case 6:
			page_offset = 256l;
		//=================== Clear Phys
		case 5:
			bitBlt(0, 0, 0, page_offset, SCREEN_WIDTH, SCREEN_HEIGHT, getColor(0), 0, CMD_HMMV);
			break;
	}
}


//=========================================================
// SOUND (SFX)
//=========================================================

#define PSG_VOLUME  8		// 0...15

#ifndef DISABLE_BEEP
// MSX PSG Tones from https://www.konamiman.com/msx/msx2th/th-5a.txt and upgraded
const uint16_t sfxFreqPSG[] = {
	0xD65, 0xC9D, 0xBEB, 0xB42, 0xA9A, 0xA04, 0x971, 0x8E8, 0x86B, 0x7E2, 0x77F, 0x719,	// Octave 1 (48-70)
	0x6B3, 0x64E, 0x5F5, 0x5A1, 0x54D, 0x502, 0x4B9, 0x474, 0x434, 0x3F9, 0x3C0, 0x38C,	// Octave 2 (72-96)
	0x359, 0x327, 0x2F6, 0x2D1, 0x2A7, 0x281, 0x25C, 0x23A, 0x21A, 0x1FD, 0x1E0, 0x1C6, // Octave 3 (98-118)
	0x1AD, 0x194, 0x17D, 0x168, 0x153, 0x141, 0x12E, 0x11D, 0x10D, 0x0FE, 0x0F0, 0x0E3, // Octave 4 (120-142)
	0x0D6, 0x0CA, 0x0BF, 0x0B4, 0x0AA, 0x0A0, 0x097, 0x08F, 0x087, 0x07F, 0x078, 0x072,	// Octave 5 (144-166)
	0x06B, 0x065, 0x05F, 0x05A, 0x055, 0x050, 0x04C, 0x047, 0x043, 0x040, 0x03C, 0x039,	// Octave 6 (168-190)
	0x036, 0x033, 0x030, 0x02D, 0x02A, 0x028, 0x026, 0x024, 0x022, 0x020, 0x01E, 0x01C,	// Octave 7 (192-214)
	0x01B, 0x019, 0x018, 0x017, 0x015, 0x014, 0x013, 0x012, 0x011, 0x010, 0x00F, 0x00E	// Octave 8 (216-238)
};
#endif//DISABLE_BEEP

/*
 * Function: sfxInit
 * --------------------------------
 * Initialize sound chip.
 * 
 * @return			none.
 */
void sfxInit() __naked
{
	#ifndef DISABLE_BEEP
	__asm
		ld   a,#7				; REG#7 Mixer disable ChannelA
		out  (0xa0),a
		ld   a,#0b00111111
		out  (0xa1),a

		ld   a,#8				; REG#8 ChannelA initialize Volume to 8
		out  (0xa0),a
		ld   a,#PSG_VOLUME
		out  (0xa1),a

		ret
	__endasm;
	#endif//DISABLE_BEEP
}

/*
 * Function: sfxWriteRegister
 * --------------------------------
 * Value is written to register 'reg' of the sound chip on 8 bit machines.
 * 
 * @param reg		Register to set value.
 * @param value		Value to write.
 * @return			none.
 */
void sfxWriteRegister(uint8_t reg, uint8_t value) __naked
{
	reg, value;
	#ifndef DISABLE_SFX
	__asm
		pop  af
		pop  bc
		push bc
		push af

		ld  a,c
		out (0xa0),a
		ld  a,b
		out (0xa1),a
		ret
	__endasm;
	#endif
}

/*
 * Function: sfxSound
 * --------------------------------
 * Sound a tone for a time.
 * 
 * @param duration	1/100 sec
 * @param tone		Values 48-238. See table sfxFreqPSG[]
 * @return			none.
 */
#ifndef DISABLE_BEEP
void sfxTone(uint8_t duration, uint8_t tone) __naked
{
	duration, tone;
	__asm
		push ix
		ld   ix,#4
		add  ix,sp

		xor  a
		ld   d,a				; DE=tone
		ld   e,0(ix)
		ld   l,1(ix)			; L=duration

		ld   ix,#_sfxFreqPSG-48	; DE=get frequency from tone table
		add  ix,de
		ld   e,0(ix)
		ld   d,1(ix)

		ld   c,#0xa1

		xor  a					; REG#0 ChannelA Tone LowByte
		out  (0xa0),a
		out  (c),e
		inc  a					; REG#1 ChannelA Tone HighByte
		out  (0xa0),a
		out  (c),d

		ld   e,#0b00111110		; Mixer enable ChannelA
		call mixer$

	silence$:
		ld   b,l
		srl  b
		jr	 z,disablepsg$
	
		ld  hl,#JIFFY			; Wait for B = 1/50 seconds
	loop0$:
		ld  a,(hl)
	loop1$:
		cp  (hl)
		jr  z,loop1$
		djnz loop0$

	disablepsg$:
		ld   e,#0b00111111		; Mixer disable ChannelA
		call mixer$

		pop  ix
		ret

	mixer$:
		ld   a,#7				; REG#7 PSG Mixer
		out  (0xa0),a
		out  (c),e
		ret

	__endasm;
}
#endif //DISABLE_BEEP


