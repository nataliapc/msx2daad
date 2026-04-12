#include <stdarg.h>
#include "../include/dos.h"
#include "../include/heap.h"
#include "assert.h"


static const char LOG_FILE[] = "tests.txt";
static bool  logFileCreated;
static char *_writePtr;


// Append [s, s+len) to tests.txt on the current drive. First call creates the
// file (truncating any previous one); subsequent calls open and seek to end.
static void _logAppend(char *s, uint16_t len)
{
	uint16_t fh;
	if (!len) return;

	if (!logFileCreated) {
		fh = fcreate((char*)LOG_FILE, O_RDWR, ATTR_NONE);
		if (fh < 0xff00) logFileCreated = true;
	} else {
		fh = fopen((char*)LOG_FILE, O_RDWR);
		if (fh < 0xff00) fseek((char)fh, 0L, SEEK_END);
	}
	if (fh < 0xff00) {
		fwrite(s, len, (char)fh);
		fclose((char)fh);
	}
}


// Char emit: writes to console (immediate) and to the scratch buffer at
// [heap_top..] so we can append the same bytes to tests.txt in one fwrite
// after the format completes. heap_top is NOT advanced (we use it as scratch).
static void _putc(char c)
{
	putchar(c);
	*_writePtr++ = c;
}

static void _printn(unsigned u, unsigned base, char issigned)
{
	const char *_hex = "0123456789ABCDEF";
	if (issigned && ((int)u < 0)) {
		_putc('-');
		u = (unsigned)-((int)u);
	}
	if (u >= base)
		_printn(u/base, base, 0);
	_putc(_hex[u%base]);
}


// Local replacement of dos.lib cprintf. Links from assert.rel before dos.lib,
// so it overrides the library symbol for the test binary only. Production
// builds use the original unchanged. Supports the same subset: %c %s %u %d %x %b.
int cprintf(const char *format, ...)
{
	char *start = (char*)heap_top;
	_writePtr = start;

	va_list va;
	va_start(va, format);

	while (*format) {
		if (*format == '%') {
			switch (*++format) {
				case 'c': {
					char c = (char)va_arg(va, int);
					_putc(c);
					break;
				}
				case 'u': _printn(va_arg(va, unsigned), 10, 0); break;
				case 'd': _printn(va_arg(va, unsigned), 10, 1); break;
				case 'x': _printn(va_arg(va, unsigned), 16, 0); break;
				case 'b': _printn(va_arg(va, unsigned),  2, 0); break;
				case 's': {
					char *s = va_arg(va, char *);
					while (*s) _putc(*s++);
					break;
				}
			}
		} else {
			_putc(*format);
		}
		format++;
	}
	va_end(va);

	_logAppend(start, (uint16_t)(_writePtr - start));
	return 0;
}


// Local replacement of dos.lib cputs (so the banner in main also reaches the file).
void cputs(char *s)
{
	char *p = s;
	while (*p) putchar(*p++);
	_logAppend(s, (uint16_t)(p - s));
}


void _ASSERT_TRUE(bool succeedCondition, const char *failMsg, char *file, char *func, int line)
{
	if (!succeedCondition) {
		cprintf("### Assert failed at: %s :: %s :: line %d\n\r    by \"%s\"\n\r\x07", file, func, line, failMsg);
		crt_exit();
	}
}

void _ASSERT_EQUAL(uint16_t value, uint16_t expected, const char *failMsg, char *file, char *func, int line)
{
	if (value != expected) {
		cprintf("### Assert failed at: %s :: %s :: line %d\n\r    by \"%s\"\n\r    received:%u expected:%u\n\r\x07", file, func, line, failMsg, value, expected);
		crt_exit();
	}
}

void _FAIL(const char *failMsg, char *file, char *func, int line)
{
	cprintf("Fail by '%s'\n\r  at %s :: %s :: line %d\n\r\x07", failMsg, file, func, line);
	crt_exit();
}

void _SUCCEED(char *file, char *func)
{
	cprintf("%s :: %s ... OK\n\r", file, func);
}

void _TODO(const char *infoMsg, char *file, char *func)
{
	cprintf("### TODO: %s :: %s [%s]\n\r", file, func, infoMsg);
}
