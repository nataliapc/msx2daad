/*
	Copyright (c) 2019 Natalia Pujol Cremades
	natypclicense@gmail.com

	See LICENSE file.

	DAAD is a trademark of Andrés Samudio
*/
#include "daad.h"
#include "daad_condacts.h"


// =============================================================================
// Condact engine state (used from daad_condacts.c and helpers)
// =============================================================================

PROCstack procStack[NUM_PROCS];		// Stack of calls using PROCESS condact.
PROCstack *currProc;				// Pointer to current active condact.

bool indirection;					// True if the current condact use indirection for the first argument.
bool isDone;						// Variable for ISDONE/ISNOTDONE condacts.
bool lastPicShow;					// True if last location picture was drawed.


// =============================================================================
// DDB image, object table, flags and RAMSAVE buffer
// =============================================================================

uint8_t    *ddb;					// Where the DDB is allocated
DDB_Header *hdr;					// Struct pointer to DDB Header
Object     *objects;				// Memory allocation for objects data
uint8_t     flags[256];				// DAAD flags (256 bytes)
char       *ramsave;				// Memory to store ram save (RAMSAVE)
const Object *nullObject;			// Placeholder for NULLWORD object references


// =============================================================================
// Windows and printing state
// =============================================================================

Window     *windows;				// 0-7 windows definitions
Window     *cw;						// Pointer to current active window
uint8_t     printedLines;			// For "More..." feature
bool        checkPrintedLines_inUse;

#ifndef DISABLE_SAVEAT
uint8_t     savedPosX;				// For SAVEAT/BACKAT
uint8_t     savedPosY;				//  "    "      "
#endif


// =============================================================================
// Text buffers and prompt state
// =============================================================================

char       *tmpMsg;					// TEXT_BUFFER_LEN, used by the print pipeline
char       *tmpTok;					// Token temp buffer (32 bytes), used by getToken
uint8_t     doingPrompt;			// True while a prompt is being rendered


// =============================================================================
// Transcript state
// =============================================================================

#ifdef TRANSCRIPT
uint16_t    trIdx;					// Index into transcript_buff
#endif
