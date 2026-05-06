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

/// Helper: set DDB to V3
static void setV3() { hdr->version = 3; isV3 = true; }


// =============================================================================
// Diagnostic: verify ISV3 works correctly
// =============================================================================

void test_ISV3_sanity()
{
	const char *_func = __func__;
	beforeEach();

	ASSERT_EQUAL(hdr->version, 2, "beforeEach must set hdr->version=2");

	setV3();

	ASSERT_EQUAL(hdr->version, 3, "setV3 must set hdr->version=3");
	SUCCEED();
}


// =============================================================================
// Tests SETAT (condact 124): set/clear/toggle attribute bits
// =============================================================================

void test_SETAT_clear()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	// Set bit 0 of flag 59 (HASAT value 0 → flag 59, bit 0)
	flags[59] = 0xFF;

	// SETAT 0, 0 → clear bit 0 of flag 59
	char proc[] = { _SETAT, 0, 0, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[59], 0xFE, "SETAT op=0 must clear the specified bit");
	SUCCEED();
}

void test_SETAT_set()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	flags[59] = 0;

	// SETAT 0, 1 → set bit 0 of flag 59
	char proc[] = { _SETAT, 0, 1, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[59], 1, "SETAT op=1 must set the specified bit");
	SUCCEED();
}

void test_SETAT_toggle()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	flags[59] = 0;

	// SETAT 0, 2 → toggle bit 0 of flag 59 (0→1)
	char proc[] = { _SETAT, 0, 2, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[59], 1, "SETAT op=2 must toggle the specified bit (0->1)");
	SUCCEED();
}

void test_SETAT_toggle_op3()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	flags[59] = 1;

	// SETAT 0, 3 → toggle (same as 2), bit 0 of flag 59 (1→0)
	char proc[] = { _SETAT, 0, 3, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[59], 0, "SETAT op=3 must toggle like op=2 (1->0)");
	SUCCEED();
}

void test_SETAT_altflags()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	// Enable alternative flags (bit 1 of fOFlags)
	flags[fOFlags] = F53_ALTFLAGS;
	flags[91] = 0;

	// SETAT 0, 1 → with altflags, targets flag 91 bit 0
	char proc[] = { _SETAT, 0, 1, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[91], 1, "SETAT with ALTFLAGS must target flag 91 range");
	ASSERT_EQUAL(flags[59], 0, "SETAT with ALTFLAGS must NOT modify standard flag 59");
	SUCCEED();
}

void test_SETAT_v2_skip()
{
	const char *_func = __func__;
	beforeEach();
	// hdr->version = 2 (set by beforeEach)

	flags[59] = 0;

	// SETAT in V2 DDB → skip 2 args, do nothing
	char proc[] = { _SETAT, 0, 1, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[59], 0, "SETAT must be no-op in V2 DDB");
	SUCCEED();
}


// =============================================================================
// Tests INDIR (condact 122): patch 2nd arg of next condact
// =============================================================================

void test_INDIR_patches_next()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	// flag 10 = 42
	flags[10] = 42;

	// INDIR 10, then LET 100 0 → INDIR patches byte at offset +3 (the 0) with flags[10]=42
	// Layout: [INDIR][10][LET][100][0][255]
	char proc[] = { _INDIR, 10, _LET, 100, 0, 255 };
	do_action(proc);

	// After INDIR, proc[4] should be patched to 42
	ASSERT_EQUAL((uint8_t)proc[4], 42, "INDIR must patch 2nd arg of next condact with flags[flagno]");
	SUCCEED();
}

void test_INDIR_v2_skip()
{
	const char *_func = __func__;
	beforeEach();
	// hdr->version = 2

	flags[10] = 42;

	char proc[] = { _INDIR, 10, _LET, 100, 0, 255 };
	do_action(proc);

	// V2: INDIR should skip 1 arg and not patch
	ASSERT_EQUAL((uint8_t)proc[4], 0, "INDIR must be no-op in V2 DDB (no patching)");
	SUCCEED();
}


// =============================================================================
// Tests XMES (condact 120): print external message by 16-bit offset
// =============================================================================

void test_XMES_offset()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	// XMES lsb=0x34 msb=0x12 → offset 0x1234
	char proc[] = { _XMES, 0x34, 0x12, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastXmesPrinted, 0x1234, "XMES must call printXMES with correct 16-bit offset");
	SUCCEED();
}

void test_XMES_zero()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	char proc[] = { _XMES, 0, 0, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastXmesPrinted, 0, "XMES with offset 0 must call printXMES(0)");
	SUCCEED();
}

void test_XMES_v2_skip()
{
	const char *_func = __func__;
	beforeEach();
	// hdr->version = 2

	char proc[] = { _XMES, 0x34, 0x12, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastXmesPrinted, 0, "XMES must be no-op in V2 DDB");
	SUCCEED();
}


// =============================================================================
// Tests HASAT/HASNAT with F53_ALTFLAGS (V3-04)
// =============================================================================

void test_HASAT_altflags()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	// Enable alt flags (bit 1 of fOFlags)
	flags[fOFlags] = F53_ALTFLAGS;
	// Set bit 0 of flag 91 (HASAT value 0 → baseFlag=91, flag 91, bit 0)
	flags[91] = 1;
	flags[59] = 0;  // standard range clear

	// HASAT 0 → with altflags, checks flag 91 bit 0
	char proc[] = { _HASAT, 0, 255 };
	do_action(proc);

	ASSERT(checkEntry, "HASAT with ALTFLAGS must check flag 91 range (bit set)");
	SUCCEED();
}

void test_HASNAT_altflags()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	flags[fOFlags] = F53_ALTFLAGS;
	flags[91] = 1;

	// HASNAT 0 → with altflags, checks flag 91 bit 0, negated
	char proc[] = { _HASNAT, 0, 255 };
	do_action(proc);

	ASSERT(!checkEntry, "HASNAT with ALTFLAGS must negate check on flag 91 range");
	SUCCEED();
}

void test_HASAT_standard_no_altflags()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	// No altflags set
	flags[fOFlags] = 0;
	flags[59] = 1;  // standard range: flag 59, bit 0

	char proc[] = { _HASAT, 0, 255 };
	do_action(proc);

	ASSERT(checkEntry, "HASAT without ALTFLAGS must check standard flag 59 range");
	SUCCEED();
}


// =============================================================================
// Tests PAUSE 0 V3 (GETKEY behavior)
// =============================================================================

void test_PAUSE0_v3_getkey()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	// Simulate a key press ('A' = 65)
	fake_keyPressed = 65;

	char proc[] = { _PAUSE, 0, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[fKey1], 65, "PAUSE 0 V3 must store key in fKey1");
	ASSERT_EQUAL(flags[fKey2], 0, "PAUSE 0 V3 must set fKey2 to 0");
	SUCCEED();
}

void test_PAUSE0_v2_no_getkey()
{
	const char *_func = __func__;
	beforeEach();
	// hdr->version = 2

	fake_keyPressed = 65;

	// V2: PAUSE 0 → waits 256/50 secs (getTime returns 0, but it won't read keyboard)
	// We can't easily test the timer loop, but we verify fKey1 is NOT set
	// Note: do_PAUSE will enter an infinite loop since getTime() returns 0.
	// We skip this test as it would hang.
	TODO("PAUSE 0 V2 timer loop not testable (getTime stub returns 0)");
}


// =============================================================================
// Tests DOALL bit 0 of fOFlags (V3-05)
// =============================================================================

void test_DOALL_bit0_set_when_no_objects()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	// No objects at location 5 (all at location 0 from memset)
	flags[fOFlags] = 0;

	// Simulate DOALL 5 → no objects at loc 5 → bit 0 stays SET
	// We need a proper process context for DOALL
	// DOALL requires a condact stream — build one
	// Layout: [DOALL][5][DONE][255]
	char proc[] = { _DOALL, 5, _DONE, 255 };
	pPROC = proc + 1;
	indirection = false;
	currProc->condactDOALL = NULL;

	do_DOALL();

	ASSERT(flags[fOFlags] & F53_DOALLNONE, "DOALL must SET bit 0 when no objects found");
	SUCCEED();
}

void test_DOALL_bit0_clear_when_objects()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	// Place object 0 at location 5
	objects[0].location = 5;
	objects[0].nounId = 10;
	flags[fOFlags] = 0;

	char proc[] = { _DOALL, 5, _DONE, 255 };
	pPROC = proc + 1;
	indirection = false;
	currProc->condactDOALL = NULL;

	do_DOALL();

	ASSERT(!(flags[fOFlags] & F53_DOALLNONE), "DOALL must CLEAR bit 0 when objects found");
	SUCCEED();
}

// =============================================================================
// PRP028 — DOALL cancellation: NOTDONE + NEWTEXT (DAAD Ready V2 §DOALL step 1)
// =============================================================================

// TEST D-CANCEL-1 — DOALL no objects: must mark NOTDONE (isDone=false)
void test_DOALL_no_objects_marks_NOTDONE()
{
	const char *_func = __func__;
	beforeEach();
	// No objects at location 5 (all default to 0).
	isDone = true;  // pre-set to verify it gets cleared by NOTDONE

	char proc[] = { _DOALL, 5, _DONE, 255 };
	pPROC = proc + 1;
	indirection = false;
	currProc->condactDOALL = NULL;

	do_DOALL();

	ASSERT_EQUAL(isDone, false, "DOALL no-objects must call NOTDONE (isDone=false), not DONE");
	SUCCEED();
}

// TEST D-CANCEL-2 — DOALL no objects: must call clearLogicalSentences (NEWTEXT effect)
void test_DOALL_no_objects_calls_NEWTEXT()
{
	const char *_func = __func__;
	beforeEach();
	// fake_clearLogicalSentences_calls reset to 0 by beforeEach()

	char proc[] = { _DOALL, 5, _DONE, 255 };
	pPROC = proc + 1;
	indirection = false;
	currProc->condactDOALL = NULL;

	do_DOALL();

	ASSERT(fake_clearLogicalSentences_calls > 0, "DOALL no-objects must call clearLogicalSentences (NEWTEXT effect)");
	SUCCEED();
}

// TEST D-CANCEL-2b — Regression: isDone must stay false even AFTER processPROC's
// post-condact `isDone |= ce->flag`. The original PRP028 fix called do_NOTDONE
// after clearing condactDOALL, which fell into popPROC + ended-up overwritten by
// flag=1 OR on DOALL. This test forces the full inner loop via do_entry().
void test_DOALL_no_objects_isDone_false_after_processPROC_OR()
{
	const char *_func = __func__;
	beforeEach();

	// No objects at location 5 (all default to 0).
	static const char entry[] = { _DOALL, 5, 0xff };
	do_entry(entry);

	ASSERT_EQUAL(isDone, false,
		"DOALL no-objects: isDone must remain false after processPROC's post-OR");
	SUCCEED();
}

// TEST D-CANCEL-2c — Integration test reproducing the wild-reported scenario:
// player issues "DEJAR TODO" twice in a row carrying a single lantern.
//   1st "DEJAR TODO": DOALL CARRIED finds the lantern, fNoun1 is filled.
//   2nd "DEJAR TODO": no carried objects remain → DOALL must NOTDONE+NEWTEXT.
// Without the PRP028 follow-up fix (DOALL flag=1 → 0 + manual cancel), the
// second DOALL left isDone=true after processPROC's post-OR, which made the
// player handler's `ISDONE / REDO` branch swallow the input silently — the
// exact failure shown in the user's screenshot.
void test_DOALL_two_consecutive_DEJAR_TODO_second_cancels_cleanly()
{
	const char *_func = __func__;
	beforeEach();

	// Setup: player at loc 5, holds ONE carried object (lantern).
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 100;
	objects[1].adjectiveId = NULLWORD;

	// 1st DEJAR TODO: DOALL CARRIED locates the lantern, sets fNoun1.
	static const char entry1[] = { _DOALL, LOC_CARRIED, 0xff };
	do_entry(entry1);
	ASSERT_EQUAL(flags[fNoun1], 100, "1st DEJAR TODO: fNoun1 must be the lantern (id=100)");

	// Simulate: lantern got dropped — moved to player's location, no longer carried.
	objects[1].location = 5;

	// Re-init proc stack: 1st DOALL completed without entering cancel path
	// (the lantern was found), but state is fine for a fresh entry.
	initializePROC();
	pushPROC(0);
	fake_clearLogicalSentences_calls = 0;
	isDone = true;        // pre-set to confirm NOTDONE actually clears it

	// 2nd DEJAR TODO: nothing carried → cancellation path must fire.
	static const char entry2[] = { _DOALL, LOC_CARRIED, 0xff };
	do_entry(entry2);

	// Critical assertions — the bug from the screenshot:
	//   - isDone stayed `true` (flag=1 OR overrode NOTDONE) → ISDONE branch fired
	//     in the player handler → REDO swallowed input silently.
	ASSERT_EQUAL(isDone, false,
		"2nd DEJAR TODO: isDone must be false after NOTDONE (post-OR must not re-set it)");
	ASSERT(fake_clearLogicalSentences_calls > 0,
		"2nd DEJAR TODO: NEWTEXT must fire (lsBuffer cleared)");
	SUCCEED();
}

// TEST D-CANCEL-2d — Regression: DOALL exhausted *after iterating* must mark DONE.
// The user-reported screenshot showed "He dejado la linterna.No puedo hacer eso."
// because cancellation marked NOTDONE despite the lantern having been dropped.
//
// This test models processPROC's line 211 path (NOT a condact dispatch):
//   if (currProc->condactDOALL) do_DONE(); else _popPROC();
// — there is NO post-OR `isDone |= ce->flag` here, unlike condact dispatch.
// We call do_DONE() directly to expose the cancellation behaviour.
void test_DOALL_implicit_DONE_after_iteration_must_mark_DONE()
{
	const char *_func = __func__;
	beforeEach();

	// Pre-iteration state: simulate that obj 1 was already iterated by DOALL.
	flags[fPlayer]   = 5;
	flags[fDAObjNo]  = 1;             // last iterated object number

	// Fake DOALL frame: condactDOALL points past [condact_byte][locno_byte].
	// _internal_doall reads condactDOALL[-1] = locno, condactDOALL[-2] = condact.
	static const uint8_t fake_proc[] = { 0, LOC_CARRIED, 0xff };
	currProc->condactDOALL = (char*)&fake_proc[2];

	// Object 1 was already dropped (now at player loc, not LOC_CARRIED anymore).
	objects[1].location = 5;
	// All other objects default to location=0, none at LOC_CARRIED.

	isDone = false;

	// Simulate processPROC line 211: implicit DONE on table exhaustion.
	do_DONE();

	// Caso B (iterated then exhausted): something WAS done, must mark DONE.
	ASSERT_EQUAL(isDone, true,
		"Step-4 implicit DONE with no more objects: must mark DONE (caso B), not NOTDONE");
	SUCCEED();
}

// TEST D-CANCEL-2e — Same idea with multi-iteration: fDAObjNo > 0 → caso B → DONE.
void test_DOALL_implicit_DONE_after_multi_iteration_marks_DONE()
{
	const char *_func = __func__;
	beforeEach();

	flags[fPlayer]   = 5;
	flags[fDAObjNo]  = 5;             // simulate 5 objects already iterated

	static const uint8_t fake_proc[] = { 0, LOC_CARRIED, 0xff };
	currProc->condactDOALL = (char*)&fake_proc[2];

	// All objects at default location=0, none carried anymore.
	isDone = false;

	do_DONE();

	ASSERT_EQUAL(isDone, true,
		"Multi-iter DOALL exhaustion: must mark DONE regardless of how many were iterated");
	SUCCEED();
}

// NOTE: a "do_DONE with condactDOALL active and fDAObjNo==NULLWORD" test is
// not reachable in the new PRP028 Annex B architecture. Caso A is handled
// exclusively in do_DOALL's initial call; if no objects are found, condactDOALL
// is cleared before any further condact runs, so do_DONE never sees that state.
// Caso A coverage is retained in test_DOALL_no_objects_marks_NOTDONE et al.

// TEST D-CANCEL-3 — V3: F53_DOALLNONE bit stays SET after cancellation; isDone=false
void test_DOALL_no_objects_v3_F53_bit0_set_after_cancel()
{
	const char *_func = __func__;
	beforeEach(); setV3();
	flags[fOFlags] = 0;

	char proc[] = { _DOALL, 5, _DONE, 255 };
	pPROC = proc + 1;
	indirection = false;
	currProc->condactDOALL = NULL;

	do_DOALL();

	ASSERT(flags[fOFlags] & F53_DOALLNONE, "V3: F53_DOALLNONE must remain SET when DOALL cancels with no objects");
	ASSERT_EQUAL(isDone, false, "V3: DOALL no-objects must mark NOTDONE (consistent with V2)");
	SUCCEED();
}


// =============================================================================
// Tests SYNONYM V3
// =============================================================================

void test_SYNONYM_v3_no_done()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	flags[fVerb]  = 1;
	flags[fNoun1] = 2;
	isDone = false;

	char proc[] = { _SYNONYM, 3, 4, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[fVerb],  3,     "SYNONYM must substitute verb in V3");
	ASSERT_EQUAL(flags[fNoun1], 4,     "SYNONYM must substitute noun in V3");
	ASSERT_EQUAL(isDone,        false, "SYNONYM must NOT mark DONE in V3");
	SUCCEED();
}

// =============================================================================
// Integration test: SYNONYM + ISDONE entry — real inventory pattern from a DSF
// =============================================================================
// DSF pattern reported in the wild for an "I" (inventory) shortcut:
//     SYNONYM COGER TODO
//     ISDONE
//     CLEAR 53
//     DONE
// Expected behaviour:
//   - V2: SYNONYM marks DONE → ISDONE passes → CLEAR runs.
//   - V3: SYNONYM does NOT mark DONE (spec §7) → ISDONE fails →
//         checkEntry=false → CLEAR is skipped.
// Uses do_entry() which replicates processPROC()'s inner loop (checkEntry
// gating + isDone |= ce->flag), so this catches the dual-mechanism bug
// where condactList[SYNONYM].flag=1 was silently re-marking DONE in V3.

void test_entry_SYNONYM_ISDONE_v3_blocks_rest()
{
	const char *_func = __func__;
	beforeEach(); setV3();

	flags[fVerb]  = 1;
	flags[fNoun1] = 2;
	flags[53]     = 42;           // sentinel: set by caller, must survive

	static const char entry[] = {
		_SYNONYM, 3, 4,           // V3: does NOT mark DONE
		_ISDONE,                  // V3: isDone=false → checkEntry=false → abort entry
		_CLEAR, 53,               // must NOT execute
		0xff
	};
	do_entry(entry);

	// SYNONYM always substitutes regardless of version
	ASSERT_EQUAL(flags[fVerb],  3,     "V3 entry: SYNONYM must substitute verb");
	ASSERT_EQUAL(flags[fNoun1], 4,     "V3 entry: SYNONYM must substitute noun");
	// The key invariant: subsequent condacts skipped
	ASSERT_EQUAL(flags[53],     42,    "V3 entry: CLEAR 53 must NOT execute (ISDONE blocked rest)");
	ASSERT_EQUAL(isDone,        false, "V3 entry: isDone must remain false after SYNONYM");
	ASSERT_EQUAL(checkEntry,    false, "V3 entry: ISDONE must set checkEntry=false");
	SUCCEED();
}

void test_entry_SYNONYM_ISDONE_v2_runs_rest()
{
	const char *_func = __func__;
	beforeEach();                 // V2 default (isV3=false)

	flags[fVerb]  = 1;
	flags[fNoun1] = 2;
	flags[53]     = 42;           // sentinel: must be wiped by CLEAR

	static const char entry[] = {
		_SYNONYM, 3, 4,           // V2: marks DONE (Z80/6502 legacy)
		_ISDONE,                  // V2: isDone=true → checkEntry stays true
		_CLEAR, 53,               // executes: flags[53] = 0
		0xff
	};
	do_entry(entry);

	ASSERT_EQUAL(flags[fVerb],  3,     "V2 entry: SYNONYM must substitute verb");
	ASSERT_EQUAL(flags[fNoun1], 4,     "V2 entry: SYNONYM must substitute noun");
	ASSERT_EQUAL(flags[53],     0,     "V2 entry: CLEAR 53 must execute (SYNONYM marked DONE)");
	ASSERT_EQUAL(isDone,        true,  "V2 entry: isDone must be true after SYNONYM");
	ASSERT_EQUAL(checkEntry,    true,  "V2 entry: ISDONE must keep checkEntry=true");
	SUCCEED();
}

int main(char** argv, int argc)
{
	cputs("### UNIT TESTS CONDACTS V3 ###\n\r### (DAAD V3 support) ###\n\r");
	beforeAll();

	// Diagnostic
	test_ISV3_sanity();

	// XMES tests (simplest V3 condact)
	test_XMES_offset();
	test_XMES_zero();
	test_XMES_v2_skip();

	// HASAT with ALTFLAGS tests (modifies existing condact)
	test_HASAT_altflags();
	test_HASNAT_altflags();
	test_HASAT_standard_no_altflags();

	// INDIR tests
	test_INDIR_patches_next();
	test_INDIR_v2_skip();

	// SETAT tests
	test_SETAT_clear();
	test_SETAT_set();
	test_SETAT_toggle();
	test_SETAT_toggle_op3();
	test_SETAT_altflags();
	test_SETAT_v2_skip();

	// PAUSE 0 V3
	test_PAUSE0_v3_getkey();
	test_PAUSE0_v2_no_getkey();

	// DOALL bit 0
	test_DOALL_bit0_set_when_no_objects();
	test_DOALL_bit0_clear_when_objects();
	test_DOALL_no_objects_marks_NOTDONE();
	test_DOALL_no_objects_calls_NEWTEXT();
	test_DOALL_no_objects_isDone_false_after_processPROC_OR();
	test_DOALL_two_consecutive_DEJAR_TODO_second_cancels_cleanly();
	test_DOALL_implicit_DONE_after_iteration_must_mark_DONE();
	test_DOALL_implicit_DONE_after_multi_iteration_marks_DONE();
	test_DOALL_no_objects_v3_F53_bit0_set_after_cancel();

	// SYNONYM V3: must NOT mark DONE
	test_SYNONYM_v3_no_done();

	// Integration: multi-condact entry simulating a real DSF pattern
	test_entry_SYNONYM_ISDONE_v3_blocks_rest();
	test_entry_SYNONYM_ISDONE_v2_runs_rest();

	return 0;
}

