#include "fors/filesystem.h"
#include "fors/fs/test.h"
#include "fors/types.h"
#include "forslib/string.h"
#include "forslib/maths.h"
#include "fors/printk.h"

enum root_ids {
    ID_ROOT_DIR,

    ID_TEST_TXT,
    ID_TEST2_TXT,
    ID_TEST3_TXT,
    ID_TESTDIR1,
    ID_TESTDIR2,

    __ROOT_SENTINAL,
};
enum dir1_ids {
    ID_CHILD1 = __ROOT_SENTINAL,
    ID_CHILD2,

    __DIR1_SENTINAL,
};
enum dir2_ids {
    ID_CHILDA = __DIR1_SENTINAL,
    ID_CHILDB,

    __DIR2_SENTINAL,
};

#define __FINAL_SENTINAL __DIR2_SENTINAL

const char *node_names[] = {
    ".",
    "test.txt",
    "test2.txt",
    "test3.txt",
    "testdir1",
    "testdir2",

    // Dir1 contents
    "child1",
    "child2",

    // Dir2 contents
    "child_a",
    "child_b",
};

filesystem_type_t testfs_type = {
    "TESTFS",

    tfs_initmnt,
    tfs_retrieve_child,
    tfs_node_from_id,
    tfs_save_node,
    tfs_open,
    tfs_close,
    tfs_seek,
    tfs_read,
    tfs_write,
    tfs_newfile,
    tfs_newdir,
    tfs_delnode,
};

int tfs_initmnt(mount_t *mnt)
{
    if (mnt->dev >= 0) {
	printk("[test_fs] Cannot mount using any store.");
	return -1;
    }

    mnt->root_fsnode = ID_ROOT_DIR;
    mnt->fs = &testfs_type;
    return 0;
}

long tfs_retrieve_child(fsnode_t *parent, const char *name, size_t name_len)
{
    if (parent->internal_id == ID_ROOT_DIR) {
        for (int i = 0; i < __ROOT_SENTINAL; i++) {
            if (strncmp(name, node_names[i], name_len) == 0) {
                return i;
            }
        }
    } else if (parent->internal_id == ID_TESTDIR1) {
        for (int i = __ROOT_SENTINAL; i < __DIR1_SENTINAL; i++) {
            if (strncmp(name, node_names[i], name_len) == 0) {
                return i;
            }
        }
    } else if (parent->internal_id == ID_TESTDIR2) {
        for (int i = __DIR1_SENTINAL; i < __DIR2_SENTINAL; i++) {
            if (strncmp(name, node_names[i], name_len) == 0) {
                return i;
            }
        }
    }

    return -1;
}

int tfs_node_from_id(long id, fsnode_t *node)
{
    if (id < 0 || id >= __FINAL_SENTINAL) return -1;

    // We expect that node->mountpoint is already filled in

    node->internal_id = id;
    node->perms = FP_ROTH | FP_RUSR | FP_RGRP | FP_WUSR;

    if (id == ID_ROOT_DIR || id == ID_TESTDIR1 || id == ID_TESTDIR2) {
        node->perms |= FP_XOTH | FP_XUSR | FP_XGRP;
        node->type = DIRECTORY;
    } else {
        node->type = FILE;
    }

    node->access_time = 0;
    node->mod_time = 0;
    node->create_time = 0;

    node->ref_count = 0; // At this point, no open files are using this.
    node->child = NULL;  // This will get set if/when at least one child is
                         // loaded into memory.
    return 0;
}

int tfs_read(openfile_t *file, size_t nbytes, char *kbuffer)
{
    static const char msg[]
        = "Hello, world! This is my test string, which is inside all of the "
          "files in a testfs filesystem! Wow!!!";

    if (file->cursor >= (long)sizeof(msg)) return -1;

    strncpy(kbuffer, msg + file->cursor, nbytes);
    return MIN(nbytes, sizeof(msg));
}

int tfs_save_node(fsnode_t *file)
{
    return -1;
}

int tfs_open(fsnode_t *node)
{
    return 0;
}

int tfs_close(openfile_t *file)
{
    return 0;
}

int tfs_seek(openfile_t *file, long offset, int anchor)
{
    return 0;
}

int tfs_write(openfile_t *file, size_t nbytes, const char *buffer)
{
    return -1;
}

int tfs_newfile(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t)
{
    return -1;
}

int tfs_newdir(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t)
{
    return -1;
}

int tfs_delnode(fsnode_t *, const char *)
{
    return -1;
}
