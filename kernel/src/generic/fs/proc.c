#include "fors/filesystem.h"
#include "fors/kheap.h"
#include "fors/printk.h"
#include "forslib/string.h"

int procfs_mount(const char *tag, int device, const char *param,
                 mountpoint *mount_res);
int procfs_save_all(mountpoint *mount);
int procfs_read_node(mountpoint *mount, fsnode *node_res);
int procfs_save_node(fsnode *node);
int procfs_new_node(mountpoint *mount, nodetype type);
int procfs_link_node(fsnode *dir, const char *name, fsnode *to_link);
int procfs_unlink_node(fsnode *dir, const char *name);
int procfs_lookup(fsnode *dir, const char *name, size_t name_len,
                  fsnode **node_res);
int procfs_num_children(fsnode *dir);
int procfs_list_dir(fsnode *dir, list_dir_entry *entries);
int procfs_open(file *opened);
int procfs_close(file *to_close, int pid);
int procfs_seek(file *f, size_t offset, seek_anchor anchor);
int procfs_read(file *f, size_t bytes, char *buff);
int procfs_write(file *f, size_t bytes, const char *buff);

filesystem fs_procfs = {
    .mount = procfs_mount,
    .save_all = procfs_save_all,
    .read_node = procfs_read_node,
    .save_node = procfs_save_node,
    .new_node = procfs_new_node,
    .link_node = procfs_link_node,
    .unlink_node = procfs_unlink_node,
    .lookup = procfs_lookup,
    .num_children = procfs_num_children,
    .list_dir = procfs_list_dir,
    .open = procfs_open,
    .close = procfs_close,
    .seek = procfs_seek,
    .read = procfs_read,
    .write = procfs_write,
};

int procfs_mount(const char *tag, int device, const char *param,
                 mountpoint *mount_res) {
  if (device != -1)
    return -1; // Procfs doesn't use a device

  printk("Mounting procfs with params '%s'\n", param);

  fsnode *root = kalloc(sizeof(fsnode));
  if (!root)
    return -1;

  return 0;
}

int procfs_save_all(mountpoint *mount) { return 0; }

int procfs_read_node(mountpoint *mount, fsnode *node_res) {
  if (node_res->node_id == 0) {
    node_res->link_count = 1;
    node_res->type = NT_FILE;
    return 0;
  } else {
    return -1;
  }
}

int procfs_save_node(fsnode *node) { return 0; }

int procfs_new_node(mountpoint *mount, nodetype type) { return -1; }

int procfs_num_children(fsnode *dir) {
  if (dir == dir->mount->root)
    return 1;
  return 0;
}

int procfs_list_dir(fsnode *dir, list_dir_entry *entries) {
  if (dir == dir->mount->root) {
    entries[0].name = "procfile";
    vfs_read_node(dir->mount, 0, entries[0].node);
    return 0;
  } else {
    return -1;
  }
}

