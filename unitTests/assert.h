#ifndef  __ASSERT_H__
#define  __ASSERT_H__

#include <stdint.h>
#include <stdbool.h>


#define ASSERT(cond, failMsg)	_ASSERT(cond, failMsg, __FILE__, __func__, __LINE__)
#define FAIL(failMsg)			_FAIL(failMsg, __FILE__, __func__, __LINE__)
#define SUCCEED()				_SUCCEED(__FILE__, __func__)


void _ASSERT(bool succeedCondition, const char *failMsg, char *file, char *func, int line);
void _FAIL(const char *failMsg, char *file, char *func, int line);
void _SUCCEED(char *file, char *func);


#endif //__ASSERT_H__
