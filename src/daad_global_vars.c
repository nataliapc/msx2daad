#include "daad.h"
#include "daad_condacts.h"


PROCstack procStack[NUM_PROCS];		// Stack of calls using PROCESS condact.
PROCstack *currProc;				// Pointer to current active condact.

bool indirection;					// True if the current condact use indirection for the first argument.
bool checkEntry;					// Boolean to check if a Process entry must continue or a condition fails.
bool isDone, lastIsDone;			// Variables for ISDONE/ISNOTDONE condacts.
bool lastPicShow;					// True if last location picture was drawed.
