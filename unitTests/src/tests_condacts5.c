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
	TODO(TODO_UI);
}

// =============================================================================
// Tests INK <colour>
/*	Set text colour acording to the lookup table given in the graphics editors */

void test_INK_success()
{
	TODO(TODO_UI);
}

// =============================================================================
// Tests BORDER <colour>
/*	Set border colour acording to the lookup table given in the graphics editors. */

void test_BORDER_success()
{
	TODO(TODO_UI);
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
	TODO(TODO_UI);
}

// =============================================================================
// Tests NEWLINE
/*	Prints a carriage return/line feed. */

void test_NEWLINE_success()
{
	TODO(TODO_UI);
}

// =============================================================================
// Tests MES <mesno>
/*	Prints Message mesno. */

void test_MES_success()
{
	TODO(TODO_UI);
}

// =============================================================================
// Tests MESSAGE <mesno>
/*	Prints Message mesno., then carries out a NEWLINE action. */

void test_MESSAGE_success()
{
	TODO(TODO_UI);
}

// =============================================================================
// Tests SYSMESS <sysno>
/*	Prints System Message sysno. */

void test_SYSMES_success()
{
	TODO(TODO_UI);
}

// =============================================================================
// Tests DESC <locno>
/*	Prints the text for location locno. without a NEWLINE. */

void test_DESC_success()
{
	TODO(TODO_UI);
}

// =============================================================================
// Tests PRINT <flagno>
/*	The decimal contents of Flag flagno. are displayed without leading or
	trailing spaces. */

void test_PRINT_success()
{
	TODO(TODO_UI);
}

// =============================================================================
// Tests DPRINT <flagno>
/*	Will print the contents of flagno and flagno+1 as a two byte number. */

void test_DPRINT_success()
{
	TODO(TODO_UI);
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
	TODO(TODO_UI);
}

void test_LISTOBJ_none()
{
	TODO(TODO_UI);
}

// =============================================================================
// Tests LISTAT <locno+>
/*	If any objects are present then they are listed. Otherwise SM53 ("nothing.")
	is printed - note that you will usually have to precede this action with a
	message along the lines of "In the bag is:" etc. */

void test_LISTAT_success()
{
	TODO(TODO_UI);
}

void test_LISTAT_none()
{
	TODO(TODO_UI);
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
	test_PAPER_success();
	test_INK_success();
	test_BORDER_success();
	test_PRINTAT_success(); test_PRINTAT_indirection();
	test_TAB_success(); test_TAB_indirection();
	test_SPACE_success();
	test_NEWLINE_success();
	test_MES_success();
	test_MESSAGE_success();
	test_SYSMES_success();
	test_DESC_success();
	test_PRINT_success();
	test_DPRINT_success();

	test_LISTOBJ_success(); test_LISTOBJ_none();
	test_LISTAT_success(); test_LISTAT_none();

	return 0;
}
