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
// Actions for object position manipulation [21 condacts]
// =============================================================================

// =============================================================================
// Tests GET <objno>
/*	If Object objno. is worn or carried, SM25 ("I already have the _.") is printed
	and actions NEWTEXT & DONE are performed.

	If Object objno. is not at the current location, SM26 ("There isn't one of
	those here.") is printed and actions NEWTEXT & DONE are performed.

	If the total weight of the objects carried and worn by the player plus
	Object objno. would exceed the maximum conveyable weight (Flag 52) then SM43
	("The _ weighs too much for me.") is printed and actions NEWTEXT & DONE are
	performed.

	If the maximum number of objects is being carried (Flag 1 is greater than,
	or the same as, Flag 37), SM27 ("I can't carry any more things.") is printed
	and actions NEWTEXT & DONE are performed. In addition any current DOALL loop
	is cancelled.

	Otherwise the position of Object objno. is changed to carried, Flag 1 is
	incremented and SM36 ("I now have the _.") is printed. */

void test_GET_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	objects[1].location = LOC_CARRIED;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at another location than the player
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 26, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_maxWeight()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 with weight (50), more than player can carry (25)
	flags[fPlayer] = 1;
	flags[fStrength] = 25;
	objects[1].location = 1;
	objects[1].attribs.mask.weight = 50;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 43, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_maxObjs()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at same location than player, but exceeds max number of objected carried
	flags[fPlayer] = 1;
	flags[fNOCarr] = 10;
	flags[fMaxCarr] = 10;
	objects[1].location = 1;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 27, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_GET_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at same location than player, and could be carried
	flags[fPlayer] = 1;
	flags[fMaxCarr] = 255;
	objects[1].location = 1;

	//BDD when checking GET 1
	static const char proc[] = { _GET, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(fake_lastSysMesPrinted, 36, ERROR_SYSMES);
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DROP <objno>
/*	If Object objno. is worn then SM24 ("I can't. I'm wearing the _.") is
	printed and actions NEWTEXT & DONE are performed.

	If Object objno. is at the current location (but neither worn nor carried),
	SM49 ("I don't have the _.") is printed and actions NEWTEXT & DONE are
	performed.

	If Object objno. is not at the current location then SM28 ("I don't have one
	of those.") is printed and actions NEWTEXT & DONE are performed.

	Otherwise the position of Object objno. is changed to the current location,
	Flag 1 is decremented and SM39 ("I've dropped the _.") is printed. */

void test_DROP_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	flags[fPlayer] = 1;
	flags[fMaxCarr] = 255;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking DROP 1
	static const char proc[] = { _DROP, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(fake_lastSysMesPrinted, 39, ERROR_SYSMES);
	ASSERT_EQUAL(objects[1].location, flags[fPlayer], "Droped object not here");
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_DROP_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking DROP 1
	static const char proc[] = { _DROP, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 24, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_DROP_isHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 placed at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking DROP 1
	static const char proc[] = { _DROP, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_DROP_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 placed at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking DROP 1
	static const char proc[] = { _DROP, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 28, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}


// =============================================================================
// Tests WEAR <objno>
/*	If Object objno. is at the current location (but not carried or worn) SM49
	("I don't have the _.") is printed and actions NEWTEXT & DONE are
	performed.

	If Object objno. is worn, SM29 ("I'm already wearing the _.") is printed
	and actions NEWTEXT & DONE are performed.

	If Object objno. is not carried, SM28 ("I don't have one of those.") is
	printed and actions NEWTEXT & DONE are performed.

	If Object objno. is not wearable (as specified in the object definition
	section) then SM40 ("I can't wear the _.") is printed and actions NEWTEXT &
	DONE are performed.

	Otherwise the position of Object objno. is changed to worn, Flag 1 is
	decremented and SM37 ("I'm now wearing the _.") is printed. */

void test_WEAR_isHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 placed at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 1;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 29, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_notCarried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a not carried object 1 and player at location 1
	flags[fPlayer] = 1;
	objects[1].location = LOC_NOTCREATED;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 28, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_notWareable()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 but is not wareable
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.isWareable = 0;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 40, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_WEAR_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a wareable and carried object 1
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking WEAR 1
	static const char proc[] = { _WEAR, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(fake_lastSysMesPrinted, 37, ERROR_SYSMES);
	ASSERT_EQUAL(objects[1].location, LOC_WORN, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests REMOVE <objno>
/*	If Object objno. is carried or at the current location (but not worn) then
	SM50 ("I'm not wearing the _.") is printed and actions NEWTEXT & DONE are
	performed.

	If Object objno. is not at the current location, SM23 ("I'm not wearing one
	of those.") is printed and actions NEWTEXT & DONE are performed.

	If Object objno. is not wearable (and thus removable) then SM41 ("I can't
	remove the _.") is printed and actions NEWTEXT & DONE are performed.

	If the maximum number of objects is being carried (Flag 1 is greater than,
	or the same as, Flag 37), SM42 ("I can't remove the _. My hands are full.")
	is printed and actions NEWTEXT & DONE are performed.

	Otherwise the position of Object objno. is changed to carried. Flag 1 is
	incremented and SM38 ("I've removed the _.") printed. */

void test_REMOVE_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	objects[1].location = LOC_CARRIED;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 50, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_isHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a object 1 at same location than player
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 50, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a object 1 at different location than player
	flags[fPlayer] = 1;
	objects[1].location = LOC_NOTCREATED - 1;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 23, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_notWareable()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried and not wareable object 1
	flags[fPlayer] = 1;
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 0;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 41, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_maxObjs()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1 but max carried objects reached
	flags[fNOCarr] = 10;
	flags[fMaxCarr] = 10;
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 42, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_REMOVE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 10;
	objects[1].location = LOC_WORN;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when checking REMOVE 1
	static const char proc[] = { _REMOVE, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(fake_lastSysMesPrinted, 38, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}


// =============================================================================
// Tests CREATE <objno>
/*	The position of Object objno. is changed to the current location and Flag 1
	is decremented if the object was carried. */

void test_CREATE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a uncreated object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CREATE 1
	static const char proc[] = { _CREATE, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, flags[fPlayer], ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CREATE_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking CREATE 1
	static const char proc[] = { _CREATE, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, flags[fPlayer], ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_CREATE_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a uncreated object 1 and the player at location 2 and flag 150 with value 1
	flags[150] = 1;
	flags[fPlayer] = 2;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CREATE @150
	static const char proc[] = { _CREATE|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, flags[fPlayer], ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DESTROY <objno>
/*	The position of Object objno. is changed to not-created and Flag 1 is
	decremented if the object was carried. */

void test_DESTROY_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = 1;

	//BDD when checking DESTROY 1
	static const char proc[] = { _DESTROY, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_NOTCREATED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_DESTROY_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 and the player at location 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking DESTROY 1
	static const char proc[] = { _DESTROY, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_NOTCREATED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_DESTROY_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 and the player at location 2 and flag 150 with value 1
	flags[150] = 1;
	flags[fPlayer] = 2;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_NOTCREATED;

	//BDD when checking CREATE @150
	static const char proc[] = { _DESTROY|IND, 150, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_NOTCREATED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests SWAP <objno1> <objno2>
/*	The positions of the two objects are exchanged. Flag 1 is not adjusted. The
	currently referenced object is set to be Object objno 2. */

void test_SWAP_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc1, and object 2 carried
	flags[fNOCarr] = 1;
	objects[1].location = 1;
	objects[2].location = LOC_CARRIED;

	//BDD when checking SWAP 1 2
	static const char proc[] = { _SWAP, 1, 2, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 2, "Current object is not object2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_SWAP_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc1, object 2 carried, and flag 150 with value 1
	flags[150] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = 1;
	objects[2].location = 2;

	//BDD when checking SWAP @150 2
	static const char proc[] = { _SWAP|IND, 150, 2, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 2, "Current object is not object2");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PLACE <objno> <locno+>
/*	The position of Object objno. is changed to Location locno. Flag 1 is
	decremented if the object was carried. It is incremented if the object is
	placed at location 254 (carried). */

void test_PLACE_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 1
	flags[fNOCarr] = 0;
	objects[1].location = 1;

	//BDD when checking PLACE 1 2
	static const char proc[] = { _PLACE, 1, LOC_CARRIED, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object1");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PLACE_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1, and flag 150 with value 1
	flags[150] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking PLACE @150 1
	static const char proc[] = { _PLACE|IND, 150, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object1");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PUTO <locno+>
/*	The position of the currently referenced object (i.e. that object whose
	number is given in flag 51), is changed to be Location locno. Flag 54
	remains its old location. Flag 1 is decremented if the object was carried.
	It is incremented if the object is placed at location 254 (carried). */

void test_PUTO_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 referenced
	flags[fNOCarr] = 0;
	flags[fCONum] = 1;
	flags[fCOLoc] = 1;
	objects[1].location = 1;

	//BDD when checking PUTO LOC_CARRIED
	static const char proc[] = { _PUTO, LOC_CARRIED, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object1");
	ASSERT_EQUAL(flags[fCOLoc], 1, "Flag fCOLoc has changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PUTO_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1, and flag 150 with value 2
	flags[150] = 2;
	flags[fNOCarr] = 1;
	flags[fCOLoc] = LOC_CARRIED;
	objects[1].location = LOC_CARRIED;

	//BDD when checking PUTO @150
	static const char proc[] = { _PUTO|IND, 150, 1, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT_EQUAL(flags[fCONum], 1, "Current object is not object1");
	ASSERT_EQUAL(flags[fCOLoc], LOC_CARRIED, "Flag fCOLoc has changed");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PUTIN <objno> <locno>
/*	If Object objno. is worn then SM24 ("I can't. I'm wearing the _.") is
	printed and actions NEWTEXT & DONE are performed.

	If Object objno. is at the current location (but neither worn nor carried),
	SM49 ("I don't have the _.") is printed and actions NEWTEXT & DONE are
	performed.

	If Object objno. is not at the current location, but not carried, then SM28
	("I don't have one of those.") is printed and actions NEWTEXT & DONE are
	performed.

	Otherwise the position of Object objno. is changed to Location locno.
	Flag 1 is decremented and SM44 ("The _ is in the"), a description of Object
	locno. and SM51 (".") is printed. */

void test_PUTIN_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking PUTIN 1 2
	static const char proc[] = { _PUTIN, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 24, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PUTIN_here()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 here
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking PUTIN 1 2
	static const char proc[] = { _PUTIN, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PUTIN_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 here
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking PUTIN 1 2
	static const char proc[] = { _PUTIN, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 28, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_PUTIN_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking PUTIN 1 2
	static const char proc[] = { _PUTIN, 1, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT_EQUAL(fake_lastSysMesPrinted, 44, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_PUTIN_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 1, and a carried object 1
	flags[75] = 1;
	flags[fPlayer] = 1;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;

	//BDD when checking PUTIN @75 2
	static const char proc[] = { _PUTIN|IND, 75, 2, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, 2, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT_EQUAL(fake_lastSysMesPrinted, 44, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests TAKEOUT <objno> <locno>
/*	If Object objno. is worn or carried, SM25 ("I already have the _.") is printed
	and actions NEWTEXT & DONE are performed.

	If Object objno. is at the current location, SM45 ("The _ isn't in the"), a
	description of Object locno. and SM51 (".") is printed and actions NEWTEXT &
	DONE are performed.

	If Object objno. is not at the current location and not at Location locno.
	then SM52 ("There isn't one of those in the"), a description of Object locno.
	and SM51 (".") is printed and actions NEWTEXT & DONE are performed.

	If Object locno. is not carried or worn, and the total weight of the objects
	carried and worn by the player plus Object objno. would exceed the maximum
	conveyable weight (Flag 52) then SM43 ("The _ weighs too much for me.") is
	printed and actions NEWTEXT & DONE are performed.

	If the maximum number of objects is being carried (Flag 1 is greater than,
	or the same as, Flag 37), SM27 ("I can't carry any more things.") is printed
	and actions NEWTEXT & DONE are performed. In addition any current DOALL loop
	is cancelled.

	Otherwise the position of Object objno. is changed to carried, Flag 1 is
	incremented and SM36 ("I now have the _.") is printed.Note: No check is made,
	by either PUTIN or TAKEOUT, that Object locno. is actually present. This must
	be carried out by you if required. */

void test_TAKEOUT_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1
	objects[1].location = LOC_CARRIED;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1
	objects[1].location = LOC_WORN;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_here()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 that is here
	flags[fPlayer] = 1;
	objects[1].location = 1;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 45, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_notHere()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 that is here, and not at loc 3
	flags[fPlayer] = 1;
	objects[1].location = 2;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 52, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_maxWeight()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 3, and total weight of carried/worn objects + object 1 exceeds the maximum
	flags[fPlayer] = 1;
	flags[fStrength] = 1;
	objects[1].location = 3;
	objects[1].attribs.mask.weight = 7;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 43, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_maxObjs()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 3, and total num of carried objects + object 1 exceeds the maximum
	flags[fPlayer] = 1;
	flags[fNOCarr] = 2;
	flags[fMaxCarr] = 2;
	objects[1].location = 3;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(fake_lastSysMesPrinted, 27, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given an object 1 at loc 3, and total num of carried objects + object 1 no exceeds the maximum
	flags[fPlayer] = 1;
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 1;
	objects[1].location = 3;

	//BDD when checking TAKEOUT 1 3
	static const char proc[] = { _TAKEOUT, 1, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(fake_lastSysMesPrinted, 36, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_TAKEOUT_indirection()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given flag 75 with value 1, an object 1 at loc 3, and total num of carried objects + object 1 no exceeds the maximum
	flags[75] = 1;
	flags[fPlayer] = 2;
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 1;
	objects[1].location = 3;

	//BDD when checking TAKEOUT @75 3
	static const char proc[] = { _TAKEOUT|IND, 75, 3, 255 };
	do_action(proc);

	//BDD then fails
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(fake_lastSysMesPrinted, 36, ERROR_SYSMES);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests DROPALL
/*	All objects which are carried or worn are created at the current location (i.e.
	all objects are dropped) and Flag 1 is set to 0. This is included for
	compatibility with older writing systems.
	Note that a DOALL 254 will carry out a true DROP ALL, taking care of any special
	actions included. */

void test_DROPALL_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given 3 objects carried or worn
	flags[fPlayer] = 1;
	flags[fNOCarr] = 3;
	objects[1].location = LOC_WORN;
	objects[2].location = LOC_CARRIED;
	objects[3].location = LOC_CARRIED;

	//BDD when checking DROPALL
	static const char proc[] = { _DROPALL, 255 };
	do_action(proc);

	//BDD then success
	ASSERT_EQUAL(objects[1].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[2].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(objects[3].location, 1, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// Regression test for PRP002 (do_DROPALL off-by-one).
// The old buggy loop iterated 0..numObjDsc (inclusive), reading one byte past
// the objects array. beforeAll() allocates windows[] immediately after
// objects[] via the bump allocator, so windows[0].winX physically overlaps
// where objects[MOCK_NUM_OBJECTS].location would be. Placing LOC_CARRIED
// there triggers the buggy overwrite; the fixed loop stops at numObjDsc-1
// and leaves the canary intact.
void test_DROPALL_no_overflow()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given player at loc 7 and a canary just past the objects array
	flags[fPlayer] = 7;
	windows[0].winX = LOC_CARRIED;

	//BDD when executing DROPALL
	static const char proc[] = { _DROPALL, 255 };
	do_action(proc);

	//BDD then the canary is untouched
	ASSERT_EQUAL(windows[0].winX, LOC_CARRIED, "DROPALL iterated past numObjDsc");
	SUCCEED();
}

// =============================================================================
// Tests AUTOG
/*	A search for the object number represented by Noun(Adjective)1 is made in
	the object definition section in order of location priority; here, carried,
	worn. i.e. The player is more likely to be trying to GET an object that is
	at the current location than one that is carried or worn. If an object is
	found its number is passed to the GET action. Otherwise if there is an
	object in existence anywhere in the game or if Noun1 was not in the
	vocabulary then SM26 ("There isn't one of those here.") is printed. Else
	SM8 ("I can't do that.") is printed (i.e. It is not a valid object but does
	exist in the game). Either way actions NEWTEXT & DONE are performed */

void test_AUTOG_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 with nounId=10
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	//BDD when AUTOG executes - finds object via LOC_CARRIED, then _internal_get sees it's carried
	static const char proc[] = { _AUTOG, 255 };
	do_action(proc);

	//BDD then _internal_get prints SM25 and calls DONE
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, "already carried - must stay carried");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AUTOG_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1 with nounId=10
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 0;
	objects[1].location = LOC_WORN;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	//BDD when AUTOG executes - finds object via LOC_WORN, then _internal_get sees it's worn
	static const char proc[] = { _AUTOG, 255 };
	do_action(proc);

	//BDD then _internal_get prints SM25 and calls DONE
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT_EQUAL(objects[1].location, LOC_WORN, "worn - must stay worn");
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AUTOG_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given object 1 at player's location with nounId=10
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 4;
	flags[fStrength] = 20;
	objects[1].location = 5;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.weight = 2;

	//BDD when AUTOG executes - finds object at player's location, _internal_get picks it up
	static const char proc[] = { _AUTOG, 255 };
	do_action(proc);

	//BDD then object is carried and fNOCarr is incremented
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, "must carry the object");
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests AUTOD
/*	A search for the object number represented by Noun(Adjective)1 is made in
	the object definition section in order of location priority; carried, worn,
	here. i.e. The player is more likely to be trying to DROP a carried object
	than one that is worn or here. If an object is found its number is passed
	to the DROP action. Otherwise if there is an object in existence anywhere
	in the game or if Noun1 was not in the vocabulary then SM28 ("I don't have
	one of those.") is printed. Else SM8 ("I can't do that.") is printed (i.e.
	It is not a valid object but does exist in the game). Either way actions
	NEWTEXT & DONE are performed */

void test_AUTOD_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 with nounId=10
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	//BDD when AUTOD executes - finds object via LOC_CARRIED, _internal_drop drops it
	static const char proc[] = { _AUTOD, 255 };
	do_action(proc);

	//BDD then object is at player's location and fNOCarr is decremented
	ASSERT_EQUAL(objects[1].location, 5, "must drop to player location");
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	SUCCEED();
}

// =============================================================================
// Tests AUTOW
/*	A search for the object number represented by Noun(Adjective)1 is made in
	the object definition section in order of location priority; carried, worn,
	here. i.e. The player is more likely to be trying to WEAR a carried object
	than one that is worn or here. If an object is found its number is passed
	to the WEAR action. Otherwise if there is an object in existence anywhere
	in the game or if Noun1 was not in the vocabulary then SM28 ("I don't have
	one of those.") is printed. Else SM8 ("I can't do that.") is printed (i.e.
	It is not a valid object but does exist in the game). Either way actions
	NEWTEXT & DONE are performed */

void test_AUTOW_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried and wearable object 1 with nounId=10
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when AUTOW executes - finds object via LOC_CARRIED, _internal_wear wears it
	static const char proc[] = { _AUTOW, 255 };
	do_action(proc);

	//BDD then object is worn and fNOCarr is decremented
	ASSERT_EQUAL(objects[1].location, LOC_WORN, "must wear the object");
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	SUCCEED();
}

// =============================================================================
// Tests AUTOR
/*	A search for the object number represented by Noun(Adjective)1 is made in
	the object definition section in order of location priority; worn, carried,
	here. i.e. The player is more likely to be trying to REMOVE a worn object
	than one that is carried or here. If an object is found its number is passed
	to the REMOVE action. Otherwise if there is an object in existence anywhere
	in the game or if Noun1 was not in the vocabulary then SM23 ("I'm not
	wearing one of those.") is printed. Else SM8 ("I can't do that.") is printed
	(i.e. It is not a valid object but does exist in the game). Either way
	actions NEWTEXT & DONE are performed */

void test_AUTOR_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn and wearable object 1 with nounId=10
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 4;
	objects[1].location = LOC_WORN;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.isWareable = 1;

	//BDD when AUTOR executes - finds object via LOC_WORN, _internal_remove removes it
	static const char proc[] = { _AUTOR, 255 };
	do_action(proc);

	//BDD then object is carried and fNOCarr is incremented
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, "must carry after remove");
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	SUCCEED();
}

// =============================================================================
// Tests AUTOP
/*	A search for the object number represented by Noun(Adjective)1 is made in the
	object definition section in order of location priority; carried, worn, here.
	i.e. The player is more likely to be trying to PUT a carried object inside
	another than one that is worn or here. If an object is found its number is
	passed to the PUTIN action. Otherwise if there is an object in existence
	anywhere in the game or if Noun1 was not in the vocabulary then SM28 ("I don't
	have one of those.") is printed. Else SM8 ("I can't do that.") is printed
	(i.e. It is not a valid object but does exist in the game). Either way actions
	NEWTEXT & DONE are performed */

void test_AUTOP_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 with nounId=10, and obj0 is a container
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 1;
	objects[0].attribs.mask.isContainer = 1;
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	//BDD when AUTOP executes with container=obj0
	// AUTOP reads locno via getValueOrIndirection (from pPROC)
	static const char proc[] = { _AUTOP, 0, 255 };
	do_action(proc);

	//BDD then object is placed in container (location=0) and fNOCarr decremented
	ASSERT_EQUAL(objects[1].location, 0, "must be in container obj0");
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	SUCCEED();
}

// =============================================================================
// Tests AUTOT
/*	A search for the object number represented by Noun(Adjective)1 is made in the
	object definition section in order of location priority; in container,
	carried, worn, here. i.e. The player is more likely to be trying to get an
	object out of a container which is actually in there than one that is carried,
	worn or here. If an object is found its number is passed to the TAKEOUT action.
	Otherwise if there is an object in existence anywhere in the game or if Noun1
	was not in the vocabulary then SM52 ("There isn't one of those in the"), a
	description of Object locno. and SM51 (".") is printed. Else SM8 ("I can't do
	that.") is printed (i.e. It is not a valid object but does exist in the game).
	Either way actions NEWTEXT & DONE are performed */

void test_AUTOT_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1 with nounId=10, and container obj0.
	// AUTOT searches: LOC_CONTAINER (fails: LOC_CONTAINER=256 >= numObjDsc, see note),
	// then LOC_CARRIED (found!), calls _internal_takeout.
	// _internal_takeout: obj already carried -> SM25 "I already have the _." + DONE.
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 1;
	objects[0].attribs.mask.isContainer = 1;
	objects[0].attribs.mask.weight = 5;
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.weight = 2;

	static const char proc[] = { _AUTOT, 0, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AUTOT_success()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj0 is a container at some game location,
	//     obj1 is inside it (location=0) with nounId=10.
	// AUTOT searches LOC_CONTAINER first:
	//   getObjectId finds obj1 because objects[1].location=0 < numObjDsc
	//   AND objects[0].isContainer=1.
	// _internal_takeout(1, 0): obj1 not worn/carried/at fPlayer(5) -> success.
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 4;
	flags[fStrength] = 20;
	objects[0].attribs.mask.isContainer = 1;
	objects[0].attribs.mask.weight = 5;
	objects[1].location = 0;		// inside container obj0
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.weight = 2;

	static const char proc[] = { _AUTOT, 0, 255 };
	do_action(proc);

	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, "object must be carried after AUTOT");
	ASSERT_EQUAL(flags[fNOCarr], 1, ERROR_CARROBJNUM);
	ASSERT_EQUAL(fake_lastSysMesPrinted, 36, ERROR_SYSMES);
	SUCCEED();
}


// =============================================================================
// PRP009 — Tests AUTOG: priority and not-found cases
// Spec (Manual 1991 L.1306): "in order of location priority; here, carried, worn"

void test_AUTOG_priority_here_over_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 carried nounId=10, obj2 HERE nounId=10
	//     AUTOG priority is here > carried, so obj2 must be picked up
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 1;
	flags[fMaxCarr] = 4;
	flags[fStrength] = 20;
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.weight = 1;
	objects[2].location = 5;	// HERE
	objects[2].nounId = 10;
	objects[2].adjectiveId = NULLWORD;
	objects[2].attribs.mask.weight = 1;

	static const char proc[] = { _AUTOG, 255 };
	do_action(proc);

	//BDD then obj2 (here) is carried, obj1 still carried (Manual L.1306)
	ASSERT_EQUAL(objects[2].location, LOC_CARRIED, "AUTOG must pick up obj HERE before CARRIED");
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, "already-carried obj must stay carried");
	ASSERT_EQUAL(flags[fNOCarr], 2, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

void test_AUTOG_not_found()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 exists at loc 3 (not here/carried/worn); player at loc 5
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[1].location = 3;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	static const char proc[] = { _AUTOG, 255 };
	do_action(proc);

	//BDD then SM26 "There isn't one of those here." (Manual L.1311) + NEWTEXT
	ASSERT_EQUAL(fake_lastSysMesPrinted, 26, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	ASSERT(fake_clearLogicalSentences_calls > 0, "AUTOG fail must perform NEWTEXT (Manual L.1311)");
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests AUTOD: worn and not-found cases
// Spec (Manual 1991 L.1320): "in order of location priority; carried, worn, here"

void test_AUTOD_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 worn nounId=10; AUTOD finds it via worn, calls DROP
	//     DROP on worn object -> SM24 "I can't. I'm wearing the _." (Manual L.1147)
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[1].location = LOC_WORN;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.isWareable = 1;

	static const char proc[] = { _AUTOD, 255 };
	do_action(proc);

	//BDD then SM24 (Manual L.1320 + L.1147)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 24, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AUTOD_not_found()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 exists at loc 3 (not carried/worn/here); player at loc 5
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[1].location = 3;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	static const char proc[] = { _AUTOD, 255 };
	do_action(proc);

	//BDD then SM28 "I don't have one of those." (Manual L.1324) + NEWTEXT
	ASSERT_EQUAL(fake_lastSysMesPrinted, 28, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	ASSERT(fake_clearLogicalSentences_calls > 0, "AUTOD fail must perform NEWTEXT (Manual L.1324)");
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests AUTOW: worn and here cases
// Spec (Manual 1991 L.1333): "in order of location priority; carried, worn, here"

void test_AUTOW_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 already worn nounId=10; AUTOW finds it via worn, calls WEAR
	//     WEAR on worn object -> SM29 "I'm already wearing the _." (Manual L.1168)
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[1].location = LOC_WORN;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.isWareable = 1;

	static const char proc[] = { _AUTOW, 255 };
	do_action(proc);

	//BDD then SM29 (Manual L.1333 + L.1168)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 29, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AUTOW_here()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 HERE (at player's loc, not carried, not worn) nounId=10
	//     AUTOW finds it via here, calls WEAR
	//     WEAR on obj at current loc (not carried/worn) -> SM49 "I don't have the _."
	//     Note: SM49, NOT SM28. SM28 is for objects not at current loc AND not carried.
	//     (Manual L.1164: "at the current location (but not carried or worn) SM49")
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[1].location = 5;	// at player's location, not carried
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.isWareable = 1;

	static const char proc[] = { _AUTOW, 255 };
	do_action(proc);

	//BDD then SM49 (Manual L.1333 + L.1164)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests AUTOR: carried and not-found cases
// Spec (Manual 1991 L.1346): "in order of location priority; worn, carried, here"

void test_AUTOR_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 carried nounId=10; AUTOR finds it via carried, calls REMOVE
	//     REMOVE on carried obj -> SM50 "I'm not wearing the _." (Manual L.1183)
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;
	objects[1].attribs.mask.isWareable = 1;

	static const char proc[] = { _AUTOR, 255 };
	do_action(proc);

	//BDD then SM50 (Manual L.1346 + L.1183)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 50, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AUTOR_not_found()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 exists at loc 3 (not worn/carried/here); player at loc 5
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[1].location = 3;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	static const char proc[] = { _AUTOR, 255 };
	do_action(proc);

	//BDD then SM23 "I'm not wearing one of those." (Manual L.1350) + NEWTEXT
	ASSERT_EQUAL(fake_lastSysMesPrinted, 23, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	ASSERT(fake_clearLogicalSentences_calls > 0, "AUTOR fail must perform NEWTEXT (Manual L.1350)");
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests AUTOP: worn and here cases
// Spec (Manual 1991 L.1359): "in order of location priority; carried, worn, here"

void test_AUTOP_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 worn nounId=10; container obj0; AUTOP finds obj1 via worn, calls PUTIN
	//     PUTIN on worn object -> SM24 "I can't. I'm wearing the _." (Manual L.1235)
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[0].attribs.mask.isContainer = 1;
	objects[1].location = LOC_WORN;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	static const char proc[] = { _AUTOP, 0, 255 };
	do_action(proc);

	//BDD then SM24 (Manual L.1359 + L.1235)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 24, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AUTOP_here()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 HERE nounId=10; container obj0; AUTOP finds obj1 via here, calls PUTIN
	//     PUTIN on obj at current loc (not carried) -> SM49 "I don't have the _." (Manual L.1238)
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[0].attribs.mask.isContainer = 1;
	objects[1].location = 5;	// here
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	static const char proc[] = { _AUTOP, 0, 255 };
	do_action(proc);

	//BDD then SM49 (Manual L.1359 + L.1238)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 49, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// PRP009 — Tests AUTOT: worn and here cases
// Spec (Manual 1991 L.1373): "in order of location priority; in container, carried, worn, here"

void test_AUTOT_worn()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 worn nounId=10; container obj0
	//     AUTOT: container (fails, obj1 not inside), carried (fails), worn (found!)
	//     Calls TAKEOUT; TAKEOUT on worn -> SM25 "I already have the _." (Manual L.1252)
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[0].attribs.mask.isContainer = 1;
	objects[1].location = LOC_WORN;
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	static const char proc[] = { _AUTOT, 0, 255 };
	do_action(proc);

	//BDD then SM25 (Manual L.1373 + L.1252)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 25, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

void test_AUTOT_here()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 HERE nounId=10; container obj0 also here
	//     AUTOT: container (fails, obj1 not inside obj0), carried (fails), worn (fails), here (found!)
	//     Calls TAKEOUT; TAKEOUT on obj at current loc -> SM45 "The _ isn't in the" (Manual L.1255)
	flags[fPlayer] = 5;
	flags[fNoun1] = 10;
	flags[fAdject1] = NULLWORD;
	objects[0].attribs.mask.isContainer = 1;
	objects[0].location = 5;	// container also here
	objects[1].location = 5;	// obj1 here, not inside container
	objects[1].nounId = 10;
	objects[1].adjectiveId = NULLWORD;

	static const char proc[] = { _AUTOT, 0, 255 };
	do_action(proc);

	//BDD then SM45 (Manual L.1373 + L.1255)
	ASSERT_EQUAL(fake_lastSysMesPrinted, 45, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests GET - magic bag weight (zero-weight container contents don't count)
void test_GET_magic_bag_weight()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 = zero-weight container (magic bag) carried with obj2 inside (weight 100)
	//     obj3 at player location with weight 10; fStrength = 10
	// GET obj3 must succeed because magic bag contents weigh 0
	flags[fPlayer] = 5;
	flags[fStrength] = 10;
	flags[fNOCarr] = 1;
	flags[fMaxCarr] = 4;
	objects[1].location = LOC_CARRIED;
	objects[1].attribs.mask.isContainer = 1;
	objects[1].attribs.mask.weight = 0;		// zero-weight = magic bag
	objects[2].location = 1;				// inside magic bag
	objects[2].attribs.mask.weight = 63;	// heavy contents that MUST NOT count
	objects[3].location = 5;
	objects[3].attribs.mask.weight = 10;

	static const char proc[] = { _GET, 3, 255 };
	do_action(proc);

	//BDD then GET succeeds: magic bag (0) + obj3 (10) = 10 <= fStrength(10)
	ASSERT_EQUAL(objects[3].location, LOC_CARRIED, "GET must succeed with magic bag carried");
	ASSERT_EQUAL(flags[fNOCarr], 2, ERROR_CARROBJNUM);
	SUCCEED();
}

// =============================================================================
// Tests TAKEOUT - weight check when object is in a container
void test_TAKEOUT_weight_from_container()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 inside container obj0 (location=0), weight=60; fStrength=10
	// TAKEOUT obj1 from obj0 must fail with SM43 (too heavy)
	flags[fPlayer] = 5;
	flags[fStrength] = 10;
	flags[fNOCarr] = 0;
	flags[fMaxCarr] = 4;
	objects[0].attribs.mask.isContainer = 1;
	objects[1].location = 0;				// inside container obj0
	objects[1].attribs.mask.weight = 60;	// exceeds fStrength

	static const char proc[] = { _TAKEOUT, 1, 0, 255 };
	do_action(proc);

	ASSERT_EQUAL(fake_lastSysMesPrinted, 43, ERROR_SYSMES);
	ASSERT(!checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PLACE - from carried to carried: fNOCarr unchanged (decrement then increment)
void test_PLACE_carried_to_carried()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a carried object 1; fNOCarr=2
	flags[fNOCarr] = 2;
	objects[1].location = LOC_CARRIED;

	//BDD when PLACE 1 254 (place at LOC_CARRIED again)
	static const char proc[] = { _PLACE, 1, LOC_CARRIED, 255 };
	do_action(proc);

	//BDD then fNOCarr unchanged (decrement + increment cancel out)
	ASSERT_EQUAL(objects[1].location, LOC_CARRIED, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 2, "fNOCarr must be unchanged when placing carried obj back to carried");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests CREATE - from worn: fNOCarr must NOT decrement (only CARRIED decrements)
void test_CREATE_from_worn_no_decrement()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given a worn object 1; fNOCarr=2
	flags[fPlayer] = 5;
	flags[fNOCarr] = 2;
	objects[1].location = LOC_WORN;

	static const char proc[] = { _CREATE, 1, 255 };
	do_action(proc);

	//BDD then object moved to player location, fNOCarr unchanged (was worn, not carried)
	ASSERT_EQUAL(objects[1].location, 5, ERROR_OBJLOC);
	ASSERT_EQUAL(flags[fNOCarr], 2, "fNOCarr must not change when creating a worn object");
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}

// =============================================================================
// Tests PUTIN - no validation on container location (spec: "No check is made that Object locno. is actually present")
void test_PUTIN_remote_container()
{
	const char *_func = __func__;
	beforeEach();

	//BDD given obj1 carried, container obj2 at location 99 (far away, not at player loc)
	flags[fPlayer] = 5;
	flags[fNOCarr] = 1;
	objects[1].location = LOC_CARRIED;
	objects[2].location = 99;				// remote container, NOT at player location
	objects[2].attribs.mask.isContainer = 1;

	//BDD when PUTIN 1 2 (put obj1 into remote container obj2)
	static const char proc[] = { _PUTIN, 1, 2, 255 };
	do_action(proc);

	//BDD then PUTIN succeeds without validation of container presence
	ASSERT_EQUAL(objects[1].location, 2, "PUTIN must place object in container regardless of container location");
	ASSERT_EQUAL(flags[fNOCarr], 0, ERROR_CARROBJNUM);
	ASSERT(checkEntry, ERROR);
	SUCCEED();
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS CONDACTS (3/6) ###\n\r### (Object manipulation) ###\n\r");
	beforeAll();

	test_GET_carried(); test_GET_worn(); test_GET_notHere(); test_GET_maxWeight(); test_GET_maxObjs(); test_GET_success();
	test_GET_magic_bag_weight();
	test_DROP_success(); test_DROP_worn(); test_DROP_isHere(); test_DROP_notHere();
	test_WEAR_isHere(); test_WEAR_worn(); test_WEAR_notCarried(); test_WEAR_notWareable(); test_WEAR_success();
	test_REMOVE_carried(); test_REMOVE_isHere(); test_REMOVE_notHere(); test_REMOVE_notWareable(); test_REMOVE_maxObjs();
		test_REMOVE_success();
	test_CREATE_success(); test_CREATE_carried(); test_CREATE_indirection();
	test_CREATE_from_worn_no_decrement();
	test_DESTROY_success(); test_DESTROY_carried(); test_DESTROY_indirection();
	test_SWAP_success(); test_SWAP_indirection();
	test_PLACE_success(); test_PLACE_indirection();
	test_PLACE_carried_to_carried();
	test_PUTO_success(); test_PUTO_indirection();
	test_PUTIN_worn(); test_PUTIN_here(); test_PUTIN_notHere(); test_PUTIN_success(); test_PUTIN_indirection();
	test_PUTIN_remote_container();
	test_TAKEOUT_carried(); test_TAKEOUT_worn(); test_TAKEOUT_here(); test_TAKEOUT_notHere(); test_TAKEOUT_maxWeight();
		test_TAKEOUT_maxObjs(); test_TAKEOUT_success(); test_TAKEOUT_indirection();
	test_TAKEOUT_weight_from_container();
	test_DROPALL_success(); test_DROPALL_no_overflow();
	test_AUTOG_carried(); test_AUTOG_worn(); test_AUTOG_success();
	test_AUTOG_priority_here_over_carried(); test_AUTOG_not_found();
	test_AUTOD_success(); test_AUTOD_worn(); test_AUTOD_not_found();
	test_AUTOW_success(); test_AUTOW_worn(); test_AUTOW_here();
	test_AUTOR_success(); test_AUTOR_carried(); test_AUTOR_not_found();
	test_AUTOP_success(); test_AUTOP_worn(); test_AUTOP_here();
	test_AUTOT_carried(); test_AUTOT_success();
	test_AUTOT_worn(); test_AUTOT_here();

	return 0;
}
