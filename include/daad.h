#ifndef  __DAAD_H__
#define  __DAAD_H__

#include <stdint.h>
#include <ctype.h>
#include "utils.h"
#include "heap.h"
#include "dos.h"
#include "asm.h"
#include "vdp.h"


#define MAX_PICFILE_READ	2048

#ifndef FONTWIDTH
	#define FONTWIDTH       6
#endif
#if FONTWIDTH!=6 && FONTWIDTH!=8
	#error FONTWIDTH constant must be 6 or 8!
#endif

#if SCREEN==6 || SCREEN==7
	#define SCREEN_WIDTH    512
#else
	#define SCREEN_WIDTH    256
#endif

#define MAX_COLUMNS		    ((int)(SCREEN_WIDTH/FONTWIDTH))
#define MAX_LINES			26

#define PROMPT_TEXT_LEN		100
#define TEXT_BUFFER_LEN		PROMPT_TEXT_LEN	// Double buffer

// Used in struct Object->location
#define LOC_NOTCREATED		252
#define LOC_WORN			253
#define LOC_CARRIED			254
#define LOC_HERE			255

// Vocabulary constant
#define NULLWORD			255

// Used by HASHAT/HASNAT
#define HA_WAREABLE			23		// Flag 57 Bit#7
#define HA_CONTAINER		31		// Flag 56 Bit#7
#define HA_LISTED			55		// Flag 53 Bit#7
#define HA_TIMEOUT			87		// Flag 49 Bit#7
#define HA_MOUSE			240		// Flag 29 Bit#0
#define HA_GMODE			247		// Flag 29 Bit#7


// DDB header
typedef struct {

	uint8_t	 version;		// 0x00 | 1 byte  | DAAD version number (1 for Aventura Original and Jabato, 1989, 2 for the rest)
	union {					// 0x01 | 1 byte  | High nibble: target machine | Low nibble: target language
		uint8_t byte;
		struct {
			unsigned language : 4;
			unsigned machine  : 4;
		} value;
	} target;
	uint8_t  magic;			// 0x02 | 1 byte  | Always contains CTL value: 95d (ASCII '_')
	uint8_t  numObjDsc;		// 0x03 | 1 byte  | Number of object descriptions
	uint8_t  numLocDsc;		// 0x04 | 1 byte  | Number of location descriptions
	uint8_t  numUsrMsg;		// 0x05 | 1 byte  | Number of user messages
	uint8_t  numSysMsg;		// 0x06 | 1 byte  | Number of system messages
	uint8_t  numPrc;		// 0x07 | 1 byte  | Number of processes
	uint16_t tokensPos;		// 0x08 | 2 bytes | Compressed text position
	uint16_t prcLstPos;		// 0x0A | 2 bytes | Process list position
	uint16_t objLstPos;		// 0x0C | 2 bytes | Objects lookup list position
	uint16_t locLstPos;		// 0x0E | 2 bytes | Locations lookup list position
	uint16_t usrMsgPos;		// 0x10 | 2 bytes | User messages lookup list position
	uint16_t sysMsgPos;		// 0x12 | 2 bytes | System messages lookup list position
	uint16_t conLstPos;		// 0x14 | 2 bytes | Connections lookup list position
	uint16_t vocPos;		// 0x16 | 2 bytes | Vocabulary
	uint16_t objLocLst;		// 0x18 | 2 bytes | Objects "initialy at" list position
	uint16_t objNamePos;	// 0x1A | 2 bytes | Object names positions
	uint16_t objAttrPos;	// 0x1C | 2 bytes | Object weight and container/wearable attributes
	uint16_t objExtrPos;	// 0x1E | 2 bytes | Extra object attributes 
	uint16_t fileLength;	// 0x20 | 2 bytes | File length
} DDB_Header;

typedef struct {
	uint8_t word[5];
	uint8_t id;
	uint8_t type;
} Vocabulary;

typedef struct {
	uint8_t location;
	union {
		uint8_t byte;		// bits0-5:Weight|bit6:IsContainer|bit7:IsWorn
		struct {
			unsigned weight      : 6;
			unsigned isContainer : 1;
			unsigned isWareable  : 1;
		} mask;
	} attribs;
	uint8_t extAttr1;		// Extended attributes
	uint8_t extAttr2;
	uint8_t nounId;			// Noun
	uint8_t adjectiveId;	// Adjective
} Object;

typedef struct {
	uint8_t winX;
	uint8_t winY;
	uint8_t winH;
	uint8_t winW;
	uint8_t cursorX;
	uint8_t cursorY;
} Window;

typedef struct {
	uint8_t verb;
	uint8_t noun;
	char *pCondacts;
} PROCentry;

typedef struct {
	PROCentry *entryIni;		// First entry in current PROCess
	PROCentry *entry;			// Current entry in current PROCess
	char *condactIni;			// First condact in current entry
	char *condact;				// Current condact in current entry
} PROCstack;

enum VOC_TYPE {
	VERB,						// 0
	ADVERB,						// 1
	NOUN,						// 2
	ADJECTIVE,					// 3
	PREPOSITION,				// 4
	CONJUNCTION,				// 5
	PRONOUN						// 6
};



// System Flags (0-63)
#define fDark      0	// when non zero indicates game is dark (see also object 0)
#define fNOCarr    1	// Holds quantity of objects player is carrying (but not wearing)
#define fWork1     2	// These are system as we consider the stack such
#define fWork2     3
#define fStack    24	// A small stack (always 2 bytes pushed) 10 pushes
#define fEMPTY    23	// Stack can run from here
#define fFULL      3	// to here - There will be an internal one soon
#define fO2Num    25	// 1st free in system 64
#define fO2Con    26	// Object 2 is a container
#define fO2Loc    27
#define fDarkF    28
#define fGFlags   29	// This is best tested using HASAT GMODE
#define fScore    30	// (Optional) Score flag
#define fTurns    31	// Number of turns taken (2 bytes LE)
#define fVerb     33	// Verb for the current logical sentence
#define fNoun1    34	// First Noun in the current logical sentence
#define fAdject1  35	// Adjective for first Noun
#define fAdverb   36	// Adverb for the current logical sentence
#define fMaxCarr  37	// Maximum number of objects conveyable (initially 4) Set using ABILITY action.
#define fPlayer   38	// Current location of player
#define fO2Att    39	// (Optional) Using Flags 39 and 40 to contain attribs for other obj
#define fInStream 41	// Gives stream number for input to use. 0 means current stream. Used Mod 8. i.e. 8 is considered as 0
#define fPrompt   42	// Holds prompt to use a system message number (0 selects one of four randomly)
#define fPrep     43	// Preposition in the current logical sentence
#define fNoun2    44	// Second Noun in the current logical sentence
#define fAdject2  45	// Adjective for the second Noun
#define fCPNoun   46	// Current pronoun ("IT" usually) Noun
#define fCPAdject 47	// Current pronoun ("IT" usually) Adjective
#define fTime     48	// Timeout duration required
#define fTIFlags  49	// Timeout Control bitmask flags (see documentation)
#define fDAObjNo  50	// Objno. for DOALL loop. i.e. value following DOALL
#define fCONum    51	// Last object referenced by GET/DROP/WEAR/WHATO etc.
#define fStrength 52	// Players strength (maximum weight of objects carried and worn - initially 10)
#define fOFlags   53	// Holds object print flags (bitmask see documentation)
#define fCOLoc    54	// Holds the present location of the currently referenced object
#define fCOWei    55	// Holds the weight of the currently referenced object
#define fCOCon    56	// Is 128 if the currently referenced object is a container
#define fCOWR     57	// Is 128 if the currently referenced object is wearable
#define fCOAtt    58	// Currently referenced objects user attribs
#define fKey1     60	// Key returned by INKEY
#define fKey2     61	// Key for IBM extended code only (0 otherwise)
#define fScMode   62	// 2=Text, 4=CGA, 13=EGA, 141=VGA
#define fCurWin   63	// Which window is active at the moment (for read only)


// Condacts
#define CDT_AT         0   //   1
#define CDT_NOTAT      1   //   1
#define CDT_ATGT       2   //   1
#define CDT_ATLT       3   //   1
#define CDT_PRESENT    4   //   1
#define CDT_ABSENT     5   //   1
#define CDT_WORN       6   //   1
#define CDT_NOTWORN    7   //   1
#define CDT_CARRIED    8   //   1
#define CDT_NOTCARR    9   //   1
#define CDT_CHANCE     10  //   1
#define CDT_ZERO       11  //   1
#define CDT_NOTZERO    12  //   1
#define CDT_EQ         13  //   2
#define CDT_GT         14  //   2
#define CDT_LT         15  //   2
#define CDT_ADJECT1    16  //   1
#define CDT_ADVERB     17  //   1
#define CDT_SFX        18  //   2
#define CDT_DESC       19  //   1
#define CDT_QUIT       20  //   0
#define CDT_END        21  //   0
#define CDT_DONE       22  //   0
#define CDT_OK         23  //   0
#define CDT_ANYKEY     24  //   0
#define CDT_SAVE       25  //   1
#define CDT_LOAD       26  //   1
#define CDT_DPRINT     27  //   1 *
#define CDT_DISPLAY    28  //   1 *
#define CDT_CLS        29  //   0
#define CDT_DROPALL    30  //   0
#define CDT_AUTOG      31  //   0
#define CDT_AUTOD      32  //   0
#define CDT_AUTOW      33  //   0
#define CDT_AUTOR      34  //   0
#define CDT_PAUSE      35  //   1
#define CDT_SYNONYM    36  //   2 *
#define CDT_GOTO       37  //   1
#define CDT_MESSAGE    38  //   1
#define CDT_REMOVE     39  //   1
#define CDT_GET        40  //   1
#define CDT_DROP       41  //   1
#define CDT_WEAR       42  //   1
#define CDT_DESTROY    43  //   1
#define CDT_CREATE     44  //   1
#define CDT_SWAP       45  //   2
#define CDT_PLACE      46  //   2
#define CDT_SET        47  //   1
#define CDT_CLEAR      48  //   1
#define CDT_PLUS       49  //   2
#define CDT_MINUS      50  //   2
#define CDT_LET        51  //   2
#define CDT_NEWLINE    52  //   0
#define CDT_PRINT      53  //   1
#define CDT_SYSMESS    54  //   1
#define CDT_ISAT       55  //   2
#define CDT_SETCO      56  //   1
#define CDT_SPACE      57  //   0
#define CDT_HASAT      58  //   1
#define CDT_HASNAT     59  //   1
#define CDT_LISTOBJ    60  //   0
#define CDT_EXTERN     61  //   2
#define CDT_RAMSAVE    62  //   0
#define CDT_RAMLOAD    63  //   1
#define CDT_BEEP       64  //   2
#define CDT_PAPER      65  //   1
#define CDT_INK        66  //   1
#define CDT_BORDER     67  //   1
#define CDT_PREP       68  //   1
#define CDT_NOUN2      69  //   1
#define CDT_ADJECT2    70  //   1
#define CDT_ADD        71  //   2
#define CDT_SUB        72  //   2
#define CDT_PARSE      73  //   1
#define CDT_LISTAT     74  //   1
#define CDT_PROCESS    75  //   1
#define CDT_SAME       76  //   2
#define CDT_MES        77  //   1
#define CDT_WINDOW     78  //   1
#define CDT_NOTEQ      79  //   2
#define CDT_NOTSAME    80  //   2
#define CDT_MODE       81  //   1
#define CDT_WINAT      82  //   2
#define CDT_TIME       83  //   2
#define CDT_PICTURE    84  //   1
#define CDT_DOALL      85  //   1
#define CDT_MOUSE      86  //   1
#define CDT_GFX        87  //   2
#define CDT_ISNOTAT    88  //   2
#define CDT_WEIGH      89  //   2
#define CDT_PUTIN      90  //   2
#define CDT_TAKEOUT    91  //   2
#define CDT_NEWTEXT    92  //   0
#define CDT_ABILITY    93  //   2
#define CDT_WEIGHT     94  //   1
#define CDT_RANDOM     95  //   1
#define CDT_INPUT      96  //   2
#define CDT_SAVEAT     97  //   0
#define CDT_BACKAT     98  //   0
#define CDT_PRINTAT    99  //   2
#define CDT_WHATO      100 //   0
#define CDT_CALL       101 //   1
#define CDT_PUTO       102 //   1
#define CDT_NOTDONE    103 //   0
#define CDT_AUTOP      104 //   1
#define CDT_AUTOT      105 //   1
#define CDT_MOVE       106 //   1
#define CDT_WINSIZE    107 //   2
#define CDT_REDO       108 //   0
#define CDT_CENTRE     109 //   0
#define CDT_EXIT       110 //   1
#define CDT_INKEY      111 //   0
#define CDT_BIGGER     112 //   2
#define CDT_SMALLER    113 //   2
#define CDT_ISDONE     114 //   0
#define CDT_ISNDONE    115 //   0
#define CDT_SKIP       116 //   1
#define CDT_RESTART    117 //   0
#define CDT_TAB        118 //   1
#define CDT_COPYOF     119 //   2
#define CDT_dumb1      120 //   0 (according DAAD manual, internal)
#define CDT_COPYOO     121 //   2
#define CDT_dumb2      122 //   0 (according DAAD manual, internal)
#define CDT_COPYFO     123 //   2
#define CDT_dumb3      124 //   0 (according DAAD manual, internal)
#define CDT_COPYFF     125 //   2
#define CDT_COPYBF     126 //   2
#define CDT_RESET      127 //   0


// Global variables
extern uint8_t    *ddb;
extern DDB_Header *hdr;
extern Object     *objects;
extern char       *ramsave;

extern Z80_registers regs;
extern char *pPROC;						// Pointer to currect process condact

extern char        *tmpMsg;				// MAX_TEXT_LEN
extern uint8_t     flags[256];

extern Window windows[8];				// 0-7 windows definitions
extern Window *cw;						// Pointer to current active window
extern uint8_t savedPosX;
extern uint8_t savedPosY;



// Core function definitions
bool initDDB();
void initFlags();
void initObjects();
void prompt();
void parser();
void clearLogicalSentences();
bool getLogicalSentence();
void nextLogicalSentence();
void printBase10(uint16_t value);
void mainLoop();

char* getToken(uint8_t num);
void getSystemMsg(uint8_t num);
void printSystemMsg(uint8_t num);
void printUserMsg(uint8_t num);
void printLocationMsg(uint8_t num);
void printObjectMsg(uint8_t num);
void printObjectMsgModif(uint8_t num, char modif);
uint8_t getObjectById(uint8_t noun, uint8_t adjc);
uint8_t getObjectWeight(uint8_t objno, bool isCarriedWorn);
void referencedObject(uint8_t objno);

void initializePROC();
void pushPROC(uint8_t proc);
bool popPROC();
PROCentry* getPROCess(uint8_t proc);
char* getPROCEntryCondacts();
char* stepPROCEntryCondacts(int8_t step);
void processPROC();

// GFX function definitions
extern uint16_t colorTranslation[];

void gfxSetScreen();
void gfxClearLines(uint16_t start, uint16_t lines);
void gfxClearScreen();
void gfxClearWindow();
void gfxSetPaperCol(uint8_t col);
void gfxSetInkCol(uint8_t col);
void gfxSetBorderCol(uint8_t col);
void gfxPutChPixels(char c);
void gfxPutCh(char c);
void gfxPuts(char *str);
void gfxPutsln(char *str);
void gfxScrollUp();
bool gfxPicturePrepare(uint8_t location);
void gfxPictureShow();


#endif //__DAAD_H__
