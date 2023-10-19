#include "fors/printk.h"
#include "fors/thread.h"
#include "fors/init.h"
#include "fors/memory.h"

#include "forslib/string.h"

#include "limine.h"

#include <stdint.h>
#include <stddef.h>


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

    char user_code[] = {
0x55,
0x48, 0x89, 0xe5,
0x48, 0x83, 0xec, 0x08,
0x48, 0x89, 0x7d, 0xf8,
0xb8, 0x37, 0x13, 0x00, 0x00,
0xcd, 0xf0, 
0x90,
0xeb, 0xfd};

    /*
    user_code is the compiled version of:
void user_code(void*) {
    __asm__ ("xchg %bx, %bx");

    __asm__ ("int $0xf0" : : "a"(0x1337));

    for (;;);
}
    */

    void *user_start = (void*)0x200000000;

    void *user_code_phys = pfalloc_one();
    void *user_stack = pfalloc_one();

    void *tmp = tmpmap(user_code_phys);
    if (tmp)
        memcpy(tmp, user_code, 4096);

    int tid = mkthread("Funky Function", 
        user_start, NULL, 
        user_start + 4096*2, 1);

    printk("Mapping in user thread...");
    if (vmap(0, user_code_phys, (void*)0x200000000, 4096, 
    VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT) < 0) {
        printk("Couldn't map userspace page\n"); for (;;);
    }

    if (vmap(0, user_stack, (void*)0x200000000 + 4096, 4096,
    VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT) < 0) {
        printk("Couldn't map user stack.\n"); for (;;);
    }

    printk("Created thread with TID %d\n", tid);

    for (;;) {
        __asm__("hlt");
        printk("Back to main thread!\n");
        for (int i = 0; i < 256; i++) {
            for (int j = 0; j < 256; j++) {
                fb_arr[i * fb->pitch/4 + j] = (i % 256 << 8) + (j % 256 << 16);
            }
        }
    }
}