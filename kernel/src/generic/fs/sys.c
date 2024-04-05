#include "fors/filesystem.h"
#include "fors/kheap.h"
#include "fors/panic.h"
#include "fors/syscall.h"
#include "fors/process.h"
#include "fors/printk.h"
#include "forslib/string.h"

/** Allocates a new string which holds the full path made from
 * a current working directory plus a relative path from there.
 * Must be freed after use. */
char *new_prepend_cwd(const char *cwd, const char *rel)
{
    size_t cwd_len = strlen(cwd);
    size_t full_len = cwd_len + strlen(rel) + 1;
    char *full_path = kalloc(full_len);
    if (full_path == NULL) return NULL;

    strcpy(full_path, cwd);
    strcpy(full_path + cwd_len, rel);

    return full_path;
}

int __sys_open(const char *path, of_mode_t mode)
{
    char *full_path = new_prepend_cwd(procs[current_proc].cwd, path);
    if (!full_path) KPANIC("Problem allocating memory.");

    int ret = vfs_open(current_proc, full_path, mode);

    kfree(full_path);
    return ret;
}
