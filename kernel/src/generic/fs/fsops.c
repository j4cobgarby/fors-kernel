#include "fors/filesystem.h"
#include "fors/types.h"
#include "forslib/string.h"
#include "fors/printk.h"

/* Open a file (or directory, etc.) at a given path.
 * Permissions are checked as if opened by process `p`, so the file is only
 * opened if all of the following conditions are met:
 *  - All of the dirs leading up to the target have execute permission.
 *  - If opening with write or append mode, target is writeable.
 *  - If opening with read mode, target is readable.
 * A new `openfile_t` is assigned if it's allowed, and its index in `open_files`
 * is returned. Otherwise, some negative value is returned. */
fd_t vfs_open(pid_t p, const char *full_path, of_mode_t mode)
{
    fsnode_t *parent = find_parent_checkperm(vfs_root, full_path, p);
    printk("[vfs_open] Parent @ %p\n", parent);
    if (!parent) return -1;
    printk("[vfs_open] Parent id = %d\n", parent->internal_id);
    if (!can_exec(parent, p)) return -1;

    fsnode_t *node = get_node(parent, basename(full_path));
    printk("[vfs_open] Node id = %d\n", node->internal_id);
    if ((mode & OF_APPEND || mode & OF_WRITE) && !can_write(node, p)) return -1;
    if (mode & OF_READ && !can_read(node, p)) return -1;

    if (node->mountpoint->fs->f_open(node) < 0) return -1;

    fd_t new_fd = find_free_fd();
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
 * This doesn't need to do any permission checking, because a process can only
 * refer to a global file descriptor through its local fd table. This function
 * just needs to mark the openfile as free, and decrease its node's reference
 * count. */
int vfs_close(fd_t fd)
{
    openfile_t *f = &open_files[fd];
    if (!f->node) return -1;

    if (f->node->mountpoint->fs->f_close(f) < 0) return -1;

    f->node->ref_count--;
    f->node = NULL;

    return 0;
}

int vfs_seek(fd_t fd, long offset, int anchor)
{
    openfile_t *f = &open_files[fd];
    if (!f->node) return -1;

    return f->node->mountpoint->fs->f_seek(f, offset, anchor);

    switch (anchor) {
    case ANCH_REL:
        if (f->cursor < 0) f->cursor = 0;
        f->cursor += offset;
        break;
    case ANCH_START:
        if (offset < 0) return -1;
        f->cursor = offset;
        break;
    }
}

int vfs_read(fd_t fd, long nbytes, char *kbuffer)
{
    openfile_t *f = &open_files[fd];
    if (!f->node) return -1;
    if (f->node->type != FILE) return -1;
    if (!(f->mode & OF_READ)) return -1;
    return f->node->mountpoint->fs->f_read(f, nbytes, kbuffer);
    // TODO: Update node access time
}

int vfs_write(fd_t fd, long nbytes, const char *kbuffer)
{
    openfile_t *f = &open_files[fd];
    if (!f->node) return -1;
    if (f->node->type != FILE) return -1;
    if (!(f->mode & OF_WRITE)) return -1;
    return f->node->mountpoint->fs->f_write(f, nbytes, kbuffer);
    // TODO: Update node access and modify times
}

int vfs_readdir(fd_t fd, size_t n, dir_entry_t *kbuffer)
{
    return -1;
}

/* Instructs the filesystem implementation of the parent of the new filepath to
 * create a new file, in whichever way it wants. Then, whenever get_node and
 * friends are called later, this new file will be available. */
int vfs_mkfile(pid_t p, const char *full_path, fsn_perm_t perms)
{
    fsnode_t *parent = find_parent_checkperm(vfs_root, full_path, p);
    if (!parent) return -1;
    if (!can_write(parent, p)) return -1;
    // TODO: Retrieve uid and gid based on pid, to pass to newfile
    if (parent->mountpoint->fs->newfile(
            parent, basename(full_path), perms, 0, 0)
        < 0)
        return -1;
    return 0;
}

int vfs_mkdir(pid_t p, const char *full_path, fsn_perm_t perms)
{
    fsnode_t *parent = find_parent_checkperm(vfs_root, full_path, p);
    if (!parent) return -1;
    if (!can_write(parent, p)) return -1;
    if (parent->mountpoint->fs->newdir(parent, basename(full_path), perms, 0, 0)
        < 0)
        return -1;
}

int vfs_delnode(const char *full_path)
{
    fsnode_t *parent = find_parent(vfs_root, full_path);
    if (!parent) return -1;
    if (parent->mountpoint->fs->delnode(parent, basename(full_path)) < 0)
        return -1;
    return 0;
}

int vfs_mkhardlink(pid_t p, const char *full_path, const char *link_to)
{
    return -1;
}
