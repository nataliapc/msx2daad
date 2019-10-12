#ifndef  __DAAD_H__
#define  __DAAD_H__

#include <stdint.h>
#include <ctype.h>
#include "daad_platform_api.h"
#include "daad_defines.h"
#include "utils.h"
#ifdef MSX2
	#include "heap.h"
	#include "dos.h"
#endif


#define TEXT_BUFFER_LEN		100

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

// Used by TIME and flag fTIFlags
#define TIME_FIRSTCHAR		1		// Set this so timeout can occur at start of input only
#define TIME_MORE			2		// Set this so timeout can occur on "More..."
#define TIME_ANYKEY			4		// Set this so timeout can occur on ANYKEY
#define TIME_CLEAR			8		// TODO Set this to clear input window
#define TIME_INPUT			16		// TODO Set this to print input in current stream after edit
#define TIME_RECALL			32		// TODO Set this to cause auto recall of input buffer after timeout
#define TIME_AVAILABLE		64		// TODO Set if data available for recall (not of use to writer)
#define TIME_TIMEOUT		128		// Set if timeout occurred last frame


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

// Process entry
typedef struct {
	uint8_t verb;
	uint8_t noun;
	char *pCondacts;
} PROCentry;

// Process call stack
typedef struct {
	PROCentry *entryIni;		// First entry in current PROCess
	PROCentry *entry;			// Current entry in current PROCess
	char *condactIni;			// First condact in current entry
	char *condact;				// Current condact in current entry
} PROCstack;

// Condact struct
typedef struct {
	unsigned condact     : 7;
	unsigned indirection : 1;
} CondactStruct;

// Window
typedef struct {
	uint8_t winX;
	uint8_t winY;
	uint8_t winW;
	uint8_t winH;
	uint8_t cursorX;
	uint8_t cursorY;
} Window;

// Object Entry
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

// Vocabulary Entry
typedef struct {
	uint8_t word[5];
	uint8_t id;
	uint8_t type;
} Vocabulary;

// Vocabulary types
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


// Global variables
extern uint8_t    *ddb;				// Where the DDB is allocated
extern DDB_Header *hdr;				// Struct pointer to DDB Header
extern Object     *objects;			// Memory allocation for objects data
extern char       *ramsave;			// Memory to store ram save

extern char       *tmpMsg;			// MAX_TEXT_LEN
extern uint8_t     offsetText;
extern uint8_t     flags[256];		// DAAD flags

extern Window      windows[8];		// 0-7 windows definitions
extern Window     *cw;				// Pointer to current active window
#ifndef DISABLE_SAVEAT
extern uint8_t     savedPosX;		// For SAVEAT/BACKAT
extern uint8_t     savedPosY;		//  "    "      "
#endif

// DAAD Core function definitions
bool initDAAD();
void initFlags();
void initObjects();
void prompt();
void parser();
void clearLogicalSentences();
bool getLogicalSentence();
void nextLogicalSentence();
void printBase10(uint16_t value);
bool waitForTimeout(uint16_t timerFlag);
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


#endif //__DAAD_H__
