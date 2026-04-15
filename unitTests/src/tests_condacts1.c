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
// Conditions of player locations [4 condacts]
// =============================================================================

// =============================================================================
// Tests AT <locno>
/*	Succeeds if the current location is the same as locno. */

void test_AT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with AT 5
	static const char proc[] = { _AT, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_AT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 21
	flags[fPlayer] = 21;

	//BDD when check it with AT 5
	static const char proc[] = { _AT, 5, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 5;

	//BDD when check it with AT @150
	static const char proc[] = { _AT|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTAT <locno>
/*	Succeeds if the current location is different to locno. */

void test_NOTAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with NOTAT 2
	char proc[] = { _NOTAT, 2, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTAT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with NOTAT 5
	char proc[] = { _NOTAT, 5, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 11;

	//BDD when check it with NOTAT @150
	static const char proc[] = { _NOTAT|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ATGT <locno>
/*	Succeeds if the current location is greater than locno. */

void test_ATGT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATGT 2
	static const char proc[] = { _ATGT, 2, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ATGT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATGT 5
	static const char proc[] = { _ATGT, 5, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ATGT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 2;

	//BDD when check it with ATGT @150
	static const char proc[] = { _ATGT|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ATLT <locno>
/*	Succeeds if the current location is less than locno. */

void test_ATLT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATLT 12
	static const char proc[] = { _ATLT, 12, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ATLT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when check it with ATLT 2
	static const char proc[] = { _ATLT, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ATLT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;
	flags[150] = 12;

	//BDD when check it with ATLT @150
	static const char proc[] = { _ATLT|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions of object locations [8 condacts]
// =============================================================================

// =============================================================================
// Tests PRESENT <objno>
/*	Succeeds if Object objno. is carried (254), worn (253) or at the current
	location [fPlayer]. */

void test_PRESENT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at same place
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking PRESENT 1
	static const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking PRESENT 1
	static const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 5
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = LOC_HERE;

	//BDD when checking PRESENT @150
	static const char proc[] = { _PRESENT|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ABSENT <objno>
/*	Succeeds if Object objno. is not carried (254), not worn (253) and not at
	the current location [fPlayer]. */

void test_ABSENT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking ABSENT 1
	static const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ABSENT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at same place
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking ABSENT 1
	static const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ABSENT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking ABSENT @150
	static const char proc[] = { _ABSENT|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WORN <objno>
/*	Succeeds if object objno. is worn. */

void test_WORN_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is worn
	flags[fPlayer] = 5;
	objects[1].location = LOC_WORN;

	//BDD when checking WORN 1
	static const char proc[] = { _WORN, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WORN_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 5
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking WORN 1
	static const char proc[] = { _WORN, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WORN_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is worn
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = LOC_WORN;

	//BDD when checking WORN @150
	static const char proc[] = { _WORN|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTWORN <objno>
/*	Succeeds if Object objno. is not worn. */

void test_NOTWORN_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking NOTWORN 1
	static const char proc[] = { _NOTWORN, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTWORN_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is worn
	flags[fPlayer] = 5;
	objects[1].location = LOC_WORN;

	//BDD when checking NOTWORN 1
	static const char proc[] = { _NOTWORN, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTWORN_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking NOTWORN @150
	static const char proc[] = { _NOTWORN|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CARRIED <objno>
/*	Succeeds if Object objno. is carried. */

void test_CARRIED_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is carried
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;

	//BDD when checking CARRIED 1
	static const char proc[] = { _CARRIED, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CARRIED_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 5
	flags[fPlayer] = 5;
	objects[1].location = 5;

	//BDD when checking CARRIED 1
	static const char proc[] = { _CARRIED, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_CARRIED_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is carried
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking CARRIED @150
	static const char proc[] = { _CARRIED|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTCARR <objno>
/*	Succeeds if Object objno. is not carried. */

void test_NOTCARR_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	objects[1].location = 2;

	//BDD when checking NOTCARR 1
	static const char proc[] = { _NOTCARR, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTCARR_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is carried
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;

	//BDD when checking NOTCARR 1
	static const char proc[] = { _NOTCARR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTCARR_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 2
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking NOTCARR @150
	static const char proc[] = { _NOTCARR|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ISAT <objno> <locno+>
/*	Succeeds if Object objno. is at Location locno. */

void test_ISAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 is at loc 5
	objects[1].location = 5;

	//BDD when checking ISAT 1 5
	static const char proc[] = { _ISAT, 1, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 at loc 5
	objects[1].location = 5;

	//BDD when checking ISAT 1 6
	static const char proc[] = { _ISAT, 1, 6, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 is HERE
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 5;

	//BDD when checking ISAT @150 HERE
	static const char proc[] = { _ISAT|IND, 150, LOC_HERE, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ISNOTAT <objno> <locno+>
/*	Succeeds if Object objno. is not at Location locno. */

void test_ISNOTAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 at loc 2
	objects[1].location = 2;

	//BDD when checking ISNOTAT 1 5
	static const char proc[] = { _ISNOTAT, 1, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISNOTAT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 is at loc 5
	objects[1].location = 5;

	//BDD when checking ISNOTAT 1 5
	static const char proc[] = { _ISNOTAT, 1, 5, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ISNOTAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 at loc 2 and flag 150 with value 1
	flags[fPlayer] = 5;
	flags[150] = 1;
	objects[1].location = 2;

	//BDD when checking ISNOTAT @150 HERE
	static const char proc[] = { _ISNOTAT|IND, 150, LOC_HERE, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}


// =============================================================================
// PRP009 — Tests PRESENT: special location values
// Spec (Manual 1991 L.914): "Succeeds if Object objno. is carried, worn or at the current location"

void test_PRESENT_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 carried (LOC_CARRIED=254)
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;

	//BDD when checking PRESENT 1
	static const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc);

	//BDD then success: carried counts as present (Manual L.914)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 worn (LOC_WORN=253)
	flags[fPlayer] = 5;
	objects[1].location = LOC_WORN;

	//BDD when checking PRESENT 1
	static const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc);

	//BDD then success: worn counts as present (Manual L.914)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_not_created()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 not created (LOC_NOTCREATED=252)
	flags[fPlayer] = 5;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking PRESENT 1
	static const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc);

	//BDD then fails: not-created is not carried, worn or at current loc (Manual L.914 + L.855)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_in_container_at_loc()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, container obj0 also at loc 5,
	//      obj1 inside container (obj1.location = 0, not fPlayer(5))
	flags[fPlayer] = 5;
	objects[0].location = 5;
	objects[0].attribs.mask.isContainer = 1;
	objects[1].location = 0;	// inside container obj0

	//BDD when checking PRESENT 1
	static const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc);

	//BDD then fails: obj1.location=0 is not LOC_CARRIED, LOC_WORN, or fPlayer(5)
	//      Objects inside containers need TAKEOUT first (Manual L.914 + L.1246)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PRESENT_in_carried_container()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, container obj0 carried,
	//      obj1 inside container (obj1.location = 0)
	flags[fPlayer] = 5;
	objects[0].location = LOC_CARRIED;
	objects[0].attribs.mask.isContainer = 1;
	objects[1].location = 0;	// inside carried container

	//BDD when checking PRESENT 1
	static const char proc[] = { _PRESENT, 1, 255 };
	do_action(proc);

	//BDD then fails: obj1 is not directly carried/worn/here (Manual L.914 + L.1246)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests ABSENT: special location values
// Spec (Manual 1991 L.919): "Succeeds if Object objno. is not carried, not worn and not at the current location"

void test_ABSENT_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 carried
	flags[fPlayer] = 5;
	objects[1].location = LOC_CARRIED;

	//BDD when checking ABSENT 1
	static const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc);

	//BDD then fails: carried = present, so ABSENT fails (Manual L.919)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ABSENT_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 worn
	flags[fPlayer] = 5;
	objects[1].location = LOC_WORN;

	//BDD when checking ABSENT 1
	static const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc);

	//BDD then fails: worn = present, so ABSENT fails (Manual L.919)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ABSENT_not_created()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5 and object 1 not created
	flags[fPlayer] = 5;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking ABSENT 1
	static const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc);

	//BDD then success: not-created is not carried, worn or here (Manual L.919 + L.855)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ABSENT_in_carried_container()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, container obj0 carried, obj1 inside (location=0)
	flags[fPlayer] = 5;
	objects[0].location = LOC_CARRIED;
	objects[0].attribs.mask.isContainer = 1;
	objects[1].location = 0;	// inside carried container

	//BDD when checking ABSENT 1
	static const char proc[] = { _ABSENT, 1, 255 };
	do_action(proc);

	//BDD then success: obj1 not directly carried/worn/here (Manual L.919 + L.1246)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests WORN/NOTWORN: additional special values
// Spec (Manual 1991 L.924): "Succeeds if object objno. is worn"
// Spec (Manual 1991 L.928): "Succeeds if Object objno. is not worn"

void test_WORN_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 carried (LOC_CARRIED=254, not LOC_WORN=253)
	objects[1].location = LOC_CARRIED;

	//BDD when checking WORN 1
	static const char proc[] = { _WORN, 1, 255 };
	do_action(proc);

	//BDD then fails: carried != worn (Manual L.924)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WORN_not_created()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 not created (LOC_NOTCREATED=252, not LOC_WORN=253)
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking WORN 1
	static const char proc[] = { _WORN, 1, 255 };
	do_action(proc);

	//BDD then fails: not-created != worn (Manual L.924)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTWORN_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 carried
	objects[1].location = LOC_CARRIED;

	//BDD when checking NOTWORN 1
	static const char proc[] = { _NOTWORN, 1, 255 };
	do_action(proc);

	//BDD then success: carried is not worn (Manual L.928)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTWORN_not_created()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 not created
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking NOTWORN 1
	static const char proc[] = { _NOTWORN, 1, 255 };
	do_action(proc);

	//BDD then success: not-created is not worn (Manual L.928)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests CARRIED/NOTCARR: additional special values
// Spec (Manual 1991 L.932): "Succeeds if Object objno. is carried"
// Spec (Manual 1991 L.936): "Succeeds if Object objno. is not carried"

void test_CARRIED_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 worn (LOC_WORN=253, not LOC_CARRIED=254)
	objects[1].location = LOC_WORN;

	//BDD when checking CARRIED 1
	static const char proc[] = { _CARRIED, 1, 255 };
	do_action(proc);

	//BDD then fails: worn != carried (Manual L.932)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_CARRIED_not_created()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 not created (LOC_NOTCREATED=252)
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CARRIED 1
	static const char proc[] = { _CARRIED, 1, 255 };
	do_action(proc);

	//BDD then fails: not-created != carried (Manual L.932)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTCARR_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 worn
	objects[1].location = LOC_WORN;

	//BDD when checking NOTCARR 1
	static const char proc[] = { _NOTCARR, 1, 255 };
	do_action(proc);

	//BDD then success: worn is not carried (Manual L.936)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTCARR_not_created()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 not created
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking NOTCARR 1
	static const char proc[] = { _NOTCARR, 1, 255 };
	do_action(proc);

	//BDD then success: not-created is not carried (Manual L.936)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests ISAT: special location values as locno argument
// Spec (Manual 1991 L.940): "Succeeds if Object objno. is at Location locno"
// Spec (Manual 1991 L.855-856): locno+ valid values include 252, 253, 254, 255

void test_ISAT_loc_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 at LOC_CARRIED (254)
	objects[1].location = LOC_CARRIED;

	//BDD when checking ISAT 1 LOC_CARRIED
	static const char proc[] = { _ISAT, 1, LOC_CARRIED, 255 };
	do_action(proc);

	//BDD then success: obj1.location == LOC_CARRIED (Manual L.940 + L.856)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_loc_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 at LOC_WORN (253)
	objects[1].location = LOC_WORN;

	//BDD when checking ISAT 1 LOC_WORN
	static const char proc[] = { _ISAT, 1, LOC_WORN, 255 };
	do_action(proc);

	//BDD then success: obj1.location == LOC_WORN (Manual L.940 + L.856)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_loc_not_created()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 not created (LOC_NOTCREATED=252)
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking ISAT 1 LOC_NOTCREATED
	static const char proc[] = { _ISAT, 1, LOC_NOTCREATED, 255 };
	do_action(proc);

	//BDD then success: obj1.location == LOC_NOTCREATED (Manual L.940 + L.855)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_in_container()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 inside container obj0 (obj1.location = 0)
	objects[0].attribs.mask.isContainer = 1;
	objects[1].location = 0;	// inside container obj0

	//BDD when checking ISAT 1 0
	static const char proc[] = { _ISAT, 1, 0, 255 };
	do_action(proc);

	//BDD then success: obj1.location == 0 (inside obj0) (Manual L.940 + L.853)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISAT_loc_carried_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 at a real location (not LOC_CARRIED)
	objects[1].location = 5;

	//BDD when checking ISAT 1 LOC_CARRIED
	static const char proc[] = { _ISAT, 1, LOC_CARRIED, 255 };
	do_action(proc);

	//BDD then fails: obj1.location(5) != LOC_CARRIED(254) (Manual L.940)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests ISNOTAT: special location values as locno argument
// Spec (Manual 1991 L.944): "Succeeds if Object objno. is not at Location locno"

void test_ISNOTAT_loc_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 IS at LOC_CARRIED
	objects[1].location = LOC_CARRIED;

	//BDD when checking ISNOTAT 1 LOC_CARRIED
	static const char proc[] = { _ISNOTAT, 1, LOC_CARRIED, 255 };
	do_action(proc);

	//BDD then fails: obj1.location == LOC_CARRIED (Manual L.944)
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ISNOTAT_loc_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 at a real location (not LOC_WORN)
	objects[1].location = 5;

	//BDD when checking ISNOTAT 1 LOC_WORN
	static const char proc[] = { _ISNOTAT, 1, LOC_WORN, 255 };
	do_action(proc);

	//BDD then success: obj1.location(5) != LOC_WORN(253) (Manual L.944)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISNOTAT_in_container()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 at location 1 (not inside container obj0 which is location 0)
	objects[1].location = 1;

	//BDD when checking ISNOTAT 1 0
	static const char proc[] = { _ISNOTAT, 1, 0, 255 };
	do_action(proc);

	//BDD then success: obj1.location(1) != 0 (Manual L.944)
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS CONDACTS (1/6) ###\n\r### (Location + object location) ###\n\r");
	beforeAll();

	test_AT_success(); test_AT_fails(); test_AT_indirection();
	test_NOTAT_success(); test_NOTAT_fails(); test_NOTAT_indirection();
	test_ATGT_success(); test_ATGT_fails(); test_ATGT_indirection();
	test_ATLT_success(); test_ATLT_fails(); test_ATLT_indirection();

	test_PRESENT_success(); test_PRESENT_fails(); test_PRESENT_indirection();
	test_PRESENT_carried(); test_PRESENT_worn(); test_PRESENT_not_created();
	test_PRESENT_in_container_at_loc(); test_PRESENT_in_carried_container();
	test_ABSENT_success(); test_ABSENT_fails(); test_ABSENT_indirection();
	test_ABSENT_carried(); test_ABSENT_worn(); test_ABSENT_not_created();
	test_ABSENT_in_carried_container();
	test_WORN_success(); test_WORN_fails(); test_WORN_indirection();
	test_WORN_carried(); test_WORN_not_created();
	test_NOTWORN_success(); test_NOTWORN_fails(); test_NOTWORN_indirection();
	test_NOTWORN_carried(); test_NOTWORN_not_created();
	test_CARRIED_success(); test_CARRIED_fails(); test_CARRIED_indirection();
	test_CARRIED_worn(); test_CARRIED_not_created();
	test_NOTCARR_success(); test_NOTCARR_fails(); test_NOTCARR_indirection();
	test_NOTCARR_worn(); test_NOTCARR_not_created();
	test_ISAT_success(); test_ISAT_fails(); test_ISAT_indirection();
	test_ISAT_loc_carried(); test_ISAT_loc_worn(); test_ISAT_loc_not_created();
	test_ISAT_in_container(); test_ISAT_loc_carried_fails();
	test_ISNOTAT_success(); test_ISNOTAT_fails(); test_ISNOTAT_indirection();
	test_ISNOTAT_loc_carried(); test_ISNOTAT_loc_worn(); test_ISNOTAT_in_container();

	return 0;
}
