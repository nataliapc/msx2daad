#include "daad_stubs.h"

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
// Tests getObjectId

void test_getObjectId_found_by_noun()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj2 has nounId=10 at player location=5
	flags[fPlayer] = 5;
	objects[2].nounId = 10;
	objects[2].adjectiveId = NULLWORD;
	objects[2].location = 5;

	//BDD when calling getObjectId(10, NULLWORD, LOC_HERE)
	uint8_t result = getObjectId(10, NULLWORD, LOC_HERE);

	//BDD then result is 2
	ASSERT_EQUAL(result, 2, "Must find object 2 by noun");
	SUCCEED();
}

void test_getObjectId_found_with_adjective()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj2 has nounId=10, adjectiveId=3 at player location=5
	flags[fPlayer] = 5;
	objects[2].nounId = 10;
	objects[2].adjectiveId = 3;
	objects[2].location = 5;

	//BDD when calling getObjectId(10, 3, LOC_HERE)
	uint8_t result = getObjectId(10, 3, LOC_HERE);

	//BDD then result is 2
	ASSERT_EQUAL(result, 2, "Must find object 2 with matching adjective");
	SUCCEED();
}

void test_getObjectId_wrong_adjective()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj2 has nounId=10, adjectiveId=2 at player location=5
	flags[fPlayer] = 5;
	objects[2].nounId = 10;
	objects[2].adjectiveId = 2;
	objects[2].location = 5;

	//BDD when calling getObjectId(10, 3, LOC_HERE) - wrong adjective
	uint8_t result = getObjectId(10, 3, LOC_HERE);

	//BDD then result is NULLWORD
	ASSERT_EQUAL(result, NULLWORD, "Must not find object with wrong adjective");
	SUCCEED();
}

void test_getObjectId_at_specific_location()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj2 has nounId=10 at location=7
	objects[2].nounId = 10;
	objects[2].adjectiveId = NULLWORD;
	objects[2].location = 7;

	//BDD when calling getObjectId(10, NULLWORD, 7)
	uint8_t result = getObjectId(10, NULLWORD, 7);

	//BDD then result is 2
	ASSERT_EQUAL(result, 2, "Must find object 2 at specific location 7");
	SUCCEED();
}

void test_getObjectId_wrong_location()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj2 has nounId=10 at location=7
	objects[2].nounId = 10;
	objects[2].adjectiveId = NULLWORD;
	objects[2].location = 7;

	//BDD when calling getObjectId(10, NULLWORD, 8) - wrong location
	uint8_t result = getObjectId(10, NULLWORD, 8);

	//BDD then result is NULLWORD
	ASSERT_EQUAL(result, NULLWORD, "Must not find object at wrong location");
	SUCCEED();
}

void test_getObjectId_not_found()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given no object has nounId=10

	//BDD when calling getObjectId(10, NULLWORD, LOC_HERE)
	uint8_t result = getObjectId(10, NULLWORD, LOC_HERE);

	//BDD then result is NULLWORD
	ASSERT_EQUAL(result, NULLWORD, "Must return NULLWORD when not found");
	SUCCEED();
}


// =============================================================================
// Tests referencedObject

void test_referencedObject_sets_all_flags()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj2: location=5, weight=7, isContainer=0, isWareable=1, extAttr1=0xAB, extAttr2=0xCD
	objects[2].location = 5;
	objects[2].attribs.mask.weight = 7;
	objects[2].attribs.mask.isContainer = 0;
	objects[2].attribs.mask.isWareable = 1;
	objects[2].extAttr1 = 0xAB;
	objects[2].extAttr2 = 0xCD;

	//BDD when calling referencedObject(2)
	referencedObject(2);

	//BDD then flags are set correctly
	ASSERT_EQUAL(flags[fCONum], 2, "flags[fCONum] must be 2");
	ASSERT_EQUAL(flags[fCOLoc], 5, "flags[fCOLoc] must be 5");
	ASSERT_EQUAL(flags[fCOWei], 7, "flags[fCOWei] must be 7");
	ASSERT_EQUAL(flags[fCOCon], 0, "flags[fCOCon] must be 0 (not container)");
	ASSERT_EQUAL(flags[fCOWR], 128, "flags[fCOWR] must be 128 (wearable)");
	ASSERT_EQUAL(flags[fCOAtt], 0xCD, "flags[fCOAtt] must be extAttr2=0xCD");
	ASSERT_EQUAL(flags[fCOAtt+1], 0xAB, "flags[fCOAtt+1] must be extAttr1=0xAB");
	SUCCEED();
}

void test_referencedObject_nullword()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given nullObject is all zeros

	//BDD when calling referencedObject(NULLWORD)
	referencedObject(NULLWORD);

	//BDD then flags reference the nullObject
	ASSERT_EQUAL(flags[fCONum], 255, "flags[fCONum] must be NULLWORD=255");
	ASSERT_EQUAL(flags[fCOLoc], 0, "flags[fCOLoc] must be 0 (from nullObject)");
	ASSERT_EQUAL(flags[fCOWei], 0, "flags[fCOWei] must be 0 (from nullObject)");
	ASSERT((flags[fCOCon] & 0x80) == 0, "flags[fCOCon] bit7 must be 0");
	ASSERT((flags[fCOWR] & 0x80) == 0, "flags[fCOWR] bit7 must be 0");
	ASSERT_EQUAL(flags[fCOAtt], 0, "flags[fCOAtt] must be 0");
	ASSERT_EQUAL(flags[fCOAtt+1], 0, "flags[fCOAtt+1] must be 0");
	SUCCEED();
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS daad_objects ###\n\r");
	daad_beforeAll();

	test_getObjectId_found_by_noun();
	test_getObjectId_found_with_adjective();
	test_getObjectId_wrong_adjective();
	test_getObjectId_at_specific_location();
	test_getObjectId_wrong_location();
	test_getObjectId_not_found();

	test_referencedObject_sets_all_flags();
	test_referencedObject_nullword();

	return 0;
}
