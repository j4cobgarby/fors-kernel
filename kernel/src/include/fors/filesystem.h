#ifndef INCLUDE_FORS_FILESYSTEM_H_
#define INCLUDE_FORS_FILESYSTEM_H_

#include "fors/types.h"

#define FILENAME_SIZE  32
#define NUM_FSNODES    256
#define NUM_MOUNTS     32
#define NUM_OPEN_FILES 512

typedef enum fsn_type_t {
    FILE,
    DIRECTORY,
    MOUNTPOINT,
} fsn_type_t;

typedef struct fsnode_t {
    /* metadata */
    char name[FILENAME_SIZE];

    struct mount_t *mountpoint;
    long internal_id; // fs-internal "inode" num
    fsn_id_t id;

    fsn_type_t type;
    fsn_perm_t perms;

    uid_t user;
    gid_t group;

    timestamp_t access_time;
    timestamp_t mod_time;
    timestamp_t create_time;

    unsigned ref_count;

    /* tree */
    struct fsnode_t *sibling_left, *sibling_right;
    struct fsnode_t *child;

    /*            ...
                   |
                parent <-> ...
                /     \
               /       \
           *this* <-> sibling <-> ...
            /  \
           /    \
       child <-> child 2 <-> ...
    */

} fsnode_t;

typedef struct mount_t {
    dev_id_t dev;
    fsnode_t *root_dir;
} mount_t;

typedef struct openfile_t {
    fsnode_t *node; /* fsnodes are guaranteed to stay in-core during the time that any
                       open file referring to them exists. */
    long cursor;
    of_mode_t mode;
    pid_t proc;
} openfile_t;

typedef struct filesystem_type_t {
    fsnode_t *(*retrieve_child)(fsnode_t *parent, const char *name);
    int (*save_out)(openfile_t *file);

    int (*f_seek)(openfile_t *file, long offset, int anchor);
    int (*f_read)(openfile_t *file, long nbytes, char *kbuffer);
    int (*f_write)(openfile_t *file, long nbytes, const char *kbuffer);
} filesystem_type_t;

extern fsnode_t fsnodes[NUM_FSNODES];
extern mount_t mounts[NUM_MOUNTS];
extern openfile_t open_files[NUM_OPEN_FILES];

/* Adding a new (in-core) child node to a given parent directory's children list. */
int add_child(fsnode_t *parent, fsnode_t *new);
int add_child_byname(fsnode_t *parent, const char *new);

fsnode_t *get_node(fsnode_t *parent, const char *path);
/* Try to get a node of a given path under a given root dir. If the requested node has
 * been got before, then it's simply returned by reference. If not, the immediate parent
 * is found and the filesystem implementation is asked to provide the node. Failing this,
 * NULL is returned. */
fsnode_t *find_node(fsnode_t *root, const char *path);
fsnode_t *get_node_byiid(mount_t *root, long internal_id);

/* Return a node, freeing its space in the nodes array. */
void put_node(fsnode_t *node);

/* Normally when reading a file, the contents is buffered, true also for writing. This
 * function will tell the fs to actually save the file's contents to the file */
int save_out(openfile_t *file);

#endif // INCLUDE_FORS_FILESYSTEM_H_
