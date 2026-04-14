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


// Minimal DDB sections for 3 objects
static const uint8_t test_obj_locs[]  = { 5, LOC_CARRIED, LOC_WORN };
// weight=7; weight=2+container; weight=0+wearable
static const uint8_t test_obj_attrs[] = { 0x07, 0x42, 0x80 };
// extAttr1, extAttr2 per object
static const uint8_t test_obj_extr[]  = { 0xAB, 0xCD,  0x11, 0x22,  0x00, 0x00 };
// noun, adj per object
static const uint8_t test_obj_names[] = { 10, 255,  20, 30,  40, 255 };


static void setup_init_objects_hdr()
{
	hdr->numObjDsc  = 3;
	hdr->objLocLst  = (uint16_t)test_obj_locs;
	hdr->objAttrPos = (uint16_t)test_obj_attrs;
	hdr->objExtrPos = (uint16_t)test_obj_extr;
	hdr->objNamePos = (uint16_t)test_obj_names;
}


// =============================================================================
// Tests initObjects

void test_initObjects_locations()
{
	const char *_func = __func__;
	daad_beforeEach();
	setup_init_objects_hdr();

	//BDD when calling initObjects()
	initObjects();

	//BDD then object locations are loaded correctly
	ASSERT_EQUAL(objects[0].location, 5, "objects[0].location must be 5");
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, "objects[1].location must be LOC_CARRIED");
	ASSERT_EQUAL(objects[2].location, LOC_WORN, "objects[2].location must be LOC_WORN");
	SUCCEED();
}

void test_initObjects_attribs()
{
	const char *_func = __func__;
	daad_beforeEach();
	setup_init_objects_hdr();

	//BDD when calling initObjects()
	initObjects();

	//BDD then object attributes are loaded correctly
	// 0x07 => weight=7, isContainer=0, isWareable=0
	ASSERT_EQUAL(objects[0].attribs.mask.weight, 7, "objects[0].weight must be 7");
	// 0x42 => weight=2, isContainer=1 (bit6), isWareable=0
	ASSERT_EQUAL(objects[1].attribs.mask.isContainer, 1, "objects[1].isContainer must be 1");
	ASSERT_EQUAL(objects[1].attribs.mask.weight, 2, "objects[1].weight must be 2");
	// 0x80 => weight=0, isContainer=0, isWareable=1 (bit7)
	ASSERT_EQUAL(objects[2].attribs.mask.isWareable, 1, "objects[2].isWareable must be 1");
	SUCCEED();
}

void test_initObjects_noun_adj()
{
	const char *_func = __func__;
	daad_beforeEach();
	setup_init_objects_hdr();

	//BDD when calling initObjects()
	initObjects();

	//BDD then noun/adj are loaded correctly
	ASSERT_EQUAL(objects[0].nounId, 10, "objects[0].nounId must be 10");
	ASSERT_EQUAL(objects[0].adjectiveId, 255, "objects[0].adjectiveId must be 255");
	ASSERT_EQUAL(objects[1].nounId, 20, "objects[1].nounId must be 20");
	ASSERT_EQUAL(objects[1].adjectiveId, 30, "objects[1].adjectiveId must be 30");
	SUCCEED();
}

void test_initObjects_counts_carried()
{
	const char *_func = __func__;
	daad_beforeEach();
	setup_init_objects_hdr();

	//BDD when calling initObjects() with 1 object at LOC_CARRIED
	initObjects();

	//BDD then fNOCarr == 1
	ASSERT_EQUAL(flags[fNOCarr], 1, "fNOCarr must be 1 (one carried object)");
	SUCCEED();
}


// =============================================================================
// Tests initFlags

void test_initFlags_clears_player()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given flags[fPlayer]=5
	flags[fPlayer] = 5;

	//BDD when calling initFlags()
	initFlags();

	//BDD then flags[fPlayer] is cleared
	ASSERT_EQUAL(flags[fPlayer], 0, "initFlags must clear flags[fPlayer]");
	SUCCEED();
}

void test_initFlags_allocates_windows()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD when calling initFlags()
	initFlags();

	//BDD then windows is allocated and cw points to first window
	ASSERT(windows != 0, "windows must be allocated");
	ASSERT(cw == windows, "cw must point to windows[0]");
	SUCCEED();
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS daad_init ###\n\r");
	daad_beforeAll();

	test_initObjects_locations();
	test_initObjects_attribs();
	test_initObjects_noun_adj();
	test_initObjects_counts_carried();

	test_initFlags_clears_player();
	test_initFlags_allocates_windows();

	return 0;
}
