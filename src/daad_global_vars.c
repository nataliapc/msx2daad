#include "daad.h"
#include "daad_condacts.h"


PROCstack procStack[NUM_PROCS];		// Stack of calls using PROCESS condact.
PROCstack *currProc;				// Pointer to current active condact.

bool indirection;					// True if the current condact use indirection for the first argument.
bool isDone;						// Variable for ISDONE/ISNOTDONE condacts.
bool lastPicShow;					// True if last location picture was drawed.
