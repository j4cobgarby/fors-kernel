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

    struct limine_framebuffer *fb = framebuf_req.response->framebuffers[0];
    uint32_t *fb_arr = fb->address;

//     printk("==== Framebuffer:\n\
// \tAddr: %x\n\
// \tWidth: %d\n\
// \tHeight: %d\n\
// \tPitch: %d\n\
// \tBits/pixel: %d\n", fb->address, fb->width, fb->height, fb->pitch, fb->bpp);

    for (int i = 0; i < 100; i++) {
        fb_arr[i * (fb->pitch / 4) + i] = 0xffffff;
    }
    
    for (;;) {
        __asm__("hlt");
    }
}