#pragma opt_code_size

#include "daad_stubs.h"


// =============================================================================
// Global variables (same layout as daad_stubs.c)

static const uint8_t nullObjFake[] = { 0, 0, 0, 0, 0, 0 };
const Object *nullObject = (const Object*)nullObjFake;
char *tmpTok;
uint8_t doingPrompt;
bool checkPrintedLines_inUse;

int16_t fake_lastSysMesPrinted;

PROCstack procStack[NUM_PROCS];
PROCstack *currProc;

bool indirection;
bool isDone;
bool lastPicShow;
#ifdef DAADV3
bool isV3;
#endif

uint8_t    *ddb;
DDB_Header *hdr;
Object     *objects;
uint8_t     flags[256];
char       *ramsave;

Window	*windows;
Window	*cw;
uint8_t printedLines;

uint8_t savedPosX;
uint8_t savedPosY;

char *tmpMsg;


// =============================================================================
// Stub functions
// NOTE: parser functions are NOT stubbed here — they come from daad_parser_sentences.rel

void loadFilesBin(int argc, char **argv) {}
uint16_t loadFile(char *filename, uint8_t *destaddress, uint16_t size) {}
uint16_t getFreeMemory() { return 0x4000; }
bool checkPlatformSystem() { return true; }
char* getCharsTranslation() { return 0; }

// prompt — called by getLogicalSentence (in daad_parser_sentences.c)
void prompt(bool p) {}

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
// parser() calls safeMemoryAllocate() for tmpVOC (5 bytes); needs real memory.
static uint8_t _tmpVOC_buf[8];
inline void *safeMemoryAllocate() { return _tmpVOC_buf; }
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

// PROC stack stubs
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
