/* ext2.h
 *
 * Declarations for EXT2 filesystem.
 *
 */

#ifndef __INCLUDE_FS_EXT2_H__
#define __INCLUDE_FS_EXT2_H__

#include "fors/filesystem.h"

#include <stdint.h>

int ext2_initmnt(mount_t *mnt);
long ext2_retrieve_child(fsnode_t *parent, const char *name, size_t name_len);
int ext2_node_from_id(long id, fsnode_t *node);
int ext2_read(openfile_t *file, size_t nbytes, char *kbuffer);
int ext2_save_node(fsnode_t *file);
int ext2_open(fsnode_t *node);
int ext2_close(openfile_t *file);
int ext2_seek(openfile_t *file, long offset, int anchor);
int ext2_write(openfile_t *file, size_t nbytes, const char *buffer);
int ext2_newfile(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t);
int ext2_newdir(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t);
int ext2_delnode(fsnode_t *, const char *);

extern filesystem_type_t ext2_type;

typedef enum ext2_state_t : uint16_t { S_CLEAN, S_ERRORS } ext2_state_t;

typedef enum ext2_errhandling_t : uint16_t {
    H_IGNORE,
    H_RO,
    H_PANIC,
} ext2_errhandling_t;

typedef struct ext2_superblock_t {
    uint32_t n_ino;
    uint32_t n_blks;
    uint32_t n_blks_resvd;
    uint32_t n_blks_unalloc;
    uint32_t n_ino_unalloc;
    uint32_t superblk_ind;
    uint32_t blk_sz;  // log2(blk size) - 10
    uint32_t frag_sz; // log2(frag size) - 10
    uint32_t blks_per_group;
    uint32_t frags_per_group;
    uint32_t ino_per_group;
    uint32_t time_mounted;
    uint32_t time_written;
    uint16_t mounts_since_check;
    uint16_t max_mounts_since_check;
    uint16_t signature; // Should be 0xef53
    ext2_state_t state;
    ext2_errhandling_t error_method;
    uint16_t vers_minor;
    uint32_t time_last_fsck;
    uint32_t time_between_force_fsck;
    uint32_t os_id;
    uint32_t vers_major;
    uint16_t resvd_uid;
    uint16_t resvd_gid;
} ext2_superblock_t;

typedef struct ext2_extended_superblock_t {
    uint32_t first_avail_ino;
    uint16_t sz_ino_struct;
    uint16_t super_block_group;
    uint32_t opt_features; // Optional features
    uint32_t req_features; // Reqd. features
    uint32_t wr_features;  // Features reqd. for writing
    char fs_id[16];
    char volume_name[16];
    char last_mountpoint[64];
    uint32_t compress_algo;
    uint8_t prealloc_blocks_file;
    uint8_t prealloc_blocks_dir;
    uint16_t _unused;
    char journ_id[16];
    uint32_t journ_inode;
    uint32_t journ_device;
    uint32_t orphans_head;
} ext2_extended_superblock_t;

// Required features flags
#define EXT2_REQF_COMPRESSION 0x01
#define EXT2_REQF_TYPEDDIRS   0x02
#define EXT2_REQF_REPLAYJOURN 0x04
#define EXT2_REQF_USESJOURN   0x08

// Write-required features flags
#define EXT2_WREQF_SPARSE    0x01
#define EXT2_WREQF_64BITFILE 0x02
#define EXT2_WREQF_BTREE     0x04

#endif
