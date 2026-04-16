/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include "daad.h"


/*
 * Function: errorCode
 * --------------------------------
 * Show a system error (see DAAD manual section 4.3)
 * 
 * Print "Game error n" where n is one of:
 * 		0 - Invalid object number
 * 		1 - Illegal assignment to HERE (Flag 38)
 * 		2 - Attempt to set object to loc 255
 * 		3 - Limit reached on PROCESS calls
 * 		4 - Attempt to nest DOALL
 * 		5 - Illegal CondAct (corrupt or old db!)
 * 		6 - Invalid process call
 * 		7 - Invalid message number
 * 		8 - Invalid PICTURE (drawstring only)
 * 
 * In the TEST version the error are followed by four numbers:
 * 		p:v,n>c
 * Where p is the process number, v and n are the word numbers 
 * of the verb and noun of the entry which caused the error, 
 * and c is the CondAct number
 * 
 * @param   code	Error code to show (supported: 0 1 3 5 6 7).
 * @return			none.
 */
void errorCode(uint8_t code)
{
	gfxSetInkCol(14);
	gfxSetPaperCol(4);
	printOutMsg("\nGame Error ");
	printChar(code+'0');

	#ifdef TEST
		printChar(' ');
		gfxSetPaperCol(1);
		printBase10(currProc->num);
		printChar(':');
		printBase10(currProc->entry->verb);
		printChar(',');
		printBase10(currProc->entry->noun);
		printChar('>');
		printBase10(*(currProc->condact));
	#endif
	for (;;);
}

/*
 * Function: waitForTimeout
 * --------------------------------
 * Wait for a timeout or key pressed
 * 
 * @param timeFlag	A mask to compare with Timer Flag.
 * @return			Boolean for timeout if reached or not.
 */
bool waitForTimeout(uint16_t timerFlag)
{
	static uint16_t timeout;
	timeout = flags[fTime]*50;

	clearKeyboardBuffer();
	if (flags[fTIFlags] & timerFlag) {
		flags[fTIFlags] &= TIME_TIMEOUT^255;
		setTime(0);
		while (!checkKeyboardBuffer()) {
			waitingForInput();
			if (getTime() > timeout) {
				flags[fTIFlags] |= TIME_TIMEOUT;
				return true;
			}
		}	
	} else {
		while (!checkKeyboardBuffer()) waitingForInput();
	}
	return false;
}

