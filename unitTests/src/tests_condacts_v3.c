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

	return 0;
}
