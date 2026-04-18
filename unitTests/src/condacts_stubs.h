#ifndef CONDACTS_STUBS_H
#define CONDACTS_STUBS_H

#define WINDOWS_NUM		8

#include <string.h>
#include "assert.h"
#include "daad.h"
#include "daad_platform_api.h"
#include "daad_condacts.h"

#define pPROC 		currProc->condact
#define checkEntry  currProc->continueEntry

#define IND						128
#define MOCK_NUM_OBJECTS		10

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
	_XMES,      _COPYOO,    _INDIR,    _COPYFO,   _SETAT,    _COPYFF,    _COPYBF,    _RESET								// 120-127
};

// Global variables
extern uint8_t  fake_keyPressed;
extern int16_t  fake_lastSysMesPrinted;
extern int      fake_lastCharPrinted;
extern int16_t  fake_lastUserMsgPrinted;
extern int16_t  fake_lastLocMsgPrinted;
extern uint16_t fake_lastBase10Printed;
extern uint16_t fake_lastXmesPrinted;
extern uint8_t  fake_lastPaperCol;
extern uint8_t  fake_lastInkCol;
extern uint8_t  fake_lastBorderCol;

extern const CONDACT_LIST condactList[];

extern PROCstack procStack[];
extern PROCstack *currProc;

extern bool indirection;
extern bool isDone;
extern bool lastPicShow;

extern uint8_t    *ddb;
extern DDB_Header *hdr;
extern Object     *objects;
extern uint8_t     flags[];
extern char       *ramsave;

extern Window	*windows;
extern Window	*cw;
extern uint8_t printedLines;

extern uint8_t savedPosX;
extern uint8_t savedPosY;

extern char *tmpMsg;

// Palette test arrays (mirror of daad_platform_msx2.c, reset by beforeEach)
#if SCREEN == 8
extern uint8_t  test_colorTranslationSC8[];
#else
extern uint16_t test_colorTranslation[];
#endif

// Const string arrays
extern const char ERROR[];
extern const char ERROR_ISDONE[];
extern const char ERROR_SYSMES[];
extern const char ERROR_OBJLOC[];
extern const char ERROR_FLAG[];
extern const char ERROR_CARROBJNUM[];
extern const char TODO_GENERIC[];
extern const char TODO_UI[];

// Function prototypes
void beforeAll();
void beforeEach();
void do_action(char *pProc);

#endif // CONDACTS_STUBS_H
