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

pfd_t find_free_proc_fd(pid_t pid) {
    if (pid >= MAX_PROCS) {
        return -1;
    }
    struct process_t *p = &procs[pid];
    if (!p) {
        return -1;
    }

    for (long i = 0; i < FDS_PER_PROC; i++) {
        if (p->fdmap[i] == -1) return i;
    }

    return -1;
}

long long __sys_open(const char *path, of_mode_t mode)
{
    // Find free slot in proc's fd map
    pfd_t pfd = find_free_proc_fd(current_proc);
    if (pfd < 0) return -1;

    char *full_path = new_prepend_cwd(procs[current_proc].cwd, path);
    if (!full_path) KPANIC("Problem allocating memory.");

    // Open to global fd
    fd_t fd = vfs_open(current_proc, full_path, mode);
    kfree(full_path);

    if (fd < 0) return -1;

    procs[current_proc].fdmap[pfd] = fd;
    return pfd;
}

long long __sys_close(pfd_t pfd)
{
    if (!PFD_VALID(pfd)) return -1;
    fd_t fd = procs[current_proc].fdmap[pfd];
    if (fd < 0) return -1;
    procs[current_proc].fdmap[pfd] = -1;
    return vfs_close(fd);
}

long long __sys_read(pfd_t pfd, size_t num, char *buff) {
    if (!PFD_VALID(pfd)) return -1;
    fd_t fd = procs[current_proc].fdmap[pfd];
    if (fd < 0) return -1;

    return vfs_read(fd, num, buff);
}

long long __sys_write(pfd_t pfd, size_t num, const char *buff) {
    if (!PFD_VALID(pfd)) return -1;
    fd_t fd = procs[current_proc].fdmap[pfd];
    if (fd < 0) return -1;
    return vfs_write(fd, num, buff);
}
