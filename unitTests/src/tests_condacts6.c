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
// Actions for current game state save/load [4 condacts]
// =============================================================================

// =============================================================================
// Tests SAVE <opt>
/*	This action saves the current game position on disc or tape. SM60 ("Type in
	name of file.") is printed and the input routine is called to get the filename
	from the player. If the supplied filename is not acceptable SM59 ("File name
	error.") is printed - this is not checked on 8 bit machines, the file name
	is MADE acceptable! */

// =============================================================================
// Tests LOAD <opt>
/*	This action loads a game position from disc or tape. A filename is obtained
	in the same way as for SAVE. A variety of errors may appear on each machine
	if the file is not found or suffers a load error. Usually 'I/O Error'. The
	next action is carried out only if the load is successful. Otherwise a system
	clear, GOTO 0, RESTART is carried out. */

void test_SAVE_LOAD_success()
{
	const char *_func = __func__;
	uint16_t i;
	beforeEach();

	//BDD given flags and objects
	for (i=0; i<256; i++) flags[i] = i;
	for (i=0; i<MOCK_NUM_OBJECTS; i++) objects[i].location = i;

	//BDD when checking SAVE
	static const char proc[] = { _SAVE, 255 };
	do_action(proc);
	//BDD and cleaning data
	for (i=0; i<256; i++) flags[i] = 0;
	for (i=0; i<MOCK_NUM_OBJECTS; i++) objects[i].location = 0;
	//BDD and checking LOAD
	static const char proc2[] = { _LOAD, 255 };
	do_action(proc2);

	//BDD then success
	for (i=0; i<256; i++) ASSERT_EQUAL(i, flags[i], "Incorrect flag");
	for (i=0; i<MOCK_NUM_OBJECTS; i++) ASSERT_EQUAL(i, objects[i].location, "Incorrect obj loc");
	SUCCEED();
}

// =============================================================================
// Tests RAMSAVE
/*	In a similar way to SAVE this action saves all the information relevant to
	the game in progress not onto disc but into a memory buffer. This buffer is
	of course volatile and will be destroyed when the machine is turned off
	which should be made clear to the player. The next action is always carried
	out. */

void test_RAMSAVE_success()
{
	const char *_func = __func__;
	uint16_t i;
	beforeEach();

	//BDD given flags and objects
	for (i=0; i<256; i++) flags[i] = i;
	for (i=0; i<MOCK_NUM_OBJECTS; i++) objects[i].location = i;

	//BDD when checking RAMSAVE
	static const char proc[] = { _RAMSAVE, 255 };
	do_action(proc);

	//BDD then success
	for (i=0; i<256; i++) ASSERT_EQUAL(flags[i], ramsave[i], "Incorrect flag");
	for (i=0; i<MOCK_NUM_OBJECTS; i++) ASSERT_EQUAL(objects[i].location, ramsave[256+i], "Incorrect obj loc");
	SUCCEED();
}

// Regression test for PRP003 (do_RAMSAVE hardcoded 256-object loop).
// The old buggy version iterated 256 objects regardless of numObjDsc, writing
// bytes read from past the objects[] array into ramsave[256+numObjDsc..511].
// Place a canary in that trailing region and verify RAMSAVE leaves it intact.
void test_RAMSAVE_no_overflow()
{
	const char *_func = __func__;
	uint16_t i;
	beforeEach();

	//BDD given a canary in the unused tail of ramsave
	for (i=256+MOCK_NUM_OBJECTS; i<512; i++) ramsave[i] = 0xAA;
	for (i=0; i<MOCK_NUM_OBJECTS; i++) objects[i].location = i;

	//BDD when executing RAMSAVE
	static const char proc[] = { _RAMSAVE, 255 };
	do_action(proc);

	//BDD then the canary is untouched
	for (i=256+MOCK_NUM_OBJECTS; i<512; i++)
		ASSERT_EQUAL(ramsave[i], 0xAA, "RAMSAVE wrote past numObjDsc");
	SUCCEED();
}

// =============================================================================
// Tests RAMLOAD <flagno>
/*	This action is the counterpart of RAMSAVE and allows the saved buffer to be
	restored. The parameter specifies the last flag to be reloaded which can be
	used to preserve values over a restore.
	Note 1: The RAM actions could be used to implement an OOPS command that is
	common on other systems to take back the previous move; by creating an entry
	in the main loop which does an automatic RAMSAVE every time the player enters
	a move.
	Note 2: These four actions allow the next Condact to be carried out. They
	should normally always be followed by a RESTART or describe in order that
	the game state is restored to an identical position. */

void test_RAMLOAD_success()
{
	const char *_func = __func__;
	uint16_t i;
	beforeEach();

	//BDD given flags and objects
	for (i=0; i<512; i++) ramsave[i] = i % 256;

	//BDD when checking RAMLOAD
	static const char proc[] = { _RAMLOAD, 255 };
	do_action(proc);

	//BDD then success
	for (i=0; i<256; i++) ASSERT_EQUAL(i, flags[i], "Incorrect flag");
	for (i=0; i<MOCK_NUM_OBJECTS; i++) ASSERT_EQUAL(i, objects[i].location, "Incorrect obj loc");
	SUCCEED();
}

// =============================================================================
// Actions to pause game [2 condacts]
// =============================================================================

// =============================================================================
// Tests ANYKEY
/*	SM16 ("Press any key to continue") is printed and the keyboard is scanned until
	a key is pressed or until the timeout duration has elapsed if enabled. */

void test_ANYKEY_success()
{
	const char *_func = __func__;
	beforeEach();
	printedLines = 5;

	static const char proc[] = { _ANYKEY, 255 };
	do_action(proc);

	//BDD then SM16 ("Press any key") is printed and printedLines reset to 0
	ASSERT_EQUAL(fake_lastSysMesPrinted, 16, ERROR_SYSMES);
	ASSERT_EQUAL(printedLines, 0, "ANYKEY must reset printedLines to 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PAUSE <value>
/*	Pauses for value/50 secs. However, if value is zero then the pause is for
	256/50 secs. */

void test_PAUSE_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions to control the parse [3 condacts]
// =============================================================================

// =============================================================================
// Tests PARSE <n>
/*	The parameter 'n' controls which level of string indentation is to be
	searched. At the moment only two are supported by the interpreters so only
	the values 0 and 1 are valid.
		0 - Parse the main input line for the next LS.
		1 - Parse any string (phrase enclosed in quotes [""]) that was contained
		    in the last LS extracted. */

void test_PARSE_success()
{
	const char *_func = __func__;
	beforeEach();
	isDone = true;

	//BDD when PARSE 0: getLogicalSentence() (stub) returns false -> checkEntry = !false = true; isDone = false
	static const char proc[] = { _PARSE, 0, 255 };
	do_action(proc);

	ASSERT(checkEntry, "PARSE 0: checkEntry must be true when no sentence found");
	ASSERT(!isDone, "PARSE must reset isDone to false");
	SUCCEED();
}

// =============================================================================
// Tests NEWTEXT
/*	Forces the loss of any remaining phrases on the current input line. You
	would use this to prevent the player continuing without a fresh input
	should something go badly for his situation. e.g. the GET action carries
	out a NEWTEXT if it fails to get the required object for any reason, to
	prevent disaster with a sentence such as:
		GET SWORD AND KILL ORC WITH IT
	as attacking the ORC without the sword may be dangerous! */

void test_NEWTEXT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD when NEWTEXT: must call clearLogicalSentences(); checkEntry unchanged (true)
	static const char proc[] = { _NEWTEXT, 255 };
	do_action(proc);

	ASSERT(checkEntry, "NEWTEXT must not affect checkEntry");
	ASSERT_EQUAL(fake_clearLogicalSentences_calls, 1, "NEWTEXT must call clearLogicalSentences exactly once");
	SUCCEED();
}

// =============================================================================
// Tests SYNONYM
/*	Substitutes the given verb and noun in the LS. Nullword (Usually '_') can be
	used to suppress substitution for one or the other - or both I suppose! e.g.
	        MATCH    ON         SYNONYM LIGHT MATCH
	        STRIKE   MATCH      SYNONYM LIGHT _
	        LIGHT    MATCH      ....                 ; Actions...
	will switch the LS into a standard format for several different entries.
	Allowing only one to deal with the actual actions. */

void test_SYNONYM_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flags and objects
	flags[fVerb] = 1;
	flags[fNoun1] = 2;

	//BDD when checking RAMLOAD
	static const char proc[] = { _SYNONYM, 3, 4, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fVerb], 3, ERROR_FLAG);
	ASSERT_EQUAL(flags[fNoun1], 4, ERROR_FLAG);
	ASSERT_EQUAL(isDone, true, "SYNONYM must mark DONE in V2");
	SUCCEED();
}

void test_SYNONYM_success_verb()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flags and objects
	flags[fVerb] = 1;
	flags[fNoun1] = 2;

	//BDD when checking RAMLOAD
	static const char proc[] = { _SYNONYM, NULLWORD, 4, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fVerb], 1, ERROR_FLAG);
	ASSERT_EQUAL(flags[fNoun1], 4, ERROR_FLAG);
	ASSERT_EQUAL(isDone, true, "SYNONYM must mark DONE in V2");
	SUCCEED();
}

void test_SYNONYM_success_noun()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flags and objects
	flags[fVerb] = 1;
	flags[fNoun1] = 2;

	//BDD when checking RAMLOAD
	static const char proc[] = { _SYNONYM, 3, NULLWORD, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fVerb], 3, ERROR_FLAG);
	ASSERT_EQUAL(flags[fNoun1], 2, ERROR_FLAG);
	ASSERT_EQUAL(isDone, true, "SYNONYM must mark DONE in V2");
	SUCCEED();
}

// =============================================================================
// Actions for flow control [7 condacts]
// =============================================================================

// =============================================================================
// Tests PROCESS
/*	This powerful action transfers the attention of DAAD to the specified Process
	table number. Note that it is a true subroutine call and any exit from the
	new table (e.g. DONE, OK etc) will return control to the condact which follows
	the calling PROCESS action. A sub-process can call (nest) further process' to
	a depth of 10 at which point a run time error will be generated. */

void test_PROCESS_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD when PROCESS 0: pushPROC(0) advances PROC stack; checkEntry=false, isDone=false
	static const char proc[] = { _PROCESS, 0, 255 };
	do_action(proc);

	ASSERT(!checkEntry, "PROCESS must set checkEntry=false (pushPROC behaviour)");
	ASSERT(!isDone, "PROCESS must set isDone=false");
	SUCCEED();
}

// =============================================================================
// Tests REDO
/*	Will restart the currently executing table, allowing...
	TODO:incomplete descripcion in documentation */

void test_REDO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD when REDO: resets currProc->entry to entryIni and recalculates condact pointer
	// With test stubs, this reassigns pointers into ddb buffer without crashing.
	static const char proc[] = { _REDO, 255 };
	do_action(proc);

	//BDD then checkEntry stays true (REDO does not change it)
	ASSERT(checkEntry, "REDO must not affect checkEntry");
	SUCCEED();
}

// =============================================================================
// Tests DOALL
/*	Another powerful action which allows the implementation 'ALL' type command.

	1 - An attempt is made to find an object at Location locno.
	    If this is unsuccessful the DOALL is cancelled and action DONE is performed.
	2 - The object number is converted into the LS Noun1 (and Adjective1 if present)
	    by reference to the object definition section. If Noun(Adjective)1 matches
	    Noun(Adjective)2 then a return is made to step 1. This implements the "Verb
	    ALL EXCEPT object" facility of the parser.
	3 - The next condact and/or entry in the table is then considered. This
	    effectively converts a phrase of "Verb All" into "Verb object" which is
	    then processed by the table as if the player had typed it in.
	4 - When an attempt is made to exit the current table, if the DOALL is still
	    active (i.e. has not been cancelled by an action) then the attention of
	    DAAD is returned to the DOALL as from step 1; with the object search
	    continuing from the next highest object number to that just considered.

	The main ramification of the search method through the object definition
	section is; objects which have the Same Noun(Adjective) description (where the
	game works out which object is referred to by its presence) must be checked for
	in ascending order of object number, or one of them may be missed.
	Use the of DOALL to implement things like OPEN ALL must account for fact that
	doors are often flags only and would have to bemade into objects if they were to
	be included in a DOALL. */

void test_DOALL_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests SKIP
/*	Skip a distance of -128 to 128, or to the specified label. Will move the
	current entry in a table back or fore. 0 means next entry (so is meaningless).
	-1 means restart current entry (Dangerous). */

void test_SKIP_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD when SKIP 1: stepPROCEntryCondacts(1) advances entry by 1 and sets checkEntry=false
	static const char proc[] = { _SKIP, 1, 255 };
	do_action(proc);

	ASSERT(!checkEntry, "SKIP must set checkEntry=false");
	SUCCEED();
}

// =============================================================================
// Tests RESTART
/*	Will cancel any DOALL loop, any sub-process calls and make a jump
	to execute process 0 again from the start.*/

void test_RESTART_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD when RESTART: cancels DOALL, pops all PROCs, pushes proc 0, sets checkEntry=false.
	//                  Note: RESTART does NOT clear logical sentences (END does).
	static const char proc[] = { _RESTART, 255 };
	do_action(proc);

	ASSERT(!checkEntry, "RESTART must set checkEntry=false");
	ASSERT_EQUAL(fake_clearLogicalSentences_calls, 0, "RESTART must NOT clear logical sentences (only END/RESTART caller does)");
	SUCCEED();
}

// =============================================================================
// Tests END
/*	SM13 ("Would you like to play again?") is printed and the input routine called.
	Any DOALL loop and sub-process calls are cancelled. If the reply does not start
	with the first character of SM31 a jump is made to Initialise.
	Otherwise the player is returned to the operating system - by doing the command
	EXIT 0.*/

void test_END_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests EXIT <value>
/*	If value is 0 then will return directly to the operating system.
	reset ensure you use your PART number as the non zero value! */

void test_EXIT_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions for exit tables [3 condacts]
// =============================================================================

// =============================================================================
// Tests DONE
/*	This action jumps to the end of the process table and flags to DAAD that an
	action has been carried out. i.e. no more condacts or entries are considered.
	A return will thus be made to the previous calling process table, or to the
	start point of any active DOALL loop. */

void test_DONE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given no DOALL active and isDone false
	isDone = false;
	currProc->condactDOALL = NULL;

	//BDD when executing DONE
	static const char proc[] = { _DONE, 255 };
	do_action(proc);

	//BDD then isDone is true (signals the table ran at least one action)
	ASSERT(isDone, ERROR_ISDONE);
	SUCCEED();
}

// =============================================================================
// Tests NOTDONE
/*	This action jumps to the end of the process table and flags PAW that #no#
	action has been carried out. i.e. no more condacts or entries are considered.
	A return will thus be made to the previous calling process table or to the
	start point of any active DOALL loop. This will cause PAW to print one of the
	"I can't" messages if needed. i.e. if no other action is carried out and no
	entry is present in the connections section for the current Verb. */

void test_NOTDONE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given no DOALL active and isDone pre-set to true
	isDone = true;
	currProc->condactDOALL = NULL;

	//BDD when executing NOTDONE
	static const char proc[] = { _NOTDONE, 255 };
	do_action(proc);

	//BDD then isDone is reset to false
	ASSERT(!isDone, ERROR_ISDONE);
	SUCCEED();
}

// =============================================================================
// Tests OK
/*	SM15 ("OK") is printed and action DONE is performed. */

void test_OK_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given no DOALL active and isDone false
	isDone = false;
	currProc->condactDOALL = NULL;
	fake_lastSysMesPrinted = -1;

	//BDD when executing OK
	static const char proc[] = { _OK, 255 };
	do_action(proc);

	//BDD then SM15 ("OK") is printed and isDone becomes true (OK == DONE after printing)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 15, ERROR_SYSMES);
	ASSERT(isDone, ERROR_ISDONE);
	SUCCEED();
}

// =============================================================================
// Actions to call external routines [4 condacts]
// =============================================================================

// =============================================================================
// Tests EXTERN <value> <routine>
/*	Calls external routine with parameter value. The address is set by linking
	the #extern pre-compiler command */

void test_EXTERN_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests CALL <address(dword)>
/*	Allows 'address' in memory (or in the database segment for 16bit) to be
	executed. See the extern secion for more details. */

void test_CALL_success()
{
	TODO("CALL Not Implemented");
}

// =============================================================================
// Tests SFX <value1> <value2>
//       SFX <pa> <routine>
/*	This is a second EXTERN type action designed for Sound Effects extensions.
	e.g. It has a 'default' function which allows value 'value1' to be written
	to register 'value2' of the sound chip on 8 bit machines. This can be
	changed with #sfx or through linking - see the machine details and extern
	section for specifics. */

void test_SFX_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests GFX <pa> <routine>
/*	An EXTERN which is meant to deal with any graphics extensions to DAAD. On
	16bit it is used to implement the screen switching facilities. This can be
	changed with #gfx or through linking. See the machine details and extern
	section for specifics.

	GFX pa routine

	where routine can be:
       0*   Back->Phys
       1*   Phys->Back
       2*   SWAP (Phys<>Back) (In CGA this is a bit rough...)
       3*   Graphics Write to Phys
       4*   Graphics Write to Back
       5*   Clear Phys
       6*   Clear Back
       7    Text Write to Phys      -ST only
       8    Text Write to Back      -ST only
       9*   Set Palette value (Value is offset of 4 flag data block containing
   	    Num,Red,Green,Blue. RGB values are 0-255
      10    Read Palette value (Value is offset of 4 flag data block)

	* = supported by MSX2 interpreter. */

// GFX_SET_PALETTE updates the internal color table (SC8: GGGRRRBB)
// Ref: docs/GFX.md lines 9-27; plan/PRP020_GFX_Palette.md
void test_GFX_SET_PALETTE()
{
	const char *_func = __func__;
	beforeEach();

	// given: flags[20]=idx=5, R=192, G=96, B=128
	flags[20] = 5;
	flags[21] = 192;  // Red
	flags[22] = 96;   // Green
	flags[23] = 128;  // Blue

	static const char proc[] = { _GFX, 20, GFX_SET_PALETTE, 255 };
	do_action(proc);

	// then: test_colorTranslationSC8[5] = (G & 0xE0) | ((R & 0xE0)>>3) | ((B & 0xE0)>>6)
	//   G=96 & 0xE0=0x60, R=192 & 0xE0=0xC0 >>3=0x18, B=128 & 0xE0=0x80 >>6=0x02
	//   expected: 0x60|0x18|0x02 = 0x7A
	ASSERT_EQUAL(test_colorTranslationSC8[5], 0x7A, "SET_PALETTE: wrong GRB332 value in color table");
	SUCCEED();
}

// GFX_GET_PALETTE reads the internal color table and fills flags[v+1..v+3] with R,G,B
// Ref: docs/GFX.md lines 9-27; plan/PRP020_GFX_Palette.md
void test_GFX_GET_PALETTE()
{
	const char *_func = __func__;
	beforeEach();

	// given: color 10 = 0xED in SC8 default table (GRB332=0b11101101 → G=0xE0,R=0x60,B=0x40)
	flags[30] = 10;

	static const char proc[] = { _GFX, 30, GFX_GET_PALETTE, 255 };
	do_action(proc);

	// then: flags[31..33] = R=0x60, G=0xE0, B=0x40 (all non-zero, all different)
	ASSERT_EQUAL(flags[31], 0x60, "GET_PALETTE idx=10: R must be 0x60");
	ASSERT_EQUAL(flags[32], 0xE0, "GET_PALETTE idx=10: G must be 0xE0");
	ASSERT_EQUAL(flags[33], 0x40, "GET_PALETTE idx=10: B must be 0x40");
	SUCCEED();
}

// SET then GET round-trip; SC8 truncates R/G to 3 bits (mask 0xE0) and B to 2 bits (mask 0xC0)
// Ref: plan/PRP020_GFX_Palette.md §Tests
void test_GFX_SET_GET_PALETTE_roundtrip()
{
	const char *_func = __func__;
	beforeEach();

	// given: SET color 3 to R=200, G=100, B=150
	flags[40] = 3;
	flags[41] = 200;
	flags[42] = 100;
	flags[43] = 150;

	static const char proc_set[] = { _GFX, 40, GFX_SET_PALETTE, 255 };
	do_action(proc_set);

	static const char proc_get[] = { _GFX, 40, GFX_GET_PALETTE, 255 };
	do_action(proc_get);

	// then: R and G lose bits 4-0; B loses bits 5-0 (only 2 bits in GRB332)
	ASSERT_EQUAL(flags[41], (200 & 0xE0), "SET->GET: Red truncated to 3 bits (mask 0xE0)");
	ASSERT_EQUAL(flags[42], (100 & 0xE0), "SET->GET: Green truncated to 3 bits (mask 0xE0)");
	ASSERT_EQUAL(flags[43], (150 & 0xC0), "SET->GET: Blue truncated to 2 bits in SC8 (mask 0xC0)");
	SUCCEED();
}

// GFX_TEXTS_IN_PHYS (7) / GFX_TEXTS_IN_BACK (8): redirect text writing to Phys/Back VRAM page
// Ref: docs/GFX.md lines 13-14; plan/PRP021_GFX_Text_Phys_Back.md
void test_GFX_TEXTS_default()
{
	const char *_func = __func__;
	beforeEach();

	// then: after reset the offset must be 0 (text goes to Phys by default)
	ASSERT_EQUAL(test_gfxTextOffset, 0, "TEXTS default: offset must be 0 after beforeEach");
	SUCCEED();
}

void test_GFX_TEXTS_IN_BACK()
{
	const char *_func = __func__;
	beforeEach();

	static const char proc[] = { _GFX, 0, GFX_TEXTS_IN_BACK, 255 };
	do_action(proc);

	// then: offset must be 256 (pixels) to redirect text to Back page
	ASSERT_EQUAL(test_gfxTextOffset, 256, "TEXTS_IN_BACK: offset must be 256");
	SUCCEED();
}

void test_GFX_TEXTS_IN_PHYS()
{
	const char *_func = __func__;
	beforeEach();

	// given: offset previously set to Back
	static const char proc_back[] = { _GFX, 0, GFX_TEXTS_IN_BACK, 255 };
	do_action(proc_back);

	// when: restore to Phys
	static const char proc_phys[] = { _GFX, 0, GFX_TEXTS_IN_PHYS, 255 };
	do_action(proc_phys);

	// then: offset must be 0 again
	ASSERT_EQUAL(test_gfxTextOffset, 0, "TEXTS_IN_PHYS: offset must be 0 after switching back from Back");
	SUCCEED();
}

// =============================================================================
// Actions to show pictures [2 condacts]
// =============================================================================

// =============================================================================
// Tests PICTURE <picno>
/*	Will load into the picture buffer the given picture. If there no corresponding
	picture the next entry will be carried out, if there is then the next CondAct
	is executed. */

void test_PICTURE_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Tests DISPLAY <value>
/*	If value=0 then the last buffered picture is placed onscreen.
	If value !=0 and the picture is not a subroutine then the given window area
	is cleared. This is normally used with indirection and a flag to check and
	display darkness. */

void test_DISPLAY_success()
{
	TODO(TODO_GENERIC);
}

// =============================================================================
// Actions miscellaneous [2 condacts]
// =============================================================================

// =============================================================================
// Tests MOUSE <option>
/*	This action in preparation for the hypercard system implements skeleton
	mouse handler on the IBM. */

void test_MOUSE_success()
{
	TODO("MOUSE Not Implemented");
}

// =============================================================================
// Tests BEEP <length> <tone>
/*	Length is the duration in 1/50 seconds. Tone is like BEEP in ZX Basic but
	adding 60 to it and multiplied by 2.
	http://www.worldofspectrum.org/ZXBasicManual/zxmanchap19.html */

void test_BEEP_success()
{
	TODO(TODO_GENERIC);
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS CONDACTS (6/6) ###\n\r### (I/O + control flow) ###\n\r");
	beforeAll();

	test_SAVE_LOAD_success();
	test_RAMSAVE_success(); test_RAMSAVE_no_overflow();
	test_RAMLOAD_success();

	test_ANYKEY_success();
	test_PAUSE_success();

	test_PARSE_success();
	test_NEWTEXT_success();
	test_SYNONYM_success(); test_SYNONYM_success_verb(); test_SYNONYM_success_noun();

	test_PROCESS_success();
	test_REDO_success();
	test_DOALL_success();
	test_SKIP_success();
	test_RESTART_success();
	test_END_success();
	test_EXIT_success();

	test_DONE_success();
	test_NOTDONE_success();
	test_OK_success();

	test_EXTERN_success();
	test_CALL_success();
	test_SFX_success();
	test_GFX_SET_PALETTE(); test_GFX_GET_PALETTE(); test_GFX_SET_GET_PALETTE_roundtrip();
	test_GFX_TEXTS_default(); test_GFX_TEXTS_IN_BACK(); test_GFX_TEXTS_IN_PHYS();

	test_PICTURE_success();
	test_DISPLAY_success();

	test_MOUSE_success();
	test_BEEP_success();

	return 0;
}
