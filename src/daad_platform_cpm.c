/*
=========================================================

	MSX2: Platform dependent API functions

=========================================================
*/
#ifdef CPM

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "daad_condacts.h"
#include "daad_platform_api.h"
#include "daad.h"
#include "vdp.h"
#include "dos.h"

//=========================================================
// Constants
//=========================================================

//Optional file containing changes to default filename for FONT and DDB files.
const char FILES_BIN[] = "FILES.BIN";

//Default FILES.BIN content (overwrited if a FILES.BIN exists)
const char FILES[] = "\0            DAAD.DDB\0    ";

//MSX special chars (DAAD chars 16-31)
const char EXTENDED_CHARS[]  = "\xA6\xAD\xA8\xAE\xAF\xA0\x82\xA1\xA2\xA3\xA4\xA5\x87\x80\x81\x9A";	//ª¡¿«»áéíóúñÑçÇüÜ


//=========================================================
// Variables
//=========================================================


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
	return EXTENDED_CHARS;
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
	time;
	//TODO: ADDR_POINTER_WORD(JIFFY) = time;
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
	//TODO: return ADDR_POINTER_WORD(JIFFY);
	return 0xffff;
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
	uint16_t size = 0;

	//Load name replacements for files if exists
	loadFile(FILES_BIN, FILES, 26);

	//Load DDB file
	size = filesize(&FILES[13]);
	ddb = malloc(size);
	loadFile(&FILES[13], ddb, size);
}


//=========================================================
// GRAPHICS (GFX)
//=========================================================

/*
 * Function: gfxSetScreen
 * --------------------------------
 * Change and initialize the screen to graphical mode.
 * 
 * @return				none.
 */
void gfxSetScreen()
{
	putchar('\33');
	putchar('y');
	putchar('4');
	do_CLS();
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
	start, lines;
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
	do_CLS();
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
	//TODO: check this
	putchar('\33');
	putchar('E');
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
	c, dx, dy;
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
	putchar(c);
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
		case 127:		// \f    Non Break Space
			c = ' ';
	}
	cw->cursorX++;
	if (c>=16 && c<=31) {
		gfxPutChWindow(EXTENDED_CHARS[c-16]);
	} else
	if (c=='\r') {						// Carriage return
		putchar('\n');
		cw->cursorX = 0;
	} else {
		gfxPutChWindow(offsetText + c);
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
	do_NEWLINE();
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
	keepPos;

	if (c==0x08 && !cw->cursorX) {
		putchar('\33');
		putchar('C');
	} else
	if (c=='\r') {
		do_NEWLINE();
	}
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
	location;
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
	putchar('0x07');
}

#endif //CPM