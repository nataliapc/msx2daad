#include "condacts_stubs.h"

#pragma opt_code_size

static const char __THIS_FILE__[] = __FILE__;

#undef ASSERT
#undef ASSERT_EQUAL
#undef FAIL
#undef SUCCEED
#undef TODO
#define ASSERT(cond, failMsg)                   _ASSERT_TRUE(cond, failMsg, __THIS_FILE__, _func, __LINE__)
#define ASSERT_EQUAL(value, expected, failMsg)  _ASSERT_EQUAL((uint16_t)(value), (uint16_t)(expected), failMsg, __THIS_FILE__, _func, __LINE__)
#define FAIL(failMsg)                           _FAIL(failMsg, __THIS_FILE__, _func, __LINE__)
#define SUCCEED()                               _SUCCEED(__THIS_FILE__, _func)
#define TODO(infoMsg)                           _TODO(infoMsg, __THIS_FILE__, __func__)


// =============================================================================
// Actions for screen mode/format flags [3 condacts]
// =============================================================================

// =============================================================================
// Tests MODE <option>
/*	Allows the current window to have its operation flags changed. In order to
	calculate the number to use for the option just add the numbers shown next
	to each item to achieve the required bitmask combination:
		1 - Use the upper character set. (A permanent ^G)
		2 - SM32 ("More...") will not appear when the window fills.
	e.g. MODE 3 stops the 'More...' prompt and causes all to be translated to
	the 128-256 range. */

void test_MODE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window mode assigned to 0
	cw->mode = 0;

	//BDD when checking MODE 3
	static const char proc[] = { _MODE, 3, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->mode, 3, "New mode not is 3");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_MODE_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 3, current window mode assigned to 0
	cw->mode = 0;
	flags[75] = 3;

	//BDD when checking MODE @75
	static const char proc[] = { _MODE|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->mode, 3, "New mode not is 3");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests INPUT <stream> <option>
/*	The 'stream' parameter will set the bulk of input to come from the given
	window/stream. A value of 0 for 'stream' will not use the graphics stream
	as might be expected, but instead causes input to come from the current
	stream when the input occurs.
	Bitmask options:
		1 - Clear window after input.
		2 - Reprint input line in current stream when complete.
		4 - Reprint current text of input after a timeout. */

void test_INPUT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given fInStream w/value 0, and fTIFlags w/value 0b11000111
	flags[fInStream] = 0;
	flags[fTIFlags] = 0xc7;

	//BDD when checking INPUT 5 7
	static const char proc[] = { _INPUT, 5, 7, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fInStream], 5, "Flag fInStream is not 5");
	ASSERT_EQUAL(flags[fTIFlags], 0xff, "Flag fTIFlags is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_INPUT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 w/value 5, fInStream w/value 0, and fTIFlags w/value 0b11000111
	flags[75] = 5;
	flags[fInStream] = 0;
	flags[fTIFlags] = 0xc7;

	//BDD when checking INPUT 5 7
	static const char proc[] = { _INPUT|IND, 75, 7, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fInStream], 5, "Flag fInStream is not 5");
	ASSERT_EQUAL(flags[fTIFlags], 0xff, "Flag fTIFlags is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests TIME <duration> <option>
/*	Allows input to be set to 'timeout' after a specific duration in 1 second
	intervals, i.e. the Process 2 table will be called again if the player types
	nothing for the specified period. This action alters flags 48 & 49. 'option'
	allows this to also occur on ANYKEY and the "More..." prompt. In order to
	calculate the number to use for the option just add the numbers shown next to
	each item to achieve the required combination;
	    1 - While waiting for first character of Input only.
	    2 - While waiting for the key on the "More..." prompt.
	    4 - While waiting for the key on the ANYKEY action.
	e.g. TIME 5 6 (option = 2+4) will allow 5 seconds of inactivity on behalf of
	the player on input, ANYKEY or "More..." and between each key press. Whereas
	TIME 5 3 (option = 1+2) allows it only on the first character of input and on
	"More...".
	TIME 0 0 will stop timeouts (default). */

void test_TIME_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given fTime w/value 0, and fTIFlags w/value 0b11111000
	flags[fTime] = 0;
	flags[fTIFlags] = 0xf8;

	//BDD when checking TIME 10 7
	static const char proc[] = { _TIME, 10, 7, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fTime], 10, "Flag fTime is not 10");
	ASSERT_EQUAL(flags[fTIFlags], 0xff, "Flag fTIFlags is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_TIME_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 w/value 10, fTime w/value 0, and fTIFlags w/value 0b11111000
	flags[75] = 10;
	flags[fTime] = 0;
	flags[fTIFlags] = 0xf8;

	//BDD when checking TIME @75 7
	static const char proc[] = { _TIME|IND, 75, 7, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fTime], 10, "Flag fTime is not 10");
	ASSERT_EQUAL(flags[fTIFlags], 0xff, "Flag fTIFlags is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Actions for screen control & output [20 condacts]
// =============================================================================

// =============================================================================
// Tests WINDOW <window>
/*	Selects window (0-7) as current print output stream. */

void test_WINDOW_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current windows is 0
	cw = windows;

	//BDD when checking WINDOW 1
	static const char proc[] = { _WINDOW, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw, &windows[1], "Current window is not 1");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINDOW_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current windows is 0, and flag 75 w/value 1
	cw = windows;
	flags[75] = 1;

	//BDD when checking WINDOW @75
	static const char proc[] = { _WINDOW|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw, &windows[1], "Current window is not 1");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WINAT <line> <col>
/*	Sets current window to start at given line and column. Height and width to fit
	available screen. */

void test_WINAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window at (0,0)
	cw->winX = 0;
	cw->winY = 0;

	//BDD when checking WINAT 5 10
	static const char proc[] = { _WINAT, 5, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winX, 10, "Current window X is not 10");
	ASSERT_EQUAL(cw->winY, 5, "Current window Y is not 5");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINAT_oversize()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window at (10,10) w/size (85x25)
	cw->winX = 0;
	cw->winY = 0;
	cw->winW = 85;
	cw->winH = 25;

	//BDD when checking WINAT 10 10
	static const char proc[] = { _WINAT, 10, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winW, MAX_COLUMNS - cw->winX, "Current window W not match");
	ASSERT_EQUAL(cw->winH, MAX_LINES - cw->winY, "Current window H not match");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window at (0,0), and flag 75 w/value 5
	cw->winX = 0;
	cw->winY = 0;
	flags[75] = 5;

	//BDD when checking WINAT @75 10
	static const char proc[] = { _WINAT|IND, 75, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winX, 10, "Current window X is not 10");
	ASSERT_EQUAL(cw->winY, 5, "Current window Y is not 5");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WINSIZE <height> <width>
/*	Sets current window size to given height and width. Clipping needed to fit
	available screen. */

void test_WINSIZE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window size is (20x15)
	cw->winW = 20;
	cw->winH = 15;

	//BDD when checking WINSIZE 15 25
	static const char proc[] = { _WINSIZE, 15, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winW, 25, "Current window W is not 25");
	ASSERT_EQUAL(cw->winH, 15, "Current window H is not 15");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINSIZE_oversize()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window size is (20x15) and at (10,10)
	cw->winX = 10;
	cw->winY = 10;
	cw->winW = 20;
	cw->winH = 15;

	//BDD when checking WINSIZE 127 127
	static const char proc[] = { _WINSIZE, 127, 127, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winW, MAX_COLUMNS - cw->winX, "Current window W not match");
	ASSERT_EQUAL(cw->winH, MAX_LINES - cw->winY, "Current window H not match");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WINSIZE_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window size is (20x15), and flag 75 w/value 15
	cw->winW = 20;
	cw->winH = 15;
	flags[75] = 15;

	//BDD when checking WINSIZE @75 25
	static const char proc[] = { _WINSIZE|IND, 75, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winW, 25, "Current window W is not 25");
	ASSERT_EQUAL(cw->winH, 15, "Current window H is not 15");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CENTRE
/*	Will ensure the current window is centered for the current column width of the
	screen. (Does not affect line position). */

void test_CENTRE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current window at (1,1) w/size (20x15)
	cw->winX = 1;
	cw->winY = 1;
	cw->winW = 20;
	cw->winH = 15;

	//BDD when checking CENTRE
	static const char proc[] = { _CENTRE, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->winX, (MAX_COLUMNS - cw->winW)/2, "Current window W not match");
	ASSERT_EQUAL(cw->winY, 1, "Current window Y has changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CLS
/*	Clears the current window. */

void test_CLS_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a used current window
	cw->cursorX = 1;
	cw->cursorY = 2;
	cw->lastPicLocation = 20;
	lastPicShow = true;
	printedLines = 5;

	//BDD when checking CLS
	static const char proc[] = { _CLS, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 0, "Current window cursorX not reset");
	ASSERT_EQUAL(cw->cursorY, 0, "Current window cursorY not reset");
	ASSERT_EQUAL(cw->lastPicLocation, NO_LASTPICTURE, "Current window lastPicLocation not reset");
	ASSERT_EQUAL(lastPicShow, false, "lastPicShow not reset");
	ASSERT_EQUAL(printedLines, 0, "printedLines not reset");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SAVEAT
/*	Save and Restore print position for current window. This allows you to
	maintain the print position for example while printing elsewhere in the
	window. You should consider using a seperate window for most tasks. This
	may find use in the creation of a new input line or in animation
	sequences... */

void test_SAVEAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given not saved cursor pos, and cursor at (1,2)
	savedPosX = 0;
	savedPosY = 0;
	cw->cursorX = 1;
	cw->cursorY = 2;

	//BDD when checking SAVEAT
	static const char proc[] = { _SAVEAT, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(savedPosX, 1, "Saved pos X not 1");
	ASSERT_EQUAL(savedPosY, 2, "Saved pos Y not 2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests BACKAT

void test_BACKAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a saved cursor pos (1,2), and cursor at (0,0)
	savedPosX = 1;
	savedPosY = 2;
	cw->cursorX = 0;
	cw->cursorY = 0;

	//BDD when checking BACKAT
	static const char proc[] = { _BACKAT, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 1, "Restored pos X not 1");
	ASSERT_EQUAL(cw->cursorY, 2, "Restored pos Y not 2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PAPER <colour>
/*	Set paper colour acording to the lookup table given in the graphics editors */

void test_PAPER_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given colour 3
	static const char proc[] = { _PAPER, 3, 255 };
	do_action(proc);

	//BDD then gfxSetPaperCol(3) was called
	ASSERT_EQUAL(fake_lastPaperCol, 3, "PAPER must call gfxSetPaperCol with given colour");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests INK <colour>
/*	Set text colour acording to the lookup table given in the graphics editors */

void test_INK_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given colour 7
	static const char proc[] = { _INK, 7, 255 };
	do_action(proc);

	//BDD then gfxSetInkCol(7) was called
	ASSERT_EQUAL(fake_lastInkCol, 7, "INK must call gfxSetInkCol with given colour");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests BORDER <colour>
/*	Set border colour acording to the lookup table given in the graphics editors. */

void test_BORDER_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given colour 2
	static const char proc[] = { _BORDER, 2, 255 };
	do_action(proc);

	//BDD then gfxSetBorderCol(2) was called
	ASSERT_EQUAL(fake_lastBorderCol, 2, "BORDER must call gfxSetBorderCol with given colour");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PRINTAT <line> <col>
/*	Sets current print position to given point if in current window. If not then
	print position becomes top left of window. */

void test_PRINTAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given cursor at (0,0)
	cw->cursorX = 0;
	cw->cursorY = 0;

	//BDD when checking PRINTAT 10 5
	static const char proc[] = { _PRINTAT, 10, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 5, "Cursor X not 5");
	ASSERT_EQUAL(cw->cursorY, 10, "Cursor Y not 10");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PRINTAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given cursor at (0,0), and flag 75 w/value 10
	cw->cursorX = 0;
	cw->cursorY = 0;
	flags[75] = 10;

	//BDD when checking PRINTAT @75 5
	static const char proc[] = { _PRINTAT|IND, 75, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 5, "Cursor X not 5");
	ASSERT_EQUAL(cw->cursorY, 10, "Cursor Y not 10");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests TAB <col>
/*	Sets current print position to given column on current line. */

void test_TAB_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given cursor at (2,3)
	cw->cursorX = 2;
	cw->cursorY = 3;

	//BDD when checking TAB 10
	static const char proc[] = { _TAB, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 10, "Cursor X not 10");
	ASSERT_EQUAL(cw->cursorY, 3, "Cursor Y not 3");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_TAB_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given cursor at (2,3), and flag 75 w/value 10
	cw->cursorX = 2;
	cw->cursorY = 3;
	flags[75] = 10;

	//BDD when checking TAB @75
	static const char proc[] = { _TAB|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(cw->cursorX, 10, "Cursor X not 10");
	ASSERT_EQUAL(cw->cursorY, 3, "Cursor Y not 3");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SPACE
/*	Will simply print a space to the current output stream. Shorter than MES
	Space! */

void test_SPACE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD when SPACE is called
	static const char proc[] = { _SPACE, 255 };
	do_action(proc);

	//BDD then printChar(' ') was called
	ASSERT_EQUAL(fake_lastCharPrinted, ' ', "SPACE must call printChar(' ')");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NEWLINE
/*	Prints a carriage return/line feed. */

void test_NEWLINE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD when NEWLINE is called
	static const char proc[] = { _NEWLINE, 255 };
	do_action(proc);

	//BDD then printChar('\r') was called
	ASSERT_EQUAL(fake_lastCharPrinted, '\r', "NEWLINE must call printChar('\\r')");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests MES <mesno>
/*	Prints Message mesno. */

void test_MES_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given user message 10
	static const char proc[] = { _MES, 10, 255 };
	do_action(proc);

	//BDD then printUserMsg(10) was called
	ASSERT_EQUAL(fake_lastUserMsgPrinted, 10, "MES must call printUserMsg with given message number");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests MESSAGE <mesno>
/*	Prints Message mesno., then carries out a NEWLINE action. */

void test_MESSAGE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given user message 10
	//     MESSAGE = MES + NEWLINE: calls printUserMsg(10) then printChar('\r')
	static const char proc[] = { _MESSAGE, 10, 255 };
	do_action(proc);

	//BDD then both were called in order
	ASSERT_EQUAL(fake_lastUserMsgPrinted, 10, "MESSAGE must call printUserMsg with given message number");
	ASSERT_EQUAL(fake_lastCharPrinted, '\r', "MESSAGE must call NEWLINE after MES");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SYSMESS <sysno>
/*	Prints System Message sysno. */

void test_SYSMES_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given system message 5
	static const char proc[] = { _SYSMESS, 5, 255 };
	do_action(proc);

	//BDD then printSystemMsg(5) was called (captured by fake_lastSysMesPrinted)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 5, "SYSMESS must call printSystemMsg with given number");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DESC <locno>
/*	Prints the text for location locno. without a NEWLINE. */

void test_DESC_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given location 5
	static const char proc[] = { _DESC, 5, 255 };
	do_action(proc);

	//BDD then printLocationMsg(5) was called
	ASSERT_EQUAL(fake_lastLocMsgPrinted, 5, "DESC must call printLocationMsg with given location number");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PRINT <flagno>
/*	The decimal contents of Flag flagno. are displayed without leading or
	trailing spaces. */

void test_PRINT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 100 = 42
	flags[100] = 42;

	//BDD when PRINT 100
	static const char proc[] = { _PRINT, 100, 255 };
	do_action(proc);

	//BDD then printBase10(42) was called
	ASSERT_EQUAL(fake_lastBase10Printed, 42, "PRINT must call printBase10 with the value of the given flag");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DPRINT <flagno>
/*	Will print the contents of flagno and flagno+1 as a two byte number. */

void test_DPRINT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 100 = 0x34, flag 101 = 0x12 -> 16-bit value 0x1234 = 4660
	flags[100] = 0x34;
	flags[101] = 0x12;

	//BDD when DPRINT 100
	static const char proc[] = { _DPRINT, 100, 255 };
	do_action(proc);

	//BDD then printBase10(0x1234) was called
	ASSERT_EQUAL(fake_lastBase10Printed, 0x1234, "DPRINT must call printBase10 with 16-bit value (flagno | flagno+1<<8)");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Actions for listing objects [2 condacts]
// =============================================================================

// =============================================================================
// Tests LISTOBJ
/*	If any objects are present then SM1 ("I can also see:") is printed, followed
	by a list of all objects present at the current location.
	If there are no objects then nothing is printed. */

void test_LISTOBJ_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, obj1 and obj2 at loc 5
	flags[fPlayer] = 5;
	objects[1].location = 5;
	objects[1].nounId = 10;
	objects[2].location = 5;
	objects[2].nounId = 11;

	//BDD when LISTOBJ
	static const char proc[] = { _LISTOBJ, 255 };
	do_action(proc);

	//BDD then F53_LISTED is set (objects were found at player location)
	ASSERT(flags[fOFlags] & F53_LISTED, "LISTOBJ must set F53_LISTED when objects present at player location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_LISTOBJ_none()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, no objects at loc 5
	//     (beforeEach memsets all objects to location=0; player at 5)
	flags[fPlayer] = 5;

	//BDD when LISTOBJ
	static const char proc[] = { _LISTOBJ, 255 };
	do_action(proc);

	//BDD then F53_LISTED is NOT set (no objects at player location)
	ASSERT(!(flags[fOFlags] & F53_LISTED), "LISTOBJ must not set F53_LISTED when no objects at player location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests LISTAT <locno+>
/*	If any objects are present then they are listed. Otherwise SM53 ("nothing.")
	is printed - note that you will usually have to precede this action with a
	message along the lines of "In the bag is:" etc. */

void test_LISTAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 at loc 3
	objects[1].location = 3;
	objects[1].nounId = 10;

	//BDD when LISTAT 3
	static const char proc[] = { _LISTAT, 3, 255 };
	do_action(proc);

	//BDD then F53_LISTED is set (objects found at given location)
	//     Note: do_LISTAT prints SM51 on success, but the stub filters SM51
	//     (if num!=51), so fake_lastSysMesPrinted does NOT capture it.
	ASSERT(flags[fOFlags] & F53_LISTED, "LISTAT must set F53_LISTED when objects present at given location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_LISTAT_none()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given no objects at loc 7 (all objects at location=0 from memset)
	//BDD when LISTAT 7
	static const char proc[] = { _LISTAT, 7, 255 };
	do_action(proc);

	//BDD then F53_LISTED NOT set, and SM53 ("Nada.") was printed
	ASSERT(!(flags[fOFlags] & F53_LISTED), "LISTAT must not set F53_LISTED when no objects at given location");
	ASSERT_EQUAL(fake_lastSysMesPrinted, 53, "LISTAT must print SM53 when no objects found");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}


// =============================================================================
// PRP010 — Edge case tests for UI condacts

// PAPER/INK/BORDER: indirection resolves colour from flag
void test_PAPER_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 = 5
	flags[75] = 5;

	//BDD when PAPER @75
	static const char proc[] = { _PAPER|IND, 75, 255 };
	do_action(proc);

	//BDD then gfxSetPaperCol(5) was called via indirection
	ASSERT_EQUAL(fake_lastPaperCol, 5, "PAPER @flag must resolve colour from flag value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_INK_indirection()
{
	const char *_func = __func__;
	beforeEach();

	flags[75] = 2;
	static const char proc[] = { _INK|IND, 75, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastInkCol, 2, "INK @flag must resolve colour from flag value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_BORDER_indirection()
{
	const char *_func = __func__;
	beforeEach();

	flags[75] = 4;
	static const char proc[] = { _BORDER|IND, 75, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastBorderCol, 4, "BORDER @flag must resolve colour from flag value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// MES: indirection resolves message number from flag
void test_MES_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 80 = 15
	flags[80] = 15;

	//BDD when MES @80
	static const char proc[] = { _MES|IND, 80, 255 };
	do_action(proc);

	//BDD then printUserMsg(15) was called via indirection
	ASSERT_EQUAL(fake_lastUserMsgPrinted, 15, "MES @flag must resolve message number from flag value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// SYSMESS: indirection resolves system message number from flag
void test_SYSMES_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 80 = 8
	flags[80] = 8;

	//BDD when SYSMESS @80
	static const char proc[] = { _SYSMESS|IND, 80, 255 };
	do_action(proc);

	//BDD then printSystemMsg(8) was called via indirection
	ASSERT_EQUAL(fake_lastSysMesPrinted, 8, "SYSMESS @flag must resolve system message number from flag value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// SYSMESS with SM51: the stub filters SM51 by design — must not be captured
void test_SYSMES_sm51_not_captured()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given fake_lastSysMesPrinted = -1 (reset by beforeEach)
	//BDD when SYSMESS 51 (SM51 = "." sentence terminator, filtered by stub)
	static const char proc[] = { _SYSMESS, 51, 255 };
	do_action(proc);

	//BDD then fake_lastSysMesPrinted is still -1 (SM51 filtered: if (num!=51))
	//     This documents the stub's intentional filter to avoid SM51 overwriting
	//     the meaningful SM in condact sequences like PUTIN/TAKEOUT/ISAT.
	ASSERT_EQUAL(fake_lastSysMesPrinted, -1, "SYSMESS 51 must NOT update fake_lastSysMesPrinted (stub filters SM51)");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// DESC: indirection resolves location number from flag
void test_DESC_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 80 = 3
	flags[80] = 3;

	//BDD when DESC @80
	static const char proc[] = { _DESC|IND, 80, 255 };
	do_action(proc);

	//BDD then printLocationMsg(3) was called via indirection
	ASSERT_EQUAL(fake_lastLocMsgPrinted, 3, "DESC @flag must resolve location number from flag value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// PRINT: indirection resolves flag NUMBER from another flag, then prints that flag's VALUE
void test_PRINT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 80 = 100, flag 100 = 42
	//     PRINT @80 -> reads flags[80]=100 -> prints flags[100]=42
	flags[80]  = 100;
	flags[100] = 42;

	//BDD when PRINT @80
	static const char proc[] = { _PRINT|IND, 80, 255 };
	do_action(proc);

	//BDD then printBase10(42) was called
	ASSERT_EQUAL(fake_lastBase10Printed, 42, "PRINT @flag must resolve flagno from flag, then print its value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// PRINT: flag value 0 (boundary)
void test_PRINT_zero()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 100 = 0
	flags[100] = 0;

	static const char proc[] = { _PRINT, 100, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastBase10Printed, 0, "PRINT must print 0 when flag value is 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// DPRINT: indirection + 16-bit max value (0xFFFF)
void test_DPRINT_max_value()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 100 = 0xFF (low byte), flag 101 = 0xFF (high byte) -> 0xFFFF
	flags[100] = 0xFF;
	flags[101] = 0xFF;

	static const char proc[] = { _DPRINT, 100, 255 };
	do_action(proc);

	//BDD then printBase10(65535) was called
	ASSERT_EQUAL(fake_lastBase10Printed, 0xFFFF, "DPRINT must handle maximum 16-bit value (0xFFFF)");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// LISTOBJ: carried objects at player location are NOT listed
// LISTOBJ calls _internal_listat(fPlayer, true) which matches objects[i].location == fPlayer.
// LOC_CARRIED (254) != player location (5), so carried objects are not shown.
void test_LISTOBJ_carried_not_shown()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, obj1 carried (not at location 5)
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 10;

	//BDD when LISTOBJ
	static const char proc[] = { _LISTOBJ, 255 };
	do_action(proc);

	//BDD then F53_LISTED is NOT set (carried objects are not at the current location)
	ASSERT(!(flags[fOFlags] & F53_LISTED), "LISTOBJ must not list carried objects (LOC_CARRIED != player location)");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// LISTOBJ: worn objects at player location are NOT listed
void test_LISTOBJ_worn_not_shown()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, obj1 worn
	flags[fPlayer] = 5;
	objects[1].location = LOC_WORN;
	objects[1].nounId = 10;

	static const char proc[] = { _LISTOBJ, 255 };
	do_action(proc);

	//BDD then F53_LISTED NOT set (worn objects are not at the current location)
	ASSERT(!(flags[fOFlags] & F53_LISTED), "LISTOBJ must not list worn objects (LOC_WORN != player location)");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// LISTOBJ: SM48 (".\n") is printed as terminator after listing objects
void test_LISTOBJ_sm48_terminator()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, obj1 at loc 5
	flags[fPlayer] = 5;
	objects[1].location = 5;
	objects[1].nounId = 10;

	static const char proc[] = { _LISTOBJ, 255 };
	do_action(proc);

	//BDD then SM48 (".\n") is the last system message printed as list terminator
	//     Order: SM1 ("I can also see:") -> object name -> SM48 (".\n")
	//     SM48 != 51 so it IS captured by fake_lastSysMesPrinted
	ASSERT_EQUAL(fake_lastSysMesPrinted, 48, "LISTOBJ must print SM48 as list terminator after objects");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// LISTOBJ: bit 6 = 0 (default) → newline between objects, not SM46/SM47
void test_LISTOBJ_contlist_off()
{
	const char *_func = __func__;
	beforeEach();

	flags[fPlayer] = 3;
	flags[fOFlags] = 0;                        // bit 6 = 0 → newline mode
	objects[0].location = 3; objects[0].nounId = 10;
	objects[1].location = 3; objects[1].nounId = 11;

	static const char proc[] = { _LISTOBJ, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastCharPrinted, '\r',
		"LISTOBJ with bit6=0 must use do_NEWLINE between objects");
	SUCCEED();
}

// LISTOBJ: bit 6 = 1 → continuous mode, SM46/SM47 separators, no NEWLINE
void test_LISTOBJ_contlist_on()
{
	const char *_func = __func__;
	beforeEach();

	flags[fPlayer] = 3;
	flags[fOFlags] = 64;                       // bit 6 = 1 → continuous mode
	objects[0].location = 3; objects[0].nounId = 10;
	objects[1].location = 3; objects[1].nounId = 11;

	static const char proc[] = { _LISTOBJ, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastCharPrinted, -1,
		"LISTOBJ with bit6=1 must NOT use do_NEWLINE (uses SM separators)");
	SUCCEED();
}

// LISTAT with LOC_CARRIED: lists objects in player's inventory
// Common game pattern: LISTAT 254 to show carried objects
void test_LISTAT_loc_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 and obj2 carried (location == LOC_CARRIED == 254)
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 10;
	objects[2].location = LOC_CARRIED;
	objects[2].nounId = 11;

	//BDD when LISTAT LOC_CARRIED (254)
	static const char proc[] = { _LISTAT, LOC_CARRIED, 255 };
	do_action(proc);

	//BDD then F53_LISTED is set (carried objects found at location LOC_CARRIED)
	ASSERT(flags[fOFlags] & F53_LISTED, "LISTAT LOC_CARRIED must list carried objects");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS CONDACTS (5/6) ###\n\r### (Window + display) ###\n\r");
	beforeAll();

	test_MODE_success(); test_MODE_indirection();
	test_INPUT_success(); test_INPUT_indirection();
	test_TIME_success(); test_TIME_indirection();

	test_WINDOW_success(); test_WINDOW_indirection();
	test_WINAT_success(); test_WINAT_oversize(); test_WINAT_indirection();
	test_WINSIZE_success(); test_WINSIZE_oversize(); test_WINSIZE_indirection();
	test_CENTRE_success();
	test_CLS_success();
	test_SAVEAT_success();
	test_BACKAT_success();
	test_PAPER_success(); test_PAPER_indirection();
	test_INK_success(); test_INK_indirection();
	test_BORDER_success(); test_BORDER_indirection();
	test_PRINTAT_success(); test_PRINTAT_indirection();
	test_TAB_success(); test_TAB_indirection();
	test_SPACE_success();
	test_NEWLINE_success();
	test_MES_success(); test_MES_indirection();
	test_MESSAGE_success();
	test_SYSMES_success(); test_SYSMES_indirection(); test_SYSMES_sm51_not_captured();
	test_DESC_success(); test_DESC_indirection();
	test_PRINT_success(); test_PRINT_indirection(); test_PRINT_zero();
	test_DPRINT_success(); test_DPRINT_max_value();

	test_LISTOBJ_success(); test_LISTOBJ_none();
	test_LISTOBJ_carried_not_shown(); test_LISTOBJ_worn_not_shown();
	test_LISTOBJ_sm48_terminator();
	test_LISTOBJ_contlist_off(); test_LISTOBJ_contlist_on();
	test_LISTAT_success(); test_LISTAT_none(); test_LISTAT_loc_carried();

	return 0;
}
