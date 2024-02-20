#ifndef INCLUDE_FORS_FILESYSTEM_H_
#define INCLUDE_FORS_FILESYSTEM_H_

#include "fors/types.h"
#include <stddef.h>
#include <stdbool.h>

#define FILENAME_SIZE  32
#define NUM_FSNODES    256
#define NUM_MOUNTS     32
#define NUM_OPEN_FILES 512
#define NUM_FSLINKS    256

typedef enum fsn_type_t {
    EMPTY,
    FILE,
    DIRECTORY,
    MOUNTPOINT,
} fsn_type_t;

typedef struct fsnode_t {
    /* metadata */
    struct mount_t *mountpoint;
    long internal_id; // fs-internal "inode" num

    fsn_type_t type;
    fsn_perm_t perms;

    uid_t user;
    gid_t group;

    timestamp_t access_time;
    timestamp_t mod_time;
    timestamp_t create_time;

    unsigned ref_count;

    /* Only used if the node is a directory */
    struct fslink_t *child;
} fsnode_t;

typedef struct mount_t {
    dev_id_t dev;
    struct filesystem_type_t *fs;
    fsnode_t *root_dir;
} mount_t;

typedef struct fslink_t {
    char name[FILENAME_SIZE + 1];
    fsnode_t *node;
    struct fslink_t *prev, *next;
} fslink_t;

typedef struct openfile_t {
    fslink_t *inst;
    long cursor;
    of_mode_t mode;
    pid_t proc;
} openfile_t;

typedef struct dir_entry_t {
    fsn_id_t node_id;
    size_t off; /* offset to next dir entry */
    char filename[];
} dir_entry_t;

/* fsnode_t: metadata of filesystem nodes.
 *    ^
 * fslink_t: name of a node; these represent the tree structure, and point to fsnodes.
 *    ^
 * openfile_t: an fslink_t that someone has got open.
 *
               ...
                |
            parent <-> ...
            /     \
           /       \
        *this* <-> sibling <-> ...
        /  \
       /    \
    child <-> child 2 <-> ...
*/

typedef struct filesystem_type_t {
    fsnode_t *(*retrieve_child)(fsnode_t *parent, const char *name, size_t len);
    fsnode_t *(*node_from_id)(long internal_id);
    void (*put_node)(fsnode_t *file);

    int (*f_seek)(openfile_t *file, long offset, int anchor);
    int (*f_read)(openfile_t *file, long nbytes, char *kbuffer);
    int (*f_write)(openfile_t *file, long nbytes, const char *kbuffer);

    int (*newfile)(fslink_t *parent, const char *name, int flags);
    int (*newdir)(fslink_t *parent, const char *name, int flags);
} filesystem_type_t;

extern fsnode_t fsnodes[NUM_FSNODES];
extern fslink_t fslinks[NUM_FSLINKS];
extern openfile_t open_files[NUM_OPEN_FILES];
extern mount_t mounts[NUM_MOUNTS];

int can_read(fsnode_t *dir, pid_t p);
int can_write(fsnode_t *dir, pid_t p);
int can_exec(fsnode_t *dir, pid_t p);

int add_child(fsnode_t *parent, fsnode_t *new, const char *name);
int del_child(fsnode_t *parent, fslink_t *to_del);
void put_node(fsnode_t *node);

fsnode_t *get_node(fsnode_t *parent, const char *name);
fsnode_t *find_node(fsnode_t *root, const char *path);
fsnode_t *find_parent(fsnode_t *root, const char *path);
fsnode_t *find_parent_checkperm(fsnode_t *root, const char *path, pid_t p);
fsnode_t *get_node_byid(mount_t *root, long internal_id);

/* e.g. /home/bob/main.c => main.c
 *      /home/bob/ => bob
 *      / => '' */
const char *basename(const char *path);

fd_t vfs_open(pid_t p, const char *rel_path, of_mode_t mode);
int vfs_close(pid_t p, fd_t fd);
int vfs_seek(pid_t p, fd_t fd, long offset, seek_anchor_t anchor);

int vfs_read(pid_t p, fd_t fd, long nbytes, char *kbuffer);
int vfs_write(pid_t p, fd_t fd, long nbytes, const char *kbuffer);
int vfs_readdir(pid_t p, fd_t fd, long buf_bytes, char *kbuffer);

int vfs_mkfile(pid_t p, const char *rel_path, fsn_perm_t perms);
int vfs_mklink(pid_t p, const char *rel_path, const char *link_to);
int vfs_mkdir(pid_t p, const char *rel_path, fsn_perm_t perms);
int vfs_mountat(pid_t p, const char *rel_path, dev_id_t device);

int vfs_delnode(pid_t p, const char *rel_path);

#endif // INCLUDE_FORS_FILESYSTEM_H_
