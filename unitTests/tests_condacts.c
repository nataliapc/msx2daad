#define WINDOWS_NUM		8

#include <string.h>
#include "assert.h"
#include "daad.h"
#include "daad_platform_api.h"
#include "daad_condacts.h"


#define IND		128

uint8_t fake_keyPressed;
int16_t fake_lastSysMesPrinted;

// =============================================================================
// Global variables

PROCstack procStack[NUM_PROCS];		// Stack of calls using PROCESS condact.
PROCstack *currProc;				// Pointer to current active condact.

bool indirection;					// True if the current condact use indirection for the first argument.
bool checkEntry;					// Boolean to check if a Process entry must continue or a condition fails.
bool isDone, lastIsDone;			// Variables for ISDONE/ISNDONE condacts.
bool lastPicShow;					// True if last location picture was drawed.


uint8_t    *ddb;						// Where the DDB is allocated
DDB_Header *hdr;						// Struct pointer to DDB Header
Object     *objects;					// Memory allocation for objects data
uint8_t     flags[256];					// DAAD flags (256 bytes)
char       *ramsave;					// Memory to store ram save (RAMSAVE)

char *pPROC;

//static const uint8_t nullObjFake[] = { 0, 0, 0, 0, 0, 0 };
//static const Object  *nullObject;

Window	*windows;						// 0-7 windows definitions
Window	*cw;							// Pointer to current active window
uint8_t printedLines;					// For "More..." feature

uint8_t savedPosX;						// For SAVEAT/BACKAT
uint8_t savedPosY;						//  "    "      "

char *tmpMsg;							// TEXT_BUFFER_LEN

typedef enum CONDACT {
	_AT,        _NOTAT,     _ATGT,     _ATLT,     _PRESENT,  _ABSENT,    _WORN,      _NOTWORN,  _CARRIED,  _NOTCARR,	// 0-9
	_CHANCE,    _ZERO,      _NOTZERO,  _EQ,       _GT,       _LT,        _ADJECT1,   _ADVERB,   _SFX,      _DESC,		// 10-19
	_QUIT,      _END,       _DONE,     _OK,       _ANYKEY,   _SAVE,      _LOAD,      _DPRINT,   _DISPLAY,  _CLS,     	// 20-29
	_DROPALL,   _AUTOG,     _AUTOD,    _AUTOW,    _AUTOR,    _PAUSE,     _SYNONYM,   _GOTO,     _MESSAGE,  _REMOVE,  	// 30-39
	_GET,       _DROP,      _WEAR,     _DESTROY,  _CREATE,   _SWAP,      _PLACE,     _SET,      _CLEAR,    _PLUS,    	// 40-49
	_MINUS,     _LET,       _NEWLINE,  _PRINT,    _SYSMESS,  _ISAT,      _SETCO,     _SPACE,    _HASAT,    _HASNAT,  	// 50-59
	_LISTOBJ,   _EXTERN,    _RAMSAVE,  _RAMLOAD,  _BEEP,     _PAPER,     _INK,       _BORDER,   _PREP,     _NOUN2,   	// 60-69
	_ADJECT2,   _ADD,       _SUB,      _PARSE,    _LISTAT,   _PROCESS,   _SAME,      _MES,      _WINDOW,   _NOTEQ,   	// 70-79
	_NOTSAME,   _MODE,      _WINAT,    _TIME,     _PICTURE,  _DOALL,     _MOUSE,     _GFX,      _ISNOTAT,  _WEIGH,   	// 80-89
	_PUTIN,     _TAKEOUT,   _NEWTEXT,  _ABILITY,  _WEIGHT,   _RANDOM,    _INPUT,     _SAVEAT,   _BACKAT,   _PRINTAT, 	// 90-99
	_WHATO,     _CALL,      _PUTO,     _NOTDONE,  _AUTOP,    _AUTOT,     _MOVE,      _WINSIZE,  _REDO,     _CENTRE,  	// 100-109
	_EXIT,      _INKEY,     _BIGGER,   _SMALLER,  _ISDONE,   _ISNDONE,   _SKIP,      _RESTART,  _TAB,      _COPYOF,  	// 110-119
	_NOTUSED1,  _COPYOO,    _NOTUSED2, _COPYFO,   _NOTUSED3, _COPYFF,    _COPYBF,    _RESET								// 120-127
};
	
// =============================================================================
// daad.c

bool initDAAD(int argc, char **argv) {}
void initFlags() {}
void initObjects() {}
//void mainLoop() {}

void prompt() {}
//void parser() {}

void clearLogicalSentences() {}
bool getLogicalSentence() {}
//void nextLogicalSentence();
bool useLiteralSentence() {}
//bool populateLogicalSentence();

void printBase10(uint16_t value) __z88dk_fastcall {}
bool waitForTimeout(uint16_t timerFlag) __z88dk_fastcall {}
void errorCode(uint8_t code) {}

//char* getToken(uint8_t num) __z88dk_fastcall;
//void printMsg(char *p, bool print);
//void printOutMsg(char *str) __z88dk_fastcall;
void printChar(int c) __z88dk_fastcall {}
//void checkPrintedLines(); {}
void getSystemMsg(uint8_t num) __z88dk_fastcall {}
void printSystemMsg(uint8_t num) __z88dk_fastcall { fake_lastSysMesPrinted = num; }
void printUserMsg(uint8_t num) __z88dk_fastcall {}
void printLocationMsg(uint8_t num) __z88dk_fastcall {}
void printObjectMsg(uint8_t num) __z88dk_fastcall {}
void printObjectMsgModif(uint8_t num, char modif) {}
uint8_t getObjectId(uint8_t noun, uint8_t adjc, uint16_t location) {}
uint8_t getObjectWeight(uint8_t objno, bool isCarriedWorn) { if (objno==NULLWORD) return 0; else return objects[objno].attribs.mask.weight; }
void referencedObject(uint8_t objno) __z88dk_fastcall {}

//void transcript_flush();
//void transcript_char(char c) __z88dk_fastcall;

// =============================================================================
// daad_condacts.h

//void initializePROC();
//void pushPROC(uint8_t proc) __z88dk_fastcall;
//bool popPROC();
//PROCentry* getPROCess(uint8_t proc) __z88dk_fastcall;
//char* getPROCEntryCondacts();
//char* stepPROCEntryCondacts(int8_t step) __z88dk_fastcall;
//void processPROC();


// =============================================================================
// daad_platform_api.c

// System functions
//bool     checkPlatformSystem();
//uint16_t getFreeMemory();
//char*    getCharsTranslation();
void     setTime(uint16_t time) __z88dk_fastcall {}
uint16_t getTime() {}
uint16_t checkKeyboardBuffer() { return fake_keyPressed; }
void     clearKeyboardBuffer() { fake_keyPressed = 0; }
uint8_t  getKeyInBuffer() { uint8_t ret = fake_keyPressed; fake_keyPressed = 13; return ret; }
//void     waitingForInput() {}
//inline void *safeMemoryAllocate();
//inline void safeMemoryDeallocate(void*);

// Filesystem
//void     loadFilesBin(int argc, char **argv);
//uint16_t loadFile(char *filename, uint8_t *destaddress, uint16_t size);

// External texts
void printXMES(uint16_t address) __z88dk_fastcall {}

// GFX functions
//void gfxSetScreen();
//void gfxSetScreenModeFlags();
//void gfxClearScreenBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
inline void gfxClearWindow() {}
//inline void gfxClearCurrentLine();
//inline void gfxScrollUp();
inline void gfxSetPaperCol(uint8_t col) {}
inline void gfxSetInkCol(uint8_t col) {}
inline void gfxSetBorderCol(uint8_t col) {}
//inline void gfxSetGraphCharset(bool value);
//inline void gfxPutChWindow(uint8_t c);
//static void gfxPutChPixels(uint8_t c, uint16_t dx, uint16_t dy);
//inline void gfxPutInputEcho(char c, bool keepPos);
//inline void gfxSetPalette(uint8_t index, uint8_t red, uint8_t green, uint8_t blue);
bool gfxPicturePrepare(uint8_t location) __z88dk_fastcall {}
inline bool gfxPictureShow() {}
inline void gfxRoutines(uint8_t routine, uint8_t value) {}

// SFX functions
//void sfxInit();
void sfxWriteRegister(uint8_t reg, uint8_t value) {}
void sfxTone(uint8_t value1, uint8_t value2) {}




#define ERROR 			"ERROR"

void beforeAll()
{
	objects = NULL;
}

void beforeEach()
{
	initFlags();
	initializePROC();
	pushPROC(0);
	
	checkEntry = isDone = lastIsDone = false;

	if (objects == NULL) objects = malloc(sizeof(Object) * 10);
	memset(objects, 0, sizeof(Object) * 10);
	fake_keyPressed = 0;
	fake_lastSysMesPrinted = -1;
}

void do_action(char *pProc, void (* const function)())
{
	pPROC = pProc + 1;
	indirection = *pProc & IND;
	function();
}


// =============================================================================
// Conditions of player locations [4 condacts]
// =============================================================================

// =============================================================================
// Tests AT

void test_AT_success()
{
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with AT 5
	const char proc[] = { _AT, 5, 255 };
	do_action(proc, do_AT);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_AT_fails()
{
	beforeEach();
	
	//BDD given player at loc 21
	flags[fPlayer] = 21;

	//BDD when check it with AT 5
	const char proc[] = { _AT, 5, 255 };
	do_action(proc, do_AT);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AT_indirection()
{
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 5;

	//BDD when check it with AT [150]
	const char proc[] = { _AT|IND, 150, 255 };
	do_action(proc, do_AT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTAT

void test_NOTAT_success()
{
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with NOTAT 2
	char proc[] = { _NOTAT, 2, 255 };
	do_action(proc, do_NOTAT);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTAT_fails()
{
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with NOTAT 5
	char proc[] = { _NOTAT, 5, 255 };
	do_action(proc, do_NOTAT);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTAT_indirection()
{
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 11;

	//BDD when check it with NOTAT [150]
	const char proc[] = { _NOTAT|IND, 150, 255 };
	do_action(proc, do_NOTAT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ATGT

void test_ATGT_success()
{
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATGT 2
	const char proc[] = { _ATGT, 2, 255 };
	do_action(proc, do_ATGT);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ATGT_fails()
{
	beforeEach();
	
	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATGT 5
	const char proc[] = { _ATGT, 5, 255 };
	do_action(proc, do_ATGT);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ATGT_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 2;

	//BDD when check it with ATGT [150]
	const char proc[] = { _ATGT|IND, 150, 255 };
	do_action(proc, do_ATGT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ATLT

void test_ATLT_success()
{
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATLT 12
	const char proc[] = { _ATLT, 12, 255 };
	do_action(proc, do_ATLT);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ATLT_fails()
{
	beforeEach();
	
	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATLT 2
	const char proc[] = { _ATLT, 2, 255 };
	do_action(proc, do_ATLT);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ATLT_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 12;

	//BDD when check it with ATLT [150]
	const char proc[] = { _ATLT|IND, 150, 255 };
	do_action(proc, do_ATLT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions of object locations [8 condacts]
// =============================================================================

// =============================================================================
// Tests PRESENT

void test_PRESENT_success()
{
	beforeEach();

	//BDD given player at loc 5 and object 1 at same place
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking PRESENT 1
	const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc, do_PRESENT);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_fails()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking PRESENT 1
	const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc, do_PRESENT);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 5
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = LOC_HERE;

	//BDD when checking PRESENT [150]
	const char proc[] = { _PRESENT|IND, 150, 255 };
	do_action(proc, do_PRESENT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ABSENT

void test_ABSENT_success()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking ABSENT 1
	const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc, do_ABSENT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ABSENT_fails()
{
	beforeEach();

	//BDD given player at loc 5 and object 1 at same place
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking ABSENT 1
	const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc, do_ABSENT);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ABSENT_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking ABSENT [150]
	const char proc[] = { _ABSENT|IND, 150, 255 };
	do_action(proc, do_ABSENT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WORN

void test_WORN_success()
{
	beforeEach();

	//BDD given player at loc 5 and object 1 is worn
	flags[fPlayer] = 5;
	objects[1].location = LOC_WORN;

	//BDD when checking WORN 1
	const char proc[] = { _WORN, 1, 255 };
	do_action(proc, do_WORN);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WORN_fails()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 5
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking WORN 1
	const char proc[] = { _WORN, 1, 255 };
	do_action(proc, do_WORN);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WORN_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 is worn
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = LOC_WORN;

	//BDD when checking WORN [150]
	const char proc[] = { _WORN|IND, 150, 255 };
	do_action(proc, do_WORN);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTWORN

void test_NOTWORN_success()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking NOTWORN 1
	const char proc[] = { _NOTWORN, 1, 255 };
	do_action(proc, do_NOTWORN);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTWORN_fails()
{
	beforeEach();

	//BDD given player at loc 5 and object 1 is worn
	flags[fPlayer] = 5;
	objects[1].location = LOC_WORN;

	//BDD when checking NOTWORN 1
	const char proc[] = { _NOTWORN, 1, 255 };
	do_action(proc, do_NOTWORN);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTWORN_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking NOTWORN [150]
	const char proc[] = { _NOTWORN|IND, 150, 255 };
	do_action(proc, do_NOTWORN);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CARRIED

void test_CARRIED_success()
{
	beforeEach();

	//BDD given player at loc 5 and object 1 is carried
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;

	//BDD when checking CARRIED 1
	const char proc[] = { _CARRIED, 1, 255 };
	do_action(proc, do_CARRIED);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CARRIED_fails()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 5
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking CARRIED 1
	const char proc[] = { _CARRIED, 1, 255 };
	do_action(proc, do_CARRIED);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_CARRIED_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 is carried
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking CARRIED [150]
	const char proc[] = { _CARRIED|IND, 150, 255 };
	do_action(proc, do_CARRIED);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTCARR

void test_NOTCARR_success()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking NOTCARR 1
	const char proc[] = { _NOTCARR, 1, 255 };
	do_action(proc, do_NOTCARR);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTCARR_fails()
{
	beforeEach();

	//BDD given player at loc 5 and object 1 is carried
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;

	//BDD when checking NOTCARR 1
	const char proc[] = { _NOTCARR, 1, 255 };
	do_action(proc, do_NOTCARR);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTCARR_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking NOTCARR [150]
	const char proc[] = { _NOTCARR|IND, 150, 255 };
	do_action(proc, do_NOTCARR);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ISAT

void test_ISAT_success()
{
	beforeEach();

	//BDD given object 1 is at loc 5
	objects[1].location = 5;

	//BDD when checking ISAT 1 5
	const char proc[] = { _ISAT, 1, 5, 255 };
	do_action(proc, do_ISAT);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_fails()
{
	beforeEach();
	
	//BDD given object 1 at loc 5
	objects[1].location = 5;

	//BDD when checking ISAT 1 6
	const char proc[] = { _ISAT, 1, 6, 255 };
	do_action(proc, do_ISAT);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 is HERE
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 5;

	//BDD when checking ISAT [150] HERE
	const char proc[] = { _ISAT|IND, 150, LOC_HERE, 255 };
	do_action(proc, do_ISAT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ISNOTAT

void test_ISNOTAT_success()
{
	beforeEach();
	
	//BDD given object 1 at loc 2
	objects[1].location = 2;

	//BDD when checking ISNOTAT 1 5
	const char proc[] = { _ISNOTAT, 1, 5, 255 };
	do_action(proc, do_ISNOTAT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISNOTAT_fails()
{
	beforeEach();

	//BDD given object 1 is at loc 5
	objects[1].location = 5;

	//BDD when checking ISNOTAT 1 5
	const char proc[] = { _ISNOTAT, 1, 5, 255 };
	do_action(proc, do_ISNOTAT);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ISNOTAT_indirection()
{
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2 and flag 150 with value 1
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking ISNOTAT [150] HERE
	const char proc[] = { _ISNOTAT|IND, 150, LOC_HERE, 255 };
	do_action(proc, do_ISNOTAT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for values/flags comparation [10 condacts]
// =============================================================================

// =============================================================================
// Tests ZERO

void test_ZERO_success()
{
	beforeEach();

	//BDD given flag 150 with value 0
	flags[150] = 0;

	//BDD when checking ZERO 150
	const char proc[] = { _ZERO, 150, 255 };
	do_action(proc, do_ZERO);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ZERO_fails()
{
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking ZERO 150
	const char proc[] = { _ZERO, 150, 255 };
	do_action(proc, do_ZERO);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ZERO_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 5
	flags[150] = 1;
	flags[1] = 5;

	//BDD when checking ZERO [150]
	const char proc[] = { _ZERO|IND, 150, 255 };
	do_action(proc, do_ZERO);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTZERO

void test_NOTZERO_success()
{
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking NOTZERO 150
	const char proc[] = { _NOTZERO, 150, 255 };
	do_action(proc, do_NOTZERO);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTZERO_fails()
{
	beforeEach();

	//BDD given flag 150 with value 0
	flags[150] = 0;

	//BDD when checking NOTZERO 150
	const char proc[] = { _NOTZERO, 150, 255 };
	do_action(proc, do_NOTZERO);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTZERO_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking NOTZERO [150]
	const char proc[] = { _NOTZERO|IND, 150, 255 };
	do_action(proc, do_NOTZERO);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests EQ

void test_EQ_success()
{
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking EQ 150 25
	const char proc[] = { _EQ, 150, 25, 255 };
	do_action(proc, do_EQ);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_EQ_fails()
{
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking EQ 150 0
	const char proc[] = { _EQ, 150, 0, 255 };
	do_action(proc, do_EQ);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_EQ_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking EQ [150] 25
	const char proc[] = { _EQ|IND, 150, 25, 255 };
	do_action(proc, do_EQ);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTEQ

void test_NOTEQ_success()
{
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking NOTEQ 150 0
	const char proc[] = { _NOTEQ, 150, 0, 255 };
	do_action(proc, do_NOTEQ);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTEQ_fails()
{
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking NOTEQ 150 25
	const char proc[] = { _NOTEQ, 150, 25, 255 };
	do_action(proc, do_NOTEQ);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTEQ_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking NOTEQ [150] 5
	const char proc[] = { _NOTEQ|IND, 150, 5, 255 };
	do_action(proc, do_NOTEQ);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests GT

void test_GT_success()
{
	beforeEach();

	//BDD given flag 150 with value 50
	flags[150] = 50;

	//BDD when checking GT 150 25
	const char proc[] = { _GT, 150, 25, 255 };
	do_action(proc, do_GT);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_GT_fails()
{
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking GT 150 50
	const char proc[] = { _GT, 150, 50, 255 };
	do_action(proc, do_GT);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GT_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 50
	flags[150] = 1;
	flags[1] = 50;

	//BDD when checking GT [150] 25
	const char proc[] = { _GT|IND, 150, 25, 255 };
	do_action(proc, do_GT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests LT

void test_LT_success()
{
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking LT 150 50
	const char proc[] = { _LT, 150, 50, 255 };
	do_action(proc, do_LT);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_LT_fails()
{
	beforeEach();
	
	//BDD given flag 150 with value 50
	flags[150] = 50;

	//BDD when checking LT 150 25
	const char proc[] = { _LT, 150, 25, 255 };
	do_action(proc, do_LT);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_LT_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking LT [150] 50
	const char proc[] = { _LT|IND, 150, 50, 255 };
	do_action(proc, do_LT);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SAME

void test_SAME_success()
{
	beforeEach();

	//BDD given flag 150 and 151 with value 25
	flags[150] = 25;
	flags[151] = 25;

	//BDD when checking SAME 150 151
	const char proc[] = { _SAME, 150, 151, 255 };
	do_action(proc, do_SAME);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SAME_fails()
{
	beforeEach();
	
	//BDD given flag 150 with value 25 and flag 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking SAME 150 151
	const char proc[] = { _SAME, 150, 151, 255 };
	do_action(proc, do_SAME);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_SAME_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25 and flag 151 with value 25
	flags[150] = 1;
	flags[1] = 25;
	flags[151] = 25;

	//BDD when checking SAME [150] 151
	const char proc[] = { _SAME|IND, 150, 151, 255 };
	do_action(proc, do_SAME);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTSAME

void test_NOTSAME_success()
{
	beforeEach();
	
	//BDD given flag 150 with value 25 and flag 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking NOTSAME 150 151
	const char proc[] = { _NOTSAME, 150, 151, 255 };
	do_action(proc, do_NOTSAME);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTSAME_fails()
{
	beforeEach();

	//BDD given flag 150 with value 25 and flag 151 with value 25
	flags[150] = 25;
	flags[151] = 25;

	//BDD when checking NOTSAME 150 151
	const char proc[] = { _NOTSAME, 150, 151, 255 };
	do_action(proc, do_NOTSAME);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTSAME_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25 and flag 151 with value 50
	flags[150] = 1;
	flags[1] = 25;
	flags[151] = 50;

	//BDD when checking NOTSAME [150] 151
	const char proc[] = { _NOTSAME|IND, 150, 5, 255 };
	do_action(proc, do_NOTSAME);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests BIGGER

void test_BIGGER_success()
{
	beforeEach();

	//BDD given flag 150 with value 50 and 151 with value 25
	flags[150] = 50;
	flags[151] = 25;

	//BDD when checking BIGGER 150 151
	const char proc[] = { _BIGGER, 150, 151, 255 };
	do_action(proc, do_BIGGER);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_BIGGER_fails()
{
	beforeEach();
	
	//BDD given flag 150 with value 25 and flag 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking BIGGER 150 151
	const char proc[] = { _BIGGER, 150, 151, 255 };
	do_action(proc, do_BIGGER);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_BIGGER_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 50 and flag 151 with value 25
	flags[150] = 1;
	flags[1] = 50;
	flags[151] = 25;

	//BDD when checking BIGGER [150] 151
	const char proc[] = { _BIGGER|IND, 150, 151, 255 };
	do_action(proc, do_BIGGER);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SMALLER

void test_SMALLER_success()
{
	beforeEach();

	//BDD given flag 150 with value 25 and 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking SMALLER 150 151
	const char proc[] = { _SMALLER, 150, 151, 255 };
	do_action(proc, do_SMALLER);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SMALLER_fails()
{
	beforeEach();
	
	//BDD given flag 150 with value 50 and flag 151 with value 25
	flags[150] = 50;
	flags[151] = 25;

	//BDD when checking SMALLER 150 151
	const char proc[] = { _SMALLER, 150, 151, 255 };
	do_action(proc, do_SMALLER);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_SMALLER_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25 and flag 151 with value 50
	flags[150] = 1;
	flags[1] = 25;
	flags[151] = 50;

	//BDD when checking SMALLER [150] 151
	const char proc[] = { _SMALLER|IND, 150, 151, 255 };
	do_action(proc, do_SMALLER);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions to check logical sentence [5 condacts]
// =============================================================================

// =============================================================================
// Tests ADJECT1

void test_ADJECT1_success()
{
	beforeEach();

	//BDD given the adject1 1 from Logical Sentence
	flags[fAdject1] = 1;

	//BDD when checking ADJECT1 1
	const char proc[] = { _ADJECT1, 1, 255 };
	do_action(proc, do_ADJECT1);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT1_fails()
{
	beforeEach();
	
	//BDD given the adject1 1 from Logical Sentence
	flags[fAdject1] = 1;

	//BDD when checking ADJECT1 2
	const char proc[] = { _ADJECT1, 2, 255 };
	do_action(proc, do_ADJECT1);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT1_indirection()
{
	beforeEach();
	
	//BDD given the adject1 1 from Logical Sentence and flag 150 with value 1
	flags[fAdject1] = 1;
	flags[150] = 1;

	//BDD when checking ADJECT1 [150]
	const char proc[] = { _ADJECT1|IND, 150, 255 };
	do_action(proc, do_ADJECT1);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ADVERB

void test_ADVERB_success()
{
	beforeEach();

	//BDD given the adverb 1 from Logical Sentence
	flags[fAdverb] = 1;

	//BDD when checking ADVERB 1
	const char proc[] = { _ADVERB, 1, 255 };
	do_action(proc, do_ADVERB);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADVERB_fails()
{
	beforeEach();
	
	//BDD given the adverb 1 from Logical Sentence
	flags[fAdverb] = 1;

	//BDD when checking ADVERB 2
	const char proc[] = { _ADVERB, 2, 255 };
	do_action(proc, do_ADVERB);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ADVERB_indirection()
{
	beforeEach();
	
	//BDD given the adverb 1 from Logical Sentence and flag 150 with value 1
	flags[fAdverb] = 1;
	flags[150] = 1;

	//BDD when checking ADVERB [150]
	const char proc[] = { _ADVERB|IND, 150, 255 };
	do_action(proc, do_ADVERB);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PREP

void test_PREP_success()
{
	beforeEach();

	//BDD given the prep 1 from Logical Sentence
	flags[fPrep] = 1;

	//BDD when checking PREP 1
	const char proc[] = { _PREP, 1, 255 };
	do_action(proc, do_PREP);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PREP_fails()
{
	beforeEach();
	
	//BDD given the prep 1 from Logical Sentence
	flags[fPrep] = 1;

	//BDD when checking PREP 2
	const char proc[] = { _PREP, 2, 255 };
	do_action(proc, do_PREP);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PREP_indirection()
{
	beforeEach();
	
	//BDD given the prep 1 from Logical Sentence and flag 150 with value 1
	flags[fPrep] = 1;
	flags[150] = 1;

	//BDD when checking PREP [150]
	const char proc[] = { _PREP|IND, 150, 255 };
	do_action(proc, do_PREP);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOUN2

void test_NOUN2_success()
{
	beforeEach();

	//BDD given the noun2 1 from Logical Sentence
	flags[fNoun2] = 1;

	//BDD when checking NOUN2 1
	const char proc[] = { _NOUN2, 1, 255 };
	do_action(proc, do_NOUN2);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOUN2_fails()
{
	beforeEach();
	
	//BDD given the noun2 1 from Logical Sentence
	flags[fNoun2] = 1;

	//BDD when checking NOUN2 2
	const char proc[] = { _NOUN2, 2, 255 };
	do_action(proc, do_NOUN2);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOUN2_indirection()
{
	beforeEach();
	
	//BDD given the noun2 1 from Logical Sentence and flag 150 with value 1
	flags[fNoun2] = 1;
	flags[150] = 1;

	//BDD when checking NOUN2 [150]
	const char proc[] = { _NOUN2|IND, 150, 255 };
	do_action(proc, do_NOUN2);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ADJECT2

void test_ADJECT2_success()
{
	beforeEach();

	//BDD given the adject2 1 from Logical Sentence
	flags[fAdject2] = 1;

	//BDD when checking ADJECT2 1
	const char proc[] = { _ADJECT2, 1, 255 };
	do_action(proc, do_ADJECT2);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT2_fails()
{
	beforeEach();
	
	//BDD given the adject2 1 from Logical Sentence
	flags[fAdject2] = 1;

	//BDD when checking ADJECT2 2
	const char proc[] = { _ADJECT2, 2, 255 };
	do_action(proc, do_ADJECT2);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT2_indirection()
{
	beforeEach();
	
	//BDD given the adject2 1 from Logical Sentence and flag 150 with value 1
	flags[fAdject2] = 1;
	flags[150] = 1;

	//BDD when checking ADJECT2 [150]
	const char proc[] = { _ADJECT2|IND, 150, 255 };
	do_action(proc, do_ADJECT2);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for random [1 condacts]
// =============================================================================

// =============================================================================
// Tests CHANCE

void test_CHANCE_0_fails()
{
	beforeEach();

	//BDD given none

	//BDD when checking CHANCE 0
	const char proc[] = { _CHANCE, 0, 255 };
	do_action(proc, do_CHANCE);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_CHANCE_255_success()
{
	beforeEach();
	
	//BDD given none

	//BDD when checking CHANCE 255
	const char proc[] = { _CHANCE, 255, 255 };
	do_action(proc, do_CHANCE);
	
	//BDD then succes
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CHANCE_indirection()
{
	beforeEach();
	
	//BDD given flag 150 with value 255
	flags[150] = 255;

	//BDD when checking CHANCE [150]
	const char proc[] = { _CHANCE|IND, 150, 255 };
	do_action(proc, do_CHANCE);
	
	//BDD then succes
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for sub-process success/fail [2 condacts]
// =============================================================================

// =============================================================================
// Tests ISDONE

void test_ISDONE_success()
{
	beforeEach();

	//BDD given last table executed at least one action
	lastIsDone = 1;

	//BDD when checking ISDONE
	const char proc[] = { _ISDONE, 255 };
	do_action(proc, do_ISDONE);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	//SUCCEED();
	TODO("Simulate DONE");
}

void test_ISDONE_fails()
{
	beforeEach();
	
	//BDD given last table not executed at least one action
	lastIsDone = 0;

	//BDD when checking ISDONE
	const char proc[] = { _ISDONE, 255 };
	do_action(proc, do_ISDONE);
	
	//BDD then succes
	ASSERT(!checkEntry, ERROR);
	//SUCCEED();
	TODO("Simulate NOTDONE");
}

// =============================================================================
// Tests ISNDONE

void test_ISNDONE_success()		//TODO improve this test
{
	beforeEach();

	//BDD given last table not executed at least one action
	lastIsDone = 0;

	//BDD when checking ISNDONE
	const char proc[] = { _ISNDONE, 255 };
	do_action(proc, do_ISNDONE);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISNDONE_fails()		//TODO improve this test
{
	beforeEach();
	
	//BDD given last table executed at least one action
	lastIsDone = 1;

	//BDD when checking ISNDONE
	const char proc[] = { _ISNDONE, 255 };
	do_action(proc, do_ISNDONE);
	
	//BDD then succes
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for object attributes [2 condacts]
// =============================================================================

// =============================================================================
// Tests HASAT

void test_HASAT_success()
{
	beforeEach();

	//BDD given current object attribute 10 is set
	flags[fCOAtt] = 0b00000100;

	//BDD when checking HASAT 10
	const char proc[] = { _HASAT, 10, 255 };
	do_action(proc, do_HASAT);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_HASAT_fails()
{
	beforeEach();

	//BDD given current object attribute 5 is not set
	flags[fCOAtt + 1] = 0b11011111;

	//BDD when checking HASAT 5
	const char proc[] = { _HASAT, 5, 255 };
	do_action(proc, do_HASAT);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_HASAT_indirection()
{
	beforeEach();

	//BDD given current object attribute 10 is set and flag 150 with value 10
	flags[fCOAtt] = 0b00000100;
	flags[150] = 10;

	//BDD when checking HASAT [150]
	const char proc[] = { _HASAT|IND, 150, 255 };
	do_action(proc, do_HASAT);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests HASNAT

void test_HASNAT_success()
{
	beforeEach();

	//BDD given current object attribute 5 is not set
	flags[fCOAtt + 1] = 0b11011111;

	//BDD when checking HASNAT 5
	const char proc[] = { _HASNAT, 5, 255 };
	do_action(proc, do_HASNAT);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_HASNAT_fails()
{
	beforeEach();

	//BDD given current object attribute 10 is set
	flags[fCOAtt] = 0b00000100;

	//BDD when checking HASNAT 10
	const char proc[] = { _HASNAT, 10, 255 };
	do_action(proc, do_HASNAT);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_HASNAT_indirection()
{
	beforeEach();

	//BDD given current object attribute 10 is set and flag 150 with value 10
	flags[fCOAtt] = 0b11111011;
	flags[150] = 10;

	//BDD when checking HASNAT [150]
	const char proc[] = { _HASNAT|IND, 150, 255 };
	do_action(proc, do_HASNAT);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for player interaction [2 condacts]
// =============================================================================

// =============================================================================
// Tests INKEY

void test_INKEY_success()
{
	beforeEach();

	//BDD given 'A' key pressed
	fake_keyPressed = 'A';

	//BDD when checking INKEY
	const char proc[] = { _INKEY, 255 };
	do_action(proc, do_INKEY);

	//BDD then success
	ASSERT(flags[fKey1] == 'A', ERROR);
	SUCCEED();
}

// =============================================================================
// Tests QUIT

void test_QUIT_success()
{
	beforeEach();

	//BDD given 'Y' key pressed
	fake_keyPressed = 'Y';

	//BDD when checking QUIT
	// const char proc[] = { _QUIT, 255 };
	// do_action(proc, do_QUIT);

	//BDD then success
	// ASSERT(!checkEntry, ERROR);
	//SUCCEED();
	TODO("Simulate user input");
}

// =============================================================================
// Actions for object position manipulation [21 condacts]
// =============================================================================

// =============================================================================
// Tests GET

void test_GET_carried()
{
	beforeEach();

	//BDD given a carried object 1
	objects[1].location = LOC_CARRIED;

	//BDD when checking GET 1
	const char proc[] = { _GET, 1, 255 };
	do_action(proc, do_GET);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 25, "SystemMessage 25 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_worn()
{
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking GET 1
	const char proc[] = { _GET, 1, 255 };
	do_action(proc, do_GET);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 25, "SystemMessage 25 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_notHere()
{
	beforeEach();

	//BDD given an object 1 at another location than the player
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking GET 1
	const char proc[] = { _GET, 1, 255 };
	do_action(proc, do_GET);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 26, "SystemMessage 26 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_maxWeight()
{
	beforeEach();

	//BDD given an object 1 with weight (50), more than player can carry (25)
	flags[fPlayer] = 1;
	flags[fStrength] = 25;
	objects[1].location = 1;
	objects[1].attribs.mask.weight = 50;

	//BDD when checking GET 1
	const char proc[] = { _GET, 1, 255 };
	do_action(proc, do_GET);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 43, "SystemMessage 43 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_maxObjs()
{
	beforeEach();

	//BDD given an object 1 at same location than player, but exceeds max number of objected carried
	flags[fPlayer] = 1;
	flags[fNOCarr] = 10;
	flags[fMaxCarr] = 10;
	objects[1].location = 1;

	//BDD when checking GET 1
	const char proc[] = { _GET, 1, 255 };
	do_action(proc, do_GET);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 27, "SystemMessage 27 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_success()
{
	beforeEach();

	//BDD given an object 1 at same location than player, and could be carried
	flags[fPlayer] = 1;
	flags[fMaxCarr] = 255;
	objects[1].location = 1;

	//BDD when checking GET 1
	const char proc[] = { _GET, 1, 255 };
	do_action(proc, do_GET);

	//BDD then success
	ASSERT(fake_lastSysMesPrinted == 36, "SystemMessage 36 not printed");
	ASSERT(objects[1].location == LOC_CARRIED, "Object not carried");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DROP

void test_DROP_success()
{
	beforeEach();

	//BDD given a carried object 1
	flags[fPlayer] = 1;
	flags[fMaxCarr] = 255;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking DROP 1
	const char proc[] = { _DROP, 1, 255 };
	do_action(proc, do_DROP);

	//BDD then success
	ASSERT(fake_lastSysMesPrinted == 39, "SystemMessage 39 not printed");
	ASSERT(objects[1].location == flags[fPlayer], "Droped object not here");
	ASSERT(flags[fNOCarr] == 0, "Object counter fail");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_DROP_worn()
{
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking DROP 1
	const char proc[] = { _DROP, 1, 255 };
	do_action(proc, do_DROP);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 24, "SystemMessage 24 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_DROP_isHere()
{
	beforeEach();

	//BDD given an object 1 placed at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking DROP 1
	const char proc[] = { _DROP, 1, 255 };
	do_action(proc, do_DROP);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 49, "SystemMessage 49 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_DROP_notHere()
{
	beforeEach();

	//BDD given an object 1 placed at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking DROP 1
	const char proc[] = { _DROP, 1, 255 };
	do_action(proc, do_DROP);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 28, "SystemMessage 28 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}


// =============================================================================
// Tests WEAR

void test_WEAR_isHere()
{
	beforeEach();

	//BDD given an object 1 placed at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 1;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	const char proc[] = { _WEAR, 1, 255 };
	do_action(proc, do_WEAR);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 49, "SystemMessage 49 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_worn()
{
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	const char proc[] = { _WEAR, 1, 255 };
	do_action(proc, do_WEAR);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 29, "SystemMessage 29 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_notCarried()
{
	beforeEach();

	//BDD given a not carried object 1 and player at location 1
	flags[fPlayer] = 1;
	objects[1].location = LOC_NOTCREATED;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	const char proc[] = { _WEAR, 1, 255 };
	do_action(proc, do_WEAR);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 28, "SystemMessage 28 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_notWareable()
{
	beforeEach();

	//BDD given a carried object 1 but is not wareable
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.isWareable = 0;

	//BDD when checking WEAR 1
	const char proc[] = { _WEAR, 1, 255 };
	do_action(proc, do_WEAR);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 40, "SystemMessage 40 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_success()
{
	beforeEach();

	//BDD given a wareable and carried object 1
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	const char proc[] = { _WEAR, 1, 255 };
	do_action(proc, do_WEAR);

	//BDD then success
	ASSERT(fake_lastSysMesPrinted == 37, "SystemMessage 37 not printed");
	ASSERT(objects[1].location == LOC_WORN, "Object not worn");
	ASSERT(flags[fNOCarr] == 0, "Object counter fail");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests REMOVE

void test_REMOVE_carried()
{
	beforeEach();

	//BDD given a carried object 1
	objects[1].location = LOC_CARRIED;

	//BDD when checking REMOVE 1
	const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc, do_REMOVE);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 50, "SystemMessage 50 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_isHere()
{
	beforeEach();

	//BDD given a object 1 at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking REMOVE 1
	const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc, do_REMOVE);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 50, "SystemMessage 50 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_notHere()
{
	beforeEach();

	//BDD given a object 1 at different location than player
	flags[fPlayer] = 1;
	objects[1].location = LOC_NOTCREATED - 1;

	//BDD when checking REMOVE 1
	const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc, do_REMOVE);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 23, "SystemMessage 23 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_notWareable()
{
	beforeEach();

	//BDD given a carried and not wareable object 1
	flags[fPlayer] = 1;
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 0;

	//BDD when checking REMOVE 1
	const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc, do_REMOVE);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 41, "SystemMessage 41 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_maxObjs()
{
	beforeEach();

	//BDD given a worn object 1 but max carried objects reached
	flags[fNOCarr] = 10;
	flags[fMaxCarr] = 10;
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking REMOVE 1
	const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc, do_REMOVE);

	//BDD then fails
	ASSERT(fake_lastSysMesPrinted == 42, "SystemMessage 42 not printed");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_success()
{
	beforeEach();

	//BDD given a worn object 1
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 10;
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking REMOVE 1
	const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc, do_REMOVE);

	//BDD then success
	ASSERT(fake_lastSysMesPrinted == 38, "SystemMessage 38 not printed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}


// =============================================================================
// Tests CREATE

void test_CREATE_success()
{
	beforeEach();

	//BDD given a uncreated object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CREATE 1
	const char proc[] = { _CREATE, 1, 255 };
	do_action(proc, do_CREATE);

	//BDD then success
	ASSERT(objects[1].location == flags[fPlayer], "Object not in player location");
	ASSERT(flags[fNOCarr] == 1, "Carried objects number changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CREATE_carried()
{
	beforeEach();

	//BDD given a carried object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking CREATE 1
	const char proc[] = { _CREATE, 1, 255 };
	do_action(proc, do_CREATE);

	//BDD then success
	ASSERT(objects[1].location == flags[fPlayer], "Object not in player location");
	ASSERT(flags[fNOCarr] == 0, "Carried objects number not changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CREATE_indirection()
{
	beforeEach();

	//BDD given a uncreated object 1 and the player at location 2 and flag 150 with value 1
	flags[150] = 1;
	flags[fPlayer] = 2;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CREATE 1
	const char proc[] = { _CREATE, 1, 255 };
	do_action(proc, do_CREATE);

	//BDD then success
	ASSERT(objects[1].location == flags[fPlayer], "Object not in player location");
	ASSERT(flags[fNOCarr] == 1, "Carried objects number changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DESTROY

void test_DESTROY_success()
{
	beforeEach();

	//BDD given an object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = 1;

	//BDD when checking DESTROY 1
	const char proc[] = { _DESTROY, 1, 255 };
	do_action(proc, do_DESTROY);

	//BDD then success
	ASSERT(objects[1].location == LOC_NOTCREATED, "Object not destroyed");
	ASSERT(flags[fNOCarr] == 1, "Carried objects number changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_DESTROY_carried()
{
	beforeEach();

	//BDD given a carried object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking DESTROY 1
	const char proc[] = { _DESTROY, 1, 255 };
	do_action(proc, do_DESTROY);

	//BDD then success
	ASSERT(objects[1].location == LOC_NOTCREATED, "Object not destroyed");
	ASSERT(flags[fNOCarr] == 0, "Carried objects number not changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_DESTROY_indirection()
{
	beforeEach();

	//BDD given an object 1 and the player at location 2 and flag 150 with value 1
	flags[150] = 1;
	flags[fPlayer] = 2;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CREATE 1
	const char proc[] = { _DESTROY, 1, 255 };
	do_action(proc, do_DESTROY);

	//BDD then success
	ASSERT(objects[1].location == LOC_NOTCREATED, "Object not destroyed");
	ASSERT(flags[fNOCarr] == 1, "Carried objects number changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SWAP

// =============================================================================
// Tests PLACE

// =============================================================================
// Tests PUTO

// =============================================================================
// Tests PUTIN

// =============================================================================
// Tests TAKEOUT

// =============================================================================
// Tests DROPALL

// =============================================================================
// Tests AUTOG

// =============================================================================
// Tests AUTOD

// =============================================================================
// Tests AUTOW

// =============================================================================
// Tests AUTOR

// =============================================================================
// Tests AUTOP

// =============================================================================
// Tests AUTOT

// =============================================================================
// Tests COPYOO

// =============================================================================
// Tests RESET

// =============================================================================
// Actions for object in flags manipulation [5 condacts]
// =============================================================================

// =============================================================================
// Tests COPYOF

// =============================================================================
// Tests COPYFO

// =============================================================================
// Tests WHATO

// =============================================================================
// Tests SETCO

// =============================================================================
// Tests WEIGH

// =============================================================================
// Actions to manipulate flags [11 condacts]
// =============================================================================

// =============================================================================
// Tests SET

// =============================================================================
// Tests CLEAR

// =============================================================================
// Tests LET

// =============================================================================
// Tests PLUS

// =============================================================================
// Tests MINUS

// =============================================================================
// Tests ADD

// =============================================================================
// Tests HASNAT

// =============================================================================
// Tests SUB

// =============================================================================
// Tests COPYFF

// =============================================================================
// Tests COPYBF

// =============================================================================
// Tests RANDOM

// =============================================================================
// Tests MOVE

// =============================================================================
// Actions to manipulate player flags [3 condacts]
// =============================================================================














// =============================================================================
// main

int main(char** argv, int argc)
{
	cputs("### UNIT TESTS of MSX2DAAD ###");
	beforeAll();

	test_AT_success(); test_AT_fails(); test_AT_indirection();
	test_NOTAT_success(); test_NOTAT_fails(); test_NOTAT_indirection();
	test_ATGT_success(); test_ATGT_fails(); test_ATGT_indirection();
	test_ATLT_success(); test_ATLT_fails(); test_ATLT_indirection();

	test_PRESENT_success(); test_PRESENT_fails(); test_PRESENT_indirection();
	test_ABSENT_success(); test_ABSENT_fails(); test_ABSENT_indirection();
	test_WORN_success(); test_WORN_fails(); test_WORN_indirection();
	test_NOTWORN_success(); test_NOTWORN_fails(); test_NOTWORN_indirection();
	test_CARRIED_success(); test_CARRIED_fails(); test_CARRIED_indirection();
	test_NOTCARR_success(); test_NOTCARR_fails(); test_NOTCARR_indirection();
	test_ISAT_success(); test_ISAT_fails(); test_ISAT_indirection();
	test_ISNOTAT_success(); test_ISNOTAT_fails(); test_ISNOTAT_indirection();

	test_ZERO_success(); test_ZERO_fails(); test_ZERO_indirection();
	test_NOTZERO_success(); test_NOTZERO_fails(); test_NOTZERO_indirection();
	test_EQ_success(); test_EQ_fails(); test_EQ_indirection();
	test_NOTEQ_success(); test_NOTEQ_fails(); test_NOTEQ_indirection();
	test_GT_success(); test_GT_fails(); test_GT_indirection();
	test_LT_success(); test_LT_fails(); test_LT_indirection();
	test_SAME_success(); test_SAME_fails(); test_SAME_indirection();
	test_NOTSAME_success(); test_NOTSAME_fails(); test_NOTSAME_indirection();
	test_BIGGER_success(); test_BIGGER_fails(); test_BIGGER_indirection();
	test_SMALLER_success(); test_SMALLER_fails(); test_SMALLER_indirection();
	
	test_ADJECT1_success(); test_ADJECT1_fails(); test_ADJECT1_indirection();
	test_ADVERB_success(); test_ADVERB_fails(); test_ADVERB_indirection();
	test_PREP_success(); test_PREP_fails(); test_PREP_indirection();
	test_NOUN2_success(); test_NOUN2_fails(); test_NOUN2_indirection();
	test_ADJECT2_success(); test_ADJECT2_fails(); test_ADJECT2_indirection();

	test_CHANCE_0_fails(); test_CHANCE_255_success(); test_CHANCE_indirection();

	test_ISDONE_success(); test_ISDONE_fails();
	test_ISNDONE_success(); test_ISNDONE_fails();

	test_HASAT_success(); test_HASAT_fails(); test_HASAT_indirection();
	test_HASNAT_success(); test_HASNAT_fails(); test_HASNAT_indirection();

	test_INKEY_success();
	test_QUIT_success();

	test_GET_carried(); test_GET_worn(); test_GET_notHere(); test_GET_maxWeight(); test_GET_maxObjs(); test_GET_success();
	test_DROP_success(); test_DROP_worn(); test_DROP_isHere(); test_DROP_notHere();
	test_WEAR_isHere(); test_WEAR_worn(); test_WEAR_notCarried(); test_WEAR_notWareable(); test_WEAR_success();
	test_REMOVE_carried(); test_REMOVE_isHere(); test_REMOVE_notHere(); test_REMOVE_notWareable(); test_REMOVE_maxObjs(); test_REMOVE_success();
	test_CREATE_success(); test_CREATE_carried(); test_CREATE_indirection();
	test_DESTROY_success(); test_DESTROY_carried(); test_DESTROY_indirection();

}
