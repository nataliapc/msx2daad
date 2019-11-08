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
const char FILES_BIN[] = "FILES.BIN\0  ";

//Default FILES.BIN content (overwrited if a FILES.BIN exists)
const char FILES[] = 
	"FONT.IM"STRINGIFY(SCREEN_CHAR)"\0    "		// Filename for Font file
	"DAAD.DDB\0    "							// Filename for DDB file
	"000.IM"STRINGIFY(SCREEN_CHAR)"\0     "		// Buffer to fill with the image to load (XPICTURE)
	"TEXTS.XDB\0   ";							// Filename with externalized texts (XMES/XMESSAGE)

#define FILE_FONT	&FILES[0]
#define FILE_DDB	&FILES[13]
#define FILE_IMG	&FILES[26]
#define FILE_XDB	&FILES[39]

//MSX special chars (DAAD chars 16-31)
const char CHARS_MSX[]  = "\xA6\xAD\xA8\xAE\xAF\xA0\x82\xA1\xA2\xA3\xA4\xA5\x87\x80\x81\x9A";	//ª¡¿«»áéíóúñÑçÇüÜ

//Function keys redefinitions
#ifdef LANG_ES
	const char FUNC_KEYS[5][10] = { "examinar ", "coger \0  ", "soltar \0 ", "buscar \0 ", "lanzar \0 " };
#endif
#ifdef LANG_EN
	const char FUNC_KEYS[5][8]  = { "examine ", "get \0   ", "drop \0  ", "search \0", "throw \0 " };
#endif

//=========================================================
// Variables
//=========================================================

// Start position in VRAM where draw a image file
uint32_t posVRAM;

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
		die("MSX2 with 128Kb VRAM is needed to run!\n");
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
	return kbhit();
}

void clearKeyboardBuffer()
{
	while (checkKeyboardBuffer()) getchar();
}

void waitingForInput()
{
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
	uint16_t size = 0;
	// Commandline parameter with new FILES_BIN?
	if (argc>0) {
		memcpy(FILES_BIN, argv[0], 13);
	}

	// Load name replacements for files if exists
	loadFile(FILES_BIN, FILES, sizeof(FILES));

	// Load image FONT (old DMG)
	aux = malloc(13);
	memcpy(aux, FILE_IMG, 13);
	memcpy(FILE_IMG, FILE_FONT, 13);
	// Read FONT from file to VRAM 2nd page
	posVRAM = 0x10000;
	gfxPictureShow();
	// Restore & free
	memcpy(FILE_IMG, aux, 13);
	free(13);

	// Load DDB file
	size = filesize(FILE_DDB);
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
// GRAPHICS (GFX)
//=========================================================

#if SCREEN == 8 
	uint8_t COLOR_INK;		// 255
	uint8_t COLOR_PAPER;	// 0
#elif SCREEN >= 10
	#define COLOR_INK		0xff
	#define COLOR_PAPER		0x00
#elif SCREEN == 6
	#define COLOR_INK		3
	#define COLOR_PAPER		0
#elif SCREEN < 8
	#define COLOR_INK		15
	#define COLOR_PAPER		0
#endif

#if SCREEN == 8		//SCREEN 8 fixed colors
	const uint8_t colorTranslation[] = {	// EGA Palette -> MSX SC8 Palette (GGGRRRBB)
	//  000   006   600   606   060   066   260   666   222   447   733   737   373   377   773   777      (GGGRRRBBB) 9bits color guide
		0x00, 0x02, 0xc0, 0xc3, 0x18, 0x1b, 0x58, 0xdb, 0x49, 0x93, 0xed, 0xef, 0x7d, 0x7f, 0xfd, 0xff	// (GGGRRRBB) 8bits real color used
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
		0x666, // 7: 555 light gray
		0x222, // 8: 222 dark gray
		0x447, // 9: 337 light blue
		0x733, //10: 272 light green
		0x737, //11: 277 light cyan
		0x373, //12: 722 light red
		0x377, //13: 727 light purple
		0x773, //14: 772 yellow
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
			ld   bc,#0x0899		; enable YJK colors
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
	enable50Hz();
	disableSPR();
	ADDR_POINTER_BYTE(CLIKSW) = 0;	// Disable keys typing sound
	
	//Set Function keys with basic orders
	uint8_t *fk = (void*)FNKSTR;
	memset(fk, 0, 160);

	//Define new function keys with basic orders
	for (int i=0; i<5; i++,fk+=16) {
		memcpy(fk, FUNC_KEYS[i], 16);
	}

	#if SCREEN == 8
		COLOR_INK	=   colorTranslation[15];
		COLOR_PAPER	=	colorTranslation[0];
	#endif
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
 * 					128|16|5  - SCREEN 5
 * 					128|16|6  - SCREEN 6
 * 					128|16|7  - SCREEN 7
 * 					    16|8  - SCREEN 8
 * 					    16|12 - SCREEN 12
 * 
 * @return		Return the value to be assigned to the flag
 */
void gfxSetScreenModeFlags()
{
	// Set Graphics flag
	flags[fGFlags] = 0b10000000;
	// Set ScreenMode flag
	#if SCREEN == 5
		flags[fGFlags] = 128|16|5;
	#elif SCREEN == 6
		flags[fGFlags] =  128|16|6;
	#elif SCREEN == 7
		flags[fGFlags] =  128|16|7;
	#elif SCREEN == 8
		flags[fGFlags] =  16|8;
	#elif SCREEN == 12
		flags[fGFlags] =  16|12;
	#endif
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
	if (cw->winH > 1) {
		bitBlt(0, (cw->winY+1)*FONTHEIGHT, cw->winX*FONTWIDTH, cw->winY*FONTHEIGHT, cw->winW*FONTWIDTH, (cw->winH-1)*FONTHEIGHT, 0, 0, CMD_YMMM);
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
	#if SCREEN == 8
		COLOR_PAPER = colorTranslation[col];
	#elif SCREEN < 8
		setColorPal(COLOR_PAPER, colorTranslation[col]);
	#else
		// No PAPER color change
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
	#if SCREEN == 8
		COLOR_INK = colorTranslation[col];
	#elif SCREEN < 8
		setColorPal(COLOR_INK, colorTranslation[col]);
	#else
		// No INK color change
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
inline void gfxSetBorderCol(uint8_t col)
{
	gfxSetPaperCol(col);	//TODO revise for real border change code
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
	         sy = (c/(SCREEN_WIDTH/FONTHEIGHT)*FONTHEIGHT)+256;
	#if SCREEN==8
		#ifdef DISABLE_GFXCHAR_COLOR
			if (c>=128-16) {
				bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_LMMM|LOG_IMP);
			} else
		#endif
		if (COLOR_PAPER==0) {
			bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_HMMM);						// Paint char in white
			bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_INK, 0, CMD_LMMV|LOG_TAND);			// Paint char INK foreground
		} else {
			//Use VRAM 2nd page like TEMP space working to avoid glitches
			if (COLOR_INK==0) {
				bitBlt( 0,  0, dx, 256+64, FONTWIDTH, FONTHEIGHT, 255, 0, CMD_HMMV);				// Paint white background destination
				bitBlt(sx, sy, dx, 256+64, FONTWIDTH, FONTHEIGHT, 0, 0, CMD_LMMM|LOG_XOR);			// Paint char in black
				bitBlt( 0,  0, dx, 256+64, FONTWIDTH, FONTHEIGHT, COLOR_PAPER, 0, CMD_LMMV|LOG_AND);// Paint PAPER background destination
				bitBlt(dx, 256+64, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_HMMM);				// Copy TEMP char to destination
			} else {
				bitBlt( 0,  0, dx, dy, FONTWIDTH, FONTHEIGHT, COLOR_PAPER, 0, CMD_HMMV);			// Paint PAPER background destination
				bitBlt(sx, sy, dx, 256+64, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_HMMM);				// Paint TEMP char in white
				bitBlt(sx, sy, dx, 256+64, FONTWIDTH, FONTHEIGHT, COLOR_INK, 0, CMD_LMMV|LOG_TAND);	// Paint TEMP INK color foreground
				bitBlt(dx, 256+64, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_LMMM|LOG_TIMP);		// Copy TEMP char to destination
			}
		}
	#else
		bitBlt(sx, sy, dx, dy, FONTWIDTH, FONTHEIGHT, 0x00, 0, CMD_LMMM|LOG_IMP);				// Copy char in white
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
	
	#if SCREEN==7 || SCREEN==8 || SCREEN>=10
		posVRAM = cw->winY * 8 * 256;
	#elif SCREEN==5 || SCREEN==6
		posVRAM = cw->winY * 8 * 128;
	#endif

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
void gfxPictureShow()
{
	uint16_t fp;
	IMG_CHUNK *chunk = (IMG_CHUNK*)heap_top;

	fp = fopen(FILE_IMG, O_RDONLY);
	if (fp<0xff00) {
		uint16_t size;

		fread((char*)chunk, 4, fp);						// Skip IMAGE_MAGIC "IMGx"
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
				#if SCREEN < 8
					size = fread(chunk->data, 32, fp);
					if (!(size & 0xff00))
						setPalette(chunk->data);
				#else
					fseek(fp, 32, SEEK_CUR);
				#endif
			} else {
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
				posVRAM += chunk->outSize;
			}
		} while (!(size & 0xff00));

		fclose(fp);
	}
}


//=========================================================
// SOUND (SFX)
//=========================================================


#ifndef DISABLE_BEEP
// MSX PSG Tones from https://www.konamiman.com/msx/msx2th/th-5a.txt and upgraded
const uint16_t sfxFreqPSG[] = {
	0xD65, 0xC9D, 0xBEB, 0xB42, 0xA9A, 0xA04, 0x971, 0x8E8, 0x86B, 0x7E2, 0x77F, 0x719,	// Octave 1 (48-70) 
	0x6B3, 0x64E, 0x5F5, 0x5A1, 0x54D, 0x502, 0x4B9, 0x474, 0x434, 0x3F9, 0x3C0, 0x38C,	// Octave 2 (72-98)
	0x359, 0x327, 0x2F6, 0x2D1, 0x2A7, 0x281, 0x25C, 0x23A, 0x21A, 0x1FD, 0x1E0, 0x1C6, // Octave 3 (96-118) 
	0x1AD, 0x194, 0x17D, 0x168, 0x153, 0x141, 0x12E, 0x11D, 0x10D, 0x0FE, 0x0F0, 0x0E3, // Octave 4 (120-142)
	0x0D6, 0x0CA, 0x0BF, 0x0B4, 0x0AA, 0x0A0, 0x097, 0x08F, 0x087, 0x07F, 0x078, 0x072,	// Octave 5 (144-166) 
	0x06B, 0x065, 0x05F, 0x05A, 0x055, 0x050, 0x04C, 0x047, 0x043, 0x040, 0x03C, 0x039,	// Octave 6 (168-190)
	0x036, 0x033, 0x030, 0x02D, 0x02A, 0x028, 0x026, 0x024, 0x022, 0x020, 0x01E, 0x01C,	// Octave 7 (192-214) 
	0x01B, 0x019, 0x018, 0x017, 0x015, 0x014, 0x013, 0x012, 0x011, 0x010, 0x00F, 0x00E	// Octave 8 (216-238)
};

#define PSG_VOLUME  12		// 0...15

/*
 * Function: sfxSound
 * --------------------------------
 * Sound a tone for a time.
 * 
 * @param duration	1/100 sec
 * @param tone		Values 48-238. See table sfxFreqPSG[]
 * @return			none.
 */
void sfxSound(uint8_t duration, uint8_t tone) __naked
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

		or   e					; tone==0 => is a silence
		jr   z,silence$

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

		ld   a,#7				; REG#7 Mixer enable ChannelA
		out  (0xa0),a
		ld   d,#0b00111110
		out  (c),d

		inc  a					; REG#8 ChannelA set Volume to 8
		out  (0xa0),a
		ld   d,#PSG_VOLUME
		out  (c),d

	silence$:
		ex   de,hl
		call wait$

		ld   a,#8				; REG#8 ChannelA set Volume to 0
		out  (0xa0),a
		ld   d,#0
		out  (c),d

		ld   e,#1
		call wait$

		pop  ix
		ret

	wait$:						; Wait for E = 1/50 seconds
		ld  hl,#JIFFY
	loop0$:
		ld  a,(hl)
	loop1$:
		cp  (hl)
		jr  z,loop1$
		dec e
		jr  nz,loop0$
		ret

	__endasm;
}
#endif //DISABLE_BEEP

