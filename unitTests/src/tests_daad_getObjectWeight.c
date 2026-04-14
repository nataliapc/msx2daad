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
// Tests getObjectWeight

// Test single object weight
void test_getObjectWeight_single()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given object 2 with weight 7
	objects[2].attribs.mask.weight = 7;

	//BDD when calling getObjectWeight(2, false)
	uint8_t result = getObjectWeight(2, false);

	//BDD then weight is 7
	ASSERT_EQUAL(result, 7, "Single object weight must be 7");
	SUCCEED();
}

// Test NULLWORD: sum of carried/worn objects
void test_getObjectWeight_nullword_carried_worn()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj1 carried weight=5, obj2 worn weight=3, obj3 not carried weight=10
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.weight = 5;
	objects[2].location = LOC_WORN;
	objects[2].attribs.mask.weight = 3;
	objects[3].location = 2; // not carried/worn
	objects[3].attribs.mask.weight = 10;

	//BDD when calling getObjectWeight(NULLWORD, true)
	uint8_t result = getObjectWeight(NULLWORD, true);

	//BDD then total is 8 (only carried + worn)
	ASSERT_EQUAL(result, 8, "Sum of carried+worn must be 8");
	SUCCEED();
}

// Test container adds contents
void test_getObjectWeight_container_adds_contents()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj1 is container weight=5, obj2 is inside (location=1) weight=3
	// Note: use index 1 (not 0) because memset initialises all locations to 0,
	// so _sumLocation(0) would sum all objects instead of just the contents.
	objects[1].attribs.mask.isContainer = 1;
	objects[1].attribs.mask.weight = 5;
	objects[2].location = 1; // inside container obj1
	objects[2].attribs.mask.weight = 3;

	//BDD when calling getObjectWeight(1, false)
	uint8_t result = getObjectWeight(1, false);

	//BDD then weight is 8 (5 + 3)
	ASSERT_EQUAL(result, 8, "Container weight must include contents (5+3=8)");
	SUCCEED();
}

// Test zero-weight container (magic bag): contents not added
void test_getObjectWeight_zero_weight_magic_bag()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj1 is container weight=0, obj2 inside weight=3
	objects[1].attribs.mask.isContainer = 1;
	objects[1].attribs.mask.weight = 0;
	objects[2].location = 1; // inside container obj1
	objects[2].attribs.mask.weight = 3;

	//BDD when calling getObjectWeight(1, false)
	uint8_t result = getObjectWeight(1, false);

	//BDD then weight is 0 (zero-weight containers don't add contents)
	ASSERT_EQUAL(result, 0, "Zero-weight container must weigh 0 (magic bag)");
	SUCCEED();
}

// Test weight capped at 255
void test_getObjectWeight_cap_255()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj1 is container weight=63, objs 2-5 inside (location=1) weight=63 each
	// Weight field is 6 bits (max 63). 63 + 4*63 = 315 > 255 → capped at 255.
	objects[1].attribs.mask.isContainer = 1;
	objects[1].attribs.mask.weight = 63;
	objects[2].location = 1; objects[2].attribs.mask.weight = 63;
	objects[3].location = 1; objects[3].attribs.mask.weight = 63;
	objects[4].location = 1; objects[4].attribs.mask.weight = 63;
	objects[5].location = 1; objects[5].attribs.mask.weight = 63;

	//BDD when calling getObjectWeight(1, false)
	uint8_t result = getObjectWeight(1, false);

	//BDD then weight is capped at 255
	ASSERT_EQUAL(result, 255, "Weight must be capped at 255");
	SUCCEED();
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS daad_getObjectWeight ###\n\r");
	daad_beforeAll();

	test_getObjectWeight_single();
	test_getObjectWeight_nullword_carried_worn();
	test_getObjectWeight_container_adds_contents();
	test_getObjectWeight_zero_weight_magic_bag();
	test_getObjectWeight_cap_255();

	return 0;
}
