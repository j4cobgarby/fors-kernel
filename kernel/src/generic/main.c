#include "fors/printk.h"
#include "fors/thread.h"
#include "fors/init.h"
#include "fors/memory.h"
#include "fors/filesystem.h"
#include "fors/fs/test.h"
#include "fors/panic.h"

#include "fors/types.h"
#include "forslib/string.h"

#include "limine.h"

#include <stddef.h>

volatile struct limine_framebuffer_request framebuf_req = {
    .id = LIMINE_FRAMEBUFFER_REQUEST,
    .revision = 0,
};

void task1(void *)
{
    const char *test_msg = "Initialising system!\n";
    __asm__ volatile("int $0xf0" : : "a"(0), "S"(test_msg));
    for (;;) { }
}

void _start(void)
{
    arch_initialise();

    /* Initialise filesystem root */
    mount_t *testmnt = &mounts[0];
    testmnt->dev = -1;
    testfs_type.initmnt(testmnt);
    vfs_root = get_node_byid(testmnt, testmnt->root_fsnode);

    void *user_start = (void *)0x200000000;
    void *user_code_phys = pfalloc_one();
    void *user_stack = pfalloc_one();

    void *tmp = tmpmap(user_code_phys);
    if (tmp) memcpy(tmp, &task1, 4096);

    int tid = mkthread(
        "Funky Function", user_start, NULL, user_start + 4096 * 2, 1);

    if (vmap(tid, user_code_phys, (void *)0x200000000, 4096,
            VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT)
        < 0) {
        KPANIC("Couldn't map userspace page\n");
        for (;;)
            ;
    }

    if (vmap(tid, user_stack, (void *)0x200000000 + 4096, 4096,
            VMAP_4K | VMAP_EXEC | VMAP_USER | VMAP_WRIT)
        < 0) {
        KPANIC("Couldn't map user stack.\n");
        for (;;)
            ;
    }

    enqueue_thread(tid);
    printk(") Now going to start thread %d\n", tid);
    arch_start_running_threads();

    for (;;) {
        __asm__("hlt");
    }
}
