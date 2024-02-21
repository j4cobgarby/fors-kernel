#include "fors/filesystem.h"
#include "fors/types.h"

fd_t find_free_fd(openfile_t *arr, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (arr[i].node == NULL) return i;
    }
    return -1;
}

/* Open a file (or directory, etc.) at a given path.
 * Permissions are checked as if opened by process `p`, so the file is only opened if
 * all of the following conditions are met:
 *  - All of the dirs leading up to the target have execute permission.
 *  - If opening with write or append mode, target is writeable.
 *  - If opening with read mode, target is readable.
 * A new `openfile_t` is assigned if it's allowed, and its index in `open_files` is
 * returned. Otherwise, some negative value is returned. */
fd_t vfs_open(pid_t p, const char *full_path, of_mode_t mode)
{
    fsnode_t *parent = find_parent_checkperm(vfs_root, full_path, p);
    if (!parent) return -1;
    if (!can_exec(parent, p)) return -1;

    fsnode_t *node = get_node(parent, basename(full_path));
    if ((mode & OF_APPEND || mode & OF_WRITE) && !can_write(node, p)) return -1;
    if (mode & OF_READ && !can_read(node, p)) return -1;

    if (node->mountpoint->fs->f_open(node) < 0) return -1;

    fd_t new_fd = find_free_fd(open_files, NUM_OPEN_FILES);
    if (new_fd == -1) return -1;

    openfile_t *new_of = &open_files[new_fd];
    if (mode & OF_APPEND) mode |= OF_WRITE;
    new_of->mode = mode;
    new_of->node = node;
    new_of->cursor = 0;
    new_of->proc = p;

    new_of->node->ref_count++;

    return new_fd;
}

/* Close a file with a given descriptor.
 * This doesn't need to do any permission checking, because a process can only refer to a
 * global file descriptor through its local fd table.
 * This function just needs to mark the openfile as free, and decrease its node's
 * reference count. */
int vfs_close(fd_t fd)
{
    openfile_t *f = &open_files[fd];
    if (!f->node) return -1;

    if (f->node->mountpoint->fs->f_close(f) < 0) return -1;

    f->node->ref_count--;
    f->node = NULL;

    return 0;
}

int vfs_read(fd_t fd, long nbytes, char *kbuffer)
{
    openfile_t *f = &open_files[fd];
    if (!f->node) return -1;
    if (!(f->mode & OF_READ)) return -1;
    if (f->node->mountpoint->fs->f_read(f, nbytes, kbuffer) < 0) return -1;
    // TODO: Update node access time
    return 0;
}

int vfs_write(fd_t fd, long nbytes, const char *kbuffer)
{
    openfile_t *f = &open_files[fd];
    if (!f->node) return -1;
    if (!(f->mode & OF_WRITE)) return -1;
    if (f->node->mountpoint->fs->f_write(f, nbytes, kbuffer) < 0) return -1;
    // TODO: Update node access and modify times
    return 0;
}

int vfs_readdir(fd_t fd, long buf_bytes, dir_entry_t *kbuffer)
{
    return -1;
}
