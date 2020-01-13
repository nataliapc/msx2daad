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
#if defined(DEBUG) || defined(VERBOSE) || defined(VERBOSE2)
	#include <stdio.h>
#endif


// External
extern void do_CLS();
extern void do_INKEY();
extern void do_NEWLINE();

// Global variables
uint8_t    *ddb;						// Where the DDB is allocated
DDB_Header *hdr;						// Struct pointer to DDB Header
Object     *objects;					// Memory allocation for objects data
uint8_t     flags[256];					// DAAD flags (256 bytes)
char       *ramsave;					// Memory to store ram save (RAMSAVE)

#ifndef DISABLE_WINDOW
	#define WINDOWS_NUM		8
#else
	#define WINDOWS_NUM		1
#endif
Window     *windows;					// 0-7 windows definitions
Window     *cw;							// Pointer to current active window
uint8_t 	printedLines;				// For "More..." feature

#ifndef DISABLE_SAVEAT
uint8_t     savedPosX;					// For SAVEAT/BACKAT
uint8_t     savedPosY;					//  "    "      "
#endif

// Internal variables
uint8_t lsBuffer[TEXT_BUFFER_LEN/2+1];	// Logical sentence buffer [type+id]
char   *tmpMsg;							// TEXT_BUFFER_LEN
char   *tmpTok;							// Token temp buffer
char    lastPrompt;
uint8_t doingPrompt;

// Transcript variables
#ifdef TRANSCRIPT
	uint16_t trIdx;
#endif

//=========================================================

/*
 * Function: initDAAD
 * --------------------------------
 * Initialize DDB and DAAD engine.
 * 
 * @return			none.
 */
bool initDAAD(int argc, char **argv)
{
	uint16_t *p;
	
	loadFilesBin(argc, argv);

	hdr = (DDB_Header*)ddb;
	p = (uint16_t *)&hdr->tokensPos;

	#ifdef DEBUG
		printf("Version.......... %u\n", hdr->version);
		printf("Machine.......... %u\n", hdr->target.value.machine);
		printf("Language......... %u\n", hdr->target.value.language);
		printf("Magic............ 0x%02x\n", hdr->magic);
		printf("Num.Obj.......... %u\n", hdr->numObjDsc);
		printf("Num.Locations.... %u\n", hdr->numLocDsc);
		printf("Num.Usr.Msg...... %u\n", hdr->numUsrMsg);
		printf("Num.Sys.Msg...... %u\n", hdr->numSysMsg);
		printf("Num.Proc......... %u\n", hdr->numPrc);
		printf("Tokens pos....... 0x%04x\n", hdr->tokensPos);
		printf("Proc list pos.... 0x%04x\n", hdr->prcLstPos);
		printf("Obj. list pos.... 0x%04x\n", hdr->objLstPos);
		printf("Loc. list pos.... 0x%04x\n", hdr->locLstPos);
		printf("Usr. msg. pos.... 0x%04x\n", hdr->usrMsgPos);
		printf("Sys. msg. pos.... 0x%04x\n", hdr->sysMsgPos);
		printf("Connections pos.. 0x%04x\n", hdr->conLstPos);
		printf("Vocabulary pos... 0x%04x\n", hdr->vocPos);
		printf("Obj.Loc. list.... 0x%04x\n", hdr->objLocLst);
		printf("Obj.Name pos..... 0x%04x\n", hdr->objNamePos);
		printf("Obj.Attr pos..... 0x%04x\n", hdr->objAttrPos);
		printf("Obj.Extr pos..... 0x%04x\n", hdr->objExtrPos);
		printf("File length...... %u bytes\n", hdr->fileLength);
	#endif

	//If not a valid DDB version exits
	if (hdr->version != 2)
		return false;

	//Update header positions addresses
	for (int i=0; i<12; i++) {
		*(p++) += (uint16_t)ddb;
	}

	//Skip first token
	while ((*(char*)(hdr->tokensPos++) & 0x80) == 0);

	//Get memory for RAMSAVE
	ramsave = (char*)malloc(256+sizeof(Object)*hdr->numObjDsc);
	memset(ramsave, 0, 1+256+sizeof(Object)*hdr->numObjDsc);
	//Get memory for objects
	objects = (Object*)malloc(sizeof(Object)*hdr->numObjDsc);
	//Get memory for tmpTok & tmpMsg
	tmpTok = (char*)malloc(32);
	tmpMsg = (char*)malloc(TEXT_BUFFER_LEN);

	#ifdef DEBUG
		printf("\nDDB max size..... %u bytes\n", getFreeMemory());
	#endif

	#ifdef TRANSCRIPT
		trIdx = 0;
	#endif

	return true;
}

/*
 * Function: initFlags
 * --------------------------------
 * Initialize DAAD flags and some structs.
 * 
 * @return			none.
 */
void initFlags()
{
	//Clear flag of player location
	flags[fPlayer] = 0;

	gfxSetScreenModeFlags();

	//Initialize DAAD windows
	windows = malloc(sizeof(Window)*WINDOWS_NUM);
	memset(windows, 0, sizeof(windows)*WINDOWS_NUM);
	flags[fCurWin] = 0;
	for (int i=0; i<WINDOWS_NUM; i++) {
		cw = &windows[i];
		cw->winW = MAX_COLUMNS;
		cw->winH = MAX_LINES;
		cw->mode = 0;
		gfxSetPaperCol(0);
		gfxSetInkCol(15);
	}
	cw = &windows[0];
	#ifndef DISABLE_SAVEAT
		savedPosX = savedPosY = 0;
	#endif

	//Initialize last onscreen picture
	lastPicLocation = 255;
	lastPicShow = false;

	//Clear logical sentences
	clearLogicalSentences();

	doingPrompt = false;
}

/*
 * Function: initObjects
 * --------------------------------
 * Initialize Objects.
 * 
 * @return			none.
 */
void initObjects()
{
	uint8_t  *objLoc = (uint8_t*)hdr->objLocLst;
	uint8_t  *attrLoc = (uint8_t*)hdr->objAttrPos;
	uint8_t  *extAttrLoc = (uint8_t*)hdr->objExtrPos;
	uint8_t  *nameObj = (uint8_t*)hdr->objNamePos;

	flags[fNOCarr] = 0;

	for (int i=0; i < hdr->numObjDsc; i++) {
		objects[i].location     = *(objLoc++);
		objects[i].attribs.byte = *(attrLoc++);
		objects[i].extAttr1     = *(extAttrLoc++);
		objects[i].extAttr2     = *(extAttrLoc++);
		objects[i].nounId       = *(nameObj++);
		objects[i].adjectiveId  = *(nameObj++);
		if (objects[i].location==LOC_CARRIED) flags[fNOCarr]++;
	}
}

/*
 * Function: mainLoop
 * --------------------------------
 * DAAD main loop start.
 * 
 * @return			none.
 */
void mainLoop()
{
	initFlags();
	initializePROC();

	pushPROC(0);
	processPROC();
}

/*
 * Function: prompt
 * --------------------------------
 * Wait for user entry text and fill tmpMsg 
 * variable with it.
 * 
 * @return			none.
 */
void prompt()
{
	char c, *p = tmpMsg, *extChars;

	#ifdef TRANSCRIPT
		transcript_flush();
	#endif

	doingPrompt = true;
	printedLines = 0;
	clearKeyboardBuffer();
	printSystemMsg(33);	//">"
	*p = '\0';
	do {
		// Check first char Timeout flag
		if (p==tmpMsg) {
			if (waitForTimeout(TIME_FIRSTCHAR)) {
				doingPrompt = false;
				return;
			}
		}
		while (!checkKeyboardBuffer()) waitingForInput();
		c = getchar();
		if ((c=='\r' || c==' ') && p<=tmpMsg) { c = 0; continue; }	// Avoid enter an empty text order
		if (c==0x08) {												// Back space (BS)
			if (p<=tmpMsg) continue;
			*--p = '\0';
			if (cw->cursorX>0) cw->cursorX--; else { cw->cursorX = cw->winW-1; cw->cursorY--; }
			gfxPutInputEcho(c, true);
		} else
		if (c>=' ' || c=='\r') {
			if (p-tmpMsg > TEXT_BUFFER_LEN) continue;				// Avoid chars at buffer limit end
			extChars = strchr(getCharsTranslation(), c);
			if (extChars) c = (char)(extChars-getCharsTranslation()+0x10);
			gfxPutInputEcho(c, false);
			*p++ = toupper(c);										// TODO: revise this line
		}
	} while (c!='\r');
	gfxPutInputEcho(c, false);
	*--p = '\0';
	printedLines = 0;
	doingPrompt = false;
}

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
	char *tmpVOC = heap_top, *p = tmpMsg, *p2;
	uint8_t ils = 0;
	Vocabulary *voc;

	//Clear logical sentences buffer
	clearLogicalSentences();

	while (*p) {
		//Clear tmpVOC
		memset(tmpVOC, ' ', 5);

		//Copy first 5 chars max of word
		p2 = p;
		while (p2-p<5 && *p2!=' ' && *p2!='\0') p2++;
		memcpy(tmpVOC, p, p2-p);
#ifdef VERBOSE2
printf("%u %c%c%c%c%c: ",p2-p, tmpVOC[0],tmpVOC[1],tmpVOC[2],tmpVOC[3],tmpVOC[4]);
#endif
		for (int i=0; i<5; i++) tmpVOC[i] = 255 - tmpVOC[i];

		//Search it in VOCabulary table
		voc = (Vocabulary*)hdr->vocPos;
		while (voc->word[0]) {
			if (!memcmp(tmpVOC, voc->word, 5)) {
				lsBuffer[ils++] = voc->id;
				lsBuffer[ils++] = voc->type;
#ifdef VERBOSE2
printf("Found! %u / %u\n",voc->id, voc->type);
#endif
				break;
			}
			voc++;
		}
#ifdef VERBOSE2
if (!voc->word[0]) printf("NOT FOUND!\n");
#endif
		p = strchr(p2, ' ');
		if (!p) break;
		p++;
	}
#ifdef VERBOSE2
printf("%02u %02u %02u %02u %02u %02u %02u %02u \n",lsBuffer[0],lsBuffer[1],lsBuffer[2],lsBuffer[3],lsBuffer[4],lsBuffer[5],lsBuffer[6],lsBuffer[7]);
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
	char *p = lsBuffer, type, id, adj = fAdject1;
	bool ret = false;

	// If not logical sentences in buffer we ask to user again
	if (!*p) {
		char newPrompt;

		newPrompt = flags[fPrompt];
		if (!newPrompt)
			while ((newPrompt=(rand()%4)+2)==lastPrompt);
		printSystemMsg(newPrompt);
		lastPrompt = newPrompt;

		prompt();
		parser();
	}

	// Clear parser flags
	flags[fVerb] = flags[fNoun1] = flags[fAdject1] = flags[fAdverb] = flags[fPrep] = flags[fNoun2] = flags[fAdject2] = 
		flags[fCPNoun] = flags[fCPAdject] = NULLWORD;
#ifdef VERBOSE2
printf("populateLogicalSentence()\n");
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
printf("VERB:%u NOUN1:%u ADJ1:%u, ADVERB:%u PREP: %u NOUN2:%u, ADJ2:%u %u\n",flags[fVerb],flags[fNoun1],flags[fAdject1],flags[fAdverb],flags[fPrep],flags[fNoun2],flags[fAdject2]);
#endif
	nextLogicalSentence();

	return ret;
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
printf("clearLogicalSentences()\n");
#endif
	memset(lsBuffer, 0, sizeof(lsBuffer));
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
printf("nextLogicalSentence()\n");
#endif
	char *p = lsBuffer, *c = lsBuffer;
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
void printBase10(uint16_t value)
{
	if (value<10) {
		if (value) printChar('0'+(uint8_t)value);
		return;
	}
	printBase10(value/10);
	printChar('0'+(uint8_t)(value%10));
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
	uint16_t timeout = flags[fTime]*50;

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
	printOutMsg("Game Error ");
	printChar(code+'0');
// TODO
//	#ifdef TEST
//		printChar(' ');
//		printfBase10(currProc->num);
//		printChar(':');
//		printfBase10(currProc->entry->verb);
//		printChar(',');
//		printfBase10(currProc->entry->noun);
//		printChar('>');
//		printfBase10(currCondact->condact);
//	#endif
	for (;;);
}


//=========================================================

/*
 * Function: getToken
 * --------------------------------
 * Return the requested token.
 * 
 * @param num   	To get the token number 'num' in the token list.
 * @return			Return a pointer to the requested token.
 */
char* getToken(uint8_t num)
{
	char *p = (char*)hdr->tokensPos;
	uint8_t i=0;

	// Skip previous tokens
	while (num) {
		if (*p > 127) num--;
		p++;
		if (!num) break;
	}
	// Copy selected token
	do {
		tmpTok[i++] = *p & 0x7f;
	} while (*p++ < 127);
	tmpTok[i]='\0';

	return tmpTok;
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
 */
void printMsg(char *p, bool print)
{
	char c, *token;
	uint16_t i = 0;

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
 */
void printOutMsg(char *str)
{
	char *p = str, *aux = NULL, c;

	while ((c = *p)) {
		if (c==' ' || !aux) {
			// Check if next word can be printed in current line
			aux = p+1;
			while (*aux && *aux!=' ' && *aux!='\n' && *aux!='\r') {
				aux++;
			}
			if (cw->cursorX+(aux-(p+1)) >= cw->winW) {
				if (c==' ') c = '\r'; else do_NEWLINE();
			}
		}
		if (doingPrompt || p==str || !(c==' ' && cw->cursorX==0)) {	// If not a SPACE in column 0 then print char
			printChar(c);
		}
		p++;
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
void printChar(char c)
{
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
	if (cw->mode & MODE_DISABLEMORE || cw->winH==1) return;
	if (++printedLines >= cw->winH-1) {	// Must show "More..."?
		if (cw->cursorY >= cw->winH) {
			cw->cursorX = 0;
			cw->cursorY--;
			gfxScrollUp();
		}
		// Print SYS32 "More..."
		char *oldTmpMsg = tmpMsg;
		tmpMsg = malloc(0);
		printSystemMsg(32);
		tmpMsg = oldTmpMsg;
		waitForTimeout(TIME_MORE);
		gfxClearCurrentLine();
		cw->cursorX = 0;
		printedLines=0;
	}
}

/*
 * Function: _ptrToMessage
 * --------------------------------
 * Return pointer to a DDB message.
 * 
 * @param lst		Section pointer.
 * @param num		Message number to extract.
 * @return			none.
 */
char* _ptrToMessage(uint16_t *lst, uint8_t num)
{
	return &ddb[*(lst + num)];
}

/*
 * Function: getSystemMsg
 * --------------------------------
 * Extract system message.
 * 
 * @param num		Number of system message.
 * @return			none.
 */
void getSystemMsg(uint8_t num)
{
	printMsg(_ptrToMessage((uint16_t*)hdr->sysMsgPos, num), false);
}

/*
 * Function: printSystemMsg
 * --------------------------------
 * Extract system message and print it.
 * 
 * @param num		Number of system message.
 * @return			none.
 */
void printSystemMsg(uint8_t num)
{
	printMsg(_ptrToMessage((uint16_t*)hdr->sysMsgPos, num), true);
}

/*
 * Function: printUserMsg
 * --------------------------------
 * Extract user message and print it.
 * 
 * @param num		Number of user message.
 * @return			none.
 */
void printUserMsg(uint8_t num)
{
	if (num > hdr->numUsrMsg) errorCode(7);
	printMsg(_ptrToMessage((uint16_t*)hdr->usrMsgPos, num), true);
}

/*
 * Function: printLocationMsg
 * --------------------------------
 * Extract location message and print it.
 * 
 * @param num		Number of location message.
 * @return			none.
 */
void printLocationMsg(uint8_t num)
{
	if (num > hdr->numLocDsc) errorCode(1);
	printMsg(_ptrToMessage((uint16_t*)hdr->locLstPos, num), true);
}

/*
 * Function: printObjectMsg
 * --------------------------------
 * Extract object message and print it.
 * 
 * @param num		Number of object message.
 * @return			none.
 */
void printObjectMsg(uint8_t num)
{
	if (num > hdr->numObjDsc) errorCode(0);
	printMsg(_ptrToMessage((uint16_t*)hdr->objLstPos, num), true);
}

/*
 * Function: printObjectMsgModif
 * --------------------------------
 * Extract object message, change the article and print it:
 * "Una linterna" -> "La linterna"
 * "A lantern" -> "Lantern"
 *  
 * @param num		Number of object name.
 * @param modif		Modifier for uppercase.
 * @return			none.
 */
void printObjectMsgModif(uint8_t num, char modif)
{
	modif;
	char *ini = tmpMsg, *p = tmpMsg;
	printMsg(_ptrToMessage((uint16_t*)hdr->objLstPos, num), false);
#ifdef LANG_ES
	if (tmpMsg[2]==' ') {
		tmpMsg[0] = modif=='@'?'E':'e';
		tmpMsg[1] = 'l';
	} else
	if (tmpMsg[3]==' ') {
		ini++;
		tmpMsg[1] = modif=='@'?'L':'l';
	}
	while (*p) {
		if (*p=='.' || *p==0x0a) { *p--='\0'; }
		p++;
	}
#elif LANG_EN
	if (tmpMsg[1]==' ') ini+=2;
	else
	if (tmpMsg[2]==' ') ini+=3;
#endif
	printOutMsg(ini);
}

/*
 * Function: getObjectId
 * --------------------------------
 * Return de object ID by Noun+Adjc ID.
 *  
 * @param noun		Noun ID.
 * @param adjc		Adjective ID, or NULLWORD to disable adjective filter.
 * @location		Location where the object must be, LOC_HERE to disable location filter, or LOC_CONTAINER if location is a container.
 * @return			Object ID if found or NULLWORD.
 */
uint8_t getObjectId(uint8_t noun, uint8_t adjc, uint16_t location)
{
	for (int i=0; i<hdr->numObjDsc; i++) {
		if (objects[i].nounId==noun && 
		   (adjc==NULLWORD || objects[i].adjectiveId==adjc) && 							// If 'adjc' not needed or 'adjc' matchs
		   ((location==LOC_HERE || objects[i].location==location) ||					// It's in anywhere or placed in 'location'...
		    (location==LOC_CONTAINER && location<hdr->numObjDsc && 
										objects[location].attribs.mask.isContainer)))	// ...or if it's in a container
		{
			return i;
		}
	}
	return NULLWORD;
}

/*
 * Function: getObjectWeight
 * --------------------------------
 * Return the weight of a object by ID. Also can return 
 * the total weight of location or carried/worn objects
 * if objno is NULLWORD.
 *  
 * @param objno			Object ID or NULLWORD.
 * @param isCarriedWorn	Check carried/worn objects if True.
 * @return				Return the weight of one or a sum of objects.
 */
uint8_t getObjectWeight(uint8_t objno, bool isCarriedWorn)
{
	uint16_t weight = 0;
	Object *obj = objects;
	for (int i=0; i<hdr->numObjDsc; i++) {
		if ((objno==NULLWORD || objno==i) && (!isCarriedWorn || obj->location==LOC_CARRIED || obj->location==LOC_WORN)) {
			if (obj->attribs.mask.isContainer && obj->attribs.mask.weight!=0) {
				weight += getObjectWeight(i, false);
			}
			weight += obj->attribs.mask.weight;
		}
		obj++;
	}
	return weight>255 ? 255 : (uint8_t)weight;
}

/*
 * Function: referencedObject
 * --------------------------------
 * Modify DAAD flags to reference the las object used
 * in a logical sentence.
 *  
 * @param objno		Object ID.
 * @return			none.
 */
void referencedObject(uint8_t objno)
{
	flags[fCONum] = objno;
	flags[fCOLoc] = objects[objno].location;
	flags[fCOWei] = objects[objno].attribs.mask.weight;
	flags[fCOCon] = flags[fCOCon] & 0b01111111 | (objects[objno].attribs.mask.isContainer << 7);
	flags[fCOWR]  = flags[fCOWR] & 0b01111111 | (objects[objno].attribs.mask.isWareable << 7);
	flags[fCOAtt] = objects[objno].extAttr1;
	flags[fCOAtt+1] = objects[objno].extAttr2;
}

/*
 * Function: transcript
 * --------------------------------
 * Store the game texts and sentences in a file named "TRANSCR.TXT"
 *  
 * @param c			Char to store.
 * @return			none.
 */
#ifdef TRANSCRIPT
const uint8_t transcript_translate[] = {
	"ª¡¿«»áéíóúñÑçÇüÜ"
};

const char transcript_filename[] = "TRANSCR.TXT";
char transcript_buff[1024];

void transcript_flush()
{
	uint16_t fp;
	uint32_t size;

	size = filesize(transcript_filename);
	if (size>=0xff00)
		fp = fcreate(transcript_filename, 0, 0);
	else {
		fp = fopen(transcript_filename, O_WRONLY);
		fseek(fp, size, SEEK_SET);
	}
	if (fp<0xff00) {
		fputs(transcript_buff, fp);
		fclose(fp);
	}
	trIdx = 0;
	transcript_buff[0] = '\0';
}

void transcript_char(char c)
{
	if (c == 127) {
		transcript_buff[trIdx++] = ' ';
	} else
	if (c < 16) {
		transcript_buff[trIdx++] = '\n';
	} else
	if (c < 32) {
		char *utf = transcript_translate+((c-16)<<1);
		transcript_buff[trIdx++] = *utf++;
		transcript_buff[trIdx++] = *utf;
	} else {
		transcript_buff[trIdx++] = c;
	}
	transcript_buff[trIdx] = 0;

	if (trIdx>1020) {
		transcript_flush();
	}
}

#endif
