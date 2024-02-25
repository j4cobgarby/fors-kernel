#ifndef INCLUDE_FS_TEST_H_
#define INCLUDE_FS_TEST_H_

#include "fors/filesystem.h"

/* Some extra functionality we need:
 *  - Function to, given a mountpoint and an internal fsnode id, return the
 * given fsnode if it exists otherwise query the fs.
 */

int tfs_initmnt(mount_t *mnt);
long tfs_retrieve_child(fsnode_t *parent, const char *name, size_t name_len);
int tfs_node_from_id(long id, fsnode_t *node);
int tfs_read(openfile_t *file, size_t nbytes, char *kbuffer);
int tfs_save_node(fsnode_t *file);
int tfs_open(fsnode_t *node);
int tfs_close(openfile_t *file);
int tfs_seek(openfile_t *file, long offset, int anchor);
int tfs_write(openfile_t *file, size_t nbytes, const char *buffer);
int tfs_newfile(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t);
int tfs_newdir(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t);
int tfs_delnode(fsnode_t *, const char *);

extern filesystem_type_t testfs_type;

#endif // INCLUDE_FS_TEST_H_
