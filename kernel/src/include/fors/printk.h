#ifndef __INCLUDE_FORS_PRINTK_H__
#define __INCLUDE_FORS_PRINTK_H__

#include <stdarg.h>

#define PRINTK_HIGHLIGHT_FORMATS 

#define TRACE() printk("*** TRACE() at %s:%d [%s]\n", __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define TRACE_MSG(msg) printk("*** " msg " at %s:%d [%s]\n", __FILE__, __LINE__, __PRETTY_FUNCTION__)

#define ERROR(msg) TRACE_MSG("ERROR! ")

// Arch-defined functions
void kputs(const char *restrict str);
void kputc(const char);

// Generic

void __attribute__((no_caller_saved_registers)) printk(const char *restrict fmt, ...);

#endif /* __INCLUDE_FORS_PRINTK_H__ */