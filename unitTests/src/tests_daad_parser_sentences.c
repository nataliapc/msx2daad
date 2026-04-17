#include "daad_stubs.h"

#pragma opt_code_size

static const char __THIS_FILE__[] = __FILE__;
extern uint8_t     lsBuffer0[];		// Logical sentence buffer [type+id] for PARSE 0

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
// Tests nextLogicalSentence
//
// lsBuffer0 format: pairs [id, type], terminated by id==0.
// CONJUNCTION == 5 (type value, NOT id). Real game conjunctions have id=2.
// After the fix: checks *(p+1)==CONJUNCTION (type byte), plus null guard.

// TEST 1 — empty buffer stays empty (no crash on double-call)
void test_nextLS_empty_buffer()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given lsBuffer0 is already empty
	lsBuffer0[0] = 0;

	//BDD when calling nextLogicalSentence()
	nextLogicalSentence();

	//BDD then buffer is still empty (no crash, no garbage written)
	ASSERT_EQUAL(lsBuffer0[0], 0, "empty buffer must stay empty after nextLogicalSentence");
	SUCCEED();
}

// TEST 2 — single word (verb), no conjunction → buffer cleared
void test_nextLS_single_word_no_conjunction()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given a single-verb sentence: [5, VERB, 0]
	lsBuffer0[0] = 5;       // id=5 (verb)
	lsBuffer0[1] = VERB;    // type=VERB(0)
	lsBuffer0[2] = 0;       // terminator

	//BDD when calling nextLogicalSentence()
	nextLogicalSentence();

	//BDD then buffer is empty — no more sentences
	ASSERT_EQUAL(lsBuffer0[0], 0, "single word, no conjunction: buffer must be empty after nextLogicalSentence");
	SUCCEED();
}

// TEST 3 — full sentence where noun has id=2 (real conjunction id) — must NOT be confused
void test_nextLS_noun_id2_not_confused_with_conjunction()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given sentence [20, VERB, 2, NOUN, 0]
	//     noun has id=2 which equals the id of real conjunctions in DDB.
	//     After the fix, the check is on type byte (*(p+1)), so id=2 on a NOUN is harmless.
	lsBuffer0[0] = 20;      // id=20 (verb)
	lsBuffer0[1] = VERB;    // type=VERB
	lsBuffer0[2] = 2;       // id=2 (noun — same value as real conjunction ids)
	lsBuffer0[3] = NOUN;    // type=NOUN (not CONJUNCTION)
	lsBuffer0[4] = 0;       // terminator

	//BDD when calling nextLogicalSentence()
	nextLogicalSentence();

	//BDD then buffer is empty — single sentence, no conjunction type found
	ASSERT_EQUAL(lsBuffer0[0], 0, "noun with id=2 must not be confused with conjunction");
	SUCCEED();
}

// TEST 4 — two sentences with conjunction (id=2, type=CONJUNCTION) → advance to second
void test_nextLS_two_sentences_advances_correctly()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given two sentences: [20,VERB, 2,CONJUNCTION, 15,VERB, 3,NOUN, 0]
	//     conjunction entry: id=2 (real DDB convention), type=CONJUNCTION(5)
	lsBuffer0[0] = 20;          // sentence 1: verb id=20
	lsBuffer0[1] = VERB;
	lsBuffer0[2] = 2;           // conjunction: id=2 (as in real games)
	lsBuffer0[3] = CONJUNCTION; // type=CONJUNCTION(5)
	lsBuffer0[4] = 15;          // sentence 2: verb id=15
	lsBuffer0[5] = VERB;
	lsBuffer0[6] = 3;           // sentence 2: noun id=3
	lsBuffer0[7] = NOUN;
	lsBuffer0[8] = 0;           // terminator

	//BDD when calling nextLogicalSentence()
	nextLogicalSentence();

	//BDD then buffer starts with sentence 2
	ASSERT_EQUAL(lsBuffer0[0], 15, "after conjunction: buffer[0] must be second sentence verb id");
	ASSERT_EQUAL(lsBuffer0[1], VERB, "after conjunction: buffer[1] must be VERB type");
	ASSERT_EQUAL(lsBuffer0[2], 3, "after conjunction: buffer[2] must be second sentence noun id");
	ASSERT_EQUAL(lsBuffer0[3], NOUN, "after conjunction: buffer[3] must be NOUN type");
	ASSERT_EQUAL(lsBuffer0[4], 0, "after conjunction: buffer[4] must be terminator");
	SUCCEED();
}

// TEST 5 — three sentences, advance only moves to second (not third)
void test_nextLS_three_sentences_advances_to_second_only()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given three sentences: [20,VERB, 2,CONJ, 15,VERB, 2,CONJ, 10,VERB, 0]
	lsBuffer0[0]  = 20;          // sentence 1
	lsBuffer0[1]  = VERB;
	lsBuffer0[2]  = 2;           // 1st conjunction
	lsBuffer0[3]  = CONJUNCTION;
	lsBuffer0[4]  = 15;          // sentence 2
	lsBuffer0[5]  = VERB;
	lsBuffer0[6]  = 2;           // 2nd conjunction
	lsBuffer0[7]  = CONJUNCTION;
	lsBuffer0[8]  = 10;          // sentence 3
	lsBuffer0[9]  = VERB;
	lsBuffer0[10] = 0;           // terminator

	//BDD when calling nextLogicalSentence()
	nextLogicalSentence();

	//BDD then buffer starts with sentence 2 (not sentence 3)
	ASSERT_EQUAL(lsBuffer0[0], 15, "three sentences: buffer[0] must be second sentence verb id");
	ASSERT_EQUAL(lsBuffer0[1], VERB, "three sentences: buffer[1] must be VERB");
	ASSERT_EQUAL(lsBuffer0[2], 2, "three sentences: second conjunction still present at buffer[2]");
	ASSERT_EQUAL(lsBuffer0[3], CONJUNCTION, "three sentences: buffer[3] must be CONJUNCTION type");
	ASSERT_EQUAL(lsBuffer0[4], 10, "three sentences: buffer[4] must be third sentence verb id");
	SUCCEED();
}

// TEST 6 — conjunction at end with no second phrase → buffer cleared
void test_nextLS_conjunction_at_end_clears_buffer()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given sentence ending with a conjunction and no second phrase: [20,VERB, 2,CONJUNCTION, 0]
	lsBuffer0[0] = 20;
	lsBuffer0[1] = VERB;
	lsBuffer0[2] = 2;
	lsBuffer0[3] = CONJUNCTION;
	lsBuffer0[4] = 0;

	//BDD when calling nextLogicalSentence()
	nextLogicalSentence();

	//BDD then buffer is empty (dangling conjunction, no sentence follows)
	ASSERT_EQUAL(lsBuffer0[0], 0, "conjunction at end: buffer must be empty");
	SUCCEED();
}

// TEST 7 — sentence with multiple word types (verb+noun+adverb), no conjunction
void test_nextLS_full_sentence_no_conjunction()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given [20,VERB, 4,ADVERB, 3,NOUN, 0]
	lsBuffer0[0] = 20;
	lsBuffer0[1] = VERB;
	lsBuffer0[2] = 4;
	lsBuffer0[3] = ADVERB;
	lsBuffer0[4] = 3;
	lsBuffer0[5] = NOUN;
	lsBuffer0[6] = 0;

	//BDD when calling nextLogicalSentence()
	nextLogicalSentence();

	//BDD then buffer is empty
	ASSERT_EQUAL(lsBuffer0[0], 0, "full single sentence with no conjunction: buffer must be empty");
	SUCCEED();
}


// =============================================================================

int main(char** argv, int argc)
{
	cputs("### UNIT TESTS daad_parser_sentences ###\n\r");

	daad_beforeAll();

	test_nextLS_empty_buffer();
	test_nextLS_single_word_no_conjunction();
	test_nextLS_noun_id2_not_confused_with_conjunction();
	test_nextLS_two_sentences_advances_correctly();
	test_nextLS_three_sentences_advances_to_second_only();
	test_nextLS_conjunction_at_end_clears_buffer();
	test_nextLS_full_sentence_no_conjunction();

	return 0;
}
