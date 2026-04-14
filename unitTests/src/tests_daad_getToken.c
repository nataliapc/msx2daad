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


// Token data: [0]="THE", [1]="A", [2]="OF"
// "THE" = 'T'(0x54), 'H'(0x48), 'E'|0x80 (0xC5)
// "A"   = 'A'|0x80 (0xC1)
// "OF"  = 'O'(0x4F), 'F'|0x80 (0xC6)
static const char test_tokens[] = { 0x54, 0x48, 0xC5, 0xC1, 0x4F, 0xC6 };


// =============================================================================
// Tests getToken

void test_getToken_0()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given token data pointing to test_tokens
	hdr->tokensPos = (uint16_t)test_tokens;

	//BDD when calling getToken(0)
	char *result = getToken(0);

	//BDD then result is "THE"
	ASSERT(tmpTok[0]=='T' && tmpTok[1]=='H' && tmpTok[2]=='E' && tmpTok[3]=='\0', "token 0 must be THE");
	SUCCEED();
}

void test_getToken_1()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given token data pointing to test_tokens
	hdr->tokensPos = (uint16_t)test_tokens;

	//BDD when calling getToken(1)
	char *result = getToken(1);

	//BDD then result is "A"
	ASSERT(tmpTok[0]=='A' && tmpTok[1]=='\0', "token 1 must be A");
	SUCCEED();
}

void test_getToken_2()
{
	const char *_func = __func__;
	daad_beforeEach();

	//BDD given token data pointing to test_tokens
	hdr->tokensPos = (uint16_t)test_tokens;

	//BDD when calling getToken(2)
	char *result = getToken(2);

	//BDD then result is "OF"
	ASSERT(tmpTok[0]=='O' && tmpTok[1]=='F' && tmpTok[2]=='\0', "token 2 must be OF");
	SUCCEED();
}


int main(char** argv, int argc)
{
	cputs("### UNIT TESTS daad_getToken ###\n\r");
	daad_beforeAll();

	test_getToken_0();
	test_getToken_1();
	test_getToken_2();

	return 0;
}
