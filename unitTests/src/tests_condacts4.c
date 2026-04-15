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
// Tests COPYOO <objno1> <objno2>
/*	The position of Object objno2 is set to be the same as the position of
	Object Objno1. The currently referenced object is set to be Object objno2 */

void test_COPYOO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 2, and an object 2 at loc 4
	objects[1].location = 2;
	objects[2].location = 4;

	//BDD when checking COPYOO 1 2
	static const char proc[] = { _COPYOO, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fCONum], 2, "Current object not object2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYOO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given floag 75 with value 1, an object 1 at loc 2, and an object 2 at loc 4
	flags[75] = 1;
	objects[1].location = 2;
	objects[2].location = 4;

	//BDD when checking COPYOO @75 2
	static const char proc[] = { _COPYOO|IND, 75, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fCONum], 2, "Current object not object2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests RESET
/*	This Action bears no resemblance to the one with the same name in PAW. It has
	the pure function of placing all objects at the position given in the Object
	start table. It also sets the relevant flags dealing with no of objects
	carried etc. */

void test_RESET_success()
{
	TODO("Must mock initObject()");
}

// =============================================================================
// Actions for object in flags manipulation [5 condacts]
// =============================================================================

// =============================================================================
// Tests COPYOF <objno> <flagno>
/*	The position of Object objno. is copied into Flag flagno. This could be used
	to examine the location of an object in a comparison with another flag value. */

void test_COPYOF_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1, and an empty flag 200
	flags[200] = 0;
	objects[1].location = LOC_WORN;

	//BDD when checking COPYOF 1 200
	static const char proc[] = { _COPYOF, 1, 200, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[200], LOC_WORN, "Flag 200 don't have the object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYOF_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1, a flag 75 with value 1, and an empty flag 200
	flags[75] = 1;
	flags[200] = 0;
	objects[1].location = LOC_WORN;

	//BDD when checking COPYOF @75 200
	static const char proc[] = { _COPYOF|IND, 75, 200, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[200], LOC_WORN, "Flag 200 don't have the object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests COPYFO <flagno> <objno>
/*	The position of Object objno. is set to be the contents of Flag flagno. An
	attempt to copy from a flag containing 255 will result in a run time error.
	Setting an object to an invalid location will still be accepted as it
	presents no danger to the operation of PAW. */

void test_COPYFO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1, and an empty flag 200
	flags[200] = LOC_WORN;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking COPYFO 200 1
	static const char proc[] = { _COPYFO, 200, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_WORN, "Flag 200 don't have the object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYFO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1, a flag 75 with value 1, and an empty flag 200
	flags[75] = 200;
	flags[200] = LOC_WORN;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking COPYFO @75 1
	static const char proc[] = { _COPYFO|IND, 75, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_WORN, "Flag 200 don't have the object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WHATO
/*	A search for the object number represented by Noun(Adjective)1 is made in
	the object definition section in order of location priority; carried, worn,
	here. This is because it is assumed any use of WHATO will be related to
	carried objects rather than any that are worn or here. If an object is found
	its number is placedin flag 51, along with the standard current object
	parameters in flags 54-57. This allows you to create other auto actions (the
	tutorial gives an example of this for dropping objects in the tree). */

void test_WHATO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 2 with nounId=10 at player's location=5, wearable, non-container
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[2].location = 5;
	objects[2].nounId = 10;
	objects[2].adjectiveId = NULLWORD;
	objects[2].attribs.mask.weight = 3;
	objects[2].attribs.mask.isWareable = 1;
	objects[2].attribs.mask.isContainer = 0;

	//BDD when WHATO executes - checks CARRIED/WORN/HERE, finds object at HERE
	static const char proc[] = { _WHATO, 255 };
	do_action(proc);

	//BDD then object 2 is referenced in flags 51+54-57 (per spec: "flags 54-57")
	ASSERT_EQUAL(flags[fCONum], 2, "WHATO must reference object 2");
	ASSERT_EQUAL(flags[fCOLoc], 5, "Current object location must be 5");
	ASSERT_EQUAL(flags[fCOWei], 3, "Current object weight must be 3");
	ASSERT((flags[fCOCon] & 0x80) == 0, "flag56 bit7 must be 0 (not a container)");
	ASSERT((flags[fCOWR]  & 0x80) != 0, "flag57 bit7 must be 1 (wearable)");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SETCO <objno>
/*	Sets the currently referenced object to objno. */

void test_SETCO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 2
	objects[1].location = 2;

	//BDD when checking SETCO 1
	static const char proc[] = { _SETCO, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object 1");
	ASSERT_EQUAL(flags[fCOLoc], 2, "Current object location is not 2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SETCO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 1, and object 1 at loc 2
	flags[75] = 1;
	objects[1].location = 2;

	//BDD when checking SETCO @75
	static const char proc[] = { _SETCO|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object 1");
	ASSERT_EQUAL(flags[fCOLoc], 2, "Current object location is not 2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WEIGH <objno> <flagno>
/*	The true weight of Object objno. is calculated (i.e. if it is a container,
	any objects inside have their weight added - don't forget that nested
	containers stop adding their contents after ten levels) and the value is
	placed in Flag flagno. This will have a maximum value of 255 which will not
	be exceeded. If Object objno. is a container of zero weight, Flag flagno
	will be cleared as objects in zero weight containers, also weigh zero! */

void test_WEIGH_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 3 with weight 7
	objects[3].attribs.mask.weight = 7;

	//BDD when checking WEIGH 3 100
	static const char proc[] = { _WEIGH, 3, 100, 255 };
	do_action(proc);

	//BDD then flags[100] == 7
	ASSERT_EQUAL(flags[100], 7, "WEIGH must store object weight in flag");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_WEIGH_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 50 = 3 (indirection), object 3 with weight 7
	flags[50] = 3;
	objects[3].attribs.mask.weight = 7;

	//BDD when checking WEIGH @50 100
	static const char proc[] = { _WEIGH|IND, 50, 100, 255 };
	do_action(proc);

	//BDD then flags[100] == 7
	ASSERT_EQUAL(flags[100], 7, "WEIGH indirection must resolve objno from flag");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Actions to manipulate flags [11 condacts]
// =============================================================================

// =============================================================================
// Tests SET <flagno>
/*	Flag flagno. is set to 255. */

void test_SET_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 1
	flags[100] = 1;

	//BDD when checking SET 100
	static const char proc[] = { _SET, 100, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 255, "Flag 100 is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SET_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, and flag 100 with value 1
	flags[75] = 100;
	flags[100] = 1;

	//BDD when checking SET @75
	static const char proc[] = { _SET|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[75], 100, "Flag 75 have changed");
	ASSERT_EQUAL(flags[100], 255, "Flag 100 is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CLEAR <flagno>
/*	Flag flagno. is cleared to 0. */

void test_CLEAR_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 100
	flags[100] = 100;

	//BDD when checking CLEAR 100
	static const char proc[] = { _CLEAR, 100, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 0, "Flag 100 is not 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CLEAR_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 150
	flags[75] = 100;
	flags[100] = 150;

	//BDD when checking CLEAR @75
	static const char proc[] = { _CLEAR|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[75], 100, "Flag 75 have changed");
	ASSERT_EQUAL(flags[100], 0, "Flag 100 is not 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests LET <flagno> <value>
/*	Flag flagno. is set to value. */

void test_LET_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 1
	flags[100] = 1;

	//BDD when checking LET 100 50
	static const char proc[] = { _LET, 100, 50, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 50, "Flag 100 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_LET_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with valur 100, flag 100 with value 1
	flags[75] = 100;
	flags[100] = 1;

	//BDD when checking LET @75 80
	static const char proc[] = { _LET|IND, 75, 80, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 80, "Flag 100 is not 80");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PLUS <flagno> <value>
/*	Flag flagno. is increased by value. If the result exceeds 255 the flag is
	set to 255. */

void test_PLUS_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10
	flags[100] = 10;

	//BDD when checking PLUS 100 50
	static const char proc[] = { _PLUS, 100, 50, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 60, "Flag 100 is not 60");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PLUS_overflow()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 210
	flags[100] = 210;

	//BDD when checking PLUS 100 50
	static const char proc[] = { _PLUS, 100, 50, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 255, "Flag 100 is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PLUS_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10
	flags[75] = 100;
	flags[100] = 10;

	//BDD when checking PLUS @75 80
	static const char proc[] = { _PLUS|IND, 75, 80, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 90, "Flag 100 is not 90");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests MINUS <flagno> <value>
/*	Flag flagno. is decreased by value. If the result is negative the flag is
	set to 0. */

void test_MINUS_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 60
	flags[100] = 60;

	//BDD when checking MINUS 100 10
	static const char proc[] = { _MINUS, 100, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 50, "Flag 100 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_MINUS_overflow()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 60
	flags[100] = 60;

	//BDD when checking MINUS 100 150
	static const char proc[] = { _MINUS, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 0, "Flag 100 is not 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_MINUS_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 60
	flags[75] = 100;
	flags[100] = 60;

	//BDD when checking MINUS @75 10
	static const char proc[] = { _MINUS|IND, 75, 10, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[100], 50, "Flag 100 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ADD <flagno1> <flagno2>
/*	Flag flagno 2 has the contents of Flag flagno 1 added to it. If the result
	exceeds 255 the flag is set to 255. */

void test_ADD_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 50
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking ADD 100 150
	static const char proc[] = { _ADD, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 60, "Flag 150 is not 60");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADD_overflow()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 250
	flags[100] = 10;
	flags[150] = 250;

	//BDD when checking ADD 100 150
	static const char proc[] = { _ADD, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 255, "Flag 150 is not 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ADD_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10, and flag 150 with value 50
	flags[75] = 100;
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking ADD @75 150
	static const char proc[] = { _ADD|IND, 75, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 60, "Flag 150 is not 60");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SUB <flagno1> <flagno2>
/*	Flag flagno 2 has the contents of Flag flagno 1 subtracted from it. If the
	result is negative the flag is set to 0. */

void test_SUB_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 50
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking SUB 100 150
	static const char proc[] = { _SUB, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 40, "Flag 150 is not 40");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SUB_overflow()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 50, and flag 150 with value 10
	flags[100] = 50;
	flags[150] = 10;

	//BDD when checking SUB 100 150
	static const char proc[] = { _SUB, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 0, "Flag 150 is not 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SUB_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10, and flag 150 with value 50
	flags[75] = 100;
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking SUB @75 150
	static const char proc[] = { _SUB|IND, 75, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 40, "Flag 150 is not 40");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests COPYFF <flagno1> <flagno2>
/*	The contents of Flag flagno 1 is copied to Flag flagno 2. */

void test_COPYFF_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 50
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking COPYFF 100 150
	static const char proc[] = { _COPYFF, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 10, "Flag 150 is not 10");
	ASSERT_EQUAL(flags[100], 10, "Flag 150 is not 10");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYFF_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10, and flag 150 with value 50
	flags[75] = 100;
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking COPYFF @75 150
	static const char proc[] = { _COPYFF|IND, 75, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 10, "Flag 150 is not 10");
	ASSERT_EQUAL(flags[100], 10, "Flag 150 is not 10");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests COPYBF <flagno1> <flagno2>
/*	Same as COPYFF but the source and destination are reversed, so that
	indirection can be used. */

void test_COPYBF_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 100 with value 10, and flag 150 with value 50
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking COPYBF 100 150
	static const char proc[] = { _COPYBF, 100, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 50, "Flag 150 is not 50");
	ASSERT_EQUAL(flags[100], 50, "Flag 150 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_COPYBF_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 75 with value 100, flag 100 with value 10, and flag 150 with value 50
	flags[75] = 100;
	flags[100] = 10;
	flags[150] = 50;

	//BDD when checking COPYBF @75 150
	static const char proc[] = { _COPYBF|IND, 75, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[150], 50, "Flag 150 is not 50");
	ASSERT_EQUAL(flags[100], 50, "Flag 150 is not 50");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests RANDOM <flagno>
/*	Flag flagno. is set to a number from the Pseudo-random sequence from 1
	to 100. */

void test_RANDOM_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 150 with value 255
	flags[150] = 255;

	//BDD when checking RANDOM 150
	static const char proc[] = { _RANDOM, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(flags[150] >= 1, "Flag 150 is 0");
	ASSERT(flags[150] <= 100, "Flag 150 is greater than 100");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_RANDOM_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a flag 150 with value 255, and flag 75 with valur 150
	flags[75] = 150;
	flags[150] = 255;

	//BDD when checking RANDOM @75
	static const char proc[] = { _RANDOM|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT(flags[150] >= 1, "Flag 150 is 0");
	ASSERT(flags[150] <= 100, "Flag 150 is greater than 100");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests MOVE <flagno>
/*	This is a very powerful action designed to manipulate PSI's. It allows the
	current LS Verb to be used to scan the connections section for the location
	given in Flag flagno.
	If the Verb is found then Flag flagno is changed to be the location number
	associated with it, and the next condact is considered.
	If the verb is not found, or the original location number was invalid, then
	PAW considers the next entry in the table - if present. */

void test_MOVE_success()
{
	TODO("Must mock Verbs table and Connections table");
}

void test_MOVE_indirection()
{
	TODO("Must mock Verbs table and Connections table");
}

// =============================================================================
// Actions to manipulate player flags [3 condacts]
// =============================================================================

// =============================================================================
// Tests GOTO <locno>
/*	Changes the current location to locno. This effectively sets flag 38 to the value
	locno. */

void test_GOTO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5
	flags[fPlayer] = 5;

	//BDD when checking GOTO 1
	static const char proc[] = { _GOTO, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fPlayer], 1, "Player not moved");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_GOTO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 5, and flag 75 with value 1
	flags[75] = 1;
	flags[fPlayer] = 5;

	//BDD when checking GOTO @75
	static const char proc[] = { _GOTO|IND, 75, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fPlayer], 1, "Player not moved");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WEIGHT <flagno>
/*	Calculates the true weight of all objects carried and worn by the player
	(i.e. any containers will have the weight of their contents added up to a
	maximum of 255), this value is then placed in Flag flagno.
	This would be useful to ensure the player was not carrying too much weight
	to cross a bridge without it collapsing etc. */

void test_WEIGHT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given objects 1 (carried, weight 5), 2 (carried, weight 3), 3 (worn, weight 2)
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.weight = 5;
	objects[2].location = LOC_CARRIED;
	objects[2].attribs.mask.weight = 3;
	objects[3].location = LOC_WORN;
	objects[3].attribs.mask.weight = 2;

	//BDD when checking WEIGHT 100
	static const char proc[] = { _WEIGHT, 100, 255 };
	do_action(proc);

	//BDD then flags[100] == 10 (5+3+2)
	ASSERT_EQUAL(flags[100], 10, "WEIGHT must store total carried+worn weight in flag");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ABILITY <value1> <value2>
/*	This sets Flag 37, the maximum number of objects conveyable, to value 1 and
	Flag 52, the maximum weight of objects the player may carry and wear at any
	one time (or their strength), to be value 2 .
	No checks are made to ensure that the player is not already carrying more
	than the maximum. GET and so on, which check the values, will still work
	correctly and prevent the player carrying any more objects, even if you set
	the value lower than that which is already carried! */

void test_ABILITY_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given fMaxCarr with value 5, and fStrength with value 50
	flags[fMaxCarr] = 5;
	flags[fStrength] = 50;

	//BDD when checking ABILITY 10 100
	static const char proc[] = { _ABILITY, 10, 100, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fMaxCarr], 10, "Flag fMaxCarr bad value");
	ASSERT_EQUAL(flags[fStrength], 100, "Flag fStrength bad value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_ABILITY_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 10, fMaxCarr with value 5, and fStrength with value 50
	flags[75] = 10;
	flags[fMaxCarr] = 5;
	flags[fStrength] = 50;

	//BDD when checking ABILITY @75 100
	static const char proc[] = { _ABILITY|IND, 75, 100, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(flags[fMaxCarr], 10, "Flag fMaxCarr bad value");
	ASSERT_EQUAL(flags[fStrength], 100, "Flag fStrength bad value");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}


// =============================================================================
// PRP009 — Tests WEIGH: additional edge cases
// Spec (Manual 1991 L.1447): "if it is a container, any objects inside have their weight added"
// Spec (Manual 1991 L.1450): "maximum value of 255 which will not be exceeded"
// Spec (Manual 1991 L.1451): "container of zero weight, Flag flagno. will be cleared"

// Tests WEIGH - zero-weight container clears the flag
void test_WEIGH_zero_weight_container()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj3 = zero-weight container with obj4 inside (weight 50)
	flags[100] = 255;
	objects[3].attribs.mask.isContainer = 1;
	objects[3].attribs.mask.weight = 0;
	objects[4].location = 3;				// inside container obj3
	objects[4].attribs.mask.weight = 50;

	//BDD when WEIGH 3 100
	static const char proc[] = { _WEIGH, 3, 100, 255 };
	do_action(proc);

	//BDD then flags[100] = 0 (zero-weight container -> flag cleared)
	ASSERT_EQUAL(flags[100], 0, "WEIGH zero-weight container must set flag to 0");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// Tests WEIGH - container with non-zero weight and contents
void test_WEIGH_container_with_contents()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj3 = non-zero container (w=5), obj4 inside (w=3)
	objects[3].attribs.mask.isContainer = 1;
	objects[3].attribs.mask.weight = 5;
	objects[4].location = 3;	// inside container obj3
	objects[4].attribs.mask.weight = 3;

	//BDD when WEIGH 3 100
	static const char proc[] = { _WEIGH, 3, 100, 255 };
	do_action(proc);

	//BDD then flags[100] = 8 (5 + 3) (Manual L.1447)
	ASSERT_EQUAL(flags[100], 8, "WEIGH container must include contents weight (5+3=8)");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// Tests WEIGH - result capped at 255
void test_WEIGH_cap_at_255()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj3 = container (w=63), objs 4-7 inside (w=63 each)
	//     63 + 4*63 = 315 > 255 -> must be capped at 255 (Manual L.1450)
	objects[3].attribs.mask.isContainer = 1;
	objects[3].attribs.mask.weight = 63;
	objects[4].location = 3; objects[4].attribs.mask.weight = 63;
	objects[5].location = 3; objects[5].attribs.mask.weight = 63;
	objects[6].location = 3; objects[6].attribs.mask.weight = 63;
	objects[7].location = 3; objects[7].attribs.mask.weight = 63;

	//BDD when WEIGH 3 100
	static const char proc[] = { _WEIGH, 3, 100, 255 };
	do_action(proc);

	//BDD then flags[100] = 255 (capped)
	ASSERT_EQUAL(flags[100], 255, "WEIGH must cap result at 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// Tests WEIGH - empty container (no contents)
void test_WEIGH_empty_container()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj3 = non-zero container (w=10) with no objects inside
	objects[3].attribs.mask.isContainer = 1;
	objects[3].attribs.mask.weight = 10;

	//BDD when WEIGH 3 100
	static const char proc[] = { _WEIGH, 3, 100, 255 };
	do_action(proc);

	//BDD then flags[100] = 10 (container own weight only, no contents to add)
	ASSERT_EQUAL(flags[100], 10, "WEIGH empty container must return only its own weight");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// Tests WEIGH - NOT_CREATED object: location does not affect weight field
void test_WEIGH_not_created_object()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj3 with location=LOC_NOTCREATED but weight field=7
	//     WEIGH reads .weight directly; .location is irrelevant (Manual L.1447)
	objects[3].location = LOC_NOTCREATED;
	objects[3].attribs.mask.weight = 7;

	//BDD when WEIGH 3 100
	static const char proc[] = { _WEIGH, 3, 100, 255 };
	do_action(proc);

	//BDD then flags[100] = 7 (weight field is independent of creation status)
	ASSERT_EQUAL(flags[100], 7, "WEIGH must return weight regardless of object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// Tests WEIGH - nested containers: recursive weight accumulation
void test_WEIGH_nested_containers()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given:
	//   obj3 = container, weight=2
	//   obj4 = inside obj3, container, weight=2
	//   obj5 = inside obj4, plain object, weight=2
	//   Expected: 2 (obj3) + 2 (obj4) + 2 (obj5) = 6
	//   Spec: "nested containers stop adding their contents after ten levels" (Manual L.1448)
	objects[3].attribs.mask.isContainer = 1;
	objects[3].attribs.mask.weight = 2;
	objects[4].location = 3;	// inside obj3
	objects[4].attribs.mask.isContainer = 1;
	objects[4].attribs.mask.weight = 2;
	objects[5].location = 4;	// inside obj4
	objects[5].attribs.mask.weight = 2;

	//BDD when WEIGH 3 100
	static const char proc[] = { _WEIGH, 3, 100, 255 };
	do_action(proc);

	//BDD then flags[100] = 6 (recursive sum: 2 + 2 + 2)
	ASSERT_EQUAL(flags[100], 6, "WEIGH nested containers must accumulate weights recursively (2+2+2=6)");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// Tests WEIGH - object location (CARRIED/WORN) does not affect result
void test_WEIGH_location_irrelevant()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj3 carried (LOC_CARRIED), weight=5
	//     WEIGH reads the .weight field; .location is irrelevant (Manual L.1447)
	objects[3].location = LOC_CARRIED;
	objects[3].attribs.mask.weight = 5;

	//BDD when WEIGH 3 100
	static const char proc[] = { _WEIGH, 3, 100, 255 };
	do_action(proc);

	//BDD then flags[100] = 5 (same as if obj3 were at any location)
	ASSERT_EQUAL(flags[100], 5, "WEIGH result must be independent of object location");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WHATO - search priority: CARRIED before HERE
void test_WHATO_priority_carried_over_here()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given two objects with same nounId=10: obj1 at player location (HERE), obj2 carried
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[1].location = 5;				// HERE
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.weight = 3;
	objects[2].location = LOC_CARRIED;		// CARRIED (higher priority)
	objects[2].nounId = 10;
	objects[2].adjectiveId = NULLWORD;
	objects[2].attribs.mask.weight = 7;

	static const char proc[] = { _WHATO, 255 };
	do_action(proc);

	//BDD then WHATO must reference obj2 (carried has priority over here)
	ASSERT_EQUAL(flags[fCONum], 2, "WHATO must find carried object (priority over here)");
	ASSERT_EQUAL(flags[fCOLoc], LOC_CARRIED, "Current object location must be LOC_CARRIED");
	ASSERT_EQUAL(flags[fCOWei], 7, "Current object weight must be 7");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PLUS - exact boundary: 255+1 must remain 255 (no wrap-around)
void test_PLUS_exact_boundary()
{
	const char *_func = __func__;
	beforeEach();

	flags[100] = 255;
	static const char proc[] = { _PLUS, 100, 1, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[100], 255, "PLUS at 255+1 must clamp to 255, not wrap");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests MINUS - exact boundary: 0-1 must remain 0 (no wrap-around)
void test_MINUS_exact_boundary()
{
	const char *_func = __func__;
	beforeEach();

	flags[100] = 0;
	static const char proc[] = { _MINUS, 100, 1, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[100], 0, "MINUS at 0-1 must clamp to 0, not wrap");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ADD - max overflow: 255+255 must clamp to 255
void test_ADD_max_overflow()
{
	const char *_func = __func__;
	beforeEach();

	flags[100] = 255;
	flags[150] = 255;
	static const char proc[] = { _ADD, 100, 150, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[150], 255, "ADD 255+255 must clamp to 255, not wrap");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SUB - zero underflow: 0-5 must clamp to 0
void test_SUB_zero_underflow()
{
	const char *_func = __func__;
	beforeEach();

	flags[100] = 5;
	flags[150] = 0;
	static const char proc[] = { _SUB, 100, 150, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[150], 0, "SUB 0-5 must clamp to 0, not wrap");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests WEIGHT - overflow cap at 255
void test_WEIGHT_overflow_cap()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given 5 carried objects with weight 63 each: total = 315, capped to 255
	uint8_t i;
	for (i=1; i<=5; i++) {
		objects[i].location = LOC_CARRIED;
		objects[i].attribs.mask.weight = 63;
	}

	static const char proc[] = { _WEIGHT, 100, 255 };
	do_action(proc);

	ASSERT_EQUAL(flags[100], 255, "WEIGHT total >255 must be capped at 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests ABILITY - setting limits below current carried count does not drop objects
void test_ABILITY_overloaded()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player carrying 10 objects (more than new limit of 3)
	flags[fNOCarr] = 10;
	flags[fMaxCarr] = 10;
	flags[fStrength] = 100;

	static const char proc[] = { _ABILITY, 3, 20, 255 };
	do_action(proc);

	//BDD then limits are set but objects are NOT dropped (spec: "No checks are made")
	ASSERT_EQUAL(flags[fMaxCarr], 3, "ABILITY must set fMaxCarr to 3");
	ASSERT_EQUAL(flags[fStrength], 20, "ABILITY must set fStrength to 20");
	ASSERT_EQUAL(flags[fNOCarr], 10, "ABILITY must not drop carried objects");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests COPYFO - source flag = 255 (NULLWORD): sets object location to 255 and calls errorCode
void test_COPYFO_nullword()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 200 = 255 (NULLWORD), object 1 at loc 5
	flags[200] = 255;
	objects[1].location = 5;

	//BDD when COPYFO 200 1
	static const char proc[] = { _COPYFO, 200, 1, 255 };
	do_action(proc);

	//BDD then objects[1].location = 255 and errorCode(2) was called (stubbed as nop)
	ASSERT_EQUAL(objects[1].location, 255, "COPYFO with flag=255 must still set location to 255");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests RANDOM - result always in range [1..100] over 20 iterations
void test_RANDOM_always_in_range()
{
	const char *_func = __func__;
	beforeEach();

	uint8_t i;
	for (i=0; i<20; i++) {
		static const char proc[] = { _RANDOM, 100, 255 };
		do_action(proc);
		ASSERT(flags[100] >= 1, "RANDOM must never return 0");
		ASSERT(flags[100] <= 100, "RANDOM must never exceed 100");
	}
	SUCCEED();
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS CONDACTS (4/6) ###\n\r### (Object data + flag ops + movement) ###\n\r");
	beforeAll();

	test_COPYOO_success(); test_COPYOO_indirection();
	test_RESET_success();
	test_COPYOF_success(); test_COPYOF_indirection();
	test_COPYFO_success(); test_COPYFO_indirection();
	test_COPYFO_nullword();
	test_WHATO_success();
	test_WHATO_priority_carried_over_here();
	test_SETCO_success(); test_SETCO_indirection();
	test_WEIGH_success(); test_WEIGH_indirection();
	test_WEIGH_zero_weight_container();
	test_WEIGH_container_with_contents();
	test_WEIGH_cap_at_255();
	test_WEIGH_empty_container();
	test_WEIGH_not_created_object();
	test_WEIGH_nested_containers();
	test_WEIGH_location_irrelevant();

	test_SET_success(); test_SET_indirection();
	test_CLEAR_success(); test_CLEAR_indirection();
	test_LET_success(); test_LET_indirection();
	test_PLUS_success(); test_PLUS_overflow(); test_PLUS_indirection();
	test_PLUS_exact_boundary();
	test_MINUS_success(); test_MINUS_overflow(); test_MINUS_indirection();
	test_MINUS_exact_boundary();
	test_ADD_success(); test_ADD_overflow(); test_ADD_indirection();
	test_ADD_max_overflow();
	test_SUB_success(); test_SUB_overflow(); test_SUB_indirection();
	test_SUB_zero_underflow();
	test_COPYFF_success(); test_COPYFF_indirection();
	test_COPYBF_success(); test_COPYBF_indirection();
	test_RANDOM_success(); test_RANDOM_indirection();
	test_RANDOM_always_in_range();
	test_MOVE_success(); test_MOVE_indirection();

	test_GOTO_success(); test_GOTO_indirection();
	test_WEIGHT_success();
	test_WEIGHT_overflow_cap();
	test_ABILITY_success(); test_ABILITY_indirection();
	test_ABILITY_overloaded();

	return 0;
}
