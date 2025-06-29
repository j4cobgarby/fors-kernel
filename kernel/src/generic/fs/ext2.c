#include "fors/filesystem.h"
#include "fors/fs/ext2.h"
#include "fors/store.h"
#include "fors/printk.h"

// Converting between byte address and EXT2 block index
static size_t byte2blk(uint32_t blksz, size_t byte) {
    return byte / (1024 << blksz);
}

static size_t blk2byte(uint32_t blksz, size_t blk) {
    return blk * (1024 << blksz);
}

static ext2_superblock_t *get_sb(mount_t *mnt) {
    store_t *st = get_store_by_id(mnt->dev);
    if (!st) return NULL;

    const size_t lba = 1024 / st->type->seg_sz;

    ext2_superblock_t *sb;
    int ret = bc_get(mnt->dev, lba, (char**)&sb);
    if (!ret) return NULL;

    return sb;
}

static size_t blk2lba(mount_t *mnt, size_t blk) {
    store_t *st = get_store_by_id(mnt->dev);
    if (!st) return -1;

    return blk2byte(get_sb(mnt)->blk_sz, blk) / st->type->seg_sz;
}

static int n_blkgroups(mount_t *mnt) {
    ext2_superblock_t *sb = get_sb(mnt);
    if (!sb) return -1;
    return (sb->n_blks + (sb->blks_per_group-1)) / sb->blks_per_group;
}

int ext2_initmnt(mount_t *mnt)
{
    store_t *st = get_store_by_id(mnt->dev);
    if (!st) return -1;

    ext2_superblock_t *sb = get_sb(mnt);
    printk("== ext2 superblock ==\n");
    printk(" - n_ino: %d\n", sb->n_ino);
    printk(" - n_blks: %d\n", sb->n_blks);
    printk(" - n_blks_resvd: %d\n", sb->n_blks_resvd);
    printk(" - n_blks_unalloc: %d\n", sb->n_blks_unalloc);
    printk(" - n_ino_unalloc: %d\n", sb->n_ino_unalloc);
    printk(" - superblk_ind: %d\n", sb->superblk_ind);
    printk(" - blk_sz: %d\n", sb->blk_sz);
    printk(" - frag_sz: %d\n", sb->frag_sz);
    printk(" - blks_per_group: %d\n", sb->blks_per_group);
    printk(" - frags_per_group: %d\n", sb->frags_per_group);
    printk(" - ino_per_group: %d\n", sb->ino_per_group);
    printk(" - time_mounted: %d\n", sb->time_mounted);
    printk(" - time_written: %d\n", sb->time_written);
    printk(" - mounts_since_check: %d\n", sb->mounts_since_check);
    printk(" - max_mounts_since_check: %d\n", sb->max_mounts_since_check);
    printk(" - signature: %x\n", sb->signature);
    printk(" - vers_minor: %d\n", sb->vers_minor);
    printk(" - time_last_fsck: %d\n", sb->time_last_fsck);
    printk(" - time_between_force_fsck: %d\n", sb->time_between_force_fsck);
    printk(" - os_id: %d\n", sb->os_id);
    printk(" - vers_major: %d\n", sb->vers_major);
    printk(" - resvd_uid: %d\n", sb->resvd_uid);
    printk(" - resvd_gid: %d\n", sb->resvd_gid);
    printk(" - sz_ino_struct: %d\n", sb->sz_ino_struct);

    fsnode_t nd;
    nd.mountpoint = mnt;
    int ret = ext2_node_from_id(2, &nd);
    if (ret < 0) {
	printk("Failed to get inode.\n");
    } else {
	printk("Got inode!\n");
	printk("Creation time: %d\n", nd.create_time);
    }

    // printk("num block groups = %d\n", n_blkgroups(mnt));
    //
    // // The sb is at byte 1024, and is 1024 bytes long.
    // // The block group descriptor table is at the block which follows.
    // size_t table_seg = blk2lba(mnt, 1 + byte2blk(sb->blk_sz, 1024));
    //
    // ext2_blkgroup_descriptor_t *table;
    // int ret = bc_get(mnt->dev, table_seg, (char**)&table);
    // if (!ret) return -1;
    //
    // printk("== first block group descriptor ==\n");
    // printk(" - block_bitmap_addr: %d\n", table[0].block_bitmap_addr);
    // printk(" - inode_bitmap_addr: %d\n", table[0].inode_bitmap_addr);
    // printk(" - inode_table_addr: %d\n", table[0].inode_table_addr);
    // printk(" - n_unalloc_blocks: %d\n", table[0].n_unalloc_blocks);
    // printk(" - n_unalloc_inodes: %d\n", table[0].n_unalloc_inodes);
    // printk(" - n_dirs: %d\n", table[0].n_dirs);
    //
    // char *inodes;
    // ret = bc_get(mnt->dev, blk2lba(mnt, table[0].inode_table_addr), &inodes);
    // if (!ret) return -1;
    //
    // const int ino_n = 2;
    // ext2_inode_t *ino = (ext2_inode_t *)&inodes[(ino_n - 1) * sb->sz_ino_struct];
    //
    // printk("== inode #%d ==\n", ino_n);
    // printk(" - type_perm: %x\n", ino->type_perm);
    // printk(" - uid: %d\n", ino->uid);
    // printk(" - sz_lo: %d\n", ino->sz_lo);
    // printk(" - t_access: %d\n", ino->t_access);
    // printk(" - t_create: %d\n", ino->t_create);
    // printk(" - t_modify: %d\n", ino->t_modify);
    // printk(" - t_delete: %d\n", ino->t_delete);
    // printk(" - gid: %d\n", ino->gid);
    // printk(" - n_hard_links: %d\n", ino->n_hard_links);
    // printk(" - n_sects_on_disk: %d\n", ino->n_sects_on_disk);
    // printk(" - flags: %x\n", ino->flags);
    // printk(" - os_val: %d\n", ino->os_val);
    // printk(" - blk0: %d\n", ino->blk0);
    // printk(" - blk1: %d\n", ino->blk1);
    // printk(" - blk2: %d\n", ino->blk2);
    // printk(" - blk3: %d\n", ino->blk3);
    // printk(" - blk4: %d\n", ino->blk4);
    // printk(" - blk5: %d\n", ino->blk5);
    // printk(" - blk6: %d\n", ino->blk6);
    // printk(" - blk7: %d\n", ino->blk7);
    // printk(" - blk8: %d\n", ino->blk8);
    // printk(" - blk9: %d\n", ino->blk9);
    // printk(" - blk10: %d\n", ino->blk10);
    // printk(" - blk11: %d\n", ino->blk11);
    // printk(" - blk_list: %d\n", ino->blk_list);
    // printk(" - blk_list_2: %d\n", ino->blk_list_2);
    // printk(" - blk_list_3: %d\n", ino->blk_list_3);
    // printk(" - generation: %d\n", ino->generation);
    // printk(" - attr_ext: %d\n", ino->attr_ext);
    // printk(" - sz_hi: %d\n", ino->sz_hi);

    mnt->fs = &ext2_type;

    return 0;
}

long ext2_retrieve_child(fsnode_t *parent, const char *name, size_t name_len)
{
    return 0;
}

int ext2_node_from_id(long id, fsnode_t *node)
{
    ext2_superblock_t *sb = get_sb(node->mountpoint);
    size_t blk_sz = sb->blk_sz;
    size_t sz_ino_struct = sb->sz_ino_struct;

    size_t blk_group = (id - 1) / sb->ino_per_group;
    size_t ind_in_grp = (id - 1) % sb->ino_per_group;

    // Calculate the LBA of the block group table
    size_t table_seg = blk2lba(node->mountpoint, 1 + byte2blk(blk_sz, 1024));

    // Load the block group table into memory
    // (Will it always fit into one sector?)
    ext2_blkgroup_descriptor_t *table;
    int ret = bc_get(node->mountpoint->dev, table_seg, (char**)&table);
    if (!ret) return -1;

    // Look-up the block group which the target inode resides in, finding the start block of the inode table
    size_t inode_table_first_blk = table[blk_group].inode_table_addr;

    // The inode table of this block group can span many blocks, so find which one we need to look at
    size_t blk = inode_table_first_blk + (ind_in_grp * sz_ino_struct) / (1024 << blk_sz);

    // The index of the inode within this specific block = (ind_in_grp * sz_ino_struct) % blk_size
    // So here we calculate the byte on the disk which the inode starts at
    size_t start_byte = (blk * (1024 << blk_sz)) + (ind_in_grp * sz_ino_struct) / (1024 << blk_sz);

    size_t lba = blk2lba(node->mountpoint, blk);
    size_t byte_in_seg = start_byte - (lba * get_store_by_id(node->mountpoint->dev)->type->seg_sz);

    char *ino_table_seg;
    ret = bc_get(node->mountpoint->dev, lba, &ino_table_seg);

    ext2_inode_t *ino = (ext2_inode_t*)(ino_table_seg + byte_in_seg);

    node->access_time = ino->t_access;
    node->create_time = ino->t_create;
    node->mod_time = ino->t_modify;
    node->group = ino->gid;
    node->user = ino->uid;
    node->internal_id = id;
    
    return 0;
}

int ext2_read(openfile_t *file, size_t nbytes, char *kbuffer)
{
    return 0;
}

int ext2_save_node(fsnode_t *file)
{
    return 0;
}

int ext2_open(fsnode_t *node)
{
    return 0;
}

int ext2_close(openfile_t *file)
{
    return 0;
}

int ext2_seek(openfile_t *file, long offset, int anchor)
{
    return 0;
}

int ext2_write(openfile_t *file, size_t nbytes, const char *buffer)
{
    return 0;
}

int ext2_newfile(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t)
{
    return 0;
}

int ext2_newdir(fsnode_t *, const char *, fsn_perm_t, uid_t, gid_t)
{
    return 0;
}

int ext2_delnode(fsnode_t *, const char *)
{
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
