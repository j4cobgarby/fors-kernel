#include "fors/printk.h"
#include <stdarg.h>
#include <stdint.h>

#define PRINTK_I64_BUF_LEN 20
#define PRINTK_U64_HEX_LEN 16

static unsigned int printk_flags = 0;

void printctrl(unsigned int flags) {
    printk_flags = flags;
}

void printctrl_set(unsigned int setflags) {
    printk_flags |= setflags;
}

void printctrl_unset(unsigned int unsetflags) {
    printk_flags &= ~unsetflags;
}

static void print_i64(int64_t x) {
    char buf[PRINTK_I64_BUF_LEN] = {0};
    int leading_done = 0;

    if ((int64_t)x < 0) {
        kputc('-');
        x *= -1;
    } else if (printk_flags & PRINTCTRL_ALWAYS_SIGN) {
        kputc('+');
    }

    for (int i = PRINTK_I64_BUF_LEN-1; x; x /= 10, i--) {
        buf[i] = x % 10;
    }

    for (int i = 0; i < PRINTK_I64_BUF_LEN; i++) {
        if (buf[i]) {
            leading_done = 1;
        }

        if (printk_flags & PRINTCTRL_LEADING_DEC || leading_done || 
                i == PRINTK_I64_BUF_LEN-1) {
            kputc('0' + buf[i]);
        }
    }
}

static void print_u64_hex(uint64_t x) {
    static char hex_digits[] = "0123456789abcdef";
    int leading_done = 0;

    if (printk_flags & PRINTCTRL_RADIX_PREFIX) {
        kputs("0x");
    }

    for (int i = PRINTK_U64_HEX_LEN - 1; i >= 0; i--) {
        int ind = (x >> 4 * i) & 0xf;

        if (ind) {
            leading_done = 1;
        }

        if (printk_flags & PRINTCTRL_LEADING_HEX || leading_done || i == 0) {
            if (printk_flags & PRINTCTRL_SPACERS && i != PRINTK_U64_HEX_LEN - 1
                    && (i+1) % 4 == 0) {
                kputc(' ');
            }
            kputc(hex_digits[ind]);
        }
    }
}

void printk(const char *restrict fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        if (*fmt != '%') {
            kputc(*fmt);
        } else {
            switch (*(++fmt)) {
                case '\0':
                    return;

                case 's':
                    kputs(va_arg(ap, const char *));
                    break;
                
                case 'c':
                    kputc(va_arg(ap, int));
                    break;

                case 'd':
                    print_i64(va_arg(ap, int64_t));
                    break;

                case 'u':
                    print_i64(va_arg(ap, uint64_t));
                    break;

                case 'x':
                    print_u64_hex(va_arg(ap, uint64_t));
                    break;

                default:
                    break;
            }
        }

        fmt++;
    }
}