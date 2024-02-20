#include "fors/filesystem.h"
#include "fors/types.h"

fd_t find_free_fd(openfile_t *arr, size_t size)
{
    for (size_t i = 0; i < size; i++) {
        if (arr[i].node == NULL) return i;
    }
    return -1;
}

fd_t vfs_open(pid_t p, const char *rel_path, of_mode_t mode)
{
    fsnode_t *parent = find_parent_checkperm(vfs_root, rel_path, p);
    if (!parent) return -1;
    if (!can_exec(parent, p)) return -1;

    fsnode_t *node = get_node(parent, basename(rel_path));
    if ((mode & OF_APPEND || mode & OF_WRITE) && !can_write(node, p)) return -1;
    if (mode & OF_READ && !can_read(node, p)) return -1;

    if (node->mountpoint->fs->f_open(node) < 0) return -1;

    fd_t new_fd = find_free_fd(open_files, NUM_OPEN_FILES);
    if (new_fd == -1) return -1;

    openfile_t *new_of = &open_files[new_fd];
    new_of->mode = mode;
    new_of->node = node;
    new_of->cursor = 0;
    new_of->proc = p;

    new_of->node->ref_count++;

    return new_fd;
}

int vfs_close(fd_t fd)
{
    openfile_t *f = &open_files[fd];
    if (!f->node) return -1;

    if (f->node->mountpoint->fs->f_close(f) < 0) return -1;

    f->node->ref_count--;
    f->node = NULL;

    return 0;
}
