#include <string.h>
#include "dos.h"
#include "assert.h"

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


static const char TEST_FILE[]   = "TESTDOS.TMP";
static const char NOEXIST_FILE[]= "NOEXIST.TMP";
// "Hello, MSX!\r\n"  (13 bytes)
static const char TEST_DATA[]   = "Hello, MSX!\r\n";
#define TEST_DATA_LEN 13

static char buf[64];

static void removeTestFile()
{
    if (fileexists((char*)TEST_FILE)) remove((char*)TEST_FILE);
}


// =============================================================================
// dosver
// =============================================================================

void test_dosver_valid_version()
{
    const char *_func = __func__;

    char ver = dosver();
    ASSERT(ver >= VER_MSXDOS1x, "dosver must return >= VER_MSXDOS1x");
    SUCCEED();
}

void test_dosver_not_unknown()
{
    const char *_func = __func__;

    char ver = dosver();
    ASSERT(ver != VER_UNKNOWN, "dosver must not return VER_UNKNOWN");
    SUCCEED();
}

// =============================================================================
// fileexists
// =============================================================================

void test_fileexists_fails()
{
    const char *_func = __func__;

    ASSERT(!fileexists((char*)NOEXIST_FILE), "non-existent file must return false");
    SUCCEED();
}

void test_fileexists_success()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fclose();

    ASSERT(fileexists((char*)TEST_FILE), "existing file must return true");
    removeTestFile();
    SUCCEED();
}

// =============================================================================
// fcreate + fclose
// =============================================================================

void test_fcreate_success()
{
    const char *_func = __func__;

    uint16_t ret = fcreate((char*)TEST_FILE);
    fclose();

    ASSERT(ret < 0xff00, "fcreate must succeed");
    removeTestFile();
    SUCCEED();
}

void test_fcreate_existing_overwrites()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fclose();
    uint16_t ret = fcreate((char*)TEST_FILE);
    fclose();

    ASSERT(ret < 0xff00, "fcreate on existing file must succeed (overwrite)");
    removeTestFile();
    SUCCEED();
}

void test_fclose_success()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    uint8_t ret = fclose();

    ASSERT(ret == 0, "fclose must return 0");
    removeTestFile();
    SUCCEED();
}

// =============================================================================
// fopen
// =============================================================================

void test_fopen_success()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fclose();

    uint16_t ret = fopen((char*)TEST_FILE);
    fclose();

    ASSERT(ret < 0xff00, "fopen of existing file must succeed");
    removeTestFile();
    SUCCEED();
}

void test_fopen_nonexistent_fails()
{
    const char *_func = __func__;

    uint16_t ret = fopen((char*)NOEXIST_FILE);
    ASSERT(ret >= 0xff00, "fopen of non-existent file must fail");
    SUCCEED();
}

// =============================================================================
// fwrite / fread
// =============================================================================

void test_fwrite_success()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    uint16_t ret = fwrite((char*)TEST_DATA, TEST_DATA_LEN);
    fclose();

    ASSERT(ret < 0xff00, "fwrite must return success (0)");
    removeTestFile();
    SUCCEED();
}

void test_fread_success()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fwrite((char*)TEST_DATA, TEST_DATA_LEN);
    fclose();

    fopen((char*)TEST_FILE);
    memset(buf, 0, sizeof(buf));
    uint16_t ret = fread(buf, TEST_DATA_LEN);
    fclose();

    ASSERT(ret < 0xff00, "fread must return success (0)");
    ASSERT(memcmp(buf, TEST_DATA, TEST_DATA_LEN) == 0, "fread data must match written data");
    removeTestFile();
    SUCCEED();
}

void test_fread_eof_fails()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fwrite((char*)TEST_DATA, TEST_DATA_LEN);
    fclose();

    fopen((char*)TEST_FILE);
    fread(buf, TEST_DATA_LEN);        // consume all data
    uint16_t ret = fread(buf, 1);     // read past EOF
    fclose();

    ASSERT(ret >= 0xff00, "fread past EOF must return error");
    removeTestFile();
    SUCCEED();
}

// =============================================================================
// fseek
// =============================================================================

void test_fseek_set()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fwrite((char*)TEST_DATA, TEST_DATA_LEN);
    fclose();

    fopen((char*)TEST_FILE);
    char first;
    fread(&first, 1);                       // reads TEST_DATA[0], rndRecord → 1
    uint32_t pos = fseek(0L, SEEK_SET);     // back to start
    char again;
    fread(&again, 1);                       // reads TEST_DATA[0] again
    fclose();

    ASSERT_EQUAL((uint16_t)pos, 0, "fseek SEEK_SET to 0 must return position 0");
    ASSERT(first == TEST_DATA[0], "first byte must match TEST_DATA[0]");
    ASSERT(first == again, "fseek SEEK_SET: re-read must equal first read");
    removeTestFile();
    SUCCEED();
}

void test_fseek_cur()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fwrite((char*)TEST_DATA, TEST_DATA_LEN);
    fclose();

    fopen((char*)TEST_FILE);
    fseek(0L, SEEK_SET);
    fseek(5L, SEEK_CUR);    // skip 5 bytes from start
    char c;
    fread(&c, 1);
    fclose();

    ASSERT(c == TEST_DATA[5], "fseek SEEK_CUR: must read correct byte after skip");
    removeTestFile();
    SUCCEED();
}

void test_fseek_end()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fwrite((char*)TEST_DATA, TEST_DATA_LEN);
    fclose();

    fopen((char*)TEST_FILE);
    uint32_t pos = fseek((uint32_t)-1L, SEEK_END);  // one byte before end
    char c;
    fread(&c, 1);
    fclose();

    ASSERT_EQUAL((uint16_t)pos, TEST_DATA_LEN - 1, "fseek SEEK_END -1 must point to last byte");
    ASSERT(c == TEST_DATA[TEST_DATA_LEN - 1], "fseek SEEK_END: must read last byte");
    removeTestFile();
    SUCCEED();
}

// =============================================================================
// fputs / fgets
// =============================================================================

void test_fputs_success()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    uint16_t ret = fputs("Hello\n");
    fclose();

    ASSERT(ret < 0xff00, "fputs must return success (0)");
    removeTestFile();
    SUCCEED();
}

void test_fgets_success()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fputs("Hello\n");
    fclose();

    fopen((char*)TEST_FILE);
    memset(buf, 0xFF, sizeof(buf));
    uint16_t ret = fgets(buf, sizeof(buf) - 1);
    fclose();

    ASSERT(ret < 0xff00, "fgets must succeed on line-terminated file");
    ASSERT(memcmp(buf, "Hello", 5) == 0, "fgets must return the correct string");
    removeTestFile();
    SUCCEED();
}

void test_fgets_empty_file_fails()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fclose();

    fopen((char*)TEST_FILE);
    uint16_t ret = fgets(buf, sizeof(buf) - 1);
    fclose();

    ASSERT(ret >= 0xff00, "fgets on empty file must return error");
    removeTestFile();
    SUCCEED();
}

// =============================================================================
// filesize
// =============================================================================

void test_filesize_success()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fwrite((char*)TEST_DATA, TEST_DATA_LEN);
    fclose();

    uint16_t size = filesize((char*)TEST_FILE);
    ASSERT_EQUAL(size, TEST_DATA_LEN, "filesize must match number of bytes written");
    removeTestFile();
    SUCCEED();
}

void test_filesize_nonexistent_fails()
{
    const char *_func = __func__;

    uint16_t size = filesize((char*)NOEXIST_FILE);
    ASSERT(size >= 0xff00, "filesize of non-existent file must return error");
    SUCCEED();
}

// =============================================================================
// remove
// =============================================================================

void test_remove_success()
{
    const char *_func = __func__;

    fcreate((char*)TEST_FILE);
    fclose();

    uint16_t ret = remove((char*)TEST_FILE);
    ASSERT(ret == 0, "remove must succeed (low byte == 0)");
    ASSERT(!fileexists((char*)TEST_FILE), "file must not exist after remove");
    SUCCEED();
}

void test_remove_nonexistent_fails()
{
    const char *_func = __func__;

    uint16_t ret = remove((char*)NOEXIST_FILE);
    ASSERT((ret & 0xff) != 0, "remove of non-existent file must fail");
    SUCCEED();
}

// =============================================================================
// kbhit
// =============================================================================

void test_kbhit_no_key()
{
    const char *_func = __func__;

    uint8_t ret = kbhit();
    ASSERT(ret == 0, "kbhit must return 0 when no key is pressed");
    SUCCEED();
}


// =============================================================================

int main(char** argv, int argc)
{
    cputs("### UNIT TESTS DOS.LIB ###\n\r");

    RESET_TESTS();

    test_dosver_valid_version();
    test_dosver_not_unknown();

    test_fileexists_fails();
    test_fileexists_success();

    test_fcreate_success();
    test_fcreate_existing_overwrites();
    test_fclose_success();

    test_fopen_success();
    test_fopen_nonexistent_fails();

    test_fwrite_success();
    test_fread_success();
    test_fread_eof_fails();

    test_fseek_set();
    test_fseek_cur();
    test_fseek_end();

    test_fputs_success();
    test_fgets_success();
    test_fgets_empty_file_fails();

    test_filesize_success();
    test_filesize_nonexistent_fails();

    test_remove_success();
    test_remove_nonexistent_fails();

    test_kbhit_no_key();

    return 0;
}
