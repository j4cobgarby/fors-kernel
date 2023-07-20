#include "limine.h"

#include <stdint.h>
#include <stddef.h>

void _start(void) {
    for (;;) {
        __asm__("hlt");
    }
}
