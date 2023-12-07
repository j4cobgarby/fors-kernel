#include "fors/filesystem.h"
#include "fors/kheap.h"
#include "forslib/string.h"

// Canonical path form:
//
// 1) Start with a mount tag, in the form :<tag>:
// 2) The tag is immediately followed (no whitespace) by...
// 3) the first part of the path, be it a file or dir. No leading /.
// 4.5) Unless it refers to the root dir, in which case just the tag is needed.
// 4) Further parts of the path are separated by exactly one '/'.
// 5) The path may not end with a /.
//
// Examples of canonical paths (with notes stating what they refer to)
//
// :drive1:my_docs/school/homework1.txt    # A file in the directory "school"
// :usb:arch_linux_live_disk.iso           # A file in the root of a drive
// :backup:backups/2023-12-07              # A directory
// :ssd1:                                  # The root directory of drive "ssd1"
//
// Examples of non-canonical paths (with notes explaining what's wrong)
//
// :drive1:/my_docs/my_paper.txt    # There cannot be a leading slash for the
//                                  # first part of the path.
// :drive1:my_docs//my_paper.txt    # Only one '/' is allowed to separate parts.
// /usr/bin/helix                   # A mount tag needs to be specified.
// :tag::a_nice_file.txt            # ':' is not allowed in tag names.
// :usb: my_backups                 # Cannot have whitespace between tag and path

file *first_file = NULL;
fsnode *first_fsnode = NULL;
mountpoint *first_mountpoint = NULL;

#define FS_LIST_ADD(start, new)\
if (start) start->prev = new;\
new->next = start;\
new->prev = NULL;\
start = new;

#define FS_LIST_REMOVE(start, del)\
if (del->next) del->next->prev = del->prev;\
if (del->prev) del->prev->next = del->next;\
if (!del->next && !del->prev) start = NULL;

/// Opens a file on behalf of a given process, creating a new `file` object to
/// represent this instance.
//
// `tag`   : The mountpoint tag where the desired file is, i.e. what's between
//           the ':'s at the start of the full path.
// `path`  : The path to the file within the specified mountpoint
// `pid`   : The process which opens the file. If this is -1, then it's opened
//         for the kernel only.
// `access`: Access mode for this file. Refer to `enum fileaccess_e`
//
// Returns the new file's `file.fd`, or if an error occurred, returns -1
int vfs_open(const char *tag, const char *path, int pid, fileaccess access) {
    static size_t next_fd = 0;
    mountpoint *mp = NULL;
    
    for (mountpoint *c = first_mountpoint; c; c = c->next) {
        if (strncmp(c->tag, tag, TAG_SIZE) == 0) mp = c;
    }

    if (!mp) return -1; // Mountpoint not found

    fsnode *node = mp->root;
    if (!node) return -1; // Mountpoint has no root node!

    while (1) {
        const char *delim = strchr(path, '/'); // Next slash OR string null term
        size_t part_len = delim - path;

        if (part_len == 0) break; // Must be fully empty path

        if (mp->fs->lookup(node, path, part_len, &node) < 0) {
            return -1; // fs failed lookup
        }

        if (*delim == '\0') break; // Got to end of path
        else path = delim + 1;
    }

    if (node->type == NT_DIR) return -1; // Can't open a directory

    file *new_file = kalloc(sizeof(file));
    new_file->access = access;
    new_file->pid = pid;
    new_file->node = node;
    new_file->fd = next_fd;
    
    FS_LIST_ADD(first_file, new_file);

    //TODO when per-process fd tables are implemented, this should add a local
    // descriptor to the corresponding one.

    return next_fd++;
}

int vfs_close(size_t global_fd, int pid) {
    return -1;
}
