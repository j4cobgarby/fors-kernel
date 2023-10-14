#include "fors/printk.h"
#include <float.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define PRINTK_I64_BUF_LEN 64
#define PRINTK_U64_HEX_LEN 16

typedef enum conv_state {
    START, FLAGS, FIELD_WIDTH, PRECISION, LENGTH_MODIFIER, CONVSPEC
} conv_state;

typedef enum len_modifier {
    lm_NONE,
    lm_hh,
    lm_h,
    lm_l,
    lm_ll,
    lm_z,
} len_modifier;

typedef struct conversion_specifier {
    bool alternate_form;    // '#' flag
    bool zero_padded;       // '0' flag
    bool left_adjusted;     // '-' flag
    bool positive_pad;      // ' ' flag
    bool always_sign;       // '+' flag

    int field_width;
    int precision;
    len_modifier lenmod;

    char convspec;
} conversion_specifier;

static void print_i64(int64_t x, conversion_specifier *spec) {
    char buf[PRINTK_I64_BUF_LEN] = {0};
    bool leading_done = false;

    if ((int64_t)x < 0) {
        kputc('-');
        x *= -1;
    } else if (spec->always_sign) {
        kputc('+');
    }

    for (int i = PRINTK_I64_BUF_LEN-1; x; x /= 10, i--) {
        buf[i] = x % 10;
    }

    for (int i = 0; i < PRINTK_I64_BUF_LEN; i++) {
        if (buf[i]) {
            leading_done = true;
        }

        if (PRINTK_I64_BUF_LEN - i <= spec->precision || leading_done || i == PRINTK_I64_BUF_LEN-1) {
            kputc('0' + buf[i]);
        } else if (PRINTK_I64_BUF_LEN - i <= spec->field_width) {
            kputc(' ');
        }
    }
}

static void print_u64_hex(uint64_t x, bool uppercase, conversion_specifier *spec) {
    static char hex_digits[] = "0123456789abcdef";
    static char hex_digits_upper[] = "0123456789ABCDEF";

    bool leading_done = false;

    if (spec->alternate_form) {
        if (uppercase) {
            kputs("0X");
        } else {
            kputs("0x");
        }
    }

    for (int i = PRINTK_U64_HEX_LEN - 1; i >= 0; i--) {
        int ind = (x >> 4 * i) & 0xf;

        if (ind) {
            leading_done = true;
        }

        if (PRINTK_U64_HEX_LEN - i <= spec->precision || leading_done || i == 0) {
            kputc(hex_digits[ind]);}
        // } else if (PRINTK_U64_HEX_LEN - i <= spec->field_width) {
        //     kputc(' ');
        // }
    }
}

bool char_is_specifier(const char c) {
    return c == 'd' || c == 'u' || c == 'x' || c == 'X' 
        || c == 'c' || c == 's' || c == 'p' || c == '%';
}

bool char_is_flag(const char c) {
    return c == '#' || c == '0' || c == '-' || c == ' ' || c == '+';
}

bool char_is_digit(const char c) {
    return c >= '0' && c <= '9';
}

void defaults_from_convspec(const char c, int *field_width, int *precision,
bool *alternate_form) {
    switch (c) {
        case 'p':
            *precision = 16;
            *alternate_form = true;
            break;
        default:
            // Default is to not change anything
            break;
    }
}

/*
%[flags][field_width][.precision][length modifier]convspec
*/
const char *parse_conversion(const char *start, conversion_specifier *spec) {
    conv_state state = START;

    spec->alternate_form = false;    // '#' flag
    spec->zero_padded = false;       // '0' flag
    spec->left_adjusted = false;     // '-' flag
    spec->positive_pad = false;      // ' ' flag
    spec->always_sign = false;       // '+' flag

    spec->field_width = 0;
    spec->precision = 0;
    spec->lenmod = lm_NONE;

    spec->convspec = 0;

    bool precision_begun = false; // Has the '.' at the start of precision field been encountered?

    const char *c = start;

    do {
        switch (state) {
            case START:
                if (*c == '%') state = FLAGS;
                else return NULL;
                break;
            case FLAGS:
                if (!char_is_flag(*c)) {
                    state = FIELD_WIDTH;
                    continue; // Skip c++
                } else {
                    switch (*c) {
                        case '#':
                            spec->alternate_form = true;  break;
                        case '0':
                            spec->zero_padded = true;     break;
                        case '-':
                            spec->left_adjusted = true;   break;
                        case ' ':
                            spec->positive_pad = true;    break;
                        case '+':
                            spec->always_sign = true;     break;
                    }
                }
                break;
            case FIELD_WIDTH:
                if (!char_is_digit(*c)) {
                    state = PRECISION;
                    continue; // Skip c++
                } else {
                    spec->field_width *= 10;
                    spec->field_width += *c - '0';
                }
                break;
            case PRECISION:
                if (precision_begun) {
                    if (!char_is_digit(*c)) {
                        state = LENGTH_MODIFIER;
                        continue; // Skip c++
                    } else {
                        spec->precision *= 10;
                        spec->precision += *c - '0';
                    }
                } else {
                    precision_begun = *c == '.';
                    if (precision_begun) {
                        spec->precision = 0;
                    } else {
                        state = LENGTH_MODIFIER;
                        continue; // Skip c++
                    }
                }
                break;
            case LENGTH_MODIFIER:
                switch (*c) {
                    case 'h':
                        if (*(c+1) && *(c+1) == 'h') {
                            spec->lenmod = lm_hh;
                            c++; // Skip extra h
                        } else spec->lenmod = lm_h;
                        break;
                    case 'l':
                        if (*(c+1) && *(c+1) == 'l') {
                            spec->lenmod = lm_ll;
                            c++;
                        } else spec->lenmod = lm_l;
                        break;
                    case 'z':
                        spec->lenmod = lm_z;
                        break;
                    default:
                        state = CONVSPEC;
                        continue;
                }
                state = CONVSPEC;
                break;
            case CONVSPEC:
                if (char_is_specifier(*c)) {
                    spec->convspec = *c;
                    defaults_from_convspec(*c, &spec->field_width, &spec->precision, &spec->alternate_form);
                    return c + 1; // Primary return point
                } else {
                    return NULL;
                }
        }

        c++;
    } while (*c);

    return NULL;
}

void printk(const char *restrict fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    conversion_specifier spec;

    while (*fmt) {
        if (*fmt != '%') {
            kputc(*fmt++);
        } else {
            fmt = parse_conversion(fmt, &spec);

            #ifdef PRINTK_HIGHLIGHT_FORMATS
            kputs("\x1b[36m");
            #endif

            switch (spec.convspec) {
                case 's':
                    kputs(va_arg(ap, const char *));
                    break;
                case 'c':
                    kputc(va_arg(ap, int));
                    break;
                case 'd':
                    print_i64(va_arg(ap, int64_t), &spec);
                    break;
                case 'u':
                    print_i64(va_arg(ap, uint64_t), &spec);
                    break;
                case 'x':
                    print_u64_hex(va_arg(ap, uint64_t), false, &spec);
                    break;
                case 'p':
                    print_u64_hex(va_arg(ap, uint64_t), false, &spec);
                    break;
            }
            #ifdef PRINTK_HIGHLIGHT_FORMATS
            kputs("\x1b[37m");
            #endif
        }
    }
}

// void printk(const char *restrict fmt, ...) {
//     va_list ap;
//     va_start(ap, fmt);

//     while (*fmt) {
//         if (*fmt != '%') {
//             kputc(*fmt);
//         } else {
//             switch (*(++fmt)) {
//                 case '\0':
//                     return;

//                 case 's':
//                     kputs(va_arg(ap, const char *));
//                     break;
                
//                 case 'c':
//                     kputc(va_arg(ap, int));
//                     break;

//                 case 'd':
//                     print_i64(va_arg(ap, int64_t));
//                     break;

//                 case 'u':
//                     print_i64(va_arg(ap, uint64_t));
//                     break;

//                 case 'x':
//                     print_u64_hex(va_arg(ap, uint64_t));
//                     break;

//                 default:
//                     break;
//             }
//         }

//         fmt++;
//     }
// }