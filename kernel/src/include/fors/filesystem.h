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

/* The metadata of a file (or dir). A single fsnode can have many different
 * names and locations in the vfs tree. */
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

    unsigned ref_count; /* How many open files refer to me? */

    /* Only used if the node is a directory */
    struct fslink_t *child;
} fsnode_t;

/* Represents a mounted filesystem. Filesystems are mounted in a UNIXey way, at
 * a certain point in the tree. */
typedef struct mount_t {
    dev_id_t dev;
    struct filesystem_type_t *fs;
    fsnode_t *root_dir;
} mount_t;

/* Represents an object in the vfs tree structure, which points to some fsnode.
 * This is what gives a file its name. */
typedef struct fslink_t {
    char name[FILENAME_SIZE + 1];
    fsnode_t *node;
    struct fslink_t *prev, *next;
} fslink_t;

/* Represents a specific fsnode that someone has got open */
typedef struct openfile_t {
    fsnode_t *node;
    long cursor;
    of_mode_t mode;
    pid_t proc;
} openfile_t;

/* These are packed into a buffer when vfs_readdir is called. Each entry
 * represents one file (or dir, etc.) in a directory. */
typedef struct dir_entry_t {
    fsn_id_t node_id;
    char filename[FILENAME_SIZE + 1];
} dir_entry_t;

/* fsnode_t: metadata of filesystem nodes.
 *    ^
 * fslink_t: name of a node; these represent the tree structure, and point to
 fsnodes.
 *    ^
 * openfile_t: an fsnode_t that someone has got open.
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

/* An implementation of a specifical filesystem, for example ext2 or fat, or
 * even some virtual one (NFS, etc.) */
typedef struct filesystem_type_t {
    fsnode_t *(*retrieve_child)(fsnode_t *parent, const char *name, size_t len);
    fsnode_t *(*node_from_id)(long internal_id);
    void (*put_node)(fsnode_t *file);

    int (*f_open)(fsnode_t *node);
    int (*f_close)(openfile_t *file);
    int (*f_seek)(openfile_t *file, long offset, int anchor);
    int (*f_read)(openfile_t *file, long nbytes, char *kbuffer);
    int (*f_write)(openfile_t *file, long nbytes, const char *kbuffer);

    int (*newfile)(fsnode_t *parent, const char *name, fsn_perm_t perms,
        uid_t user, gid_t group);
    int (*newdir)(fsnode_t *parent, const char *name, fsn_perm_t perms,
        uid_t user, gid_t group);
    int (*delnode)(fsnode_t *parent, const char *name);
} filesystem_type_t;

/* Global vars for storing all the vfs objects. No dynamic allocation. */
extern fsnode_t fsnodes[NUM_FSNODES];
extern fslink_t fslinks[NUM_FSLINKS];
extern openfile_t open_files[NUM_OPEN_FILES];
extern mount_t mounts[NUM_MOUNTS];
extern fsnode_t *vfs_root;

/* Helper functions */
int can_read(fsnode_t *dir, pid_t p);
int can_write(fsnode_t *dir, pid_t p);
int can_exec(fsnode_t *dir, pid_t p);
int find_free_link();
int find_free_node();
fd_t find_free_fd();
const char *basename(const char *path);

/* Handling link tree and node references */
int add_child(fsnode_t *parent, fsnode_t *new, const char *name);
int del_child(fsnode_t *parent, fslink_t *to_del);
void put_node(fsnode_t *node);

/* Different ways to retrieve nodes */
fsnode_t *get_node(fsnode_t *parent, const char *name);
fsnode_t *find_node(fsnode_t *root, const char *path);
fsnode_t *find_parent(fsnode_t *root, const char *path);
fsnode_t *find_parent_checkperm(fsnode_t *root, const char *path, pid_t p);
fsnode_t *get_node_byid(mount_t *root, long internal_id);

/* High(er) level vfs functionality, maps closely to the actual syscalls */
int vfs_close(fd_t fd);
int vfs_seek(fd_t fd, long offset, seek_anchor_t anchor);
int vfs_read(fd_t fd, long nbytes, char *kbuffer);
int vfs_write(fd_t fd, long nbytes, const char *kbuffer);
int vfs_readdir(fd_t fd, size_t n, dir_entry_t *kbuffer);
fd_t vfs_open(pid_t p, const char *full_path, of_mode_t mode);
int vfs_mkfile(pid_t p, const char *full_path, fsn_perm_t perms);
int vfs_mkhardlink(pid_t p, const char *full_path, const char *link_to);
int vfs_mkdir(pid_t p, const char *full_path, fsn_perm_t perms);
int vfs_mountat(pid_t p, const char *full_path, dev_id_t device);
int vfs_delnode(const char *full_path);

#endif // INCLUDE_FORS_FILESYSTEM_H_
