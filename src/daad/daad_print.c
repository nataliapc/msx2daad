/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include "daad.h"


// External functions
extern void do_CLS();
extern void do_NEWLINE();

/*
 * Function: printChar
 * --------------------------------
 * Write a char looking for escape chars and managing 
 * DAAD Window.
 * 
 * @param c			Char to write.
 * @return			none.
 */
void printChar(int ch)
{
	char c;
	c = (char)ch;

	#if (defined(DEBUG) || defined(TEST)) && !defined(TRANSCRIPT)
		putchar(c);
	#endif
	#ifdef TRANSCRIPT
		transcript_char(c);
	#endif

	switch (c) {
		case 11:		// \b    Clear screen
			do_CLS(); return;
		case 12:		// \k    Wait for a key
			clearKeyboardBuffer();
			while (!checkKeyboardBuffer()) waitingForInput();
			printedLines = 0;
			return;
		case 14:		// \g    Enable graphical charset (128-255)
			gfxSetGraphCharset(true); return;
		case 15:		// \t    Enable text charset (0-127)
			gfxSetGraphCharset(false); return;
	}
	if (c=='\r') {						// Carriage return
		cw->cursorX = 0;
		cw->cursorY++;
		checkPrintedLines();
	} else {
		gfxPutChWindow(c);
		cw->cursorX++;
		if (cw->cursorX >= cw->winW) {
			cw->cursorX = 0;
			cw->cursorY++;
			checkPrintedLines();
		}
	}
	if (cw->cursorY >= cw->winH) {		// Check for needed scroll
		cw->cursorY--;
		gfxScrollUp();
	}
}

/*
 * Function: printBase10
 * --------------------------------
 * Prints a base 10 number.
 * 
 * @param value		Number to print.
 * @return			none.
 */
void printBase10(uint16_t value)
{
	if (value<10) {
		printChar('0'+(uint8_t)value);
		return;
	}
	printBase10(value/10);
	printChar('0'+(uint8_t)(value%10));
}

/*
 * Function: printMsg
 * --------------------------------
 * Uncompress a tokenized string and can print it.
 *
 * @param lst		List of tokenized string (sysmes, usermes, desc...).
 * @param num   	To get the string number 'num' in that list.
 * @param print		Output the string to the current window or not.
 * @return			none.
 *
 * Reentrant function, don't use static variables!
 *
 * REENTRANCY NOTE: printMsg writes to the global tmpMsg buffer.
 * The only known reentrant caller is checkPrintedLines (via printSystemMsg(32)),
 * which is guarded by checkPrintedLines_inUse + safeMemoryAllocate().
 * Any new reentrant call path MUST save/restore tmpMsg the same way.
 */
void printMsg(char *p, bool print)
{
	char c;
	uint16_t i = 0;
	char *token;

	tmpMsg[0]='\0';
	do {
		c = 255 - *p++;
		if (c & 128) {
			token = getToken(c & (128^255));
			while (*token) {
				tmpMsg[i++] = *token;
				if (*token==' ' || *token=='\r' || *token=='\n') {
					if (print) {
						tmpMsg[i] = '\0';
						printOutMsg(tmpMsg);
						i = 0;
					}
				}
				token++;
			}
		} else {
			if (print && (c=='_' || c=='@')) {
				printObjectMsgModif(flags[fCONum], c);
				i = 0;
				continue;
			}
			tmpMsg[i++] = c;
			if (c==' ' || c==0x0a) {
				if (c==0x0a) tmpMsg[--i] = '\0';
				if (print) {
					tmpMsg[i] = '\0';
					printOutMsg(tmpMsg);
					i=0;
				}
			}
		}
	} while (c != 0x0a);		// = 255 - 0xf5
}

/*
 * Function: printOutMsg
 * --------------------------------
 * Write a string in current DAAD Window.
 * 
 * @param str		String to write.
 * @return			none.
 *
 * Reentrant function, don't use static variables!
 */
void printOutMsg(char *str)
{
	char *pointer = str, *aux = NULL, c;

	while ((c = *pointer)) {
		if (c==' ' || !aux) {
			// Check if next word can be printed in current line
			aux = pointer + 1;
			while (*aux && *aux!=' ' && *aux!='\n' && *aux!='\r') {
				aux++;
			}
			if (cw->cursorX + (aux - (pointer + 1)) >= cw->winW) {
				if (c==' ') c = '\r'; else do_NEWLINE();
			}
		}
		if (doingPrompt || pointer==str || !(c==' ' && cw->cursorX==0)) {	// If not a SPACE in column 0 then print char
			printChar(c);
		}
		pointer++;
	}
}

/*
 * Function: checkPrintedLines
 * --------------------------------
 * Check for a screen overflow text and show "More..." message.
 * 
 * @return			none.
 */
void checkPrintedLines()
{
	char *oldTmpMsg;

	if (checkPrintedLines_inUse ||
		cw->mode & MODE_DISABLEMORE || 
		cw->winH==1)
	{
		return;
	}
	checkPrintedLines_inUse = true;

	if (++printedLines >= cw->winH-1) {	// Must show "More..."?
		if (cw->cursorY >= cw->winH) {
			cw->cursorX = 0;
			cw->cursorY--;
			gfxScrollUp();
		}
		// Print SYS32 "More..."
		oldTmpMsg = tmpMsg;
		tmpMsg = safeMemoryAllocate();
		printSystemMsg(32);
		tmpMsg = oldTmpMsg;
		safeMemoryDeallocate(tmpMsg);
		waitForTimeout(TIME_MORE);
		gfxClearCurrentLine();
		cw->cursorX = 0;
		printedLines=0;
	}
	checkPrintedLines_inUse = false;
}

