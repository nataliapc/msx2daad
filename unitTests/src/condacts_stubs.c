#pragma opt_code_size

#include "condacts_stubs.h"


const char ERROR[] = "Error with checkEntry";
const char ERROR_ISDONE[] = "Error with isDone";
const char ERROR_SYSMES[] = "SystemMessage";
const char ERROR_OBJLOC[] = "Object location";
const char ERROR_FLAG[] = "Bad flag value";
const char ERROR_CARROBJNUM[] = "Carried objects number";
const char TODO_GENERIC[] = "-----";
const char TODO_UI[] = "UI not mocked";

uint8_t fake_keyPressed;
int16_t fake_lastSysMesPrinted;

// =============================================================================
// Global variables

PROCstack procStack[NUM_PROCS];		// Stack of calls using PROCESS condact.
PROCstack *currProc;				// Pointer to current active condact.

bool indirection;					// True if the current condact use indirection for the first argument.
bool isDone;						// Variables for ISDONE/ISNDONE condacts.
bool lastPicShow;					// True if last location picture was drawed.


static const uint8_t nullObjFake[] = { 0, 0, 0, 0, 0, 0 };
const Object *nullObject = (const Object*)nullObjFake;
char *tmpTok;
uint8_t doingPrompt;
bool checkPrintedLines_inUse;

uint8_t    *ddb;					// Where the DDB is allocated
DDB_Header *hdr;					// Struct pointer to DDB Header
Object     *objects;				// Memory allocation for objects data
uint8_t     flags[256];				// DAAD flags (256 bytes)
char       *ramsave;				// Memory to store ram save (RAMSAVE)

Window	*windows;					// 0-7 windows definitions
Window	*cw;						// Pointer to current active window
uint8_t printedLines;				// For "More..." feature

uint8_t savedPosX;					// For SAVEAT/BACKAT
uint8_t savedPosY;					//  "    "      "

char *tmpMsg;						// TEXT_BUFFER_LEN


// =============================================================================
// daad.c

bool initDAAD(int argc, char **argv) {}
void initFlags() {}
void initObjects() {}

void prompt(bool printPromptMsg) {}

void clearLogicalSentences() {}
bool getLogicalSentence() {}
void nextLogicalSentence() {}
bool useLiteralSentence() {}
bool populateLogicalSentence() {}
void parser() {}
void mainLoop() {}

void printBase10(uint16_t value) __z88dk_fastcall {}
bool waitForTimeout(uint16_t timerFlag) __z88dk_fastcall {}
void errorCode(uint8_t code) {}

void printChar(int c) __z88dk_fastcall {}
void getSystemMsg(uint8_t num) __z88dk_fastcall {}
void printSystemMsg(uint8_t num) __z88dk_fastcall { if (num!=51) fake_lastSysMesPrinted = num; }
void printUserMsg(uint8_t num) __z88dk_fastcall {}
void printLocationMsg(uint8_t num) __z88dk_fastcall {}
void printObjectMsg(uint8_t num) __z88dk_fastcall {}
void printObjectMsgModif(uint8_t num, char modif) {}
void printMsg(char *p, bool pr) {}
void printOutMsg(char *s) {}
void checkPrintedLines() {}

void transcript_flush() {}
void transcript_char(char c) __z88dk_fastcall {}

// =============================================================================
// daad_platform_api.c

// System functions
void     setTime(uint16_t time) __z88dk_fastcall {}
uint16_t getTime() {}
uint16_t checkKeyboardBuffer() { return fake_keyPressed; }
void     clearKeyboardBuffer() { fake_keyPressed = 0; }
uint8_t  getKeyInBuffer() { uint8_t ret = fake_keyPressed; fake_keyPressed = 13; return ret; }

// External texts
void printXMES(uint16_t address) __z88dk_fastcall {}

// GFX functions
inline void gfxClearWindow() {}
inline void gfxSetPaperCol(uint8_t col) {}
inline void gfxSetInkCol(uint8_t col) {}
inline void gfxSetBorderCol(uint8_t col) {}
bool gfxPicturePrepare(uint8_t location) {}
inline bool gfxPictureShow() {}
inline void gfxRoutines(uint8_t routine, uint8_t value) {}

// SFX functions
void sfxInit() {}
void sfxWriteRegister(uint8_t reg, uint8_t value) {}
void sfxTone(uint8_t value1, uint8_t value2) {}


// =============================================================================

void beforeAll()
{
	sfxInit();
	objects = malloc(sizeof(Object) * MOCK_NUM_OBJECTS);
	windows = malloc(sizeof(Window) * WINDOWS_NUM);
	currProc = malloc(sizeof(PROCstack));

	ddb = malloc(sizeof(DDB_Header));
	hdr = (DDB_Header *)ddb;

	ramsave = malloc(512);
	tmpTok = malloc(32);
	tmpMsg = malloc(100);
	nullObject = (const Object*)nullObjFake;
}

void beforeEach()
{
	initFlags();
	initializePROC();
	pushPROC(0);

	checkEntry = true;
	isDone = false;

	memset(hdr, 0, sizeof(DDB_Header));
	memset(objects, 0, sizeof(Object) * MOCK_NUM_OBJECTS);
	memset(windows, 0, sizeof(Window) * WINDOWS_NUM);
	memset(ramsave, 0, 512);

	hdr->numObjDsc = MOCK_NUM_OBJECTS;

	cw = windows;

	fake_keyPressed = 0;
	fake_lastSysMesPrinted = -1;
}

void do_action(char *pProc)
{
	pPROC = pProc + 1;
	indirection = *pProc & IND;

	CondactStruct *currCondact = (CondactStruct*)pProc++;
	condactList[currCondact->condact].function();
}
