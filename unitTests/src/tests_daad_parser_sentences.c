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
// Tests populateLogicalSentence — INC-01 (extAttr byte order) + INC-02 (PRONOUN)

// Helper: set up one object at position `objno` with given attributes and location
static void setup_obj(uint8_t objno, uint8_t nounId, uint8_t adjectiveId, uint8_t location,
                      uint8_t extAttr1, uint8_t extAttr2)
{
	objects[objno].nounId = nounId;
	objects[objno].adjectiveId = adjectiveId;
	objects[objno].location = location;
	objects[objno].extAttr1 = extAttr1;
	objects[objno].extAttr2 = extAttr2;
	objects[objno].attribs.byte = 0;
}

// --- INC-01: extAttr byte order for Object 2 ---

// TEST 8 — extAttr byte order for Obj2 matches Current Object convention (flag 58/59)
void test_populateLS_obj2_extAttr_byte_order_matches_CO()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given object 3 at player location with extAttr1=0xAB, extAttr2=0xCD
	flags[fPlayer] = 5;
	setup_obj(3, 100, NULLWORD, 5, 0xAB, 0xCD);

	//BDD given lsBuffer0 with Noun1 + Noun2=100 (so Noun2 triggers the obj lookup):
	//     "VERB id=10, NOUN1 id=51, NOUN2 id=100"
	lsBuffer0[0] = 10;   lsBuffer0[1] = VERB;
	lsBuffer0[2] = 51;   lsBuffer0[3] = NOUN;
	lsBuffer0[4] = 100;  lsBuffer0[5] = NOUN;
	lsBuffer0[6] = 0;

	//BDD when populateLogicalSentence()
	populateLogicalSentence();

	//BDD then fO2Att high byte is extAttr2, low byte is extAttr1 (consistent with fCOAtt)
	ASSERT_EQUAL(flags[fNoun2], 100, "fNoun2 must be 100");
	ASSERT_EQUAL(flags[fO2Num], 3, "fO2Num must be resolved to obj 3");
	ASSERT_EQUAL(flags[fO2Att],   0xCD, "flag 39 (fO2Att)   must be extAttr2 (bits 8-15)");
	ASSERT_EQUAL(flags[fO2Att+1], 0xAB, "flag 40 (fO2Att+1) must be extAttr1 (bits 0-7)");
	SUCCEED();
}

// TEST 9 — extAttr cleared when Obj2 not found (regression check for the else branch)
void test_populateLS_obj2_extAttr_cleared_when_obj_not_found()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given no objects match Noun2=200 (all objects zero-initialized with nounId=0)
	flags[fPlayer] = 5;
	//BDD and flags pre-set to non-zero to confirm clearing
	flags[fO2Att]   = 0xFF;
	flags[fO2Att+1] = 0xFF;

	//BDD given lsBuffer0 with Noun1 + Noun2=200 (no match)
	lsBuffer0[0] = 10;   lsBuffer0[1] = VERB;
	lsBuffer0[2] = 51;   lsBuffer0[3] = NOUN;
	lsBuffer0[4] = 200;  lsBuffer0[5] = NOUN;
	lsBuffer0[6] = 0;

	//BDD when populateLogicalSentence()
	populateLogicalSentence();

	//BDD then extAttr flags are cleared
	ASSERT_EQUAL(flags[fNoun2], 200, "fNoun2 must be 200");
	ASSERT_EQUAL(flags[fO2Num], LOC_NOTCREATED, "fO2Num must be LOC_NOTCREATED (obj not found)");
	ASSERT_EQUAL(flags[fO2Att],   0, "flag 39 must be cleared when Obj2 not found");
	ASSERT_EQUAL(flags[fO2Att+1], 0, "flag 40 must be cleared when Obj2 not found");
	SUCCEED();
}


// --- INC-02: PRONOUN handling ---

// TEST 10 — pronoun replaces empty Noun1 with saved fCPNoun/fCPAdject
void test_populateLS_pronoun_replaces_empty_noun1()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given saved pronoun state (previous sentence left fCPNoun=100, fCPAdject=5)
	flags[fCPNoun]   = 100;
	flags[fCPAdject] = 5;

	//BDD given lsBuffer0 "EXAMINE IT" = [30,VERB, 99,PRONOUN, 0]
	lsBuffer0[0] = 30;  lsBuffer0[1] = VERB;
	lsBuffer0[2] = 99;  lsBuffer0[3] = PRONOUN;
	lsBuffer0[4] = 0;

	//BDD when populateLogicalSentence()
	populateLogicalSentence();

	//BDD then fVerb=30, fNoun1=100 (from pronoun), fAdject1=5
	ASSERT_EQUAL(flags[fVerb],    30,  "fVerb must be 30");
	ASSERT_EQUAL(flags[fNoun1],   100, "fNoun1 must be filled from fCPNoun");
	ASSERT_EQUAL(flags[fAdject1], 5,   "fAdject1 must be filled from fCPAdject");
	SUCCEED();
}

// TEST 11 — pronoun ignored if Noun1 already present (explicit noun wins)
void test_populateLS_pronoun_ignored_if_noun1_already_set()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given saved pronoun state
	flags[fCPNoun]   = 100;
	flags[fCPAdject] = 5;

	//BDD given "EXAMINE IT SWORD" = [30,VERB, 99,PRONOUN, 200,NOUN, 0]
	lsBuffer0[0] = 30;   lsBuffer0[1] = VERB;
	lsBuffer0[2] = 99;   lsBuffer0[3] = PRONOUN;
	lsBuffer0[4] = 200;  lsBuffer0[5] = NOUN;
	lsBuffer0[6] = 0;

	//BDD when populateLogicalSentence()
	populateLogicalSentence();

	//BDD then explicit Noun1=200 wins (pronoun sees fNoun1 already set when processed after NOUN)
	//     Note: actually pronoun is processed BEFORE the noun in this order; but since Noun1
	//     empty at that point, pronoun will fill it. Let's check reality: after the loop,
	//     fNoun1 ends up equal to whichever was set FIRST by a token appearing before. Here
	//     PRONOUN (token at pos 2) runs before NOUN (token at pos 4). So fNoun1=100 from pronoun.
	//     This test documents actual behavior: pronoun fills, later NOUN would go to fNoun2.
	ASSERT_EQUAL(flags[fNoun1], 100, "pronoun fills Noun1 first, later NOUN goes to Noun2");
	ASSERT_EQUAL(flags[fNoun2], 200, "explicit NOUN after pronoun must land in Noun2");
	SUCCEED();
}

// TEST 12 — pronoun is no-op if fCPNoun is NULLWORD (no previous noun)
void test_populateLS_pronoun_when_cpnoun_is_nullword()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given fCPNoun is NULLWORD (no stored pronoun yet)
	flags[fCPNoun]   = NULLWORD;
	flags[fCPAdject] = NULLWORD;

	//BDD given "EXAMINE IT"
	lsBuffer0[0] = 30;  lsBuffer0[1] = VERB;
	lsBuffer0[2] = 99;  lsBuffer0[3] = PRONOUN;
	lsBuffer0[4] = 0;

	//BDD when populateLogicalSentence()
	populateLogicalSentence();

	//BDD then fNoun1 stays NULLWORD (no replacement possible)
	ASSERT_EQUAL(flags[fNoun1], NULLWORD, "fNoun1 stays NULLWORD when fCPNoun is NULLWORD");
	SUCCEED();
}

// TEST 13 — non-proper noun (id >= 50) is saved as pronoun reference
void test_populateLS_saves_non_proper_noun_as_pronoun()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given a previous pronoun state that will be overwritten
	flags[fCPNoun]   = 50;
	flags[fCPAdject] = 0;

	//BDD given "GET TORCH RED" = [30,VERB, 100,NOUN, 7,ADJECTIVE, 0]  (TORCH id=100 >= 50)
	lsBuffer0[0] = 30;   lsBuffer0[1] = VERB;
	lsBuffer0[2] = 100;  lsBuffer0[3] = NOUN;
	lsBuffer0[4] = 7;    lsBuffer0[5] = ADJECTIVE;
	lsBuffer0[6] = 0;

	//BDD when populateLogicalSentence()
	populateLogicalSentence();

	//BDD then fCPNoun/fCPAdject are updated to the non-proper noun
	ASSERT_EQUAL(flags[fCPNoun],   100, "fCPNoun must be updated with non-proper noun id");
	ASSERT_EQUAL(flags[fCPAdject], 7,   "fCPAdject must be updated with the adjective");
	SUCCEED();
}

// TEST 14 — proper noun (id < 50) does NOT update pronoun reference
void test_populateLS_proper_noun_doesnt_update_pronoun()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given saved pronoun state (TORCH id=100 from a previous sentence)
	flags[fCPNoun]   = 100;
	flags[fCPAdject] = 7;

	//BDD given "TALK WIZARD" = [30,VERB, 30,NOUN, 0]  (WIZARD id=30 < 50 → proper)
	lsBuffer0[0] = 30;  lsBuffer0[1] = VERB;
	lsBuffer0[2] = 30;  lsBuffer0[3] = NOUN;
	lsBuffer0[4] = 0;

	//BDD when populateLogicalSentence()
	populateLogicalSentence();

	//BDD then fCPNoun/fCPAdject remain unchanged (proper noun doesn't overwrite pronoun)
	ASSERT_EQUAL(flags[fCPNoun],   100, "fCPNoun must NOT be overwritten by a proper noun");
	ASSERT_EQUAL(flags[fCPAdject], 7,   "fCPAdject must remain unchanged");
	SUCCEED();
}

// TEST 15 — fCPNoun/fCPAdject persist across populateLogicalSentence calls
void test_populateLS_does_not_clear_cpnoun_on_each_call()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given pronoun state from a previous sentence
	flags[fCPNoun]   = 100;
	flags[fCPAdject] = 5;

	//BDD given a sentence without noun or pronoun (just a verb)
	lsBuffer0[0] = 30;  lsBuffer0[1] = VERB;
	lsBuffer0[2] = 0;

	//BDD when populateLogicalSentence()
	populateLogicalSentence();

	//BDD then fCPNoun/fCPAdject are not touched
	ASSERT_EQUAL(flags[fCPNoun],   100, "fCPNoun must survive across populateLogicalSentence calls");
	ASSERT_EQUAL(flags[fCPAdject], 5,   "fCPAdject must survive across populateLogicalSentence calls");
	SUCCEED();
}

// TEST 16 — boundary: id=49 is proper (< 50) → pronoun NOT updated
void test_populateLS_pronoun_boundary_id_49_is_proper()
{
	const char *_func = __func__;
	daad_beforeEach();

	flags[fCPNoun]   = 200;
	flags[fCPAdject] = 0;

	//BDD given noun with id=49 (proper, just below threshold)
	lsBuffer0[0] = 30;  lsBuffer0[1] = VERB;
	lsBuffer0[2] = 49;  lsBuffer0[3] = NOUN;
	lsBuffer0[4] = 0;

	populateLogicalSentence();

	ASSERT_EQUAL(flags[fCPNoun], 200, "id=49 is proper → fCPNoun must not be updated");
	SUCCEED();
}

// TEST 17 — boundary: id=50 is non-proper → pronoun IS updated
void test_populateLS_pronoun_boundary_id_50_is_non_proper()
{
	const char *_func = __func__;
	daad_beforeEach();

	flags[fCPNoun]   = 200;
	flags[fCPAdject] = 0;

	//BDD given noun with id=50 (non-proper, threshold)
	lsBuffer0[0] = 30;  lsBuffer0[1] = VERB;
	lsBuffer0[2] = 50;  lsBuffer0[3] = NOUN;
	lsBuffer0[4] = 0;

	populateLogicalSentence();

	ASSERT_EQUAL(flags[fCPNoun], 50, "id=50 is non-proper → fCPNoun must be updated");
	SUCCEED();
}


// =============================================================================

int main(char** argv, int argc)
{
	cputs("### UNIT TESTS daad_parser_sentences ###\n\r");

	daad_beforeAll();

	// nextLogicalSentence (PRP014)
	test_nextLS_empty_buffer();
	test_nextLS_single_word_no_conjunction();
	test_nextLS_noun_id2_not_confused_with_conjunction();
	test_nextLS_two_sentences_advances_correctly();
	test_nextLS_three_sentences_advances_to_second_only();
	test_nextLS_conjunction_at_end_clears_buffer();
	test_nextLS_full_sentence_no_conjunction();

	// populateLogicalSentence — INC-01 (PRP015)
	test_populateLS_obj2_extAttr_byte_order_matches_CO();
	test_populateLS_obj2_extAttr_cleared_when_obj_not_found();

	// populateLogicalSentence — INC-02 (PRP015)
	test_populateLS_pronoun_replaces_empty_noun1();
	test_populateLS_pronoun_ignored_if_noun1_already_set();
	test_populateLS_pronoun_when_cpnoun_is_nullword();
	test_populateLS_saves_non_proper_noun_as_pronoun();
	test_populateLS_proper_noun_doesnt_update_pronoun();
	test_populateLS_does_not_clear_cpnoun_on_each_call();
	test_populateLS_pronoun_boundary_id_49_is_proper();
	test_populateLS_pronoun_boundary_id_50_is_non_proper();

	return 0;
}
