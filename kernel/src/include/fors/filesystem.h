#ifndef __INCLUDE_FORS_FILESYSTEM_H__
#define __INCLUDE_FORS_FILESYSTEM_H__

#include <stdbool.h>
#include <stddef.h>

typedef enum nodetype_e {
  NT_FILE,
  NT_DIR,
} nodetype;
typedef enum fileaccess_e {
  FA_READONLY,
  FA_WRITEONLY,
  FA_READWRITE,
} fileaccess;
typedef enum seek_anchor_e {
  SA_START,
  SA_END,
  SA_OFFSET,
} seek_anchor;

#define TAG_SIZE 8

typedef struct mountpoint_t {
  char tag[TAG_SIZE + 1];
  struct fsnode_t *root;
  struct filesystem_t *fs;
  int device; // -1 if no device used, otherwise the device ID
  bool dirty; // Was any part of this FS modified since last save to "disk"?

  struct mountpoint_t *next, *prev;
} mountpoint;

typedef struct fsnode_t {
  size_t vfs_id;  // Global ID, unique among all nodes
  size_t node_id; // FS-specific ID, use this however, used when reading and
                  // saving nodes from disk
  mountpoint *mount;
  nodetype type;
  bool dirty;     // Was this node modified since last save to "disk"?
  int link_count; // How many directory entries link to this node?
  void *priv;     // Private data

  struct fsnode_t *next, *prev;
} fsnode;

typedef struct file_t {
  size_t fd; // Global fd
  fsnode *node;
  int pid;
  fileaccess access;

  struct file_t *next, *prev;
} file;

typedef struct list_dir_entry_t {
  const char *name;
  fsnode *node;
} list_dir_entry;

typedef struct filesystem_t {
  int (*mount)(const char *tag, int device, const char *param,
               mountpoint *mount_res); // Create mountpoint
  int (*save_all)(mountpoint *mount);  // Make sure all files and everything
                                       // about the FS is "saved"

  int (*read_node)(mountpoint *mount,
                   fsnode *node_res); // Read existing node from disk
  int (*save_node)(fsnode *node);     // "Save" node.
  int (*new_node)(mountpoint *mount, nodetype type); // Create a brand new node.

  int (*link_node)(fsnode *dir, const char *name, fsnode *to_link);
  int (*unlink_node)(fsnode *dir, const char *name);

  int (*lookup)(
      fsnode *dir, const char *name, size_t name_len,
      fsnode **node_res); // Sets node_res if name found in dir, otherwise NULL
  int (*num_children)(
      fsnode *dir); // How many child nodes are in the directory?
  int (*list_dir)(
      fsnode *dir,
      list_dir_entry *entries); // Must allocate space for `num_children()`
                                // entries before calling

  int (*open)(
      file *opened); // any additional open operations after standard vfs stuff
  int (*close)(file *to_close, int pid);
  int (*seek)(file *f, size_t offset, seek_anchor anchor);
  int (*read)(file *f, size_t bytes, char *buff);
  int (*write)(file *f, size_t bytes, const char *buff);
} filesystem;

// Wrapper functions around FS methods

int vfs_mount(const char *tag, int device, const char *param,
              mountpoint *mount_res, filesystem *fs);
int vfs_open(const char *tag, const char *path, int pid, fileaccess access);
int vfs_close(size_t global_fd, int pid);
int vfs_seek(size_t global_fd, int pid, size_t off, seek_anchor anc);
int vfs_read(size_t global_fd, int pid, size_t bytes, char *buff);
int vfs_write(size_t global_fd, int pid, size_t bytes, const char *buff);
int vfs_read_node(mountpoint *mount, size_t fs_specific_id, fsnode *node_res);
int vfs_new_node(mountpoint *mount, nodetype type);
int vfs_unlink_node(fsnode *dir, const char *name);
// (Stubs)
int vfs_save_all(mountpoint *mount);
int vfs_save_node(fsnode *node);
int vfs_link_node(fsnode *dir, const char *name, fsnode *to_link);
int vfs_lookup(fsnode *dir, const char *name, size_t name_len,
               fsnode **node_res);
int vfs_num_children(fsnode *dir);
int vfs_list_dir(fsnode *dir, list_dir_entry *entries);

#endif /* __INCLUDE_FORS_FILESYSTEM_H__ */
