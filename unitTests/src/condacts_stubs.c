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

uint8_t  fake_keyPressed;
int16_t  fake_lastSysMesPrinted;
int      fake_lastCharPrinted;
int16_t  fake_lastUserMsgPrinted;
int16_t  fake_lastLocMsgPrinted;
uint16_t fake_lastBase10Printed;
uint16_t fake_lastXmesPrinted;
uint8_t  fake_lastPaperCol;
uint8_t  fake_lastInkCol;
uint8_t  fake_lastBorderCol;

// =============================================================================
// Global variables

PROCstack procStack[NUM_PROCS];		// Stack of calls using PROCESS condact.
PROCstack *currProc;				// Pointer to current active condact.

bool indirection;					// True if the current condact use indirection for the first argument.
bool isDone;						// Variables for ISDONE/ISNDONE condacts.
bool lastPicShow;					// True if last location picture was drawed.
#ifdef DAADV3
bool isV3;							// True if loaded DDB is version 3
#endif


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

uint8_t fake_clearLogicalSentences_calls;
void clearLogicalSentences() { fake_clearLogicalSentences_calls++; }
bool getLogicalSentence() {}
void nextLogicalSentence() {}
bool useLiteralSentence() {}
bool populateLogicalSentence() {}
void parser() {}
void mainLoop() {}

void printBase10(uint16_t value) { fake_lastBase10Printed = value; }
bool waitForTimeout(uint16_t timerFlag) {}
void errorCode(uint8_t code) {}

void printChar(int c) { fake_lastCharPrinted = c; }
void getSystemMsg(uint8_t num) {}
void printSystemMsg(uint8_t num) { if (num!=51) fake_lastSysMesPrinted = num; }
void printUserMsg(uint8_t num) { fake_lastUserMsgPrinted = num; }
void printLocationMsg(uint8_t num) { fake_lastLocMsgPrinted = num; }
void printObjectMsg(uint8_t num) {}
void printObjectMsgModif(uint8_t num, char modif) {}
void printMsg(char *p, bool pr) {}
void printOutMsg(char *s) {}
void checkPrintedLines() {}

void transcript_flush() {}
void transcript_char(char c) {}

// =============================================================================
// daad_platform_api.c

// System functions
void     setTime(uint16_t time) {}
uint16_t getTime() {}
uint16_t checkKeyboardBuffer() { return fake_keyPressed; }
void     clearKeyboardBuffer() { fake_keyPressed = 0; }
uint8_t  getKeyInBuffer() { uint8_t ret = fake_keyPressed; fake_keyPressed = 13; return ret; }

// External texts
void printXMES(uint16_t address) { fake_lastXmesPrinted = address; }

// GFX functions
inline void gfxClearWindow() {}
inline void gfxSetPaperCol(uint8_t col)  { fake_lastPaperCol = col; }
inline void gfxSetInkCol(uint8_t col)    { fake_lastInkCol = col; }
inline void gfxSetBorderCol(uint8_t col) { fake_lastBorderCol = col; }
bool gfxPicturePrepare(uint8_t location) {}
inline bool gfxPictureShow() {}

#if SCREEN == 8
uint8_t test_colorTranslationSC8[] = {
	0x00, 0x02, 0xc0, 0xc3, 0x18, 0x1b, 0x58, 0xdb, 0x49, 0x93, 0xed, 0xef, 0x7d, 0x7f, 0xfd, 0xff
};
static const uint8_t test_colorTranslationSC8_init[] = {
	0x00, 0x02, 0xc0, 0xc3, 0x18, 0x1b, 0x58, 0xdb, 0x49, 0x93, 0xed, 0xef, 0x7d, 0x7f, 0xfd, 0xff
};
#else
uint16_t test_colorTranslation[] = {
	0x000, 0x006, 0x600, 0x606, 0x060, 0x066, 0x260, 0x666,
	0x222, 0x447, 0x733, 0x737, 0x373, 0x377, 0x773, 0x777
};
static const uint16_t test_colorTranslation_init[] = {
	0x000, 0x006, 0x600, 0x606, 0x060, 0x066, 0x260, 0x666,
	0x222, 0x447, 0x733, 0x737, 0x373, 0x377, 0x773, 0x777
};
#endif

uint16_t test_gfxTextOffset;

void gfxRoutines(uint8_t routine, uint8_t value)
{
	if (routine == GFX_SET_PALETTE) {
		uint8_t idx = flags[value] % 16;
		#if SCREEN == 8
			test_colorTranslationSC8[idx] = (flags[value+2] & 0xE0)
			                              | ((flags[value+1] & 0xE0) >> 3)
			                              | ((flags[value+3] & 0xE0) >> 6);
		#elif SCREEN != 12
			uint16_t grb = (((uint16_t)flags[value+2] & 0xE0) << 3)
			             | ((flags[value+1] & 0xE0) >> 1)
			             | ((flags[value+3] & 0xE0) >> 5);
			test_colorTranslation[idx] = grb;
		#endif
	} else if (routine == GFX_GET_PALETTE) {
		uint8_t idx = flags[value] % 16;
		#if SCREEN == 8
			uint8_t grb8 = test_colorTranslationSC8[idx];
			flags[value+1] = (grb8 << 3) & 0xE0;
			flags[value+2] = grb8 & 0xE0;
			flags[value+3] = (grb8 << 6) & 0xC0;
		#elif SCREEN != 12
			uint16_t grb = test_colorTranslation[idx];
			flags[value+1] = (uint8_t)((grb << 1) & 0xE0);
			flags[value+2] = (uint8_t)((grb >> 3) & 0xE0);
			flags[value+3] = (uint8_t)((grb << 5) & 0xE0);
		#endif
	} else if (routine == GFX_TEXTS_IN_PHYS) {
		test_gfxTextOffset = 0;
	} else if (routine == GFX_TEXTS_IN_BACK) {
		test_gfxTextOffset = 256;
	}
}

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
	RESET_TESTS();
}

void beforeEach()
{
	initFlags();
	initializePROC();
	pushPROC(0);

	checkEntry = true;
	isDone = false;

	memset(hdr, 0, sizeof(DDB_Header));
	hdr->version = 2;
#ifdef DAADV3
	isV3 = false;
#endif
	memset(objects, 0, sizeof(Object) * MOCK_NUM_OBJECTS);
	memset(windows, 0, sizeof(Window) * WINDOWS_NUM);
	memset(ramsave, 0, 512);

	hdr->numObjDsc = MOCK_NUM_OBJECTS;

	cw = windows;

	fake_keyPressed                  = 0;
	fake_clearLogicalSentences_calls = 0;
	fake_lastSysMesPrinted  = -1;
	fake_lastCharPrinted    = -1;
	fake_lastUserMsgPrinted = -1;
	fake_lastLocMsgPrinted  = -1;
	fake_lastBase10Printed  = 0;
	fake_lastXmesPrinted    = 0;
	fake_lastPaperCol       = 0;
	fake_lastInkCol         = 0;
	fake_lastBorderCol      = 0;

	#if SCREEN == 8
		memcpy(test_colorTranslationSC8, test_colorTranslationSC8_init, 16);
	#else
		memcpy(test_colorTranslation, test_colorTranslation_init, 32);
	#endif

	test_gfxTextOffset = 0;
}

void do_action(char *pProc)
{
	pPROC = pProc + 1;
	indirection = *pProc & IND;

	CondactStruct *currCondact = (CondactStruct*)pProc++;
	const CONDACT_LIST *ce = &condactList[currCondact->condact];
	ce->function();
	isDone |= ce->flag;           // match processPROC() post-condact OR (daad_condacts.c:203)
}

// Simulate processPROC()'s inner loop for a full entry: execute condacts
// sequentially honoring checkEntry (ISDONE/ISNDONE can abort the rest)
// and applying the isDone |= ce->flag post-condact rule. Terminator: 0xff.
void do_entry(const char *pEntry)
{
	pPROC = (uint8_t*)pEntry;
	checkEntry = true;
	isDone = false;
	uint8_t temp;
	while (checkEntry && (temp=*pPROC)!=0xff) {
		CondactStruct *cc = (CondactStruct*)pPROC++;
		indirection = cc->indirection;
		const CONDACT_LIST *ce = &condactList[cc->condact];
		ce->function();
		isDone |= ce->flag;
	}
}
