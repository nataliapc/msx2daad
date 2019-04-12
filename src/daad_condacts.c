#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "daad_condacts.h"
#include "daad.h"


#define pPROC currProc->condact

PROCstack  procStack[10];
PROCstack *currProc;

uint8_t indirection;
uint8_t checkEntry;
uint8_t isDone, lastIsDone;
uint8_t doPopProc;
uint8_t lastPicLocation;
uint8_t lastPicShow;


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
	currProc->entry = currProc->entryIni;
	currProc->condactIni = (char*)0xffff;
	currProc->condact = 0;
}

//===============================================
bool popPROC()
{
#ifdef VERBOSE
printf("popPROC()\n");
#endif
	if (currProc > procStack) memset(currProc, 0, sizeof(PROCstack));
	currProc--;
	return (currProc < procStack);	//Yes/No more PROCs in the stack
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
	//Clear ISDONE flags
	isDone = false;

	do {
		while (currProc->entry->verb != 0) {
			lastIsDone = isDone;
			isDone = false;
#ifdef VERBOSE
printf("################ lastIsDone: %u\n", lastIsDone);
#endif
			if ((currProc->entry->verb==255 || currProc->entry->verb==flags[fVerb]) && 
				(currProc->entry->noun==255 || currProc->entry->noun==flags[fNoun1]))
			{
				currProc->condactIni = getPROCEntryCondacts();
				currProc->condact = currProc->condactIni;
				currProc->entry++;
				processPROCEntry();
			} else {
				currProc->entry++;
			}
		}
		popPROC();
	} while (currProc>=procStack);
}

//===============================================
void processPROCEntry()
{
#ifdef VERBOSE
printf("processPROCEntry()\n");
printf("    VERB:%u NOUN:%u [Pos:%p]\n",currProc->entry->verb, currProc->entry->noun, (char*)currProc->entry-ddb);
#endif
	checkEntry = true;
	doPopProc = false;
	while (checkEntry && *pPROC!=0xff) {
		processCondact();
		if (doPopProc) {
			doPopProc = false;
			popPROC();
			lastIsDone = isDone;
			isDone = false;
		}
	}
}

//===============================================
void processCondact()
{
	uint8_t condact = *pPROC++;
	uint8_t func = condact & 0x7f;
#ifdef VERBOSE
printf("processCondact(%u)\n",condact);
printf("    CONDACT: %s [Func:%p] [Pos:%p]\n",CONDACTS[func], &condactList[func].condact, (char*)(currProc->condact-1)-ddb);
#endif

	indirection = condact & 0x80;

	condactList[func].condact();
	if (!isDone) isDone |= condactList[func].flag;
//		case CDT_SAVE:			//25  //1
//		case CDT_LOAD:			//26  //1
//		case CDT_AUTOG:			//31  //0
//		case CDT_AUTOD:			//32  //0
//		case CDT_AUTOW:			//33  //0
//		case CDT_AUTOR:			//34  //0
//		case CDT_RAMSAVE:		//62  //0
//		case CDT_RAMLOAD:		//63  //1
//		case CDT_DOALL:			//85  //1
}

//===============================================
uint8_t getValueOrIndirection()
{
	uint8_t value = *pPROC++;
	return indirection ? flags[value] : value;
}

// =============================================================================
// Conditions of player locations [4 condacts]

/*	Succeeds if the current location is the same as locno. */
void do_AT()		// locno 
{
	checkEntry = (getValueOrIndirection() == flags[fPlayer]);
}
/*	Succeeds if the current location is different to locno. */
void do_NOTAT()		// locno
{
	checkEntry = (getValueOrIndirection() != flags[fPlayer]);
}
/*	Succeeds if the current location is greater than locno. */
void do_ATGT()		// locno
{
	checkEntry = (getValueOrIndirection() > flags[fPlayer]);
}
/*	Succeeds if the current location is less than locno. */
void do_ATLT()		// locno
{
	checkEntry = (getValueOrIndirection() < flags[fPlayer]);
}

// =============================================================================
// Conditions of object locations [8 condacts]

/*	Succeeds if Object objno. is carried (254), worn (253) or at the current 
	location [fPlayer]. */
void do_PRESENT()	// objno
{
	Object *obj = &objects[getValueOrIndirection()];
	if (obj->location<LOC_NOTCREATED && obj->location!=flags[fPlayer]) {
		checkEntry = false;
	}
}
/*	Succeeds if Object objno. is not carried (254), not worn (253) and not at 
	the current location [fPlayer]. */
void do_ABSENT()	// objno
{	
	Object *obj = &objects[getValueOrIndirection()];
	if (obj->location>=LOC_NOTCREATED || obj->location==flags[fPlayer]) {
		checkEntry = false;
	}
}
/*	Succeeds if object objno. is worn. */
void do_WORN()		// objno
{
	checkEntry = (objects[getValueOrIndirection()].location==LOC_WORN);
}
/*	Succeeds if Object objno. is not worn. */
void do_NOTWORN()	// objno
{
	checkEntry = (objects[getValueOrIndirection()].location!=LOC_WORN);
}
/*	Succeeds if Object objno. is carried. */
void do_CARRIED()	// objno
{
	checkEntry = (objects[getValueOrIndirection()].location==LOC_CARRIED);
}
/*	Succeeds if Object objno. is not carried. */
void do_NOTCARR()
{
	checkEntry = (objects[getValueOrIndirection()].location!=LOC_CARRIED);
}
/*	Succeeds if Object objno. is at Location locno. */
void do_ISAT()		// objno locno+
{
	checkEntry = (objects[getValueOrIndirection()].location==*pPROC++);
}
/*	Succeeds if Object objno. is not at Location locno. */
void do_ISNOTAT()	// objno locno+
{
	checkEntry = (objects[getValueOrIndirection()].location!=*pPROC++);
}

// =============================================================================
// Conditions for values/flags comparation [10 condacts]

/*	Succeeds if Flag flagno. is set to zero. */
void do_ZERO()		// flagno
{
	checkEntry = !flags[getValueOrIndirection()];
}
/*	Succeeds if Flag flagno. is not set to zero. */
void do_NOTZERO()	// flagno
{
	checkEntry = flags[getValueOrIndirection()];
}
/*	Succeeds if Flag flagno. is equal to value. */
void do_EQ()		// flagno value
{
	checkEntry = (flags[getValueOrIndirection()] == *pPROC++);
}
/*	Succeeds if Flag flagno. is not equal to value. */
void do_NOTEQ()		// flagno value
{
	checkEntry = (flags[getValueOrIndirection()] != *pPROC++);
}
/*	Succeeds if Flag flagno. is greater than value. */
void do_GT()		// flagno value
{
	checkEntry = (flags[getValueOrIndirection()] > *pPROC++);
}
/*	Succeeds if Flag flagno. is set to less than value. */
void do_LT()		// flagno value
{
	checkEntry = (flags[getValueOrIndirection()] < *pPROC++);
}
/*	Succeeds if Flag flagno 1 has the same value as Flag flagno 2. */
void do_SAME()		// flagno1 flagno2
{
	checkEntry = (flags[getValueOrIndirection()] == flags[*pPROC++]);
}
/*	Succeeds if Flag flagno 1 does not have the same value as Flag flagno 2 . */
void do_NOTSAME()	// flagno1 flagno2
{
	checkEntry = (flags[getValueOrIndirection()] != flags[*pPROC++]);
}
/*	Will be true if flagno 1 is larger than flagno 2 */
void do_BIGGER()	// flagno1 flagno2
{
	checkEntry = (flags[getValueOrIndirection()] > flags[*pPROC++]);
}
/*	Will be true if flagno 1 is smaller than flagno 2 */
void do_SMALLER()	// flagno1 flagno2
{
	checkEntry = (flags[getValueOrIndirection()] < flags[*pPROC++]);
}

// =============================================================================
// Conditions to check logical sentence [5 condacts]

/*	Succeeds if the first noun's adjective in the current LS is word. */
void do_ADJECT1()
{
	checkEntry = (flags[fAdject1] == *pPROC++);
}
/*	Succeeds if the adverb in the current LS is word. */
void do_ADVERB()
{
	checkEntry = (flags[fAdverb] == *pPROC++);
}
/*	Succeeds if the preposition in the current LS is word. */
void do_PREP()
{
	checkEntry = (flags[fPrep] == *pPROC++);
}
/*	Succeeds if the second noun in the current LS is word. */
void do_NOUN2()
{
	checkEntry = (flags[fNoun2] == *pPROC++);
}
/*	Succeeds if the second noun's adjective in the current LS is word. */
void do_ADJECT2()
{
	checkEntry = (flags[fAdject2] == *pPROC++);
}

// =============================================================================
// Conditions for random [1 condacts]

void do_CHANCE() {printf("===== CHANCE not implemented\n");}

// =============================================================================
// Conditions for sub-process success/fail [2 condacts]

/*	Succeeds if the last table ended by exiting after executing at least one 
	Action. This is useful to test for a single succeed/fail boolean value from 
	a Sub-Process. A DONE action will cause the 'done' condition, as will any 
	condact causing exit, or falling off the end of the table - assuming at 
	least one CondAct (other than NOTDONE) was done.
	See also ISNDONE and NOTDONE actions. */
void do_ISDONE()
{
	checkEntry = lastIsDone;
}
/*	Succeeds if the last table ended without doing anything or with a NOTDONE 
	action. */
void do_ISNDONE()
{
	checkEntry = !lastIsDone;
}

// =============================================================================
// Conditions for object attributes [2 condacts]

void _private_hasat(uint8_t value, bool negate)
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
		case HA_WAREABLE:  flag=fCOWR;    bit=0b10000000; break; // Flag 57 Bit#7
		case HA_CONTAINER: flag=fCOCon;   bit=0b10000000; break; // Flag 56 Bit#7
		case HA_LISTED:    flag=fOFlags;  bit=0b10000000; break; // Flag 53 Bit#7
		case HA_TIMEOUT:   flag=fTIFlags; bit=0b10000000; break; // Flag 49 Bit#7
		case HA_MOUSE:     flag=fTIFlags; bit=0b00000001; break; // Flag 29 Bit#0
		case HA_GMODE:     flag=fGFlags;  bit=0b10000000; break; // Flag 29 Bit#7
		default:
			#ifdef DEBUG
				die("===== HASAT/HASNAT value not implemented\n");
			#endif
	}
	flag = flags[flag] & bit;
	if (negate) flag = !flag;
	checkEntry = flag;
}
/*	Checks the attribute specified by value. 0-15 are the object attributes for 
	the current object. There are also several attribute numbers specified as 
	symbols in SYMBOLS.SCE which check certain parts of the DAAD system flags */
void do_HASAT()		// value
{
	_private_hasat(getValueOrIndirection(), false);
}
void do_HASNAT()	// value
{
	_private_hasat(getValueOrIndirection(), true);
}

// =============================================================================
// Conditions for player interaction [2 condacts]

/*	Is a condition which will be satisfied if the player is pressing a key. 
	In 16Bit machines Flags Key1 and Key2 (60 & 61) will be a standard IBM ASCII 
	code pair.
	On 8 bit only Key1 will be valid, and the code will be machine specific. */
void do_INKEY()
{
	while (kbhit()) getchar();
	while (!kbhit());
	flags[fKey1] = getchar();
}
/*	SM12 ("Are you sure?") is printed and called. Will succeed if the player replies
	starts with the first letter of SM30 ("Y") to then the remainder of the entry is 
	discarded is carried out. */
void do_QUIT()
{
	gfxPutsln(getSystemMsg(12));
	prompt();
	char c = *tmpMsg;
	getSystemMsg(30);
	if (*tmpMsg==c) do_END();
	checkEntry = false;
}

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
void _private_get(Object *obj)
{
printf("GET obj:%u loc:%u\n", flags[fNoun1], obj->location);
	if (obj->location==LOC_CARRIED || obj->location==LOC_WORN) {
		gfxPuts(getSystemMsg(25));
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location!=flags[fPlayer]) {
		gfxPuts(getSystemMsg(26));
		do_NEWTEXT();
		do_DONE();
	} else/* TODO
	if (weightCarriedAndWorn() + obj->attribs.mask.weight > flags[fStrength]) {
		gfxPuts(getSystemMsg(43));
		do_NEWTEXT();
		do_DONE();
	} else*/
	if (flags[fNOCarr] >= flags[fMaxCarr]) {
		gfxPuts(getSystemMsg(27));
		do_NEWTEXT();
		do_DONE();
		//TODO DOALL loop must be cancelled
	} else {
		obj->location = LOC_CARRIED;
		flags[fNOCarr]++;
	}
}
void do_GET()		// objno
{
	_private_get(&objects[getValueOrIndirection()]);
}
/*	If Object objno. is worn then SM24 ("I can't. I'm wearing the _.") is 
	printed and actions NEWTEXT & DONE are performed.

	If Object objno. is at the current location (but neither worn nor carried), 
	SM49 ("I don't have the _.") is printed and actions NEWTEXT & DONE are 
	performed.

	If Object objno. is not at the current location then SM28 ("I don't have one 
	of those.") is printed and actions NEWTEXT & DONE are performed.

	Otherwise the position of Object objno. is changed to the current location, 
	Flag 1 is decremented and SM39 ("I've dropped the _.") is printed. */
void _private_drop(Object *obj)
{
printf("DROP obj:%u loc:%u\n", flags[fNoun1], obj->location);
	if (obj->location==LOC_CARRIED) {
		obj->location = flags[fPlayer];
		gfxPuts(getSystemMsg(39));
		do_NEWTEXT();
		do_DONE();
		flags[fNOCarr]--;
	} else
	if (obj->location==LOC_WORN) {
		gfxPuts(getSystemMsg(24));
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location==flags[fPlayer]) {
		gfxPuts(getSystemMsg(49));
		do_NEWTEXT();
		do_DONE();
	} else {
		gfxPuts(getSystemMsg(28));
		do_NEWTEXT();
		do_DONE();
	}
}
void do_DROP()		// objno
{
	_private_drop(&objects[getValueOrIndirection()]);
}
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
void do_WEAR()		// objno
{
	Object *obj = &objects[getValueOrIndirection()];
	if (obj->location==flags[fPlayer]) {
		gfxPuts(getSystemMsg(49));
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location==LOC_WORN) {
		gfxPuts(getSystemMsg(29));
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location!=LOC_CARRIED) {
		gfxPuts(getSystemMsg(28));
		do_NEWTEXT();
		do_DONE();
	} else
	if (!obj->attribs.mask.isWareable) {
		gfxPuts(getSystemMsg(40));
		do_NEWTEXT();
		do_DONE();
	} else {
		gfxPuts(getSystemMsg(37));
		obj->location = LOC_WORN;
		flags[fNOCarr]--;
	}
}
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
void do_REMOVE()	// objno
{
	Object *obj = &objects[getValueOrIndirection()];
	if (obj->location==LOC_CARRIED || obj->location==flags[fPlayer]) {
		gfxPuts(getSystemMsg(50));
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location!=flags[fPlayer]) {
		gfxPuts(getSystemMsg(23));
		do_NEWTEXT();
		do_DONE();
	} else
	if (!obj->attribs.mask.isWareable) {
		gfxPuts(getSystemMsg(41));
		do_NEWTEXT();
		do_DONE();
	} else
	if (flags[fNOCarr] >= flags[fMaxCarr]) {
		gfxPuts(getSystemMsg(42));
		do_NEWTEXT();
		do_DONE();
	} else {
		gfxPuts(getSystemMsg(38));
		obj->location = LOC_CARRIED;
		flags[fNOCarr]++;
	}
}
/*	The position of Object objno. is changed to the current location and Flag 1
	is decremented if the object was carried. */
void do_CREATE()	// objno
{
	Object *obj = &objects[getValueOrIndirection()];
	if (obj->location==LOC_CARRIED) flags[fNOCarr]--;
	obj->location = flags[fPlayer];
}
/*	The position of Object objno. is changed to not-created and Flag 1 is 
	decremented if the object was carried. */
void do_DESTROY()	// objno
{
	Object *obj = &objects[getValueOrIndirection()];
	if (obj->location==LOC_CARRIED) flags[fNOCarr]--;
	obj->location = LOC_NOTCREATED;
}
/*	The positions of the two objects are exchanged. Flag 1 is not adjusted. The 
	currently referenced object is set to be Object objno 2. */
void do_SWAP()		// objno1 objno2
{
	Object *obj1 = &objects[getValueOrIndirection()];
	Object *obj2 = &objects[*pPROC++];
	uint8_t aux = obj1->location;
	obj1->location = obj2->location;
	obj2->location = aux;
}
/*	The position of Object objno. is changed to Location locno. Flag 1 is 
	decremented if the object was carried. It is incremented if the object is 
	placed at location 254 (carried). */
void do_PLACE()		// objno locno+
{
	Object *obj = &objects[getValueOrIndirection()];
	if (obj->location==LOC_CARRIED) flags[fNOCarr]--;
	obj->location = *pPROC++;
}
/*	The position of the currently referenced object (i.e. that object whose 
	number is given in flag 51), is changed to be Location locno. Flag 54 
	remains its old location. Flag 1 is decremented if the object was carried. 
	It is incremented if the object is placed at location 254 (carried). */
void do_PUTO()		// locno+
{
	Object *obj = &objects[flags[fCONum]];
	if (obj->location==LOC_CARRIED) flags[fNOCarr]--;
	else {
		obj->location = getValueOrIndirection();
		if (obj->location==LOC_CARRIED) flags[fNOCarr]++;
	}
}
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
void do_PUTIN()		// objno locno
{
	uint8_t objno = getValueOrIndirection();
	uint8_t locno = *pPROC++;
	Object *obj = &objects[objno];
	if (obj->location==LOC_WORN) {
		gfxPuts(getSystemMsg(24));
		do_NEWTEXT();
		do_DONE();		
	} else
	if (obj->location==flags[fPlayer]) {
		gfxPuts(getSystemMsg(49));
		do_NEWTEXT();
		do_DONE();		
	} else
	if (obj->location!=flags[fPlayer] && obj->location!=LOC_CARRIED) {
		gfxPuts(getSystemMsg(28));
		do_NEWTEXT();
		do_DONE();		
	} else {
		obj->location = locno;
		flags[fNOCarr]--;
		gfxPuts(getSystemMsg(44));
		do_SPACE();
		gfxPuts(getObjectMsg(objno));
		do_SPACE();
		gfxPuts(getSystemMsg(51));
	}
}
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
void do_TAKEOUT()	// objno locno
{
	uint8_t objno = getValueOrIndirection();
	uint8_t locno = *pPROC++;
	Object *obj = &objects[objno];
	if (obj->location==LOC_WORN || obj->location==LOC_CARRIED) {
		gfxPuts(getSystemMsg(25));
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location==flags[fPlayer]) {
		gfxPuts(getSystemMsg(45));
		do_SPACE();
		gfxPuts(getObjectMsg(locno));
		do_SPACE();
		gfxPuts(getSystemMsg(51));
		do_NEWTEXT();
		do_DONE();
	} else
	if (obj->location!=flags[fPlayer] || obj->location!=locno) {
		gfxPuts(getSystemMsg(52));
		do_SPACE();
		gfxPuts(getObjectMsg(locno));
		do_SPACE();
		gfxPuts(getSystemMsg(51));
		do_NEWTEXT();
		do_DONE();
	} else
	//TODO check weight of carried
	if (obj->location!=LOC_WORN && obj->location!=LOC_CARRIED /*&& total weight > flags[fStrength]*/) {
		gfxPuts(getSystemMsg(43));
		do_NEWTEXT();
		do_DONE();
	} else
	if (flags[fNOCarr] >= flags[fMaxCarr]) {
		gfxPuts(getSystemMsg(27));
		do_NEWTEXT();
		do_DONE();
		//TODO cancel DOALL loop
	} else {
		gfxPuts(getSystemMsg(36));
		obj->location = LOC_CARRIED;
		flags[fNOCarr]++;
	}
}
/*	All objects which are carried or worn are created at the current location (i.e. 
	all objects are dropped) and Flag 1 is set to 0. This is included for 
	compatibility with older writing systems.
	Note that a DOALL 254 will carry out a true DROP ALL, taking care of any special 
	actions included. */
void do_DROPALL()
{
	int i=0;
	do {
		objects[i].location = flags[fPlayer];
	} while(i++ < sizeof(objects));
	flags[fNOCarr] = 0;
}
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; here, carried, 
	worn. i.e. The player is more likely to be trying to GET an object that is 
	at the current location than one that is carried or worn. If an object is 
	found its number is passed to the GET action. Otherwise if there is an 
	object in existence anywhere in the game or if Noun1 was not in the 
	vocabulary then SM26 ("There isn't one of those here.") is printed. Else 
	SM8 ("I can't do that.") is printed (i.e. It is not a valid object but does 
	exist in the game). Either way actions NEWTEXT & DONE are performed */
void do_AUTOG()
{
	_private_get(&objects[flags[fNoun1]]);
}
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; carried, worn, 
	here. i.e. The player is more likely to be trying to DROP a carried object 
	than one that is worn or here. If an object is found its number is passed 
	to the DROP action. Otherwise if there is an object in existence anywhere 
	in the game or if Noun1 was not in the vocabulary then SM28 ("I don't have
	one of those.") is printed. Else SM8 ("I can't do that.") is printed (i.e. 
	It is not a valid object but does exist in the game). Either way actions 
	NEWTEXT & DONE are performed */
void do_AUTOD()
{
	_private_drop(&objects[flags[fNoun1]]);
}
void do_AUTOW() {printf("===== AUTOW not implemented\n");}
void do_AUTOR() {printf("===== AUTOR not implemented\n");}
void do_AUTOP() {printf("===== AUTOP not implemented\n");}
void do_AUTOT() {printf("===== AUTOT not implemented\n");}
void do_COPYOO() {printf("===== COPYOO not implemented\n");}
/*	This Action bears no resemblance to the one with the same name in PAW. It has 
	the pure function of placing all objects at the position given in the Object 
	start table. It also sets the relevant flags dealing with no of objects 
	carried etc. */
void do_RESET() {
	initObjects();
}

// =============================================================================
// Actions for object in flags manipulation [5 condacts]

void do_COPYOF() {printf("===== COPYOF not implemented\n");}
void do_COPYFO() {printf("===== COPYFO not implemented\n");}
void do_WHATO() {printf("===== WHATO not implemented\n");}
void do_SETCO() {printf("===== SETCO not implemented\n");}
void do_WEIGH() {printf("===== WEIGHT not implemented\n");}

// =============================================================================
// Actions to manipulate flags [11 condacts]

/*	Flag flagno. is set to 255. */
void do_SET()		// flagno
{
	flags[getValueOrIndirection()] = 255;
}
/*	Flag flagno. is cleared to 0. */
void do_CLEAR()		// flagno
{
	flags[getValueOrIndirection()] = 0;
}
/*	Flag flagno. is set to value. */
void do_LET()		// flagno value
{
	flags[getValueOrIndirection()] = *pPROC++;
}
/*	Flag flagno. is increased by value. If the result exceeds 255 the flag is
	set to 255. */
void do_PLUS()		// flagno value
{
	uint8_t flagno = getValueOrIndirection();
	uint16_t value = (uint16_t)flags[flagno] + *pPROC++;
	if (value>255) value = 255;
	flags[flagno] = (uint8_t)value;
}
/*	Flag flagno. is decreased by value. If the result is negative the flag is 
	set to 0. */
void do_MINUS()		// flagno value
{
	uint8_t flagno = getValueOrIndirection();
	uint8_t value = *pPROC++;
	flags[flagno] = value > flags[flagno] ? 0 : flags[flagno]-value;
}
/*	Flag flagno 2 has the contents of Flag flagno 1 added to it. If the result 
	exceeds 255 the flag is set to 255. */
void do_ADD()		// flagno1 flagno2
{
	uint16_t sum = flags[getValueOrIndirection()];
	uint8_t flagno2 = *pPROC++;
	sum += flags[flagno2];
	flags[flagno2] = sum>255 ? 255 : (uint8_t)sum;
}
/*	Flag flagno 2 has the contents of Flag flagno 1 subtracted from it. If the
	result is negative the flag is set to 0. */
void do_SUB()		// flagno1 flagno2
{
	uint8_t subs = flags[getValueOrIndirection()];
	uint8_t flagno2 = *pPROC++;
	flags[flagno2] = subs > flags[flagno2] ? 0 : flags[flagno2]-subs;
}
/*	The contents of Flag flagno 1 is copied to Flag flagno 2. */
void do_COPYFF()	// flagno1 flagno2
{
	uint8_t flagno1 = getValueOrIndirection();
	flags[*pPROC++] = flags[flagno1];
}
/*	Same as COPYFF but the source and destination are reversed, so that 
	indirection can be used. */
void do_COPYBF()	// flagno1 flagno2
{
	flags[getValueOrIndirection()] = flags[*pPROC++];
}
/*	Flag flagno. is set to a number from the Pseudo-random sequence from 1 
	to 100. */
void do_RANDOM()	// flagno
{
	flags[*pPROC++] = (rand()%100)+1;
}
/*	This is a very powerful action designed to manipulate PSI's. It allows the
	current LS Verb to be used to scan the connections section for the location 
	given in Flag flagno. 
	If the Verb is found then Flag flagno. is changed to be the location number 
	associated with it, and the next condact is considered.
	If the verb is not found, or the original location number was invalid, then 
	PAW considers the next entry in the table - if present. */
void do_MOVE()		// flagno
{
	if (flags[fVerb]<14) {
		uint8_t *cnx = &ddb[*(((uint16_t*)hdr->conLstPos) + flags[fPlayer])];
		uint8_t flagno = *pPROC++;

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

// =============================================================================
// Actions to manipulate player flags [3 condacts]

/*	Changes the current location to locno. This effectively sets flag 38 to the value
	locno. */
void do_GOTO()		// locno
{
	flags[fPlayer] = getValueOrIndirection();
}
void do_WEIGHT() {}
/*	This sets Flag 37, the maximum number of objects conveyable, to value 1 and 
	Flag 52, the maximum weight of objects the player may carry and wear at any 
	one time (or their strength), to be value 2 .
	No checks are made to ensure that the player is not already carrying more 
	than the maximum. GET and so on, which check the values, will still work 
	correctly and prevent the player carrying any more objects, even if you set 
	the value lower than that which is already carried! */
void do_ABILITY()	// value1 value2
{
	flags[fMaxCarr] = getValueOrIndirection();
	flags[fStrength] = *pPROC++;
}

// =============================================================================
// Actions for screen mode/format flags [3 condacts]

void do_MODE() {printf("===== MODE not implemented\n");}
void do_INPUT() {printf("===== INPUT not implemented\n");}
void do_TIME() {printf("===== TIME not implemented\n");}

// =============================================================================
// Actions for screen control & output [20 condacts]

/*	Selects window (0-7) as current print output stream. */
void do_WINDOW()	// window
{
	flags[fCurWin] = getValueOrIndirection();
	cw = &windows[flags[fCurWin]];
}
/*	Sets current window to start at given line and column. Height and width to fit 
	available screen. */
void do_WINAT()		// line col
{
	cw->winY = getValueOrIndirection();
	cw->winX = *pPROC++;
}
/*	Sets current window size to given height and width. Clipping needed to fit 
	available screen. */
void do_WINSIZE()	// height width
{
	uint8_t aux = getValueOrIndirection();
	cw->winH = aux+cw->winY > MAX_LINES ? MAX_LINES-cw->winY : aux;
	aux = *pPROC++;
	cw->winW = aux+cw->winX > MAX_COLUMNS ? MAX_COLUMNS-cw->winX : aux;
}
/*	Will ensure the current window is centred for the current column width of the 
	screen. (Does not affect line position). */
void do_CENTRE()
{
	cw->winX = (MAX_COLUMNS - cw->winW) >> 1;
}
/*	Clears the current window. */
void do_CLS()
{
	gfxClearWindow();
	cw->cursorX = cw->cursorY = 0;
	lastPicLocation = 255;
	lastPicShow = false;
}
/*	Save and Restore print position for current window. This allows you to 
	maintain the print position for example while printing elsewhere in the 
	window. You should consider using a seperate window for most tasks. This 
	may find use in the creation of a new input line or in animation 
	sequences... */
void do_SAVEAT()
{
	savedPosX = cw->cursorX;
	savedPosY = cw->cursorY;
}
void do_BACKAT()
{
	cw->cursorX = savedPosX;
	cw->cursorY = savedPosY;
}
/*	Set paper colour acording to the lookup table given in the graphics editors */
void do_PAPER()		// colour
{
	gfxSetPaperCol(getValueOrIndirection());
}
/*	Set text colour acording to the lookup table given in the graphics editors */
void do_INK()		// colour
{
	gfxSetInkCol(getValueOrIndirection());
}
/*	Set border colour acording to the lookup table given in the graphics editors */
void do_BORDER()	// colour
{
	do_PAPER();
}
/*	Sets current print position to given point if in current window. If not then 
	print position becomes top left of window. */
void do_PRINTAT()	// line col
{
	cw->cursorY = *pPROC++;
	cw->cursorX = *pPROC++;
}
/*	Sets current print position to given column on current line. */
void do_TAB()		// col
{
	cw->cursorX = *pPROC++;
}
/*	Will simply print a space to the current output stream. Shorter than MES 
	Space! */
void do_SPACE()
{
	gfxPutCh(' ');
}
/*	Prints a carriage return/line feed. */
void do_NEWLINE()
{
	gfxPutCh('\n');
}
/*	Prints Message mesno. */
void do_MES()		// mesno
{
	gfxPuts(getUserMsg(getValueOrIndirection()));
}
/*	Prints Message mesno., then carries out a NEWLINE action. */
void do_MESSAGE()	// mesno
{
	do_MES();
	do_NEWLINE();
}
/*	Prints System Message sysno. */
void do_SYSMESS()	// sysno
{
	gfxPuts(getSystemMsg(getValueOrIndirection()));
}
/*	Prints the text for location locno. without a NEWLINE. */
void do_DESC()		// locno
{
	gfxPuts(getLocationMsg(getValueOrIndirection()));
}
/*	The decimal contents of Flag flagno. are displayed without leading or 
	trailing spaces. */
void do_PRINT()		// flagno
{
	printBase10(flags[getValueOrIndirection()]);
}
/*	Will print the contents of flagno and flagno+1 as a two byte number. */
void do_DPRINT()	// flagno
{
	uint8_t f = getValueOrIndirection();
	printBase10((((uint16_t)flags[f+1])<<8) | flags[f]);
}

// =============================================================================
// Actions for listing objects [2 condacts]

/*	If any objects are present then SM1 ("I can also see:") is printed, followed 
	by a list of all objects present at the current location.
	If there are no objects then nothing is printed. */
void do_LISTOBJ() {
	bool first = true;
	for (int i=0; i<hdr->numObjDsc; i++) {
		if (objects[i].location == flags[fPlayer]) {
			if (first) {
				gfxPuts(getSystemMsg(1));
				first = false;
			}
			gfxPuts(getObjectMsg(i));
		}
	}
	if (!first) do_NEWLINE();
}
/*	If any objects are present then they are listed. Otherwise SM53 ("nothing.") 
	is printed - note that you will usually have to precede this action with a 
	message along the lines of "In the bag is:" etc. */
void do_LISTAT()	// locno+
{
	uint8_t loc = *pPROC++;
	bool found = false;
	for (int i=0; i<hdr->numObjDsc; i++) {
		if (objects[i].location == loc) {
			gfxPuts(getObjectMsg(i));
			found = true;
		}
	}
	if (!found) gfxPuts(getSystemMsg(53));
	else do_NEWLINE();
}

// =============================================================================
// Actions for current game state save/load [4 condacts]

void do_SAVE() {printf("===== SAVE not implemented\n"); pPROC++;}
void do_LOAD() {printf("===== LOAD not implemented\n"); pPROC++;}
void do_RAMSAVE() {printf("===== RAMSAVE not implemented\n");}
void do_RAMLOAD() {printf("===== RAMLOAD not implemented\n"); pPROC++;}

// =============================================================================
// Actions to pause game [2 condacts]

/*	SM16 ("Press any key to continue") is printed and the keyboard is scanned until 
	a key is pressed or until the timeout duration has elapsed if enabled. */
void do_ANYKEY()
{
	//TODO timeout
	gfxPuts(getSystemMsg(16));
	do_INKEY();
}
void do_PAUSE() {printf("===== PAUSE not implemented\n");}

// =============================================================================
// Actions to control the parse [3 condacts]

/*	The parameter 'n' controls which level of string indentation is to be 
	searched. At the moment only two are supported by the interpreters so only 
	the values 0 and 1 are valid.
		0 - Parse the main input line for the next LS.
		1 - Parse any string (phrase enclosed in quotes [""]) that was contained 
		    in the last LS extracted. */
void do_PARSE()
{
	if (getValueOrIndirection()==0) {
		checkEntry = !getLogicalSentence();
	} else {
		printf("===== 'PARSE 1' not implemented\n");		
	}
}
/*	Forces the loss of any remaining phrases on the current input line. You 
	would use this to prevent the player continuing without a fresh input 
	should something go badly for his situation. e.g. the GET action carries 
	out a NEWTEXT if it fails to get the required object for any reason, to 
	prevent disaster with a sentence such as:
		GET SWORD AND KILL ORC WITH IT
	as attacking the ORC without the sword may be dangerous! */
void do_NEWTEXT()
{
	clearLogicalSentences();
}
void do_SYNONYM() {printf("===== SYNONYM not implemented\n");}

// =============================================================================
// Actions for flow control [7 condacts]

/*	This powerful action transfers the attention of DAAD to the specified Process 
	table number. Note that it is a true subroutine call and any exit from the 
	new table (e.g. DONE, OK etc) will return control to the condact which follows 
	the calling PROCESS action. A sub-process can call (nest) further process' to 
	a depth of 10 at which point a run time error will be generated. */
void do_PROCESS()	// procno
{
	pushPROC(getValueOrIndirection());
	processPROC();
}
/*	Will restart the currently executing table, allowing */
void do_REDO()
{
	currProc->entry = currProc->entryIni;
	checkEntry = false;
}
void do_DOALL() {printf("===== DOALL not implemented\n");}
/*	Skip a distance of -128 to 128, or to the specified label. Will move the 
	current entry in a table back or fore. 0 means next entry (so is meaningless).
	-1 means restart current entry (Dangerous). */
void do_SKIP()		// distance
{
	stepPROCEntryCondacts(*pPROC++);
}
/*	Will cancel any DOALL loop, any sub-process calls and make a jump
	to execute process 0 again from the start.*/
void do_RESTART()
{
	//TODO cancel DOALL loop
	while (popPROC());
	pushPROC(0);
	checkEntry = false;
}
/*	SM13 ("Would you like to play again?") is printed and the input routine called.
	Any DOALL loop and sub-process calls are cancelled. If the reply does not start 
	with the first character of SM31 a jump is made to Initialise.
	Otherwise the player is returned to the operating system - by doing the command 
	EXIT 0.*/
void do_END()
{
	gfxPutsln(getSystemMsg(13));
	prompt();
	char c = *tmpMsg;
	getSystemMsg(31);
	if (*tmpMsg==c) do_EXIT();
	//Initialize game
	initFlags();
	do_RESET();
	do_RESTART();
	checkEntry = false;
}
/*	If value is 0 then will return directly to the operating system. 
	Any value other than 0 will restart the whole game. Note that unlike RESTART 
	which only restarts processing, this will clear and reset windows etc. The 
	non zero numbers actually specify a part number to jump to on AUTOLOAD 
	versions. Only the PCW supports this feature at the moment. It will probably 
	be added to PC as part of the HYPERCARD work. So if you intend using it as a
	reset ensure you use your PART number as the non zero value! */
void do_EXIT()		// value
{
	if (!getValueOrIndirection()) {
		die("Thanks for playing!");
	}
	//Initialize game
	initFlags();
	do_RESET();
	do_RESTART();
	checkEntry = false;
}

// =============================================================================
// Actions for exit tables [3 condacts]

/*	This action jumps to the end of the process table and flags to DAAD that an 
	action has been carried out. i.e. no more condacts or entries are considered. 
	A return will thus be made to the previous calling process table, or to the 
	start point of any active DOALL loop. */
void do_DONE()
{
	doPopProc = true;
	checkEntry = true;
	isDone = true;
}
/*	This action jumps to the end of the process table and flags PAW that #no# 
	action has been carried out. i.e. no more condacts or entries are considered. 
	A return will thus be made to the previous calling process table or to the 
	start point of any active DOALL loop. This will cause PAW to print one of the
	"I can't" messages if needed. i.e. if no other action is carried out and no 
	entry is present in the connections section for the current Verb. */
void do_NOTDONE()
{
	doPopProc = true;
	checkEntry = false;
	isDone = false;
}
/*	SM15 ("OK") is printed and action DONE is performed. */
void do_OK() {
	gfxPuts(getSystemMsg(15));
	do_DONE();
}

// =============================================================================
// Actions to call external routines [4 condacts]

/*	Calls external routine with parameter value. The address is set by linking 
	the #extern pre-compiler command */
void do_EXTERN()	// value routine
{
	//TODO
	#ifdef DEBUG
	printf("===== EXTERN not implemented\n");
	#endif
	pPROC+=2;
}
/*	Allows 'address' in memory (or in the database segment for 16bit) to be 
	executed. See the extern secion for more details. */
void do_CALL()		// address
{
	//TODO
	#ifdef DEBUG
	printf("===== CALL not implemented\n");
	#endif
	pPROC+=2;
}
/*	This is a second EXTERN type action designed for Sound Effects extensions. 
	e.g. It has a 'default' function which allows value 'value1' to be written 
	to register 'value2' of the sound chip on 8 bit machines. This can be 
	changed with #sfx or through linking - see the machine details and extern 
	section for specifics. */
void do_SFX()		// value1 value2
{
	//TODO
	#ifdef DEBUG
	printf("===== SFX not implemented\n");
	#endif
	pPROC+=2;
}
/*	An EXTERN which is meant to deal with any graphics extensions to DAAD. On 
	16bit it is used to implement the screen switching facilities. This can be 
	changed with #gfx or through linking. See the machine details and extern 
	section for specifics. */
void do_GFX()		// value1 value2
{
	//TODO
	#ifdef DEBUG
	printf("===== GFX not implemented\n");
	#endif
	pPROC+=2;
}

// =============================================================================
// Actions for show pictures [2 condacts]

/*	Will load into the picture buffer the given picture. If there no corresponding
	picture the next entry will be carried out, if there is then the next CondAct 
	is executed. */
void do_PICTURE()	// picno
{
	uint8_t newPic = getValueOrIndirection();
	lastPicShow = (newPic==lastPicLocation);
	lastPicLocation = newPic;
	checkEntry = gfxPicturePrepare(lastPicLocation);
}
/*	If value=0 then the last buffered picture is placed onscreen. 
	If value !=0 and the picture is not a subroutine then the given window area 
	is cleared. This is normally used with indirection and a flag to check and 
	display darkness. */
void do_DISPLAY()	// value
{
	if (getValueOrIndirection()) {
		do_CLS();
	} else {
		if (!lastPicShow) gfxPictureShow();
	}
}

// =============================================================================
// Actions miscellaneous [1 condacts]

void do_MOUSE() {}
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
void do_BEEP()		// length tone
{
	//TODO undocumented
	BiosCall(0xc0, &regs, REGS_NONE);
	pPROC+=2;
}
void do_NOT_USED()
{
	pPROC++;
}

