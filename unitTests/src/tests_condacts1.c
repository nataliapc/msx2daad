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


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS CONDACTS (1/6) ###\n\r### (Location + object location) ###\n\r");
	beforeAll();

	test_AT_success(); test_AT_fails(); test_AT_indirection();
	test_NOTAT_success(); test_NOTAT_fails(); test_NOTAT_indirection();
	test_ATGT_success(); test_ATGT_fails(); test_ATGT_indirection();
	test_ATLT_success(); test_ATLT_fails(); test_ATLT_indirection();

	test_PRESENT_success(); test_PRESENT_fails(); test_PRESENT_indirection();
	test_ABSENT_success(); test_ABSENT_fails(); test_ABSENT_indirection();
	test_WORN_success(); test_WORN_fails(); test_WORN_indirection();
	test_NOTWORN_success(); test_NOTWORN_fails(); test_NOTWORN_indirection();
	test_CARRIED_success(); test_CARRIED_fails(); test_CARRIED_indirection();
	test_NOTCARR_success(); test_NOTCARR_fails(); test_NOTCARR_indirection();
	test_ISAT_success(); test_ISAT_fails(); test_ISAT_indirection();
	test_ISNOTAT_success(); test_ISNOTAT_fails(); test_ISNOTAT_indirection();

	return 0;
}
