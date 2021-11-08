#define WINDOWS_NUM		8

#include <string.h>
#include "assert.h"
#include "daad.h"
#include "daad_platform_api.h"
#include "daad_condacts.h"

#pragma opt_code_size


static const char __THIS_FILE__[] = __FILE__;

#undef ASSERT
#undef ASSERT_EQUAL
#undef FAIL
#undef SUCCEED
#undef TODO
#define ASSERT(cond, failMsg)					_ASSERT_TRUE(cond, failMsg, __THIS_FILE__, _func, __LINE__)
#define ASSERT_EQUAL(value, expected, failMsg)	_ASSERT_EQUAL((uint16_t)(value), (uint16_t)(expected), failMsg, __THIS_FILE__, _func, __LINE__)
#define FAIL(failMsg)							_FAIL(failMsg, __THIS_FILE__, _func, __LINE__)
#define SUCCEED()								_SUCCEED(__THIS_FILE__, _func)
#define TODO(infoMsg)							_TODO(infoMsg, __THIS_FILE__, __func__)


#define pPROC 		currProc->condact
#define checkEntry  currProc->continueEntry

#define IND						128
#define MOCK_NUM_OBJECTS		10


const char ERROR[] = "Error with checkEntry";
const char ERROR_ISDONE[] = "Error with isDone";
const char ERROR_SYSMES[] = "SystemMessage";
const char ERROR_OBJLOC[] = "Object location";
const char ERROR_CARROBJNUM[] = "Carried objects number";
const char TODO_GENERIC[] = "-----";

uint8_t fake_keyPressed;
int16_t fake_lastSysMesPrinted;

extern const CONDACT_LIST condactList[];

// =============================================================================
// Global variables

PROCstack procStack[NUM_PROCS];		// Stack of calls using PROCESS condact.
PROCstack *currProc;				// Pointer to current active condact.

bool indirection;					// True if the current condact use indirection for the first argument.
//bool checkEntry;					// Boolean to check if a Process entry must continue or a condition fails.
bool isDone;						// Variables for ISDONE/ISNDONE condacts.
bool lastPicShow;					// True if last location picture was drawed.


uint8_t    *ddb;						// Where the DDB is allocated
DDB_Header *hdr;						// Struct pointer to DDB Header
Object     *objects;					// Memory allocation for objects data
uint8_t     flags[256];					// DAAD flags (256 bytes)
char       *ramsave;					// Memory to store ram save (RAMSAVE)

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

void prompt(bool printPromptMsg) {}
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
void printSystemMsg(uint8_t num) __z88dk_fastcall { if (num!=51) fake_lastSysMesPrinted = num; }
void printUserMsg(uint8_t num) __z88dk_fastcall {}
void printLocationMsg(uint8_t num) __z88dk_fastcall {}
void printObjectMsg(uint8_t num) __z88dk_fastcall {}
void printObjectMsgModif(uint8_t num, char modif) {}
uint8_t getObjectId(uint8_t noun, uint8_t adjc, uint16_t location) {}
uint8_t getObjectWeight(uint8_t objno, bool isCarriedWorn) { if (objno==NULLWORD) return 0; else return objects[objno].attribs.mask.weight; }
void referencedObject(uint8_t objno) __z88dk_fastcall { flags[fCONum] = objno; flags[fCOLoc] = objects[objno].location; }

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
bool gfxPicturePrepare(uint8_t location) {}
inline bool gfxPictureShow() {}
inline void gfxRoutines(uint8_t routine, uint8_t value) {}

// SFX functions
void sfxInit() {}
void sfxWriteRegister(uint8_t reg, uint8_t value) {}
void sfxTone(uint8_t value1, uint8_t value2) {}


// =============================================================================

static void beforeAll()
{
	sfxInit();
	objects = malloc(sizeof(Object) * MOCK_NUM_OBJECTS);
	windows = malloc(sizeof(Window) * WINDOWS_NUM);
	currProc = malloc(sizeof(PROCstack));

	ddb = malloc(sizeof(DDB_Header));
	hdr = (DDB_Header *)ddb;
}

static void beforeEach()
{
	initFlags();
	initializePROC();
	pushPROC(0);
	
	checkEntry = true;
	isDone = false;

	memset(hdr, 0, sizeof(DDB_Header));
	memset(objects, 0, sizeof(Object) * MOCK_NUM_OBJECTS);
	memset(windows, 0, sizeof(Window) * WINDOWS_NUM);

	hdr->numObjDsc = MOCK_NUM_OBJECTS;

	cw = windows;

	fake_keyPressed = 0;
	fake_lastSysMesPrinted = -1;
}

static void do_action(char *pProc)
{
	pPROC = pProc + 1;
	indirection = *pProc & IND;

	CondactStruct *currCondact = (CondactStruct*)pProc++;
	condactList[currCondact->condact].function();
}


// =============================================================================
// Conditions of player locations [4 condacts]
// =============================================================================

// =============================================================================
// Tests AT <locno>
/*	Succeeds if the current location is the same as locno. */

void test_AT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with AT 5
	static const char proc[] = { _AT, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_AT_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 21
	flags[fPlayer] = 21;

	//BDD when check it with AT 5
	static const char proc[] = { _AT, 5, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 5;

	//BDD when check it with AT @150
	static const char proc[] = { _AT|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTAT <locno>
/*	Succeeds if the current location is different to locno. */

void test_NOTAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with NOTAT 2
	char proc[] = { _NOTAT, 2, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTAT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with NOTAT 5
	char proc[] = { _NOTAT, 5, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 11;

	//BDD when check it with NOTAT @150
	static const char proc[] = { _NOTAT|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ATGT <locno>
/*	Succeeds if the current location is greater than locno. */

void test_ATGT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATGT 2
	static const char proc[] = { _ATGT, 2, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ATGT_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATGT 5
	static const char proc[] = { _ATGT, 5, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ATGT_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 2;

	//BDD when check it with ATGT @150
	static const char proc[] = { _ATGT|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ATLT <locno>
/*	Succeeds if the current location is less than locno. */

void test_ATLT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATLT 12
	static const char proc[] = { _ATLT, 12, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ATLT_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATLT 2
	static const char proc[] = { _ATLT, 2, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ATLT_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 12;

	//BDD when check it with ATLT @150
	static const char proc[] = { _ATLT|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions of object locations [8 condacts]
// =============================================================================

// =============================================================================
// Tests PRESENT <objno>
/*	Succeeds if Object objno. is carried (254), worn (253) or at the current 
	location [fPlayer]. */

void test_PRESENT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at same place
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking PRESENT 1
	static const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking PRESENT 1
	static const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 5
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = LOC_HERE;

	//BDD when checking PRESENT @150
	static const char proc[] = { _PRESENT|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ABSENT <objno>
/*	Succeeds if Object objno. is not carried (254), not worn (253) and not at 
	the current location [fPlayer]. */

void test_ABSENT_success()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking ABSENT 1
	static const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ABSENT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at same place
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking ABSENT 1
	static const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ABSENT_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking ABSENT @150
	static const char proc[] = { _ABSENT|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WORN <objno>
/*	Succeeds if object objno. is worn. */

void test_WORN_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is worn
	flags[fPlayer] = 5;
	objects[1].location = LOC_WORN;

	//BDD when checking WORN 1
	static const char proc[] = { _WORN, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WORN_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 5
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking WORN 1
	static const char proc[] = { _WORN, 1, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WORN_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 is worn
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = LOC_WORN;

	//BDD when checking WORN @150
	static const char proc[] = { _WORN|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTWORN <objno>
/*	Succeeds if Object objno. is not worn. */

void test_NOTWORN_success()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking NOTWORN 1
	static const char proc[] = { _NOTWORN, 1, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTWORN_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is worn
	flags[fPlayer] = 5;
	objects[1].location = LOC_WORN;

	//BDD when checking NOTWORN 1
	static const char proc[] = { _NOTWORN, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTWORN_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking NOTWORN @150
	static const char proc[] = { _NOTWORN|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CARRIED <objno>
/*	Succeeds if Object objno. is carried. */

void test_CARRIED_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is carried
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;

	//BDD when checking CARRIED 1
	static const char proc[] = { _CARRIED, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CARRIED_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 5
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking CARRIED 1
	static const char proc[] = { _CARRIED, 1, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_CARRIED_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 is carried
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking CARRIED @150
	static const char proc[] = { _CARRIED|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTCARR <objno>
/*	Succeeds if Object objno. is not carried. */

void test_NOTCARR_success()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking NOTCARR 1
	static const char proc[] = { _NOTCARR, 1, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTCARR_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is carried
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;

	//BDD when checking NOTCARR 1
	static const char proc[] = { _NOTCARR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTCARR_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking NOTCARR @150
	static const char proc[] = { _NOTCARR|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ISAT <objno> <locno+>
/*	Succeeds if Object objno. is at Location locno. */

void test_ISAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 is at loc 5
	objects[1].location = 5;

	//BDD when checking ISAT 1 5
	static const char proc[] = { _ISAT, 1, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given object 1 at loc 5
	objects[1].location = 5;

	//BDD when checking ISAT 1 6
	static const char proc[] = { _ISAT, 1, 6, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 is HERE
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 5;

	//BDD when checking ISAT @150 HERE
	static const char proc[] = { _ISAT|IND, 150, LOC_HERE, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ISNOTAT <objno> <locno+>
/*	Succeeds if Object objno. is not at Location locno. */

void test_ISNOTAT_success()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given object 1 at loc 2
	objects[1].location = 2;

	//BDD when checking ISNOTAT 1 5
	static const char proc[] = { _ISNOTAT, 1, 5, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISNOTAT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 is at loc 5
	objects[1].location = 5;

	//BDD when checking ISNOTAT 1 5
	static const char proc[] = { _ISNOTAT, 1, 5, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ISNOTAT_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given player at loc 5 and object 1 at loc 2 and flag 150 with value 1
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking ISNOTAT @150 HERE
	static const char proc[] = { _ISNOTAT|IND, 150, LOC_HERE, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for values/flags comparation [10 condacts]
// =============================================================================

// =============================================================================
// Tests ZERO <flagno>
/*	Succeeds if Flag flagno. is set to zero. */

void test_ZERO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 0
	flags[150] = 0;

	//BDD when checking ZERO 150
	static const char proc[] = { _ZERO, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ZERO_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking ZERO 150
	static const char proc[] = { _ZERO, 150, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ZERO_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 5
	flags[150] = 1;
	flags[1] = 5;

	//BDD when checking ZERO @150
	static const char proc[] = { _ZERO|IND, 150, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTZERO <flagno>
/*	Succeeds if Flag flagno. is not set to zero. */

void test_NOTZERO_success()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking NOTZERO 150
	static const char proc[] = { _NOTZERO, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTZERO_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 0
	flags[150] = 0;

	//BDD when checking NOTZERO 150
	static const char proc[] = { _NOTZERO, 150, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTZERO_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking NOTZERO @150
	static const char proc[] = { _NOTZERO|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests EQ <flagno> <value>
/*	Succeeds if Flag flagno. is equal to value. */

void test_EQ_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking EQ 150 25
	static const char proc[] = { _EQ, 150, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_EQ_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking EQ 150 0
	static const char proc[] = { _EQ, 150, 0, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_EQ_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking EQ @150 25
	static const char proc[] = { _EQ|IND, 150, 25, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTEQ <flagno> <value>
/*	Succeeds if Flag flagno. is not equal to value. */

void test_NOTEQ_success()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking NOTEQ 150 0
	static const char proc[] = { _NOTEQ, 150, 0, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTEQ_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking NOTEQ 150 25
	static const char proc[] = { _NOTEQ, 150, 25, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTEQ_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking NOTEQ @150 5
	static const char proc[] = { _NOTEQ|IND, 150, 5, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests GT <flagno> <value>
/*	Succeeds if Flag flagno. is greater than value. */

void test_GT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 50
	flags[150] = 50;

	//BDD when checking GT 150 25
	static const char proc[] = { _GT, 150, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_GT_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking GT 150 50
	static const char proc[] = { _GT, 150, 50, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GT_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 50
	flags[150] = 1;
	flags[1] = 50;

	//BDD when checking GT @150 25
	static const char proc[] = { _GT|IND, 150, 25, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests LT <flagno> <value>
/*	Succeeds if Flag flagno. is set to less than value. */

void test_LT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking LT 150 50
	static const char proc[] = { _LT, 150, 50, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_LT_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 50
	flags[150] = 50;

	//BDD when checking LT 150 25
	static const char proc[] = { _LT, 150, 25, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_LT_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking LT @150 50
	static const char proc[] = { _LT|IND, 150, 50, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SAME <flagno1> <flagno2>
/*	Succeeds if Flag flagno 1 has the same value as Flag flagno 2. */

void test_SAME_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 and 151 with value 25
	flags[150] = 25;
	flags[151] = 25;

	//BDD when checking SAME 150 151
	static const char proc[] = { _SAME, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SAME_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 25 and flag 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking SAME 150 151
	static const char proc[] = { _SAME, 150, 151, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_SAME_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25 and flag 151 with value 25
	flags[150] = 1;
	flags[1] = 25;
	flags[151] = 25;

	//BDD when checking SAME @150 151
	static const char proc[] = { _SAME|IND, 150, 151, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTSAME <flagno1> <flagno2>
/*	Succeeds if Flag flagno 1 does not have the same value as Flag flagno 2 . */

void test_NOTSAME_success()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 25 and flag 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking NOTSAME 150 151
	static const char proc[] = { _NOTSAME, 150, 151, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTSAME_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25 and flag 151 with value 25
	flags[150] = 25;
	flags[151] = 25;

	//BDD when checking NOTSAME 150 151
	static const char proc[] = { _NOTSAME, 150, 151, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTSAME_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25 and flag 151 with value 50
	flags[150] = 1;
	flags[1] = 25;
	flags[151] = 50;

	//BDD when checking NOTSAME @150 151
	static const char proc[] = { _NOTSAME|IND, 150, 5, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests BIGGER <flagno1> <flagno2>
/*	Will be true if flagno 1 is larger than flagno 2 */

void test_BIGGER_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 50 and 151 with value 25
	flags[150] = 50;
	flags[151] = 25;

	//BDD when checking BIGGER 150 151
	static const char proc[] = { _BIGGER, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_BIGGER_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 25 and flag 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking BIGGER 150 151
	static const char proc[] = { _BIGGER, 150, 151, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_BIGGER_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 50 and flag 151 with value 25
	flags[150] = 1;
	flags[1] = 50;
	flags[151] = 25;

	//BDD when checking BIGGER @150 151
	static const char proc[] = { _BIGGER|IND, 150, 151, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SMALLER <flagno1> <flagno2>
/*	Will be true if flagno 1 is smaller than flagno 2 */

void test_SMALLER_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25 and 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking SMALLER 150 151
	static const char proc[] = { _SMALLER, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SMALLER_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 50 and flag 151 with value 25
	flags[150] = 50;
	flags[151] = 25;

	//BDD when checking SMALLER 150 151
	static const char proc[] = { _SMALLER, 150, 151, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_SMALLER_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 1 and flag 1 with value 25 and flag 151 with value 50
	flags[150] = 1;
	flags[1] = 25;
	flags[151] = 50;

	//BDD when checking SMALLER @150 151
	static const char proc[] = { _SMALLER|IND, 150, 151, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions to check logical sentence [5 condacts]
// =============================================================================

// =============================================================================
// Tests ADJECT1 <word>
/*	Succeeds if the first noun's adjective in the current LS is word. */

void test_ADJECT1_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adject1 1 from Logical Sentence
	flags[fAdject1] = 1;

	//BDD when checking ADJECT1 1
	static const char proc[] = { _ADJECT1, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT1_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the adject1 1 from Logical Sentence
	flags[fAdject1] = 1;

	//BDD when checking ADJECT1 2
	static const char proc[] = { _ADJECT1, 2, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT1_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the adject1 1 from Logical Sentence and flag 150 with value 1
	flags[fAdject1] = 1;
	flags[150] = 1;

	//BDD when checking ADJECT1 @150
	static const char proc[] = { _ADJECT1|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ADVERB <word>
/*	Succeeds if the adverb in the current LS is word. */

void test_ADVERB_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adverb 1 from Logical Sentence
	flags[fAdverb] = 1;

	//BDD when checking ADVERB 1
	static const char proc[] = { _ADVERB, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADVERB_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the adverb 1 from Logical Sentence
	flags[fAdverb] = 1;

	//BDD when checking ADVERB 2
	static const char proc[] = { _ADVERB, 2, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ADVERB_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the adverb 1 from Logical Sentence and flag 150 with value 1
	flags[fAdverb] = 1;
	flags[150] = 1;

	//BDD when checking ADVERB @150
	static const char proc[] = { _ADVERB|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PREP <word>
/*	Succeeds if the preposition in the current LS is word. */

void test_PREP_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the prep 1 from Logical Sentence
	flags[fPrep] = 1;

	//BDD when checking PREP 1
	static const char proc[] = { _PREP, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PREP_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the prep 1 from Logical Sentence
	flags[fPrep] = 1;

	//BDD when checking PREP 2
	static const char proc[] = { _PREP, 2, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PREP_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the prep 1 from Logical Sentence and flag 150 with value 1
	flags[fPrep] = 1;
	flags[150] = 1;

	//BDD when checking PREP @150
	static const char proc[] = { _PREP|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOUN2 <word>
/*	Succeeds if the second noun in the current LS is word. */

void test_NOUN2_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the noun2 1 from Logical Sentence
	flags[fNoun2] = 1;

	//BDD when checking NOUN2 1
	static const char proc[] = { _NOUN2, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOUN2_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the noun2 1 from Logical Sentence
	flags[fNoun2] = 1;

	//BDD when checking NOUN2 2
	static const char proc[] = { _NOUN2, 2, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOUN2_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the noun2 1 from Logical Sentence and flag 150 with value 1
	flags[fNoun2] = 1;
	flags[150] = 1;

	//BDD when checking NOUN2 @150
	static const char proc[] = { _NOUN2|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ADJECT2 <word>
/*	Succeeds if the second noun's adjective in the current LS is word. */

void test_ADJECT2_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adject2 1 from Logical Sentence
	flags[fAdject2] = 1;

	//BDD when checking ADJECT2 1
	static const char proc[] = { _ADJECT2, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT2_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the adject2 1 from Logical Sentence
	flags[fAdject2] = 1;

	//BDD when checking ADJECT2 2
	static const char proc[] = { _ADJECT2, 2, 255 };
	do_action(proc);
	
	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT2_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given the adject2 1 from Logical Sentence and flag 150 with value 1
	flags[fAdject2] = 1;
	flags[150] = 1;

	//BDD when checking ADJECT2 @150
	static const char proc[] = { _ADJECT2|IND, 150, 255 };
	do_action(proc);
	
	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for random [1 condacts]
// =============================================================================

// =============================================================================
// Tests CHANCE <percent>
/*	Succeeds if percent is less than or equal to a random number in the range 
	1-100 (inclusive). Thus a CHANCE 50 condition would allow PAW to look at the 
	next CondAct only if the random number generated was between 1 and 50, a 50% 
	chance of success. */

void test_CHANCE_0_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given none

	//BDD when checking CHANCE 0
	static const char proc[] = { _CHANCE, 0, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_CHANCE_255_success()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given none

	//BDD when checking CHANCE 255
	static const char proc[] = { _CHANCE, 255, 255 };
	do_action(proc);
	
	//BDD then succes
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CHANCE_indirection()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given flag 150 with value 255
	flags[150] = 255;

	//BDD when checking CHANCE @150
	static const char proc[] = { _CHANCE|IND, 150, 255 };
	do_action(proc);
	
	//BDD then succes
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for sub-process success/fail [2 condacts]
// =============================================================================

// =============================================================================
// Tests ISDONE
/*	Succeeds if the last table ended by exiting after executing at least one 
	Action. This is useful to test for a single succeed/fail boolean value from 
	a Sub-Process. A DONE action will cause the 'done' condition, as will any 
	condact causing exit, or falling off the end of the table - assuming at 
	least one CondAct (other than NOTDONE) was done.
	See also ISNDONE and NOTDONE actions. */

void test_ISDONE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given last table executed at least one action
	isDone = 1;

	//BDD when checking ISDONE
	static const char proc[] = { _ISDONE, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	//SUCCEED();
	TODO("Mock DONE");
}

void test_ISDONE_fails()
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given last table not executed at least one action
	isDone = 0;

	//BDD when checking ISDONE
	static const char proc[] = { _ISDONE, 255 };
	do_action(proc);
	
	//BDD then succes
	ASSERT(!checkEntry, ERROR);
	//SUCCEED();
	TODO("Mock NOTDONE");
}

// =============================================================================
// Tests ISNDONE
/*	Succeeds if the last table ended without doing anything or with a NOTDONE 
	action. */

void test_ISNDONE_success()		//TODO improve this test
{
	const char *_func = __func__;
	beforeEach();

	//BDD given last table not executed at least one action
	isDone = 0;

	//BDD when checking ISNDONE
	static const char proc[] = { _ISNDONE, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISNDONE_fails()		//TODO improve this test
{
	const char *_func = __func__;
	beforeEach();
	
	//BDD given last table executed at least one action
	isDone = 1;

	//BDD when checking ISNDONE
	static const char proc[] = { _ISNDONE, 255 };
	do_action(proc);
	
	//BDD then succes
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for object attributes [2 condacts]
// =============================================================================

// =============================================================================
// Tests HASAT <value>
/*	Checks the attribute specified by value. 0-15 are the object attributes for 
	the current object. There are also several attribute numbers specified as 
	symbols in SYMBOLS.SCE which check certain parts of the DAAD system flags */

void test_HASAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 10 is set
	flags[fCOAtt] = 0b00000100;

	//BDD when checking HASAT 10
	static const char proc[] = { _HASAT, 10, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_HASAT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 5 is not set
	flags[fCOAtt + 1] = 0b11011111;

	//BDD when checking HASAT 5
	static const char proc[] = { _HASAT, 5, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_HASAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 10 is set and flag 150 with value 10
	flags[fCOAtt] = 0b00000100;
	flags[150] = 10;

	//BDD when checking HASAT @150
	static const char proc[] = { _HASAT|IND, 150, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests HASNAT <value>
/*	Inverse of HASAT */

void test_HASNAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 5 is not set
	flags[fCOAtt + 1] = 0b11011111;

	//BDD when checking HASNAT 5
	static const char proc[] = { _HASNAT, 5, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_HASNAT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 10 is set
	flags[fCOAtt] = 0b00000100;

	//BDD when checking HASNAT 10
	static const char proc[] = { _HASNAT, 10, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_HASNAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 10 is set and flag 150 with value 10
	flags[fCOAtt] = 0b11111011;
	flags[150] = 10;

	//BDD when checking HASNAT @150
	static const char proc[] = { _HASNAT|IND, 150, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for player interaction [2 condacts]
// =============================================================================

// =============================================================================
// Tests INKEY
/*	Is a condition which will be satisfied if the player is pressing a key. 
	In 16Bit machines Flags Key1 and Key2 (60 & 61) will be a standard IBM ASCII 
	code pair.
	On 8 bit only Key1 will be valid, and the code will be machine specific. */

void test_INKEY_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given 'A' key pressed
	fake_keyPressed = 'A';

	//BDD when checking INKEY
	static const char proc[] = { _INKEY, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fKey1], 'A', ERROR);
	SUCCEED();
}

// =============================================================================
// Tests QUIT
/*	SM12 ("Are you sure?") is printed and called. Will succeed if the player replies
	starts with the first letter of SM30 ("Y") to then the remainder of the entry is 
	discarded is carried out. */

void test_QUIT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given 'Y' key pressed
	fake_keyPressed = 'Y';

	//BDD when checking QUIT
	// static const char proc[] = { _QUIT, 255 };
	// do_action(proc);

	//BDD then success
	// ASSERT(!checkEntry, ERROR);
	//SUCCEED();
	TODO("Mock user input");
}

// =============================================================================
// Actions for object position manipulation [21 condacts]
// =============================================================================

// =============================================================================
// Tests GET <objno>
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

void test_GET_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	objects[1].location = LOC_CARRIED;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at another location than the player
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 26, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_maxWeight()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 with weight (50), more than player can carry (25)
	flags[fPlayer] = 1;
	flags[fStrength] = 25;
	objects[1].location = 1;
	objects[1].attribs.mask.weight = 50;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 43, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_maxObjs()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at same location than player, but exceeds max number of objected carried
	flags[fPlayer] = 1;
	flags[fNOCarr] = 10;
	flags[fMaxCarr] = 10;
	objects[1].location = 1;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 27, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at same location than player, and could be carried
	flags[fPlayer] = 1;
	flags[fMaxCarr] = 255;
	objects[1].location = 1;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(fake_lastSysMesPrinted, 36, ERROR_SYSMES);
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DROP <objno>
/*	If Object objno. is worn then SM24 ("I can't. I'm wearing the _.") is 
	printed and actions NEWTEXT & DONE are performed.

	If Object objno. is at the current location (but neither worn nor carried), 
	SM49 ("I don't have the _.") is printed and actions NEWTEXT & DONE are 
	performed.

	If Object objno. is not at the current location then SM28 ("I don't have one 
	of those.") is printed and actions NEWTEXT & DONE are performed.

	Otherwise the position of Object objno. is changed to the current location, 
	Flag 1 is decremented and SM39 ("I've dropped the _.") is printed. */

void test_DROP_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	flags[fPlayer] = 1;
	flags[fMaxCarr] = 255;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking DROP 1
	static const char proc[] = { _DROP, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(fake_lastSysMesPrinted, 39, ERROR_SYSMES);
	ASSERT_EQUAL(objects[1].location, flags[fPlayer], "Droped object not here");
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_DROP_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking DROP 1
	static const char proc[] = { _DROP, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 24, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_DROP_isHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 placed at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking DROP 1
	static const char proc[] = { _DROP, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_DROP_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 placed at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking DROP 1
	static const char proc[] = { _DROP, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 28, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}


// =============================================================================
// Tests WEAR <objno>
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

void test_WEAR_isHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 placed at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 1;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 29, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_notCarried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a not carried object 1 and player at location 1
	flags[fPlayer] = 1;
	objects[1].location = LOC_NOTCREATED;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 28, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_notWareable()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 but is not wareable
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.isWareable = 0;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 40, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a wareable and carried object 1
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(fake_lastSysMesPrinted, 37, ERROR_SYSMES);
	ASSERT_EQUAL(objects[1].location, LOC_WORN, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests REMOVE <objno>
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

void test_REMOVE_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	objects[1].location = LOC_CARRIED;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 50, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_isHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a object 1 at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 50, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a object 1 at different location than player
	flags[fPlayer] = 1;
	objects[1].location = LOC_NOTCREATED - 1;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 23, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_notWareable()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried and not wareable object 1
	flags[fPlayer] = 1;
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 0;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 41, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_maxObjs()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1 but max carried objects reached
	flags[fNOCarr] = 10;
	flags[fMaxCarr] = 10;
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 42, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 10;
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(fake_lastSysMesPrinted, 38, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}


// =============================================================================
// Tests CREATE <objno>
/*	The position of Object objno. is changed to the current location and Flag 1
	is decremented if the object was carried. */

void test_CREATE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a uncreated object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CREATE 1
	static const char proc[] = { _CREATE, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, flags[fPlayer], ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CREATE_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking CREATE 1
	static const char proc[] = { _CREATE, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, flags[fPlayer], ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CREATE_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a uncreated object 1 and the player at location 2 and flag 150 with value 1
	flags[150] = 1;
	flags[fPlayer] = 2;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CREATE @150
	static const char proc[] = { _CREATE|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, flags[fPlayer], ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DESTROY <objno>
/*	The position of Object objno. is changed to not-created and Flag 1 is 
	decremented if the object was carried. */

void test_DESTROY_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = 1;

	//BDD when checking DESTROY 1
	static const char proc[] = { _DESTROY, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_NOTCREATED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_DESTROY_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking DESTROY 1
	static const char proc[] = { _DESTROY, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_NOTCREATED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_DESTROY_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 and the player at location 2 and flag 150 with value 1
	flags[150] = 1;
	flags[fPlayer] = 2;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CREATE @150
	static const char proc[] = { _DESTROY|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_NOTCREATED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SWAP <objno1> <objno2>
/*	The positions of the two objects are exchanged. Flag 1 is not adjusted. The 
	currently referenced object is set to be Object objno 2. */

void test_SWAP_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc1, and object 2 carried
	flags[fNOCarr] = 1;
	objects[1].location = 1;
	objects[2].location = LOC_CARRIED;

	//BDD when checking SWAP 1 2
	static const char proc[] = { _SWAP, 1, 2, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 2, "Current object is not object2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SWAP_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc1, object 2 carried, and flag 150 with value 1
	flags[150] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = 1;
	objects[2].location = 2;

	//BDD when checking SWAP @150 2
	static const char proc[] = { _SWAP|IND, 150, 2, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 2, "Current object is not object2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PLACE <objno> <locno+>
/*	The position of Object objno. is changed to Location locno. Flag 1 is 
	decremented if the object was carried. It is incremented if the object is 
	placed at location 254 (carried). */

void test_PLACE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 1
	flags[fNOCarr] = 0;
	objects[1].location = 1;

	//BDD when checking PLACE 1 2
	static const char proc[] = { _PLACE, 1, LOC_CARRIED, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object1");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PLACE_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1, and flag 150 with value 1
	flags[150] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking PLACE @150 1
	static const char proc[] = { _PLACE|IND, 150, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object1");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PUTO <locno+>
/*	The position of the currently referenced object (i.e. that object whose 
	number is given in flag 51), is changed to be Location locno. Flag 54 
	remains its old location. Flag 1 is decremented if the object was carried. 
	It is incremented if the object is placed at location 254 (carried). */

void test_PUTO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 referenced
	flags[fNOCarr] = 0;
	flags[fCONum] = 1;
	flags[fCOLoc] = 1;
	objects[1].location = 1;

	//BDD when checking PUTO LOC_CARRIED
	static const char proc[] = { _PUTO, LOC_CARRIED, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object1");
	ASSERT_EQUAL(flags[fCOLoc], 1, "Flag fCOLoc has changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PUTO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1, and flag 150 with value 2
	flags[150] = 2;
	flags[fNOCarr] = 1;
	flags[fCOLoc] = LOC_CARRIED;
	objects[1].location = LOC_CARRIED;

	//BDD when checking PUTO @150
	static const char proc[] = { _PUTO|IND, 150, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object1");
	ASSERT_EQUAL(flags[fCOLoc], LOC_CARRIED, "Flag fCOLoc has changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PUTIN <objno> <locno>
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

void test_PUTIN_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking PUTIN 1 2
	static const char proc[] = { _PUTIN, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 24, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PUTIN_here()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 here
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking PUTIN 1 2
	static const char proc[] = { _PUTIN, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PUTIN_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 here
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking PUTIN 1 2
	static const char proc[] = { _PUTIN, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 28, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PUTIN_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking PUTIN 1 2
	static const char proc[] = { _PUTIN, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT_EQUAL(fake_lastSysMesPrinted, 44, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PUTIN_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 1, and a carried object 1
	flags[75] = 1;
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking PUTIN @75 2
	static const char proc[] = { _PUTIN|IND, 75, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT_EQUAL(fake_lastSysMesPrinted, 44, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests TAKEOUT <objno> <locno>
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

void test_TAKEOUT_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	objects[1].location = LOC_CARRIED;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_here()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 that is here
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 45, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 that is here, and not at loc 3
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 52, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_maxWeight()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 3, and total weight of carried/worn objects + object 1 exceeds the maximum
	flags[fPlayer] = 1;
	flags[fStrength] = 1;
	objects[1].location = 3;
	objects[1].attribs.mask.weight = 7;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 43, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_maxObjs()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 3, and total num of carried objects + object 1 exceeds the maximum
	flags[fPlayer] = 1;
	flags[fNOCarr] = 2;
	flags[fMaxCarr] = 2;
	objects[1].location = 3;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 27, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 3, and total num of carried objects + object 1 no exceeds the maximum
	flags[fPlayer] = 1;
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 1;
	objects[1].location = 3;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(fake_lastSysMesPrinted, 36, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 1, an object 1 at loc 3, and total num of carried objects + object 1 no exceeds the maximum
	flags[75] = 1;
	flags[fPlayer] = 2;
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 1;
	objects[1].location = 3;

	//BDD when checking TAKEOUT @75 3
	static const char proc[] = { _TAKEOUT|IND, 75, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(fake_lastSysMesPrinted, 36, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DROPALL 
/*	All objects which are carried or worn are created at the current location (i.e. 
	all objects are dropped) and Flag 1 is set to 0. This is included for 
	compatibility with older writing systems.
	Note that a DOALL 254 will carry out a true DROP ALL, taking care of any special 
	actions included. */

void test_DROPALL_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given 3 objects carried or worn
	flags[fPlayer] = 1;
	flags[fNOCarr] = 3;
	objects[1].location = LOC_WORN;
	objects[2].location = LOC_CARRIED;
	objects[3].location = LOC_CARRIED;

	//BDD when checking DROPALL
	static const char proc[] = { _DROPALL, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[3].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests AUTOG
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; here, carried, 
	worn. i.e. The player is more likely to be trying to GET an object that is 
	at the current location than one that is carried or worn. If an object is 
	found its number is passed to the GET action. Otherwise if there is an 
	object in existence anywhere in the game or if Noun1 was not in the 
	vocabulary then SM26 ("There isn't one of those here.") is printed. Else 
	SM8 ("I can't do that.") is printed (i.e. It is not a valid object but does 
	exist in the game). Either way actions NEWTEXT & DONE are performed */

void test_AUTOG_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking AUTOG 1
	static const char proc[] = { _AUTOG, 1, 255 };
	do_action(proc);

	//BDD then fails
//	ASSERT_EQUAL(objects[3].location, 1, ERROR_OBJLOC);
//	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
//	ASSERT(checkEntry, ERROR);
//	SUCCEED();
	TODO("Must mock the noun/adjective table");
}

void test_AUTOG_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_WORN;

	//BDD when checking AUTOG 1
	static const char proc[] = { _AUTOG, 1, 255 };
	do_action(proc);

	//BDD then fails
//	ASSERT_EQUAL(objects[3].location, 1, ERROR_OBJLOC);
//	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
//	ASSERT(checkEntry, ERROR);
//	SUCCEED();
	TODO("Must mock the noun/adjective table");
}

void test_AUTOG_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = 1;

	//BDD when checking AUTOG 1
	static const char proc[] = { _AUTOG, 1, 255 };
	do_action(proc);

	//BDD then fails
//	ASSERT_EQUAL(objects[3].location, 1, ERROR_OBJLOC);
//	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
//	ASSERT(checkEntry, ERROR);
//	SUCCEED();
	TODO("Must mock the noun/adjective table");
}

// =============================================================================
// Tests AUTOD
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; carried, worn, 
	here. i.e. The player is more likely to be trying to DROP a carried object 
	than one that is worn or here. If an object is found its number is passed 
	to the DROP action. Otherwise if there is an object in existence anywhere 
	in the game or if Noun1 was not in the vocabulary then SM28 ("I don't have
	one of those.") is printed. Else SM8 ("I can't do that.") is printed (i.e. 
	It is not a valid object but does exist in the game). Either way actions 
	NEWTEXT & DONE are performed */

void test_AUTOD_success()
{
	TODO("Must mock the noun/adjective table");
}

// =============================================================================
// Tests AUTOW
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; carried, worn, 
	here. i.e. The player is more likely to be trying to WEAR a carried object 
	than one that is worn or here. If an object is found its number is passed 
	to the WEAR action. Otherwise if there is an object in existence anywhere 
	in the game or if Noun1 was not in the vocabulary then SM28 ("I don't have
	one of those.") is printed. Else SM8 ("I can't do that.") is printed (i.e. 
	It is not a valid object but does exist in the game). Either way actions 
	NEWTEXT & DONE are performed */

void test_AUTOW_success()
{
	TODO("Must mock the noun/adjective table");
}

// =============================================================================
// Tests AUTOR
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; worn, carried, 
	here. i.e. The player is more likely to be trying to REMOVE a worn object 
	than one that is carried or here. If an object is found its number is passed 
	to the REMOVE action. Otherwise if there is an object in existence anywhere 
	in the game or if Noun1 was not in the vocabulary then SM23 ("I'm not 
	wearing one of those.") is printed. Else SM8 ("I can't do that.") is printed 
	(i.e. It is not a valid object but does exist in the game). Either way 
	actions NEWTEXT & DONE are performed */

void test_AUTOR_success()
{
	TODO("Must mock the noun/adjective table");
}

// =============================================================================
// Tests AUTOP
/*	A search for the object number represented by Noun(Adjective)1 is made in the 
	object definition section in order of location priority; carried, worn, here. 
	i.e. The player is more likely to be trying to PUT a carried object inside 
	another than one that is worn or here. If an object is found its number is 
	passed to the PUTIN action. Otherwise if there is an object in existence
	anywhere in the game or if Noun1 was not in the vocabulary then SM28 ("I don't 
	have one of those.") is printed. Else SM8 ("I can't do that.") is printed 
	(i.e. It is not a valid object but does exist in the game). Either way actions 
	NEWTEXT & DONE are performed */

void test_AUTOP_success()
{
	TODO("Must mock the noun/adjective table");
}

// =============================================================================
// Tests AUTOT
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

void test_AUTOT_success()
{
	TODO("Must mock the noun/adjective table");
}

// =============================================================================
// Tests COPYOO <objno1> <objno2>
/*	The position of Object objno2 is set to be the same as the position of 
	Object Objno1. The currently referenced object is set to be Object objno2 */

void test_COPYOO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 2, and an object 2 at loc 4
	objects[1].location = 2;
	objects[2].location = 4;

	//BDD when checking COPYOO 1 2
	static const char proc[] = { _COPYOO, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fCONum], 2, "Current object not object2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYOO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given floag 75 with value 1, an object 1 at loc 2, and an object 2 at loc 4
	flags[75] = 1;
	objects[1].location = 2;
	objects[2].location = 4;

	//BDD when checking COPYOO @75 2
	static const char proc[] = { _COPYOO|IND, 75, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fCONum], 2, "Current object not object2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests RESET
/*	This Action bears no resemblance to the one with the same name in PAW. It has 
	the pure function of placing all objects at the position given in the Object 
	start table. It also sets the relevant flags dealing with no of objects 
	carried etc. */

void test_RESET_success()
{
	TODO("Must mock initObject()");
}

// =============================================================================
// Actions for object in flags manipulation [5 condacts]
// =============================================================================

// =============================================================================
// Tests COPYOF <objno> <flagno>
/*	The position of Object objno. is copied into Flag flagno. This could be used 
	to examine the location of an object in a comparison with another flag value. */

void test_COPYOF_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1, and an empty flag 200
	flags[200] = 0;
	objects[1].location = LOC_WORN;

	//BDD when checking COPYOF 1 200
	static const char proc[] = { _COPYOF, 1, 200, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[200], LOC_WORN, "Flag 200 don't have the object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYOF_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1, a flag 75 with value 1, and an empty flag 200
	flags[75] = 1;
	flags[200] = 0;
	objects[1].location = LOC_WORN;

	//BDD when checking COPYOF @75 200
	static const char proc[] = { _COPYOF|IND, 75, 200, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[200], LOC_WORN, "Flag 200 don't have the object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests COPYFO <flagno> <objno>
/*	The position of Object objno. is set to be the contents of Flag flagno. An 
	attempt to copy from a flag containing 255 will result in a run time error. 
	Setting an object to an invalid location will still be accepted as it 
	presents no danger to the operation of PAW. */

void test_COPYFO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1, and an empty flag 200
	flags[200] = LOC_WORN;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking COPYFO 200 1
	static const char proc[] = { _COPYFO, 200, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_WORN, "Flag 200 don't have the object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYFO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1, a flag 75 with value 1, and an empty flag 200
	flags[75] = 200;
	flags[200] = LOC_WORN;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking COPYFO @75 1
	static const char proc[] = { _COPYFO|IND, 75, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_WORN, "Flag 200 don't have the object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WHATO
/*	A search for the object number represented by Noun(Adjective)1 is made in 
	the object definition section in order of location priority; carried, worn, 
	here. This is because it is assumed any use of WHATO will be related to 
	carried objects rather than any that are worn or here. If an object is found 
	its number is placedin flag 51, along with the standard current object 
	parameters in flags 54-57. This allows you to create other auto actions (the
	tutorial gives an example of this for dropping objects in the tree). */

void test_WHATO_success()
{
	TODO("Must mock the noun/adjective table");
}

// =============================================================================
// Tests SETCO <objno>
/*	Sets the currently referenced object to objno. */

void test_SETCO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 2
	objects[1].location = 2;

	//BDD when checking SETCO 1
	static const char proc[] = { _SETCO, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object 1");
	ASSERT_EQUAL(flags[fCOLoc], 2, "Current object location is not 2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SETCO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 1, and object 1 at loc 2
	flags[75] = 1;
	objects[1].location = 2;

	//BDD when checking SETCO @75
	static const char proc[] = { _SETCO|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object 1");
	ASSERT_EQUAL(flags[fCOLoc], 2, "Current object location is not 2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WEIGH <objno> <flagno>
/*	The true weight of Object objno. is calculated (i.e. if it is a container, 
	any objects inside have their weight added - don't forget that nested 
	containers stop adding their contents after ten levels) and the value is 
	placed in Flag flagno. This will have a maximum value of 255 which will not 
	be exceeded. If Object objno. is a container of zero weight, Flag flagno 
	will be cleared as objects in zero weight containers, also weigh zero! */

void test_WEIGH_success()
{
	TODO("Must mock better getObjectWeight()");
}

void test_WEIGH_indirection()
{
	TODO("Must mock better getObjectWeight()");
}

// =============================================================================
// Actions to manipulate flags [11 condacts]
// =============================================================================

// =============================================================================
// Tests SET <flagno>
/*	Flag flagno. is set to 255. */

void test_SET_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 1
	flags[100] = 1;

	//BDD when checking SET 100
	static const char proc[] = { _SET, 100, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 255, "Flag 100 is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SET_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, and flag 100 with value 1
	flags[75] = 100;
	flags[100] = 1;

	//BDD when checking SET @75
	static const char proc[] = { _SET|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[75], 100, "Flag 75 have changed");
	ASSERT_EQUAL(flags[100], 255, "Flag 100 is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CLEAR <flagno>
/*	Flag flagno. is cleared to 0. */

void test_CLEAR_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 100
	flags[100] = 100;

	//BDD when checking CLEAR 100
	static const char proc[] = { _CLEAR, 100, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 0, "Flag 100 is not 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CLEAR_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 150
	flags[75] = 100;
	flags[100] = 150;

	//BDD when checking CLEAR @75
	static const char proc[] = { _CLEAR|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[75], 100, "Flag 75 have changed");
	ASSERT_EQUAL(flags[100], 0, "Flag 100 is not 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests LET <flagno> <value>
/*	Flag flagno. is set to value. */

void test_LET_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 1
	flags[100] = 1;

	//BDD when checking LET 100 50
	static const char proc[] = { _LET, 100, 50, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 50, "Flag 100 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_LET_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with valur 100, flag 100 with value 1
	flags[75] = 100;
	flags[100] = 1;

	//BDD when checking LET @75 80
	static const char proc[] = { _LET|IND, 75, 80, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 80, "Flag 100 is not 80");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PLUS <flagno> <value>
/*	Flag flagno. is increased by value. If the result exceeds 255 the flag is
	set to 255. */

void test_PLUS_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10
	flags[100] = 10;

	//BDD when checking PLUS 100 50
	static const char proc[] = { _PLUS, 100, 50, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 60, "Flag 100 is not 60");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PLUS_overflow()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 210
	flags[100] = 210;

	//BDD when checking PLUS 100 50
	static const char proc[] = { _PLUS, 100, 50, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 255, "Flag 100 is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PLUS_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10
	flags[75] = 100;
	flags[100] = 10;

	//BDD when checking PLUS @75 80
	static const char proc[] = { _PLUS|IND, 75, 80, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 90, "Flag 100 is not 90");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests MINUS <flagno> <value>
/*	Flag flagno. is decreased by value. If the result is negative the flag is 
	set to 0. */

void test_MINUS_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 60
	flags[100] = 60;

	//BDD when checking MINUS 100 10
	static const char proc[] = { _MINUS, 100, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 50, "Flag 100 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_MINUS_overflow()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 60
	flags[100] = 60;

	//BDD when checking MINUS 100 150
	static const char proc[] = { _MINUS, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 0, "Flag 100 is not 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_MINUS_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 60
	flags[75] = 100;
	flags[100] = 60;

	//BDD when checking MINUS @75 10
	static const char proc[] = { _MINUS|IND, 75, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 50, "Flag 100 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ADD <flagno1> <flagno2>
/*	Flag flagno 2 has the contents of Flag flagno 1 added to it. If the result 
	exceeds 255 the flag is set to 255. */

void test_ADD_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 50
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking ADD 100 150
	static const char proc[] = { _ADD, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 60, "Flag 150 is not 60");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADD_overflow()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 250
	flags[100] = 10;
	flags[150] = 250;

	//BDD when checking ADD 100 150
	static const char proc[] = { _ADD, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 255, "Flag 150 is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADD_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10, and flag 150 with value 50
	flags[75] = 100;
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking ADD @75 150
	static const char proc[] = { _ADD|IND, 75, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 60, "Flag 150 is not 60");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SUB <flagno1> <flagno2>
/*	Flag flagno 2 has the contents of Flag flagno 1 subtracted from it. If the
	result is negative the flag is set to 0. */

void test_SUB_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 50
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking SUB 100 150
	static const char proc[] = { _SUB, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 40, "Flag 150 is not 40");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SUB_overflow()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 50, and flag 150 with value 10
	flags[100] = 50;
	flags[150] = 10;

	//BDD when checking SUB 100 150
	static const char proc[] = { _SUB, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 0, "Flag 150 is not 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SUB_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10, and flag 150 with value 50
	flags[75] = 100;
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking SUB @75 150
	static const char proc[] = { _SUB|IND, 75, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 40, "Flag 150 is not 40");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests COPYFF <flagno1> <flagno2>
/*	The contents of Flag flagno 1 is copied to Flag flagno 2. */

void test_COPYFF_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 50
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking COPYFF 100 150
	static const char proc[] = { _COPYFF, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 10, "Flag 150 is not 10");
	ASSERT_EQUAL(flags[100], 10, "Flag 150 is not 10");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYFF_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10, and flag 150 with value 50
	flags[75] = 100;
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking COPYFF @75 150
	static const char proc[] = { _COPYFF|IND, 75, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 10, "Flag 150 is not 10");
	ASSERT_EQUAL(flags[100], 10, "Flag 150 is not 10");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests COPYBF <flagno1> <flagno2>
/*	Same as COPYFF but the source and destination are reversed, so that 
	indirection can be used. */

void test_COPYBF_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 50
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking COPYBF 100 150
	static const char proc[] = { _COPYBF, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 50, "Flag 150 is not 50");
	ASSERT_EQUAL(flags[100], 50, "Flag 150 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYBF_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10, and flag 150 with value 50
	flags[75] = 100;
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking COPYBF @75 150
	static const char proc[] = { _COPYBF|IND, 75, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 50, "Flag 150 is not 50");
	ASSERT_EQUAL(flags[100], 50, "Flag 150 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests RANDOM <flagno>
/*	Flag flagno. is set to a number from the Pseudo-random sequence from 1 
	to 100. */

void test_RANDOM_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 150 with value 255
	flags[150] = 255;

	//BDD when checking RANDOM 150
	static const char proc[] = { _RANDOM, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(flags[150] >= 1, "Flag 150 is 0");
	ASSERT(flags[150] <= 100, "Flag 150 is greater than 100");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_RANDOM_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 150 with value 255, and flag 75 with valur 150
	flags[75] = 150;
	flags[150] = 255;

	//BDD when checking RANDOM @75
	static const char proc[] = { _RANDOM|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(flags[150] >= 1, "Flag 150 is 0");
	ASSERT(flags[150] <= 100, "Flag 150 is greater than 100");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests MOVE <flagno>
/*	This is a very powerful action designed to manipulate PSI's. It allows the
	current LS Verb to be used to scan the connections section for the location 
	given in Flag flagno. 
	If the Verb is found then Flag flagno is changed to be the location number 
	associated with it, and the next condact is considered.
	If the verb is not found, or the original location number was invalid, then 
	PAW considers the next entry in the table - if present. */

void test_MOVE_success()
{
	TODO("Must mock Verbs table and Connections table");
}

void test_MOVE_indirection()
{
	TODO("Must mock Verbs table and Connections table");
}

// =============================================================================
// Actions to manipulate player flags [3 condacts]
// =============================================================================

// =============================================================================
// Tests GOTO <locno>
/*	Changes the current location to locno. This effectively sets flag 38 to the value
	locno. */

void test_GOTO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when checking GOTO 1
	static const char proc[] = { _GOTO, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fPlayer], 1, "Player not moved");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_GOTO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, and flag 75 with value 1
	flags[75] = 1;
	flags[fPlayer] = 5;

	//BDD when checking GOTO @75
	static const char proc[] = { _GOTO|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fPlayer], 1, "Player not moved");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WEIGHT <flagno>
/*	Calculates the true weight of all objects carried and worn by the player 
	(i.e. any containers will have the weight of their contents added up to a 
	maximum of 255), this value is then placed in Flag flagno.
	This would be useful to ensure the player was not carrying too much weight 
	to cross a bridge without it collapsing etc. */

void test_WEIGHT_success()
{
	TODO("Must mock getObjectWeight()");
}

// =============================================================================
// Tests ABILITY <value1> <value2>
/*	This sets Flag 37, the maximum number of objects conveyable, to value 1 and 
	Flag 52, the maximum weight of objects the player may carry and wear at any 
	one time (or their strength), to be value 2 .
	No checks are made to ensure that the player is not already carrying more 
	than the maximum. GET and so on, which check the values, will still work 
	correctly and prevent the player carrying any more objects, even if you set 
	the value lower than that which is already carried! */

void test_ABILITY_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given fMaxCarr with value 5, and fStrength with value 50
	flags[fMaxCarr] = 5;
	flags[fStrength] = 50;

	//BDD when checking ABILITY 10 100
	static const char proc[] = { _ABILITY, 10, 100, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fMaxCarr], 10, "Flag fMaxCarr bad value");
	ASSERT_EQUAL(flags[fStrength], 100, "Flag fStrength bad value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ABILITY_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 10, fMaxCarr with value 5, and fStrength with value 50
	flags[75] = 10;
	flags[fMaxCarr] = 5;
	flags[fStrength] = 50;

	//BDD when checking ABILITY @75 100
	static const char proc[] = { _ABILITY|IND, 75, 100, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fMaxCarr], 10, "Flag fMaxCarr bad value");
	ASSERT_EQUAL(flags[fStrength], 100, "Flag fStrength bad value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Actions for screen mode/format flags [3 condacts]
// =============================================================================

// =============================================================================
// Tests MODE <option>
/*	Allows the current window to have its operation flags changed. In order to 
	calculate the number to use for the option just add the numbers shown next 
	to each item to achieve the required bitmask combination:
		1 - Use the upper character set. (A permanent ^G)
		2 - SM32 ("More...") will not appear when the window fills.
	e.g. MODE 3 stops the 'More...' prompt and causes all to be translated to 
	the 128-256 range. */

void test_MODE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window mode assigned to 0
	cw->mode = 0;

	//BDD when checking MODE 3
	static const char proc[] = { _MODE, 3, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->mode, 3, "New mode not is 3");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_MODE_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 3, current window mode assigned to 0
	cw->mode = 0;
	flags[75] = 3;

	//BDD when checking MODE @75
	static const char proc[] = { _MODE|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->mode, 3, "New mode not is 3");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests INPUT <stream> <option>
/*	The 'stream' parameter will set the bulk of input to come from the given 
	window/stream. A value of 0 for 'stream' will not use the graphics stream 
	as might be expected, but instead causes input to come from the current 
	stream when the input occurs.
	Bitmask options:
		1 - Clear window after input.
		2 - Reprint input line in current stream when complete.
		4 - Reprint current text of input after a timeout. */

void test_INPUT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given fInStream w/value 0, and fTIFlags w/value 0b11000111
	flags[fInStream] = 0;
	flags[fTIFlags] = 0xc7;

	//BDD when checking INPUT 5 7
	static const char proc[] = { _INPUT, 5, 7, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fInStream], 5, "Flag fInStream is not 5");
	ASSERT_EQUAL(flags[fTIFlags], 0xff, "Flag fTIFlags is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_INPUT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 w/value 5, fInStream w/value 0, and fTIFlags w/value 0b11000111
	flags[75] = 5;
	flags[fInStream] = 0;
	flags[fTIFlags] = 0xc7;

	//BDD when checking INPUT 5 7
	static const char proc[] = { _INPUT|IND, @75, 7, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fInStream], 5, "Flag fInStream is not 5");
	ASSERT_EQUAL(flags[fTIFlags], 0xff, "Flag fTIFlags is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests TIME <duration> <option>
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

void test_TIME_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given fTime w/value 0, and fTIFlags w/value 0b11111000
	flags[fTime] = 0;
	flags[fTIFlags] = 0xf8;

	//BDD when checking TIME 10 7
	static const char proc[] = { _TIME, 10, 7, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fTime], 10, "Flag fTime is not 10");
	ASSERT_EQUAL(flags[fTIFlags], 0xff, "Flag fTIFlags is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_TIME_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 w/value 10, fTime w/value 0, and fTIFlags w/value 0b11111000
	flags[75] = 10;
	flags[fTime] = 0;
	flags[fTIFlags] = 0xf8;

	//BDD when checking TIME @75 7
	static const char proc[] = { _TIME|IND, 75, 7, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fTime], 10, "Flag fTime is not 10");
	ASSERT_EQUAL(flags[fTIFlags], 0xff, "Flag fTIFlags is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Actions for screen control & output [20 condacts]
// =============================================================================

// =============================================================================
// Tests WINDOW <window>
/*	Selects window (0-7) as current print output stream. */

void test_WINDOW_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current windows is 0
	cw = &windows[0];

	//BDD when checking WINDOW 1
	static const char proc[] = { _WINDOW, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw, &windows[1], "Current window is not 1");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINDOW_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current windows is 0, and flag 75 w/value 1
	cw = &windows[0];
	flags[75] = 1;

	//BDD when checking WINDOW @75
	static const char proc[] = { _WINDOW|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw, &windows[1], "Current window is not 1");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WINAT <line> <col>
/*	Sets current window to start at given line and column. Height and width to fit 
	available screen. */

void test_WINAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window at (0,0)
	cw->winX = 0;
	cw->winY = 0;

	//BDD when checking WINAT 5 10
	static const char proc[] = { _WINAT, 5, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winX, 10, "Current window X is not 10");
	ASSERT_EQUAL(cw->winY, 5, "Current window Y is not 5");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINAT_oversize()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window at (10,10) w/size (85x25)
	cw->winX = 0;
	cw->winY = 0;
	cw->winW = 85;
	cw->winH = 25;

	//BDD when checking WINAT 10 10
	static const char proc[] = { _WINAT, 10, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winW, MAX_COLUMNS - cw->winX, "Current window W not match");
	ASSERT_EQUAL(cw->winH, MAX_LINES - cw->winY, "Current window H not match");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window at (0,0), and flag 75 w/value 5
	cw->winX = 0;
	cw->winY = 0;
	flags[75] = 5;

	//BDD when checking WINAT @75 10
	static const char proc[] = { _WINAT|IND, 75, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winX, 10, "Current window X is not 10");
	ASSERT_EQUAL(cw->winY, 5, "Current window Y is not 5");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WINSIZE <height> <width>
/*	Sets current window size to given height and width. Clipping needed to fit 
	available screen. */

void test_WINSIZE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window size is (20x15)
	cw->winW = 20;
	cw->winH = 15;

	//BDD when checking WINSIZE 15 25
	static const char proc[] = { _WINSIZE, 15, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winW, 25, "Current window W is not 25");
	ASSERT_EQUAL(cw->winH, 15, "Current window H is not 15");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINSIZE_oversize()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window size is (20x15) and at (10,10)
	cw->winX = 10;
	cw->winY = 10;
	cw->winW = 20;
	cw->winH = 15;

	//BDD when checking WINSIZE 127 127
	static const char proc[] = { _WINSIZE, 127, 127, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winW, MAX_COLUMNS - cw->winX, "Current window W not match");
	ASSERT_EQUAL(cw->winH, MAX_LINES - cw->winY, "Current window H not match");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINSIZE_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window size is (20x15), and flag 75 w/value 15
	cw->winW = 20;
	cw->winH = 15;
	flags[75] = 15;

	//BDD when checking WINSIZE @75 25
	static const char proc[] = { _WINSIZE|IND, 75, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winW, 25, "Current window W is not 25");
	ASSERT_EQUAL(cw->winH, 15, "Current window H is not 15");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CENTRE
/*	Will ensure the current window is centered for the current column width of the 
	screen. (Does not affect line position). */

void test_CENTRE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window at (1,1) w/size (20x15)
	cw->winX = 1;
	cw->winY = 1;
	cw->winW = 20;
	cw->winH = 15;

	//BDD when checking CENTRE
	static const char proc[] = { _CENTRE, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winX, (MAX_COLUMNS - cw->winW)/2, "Current window W not match");
	ASSERT_EQUAL(cw->winY, 1, "Current window Y has changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CLS
/*	Clears the current window. */

void test_CLS_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a used current window
	cw->cursorX = 1;
	cw->cursorY = 2;
	cw->lastPicLocation = 20;
	lastPicShow = true;
	printedLines = 5;

	//BDD when checking CLS
	static const char proc[] = { _CLS, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 0, "Current window cursorX not reset");
	ASSERT_EQUAL(cw->cursorY, 0, "Current window cursorY not reset");
	ASSERT_EQUAL(cw->lastPicLocation, NO_LASTPICTURE, "Current window lastPicLocation not reset");
	ASSERT_EQUAL(lastPicShow, false, "lastPicShow not reset");
	ASSERT_EQUAL(printedLines, 0, "printedLines not reset");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SAVEAT
/*	Save and Restore print position for current window. This allows you to 
	maintain the print position for example while printing elsewhere in the 
	window. You should consider using a seperate window for most tasks. This 
	may find use in the creation of a new input line or in animation 
	sequences... */

void test_SAVEAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given not saved cursor pos, and cursor at (1,2)
	savedPosX = 0;
	savedPosY = 0;
	cw->cursorX = 1;
	cw->cursorY = 2;

	//BDD when checking SAVEAT
	static const char proc[] = { _SAVEAT, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(savedPosX, 1, "Saved pos X not 1");
	ASSERT_EQUAL(savedPosY, 2, "Saved pos Y not 2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests BACKAT

void test_BACKAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a saved cursor pos (1,2), and cursor at (0,0)
	savedPosX = 1;
	savedPosY = 2;
	cw->cursorX = 0;
	cw->cursorY = 0;

	//BDD when checking BACKAT
	static const char proc[] = { _BACKAT, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 1, "Restored pos X not 1");
	ASSERT_EQUAL(cw->cursorY, 2, "Restored pos Y not 2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PAPER <colour>
/*	Set paper colour acording to the lookup table given in the graphics editors */

void test_PAPER_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests INK <colour>
/*	Set text colour acording to the lookup table given in the graphics editors */

void test_INK_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests BORDER <colour>
/*	Set border colour acording to the lookup table given in the graphics editors. */

void test_BORDER_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests PRINTAT <line> <col>
/*	Sets current print position to given point if in current window. If not then 
	print position becomes top left of window. */

void test_PRINTAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given cursor at (0,0)
	cw->cursorX = 0;
	cw->cursorY = 0;

	//BDD when checking PRINTAT 10 5
	static const char proc[] = { _PRINTAT, 10, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 5, "Cursor X not 5");
	ASSERT_EQUAL(cw->cursorY, 10, "Cursor Y not 10");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PRINTAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given cursor at (0,0), and flag 75 w/value 10
	cw->cursorX = 0;
	cw->cursorY = 0;
	flags[75] = 10;

	//BDD when checking PRINTAT @75 5
	static const char proc[] = { _PRINTAT|IND, 75, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 5, "Cursor X not 5");
	ASSERT_EQUAL(cw->cursorY, 10, "Cursor Y not 10");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests TAB <col>
/*	Sets current print position to given column on current line. */

void test_TAB_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given cursor at (2,3)
	cw->cursorX = 2;
	cw->cursorY = 3;

	//BDD when checking TAB 10
	static const char proc[] = { _TAB, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 10, "Cursor X not 10");
	ASSERT_EQUAL(cw->cursorY, 3, "Cursor Y not 3");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_TAB_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given cursor at (2,3), and flag 75 w/value 10
	cw->cursorX = 2;
	cw->cursorY = 3;
	flags[75] = 10;

	//BDD when checking TAB @75
	static const char proc[] = { _TAB|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 10, "Cursor X not 10");
	ASSERT_EQUAL(cw->cursorY, 3, "Cursor Y not 3");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SPACE
/*	Will simply print a space to the current output stream. Shorter than MES 
	Space! */

void test_SPACE_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests NEWLINE
/*	Prints a carriage return/line feed. */

void test_NEWLINE_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests MES <mesno>
/*	Prints Message mesno. */

void test_MES_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests MESSAGE <mesno>
/*	Prints Message mesno., then carries out a NEWLINE action. */

void test_MESSAGE_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests SYSMESS <sysno>
/*	Prints System Message sysno. */

void test_SYSMES_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests DESC <locno>
/*	Prints the text for location locno. without a NEWLINE. */

void test_DESC_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests PRINT <flagno>
/*	The decimal contents of Flag flagno. are displayed without leading or 
	trailing spaces. */

void test_PRINT_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Tests DPRINT <flagno>
/*	Will print the contents of flagno and flagno+1 as a two byte number. */

void test_DPRINT_success()
{
	TODO("UI not mocked");
}

// =============================================================================
// Actions for listing objects [2 condacts]
// =============================================================================

// =============================================================================
// Tests LISTOBJ
/*	If any objects are present then SM1 ("I can also see:") is printed, followed 
	by a list of all objects present at the current location.
	If there are no objects then nothing is printed. */

void test_LISTOBJ_success()
{
	TODO(TODO_GENERIC);
}

void test_LISTOBJ_none()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests LISTAT <locno+>
/*	If any objects are present then they are listed. Otherwise SM53 ("nothing.") 
	is printed - note that you will usually have to precede this action with a 
	message along the lines of "In the bag is:" etc. */

void test_LISTAT_success()
{
	TODO(TODO_GENERIC);
}

void test_LISTAT_none()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions for current game state save/load [4 condacts]
// =============================================================================

// =============================================================================
// Tests SAVE <opt>
/*	This action saves the current game position on disc or tape. SM60 ("Type in 
	name of file.") is printed and the input routine is called to get the filename 
	from the player. If the supplied filename is not acceptable SM59 ("File name 
	error.") is printed - this is not checked on 8 bit machines, the file name 
	is MADE acceptable! */

void test_SAVE_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests LOAD <opt>
/*	This action loads a game position from disc or tape. A filename is obtained 
	in the same way as for SAVE. A variety of errors may appear on each machine 
	if the file is not found or suffers a load error. Usually 'I/O Error'. The 
	next action is carried out only if the load is successful. Otherwise a system 
	clear, GOTO 0, RESTART is carried out. */

void test_LOAD_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests RAMSAVE
/*	In a similar way to SAVE this action saves all the information relevant to 
	the game in progress not onto disc but into a memory buffer. This buffer is 
	of course volatile and will be destroyed when the machine is turned off 
	which should be made clear to the player. The next action is always carried 
	out. */

void test_RAMSAVE_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests RAMSAVE <flagno>
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

void test_RAMLOAD_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions to pause game [2 condacts]
// =============================================================================

// =============================================================================
// Tests ANYKEY
/*	SM16 ("Press any key to continue") is printed and the keyboard is scanned until 
	a key is pressed or until the timeout duration has elapsed if enabled. */

void test_ANYKEY_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests PAUSE <value>
/*	Pauses for value/50 secs. However, if value is zero then the pause is for 
	256/50 secs. */

void test_PAUSE_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions to control the parse [3 condacts]
// =============================================================================

// =============================================================================
// Tests PARSE <n>
/*	The parameter 'n' controls which level of string indentation is to be 
	searched. At the moment only two are supported by the interpreters so only 
	the values 0 and 1 are valid.
		0 - Parse the main input line for the next LS.
		1 - Parse any string (phrase enclosed in quotes [""]) that was contained 
		    in the last LS extracted. */

void test_PARSE_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests NEWTEXT
/*	Forces the loss of any remaining phrases on the current input line. You 
	would use this to prevent the player continuing without a fresh input 
	should something go badly for his situation. e.g. the GET action carries 
	out a NEWTEXT if it fails to get the required object for any reason, to 
	prevent disaster with a sentence such as:
		GET SWORD AND KILL ORC WITH IT
	as attacking the ORC without the sword may be dangerous! */

void test_NEWTEXT_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests SYNONYM
/*	Substitutes the given verb and noun in the LS. Nullword (Usually '_') can be 
	used to suppress substitution for one or the other - or both I suppose! e.g.
	        MATCH    ON         SYNONYM LIGHT MATCH
	        STRIKE   MATCH      SYNONYM LIGHT _
	        LIGHT    MATCH      ....                 ; Actions...
	will switch the LS into a standard format for several different entries. 
	Allowing only one to deal with the actual actions. */

void test_SYNONYM_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions for flow control [7 condacts]
// =============================================================================

// =============================================================================
// Tests PROCESS
/*	This powerful action transfers the attention of DAAD to the specified Process 
	table number. Note that it is a true subroutine call and any exit from the 
	new table (e.g. DONE, OK etc) will return control to the condact which follows 
	the calling PROCESS action. A sub-process can call (nest) further process' to 
	a depth of 10 at which point a run time error will be generated. */

void test_PROCESS_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests REDO
/*	Will restart the currently executing table, allowing...
	TODO:incomplete descripcion in documentation */

void test_REDO_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests DOALL
/*	Another powerful action which allows the implementation 'ALL' type command.

	1 - An attempt is made to find an object at Location locno. 
	    If this is unsuccessful the DOALL is cancelled and action DONE is performed.
	2 - The object number is converted into the LS Noun1 (and Adjective1 if present)
	    by reference to the object definition section. If Noun(Adjective)1 matches
	    Noun(Adjective)2 then a return is made to step 1. This implements the "Verb
	    ALL EXCEPT object" facility of the parser.
	3 - The next condact and/or entry in the table is then considered. This 
	    effectively converts a phrase of "Verb All" into "Verb object" which is
	    then processed by the table as if the player had typed it in.
	4 - When an attempt is made to exit the current table, if the DOALL is still 
	    active (i.e. has not been cancelled by an action) then the attention of 
	    DAAD is returned to the DOALL as from step 1; with the object search 
	    continuing from the next highest object number to that just considered.

	The main ramification of the search method through the object definition 
	section is; objects which have the Same Noun(Adjective) description (where the 
	game works out which object is referred to by its presence) must be checked for 
	in ascending order of object number, or one of them may be missed.
	Use the of DOALL to implement things like OPEN ALL must account for fact that 
	doors are often flags only and would have to bemade into objects if they were to 
	be included in a DOALL. */

void test_DOALL_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests SKIP
/*	Skip a distance of -128 to 128, or to the specified label. Will move the 
	current entry in a table back or fore. 0 means next entry (so is meaningless).
	-1 means restart current entry (Dangerous). */

void test_SKIP_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests RESTART
/*	Will cancel any DOALL loop, any sub-process calls and make a jump
	to execute process 0 again from the start.*/

void test_RESTART_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests END
/*	SM13 ("Would you like to play again?") is printed and the input routine called.
	Any DOALL loop and sub-process calls are cancelled. If the reply does not start 
	with the first character of SM31 a jump is made to Initialise.
	Otherwise the player is returned to the operating system - by doing the command 
	EXIT 0.*/

void test_END_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests EXIT <value>
/*	If value is 0 then will return directly to the operating system. 
	reset ensure you use your PART number as the non zero value! */

void test_EXIT_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions for exit tables [3 condacts]
// =============================================================================

// =============================================================================
// Tests DONE
/*	This action jumps to the end of the process table and flags to DAAD that an 
	action has been carried out. i.e. no more condacts or entries are considered. 
	A return will thus be made to the previous calling process table, or to the 
	start point of any active DOALL loop. */

void test_DONE_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests NOTDONE
/*	This action jumps to the end of the process table and flags PAW that #no# 
	action has been carried out. i.e. no more condacts or entries are considered. 
	A return will thus be made to the previous calling process table or to the 
	start point of any active DOALL loop. This will cause PAW to print one of the
	"I can't" messages if needed. i.e. if no other action is carried out and no 
	entry is present in the connections section for the current Verb. */

void test_NOTDONE_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests OK
/*	SM15 ("OK") is printed and action DONE is performed. */

void test_OK_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions to call external routines [4 condacts]
// =============================================================================

// =============================================================================
// Tests EXTERN <value> <routine>
/*	Calls external routine with parameter value. The address is set by linking 
	the #extern pre-compiler command */

void test_EXTERN_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests CALL <address(dword)>
/*	Allows 'address' in memory (or in the database segment for 16bit) to be 
	executed. See the extern secion for more details. */

void test_CALL_success()
{
	TODO("CALL Not Implemented");
}

// =============================================================================
// Tests SFX <value1> <value2>
//       SFX <pa> <routine>
/*	This is a second EXTERN type action designed for Sound Effects extensions. 
	e.g. It has a 'default' function which allows value 'value1' to be written 
	to register 'value2' of the sound chip on 8 bit machines. This can be 
	changed with #sfx or through linking - see the machine details and extern 
	section for specifics. */

void test_SFX_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests GFX <pa> <routine>
/*	An EXTERN which is meant to deal with any graphics extensions to DAAD. On 
	16bit it is used to implement the screen switching facilities. This can be 
	changed with #gfx or through linking. See the machine details and extern 
	section for specifics.
	
	GFX pa routine

	where routine can be:
       0*   Back->Phys
       1*   Phys->Back
       2*   SWAP (Phys<>Back) (In CGA this is a bit rough...)
       3*   Graphics Write to Phys
       4*   Graphics Write to Back
       5*   Clear Phys
       6*   Clear Back
       7    Text Write to Phys      -ST only
       8    Text Write to Back      -ST only
       9*   Set Palette value (Value is offset of 4 flag data block containing 
	   	    Num,Red,Green,Blue. RGB values are 0-255
      10    Read Palette value (Value is offset of 4 flag data block)

	* = supported by MSX2 interpreter. */

void test_GFX_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions to show pictures [2 condacts]
// =============================================================================

// =============================================================================
// Tests PICTURE <picno>
/*	Will load into the picture buffer the given picture. If there no corresponding
	picture the next entry will be carried out, if there is then the next CondAct 
	is executed. */

void test_PICTURE_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests DISPLAY <value>
/*	If value=0 then the last buffered picture is placed onscreen. 
	If value !=0 and the picture is not a subroutine then the given window area 
	is cleared. This is normally used with indirection and a flag to check and 
	display darkness. */

void test_DISPLAY_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions miscellaneous [2 condacts]
// =============================================================================

// =============================================================================
// Tests MOUSE <option>
/*	This action in preparation for the hypercard system implements skeleton 
	mouse handler on the IBM. */

void test_MOUSE_success()
{
	TODO("MOUSE Not Implemented");
}

// =============================================================================
// Tests BEEP <length> <tone>
/*	Length is the duration in 1/50 seconds. Tone is like BEEP in ZX Basic but
	adding 60 to it and multiplied by 2.
	http://www.worldofspectrum.org/ZXBasicManual/zxmanchap19.html */

void test_BEEP_success()
{
	TODO(TODO_GENERIC);
}






// =============================================================================
// =============================================================================
// main
// =============================================================================
// =============================================================================

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
	test_REMOVE_carried(); test_REMOVE_isHere(); test_REMOVE_notHere(); test_REMOVE_notWareable(); test_REMOVE_maxObjs();
		test_REMOVE_success();
	test_CREATE_success(); test_CREATE_carried(); test_CREATE_indirection();
	test_DESTROY_success(); test_DESTROY_carried(); test_DESTROY_indirection();
	test_SWAP_success(); test_SWAP_indirection();
	test_PLACE_success(); test_PLACE_indirection();
	test_PUTO_success(); test_PUTO_indirection();
	test_PUTIN_worn(); test_PUTIN_here(); test_PUTIN_notHere(); test_PUTIN_success(); test_PUTIN_indirection();
	test_TAKEOUT_carried(); test_TAKEOUT_worn(); test_TAKEOUT_here(); test_TAKEOUT_notHere(); test_TAKEOUT_maxWeight();
		test_TAKEOUT_maxObjs(); test_TAKEOUT_success(); test_TAKEOUT_indirection();
	test_DROPALL_success();
	test_AUTOG_carried(); test_AUTOG_worn(); test_AUTOG_success();
	test_AUTOD_success();
	test_AUTOW_success();
	test_AUTOR_success();
	test_AUTOP_success();
	test_AUTOT_success();
	test_COPYOO_success(); test_COPYOO_indirection();
	test_RESET_success();
	test_COPYOF_success(); test_COPYOF_indirection();
	test_COPYFO_success(); test_COPYFO_indirection();
	test_WHATO_success();
	test_SETCO_success(); test_SETCO_indirection();
	test_WEIGH_success(); test_WEIGH_indirection();

	test_SET_success(); test_SET_indirection();
	test_CLEAR_success(); test_CLEAR_indirection();
	test_LET_success(); test_LET_indirection();
	test_PLUS_success(); test_PLUS_overflow(); test_PLUS_indirection();
	test_MINUS_success(); test_MINUS_overflow(); test_MINUS_indirection();
	test_ADD_success(); test_ADD_overflow(); test_ADD_indirection();
	test_SUB_success(); test_SUB_overflow(); test_SUB_indirection();
	test_COPYFF_success(); test_COPYFF_indirection();
	test_COPYBF_success(); test_COPYBF_indirection();
	test_RANDOM_success(); test_RANDOM_indirection();
	test_MOVE_success(); test_MOVE_indirection();

	test_GOTO_success(); test_GOTO_indirection();
	test_WEIGHT_success();
	test_ABILITY_success(); test_ABILITY_indirection();

	test_MODE_success(); test_MODE_indirection();
	test_INPUT_success(); test_INPUT_indirection();
	test_TIME_success(); test_TIME_indirection();

	test_WINDOW_success(); test_WINDOW_indirection();
	test_WINAT_success(); test_WINAT_oversize(); test_WINAT_indirection();
	test_WINSIZE_success(); test_WINSIZE_oversize(); test_WINSIZE_indirection();
	test_CENTRE_success();
	test_CLS_success();
	test_SAVEAT_success();
	test_BACKAT_success();
	test_PAPER_success();
	test_INK_success();
	test_BORDER_success();
	test_PRINTAT_success(); test_PRINTAT_indirection();
	test_TAB_success(); test_TAB_indirection();
	test_SPACE_success();
	test_NEWLINE_success();
	test_MES_success();
	test_MESSAGE_success();
	test_SYSMES_success();
	test_DESC_success();
	test_PRINT_success();
	test_DPRINT_success();
	
	test_LISTOBJ_success(); test_LISTOBJ_none();
	test_LISTAT_success(); test_LISTAT_none();

	test_SAVE_success();
	test_LOAD_success();
	test_RAMSAVE_success();
	test_RAMLOAD_success();

	test_ANYKEY_success();
	test_PAUSE_success();

	test_PARSE_success();
	test_NEWTEXT_success();
	test_SYNONYM_success();

	test_PROCESS_success();
	test_REDO_success();
	test_DOALL_success();
	test_SKIP_success();
	test_RESTART_success();
	test_END_success();
	test_EXIT_success();

	test_DONE_success();
	test_NOTDONE_success();
	test_OK_success();

	test_EXTERN_success();
	test_CALL_success();
	test_SFX_success();
	test_GFX_success();

	test_PICTURE_success();
	test_DISPLAY_success();

	test_MOUSE_success();
	test_BEEP_success();

}
