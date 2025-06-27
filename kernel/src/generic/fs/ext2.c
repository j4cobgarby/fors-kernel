#include "fors/filesystem.h"
#include "fors/fs/ext2.h"

int ext2_initmnt(mount_t *mnt) {
    return 0;
}

long ext2_retrieve_child(fsnode_t *parent, const char *name, size_t name_len) {
    return 0;
}

int ext2_node_from_id(long id, fsnode_t *node) {
    return 0;
}

int ext2_read(openfile_t *file, size_t nbytes, char *kbuffer) {
    return 0;
}

int ext2_save_node(fsnode_t *file) {
    return 0;
}

int ext2_open(fsnode_t *node) {
    return 0;
}

int ext2_close(openfile_t *file) {
    return 0;
}

int ext2_seek(openfile_t *file, long offset, int anchor) {
    return 0;
}

int ext2_write(openfile_t *file, size_t nbytes, const char *buffer) {
    return 0;
}

int ext2_newfile(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t) {
    return 0;
}

int ext2_newdir(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t) {
    return 0;
}

int ext2_delnode(fsnode_t *, const char *) {
    return 0;
}


filesystem_type_t ext2_type = {
    "EXT2",

    ext2_initmnt,
    ext2_retrieve_child,
    ext2_node_from_id,
    ext2_save_node,
    ext2_open,
    ext2_close,
    ext2_seek,
    ext2_read,
    ext2_write,
    ext2_newfile,
    ext2_newdir,
    ext2_delnode,
};

