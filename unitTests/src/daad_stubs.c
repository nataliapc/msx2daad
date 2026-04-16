#pragma opt_code_size

#include "daad_stubs.h"


// =============================================================================
// Global variables

static const uint8_t nullObjFake[] = { 0, 0, 0, 0, 0, 0 };
const Object *nullObject = (const Object*)nullObjFake;
char *tmpTok;
uint8_t doingPrompt;
bool checkPrintedLines_inUse;

int16_t fake_lastSysMesPrinted;

PROCstack procStack[NUM_PROCS];		// Stack of calls using PROCESS condact.
PROCstack *currProc;				// Pointer to current active condact.

bool indirection;					// True if the current condact use indirection for the first argument.
bool isDone;						// Variables for ISDONE/ISNDONE condacts.
bool lastPicShow;					// True if last location picture was drawed.
#ifdef DAADV3
bool isV3;							// True if loaded DDB is version 3
#endif

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
// Stub functions

void loadFilesBin(int argc, char **argv) {}
uint16_t loadFile(char *filename, uint8_t *destaddress, uint16_t size) {}
uint16_t getFreeMemory() { return 0x4000; }
bool checkPlatformSystem() { return true; }
char* getCharsTranslation() { return 0; }

// Parser stubs
void clearLogicalSentences() {}
bool getLogicalSentence() {}
void nextLogicalSentence() {}
bool useLiteralSentence() {}
bool populateLogicalSentence() {}
void prompt(bool p) {}
void parser() {}

// Print stubs
void printBase10(uint16_t v) {}
bool waitForTimeout(uint16_t t) {}
void errorCode(uint8_t c) {}
void printChar(int c) {}
void getSystemMsg(uint8_t n) {}
void printSystemMsg(uint8_t n) { if (n!=51) fake_lastSysMesPrinted = n; }
void printUserMsg(uint8_t n) {}
void printLocationMsg(uint8_t n) {}
void printObjectMsg(uint8_t n) {}
void printObjectMsgModif(uint8_t n, char m) {}
void printMsg(char *p, bool pr) {}
void printOutMsg(char *s) {}
void checkPrintedLines() {}
void mainLoop() {}

// Transcript stubs
void transcript_flush() {}
void transcript_char(char c) {}

// Platform stubs
void setTime(uint16_t t) {}
uint16_t getTime() {}
uint16_t checkKeyboardBuffer() { return 0; }
void clearKeyboardBuffer() {}
uint8_t getKeyInBuffer() { return 13; }
void waitingForInput() {}
inline void *safeMemoryAllocate() {}
inline void safeMemoryDeallocate(void *p) {}
void printXMES(uint16_t a) {}

// GFX stubs
void gfxSetScreen() {}
void gfxSetScreenModeFlags() {}
void gfxClearScreenBlock(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {}
inline void gfxClearWindow() {}
inline void gfxClearCurrentLine() {}
inline void gfxScrollUp() {}
inline void gfxSetPaperCol(uint8_t c) {}
inline void gfxSetInkCol(uint8_t c) {}
inline void gfxSetBorderCol(uint8_t c) {}
inline void gfxSetGraphCharset(bool v) {}
inline void gfxPutChWindow(uint8_t c) {}
inline void gfxPutInputEcho(char c, bool k) {}
inline void gfxSetPalette(uint8_t i, uint8_t r, uint8_t g, uint8_t b) {}
bool gfxPicturePrepare(uint8_t l) {}
inline bool gfxPictureShow() {}
inline void gfxRoutines(uint8_t r, uint8_t v) {}

// SFX stubs
void sfxInit() {}
void sfxWriteRegister(uint8_t r, uint8_t v) {}
void sfxTone(uint8_t v1, uint8_t v2) {}

// PROC stack stubs (minimal: just enough for daad_beforeEach setup)
void initializePROC() { if (currProc) currProc->continueEntry = true; }
void pushPROC(uint8_t proc) {}
bool popPROC() {}
PROCentry* getPROCess(uint8_t proc) { return 0; }
char* getPROCEntryCondacts() { return 0; }
char* stepPROCEntryCondacts(int8_t step) { return 0; }
void processPROC() {}


// =============================================================================

void daad_beforeAll()
{
	objects = malloc(sizeof(Object) * MOCK_DAAD_NUM_OBJECTS);
	windows = malloc(sizeof(Window) * WINDOWS_NUM);
	currProc = malloc(sizeof(PROCstack));
	ddb = malloc(sizeof(DDB_Header));
	hdr = (DDB_Header *)ddb;
	ramsave = malloc(512);
	tmpTok = malloc(32);
	tmpMsg = malloc(100);
	nullObject = (const Object*)nullObjFake;
}

void daad_beforeEach()
{
	memset(flags, 0, 256);
	memset(objects, 0, sizeof(Object) * MOCK_DAAD_NUM_OBJECTS);
	memset(windows, 0, sizeof(Window) * WINDOWS_NUM);
	memset(ramsave, 0, 512);
	memset(hdr, 0, sizeof(DDB_Header));
	memset(tmpTok, 0, 32);
	hdr->numObjDsc = MOCK_DAAD_NUM_OBJECTS;
	cw = windows;
	fake_lastSysMesPrinted = -1;
	isDone = false;
	initializePROC();
	pushPROC(0);
	currProc->continueEntry = true;
}
