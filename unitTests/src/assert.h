#ifndef  __ASSERT_H__
#define  __ASSERT_H__

#include <stdint.h>
#include <stdbool.h>


#define ASSERT(cond, failMsg)					_ASSERT_TRUE(cond, failMsg, __FILE__, __func__, __LINE__)
#define ASSERT_EQUAL(value, expected, failMsg)	_ASSERT_EQUAL((uint16_t)(value), (uint16_t)(expected), failMsg, __FILE__, __func__, __LINE__)
#define FAIL(failMsg)							_FAIL(failMsg, __FILE__, __func__, __LINE__)
#define SUCCEED()								_SUCCEED(__FILE__, __func__)
#define TODO(infoMsg)							_TODO(infoMsg, __FILE__, __func__)


void _ASSERT_TRUE(bool succeedCondition, const char *failMsg, char *file, char *func, int line);
void _ASSERT_EQUAL(uint16_t value, uint16_t expected, const char *failMsg, char *file, char *func, int line);
void _FAIL(const char *failMsg, char *file, char *func, int line);
void _SUCCEED(char *file, char *func);
void _TODO(const char *infoMsg, char *file, char *func);

void crt_exit();


#endif //__ASSERT_H__
