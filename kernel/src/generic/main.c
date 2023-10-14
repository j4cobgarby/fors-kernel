#include "fors/printk.h"
#include "limine.h"

#include <stdint.h>
#include <stddef.h>

#include "fors/init.h"
#include "fors/memory.h"

volatile struct limine_framebuffer_request framebuf_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

void _start(void) {
    arch_early_setup();
    arch_init_memory();
    arch_late_setup();

    struct limine_framebuffer *fb = framebuf_req.response->framebuffers[0];
    uint32_t *fb_arr = fb->address;

    for (int i = 0; i < 600; i++) {
        for (int j = 0; j < 800; j++) {
            fb_arr[(100 + i) * (fb->pitch / 4) + (250 + j)] = i == 0 || j == 0 || i == 599 || j == 799 ? 0xffffff : 0x270057;
        }
    }

    printk("Done.\n");
    for (;;) {
        __asm__("hlt");
    }
}