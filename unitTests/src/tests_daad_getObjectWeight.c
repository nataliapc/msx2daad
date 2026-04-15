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


// =============================================================================
// PRP009 — Tests getObjectWeight: additional edge cases
// Spec (Manual 1991 L.1542): WEIGHT "calculates the true weight of all objects carried and worn"
// Spec (Manual 1991 L.1447): "if it is a container, any objects inside have their weight added"
// Spec (Manual 1991 L.1451): "container of zero weight ... objects in zero weight containers, also weigh zero"

// Test NULLWORD with no carried/worn objects -> 0
void test_getObjectWeight_nullword_no_objects()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given no objects at LOC_CARRIED or LOC_WORN
	//     (daad_beforeEach memsets locations to 0, none is LOC_CARRIED/LOC_WORN)

	//BDD when getObjectWeight(NULLWORD, true)
	uint8_t result = getObjectWeight(NULLWORD, true);

	//BDD then result = 0 (nothing carried or worn) (Manual L.1542)
	ASSERT_EQUAL(result, 0, "NULLWORD with nothing carried/worn must return 0");
	SUCCEED();
}

// Test NULLWORD with carried container (non-zero weight) that has contents
void test_getObjectWeight_nullword_with_carried_container()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj1 = carried container (w=5), obj2 inside (w=3)
	//     NULLWORD sums all carried/worn, including container contents (Manual L.1543)
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.isContainer = 1;
	objects[1].attribs.mask.weight = 5;
	objects[2].location = 1;	// inside container obj1
	objects[2].attribs.mask.weight = 3;

	//BDD when getObjectWeight(NULLWORD, true)
	uint8_t result = getObjectWeight(NULLWORD, true);

	//BDD then result = 8 (container 5 + contents 3) (Manual L.1543)
	ASSERT_EQUAL(result, 8, "NULLWORD must include contents of carried non-zero containers (5+3=8)");
	SUCCEED();
}

// Test NULLWORD with magic bag carried (zero-weight container): contents do not count
void test_getObjectWeight_nullword_magic_bag_carried()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj1 = magic bag (carried, container, w=0) with obj2 inside (w=50)
	//           obj3 = normal carried object (w=3)
	//     Spec: "objects in zero weight containers, also weigh zero" (Manual L.1451)
	//     So: obj1(0) + obj2(0, inside magic bag) + obj3(3) = 3
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.isContainer = 1;
	objects[1].attribs.mask.weight = 0;		// magic bag
	objects[2].location = 1;				// inside magic bag
	objects[2].attribs.mask.weight = 50;	// must NOT count
	objects[3].location = LOC_CARRIED;
	objects[3].attribs.mask.weight = 3;

	//BDD when getObjectWeight(NULLWORD, true)
	uint8_t result = getObjectWeight(NULLWORD, true);

	//BDD then result = 3 (magic bag + its contents = 0, only obj3 counts) (Manual L.1451)
	ASSERT_EQUAL(result, 3, "NULLWORD with magic bag: bag and contents weigh 0, only other carried items count");
	SUCCEED();
}

// Test container with no items inside
void test_getObjectWeight_empty_container()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given obj1 = container (w=10) with no objects inside
	//     Note: use obj1 (not obj0) to avoid the loc=0 ambiguity on reset
	objects[1].attribs.mask.isContainer = 1;
	objects[1].attribs.mask.weight = 10;

	//BDD when getObjectWeight(1, false)
	uint8_t result = getObjectWeight(1, false);

	//BDD then result = 10 (no contents to add) (Manual L.1447: contents added, but there are none)
	ASSERT_EQUAL(result, 10, "Empty container must return only its own weight");
	SUCCEED();
}

// Test nested containers: recursive weight accumulation
void test_getObjectWeight_nested_containers()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given:
	//   obj1 = container (w=2)
	//   obj2 = inside obj1, container (w=3)
	//   obj3 = inside obj2, plain object (w=4)
	//   Expected: 2 + 3 + 4 = 9
	//   Spec: "nested containers stop adding their contents after ten levels" (Manual L.1448)
	objects[1].attribs.mask.isContainer = 1;
	objects[1].attribs.mask.weight = 2;
	objects[2].location = 1;	// inside obj1
	objects[2].attribs.mask.isContainer = 1;
	objects[2].attribs.mask.weight = 3;
	objects[3].location = 2;	// inside obj2
	objects[3].attribs.mask.weight = 4;

	//BDD when getObjectWeight(1, false)
	uint8_t result = getObjectWeight(1, false);

	//BDD then result = 9 (two levels of nesting: 2 + 3 + 4)
	ASSERT_EQUAL(result, 9, "Nested containers must accumulate weights recursively (2+3+4=9)");
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
	test_getObjectWeight_nullword_no_objects();
	test_getObjectWeight_nullword_with_carried_container();
	test_getObjectWeight_nullword_magic_bag_carried();
	test_getObjectWeight_empty_container();
	test_getObjectWeight_nested_containers();

	return 0;
}
