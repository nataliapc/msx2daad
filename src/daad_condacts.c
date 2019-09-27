#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "daad.h"
#include "daad_condacts.h"


#define pPROC currProc->condact

PROCstack  procStack[10];
PROCstack *currProc;

uint8_t indirection;
uint8_t checkEntry;
uint8_t isDone, lastIsDone;
uint8_t lastPicLocation;
uint8_t lastPicShow;

const CONDACT_LIST condactList[] = {
	{ do_AT, 		0 },
	{ do_NOTAT,		0 },
	{ do_ATGT,		0 },
	{ do_ATLT,		0 },
	{ do_PRESENT,	0 },
	{ do_ABSENT,	0 },
	{ do_WORN,		0 },
	{ do_NOTWORN,	0 },
	{ do_CARRIED,	0 },
	{ do_NOTCARR,	0 },
	{ do_CHANCE,	0 },
	{ do_ZERO,		0 },
	{ do_NOTZERO,	0 },
	{ do_EQ,		0 },
	{ do_GT,		0 },
	{ do_LT,		0 },
	{ do_ADJECT1,	0 },
	{ do_ADVERB,	0 },
	{ do_SFX,		1 },
	{ do_DESC,		1 },
	{ do_QUIT,		0 },
	{ do_END,		1 },
	{ do_DONE,		1 },
	{ do_OK,		1 },
	{ do_ANYKEY,	1 },
	{ do_SAVE,		1 },
	{ do_LOAD,		1 },
	{ do_DPRINT,	1 },
	{ do_DISPLAY,	1 },
	{ do_CLS,		1 },
	{ do_DROPALL,	1 },
	{ do_AUTOG,		1 },
	{ do_AUTOD,		1 },
	{ do_AUTOW,		1 },
	{ do_AUTOR,		1 },
	{ do_PAUSE,		1 },
	{ do_SYNONYM,	1 },
	{ do_GOTO,		1 },
	{ do_MESSAGE,	1 },
	{ do_REMOVE,	1 },
	{ do_GET,		1 },
	{ do_DROP,		1 },
	{ do_WEAR,		1 },
	{ do_DESTROY,	1 },
	{ do_CREATE,	1 },
	{ do_SWAP,		1 },
	{ do_PLACE,		1 },
	{ do_SET,		1 },
	{ do_CLEAR,		1 },
	{ do_PLUS,		1 },
	{ do_MINUS,		1 },
	{ do_LET,		1 },
	{ do_NEWLINE,	1 },
	{ do_PRINT,		1 },
	{ do_SYSMESS,	1 },
	{ do_ISAT,		0 },
	{ do_SETCO,		1 },
	{ do_SPACE,		1 },
	{ do_HASAT,		0 },
	{ do_HASNAT,	0 },
	{ do_LISTOBJ,	1 },
	{ do_EXTERN,	1 },
	{ do_RAMSAVE,	1 },
	{ do_RAMLOAD,	1 },
	{ do_BEEP,		1 },
	{ do_PAPER,		1 },
	{ do_INK,		1 },
	{ do_BORDER,	1 },
	{ do_PREP,		0 },
	{ do_NOUN2,		0 },
	{ do_ADJECT2,	0 },
	{ do_ADD,		1 },
	{ do_SUB,		1 },
	{ do_PARSE,		1 },
	{ do_LISTAT,	1 },
	{ do_PROCESS,	1 },
	{ do_SAME,		0 },
	{ do_MES,		1 },
	{ do_WINDOW,	1 },
	{ do_NOTEQ,		0 },
	{ do_NOTSAME,	0 },
	{ do_MODE,		1 },
	{ do_WINAT,		1 },
	{ do_TIME,		1 },
	{ do_PICTURE,	1 },
	{ do_DOALL,		1 },
	{ do_MOUSE,		1 },
	{ do_GFX,		1 },
	{ do_ISNOTAT,	0 },
	{ do_WEIGH,		1 },
	{ do_PUTIN,		1 },
	{ do_TAKEOUT,	1 },
	{ do_NEWTEXT,	1 },
	{ do_ABILITY,	1 },
	{ do_WEIGHT,	1 },
	{ do_RANDOM,	1 },
	{ do_INPUT,		1 },
	{ do_SAVEAT,	1 },
	{ do_BACKAT,	1 },
	{ do_PRINTAT,	1 },
	{ do_WHATO,		1 },
	{ do_CALL,		1 },
	{ do_PUTO,		1 },
	{ do_NOTDONE,	0 },
	{ do_AUTOP,		1 },
	{ do_AUTOT,		1 },
	{ do_MOVE,		1 },
	{ do_WINSIZE,	1 },
	{ do_REDO,		1 },
	{ do_CENTRE,	1 },
	{ do_EXIT,		1 },
	{ do_INKEY,		0 },
	{ do_BIGGER,	0 },
	{ do_SMALLER,	0 },
	{ do_ISDONE,	0 },
	{ do_ISNDONE,	0 },
	{ do_SKIP,		1 },
	{ do_RESTART,	1 },
	{ do_TAB,		1 },
	{ do_COPYOF,	1 },
	{ do_NOT_USED,	1 },
	{ do_COPYOO,	1 },
	{ do_NOT_USED,	1 },
	{ do_COPYFO,	1 },
	{ do_NOT_USED,	1 },
	{ do_COPYFF,	1 },
	{ do_COPYBF,	1 },
	{ do_RESET,		1 },
};


//==============================================================================

//===============================================
void initializePROC()
{
#ifdef VERBOSE
printf("initializePROC()\n");
#endif
	memset(procStack, 0, sizeof(procStack));
	currProc = procStack-1;
}

//===============================================
void pushPROC(uint8_t proc)
{
#ifdef VERBOSE
printf("pushPROC(%u)\n",proc);
#endif
	currProc++;
	currProc->entryIni = getPROCess(proc);
	currProc->entry = currProc->entryIni - 1;
	checkEntry = false;
}

//===============================================
bool popPROC()
{
#ifdef VERBOSE
printf("popPROC()\n");
#endif
	if (currProc > procStack) memset(currProc, 0, sizeof(PROCstack));
	currProc--;
	checkEntry = true;
	lastIsDone = isDone;
	isDone = false;
	return (currProc >= procStack);	//Any PROCs in the stack?
}

//===============================================
PROCentry* getPROCess(uint8_t proc)
{
#ifdef VERBOSE
printf("getPROCess(%u)\n",proc);
printf("    Pos: %p\n",*(((uint16_t*)hdr->prcLstPos) + proc));
#endif
	return (PROCentry*)&ddb[*(((uint16_t*)hdr->prcLstPos) + proc)];
}

//===============================================
char* getPROCEntryCondacts()
{
#ifdef VERBOSE
printf("getPROCEntryCondacts()\n");
printf("    Pos: %p\n",(uint16_t)(currProc->entry->pCondacts));
#endif
	return &ddb[(uint16_t)(currProc->entry->pCondacts)];
}

//===============================================
char* stepPROCEntryCondacts(int8_t step)
{
	currProc->entry += step;
	checkEntry = false;
}

//===============================================
void processPROC()
{
#ifdef VERBOSE
printf("processPROC()\n");
printf("    Pos: %p\n",((char*)currProc->entryIni) - ddb);
#endif
	CondactStruct *currCondact;
	uint8_t temp;

	//Clear ISDONE flags
	checkEntry = isDone = lastIsDone = false;

	for (;;) {
		//Execute condacts until 0xff (entry end) is found
		while (checkEntry && (temp=*pPROC)!=0xff) {
			currCondact = (CondactStruct*)pPROC++;
			indirection = currCondact->indirection;
#ifdef VERBOSE
printf("     [%p][%3u] %s ",(char*)(pPROC-1)-ddb, *((uint8_t*)currCondact), CONDACTS[currCondact->condact].name);
if (CONDACTS[currCondact->condact].args>=1) {
	if (indirection) printf("[");
	printf("%u", *(pPROC));
	if (indirection) printf("]");
}
if (CONDACTS[currCondact->condact].args>=2) printf(" %u", *(pPROC+1));
printf("\n");
#endif
			condactList[currCondact->condact].function();
			if (!isDone) isDone |= condactList[currCondact->condact].flag;
		}
		do {
			currProc->entry++;
			if (currProc->entry->verb==0) {
				popPROC();
				break;
			} else {
				currProc->condactIni = getPROCEntryCondacts();
				currProc->condact = currProc->condactIni;
			}
#ifdef VERBOSE
printf("  VERB:%u NOUN:%u [Pos:%p]\n",currProc->entry->verb, currProc->entry->noun, (char*)currProc->entry-ddb);
#endif
		} while (!((currProc->entry->verb==NULLWORD || currProc->entry->verb==flags[fVerb]) &&
				 (currProc->entry->noun==NULLWORD || currProc->entry->noun==flags[fNoun1])));
#ifdef VERBOSE
printf("  ======================> VERB+NOUN OK\n");
#endif
		lastIsDone = isDone;
		isDone = false;
		checkEntry = true;
	}
}
// TODO High Priority:
//		DOALL
//		SAVE
//		LOAD
//      MODE
//      INPUT
//      TIME		//TODO SYS32 "More..." if MODE option enabled
//
// TODO Low Priority:
//      EXTERN
//      CALL
//      ANYKEY		//TODO check for timeout
//      PARSE		//TODO PARSE 1
//      GET 		//TODO cancel DOALL loop
//      TAKEOUT 	//TODO cancel DOALL loop
//      RESTART 	//TODO cancel DOALL loop
//
// TODO Undocumented:
//      SFX 		//Undocumented
//      GFX 		//Undocumented
//      MOUSE 		//Undocumented
//      BEEP 		//Undocumented

//===============================================
uint8_t getValueOrIndirection()
{
	uint8_t value = *pPROC++;
	return indirection ? flags[value] : value;
}



// =============================================================================
// Conditions of player locations [4 condacts]

/*	Succeeds if the current location is the same as locno. */
#ifndef DISABLE_AT
void do_AT()		// locno 
{
	checkEntry = (getValueOrIndirection() == flags[fPlayer]);
}
#endif
/*	Succeeds if the current location is different to locno. */
#ifndef DISABLE_NOTAT
void do_NOTAT()		// locno
{
	checkEntry = (getValueOrIndirection() != flags[fPlayer]);
}
#endif
/*	Succeeds if the current location is greater than locno. */
#ifndef DISABLE_ATGT
void do_ATGT()		// locno
{
	checkEntry = (getValueOrIndirection() > flags[fPlayer]);
}
#endif
/*	Succeeds if the current location is less than locno. */
#ifndef DISABLE_ATLT
void do_ATLT()		// locno
{
	checkEntry = (getValueOrIndirection() < flags[fPlayer]);
}
#endif

// =============================================================================
// Conditions of object locations [8 condacts]

/*	Succeeds if Object objno. is carried (254), worn (253) or at the current 
	location [fPlayer]. */
#ifndef DISABLE_PRESENT
void do_PRESENT()	// objno
{
	Object *obj = &objects[getValueOrIndirection()];
	if (obj->location<LOC_NOTCREATED && obj->location!=flags[fPlayer]) {
		checkEntry = false;
	}
}
#endif
/*	Succeeds if Object objno. is not carried (254), not worn (253) and not at 
	the current location [fPlayer]. */
#ifndef DISABLE_ABSENT
void do_ABSENT()	// objno
{	
	Object *obj = &objects[getValueOrIndirection()];
	if (obj->location>=LOC_NOTCREATED || obj->location==flags[fPlayer]) {
		checkEntry = false;
	}
}
#endif
/*	Succeeds if object objno. is worn. */
#ifndef DISABLE_WORN
void do_WORN()		// objno
{
	checkEntry = (objects[getValueOrIndirection()].location==LOC_WORN);
}
#endif
/*	Succeeds if Object objno. is not worn. */
#ifndef DISABLE_NOTWORN
void do_NOTWORN()	// objno
{
	checkEntry = (objects[getValueOrIndirection()].location!=LOC_WORN);
}
#endif
/*	Succeeds if Object objno. is carried. */
#ifndef DISABLE_CARRIED
void do_CARRIED()	// objno
{
	checkEntry = (objects[getValueOrIndirection()].location==LOC_CARRIED);
}
#endif
/*	Succeeds if Object objno. is not carried. */
#ifndef DISABLE_NOTCARR
void do_NOTCARR()
{
	checkEntry = (objects[getValueOrIndirection()].location!=LOC_CARRIED);
}
#endif
/*	Succeeds if Object objno. is at Location locno. */
#ifndef DISABLE_ISAT
void do_ISAT()		// objno locno+
{
	checkEntry = (objects[getValueOrIndirection()].location==*pPROC++);
}
#endif
/*	Succeeds if Object objno. is not at Location locno. */
#ifndef DISABLE_ISNOTAT
void do_ISNOTAT()	// objno locno+
{
	checkEntry = (objects[getValueOrIndirection()].location!=*pPROC++);
}
#endif

// =============================================================================
// Conditions for values/flags comparation [10 condacts]

/*	Succeeds if Flag flagno. is set to zero. */
#ifndef DISABLE_ZERO
void do_ZERO()		// flagno
{
	checkEntry = !flags[getValueOrIndirection()];
}
#endif
/*	Succeeds if Flag flagno. is not set to zero. */
#ifndef DISABLE_NOTZERO
void do_NOTZERO()	// flagno
{
	checkEntry = flags[getValueOrIndirection()];
}
#endif
/*	Succeeds if Flag flagno. is equal to value. */
#ifndef DISABLE_EQ
void do_EQ()		// flagno value
{
	checkEntry = (flags[getValueOrIndirection()] == *pPROC++);
}
#endif
/*	Succeeds if Flag flagno. is not equal to value. */
#ifndef DISABLE_NOTEQ
void do_NOTEQ()		// flagno value
{
	checkEntry = (flags[getValueOrIndirection()] != *pPROC++);
}
#endif
/*	Succeeds if Flag flagno. is greater than value. */
#ifndef DISABLE_GT
void do_GT()		// flagno value
{
	checkEntry = (flags[getValueOrIndirection()] > *pPROC++);
}
#endif
/*	Succeeds if Flag flagno. is set to less than value. */
#ifndef DISABLE_LT
void do_LT()		// flagno value
{
	checkEntry = (flags[getValueOrIndirection()] < *pPROC++);
}
#endif
/*	Succeeds if Flag flagno 1 has the same value as Flag flagno 2. */
#ifndef DISABLE_SAME
void do_SAME()		// flagno1 flagno2
{
	checkEntry = (flags[getValueOrIndirection()] == flags[*pPROC++]);
}
#endif
/*	Succeeds if Flag flagno 1 does not have the same value as Flag flagno 2 . */
#ifndef DISABLE_NOTSAME
void do_NOTSAME()	// flagno1 flagno2
{
	checkEntry = (flags[getValueOrIndirection()] != flags[*pPROC++]);
}
#endif
/*	Will be true if flagno 1 is larger than flagno 2 */
#ifndef DISABLE_BIGGER
void do_BIGGER()	// flagno1 flagno2
{
	checkEntry = (flags[getValueOrIndirection()] > flags[*pPROC++]);
}
#endif
/*	Will be true if flagno 1 is smaller than flagno 2 */
#ifndef DISABLE_SMALLER
void do_SMALLER()	// flagno1 flagno2
{
	checkEntry = (flags[getValueOrIndirection()] < flags[*pPROC++]);
}
#endif

// =============================================================================
// Conditions to check logical sentence [5 condacts]

/*	Succeeds if the first noun's adjective in the current LS is word. */
#ifndef DISABLE_ADJECT1
void do_ADJECT1()
{
	checkEntry = (flags[fAdject1] == *pPROC++);
}
#endif
/*	Succeeds if the adverb in the current LS is word. */
#ifndef DISABLE_ADVERB
void do_ADVERB()
{
	checkEntry = (flags[fAdverb] == *pPROC++);
}
#endif
/*	Succeeds if the preposition in the current LS is word. */
#ifndef DISABLE_PREP
void do_PREP()
{
	checkEntry = (flags[fPrep] == *pPROC++);
}
#endif
/*	Succeeds if the second noun in the current LS is word. */
#ifndef DISABLE_NOUN2
void do_NOUN2()
{
	checkEntry = (flags[fNoun2] == *pPROC++);
}
#endif
/*	Succeeds if the second noun's adjective in the current LS is word. */
#ifndef DISABLE_ADJECT2
void do_ADJECT2()
{
	checkEntry = (flags[fAdject2] == *pPROC++);
}
#endif

// =============================================================================
// Conditions for random [1 condacts]

/*	Succeeds if percent is less than or equal to a random number in the range 
	1-100 (inclusive). Thus a CHANCE 50 condition would allow PAW to look at the 
	next CondAct only if the random number generated was between 1 and 50, a 50% 
	chance of success. */
#ifndef DISABLE_CHANCE
void do_CHANCE()	// percent
{
	checkEntry = (rand()%100)+1 < getValueOrIndirection();
}
#endif

// =============================================================================
// Conditions for sub-process success/fail [2 condacts]

/*	Succeeds if the last table ended by exiting after executing at least one 
	Action. This is useful to test for a single succeed/fail boolean value from 
	a Sub-Process. A DONE action will cause the 'done' condition, as will any 
	condact causing exit, or falling off the end of the table - assuming at 
	least one CondAct (other than NOTDONE) was done.
	See also ISNDONE and NOTDONE actions. */
#ifndef DISABLE_ISDONE
void do_ISDONE()
{
	checkEntry = lastIsDone;
}
#endif
/*	Succeeds if the last table ended without doing anything or with a NOTDONE 
	action. */
#ifndef DISABLE_ISNDONE
void do_ISNDONE()
{
	checkEntry = !lastIsDone;
}
#endif

// =============================================================================
// Conditions for object attributes [2 condacts]

#if !defined(DISABLE_HASAT) || !defined(DISABLE_HASNAT)
void _internal_hasat(uint8_t value, bool negate)
{
	uint8_t flag, bit;
	switch (value) {
		case 0:  flag=fCOAtt;   bit=0b00000001; break;
		case 1:  flag=fCOAtt;   bit=0b00000010; break;
		case 2:  flag=fCOAtt;   bit=0b00000100; break;
		case 3:  flag=fCOAtt;   bit=0b00001000; break;
		case 4:  flag=fCOAtt;   bit=0b00010000; break;
		case 5:  flag=fCOAtt;   bit=0b00100000; break;
		case 6:  flag=fCOAtt;   bit=0b01000000; break;
		case 7:  flag=fCOAtt;   bit=0b10000000; break;
		case 8:  flag=fCOAtt+1; bit=0b00000001; break;
		case 9:  flag=fCOAtt+1; bit=0b00000010; break;
		case 10: flag=fCOAtt+1; bit=0b00000100; break;
		case 11: flag=fCOAtt+1; bit=0b00001000; break;
		case 12: flag=fCOAtt+1; bit=0b00010000; break;
		case 13: flag=fCOAtt+1; bit=0b00100000; break;
		case 14: flag=fCOAtt+1; bit=0b01000000; break;
		case 15: flag=fCOAtt+1; bit=0b10000000; break;
		case HA_WAREABLE:  flag=fCOWR;    bit=0b10000000;   break; // Flag 57 Bit#7
		case HA_CONTAINER: flag=fCOCon;   bit=0b10000000;   break; // Flag 56 Bit#7
		case HA_LISTED:    flag=fOFlags;  bit=0b10000000;   break; // Flag 53 Bit#7
		case HA_TIMEOUT:   flag=fTIFlags; bit=TIME_TIMEOUT; break; // Flag 49 Bit#7
		case HA_MOUSE:     flag=fGFlags;  bit=0b00000001;   break; // Flag 29 Bit#0
		case HA_GMODE:     flag=fGFlags;  bit=0b10000000;   break; // Flag 29 Bit#7
	#ifdef DEBUG
		default:
				die("===== HASAT/HASNAT value not implemented\n");
	#endif
	}
	flag = flags[flag] & bit;
	if (negate) flag = !flag;
	checkEntry = flag;
}
#endif
/*	Checks the attribute specified by value. 0-15 are the object attributes for 
	the current object. There are also several attribute numbers specified as 
	symbols in SYMBOLS.SCE which check certain parts of the DAAD system flags */
#ifndef DISABLE_HASAT
void do_HASAT()		// value
{
	_internal_hasat(getValueOrIndirection(), false);
}
#endif
#ifndef DISABLE_HASNAT
void do_HASNAT()	// value
{
	_internal_hasat(getValueOrIndirection(), true);
}
#endif

// =============================================================================
// Conditions for player interaction [2 condacts]

/*	Is a condition which will be satisfied if the player is pressing a key. 
	In 16Bit machines Flags Key1 and Key2 (60 & 61) will be a standard IBM ASCII 
	code pair.
	On 8 bit only Key1 will be valid, and the code will be machine specific. */
#ifndef DISABLE_INKEY
void do_INKEY()
{
	while (kbhit()) getchar();
	while (!kbhit());
	flags[fKey1] = getchar();
}
#endif
/*	SM12 ("Are you sure?") is printed and called. Will succeed if the player replies
	starts with the first letter of SM30 ("Y") to then the remainder of the entry is 
	discarded is carried out. */
#ifndef DISABLE_QUIT
void do_QUIT()
{
	printSystemMsg(12);
	do_NEWLINE();
	clearLogicalSentences();
	prompt();
	char c = *tmpMsg;
	getSystemMsg(30);
	if (*tmpMsg==c) do_END();
	checkEntry = false;
}
#endif

// =============================================================================
// Actions for object position manipulation [21 condacts]

/*	If Object objno. is worn or carried, SM25 ("I already have the _.") is printed 
	and actions NEWTEXT & DONE are performed.
	
	If Object objno. is not at the current location, SM26 ("There isn't one of 
	those here.") is printed and actions NEWTEXT & DONE are performed.

	If the total weight of the objects carried and worn by the player plus 
	Object objno. would exceed the maximum conveyable weight (Flag 52) then SM43 
	("The _ weighs too much for me.") is printed and actions NEWTEXT & DONE are 
	performed.

	If the maximum number of objects is being carried (Flag 1 is greater than, 
	or the same as, Flag 37), SM27 ("I can't carry any more things.") is printed 
	and actions NEWTEXT & DONE are performed. In addition any current DOALL loop 
	is cancelled.

	Otherwise the position of Object objno. is changed to carried, Flag 1 is 
	incremented and SM36 ("I now have the _.") is printed. */
#if !defined(DISABLE_GET) || !defined(DISABLE_AUTOG)
void _internal_get(uint8_t objno)
{
	Object *obj = objects + objno;
	referencedObject(objno);
	if (obj->location==LOC_CARRIED || obj->location==LOC_WORN) {
		printSystemMsg(25);
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location!=flags[fPlayer]) {
		printSystemMsg(26);
		do_NEWTEXT();
		do_DONE();
	} else
	if (getObjectWeight(NULLWORD, true)+getObjectWeight(objno, false) > flags[fStrength]) {
		printSystemMsg(43);
		do_NEWTEXT();
		do_DONE();
	} else
	if (flags[fNOCarr] >= flags[fMaxCarr]) {
		printSystemMsg(27);
		do_NEWTEXT();
		do_DONE();
		//TODO cancel DOALL loop
	} else {
		printSystemMsg(36);
		obj->location = LOC_CARRIED;
		flags[fNOCarr]++;
	}
}
#endif
#ifndef DISABLE_GET
void do_GET()		// objno
{
	_internal_get(getValueOrIndirection());
}
#endif
/*	If Object objno. is worn then SM24 ("I can't. I'm wearing the _.") is 
	printed and actions NEWTEXT & DONE are performed.

	If Object objno. is at the current location (but neither worn nor carried), 
	SM49 ("I don't have the _.") is printed and actions NEWTEXT & DONE are 
	performed.

	If Object objno. is not at the current location then SM28 ("I don't have one 
	of those.") is printed and actions NEWTEXT & DONE are performed.

	Otherwise the position of Object objno. is changed to the current location, 
	Flag 1 is decremented and SM39 ("I've dropped the _.") is printed. */
#if !defined(DISABLE_DROP) || !defined(DISABLE_AUTOD)
void _internal_drop(uint8_t objno)
{
	Object *obj = objects + objno;
	referencedObject(objno);
	if (obj->location==LOC_CARRIED) {
		obj->location = flags[fPlayer];
		printSystemMsg(39);
		do_NEWTEXT();
		do_DONE();
		flags[fNOCarr]--;
	} else
	if (obj->location==LOC_WORN) {
		printSystemMsg(24);
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location==flags[fPlayer]) {
		printSystemMsg(49);
		do_NEWTEXT();
		do_DONE();
	} else {
		printSystemMsg(28);
		do_NEWTEXT();
		do_DONE();
	}
}
#endif
#ifndef DISABLE_DROP
void do_DROP()		// objno
{
	_internal_drop(getValueOrIndirection());
}
#endif
/*	If Object objno. is at the current location (but not carried or worn) SM49 
	("I don't have the _.") is printed and actions NEWTEXT & DONE are 
	performed.

	If Object objno. is worn, SM29 ("I'm already wearing the _.") is printed 
	and actions NEWTEXT & DONE are performed.

	If Object objno. is not carried, SM28 ("I don't have one of those.") is 
	printed and actions NEWTEXT & DONE are performed.

	If Object objno. is not wearable (as specified in the object definition 
	section) then SM40 ("I can't wear the _.") is printed and actions NEWTEXT & 
	DONE are performed.

	Otherwise the position of Object objno. is changed to worn, Flag 1 is 
	decremented and SM37 ("I'm now wearing the _.") is printed. */
#if !defined(DISABLE_WEAR) || !defined(DISABLE_AUTOW)
void _internal_wear(uint8_t objno)
{
	Object *obj = objects + objno;
	referencedObject(objno);
	if (obj->location==flags[fPlayer]) {
		printSystemMsg(49);
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location==LOC_WORN) {
		printSystemMsg(29);
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location!=LOC_CARRIED) {
		printSystemMsg(28);
		do_NEWTEXT();
		do_DONE();
	} else
	if (!obj->attribs.mask.isWareable) {
		printSystemMsg(40);
		do_NEWTEXT();
		do_DONE();
	} else {
		printSystemMsg(37);
		obj->location = LOC_WORN;
		flags[fNOCarr]--;
	}
}
#endif
#ifndef DISABLE_WEAR
void do_WEAR()		// objno
{
	_internal_wear(getValueOrIndirection());
}
#endif
/*	If Object objno. is carried or at the current location (but not worn) then 
	SM50 ("I'm not wearing the _.") is printed and actions NEWTEXT & DONE are 
	performed.

	If Object objno. is not at the current location, SM23 ("I'm not wearing one 
	of those.") is printed and actions NEWTEXT & DONE are performed.

	If Object objno. is not wearable (and thus removable) then SM41 ("I can't 
	remove the _.") is printed and actions NEWTEXT & DONE are performed.

	If the maximum number of objects is being carried (Flag 1 is greater than, 
	or the same as, Flag 37), SM42 ("I can't remove the _. My hands are full.") 
	is printed and actions NEWTEXT & DONE are performed.

	Otherwise the position of Object objno. is changed to carried. Flag 1 is 
	incremented and SM38 ("I've removed the _.") printed. */
#if !defined(DISABLE_REMOVE) || !defined(DISABLE_AUTOR)
void _internal_remove(uint8_t objno)
{
	Object *obj = objects + objno;
	referencedObject(objno);
	if (obj->location==LOC_CARRIED || obj->location==flags[fPlayer]) {
		printSystemMsg(50);
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location!=flags[fPlayer]) {
		printSystemMsg(23);
		do_NEWTEXT();
		do_DONE();
	} else
	if (!obj->attribs.mask.isWareable) {
		printSystemMsg(41);
		do_NEWTEXT();
		do_DONE();
	} else
	if (flags[fNOCarr] >= flags[fMaxCarr]) {
		printSystemMsg(42);
		do_NEWTEXT();
		do_DONE();
	} else {
		printSystemMsg(38);
		obj->location = LOC_CARRIED;
		flags[fNOCarr]++;
	}
}
#endif
#ifndef DISABLE_REMOVE
void do_REMOVE()	// objno
{
	_internal_remove(getValueOrIndirection());
}
#endif
/*	The position of Object objno. is changed to the current location and Flag 1
	is decremented if the object was carried. */
#ifndef DISABLE_CREATE
void do_CREATE()	// objno
{
	uint8_t objno = getValueOrIndirection();
	Object *obj = objects + objno;
	referencedObject(objno);
	if (obj->location==LOC_CARRIED) flags[fNOCarr]--;
	obj->location = flags[fPlayer];
}
#endif
/*	The position of Object objno. is changed to not-created and Flag 1 is 
	decremented if the object was carried. */
#ifndef DISABLE_DESTROY
void do_DESTROY()	// objno
{
	uint8_t objno = getValueOrIndirection();
	Object *obj = objects + objno;
	referencedObject(objno);
	if (obj->location==LOC_CARRIED) flags[fNOCarr]--;
	obj->location = LOC_NOTCREATED;
}
#endif
/*	The positions of the two objects are exchanged. Flag 1 is not adjusted. The 
	currently referenced object is set to be Object objno 2. */
#ifndef DISABLE_SWAP
void do_SWAP()		// objno1 objno2
{
	Object *obj1 = objects + getValueOrIndirection();
	uint8_t objno2 = *pPROC++;
	Object *obj2 = objects + objno2;
	uint8_t aux = obj1->location;
	obj1->location = obj2->location;
	obj2->location = aux;
	referencedObject(objno2);
}
#endif
/*	The position of Object objno. is changed to Location locno. Flag 1 is 
	decremented if the object was carried. It is incremented if the object is 
	placed at location 254 (carried). */
#ifndef DISABLE_PLACE
void do_PLACE()		// objno locno+
{
	uint8_t objno = getValueOrIndirection();
	Object *obj = objects + objno;
	referencedObject(objno);
	if (obj->location==LOC_CARRIED) flags[fNOCarr]--;
	obj->location = *pPROC++;
}
#endif
/*	The position of the currently referenced object (i.e. that object whose 
	number is given in flag 51), is changed to be Location locno. Flag 54 
	remains its old location. Flag 1 is decremented if the object was carried. 
	It is incremented if the object is placed at location 254 (carried). */
#ifndef DISABLE_PUTO
void do_PUTO()		// locno+
{
	Object *obj = objects + flags[fCONum];
	if (obj->location==LOC_CARRIED) flags[fNOCarr]--;
	obj->location = getValueOrIndirection();
	if (obj->location==LOC_CARRIED) flags[fNOCarr]++;
}
#endif
/*	If Object objno. is worn then SM24 ("I can't. I'm wearing the _.") is 
	printed and actions NEWTEXT & DONE are performed.

	If Object objno. is at the current location (but neither worn nor carried), 
	SM49 ("I don't have the _.") is printed and actions NEWTEXT & DONE are 
	performed.

	If Object objno. is not at the current location, but not carried, then SM28 
	("I don't have one of those.") is printed and actions NEWTEXT & DONE are 
	performed.

	Otherwise the position of Object objno. is changed to Location locno. 
	Flag 1 is decremented and SM44 ("The _ is in the"), a description of Object 
	locno. and SM51 (".") is printed. */
#if !defined(DISABLE_PUTIN) || !defined(DISABLE_AUTOP)
void _internal_putin(uint8_t objno, uint8_t locno)
{
	Object *obj = objects + objno;
	if (obj->location==LOC_WORN) {
		printSystemMsg(24);
		do_NEWTEXT();
		do_DONE();		
	} else
	if (obj->location==flags[fPlayer]) {
		printSystemMsg(49);
		do_NEWTEXT();
		do_DONE();		
	} else
	if (obj->location!=flags[fPlayer] && obj->location!=LOC_CARRIED) {
		printSystemMsg(28);
		do_NEWTEXT();
		do_DONE();		
	} else {
		obj->location = locno;
		flags[fNOCarr]--;
		printSystemMsg(44);
		do_SPACE();
		printObjectMsg(objno);
		do_SPACE();
		printSystemMsg(51);
	}
}
#endif
#ifndef DISABLE_PUTIN
void do_PUTIN()		// objno locno
{
	_internal_putin(getValueOrIndirection(), *pPROC++);
}
#endif
/*	If Object objno. is worn or carried, SM25 ("I already have the _.") is printed 
	and actions NEWTEXT & DONE are performed.

	If Object objno. is at the current location, SM45 ("The _ isn't in the"), a 
	description of Object locno. and SM51 (".") is printed and actions NEWTEXT & 
	DONE are performed.

	If Object objno. is not at the current location and not at Location locno. 
	then SM52 ("There isn't one of those in the"), a description of Object locno. 
	and SM51 (".") is printed and actions NEWTEXT & DONE are performed.

	If Object locno. is not carried or worn, and the total weight of the objects 
	carried and worn by the player plus Object objno. would exceed the maximum 
	conveyable weight (Flag 52) then SM43 ("The _ weighs too much for me.") is 
	printed and actions NEWTEXT & DONE are performed.

	If the maximum number of objects is being carried (Flag 1 is greater than, 
	or the same as, Flag 37), SM27 ("I can't carry any more things.") is printed 
	and actions NEWTEXT & DONE are performed. In addition any current DOALL loop 
	is cancelled. 

	Otherwise the position of Object objno. is changed to carried, Flag 1 is 
	incremented and SM36 ("I now have the _.") is printed.Note: No check is made, 
	by either PUTIN or TAKEOUT, that Object locno. is actually present. This must 
	be carried out by you if required. */
#if !defined(DISABLE_TAKEOUT) || !defined(DISABLE_AUTOT)
void _internal_takeout(uint8_t objno, uint8_t locno)
{
	Object *obj = &objects[objno];
	if (obj->location==LOC_WORN || obj->location==LOC_CARRIED) {
		printSystemMsg(25);
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location==flags[fPlayer]) {
		printSystemMsg(45);
		do_SPACE();
		printObjectMsg(locno);
		do_SPACE();
		printSystemMsg(51);
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location!=flags[fPlayer] || obj->location!=locno) {
		printSystemMsg(52);
		do_SPACE();
		printObjectMsg(locno);
		do_SPACE();
		printSystemMsg(51);
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location!=LOC_WORN && obj->location!=LOC_CARRIED && 
		getObjectWeight(NULLWORD, true)+getObjectWeight(objno, false) > flags[fStrength]) {
		printSystemMsg(43);
		do_NEWTEXT();
		do_DONE();
	} else
	if (flags[fNOCarr] >= flags[fMaxCarr]) {
		printSystemMsg(27);
		do_NEWTEXT();
		do_DONE();
		//TODO cancel DOALL loop
	} else {
		printSystemMsg(36);
		obj->location = LOC_CARRIED;
		flags[fNOCarr]++;
	}
}
#endif
#ifndef DISABLE_TAKEOUT
void do_TAKEOUT()	// objno locno
{
	_internal_takeout(getValueOrIndirection(), *pPROC++);
}
#endif
/*	All objects which are carried or worn are created at the current location (i.e. 
	all objects are dropped) and Flag 1 is set to 0. This is included for 
	compatibility with older writing systems.
	Note that a DOALL 254 will carry out a true DROP ALL, taking care of any special 
	actions included. */
#ifndef DISABLE_DROPALL
void do_DROPALL()
{
	int i=0;
	do {
		objects[i].location = flags[fPlayer];
	} while(i++ < sizeof(objects));
	flags[fNOCarr] = 0;
}
#endif
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; here, carried, 
	worn. i.e. The player is more likely to be trying to GET an object that is 
	at the current location than one that is carried or worn. If an object is 
	found its number is passed to the GET action. Otherwise if there is an 
	object in existence anywhere in the game or if Noun1 was not in the 
	vocabulary then SM26 ("There isn't one of those here.") is printed. Else 
	SM8 ("I can't do that.") is printed (i.e. It is not a valid object but does 
	exist in the game). Either way actions NEWTEXT & DONE are performed */
#ifndef DISABLE_AUTOG
void do_AUTOG()
{
	uint8_t objno = getObjectById(flags[fNoun1], flags[fAdject1]);
	if (objno==NULLWORD)
		printSystemMsg(26);
	else
		_internal_get(objno);
}
#endif
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; carried, worn, 
	here. i.e. The player is more likely to be trying to DROP a carried object 
	than one that is worn or here. If an object is found its number is passed 
	to the DROP action. Otherwise if there is an object in existence anywhere 
	in the game or if Noun1 was not in the vocabulary then SM28 ("I don't have
	one of those.") is printed. Else SM8 ("I can't do that.") is printed (i.e. 
	It is not a valid object but does exist in the game). Either way actions 
	NEWTEXT & DONE are performed */
#ifndef DISABLE_AUTOD
void do_AUTOD()
{
	uint8_t objno = getObjectById(flags[fNoun1], flags[fAdject1]);
	if (objno==NULLWORD)
		printSystemMsg(28);
	else
		_internal_drop(objno);
}
#endif
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; carried, worn, 
	here. i.e. The player is more likely to be trying to WEAR a carried object 
	than one that is worn or here. If an object is found its number is passed 
	to the WEAR action. Otherwise if there is an object in existence anywhere 
	in the game or if Noun1 was not in the vocabulary then SM28 ("I don't have
	one of those.") is printed. Else SM8 ("I can't do that.") is printed (i.e. 
	It is not a valid object but does exist in the game). Either way actions 
	NEWTEXT & DONE are performed */
#ifndef DISABLE_AUTOW
void do_AUTOW()
{
	uint8_t objno = getObjectById(flags[fNoun1], flags[fAdject1]);
	if (objno==NULLWORD)
		printSystemMsg(28);
	else
		_internal_wear(objno);
}
#endif
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; worn, carried, 
	here. i.e. The player is more likely to be trying to REMOVE a worn object 
	than one that is carried or here. If an object is found its number is passed 
	to the REMOVE action. Otherwise if there is an object in existence anywhere 
	in the game or if Noun1 was not in the vocabulary then SM23 ("I'm not 
	wearing one of those.") is printed. Else SM8 ("I can't do that.") is printed 
	(i.e. It is not a valid object but does exist in the game). Either way 
	actions NEWTEXT & DONE are performed */
#ifndef DISABLE_AUTOR
void do_AUTOR()
{
	uint8_t objno = getObjectById(flags[fNoun1], flags[fAdject1]);
	if (objno==NULLWORD)
		printSystemMsg(23);
	else
		_internal_remove(objno);
}
#endif
/*	A search for the object number represented by Noun(Adjective)1 is made in the 
	object definition section in order of location priority; carried, worn, here. 
	i.e. The player is more likely to be trying to PUT a carried object inside 
	another than one that is worn or here. If an object is found its number is 
	passed to the PUTIN action. Otherwise if there is an object in existence
	anywhere in the game or if Noun1 was not in the vocabulary then SM28 ("I don't 
	have one of those.") is printed. Else SM8 ("I can't do that.") is printed 
	(i.e. It is not a valid object but does exist in the game). Either way actions 
	NEWTEXT & DONE are performed */
#ifndef DISABLE_AUTOP
void do_AUTOP()		// locno
{
	uint8_t objno = getObjectById(flags[fNoun1], flags[fAdject1]);
	if (objno==NULLWORD)
		printSystemMsg(28);
	else
		_internal_putin(objno, getValueOrIndirection());
}
#endif
/*	A search for the object number represented by Noun(Adjective)1 is made in the 
	object definition section in order of location priority; in container, 
	carried, worn, here. i.e. The player is more likely to be trying to get an 
	object out of a container which is actually in there than one that is carried, 
	worn or here. If an object is found its number is passed to the TAKEOUT action. 
	Otherwise if there is an object in existence anywhere in the game or if Noun1 
	was not in the vocabulary then SM52 ("There isn't one of those in the"), a 
	description of Object locno. and SM51 (".") is printed. Else SM8 ("I can't do 
	that.") is printed (i.e. It is not a valid object but does exist in the game).
	Either way actions NEWTEXT & DONE are performed */
#ifndef DISABLE_AUTOT
void do_AUTOT()		// locno
{
	uint8_t objno = getObjectById(flags[fNoun1], flags[fAdject1]);
	if (objno==NULLWORD)
		printSystemMsg(28);
	else
		_internal_takeout(objno, getValueOrIndirection());
}
#endif
/*	The position of Object objno2 is set to be the same as the position of 
	Object Objno1. The currently referenced object is set to be Object objno2 */
#ifndef DISABLE_COPYOO
void do_COPYOO()	// objno1 objno2
{
	uint8_t objno1 = getValueOrIndirection();
	uint8_t objno2 = *pPROC++;
	objects[objno2].location = objects[objno1].location;
	referencedObject(objno2);
}
#endif
/*	This Action bears no resemblance to the one with the same name in PAW. It has 
	the pure function of placing all objects at the position given in the Object 
	start table. It also sets the relevant flags dealing with no of objects 
	carried etc. */
#ifndef DISABLE_RESET
void do_RESET() {
	initObjects();
}
#endif

// =============================================================================
// Actions for object in flags manipulation [5 condacts]

/*	The position of Object objno. is copied into Flag flagno. This could be used 
	to examine the location of an object in a comparison with another flag value. */
#ifndef DISABLE_COPYOF
void do_COPYOF()	// objno flagno
{
	Object *obj = objects + getValueOrIndirection();
	flags[*pPROC++] = obj->location;
}
#endif
/*	The position of Object objno. is set to be the contents of Flag flagno. An 
	attempt to copy from a flag containing 255 will result in a run time error. 
	Setting an object to an invalid location will still be accepted as it 
	presents no danger to the operation of PAW. */
#ifndef DISABLE_COPYFO
void do_COPYFO()	// flagno objno
{
	flags[getValueOrIndirection()] = (objects + *pPROC++)->location;
}
#endif
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; carried, worn, 
	here. This is because it is assumed any use of WHATO will be related to 
	carried objects rather than any that are worn or here. If an object is found 
	its number is placedin flag 51, along with the standard current object 
	parameters in flags 54-57. This allows you to create other auto actions (the
	tutorial gives an example of this for dropping objects in the tree). */
#ifndef DISABLE_WHATO
void do_WHATO()
{
	uint8_t objno = getObjectById(flags[fNoun1], flags[fAdject1]);
	if (objno!=NULLWORD)
		referencedObject(objno);
}
#endif
/*	Sets the currently referenced object to objno. */
#ifndef DISABLE_SETCO
void do_SETCO()		// objno
{
	referencedObject(getValueOrIndirection());
}
#endif
/*	The true weight of Object objno. is calculated (i.e. if it is a container, 
	any objects inside have their weight added - don't forget that nested 
	containers stop adding their contents after ten levels) and the value is 
	placed in Flag flagno. This will have a maximum value of 255 which will not 
	be exceeded. If Object objno. is a container of zero weight, Flag flagno 
	will be cleared as objects in zero weight containers, also weigh zero! */
#ifndef DISABLE_WEIGH
void do_WEIGH()		// objno flagno
{
	uint8_t weight = getObjectWeight(getValueOrIndirection(), false);
	flags[*pPROC++] = weight;
}
#endif

// =============================================================================
// Actions to manipulate flags [11 condacts]

/*	Flag flagno. is set to 255. */
#ifndef DISABLE_SET
void do_SET()		// flagno
{
	flags[getValueOrIndirection()] = 255;
}
#endif
/*	Flag flagno. is cleared to 0. */
#ifndef DISABLE_CLEAR
void do_CLEAR()		// flagno
{
	flags[getValueOrIndirection()] = 0;
}
#endif
/*	Flag flagno. is set to value. */
#ifndef DISABLE_LET
void do_LET()		// flagno value
{
	flags[getValueOrIndirection()] = *pPROC++;
}
#endif
/*	Flag flagno. is increased by value. If the result exceeds 255 the flag is
	set to 255. */
#ifndef DISABLE_PLUS
void do_PLUS()		// flagno value
{
	uint8_t flagno = getValueOrIndirection();
	uint16_t value = (uint16_t)flags[flagno] + *pPROC++;
	if (value>255) value = 255;
	flags[flagno] = (uint8_t)value;
}
#endif
/*	Flag flagno. is decreased by value. If the result is negative the flag is 
	set to 0. */
#ifndef DISABLE_MINUS
void do_MINUS()		// flagno value
{
	uint8_t flagno = getValueOrIndirection();
	uint8_t value = *pPROC++;
	flags[flagno] = value > flags[flagno] ? 0 : flags[flagno]-value;
}
#endif
/*	Flag flagno 2 has the contents of Flag flagno 1 added to it. If the result 
	exceeds 255 the flag is set to 255. */
#ifndef DISABLE_ADD
void do_ADD()		// flagno1 flagno2
{
	uint16_t sum = flags[getValueOrIndirection()];
	uint8_t flagno2 = *pPROC++;
	sum += flags[flagno2];
	flags[flagno2] = sum>255 ? 255 : (uint8_t)sum;
}
#endif
/*	Flag flagno 2 has the contents of Flag flagno 1 subtracted from it. If the
	result is negative the flag is set to 0. */
#ifndef DISABLE_SUB
void do_SUB()		// flagno1 flagno2
{
	uint8_t subs = flags[getValueOrIndirection()];
	uint8_t flagno2 = *pPROC++;
	flags[flagno2] = subs > flags[flagno2] ? 0 : flags[flagno2]-subs;
}
#endif
/*	The contents of Flag flagno 1 is copied to Flag flagno 2. */
#ifndef DISABLE_COPYFF
void do_COPYFF()	// flagno1 flagno2
{
	uint8_t flagno1 = getValueOrIndirection();
	flags[*pPROC++] = flags[flagno1];
}
#endif
/*	Same as COPYFF but the source and destination are reversed, so that 
	indirection can be used. */
#ifndef DISABLE_COPYBF
void do_COPYBF()	// flagno1 flagno2
{
	flags[getValueOrIndirection()] = flags[*pPROC++];
}
#endif
/*	Flag flagno. is set to a number from the Pseudo-random sequence from 1 
	to 100. */
#ifndef DISABLE_RANDOM
void do_RANDOM()	// flagno
{
	flags[*pPROC++] = (rand()%100)+1;
}
#endif
/*	This is a very powerful action designed to manipulate PSI's. It allows the
	current LS Verb to be used to scan the connections section for the location 
	given in Flag flagno. 
	If the Verb is found then Flag flagno is changed to be the location number 
	associated with it, and the next condact is considered.
	If the verb is not found, or the original location number was invalid, then 
	PAW considers the next entry in the table - if present. */
#ifndef DISABLE_MOVE
void do_MOVE()		// flagno
{
	uint8_t flagno = getValueOrIndirection();

	if (flags[fVerb]<14) {
		uint8_t *cnx = &ddb[*(((uint16_t*)hdr->conLstPos) + flags[fPlayer])];

		while (*cnx != 0xff) {
			if (flags[fVerb]==*cnx) {
				flags[flagno] = *++cnx;
				return;
			}
			cnx+=2;
		}
	}
	checkEntry = false;
}
#endif

// =============================================================================
// Actions to manipulate player flags [3 condacts]

/*	Changes the current location to locno. This effectively sets flag 38 to the value
	locno. */
#ifndef DISABLE_GOTO
void do_GOTO()		// locno
{
	flags[fPlayer] = getValueOrIndirection();
}
#endif
/*	Calculates the true weight of all objects carried and worn by the player 
	(i.e. any containers will have the weight of their contents added up to a 
	maximum of 255), this value is then placed in Flag flagno.
	This would be useful to ensure the player was not carrying too much weight 
	to cross a bridge without it collapsing etc. */
#ifndef DISABLE_WEIGHT
void do_WEIGHT()	// flagno
{
	flags[getValueOrIndirection()] = getObjectWeight(NULLWORD, true);
}
#endif
/*	This sets Flag 37, the maximum number of objects conveyable, to value 1 and 
	Flag 52, the maximum weight of objects the player may carry and wear at any 
	one time (or their strength), to be value 2 .
	No checks are made to ensure that the player is not already carrying more 
	than the maximum. GET and so on, which check the values, will still work 
	correctly and prevent the player carrying any more objects, even if you set 
	the value lower than that which is already carried! */
#ifndef DISABLE_ABILITY
void do_ABILITY()	// value1 value2
{
	flags[fMaxCarr] = getValueOrIndirection();
	flags[fStrength] = *pPROC++;
}
#endif

// =============================================================================
// Actions for screen mode/format flags [3 condacts]

#ifndef DISABLE_MODE
void do_MODE() {printf("===== MODE not implemented\n"); pPROC++;}
#endif
#ifndef DISABLE_INPUT
void do_INPUT() {printf("===== INPUT not implemented\n"); pPROC+=2;}
#endif
/*	Allows input to be set to 'timeout' after a specific duration in 1 second 
	intervals, i.e. the Process 2 table will be called again if the player types 
	nothing for the specified period. This action alters flags 48 & 49. 'option' 
	allows this to also occur on ANYKEY and the "More..." prompt. In order to 
	calculate the number to use for the option just add the numbers shown next to
	each item to achieve the required combination;
	    1 - While waiting for first character of Input only.
	    2 - While waiting for the key on the "More..." prompt.
	    4 - While waiting for the key on the ANYKEY action.
	e.g. TIME 5 6 (option = 2+4) will allow 5 seconds of inactivity on behalf of 
	the player on input, ANYKEY or "More..." and between each key press. Whereas 
	TIME 5 3 (option = 1+2) allows it only on the first character of input and on 
	"More...".
	TIME 0 0 will stop timeouts (default). */
#ifndef DISABLE_TIME
void do_TIME()		// duration option
{
	flags[fTime] = getValueOrIndirection();		// Timeout duration required
	flags[fTIFlags] = *pPROC++;					// Timeout Control bitmask flags
}
#endif

// =============================================================================
// Actions for screen control & output [20 condacts]

/*	Selects window (0-7) as current print output stream. */
#ifndef DISABLE_WINDOW
void do_WINDOW()	// window
{
	flags[fCurWin] = getValueOrIndirection();
	cw = &windows[flags[fCurWin]];
}
#endif
/*	Sets current window to start at given line and column. Height and width to fit 
	available screen. */
#ifndef DISABLE_WINAT
void do_WINAT()		// line col
{
	cw->winY = getValueOrIndirection();
	cw->winX = *pPROC++;
}
#endif
/*	Sets current window size to given height and width. Clipping needed to fit 
	available screen. */
#ifndef DISABLE_WINSIZE
void do_WINSIZE()	// height width
{
	uint8_t aux = getValueOrIndirection();
	cw->winH = aux+cw->winY > MAX_LINES ? MAX_LINES-cw->winY : aux;
	aux = *pPROC++;
	cw->winW = aux+cw->winX > MAX_COLUMNS ? MAX_COLUMNS-cw->winX : aux;
}
#endif
/*	Will ensure the current window is centred for the current column width of the 
	screen. (Does not affect line position). */
#ifndef DISABLE_CENTRE
void do_CENTRE()
{
	cw->winX = (MAX_COLUMNS - cw->winW) >> 1;
}
#endif
/*	Clears the current window. */
#ifndef DISABLE_CLS
void do_CLS()
{
	gfxClearWindow();
	cw->cursorX = cw->cursorY = 0;
	lastPicLocation = 255;
	lastPicShow = false;
}
#endif
/*	Save and Restore print position for current window. This allows you to 
	maintain the print position for example while printing elsewhere in the 
	window. You should consider using a seperate window for most tasks. This 
	may find use in the creation of a new input line or in animation 
	sequences... */
#ifndef DISABLE_SAVEAT
void do_SAVEAT()
{
	savedPosX = cw->cursorX;
	savedPosY = cw->cursorY;
}
#endif
#ifndef DISABLE_BACKAT
void do_BACKAT()
{
	cw->cursorX = savedPosX;
	cw->cursorY = savedPosY;
}
#endif
/*	Set paper colour acording to the lookup table given in the graphics editors */
#ifndef DISABLE_PAPER
void do_PAPER()		// colour
{
	gfxSetPaperCol(getValueOrIndirection());
}
#endif
/*	Set text colour acording to the lookup table given in the graphics editors */
#ifndef DISABLE_INK
void do_INK()		// colour
{
	gfxSetInkCol(getValueOrIndirection());
}
#endif
/*	Set border colour acording to the lookup table given in the graphics editors */
#ifndef DISABLE_BORDER
void do_BORDER()	// colour
{
	do_PAPER();
}
#endif
/*	Sets current print position to given point if in current window. If not then 
	print position becomes top left of window. */
#ifndef DISABLE_PRINTAT
void do_PRINTAT()	// line col
{
	cw->cursorY = *pPROC++;
	cw->cursorX = *pPROC++;
}
#endif
/*	Sets current print position to given column on current line. */
#ifndef DISABLE_TAB
void do_TAB()		// col
{
	cw->cursorX = *pPROC++;
}
#endif
/*	Will simply print a space to the current output stream. Shorter than MES 
	Space! */
#ifndef DISABLE_SPACE
void do_SPACE()
{
	gfxPutCh(' ');
}
#endif
/*	Prints a carriage return/line feed. */
#ifndef DISABLE_NEWLINE
void do_NEWLINE()
{
	gfxPutCh('\r');
}
#endif
/*	Prints Message mesno. */
#ifndef DISABLE_MES
void do_MES()		// mesno
{
	printUserMsg(getValueOrIndirection());
}
#endif
/*	Prints Message mesno., then carries out a NEWLINE action. */
#ifndef DISABLE_MESSAGE
void do_MESSAGE()	// mesno
{
	do_MES();
	do_NEWLINE();
}
#endif
/*	Prints System Message sysno. */
#ifndef DISABLE_SYSMESS
void do_SYSMESS()	// sysno
{
	printSystemMsg(getValueOrIndirection());
}
#endif
/*	Prints the text for location locno. without a NEWLINE. */
#ifndef DISABLE_DESC
void do_DESC()		// locno
{
	printLocationMsg(getValueOrIndirection());
}
#endif
/*	The decimal contents of Flag flagno. are displayed without leading or 
	trailing spaces. */
#ifndef DISABLE_PRINT
void do_PRINT()		// flagno
{
	printBase10(flags[getValueOrIndirection()]);
}
#endif
/*	Will print the contents of flagno and flagno+1 as a two byte number. */
#ifndef DISABLE_DPRINT
void do_DPRINT()	// flagno
{
	uint8_t f = getValueOrIndirection();
	printBase10((((uint16_t)flags[f+1])<<8) | flags[f]);
}
#endif

// =============================================================================
// Actions for listing objects [2 condacts]

/*	If any objects are present then SM1 ("I can also see:") is printed, followed 
	by a list of all objects present at the current location.
	If there are no objects then nothing is printed. */
#ifndef DISABLE_LISTOBJ
void do_LISTOBJ() {
	flags[fOFlags] &= 0b01111111;
	for (int i=0; i<hdr->numObjDsc; i++) {
		if (objects[i].location == flags[fPlayer]) {
			if (!(flags[fOFlags] & 0b10000000)) {
				printSystemMsg(1);
				do_SPACE();
				flags[fOFlags] |= 0b10000000;
			} else {
				gfxPuts(", ");
			}
			printObjectMsg(i);
		}
	}
	if (flags[fOFlags] & 0b10000000) do_NEWLINE();
}
#endif
/*	If any objects are present then they are listed. Otherwise SM53 ("nothing.") 
	is printed - note that you will usually have to precede this action with a 
	message along the lines of "In the bag is:" etc. */
#ifndef DISABLE_LISTAT
void do_LISTAT()	// locno+
{
	uint8_t loc = *pPROC++;
	flags[fOFlags] &= 0b01111111;
	for (int i=0; i<hdr->numObjDsc; i++) {
		if (objects[i].location == loc) {
			if (flags[fOFlags] & 0b10000000) gfxPuts(", ");
			flags[fOFlags] |= 0b10000000;
			printObjectMsg(i);
		} 
	}
	if (flags[fOFlags] & 0b10000000)
		do_NEWLINE();
	else
		printSystemMsg(53);
}
#endif

// =============================================================================
// Actions for current game state save/load [4 condacts]

/*	This action saves the current game position on disc or tape. SM60 ("Type in 
	name of file.") is printed and the input routine is called to get the filename 
	from the player. If the supplied filename is not acceptable SM59 ("File name 
	error.") is printed - this is not checked on 8 bit machines, the file name 
	is MADE acceptable! */
#ifndef DISABLE_SAVE
void do_SAVE()		// opt
{
	//TODO
	pPROC++;
}
#endif
/*	This action loads a game position from disc or tape. A filename is obtained 
	in the same way as for SAVE. A variety of errors may appear on each machine 
	if the file is not found or suffers a load error. Usually 'I/O Error'. The 
	next action is carried out only if the load is successful. Otherwise a system 
	clear, GOTO 0, RESTART is carried out. */
#ifndef DISABLE_LOAD
void do_LOAD()		// opt
{
	//TODO
	pPROC++;
}
#endif
/*	In a similar way to SAVE this action saves all the information relevant to 
	the game in progress not onto disc but into a memory buffer. This buffer is 
	of course volatile and will be destroyed when the machine is turned off 
	which should be made clear to the player. The next action is always carried 
	out. */
#ifndef DISABLE_RAMSAVE
void do_RAMSAVE()
{
	memcpy(ramsave, flags, 256);
	memcpy(ramsave+256, objects, sizeof(Object)*hdr->numObjDsc);
}
#endif
/*	This action is the counterpart of RAMSAVE and allows the saved buffer to be 
	restored. The parameter specifies the last flag to be reloaded which can be 
	used to preserve values over a restore.
	Note 1: The RAM actions could be used to implement an OOPS command that is 
	common on other systems to take back the previous move; by creating an entry 
	in the main loop which does an automatic RAMSAVE every time the player enters 
	a move.
	Note 2: These four actions allow the next Condact to be carried out. They 
	should normally always be followed by a RESTART or describe in order that 
	the game state is restored to an identical position. */
#ifndef DISABLE_RAMLOAD
void do_RAMLOAD()	// flagno
{
	uint8_t flagno = getValueOrIndirection();

	memcpy(flags, ramsave, flagno);
	memcpy(objects, ramsave+256, sizeof(Object)*hdr->numObjDsc);
}
#endif

// =============================================================================
// Actions to pause game [2 condacts]

/*	SM16 ("Press any key to continue") is printed and the keyboard is scanned until 
	a key is pressed or until the timeout duration has elapsed if enabled. */
#ifndef DISABLE_ANYKEY
void do_ANYKEY()
{
	//TODO timeout
	printSystemMsg(16);
	waitForTimeout(TIME_ANYKEY);
	getchar();
}
#endif
/*	Pauses for value/50 secs. However, if value is zero then the pause is for 
	256/50 secs. */
#ifndef DISABLE_PAUSE
void do_PAUSE()		// value
{
	uint16_t value = getValueOrIndirection();
	if (!value) value = 256;

	setTime(0);
	while (getTime() < value);
}
#endif

// =============================================================================
// Actions to control the parse [3 condacts]

/*	The parameter 'n' controls which level of string indentation is to be 
	searched. At the moment only two are supported by the interpreters so only 
	the values 0 and 1 are valid.
		0 - Parse the main input line for the next LS.
		1 - Parse any string (phrase enclosed in quotes [""]) that was contained 
		    in the last LS extracted. */
#ifndef DISABLE_PARSE
void do_PARSE()
{
	if (getValueOrIndirection()==0) {
		checkEntry = !getLogicalSentence();
	} else {
		//TODO PARSE 1
		printf("===== 'PARSE 1' not implemented\n");		
	}
}
#endif
/*	Forces the loss of any remaining phrases on the current input line. You 
	would use this to prevent the player continuing without a fresh input 
	should something go badly for his situation. e.g. the GET action carries 
	out a NEWTEXT if it fails to get the required object for any reason, to 
	prevent disaster with a sentence such as:
		GET SWORD AND KILL ORC WITH IT
	as attacking the ORC without the sword may be dangerous! */
#ifndef DISABLE_NEWTEXT
void do_NEWTEXT()
{
	clearLogicalSentences();
}
#endif
/*	Substitutes the given verb and noun in the LS. Nullword (Usually '_') can be 
	used to suppress substitution for one or the other - or both I suppose! e.g.
	        MATCH    ON         SYNONYM LIGHT MATCH
	        STRIKE   MATCH      SYNONYM LIGHT _
	        LIGHT    MATCH      ....                 ; Actions...
	will switch the LS into a standard format for several different entries. 
	Allowing only one to deal with the actual actions. */
#ifndef DISABLE_SYNONYM
void do_SYNONYM()	// verb noun
{
	flags[fVerb] = getValueOrIndirection();
	flags[fNoun1] = *pPROC++;
}
#endif

// =============================================================================
// Actions for flow control [7 condacts]

/*	This powerful action transfers the attention of DAAD to the specified Process 
	table number. Note that it is a true subroutine call and any exit from the 
	new table (e.g. DONE, OK etc) will return control to the condact which follows 
	the calling PROCESS action. A sub-process can call (nest) further process' to 
	a depth of 10 at which point a run time error will be generated. */
#ifndef DISABLE_PROCESS
void do_PROCESS()	// procno
{
	pushPROC(getValueOrIndirection());
}
#endif
/*	Will restart the currently executing table, allowing */
#ifndef DISABLE_REDO
void do_REDO()
{
	currProc->entry = currProc->entryIni;
	checkEntry = false;
}
#endif
#ifndef DISABLE_DOALL
void do_DOALL() {printf("===== DOALL not implemented\n"); pPROC++;}
#endif
/*	Skip a distance of -128 to 128, or to the specified label. Will move the 
	current entry in a table back or fore. 0 means next entry (so is meaningless).
	-1 means restart current entry (Dangerous). */
#ifndef DISABLE_SKIP
void do_SKIP()		// distance
{
	stepPROCEntryCondacts(*pPROC++);
}
#endif
/*	Will cancel any DOALL loop, any sub-process calls and make a jump
	to execute process 0 again from the start.*/
#ifndef DISABLE_RESTART
void do_RESTART()
{
	//TODO cancel DOALL loop
	while (popPROC());
	pushPROC(0);
	checkEntry = false;
}
#endif
/*	SM13 ("Would you like to play again?") is printed and the input routine called.
	Any DOALL loop and sub-process calls are cancelled. If the reply does not start 
	with the first character of SM31 a jump is made to Initialise.
	Otherwise the player is returned to the operating system - by doing the command 
	EXIT 0.*/
#if !defined(DISABLE_EXIT) || !defined(DISABLE_END)
void _internal_exit()
{
	die("Thanks for playing!");
}
#endif
#ifndef DISABLE_END
void do_END()
{
	printSystemMsg(13);
	do_NEWLINE();
	clearLogicalSentences();
	prompt();
	char c = *tmpMsg;
	getSystemMsg(31);
	if (*tmpMsg==c) _internal_exit();
	//Initialize game
	initFlags();
	do_RESET();
	do_RESTART();
	checkEntry = false;
}
#endif
/*	If value is 0 then will return directly to the operating system. 
	Any value other than 0 will restart the whole game. Note that unlike RESTART 
	which only restarts processing, this will clear and reset windows etc. The 
	non zero numbers actually specify a part number to jump to on AUTOLOAD 
	versions. Only the PCW supports this feature at the moment. It will probably 
	be added to PC as part of the HYPERCARD work. So if you intend using it as a
	reset ensure you use your PART number as the non zero value! */
#ifndef DISABLE_EXIT
void do_EXIT()		// value
{
	if (!getValueOrIndirection()) {
		_internal_exit();
	}
	//Initialize game
	initFlags();
	do_RESET();
	do_RESTART();
	checkEntry = false;
}
#endif

// =============================================================================
// Actions for exit tables [3 condacts]

/*	This action jumps to the end of the process table and flags to DAAD that an 
	action has been carried out. i.e. no more condacts or entries are considered. 
	A return will thus be made to the previous calling process table, or to the 
	start point of any active DOALL loop. */
#ifndef DISABLE_DONE
void do_DONE()
{
	isDone = true;
	popPROC();
}
#endif
/*	This action jumps to the end of the process table and flags PAW that #no# 
	action has been carried out. i.e. no more condacts or entries are considered. 
	A return will thus be made to the previous calling process table or to the 
	start point of any active DOALL loop. This will cause PAW to print one of the
	"I can't" messages if needed. i.e. if no other action is carried out and no 
	entry is present in the connections section for the current Verb. */
#ifndef DISABLE_NOTDONE
void do_NOTDONE()
{
	isDone = false;
	popPROC();
}
#endif
/*	SM15 ("OK") is printed and action DONE is performed. */
#ifndef DISABLE_OK
void do_OK() {
	printSystemMsg(15);
	do_DONE();
}
#endif

// =============================================================================
// Actions to call external routines [4 condacts]

/*	Calls external routine with parameter value. The address is set by linking 
	the #extern pre-compiler command */
#ifndef DISABLE_EXTERN
void do_EXTERN()	// value routine
{
	//TODO
	#ifdef DEBUG
	printf("===== EXTERN not implemented\n");
	#endif
	pPROC+=2;
}
#endif
/*	Allows 'address' in memory (or in the database segment for 16bit) to be 
	executed. See the extern secion for more details. */
#ifndef DISABLE_CALL
void do_CALL()		// address
{
	//TODO
	#ifdef DEBUG
	printf("===== CALL not implemented\n");
	#endif
	pPROC+=2;
}
#endif
/*	This is a second EXTERN type action designed for Sound Effects extensions. 
	e.g. It has a 'default' function which allows value 'value1' to be written 
	to register 'value2' of the sound chip on 8 bit machines. This can be 
	changed with #sfx or through linking - see the machine details and extern 
	section for specifics. */
#ifndef DISABLE_SFX
void do_SFX()		// value1 value2
{
	//TODO
	#ifdef DEBUG
	printf("===== SFX not implemented\n");
	#endif
	pPROC+=2;
}
#endif
/*	An EXTERN which is meant to deal with any graphics extensions to DAAD. On 
	16bit it is used to implement the screen switching facilities. This can be 
	changed with #gfx or through linking. See the machine details and extern 
	section for specifics. */
#ifndef DISABLE_GFX
void do_GFX()		// value1 value2
{
	//TODO
	#ifdef DEBUG
	printf("===== GFX not implemented\n");
	#endif
	pPROC+=2;
}
#endif

// =============================================================================
// Actions for show pictures [2 condacts]

/*	Will load into the picture buffer the given picture. If there no corresponding
	picture the next entry will be carried out, if there is then the next CondAct 
	is executed. */
#ifndef DISABLE_PICTURE
void do_PICTURE()	// picno
{
	uint8_t newPic = getValueOrIndirection();
	lastPicShow = (newPic==lastPicLocation);
	lastPicLocation = newPic;
	checkEntry = gfxPicturePrepare(lastPicLocation);
}
#endif
/*	If value=0 then the last buffered picture is placed onscreen. 
	If value !=0 and the picture is not a subroutine then the given window area 
	is cleared. This is normally used with indirection and a flag to check and 
	display darkness. */
#ifndef DISABLE_DISPLAY
void do_DISPLAY()	// value
{
	if (getValueOrIndirection()) {
		do_CLS();
	} else {
		if (!lastPicShow) gfxPictureShow();
	}
}
#endif

// =============================================================================
// Actions miscellaneous [1 condacts]

#ifndef DISABLE_MOUSE
void do_MOUSE() { /* //TODO*/ }
#endif
/*	Length is the duration in 1/100 seconds. Tone is like BEEP in ZX Basic but
	adding 60 to it and divided by 2.
	http://www.worldofspectrum.org/ZXBasicManual/zxmanchap19.html
	
	Length:
		1:400 2:200 4:100 8:50 16:25 32:12 64:6
	
	Tone:
	      C  C# D  D# E  F  F# G  G# A  A# B
	ZX    0  1  2  3  4  5  6  7  8  9  10 11
	DAAD  30 
	
	i.e.  BEEP  1  144       ; O4 F# 64
	      BEEP  1  156       ; O5 C  64
	      BEEP  1  168       ; O5 F# 64
*/
#ifndef DISABLE_BEEP
void do_BEEP()		// length tone
{
	//TODO undocumented
	sfxSound(getValueOrIndirection(), *pPROC++);
}
#endif
void do_NOT_USED()
{
}



