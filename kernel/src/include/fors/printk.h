#ifndef __INCLUDE_FORS_PRINTK_H__
#define __INCLUDE_FORS_PRINTK_H__

#include <stdarg.h>

// Arch-defined functions
void kputs(const char *restrict str);
void kputc(const char);

// Generic

// Should printk always print the sign of a number, rather than only for negative?
#define PRINTCTRL_ALWAYS_SIGN   1 << 0
// Should printk print '0x' and similar before non-decimal numbers?
#define PRINTCTRL_RADIX_PREFIX  1 << 1
// Should printk print leading zeroes for hex?
#define PRINTCTRL_LEADING_HEX   1 << 2
// Should printk print leading zeroes for decimal?
#define PRINTCTRL_LEADING_DEC   1 << 3
// Should printk print spacers for hex?
#define PRINTCTRL_SPACERS       1 << 4

void printk(const char *restrict fmt, ...);

void printctrl(unsigned int flags);

void printctrl_set(unsigned int setflags);

void printctrl_unset(unsigned int unsetflags);

#endif /* __INCLUDE_FORS_PRINTK_H__ */