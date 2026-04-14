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
// Conditions for values/flags comparation [10 condacts]
// =============================================================================

// =============================================================================
// Tests ZERO <flagno>
/*	Succeeds if Flag flagno. is set to zero. */

void test_ZERO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 0
	flags[150] = 0;

	//BDD when checking ZERO 150
	static const char proc[] = { _ZERO, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ZERO_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking ZERO 150
	static const char proc[] = { _ZERO, 150, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ZERO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 5
	flags[150] = 1;
	flags[1] = 5;

	//BDD when checking ZERO @150
	static const char proc[] = { _ZERO|IND, 150, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTZERO <flagno>
/*	Succeeds if Flag flagno. is not set to zero. */

void test_NOTZERO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking NOTZERO 150
	static const char proc[] = { _NOTZERO, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTZERO_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 0
	flags[150] = 0;

	//BDD when checking NOTZERO 150
	static const char proc[] = { _NOTZERO, 150, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTZERO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking NOTZERO @150
	static const char proc[] = { _NOTZERO|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests EQ <flagno> <value>
/*	Succeeds if Flag flagno. is equal to value. */

void test_EQ_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking EQ 150 25
	static const char proc[] = { _EQ, 150, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_EQ_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking EQ 150 0
	static const char proc[] = { _EQ, 150, 0, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_EQ_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking EQ @150 25
	static const char proc[] = { _EQ|IND, 150, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTEQ <flagno> <value>
/*	Succeeds if Flag flagno. is not equal to value. */

void test_NOTEQ_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking NOTEQ 150 0
	static const char proc[] = { _NOTEQ, 150, 0, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTEQ_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking NOTEQ 150 25
	static const char proc[] = { _NOTEQ, 150, 25, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTEQ_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking NOTEQ @150 5
	static const char proc[] = { _NOTEQ|IND, 150, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests GT <flagno> <value>
/*	Succeeds if Flag flagno. is greater than value. */

void test_GT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 50
	flags[150] = 50;

	//BDD when checking GT 150 25
	static const char proc[] = { _GT, 150, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_GT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking GT 150 50
	static const char proc[] = { _GT, 150, 50, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 50
	flags[150] = 1;
	flags[1] = 50;

	//BDD when checking GT @150 25
	static const char proc[] = { _GT|IND, 150, 25, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests LT <flagno> <value>
/*	Succeeds if Flag flagno. is set to less than value. */

void test_LT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25
	flags[150] = 25;

	//BDD when checking LT 150 50
	static const char proc[] = { _LT, 150, 50, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_LT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 50
	flags[150] = 50;

	//BDD when checking LT 150 25
	static const char proc[] = { _LT, 150, 25, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_LT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 25
	flags[150] = 1;
	flags[1] = 25;

	//BDD when checking LT @150 50
	static const char proc[] = { _LT|IND, 150, 50, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SAME <flagno1> <flagno2>
/*	Succeeds if Flag flagno 1 has the same value as Flag flagno 2. */

void test_SAME_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 and 151 with value 25
	flags[150] = 25;
	flags[151] = 25;

	//BDD when checking SAME 150 151
	static const char proc[] = { _SAME, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SAME_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25 and flag 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking SAME 150 151
	static const char proc[] = { _SAME, 150, 151, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_SAME_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 25 and flag 151 with value 25
	flags[150] = 1;
	flags[1] = 25;
	flags[151] = 25;

	//BDD when checking SAME @150 151
	static const char proc[] = { _SAME|IND, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOTSAME <flagno1> <flagno2>
/*	Succeeds if Flag flagno 1 does not have the same value as Flag flagno 2 . */

void test_NOTSAME_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25 and flag 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking NOTSAME 150 151
	static const char proc[] = { _NOTSAME, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOTSAME_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25 and flag 151 with value 25
	flags[150] = 25;
	flags[151] = 25;

	//BDD when checking NOTSAME 150 151
	static const char proc[] = { _NOTSAME, 150, 151, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOTSAME_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 25 and flag 151 with value 50
	flags[150] = 1;
	flags[1] = 25;
	flags[151] = 50;

	//BDD when checking NOTSAME @150 151
	static const char proc[] = { _NOTSAME|IND, 150, 5, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests BIGGER <flagno1> <flagno2>
/*	Will be true if flagno 1 is larger than flagno 2 */

void test_BIGGER_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 50 and 151 with value 25
	flags[150] = 50;
	flags[151] = 25;

	//BDD when checking BIGGER 150 151
	static const char proc[] = { _BIGGER, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_BIGGER_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25 and flag 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking BIGGER 150 151
	static const char proc[] = { _BIGGER, 150, 151, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_BIGGER_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 50 and flag 151 with value 25
	flags[150] = 1;
	flags[1] = 50;
	flags[151] = 25;

	//BDD when checking BIGGER @150 151
	static const char proc[] = { _BIGGER|IND, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SMALLER <flagno1> <flagno2>
/*	Will be true if flagno 1 is smaller than flagno 2 */

void test_SMALLER_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 25 and 151 with value 50
	flags[150] = 25;
	flags[151] = 50;

	//BDD when checking SMALLER 150 151
	static const char proc[] = { _SMALLER, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SMALLER_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 50 and flag 151 with value 25
	flags[150] = 50;
	flags[151] = 25;

	//BDD when checking SMALLER 150 151
	static const char proc[] = { _SMALLER, 150, 151, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_SMALLER_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 1 and flag 1 with value 25 and flag 151 with value 50
	flags[150] = 1;
	flags[1] = 25;
	flags[151] = 50;

	//BDD when checking SMALLER @150 151
	static const char proc[] = { _SMALLER|IND, 150, 151, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions to check logical sentence [5 condacts]
// =============================================================================

// =============================================================================
// Tests ADJECT1 <word>
/*	Succeeds if the first noun's adjective in the current LS is word. */

void test_ADJECT1_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adject1 1 from Logical Sentence
	flags[fAdject1] = 1;

	//BDD when checking ADJECT1 1
	static const char proc[] = { _ADJECT1, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT1_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adject1 1 from Logical Sentence
	flags[fAdject1] = 1;

	//BDD when checking ADJECT1 2
	static const char proc[] = { _ADJECT1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT1_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adject1 1 from Logical Sentence and flag 150 with value 1
	flags[fAdject1] = 1;
	flags[150] = 1;

	//BDD when checking ADJECT1 @150
	static const char proc[] = { _ADJECT1|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ADVERB <word>
/*	Succeeds if the adverb in the current LS is word. */

void test_ADVERB_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adverb 1 from Logical Sentence
	flags[fAdverb] = 1;

	//BDD when checking ADVERB 1
	static const char proc[] = { _ADVERB, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADVERB_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adverb 1 from Logical Sentence
	flags[fAdverb] = 1;

	//BDD when checking ADVERB 2
	static const char proc[] = { _ADVERB, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ADVERB_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adverb 1 from Logical Sentence and flag 150 with value 1
	flags[fAdverb] = 1;
	flags[150] = 1;

	//BDD when checking ADVERB @150
	static const char proc[] = { _ADVERB|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PREP <word>
/*	Succeeds if the preposition in the current LS is word. */

void test_PREP_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the prep 1 from Logical Sentence
	flags[fPrep] = 1;

	//BDD when checking PREP 1
	static const char proc[] = { _PREP, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PREP_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the prep 1 from Logical Sentence
	flags[fPrep] = 1;

	//BDD when checking PREP 2
	static const char proc[] = { _PREP, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PREP_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the prep 1 from Logical Sentence and flag 150 with value 1
	flags[fPrep] = 1;
	flags[150] = 1;

	//BDD when checking PREP @150
	static const char proc[] = { _PREP|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests NOUN2 <word>
/*	Succeeds if the second noun in the current LS is word. */

void test_NOUN2_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the noun2 1 from Logical Sentence
	flags[fNoun2] = 1;

	//BDD when checking NOUN2 1
	static const char proc[] = { _NOUN2, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_NOUN2_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the noun2 1 from Logical Sentence
	flags[fNoun2] = 1;

	//BDD when checking NOUN2 2
	static const char proc[] = { _NOUN2, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_NOUN2_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the noun2 1 from Logical Sentence and flag 150 with value 1
	flags[fNoun2] = 1;
	flags[150] = 1;

	//BDD when checking NOUN2 @150
	static const char proc[] = { _NOUN2|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ADJECT2 <word>
/*	Succeeds if the second noun's adjective in the current LS is word. */

void test_ADJECT2_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adject2 1 from Logical Sentence
	flags[fAdject2] = 1;

	//BDD when checking ADJECT2 1
	static const char proc[] = { _ADJECT2, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT2_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adject2 1 from Logical Sentence
	flags[fAdject2] = 1;

	//BDD when checking ADJECT2 2
	static const char proc[] = { _ADJECT2, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_ADJECT2_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given the adject2 1 from Logical Sentence and flag 150 with value 1
	flags[fAdject2] = 1;
	flags[150] = 1;

	//BDD when checking ADJECT2 @150
	static const char proc[] = { _ADJECT2|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for random [1 condacts]
// =============================================================================

// =============================================================================
// Tests CHANCE <percent>
/*	Succeeds if percent is less than or equal to a random number in the range
	1-100 (inclusive). Thus a CHANCE 50 condition would allow PAW to look at the
	next CondAct only if the random number generated was between 1 and 50, a 50%
	chance of success. */

void test_CHANCE_0_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given none

	//BDD when checking CHANCE 0
	static const char proc[] = { _CHANCE, 0, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_CHANCE_255_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given none

	//BDD when checking CHANCE 255
	static const char proc[] = { _CHANCE, 255, 255 };
	do_action(proc);

	//BDD then succes
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CHANCE_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 150 with value 255
	flags[150] = 255;

	//BDD when checking CHANCE @150
	static const char proc[] = { _CHANCE|IND, 150, 255 };
	do_action(proc);

	//BDD then succes
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// Regression test for PRP001 (do_CHANCE off-by-one).
// CHANCE 100 must always succeed (100% probability). The old buggy version
// used '<' instead of '<=' and failed ~1% of the time. Run many iterations;
// the fixed code is deterministic (always true), the buggy one would fail
// as soon as rand()%100 returns 99.
void test_CHANCE_100_always_succeeds()
{
	const char *_func = __func__;
	uint16_t n;
	beforeEach();

	static const char proc[] = { _CHANCE, 100, 255 };
	for (n=0; n<256; n++) {
		do_action(proc);
		ASSERT(checkEntry, ERROR);
	}
	SUCCEED();
}

// =============================================================================
// Conditions for sub-process success/fail [2 condacts]
// =============================================================================

// =============================================================================
// Tests ISDONE
/*	Succeeds if the last table ended by exiting after executing at least one
	Action. This is useful to test for a single succeed/fail boolean value from
	a Sub-Process. A DONE action will cause the 'done' condition, as will any
	condact causing exit, or falling off the end of the table - assuming at
	least one CondAct (other than NOTDONE) was done.
	See also ISNDONE and NOTDONE actions. */

void test_ISDONE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given last table executed at least one action
	isDone = 1;

	//BDD when checking ISDONE
	static const char proc[] = { _ISDONE, 255 };
	do_action(proc);

	//BDD then succes
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISDONE_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given last table not executed at least one action
	isDone = 0;

	//BDD when checking ISDONE
	static const char proc[] = { _ISDONE, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ISNDONE
/*	Succeeds if the last table ended without doing anything or with a NOTDONE
	action. */

void test_ISNDONE_success()		//TODO improve this test
{
	const char *_func = __func__;
	beforeEach();

	//BDD given last table not executed at least one action
	isDone = 0;

	//BDD when checking ISNDONE
	static const char proc[] = { _ISNDONE, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ISNDONE_fails()		//TODO improve this test
{
	const char *_func = __func__;
	beforeEach();

	//BDD given last table executed at least one action
	isDone = 1;

	//BDD when checking ISNDONE
	static const char proc[] = { _ISNDONE, 255 };
	do_action(proc);

	//BDD then succes
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for object attributes [2 condacts]
// =============================================================================

// =============================================================================
// Tests HASAT <value>
/*	Checks the attribute specified by value. 0-15 are the object attributes for
	the current object. There are also several attribute numbers specified as
	symbols in SYMBOLS.SCE which check certain parts of the DAAD system flags */

void test_HASAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 10 is set
	flags[fCOAtt] = 0b00000100;

	//BDD when checking HASAT 10
	static const char proc[] = { _HASAT, 10, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_HASAT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 5 is not set
	flags[fCOAtt + 1] = 0b11011111;

	//BDD when checking HASAT 5
	static const char proc[] = { _HASAT, 5, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_HASAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 10 is set and flag 150 with value 10
	flags[fCOAtt] = 0b00000100;
	flags[150] = 10;

	//BDD when checking HASAT @150
	static const char proc[] = { _HASAT|IND, 150, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests HASNAT <value>
/*	Inverse of HASAT */

void test_HASNAT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 5 is not set
	flags[fCOAtt + 1] = 0b11011111;

	//BDD when checking HASNAT 5
	static const char proc[] = { _HASNAT, 5, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_HASNAT_fails()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 10 is set
	flags[fCOAtt] = 0b00000100;

	//BDD when checking HASNAT 10
	static const char proc[] = { _HASNAT, 10, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_HASNAT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given current object attribute 10 is set and flag 150 with value 10
	flags[fCOAtt] = 0b11111011;
	flags[150] = 10;

	//BDD when checking HASNAT @150
	static const char proc[] = { _HASNAT|IND, 150, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Conditions for player interaction [2 condacts]
// =============================================================================

// =============================================================================
// Tests INKEY
/*	Is a condition which will be satisfied if the player is pressing a key.
	In 16Bit machines Flags Key1 and Key2 (60 & 61) will be a standard IBM ASCII
	code pair.
	On 8 bit only Key1 will be valid, and the code will be machine specific. */

void test_INKEY_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given 'A' key pressed
	fake_keyPressed = 'A';

	//BDD when checking INKEY
	static const char proc[] = { _INKEY, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fKey1], 'A', ERROR);
	SUCCEED();
}

// =============================================================================
// Tests QUIT
/*	SM12 ("Are you sure?") is printed and called. Will succeed if the player replies
	starts with the first letter of SM30 ("Y") to then the remainder of the entry is
	discarded is carried out. */

void test_QUIT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given 'Y' key pressed
	fake_keyPressed = 'Y';

	//BDD when checking QUIT
	// static const char proc[] = { _QUIT, 255 };
	// do_action(proc);

	//BDD then success
	// ASSERT(!checkEntry, ERROR);
	//SUCCEED();
	TODO("Mock user input");
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS CONDACTS (2/6) ###\n\r### (Flag conditions + parser + misc) ###\n\r");
	beforeAll();

	test_ZERO_success(); test_ZERO_fails(); test_ZERO_indirection();
	test_NOTZERO_success(); test_NOTZERO_fails(); test_NOTZERO_indirection();
	test_EQ_success(); test_EQ_fails(); test_EQ_indirection();
	test_NOTEQ_success(); test_NOTEQ_fails(); test_NOTEQ_indirection();
	test_GT_success(); test_GT_fails(); test_GT_indirection();
	test_LT_success(); test_LT_fails(); test_LT_indirection();
	test_SAME_success(); test_SAME_fails(); test_SAME_indirection();
	test_NOTSAME_success(); test_NOTSAME_fails(); test_NOTSAME_indirection();
	test_BIGGER_success(); test_BIGGER_fails(); test_BIGGER_indirection();
	test_SMALLER_success(); test_SMALLER_fails(); test_SMALLER_indirection();

	test_ADJECT1_success(); test_ADJECT1_fails(); test_ADJECT1_indirection();
	test_ADVERB_success(); test_ADVERB_fails(); test_ADVERB_indirection();
	test_PREP_success(); test_PREP_fails(); test_PREP_indirection();
	test_NOUN2_success(); test_NOUN2_fails(); test_NOUN2_indirection();
	test_ADJECT2_success(); test_ADJECT2_fails(); test_ADJECT2_indirection();

	test_CHANCE_0_fails(); test_CHANCE_255_success(); test_CHANCE_indirection();
	test_CHANCE_100_always_succeeds();

	test_ISDONE_success(); test_ISDONE_fails();
	test_ISNDONE_success(); test_ISNDONE_fails();

	test_HASAT_success(); test_HASAT_fails(); test_HASAT_indirection();
	test_HASNAT_success(); test_HASNAT_fails(); test_HASNAT_indirection();

	test_INKEY_success();
	test_QUIT_success();

	return 0;
}
