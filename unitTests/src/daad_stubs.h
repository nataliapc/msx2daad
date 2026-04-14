#ifndef DAAD_STUBS_H
#define DAAD_STUBS_H

#define WINDOWS_NUM		8

#include <string.h>
#include "assert.h"
#include "daad.h"
#include "daad_platform_api.h"
#include "daad_condacts.h"

#define MOCK_DAAD_NUM_OBJECTS	8
#define pPROC 		currProc->condact
#define checkEntry  currProc->continueEntry

#define IND						128

extern int16_t fake_lastSysMesPrinted;

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

// Function prototypes
void daad_beforeAll();
void daad_beforeEach();

#endif // DAAD_STUBS_H
