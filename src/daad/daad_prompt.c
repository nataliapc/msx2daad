/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include <stdlib.h>
#include <string.h>
#include "daad.h"


// External functions
extern void do_CLS();
extern void do_NEWLINE();

// Internal variables
static char lastPrompt;


/*
 * Function: prompt
 * --------------------------------
 * Wait for user entry text and fill tmpMsg 
 * variable with it.
 * 
 * @return			none.
 */
void prompt(bool printPromptMsg)
{
	char c, *p, *extChars, newPrompt;
	p = tmpMsg;

	#ifdef TRANSCRIPT
		transcript_flush();
	#endif

	// Change to input stream window
	if (flags[fInStream] != 0) {
		cw = &windows[flags[fInStream]];
	}

	// Print random prompt message
	if (printPromptMsg) {
		newPrompt = flags[fPrompt];
		if (!newPrompt)
			while ((newPrompt=((uint8_t)rand()&3)+2)==lastPrompt);
		printSystemMsg(newPrompt);
		lastPrompt = newPrompt;
	}

	doingPrompt = true;
	printedLines = 0;
	clearKeyboardBuffer();
	printSystemMsg(33);	//">"
	*p = '\0';
	do {
		// Check first char Timeout flag
		if (p==tmpMsg) {
			if (waitForTimeout(TIME_FIRSTCHAR)) {
				goto ret_continue;
			}
		}
		while (!checkKeyboardBuffer()) waitingForInput();
		c = getKeyInBuffer();
		if ((c=='\r' || c==' ') && p<=tmpMsg) { c = 0; continue; }	// Avoid enter an empty text order
		if (c==0x08) {												// Back space (BS)
			if (p<=tmpMsg) continue;
			*--p = '\0';
			if (cw->cursorX>0) cw->cursorX--; else { cw->cursorX = cw->winW-1; cw->cursorY--; }
			gfxPutInputEcho(' ', true);
		} else
		if (c>=' ' || c=='\r') {
			if (p-tmpMsg > TEXT_BUFFER_LEN) continue;				// Avoid chars at buffer limit end
			extChars = strchr(getCharsTranslation(), c);
			if (extChars) c = (char)(extChars-getCharsTranslation()+0x10);
			gfxPutInputEcho(c, false);
			*p++ = toupper(c);
		}
	} while (c!='\r');
	*--p = '\0';

ret_continue:
	// Clear input stream window if flag is enabled
	if (flags[fTIFlags] & TIME_CLEAR) { do_CLS(); }

	// Restore to current window
	cw = &windows[flags[fCurWin]];

	// Echo input to current window if flag is enabled
	if (flags[fTIFlags] & TIME_INPUT) {
		p = tmpMsg;
		tmpMsg = heap_top;
		printSystemMsg(33);	//">"
		tmpMsg = p;
		printOutMsg(tmpMsg);
		do_NEWLINE();
	}

	// Reset variables
	printedLines = 0;
	doingPrompt = false;
}
