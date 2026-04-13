/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "daad.h"


// External
extern void do_CLS();
//extern void do_INKEY();
extern void do_NEWLINE();

// Global variables
//uint8_t    *ddb;						// Where the DDB is allocated
//DDB_Header *hdr;						// Struct pointer to DDB Header
//Object     *objects;					// Memory allocation for objects data
//uint8_t     flags[256];					// DAAD flags (256 bytes)
//char       *ramsave;					// Memory to store ram save (RAMSAVE)

static const uint8_t nullObjFake[] = { 0, 0, 0, 0, 0, 0 };
//const Object  *nullObject;

//Window     *windows;					// 0-7 windows definitions
//Window     *cw;							// Pointer to current active window
//uint8_t     printedLines;				// For "More..." feature
//bool        checkPrintedLines_inUse;

#ifndef DISABLE_SAVEAT
//uint8_t     savedPosX;					// For SAVEAT/BACKAT
//uint8_t     savedPosY;					//  "    "      "
#endif

// Internal variables
static uint8_t lsBuffer0[TEXT_BUFFER_LEN/2+1];	// Logical sentence buffer [type+id] for PARSE 0
static uint8_t lsBuffer1[TEXT_BUFFER_LEN/4+1];	// Logical sentence buffer [type+id] for PARSE 1
//char   *tmpMsg;									// TEXT_BUFFER_LEN
//char   *tmpTok;							// Token temp buffer
//static char    lastPrompt;
//uint8_t doingPrompt;

// Transcript variables
#ifdef TRANSCRIPT
	static uint16_t trIdx;
#endif

//=========================================================


/*
 * Function: parser
 * --------------------------------
 * Parse the words in user entry text and compare 
 * them with VOCabulary table.
 * 
 * @return			none.
 */
void parser()
{
	uint8_t i;
	char *tmpVOC, *p, *p2;
	Vocabulary *voc;
	uint8_t *lsBuffer, *aux;
	lsBuffer = lsBuffer0;
	aux = lsBuffer1;
	p = tmpMsg;
	tmpVOC = safeMemoryAllocate();

	//Clear logical sentences buffer
	clearLogicalSentences();

	while (*p) {
		//Clear tmpVOC
		memset(tmpVOC, ' ', 5);

		if (*p=='"') {
			aux = lsBuffer;
			lsBuffer = lsBuffer1;
			p++;
#ifdef VERBOSE2
cprintf("DETECTED START of literal phrase!\n");
#endif
		}

		//Copy first 5 chars max of word
		p2 = p;
		while (p2-p<5 && *p2!=' ' && *p2!='"' && *p2!='\0') p2++;
		memcpy(tmpVOC, p, p2-p);
#ifdef VERBOSE2
cprintf("%u %c%c%c%c%c: ",p2-p, tmpVOC[0],tmpVOC[1],tmpVOC[2],tmpVOC[3],tmpVOC[4]);
#endif
		for (i=0; i<5; i++) tmpVOC[i] = 255 - tmpVOC[i];

		//Search it in VOCabulary table
		voc = (Vocabulary*)hdr->vocPos;
		while (voc->word[0]) {
			if (!memcmp(tmpVOC, voc->word, 5)) {
				*lsBuffer++ = voc->id;
				*lsBuffer++ = voc->type;
				*lsBuffer = 0;
#ifdef VERBOSE2
cprintf("Found! %u / %u [%c%c%c%c%c]\n",voc->id, voc->type, 255-voc->word[0], 255-voc->word[1], 255-voc->word[2], 255-voc->word[3], 255-voc->word[4]);
#endif
				break;
			}
			voc++;
		}
#ifdef VERBOSE2
if (!voc->word[0]) cprintf("NOT FOUND!\n");
#endif
		while (*p!=' ' && *p!='\0') {
			if (*p=='"') {
				lsBuffer = aux;
#ifdef VERBOSE2
cprintf("DETECTED END of literal phrase!\n");
#endif
			}
			p++;
		}
		while (*p!='\0' && *p==' ') p++;
	}
	safeMemoryDeallocate(tmpVOC);
#ifdef VERBOSE2
cprintf("%u %u %u %u %u %u %u %u \n",lsBuffer[0],lsBuffer[1],lsBuffer[2],lsBuffer[3],lsBuffer[4],lsBuffer[5],lsBuffer[6],lsBuffer[7]);
#endif
}

/*
 * Function: getLogicalSentence
 * --------------------------------
 * Get the first logical sentence from parsed user entry 
 * and fill noun, verbs, adjectives, etc.
 * If no sentences prompt to user.
 * 
 * @return		Boolean with True if any logical sentence found.
 */
bool getLogicalSentence()
{
	// If not logical sentences in buffer we ask the user again
	if (!*lsBuffer0) {
		prompt(true);
		parser();
	}
	return populateLogicalSentence();
}

/*
 * Function: populateLogicalSentence
 * --------------------------------
 * Set the flags with the current logical sentence.
 * 
 * @return		Boolean with True if any logical sentence found.
 */
bool populateLogicalSentence()
{
	char *p, type, id, adj;
	bool ret;
	p = lsBuffer0;
	adj = fAdject1;
	ret = false;

	// Clear parser flags
	flags[fVerb] = flags[fNoun1] = flags[fAdject1] = flags[fAdverb] = flags[fPrep] = flags[fNoun2] = flags[fAdject2] = 
		flags[fCPNoun] = flags[fCPAdject] = NULLWORD;
#ifdef VERBOSE2
cputs("populateLogicalSentence()\n");
#endif
	while (*p && *(p+1)!=CONJUNCTION) {
		id = *p;
		type = *(p+1);
		if (type==VERB && flags[fVerb]==NULLWORD) {										// VERB
			flags[fVerb] = id;
			ret = true;
		} else if (type==NOUN && flags[fNoun1]==NULLWORD) {								// NOUN1
			// word that works like noun and verb
			if (id<20 && flags[fVerb]==NULLWORD) {
				flags[fVerb] = id;
			} else {
				// workd only like noun
				flags[fNoun1] = id;
			}
			ret = true;
		} else if (type==NOUN && flags[fNoun2]==NULLWORD) {								// NOUN2
			flags[fNoun2] = id;
			adj = fAdject2;
			ret = true;
		} else if (type==ADVERB && flags[fAdverb]==NULLWORD) {							// ADVERB
			flags[fAdverb] = id;
			ret = true;
		} else if (type==PREPOSITION && flags[fPrep]==NULLWORD) {						// PREP
			flags[fPrep] = id;
			ret = true;
		} else if (type==ADJECTIVE && adj==fAdject1 && flags[fAdject1]==NULLWORD) {		// ADJ1
			flags[fAdject1] = id;
			ret = true;
		} else if (type==ADJECTIVE && adj==fAdject2 && flags[fAdject2]==NULLWORD) {		// ADJ2
			flags[fAdject2] = id;
			ret = true;
		}
		p+=2;
	}

	if (flags[fNoun2]!=NULLWORD) {
		uint8_t obj = getObjectId(flags[fNoun2], flags[fAdject2], LOC_HERE);
		if (obj!=NULLWORD) {
			flags[fO2Num] = obj;
			flags[fO2Loc] = objects[obj].location;
			flags[fO2Con] = objects[obj].attribs.mask.isContainer << 7;
			flags[fO2Att] = objects[obj].extAttr1;
			flags[fO2Att+1] = objects[obj].extAttr2;
		} else {
			flags[fO2Num] = LOC_NOTCREATED;							// TODO: check default values when Object2 is undefined
			flags[fO2Loc] = LOC_NOTCREATED;
			flags[fO2Con] = flags[fO2Att] = flags[fO2Att+1] = 0;
		}
	}
#ifdef VERBOSE2
cprintf("VERB:%u NOUN1:%u ADJ1:%u, ADVERB:%u PREP: %u NOUN2:%u, ADJ2:%u\n",
		flags[fVerb],flags[fNoun1],flags[fAdject1],flags[fAdverb],flags[fPrep],flags[fNoun2],flags[fAdject2]);
#endif
	nextLogicalSentence();

	return ret;
}

/*
 * Function: useLiteralSentence
 * --------------------------------
 * Copy sentence between "..." if any typed.
 * 
 * @return			Boolean with True if any logical sentence found.
 */
bool useLiteralSentence()
{
	memcpy(lsBuffer0, lsBuffer1, sizeof(lsBuffer1));
	return populateLogicalSentence();
}

/*
 * Function: clearLogicalSentences
 * --------------------------------
 * Clear pending logical sentences if any.
 * 
 * @return			none.
 */
void clearLogicalSentences()
{
#ifdef VERBOSE2
cputs("clearLogicalSentences()\n");
#endif
	memset(lsBuffer0, 0, sizeof(lsBuffer0));
	memset(lsBuffer1, 0, sizeof(lsBuffer1));
}

/*
 * Function: nextLogicalSentence
 * --------------------------------
 * Move next logical sentence to start of logical 
 * sentence buffer.
 * 
 * @return			none.
 */
void nextLogicalSentence()
{
#ifdef VERBOSE2
cputs("nextLogicalSentence()\n");
#endif
	char *p, *c;
	p = lsBuffer0;
	c = lsBuffer0;

	while (*p!=CONJUNCTION && *p!=0) p+=2;
	p+=2;
	for (;;) {
		*c++ = *p;
		*c++ = *(p+1);
		if (!*p) break;
		p+=2;
	}
	*c++ = 0;
	*c = 0;
}

//=========================================================
//UTILS

/*
 * Function: printBase10
 * --------------------------------
 * Prints a base 10 number.
 * 
 * @param value		Number to print.
 * @return			none.
 */
void printBase10(uint16_t value) __z88dk_fastcall
{
	if (value<10) {
		printChar('0'+(uint8_t)value);
		return;
	}
	printBase10(value/10);
	printChar('0'+(uint8_t)(value%10));
}

//=========================================================

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
 * Function: printChar
 * --------------------------------
 * Write a char looking for escape chars and managing 
 * DAAD Window.
 * 
 * @param c			Char to write.
 * @return			none.
 */
void printChar(int ch) __z88dk_fastcall
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

