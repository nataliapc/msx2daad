#include <stdarg.h>
#include "dos.h"


static void _printn(unsigned u, unsigned base, char issigned);
static void _printf(const char *format, va_list va);

/*
	Simple cprintf implementation.
	Supports %c %s %u %d %x %b
 */
int cprintf(const char *format, ...)
{
	va_list va;
	va_start(va, format);

	_printf(format, va);

	/* return printed chars */
	return 0;
}

static void _printn(unsigned u, unsigned base, char issigned)
{
	const char *_hex = "0123456789ABCDEF";
	if (issigned && ((int)u < 0)) {
		putchar('-');
		u = (unsigned)-((int)u);
	}
	if (u >= base)
		_printn(u/base, base, 0);
	putchar(_hex[u%base]);
}

static void _printf(const char *format, va_list va)
{
	while (*format) {
		if (*format == '%') {
			switch (*++format) {
				case 'c': {
					char c = (char)va_arg(va, int);
					putchar(c);
					break;
				}
				case 'u': {
					unsigned u = va_arg(va, unsigned);
					_printn(u, 10, 0);
					break;
				}
				case 'd': {
					unsigned u = va_arg(va, unsigned);
					_printn(u, 10, 1);
					break;
				}
				case 'x': {
					unsigned u = va_arg(va, unsigned);
					_printn(u, 16, 0);
					break;
				}
				case 'b': {
					unsigned u = va_arg(va, unsigned);
					_printn(u, 2, 0);
					break;
				}
				case 's': {
					char *s = va_arg(va, char *);
					while (*s) {
						putchar(*s);
						s++;
					}
				}
			}
		} else {
			putchar(*format);
		}
		format++;
	}
}
