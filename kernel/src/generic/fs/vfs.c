#include "fors/filesystem.h"
#include "fors/kheap.h"
#include "forslib/string.h"

// Canonical path form:
//
// 1) Start with a mount tag, in the form :<tag>:
// 2) The tag is immediately followed (no whitespace) by...
// 3) the first part of the path, be it a file or dir. No leading /.
// 4.5) Unless it refers to the root dir, in which case just the tag is needed.
// 4) Further parts of the path are separated by exactly one '/'.
// 5) The path may not end with a /.
//
// Examples of canonical paths (with notes stating what they refer to)
//
// :drive1:my_docs/school/homework1.txt    # A file in the directory "school"
// :usb:arch_linux_live_disk.iso           # A file in the root of a drive
// :backup:backups/2023-12-07              # A directory
// :ssd1:                                  # The root directory of drive "ssd1"
// :tag::a_nice_file.txt            # In this weird case :tag: is the tag and
// ':a_nice_file.txt' is the path
//
// Examples of non-canonical paths (with notes explaining what's wrong)
//
// :drive1:/my_docs/my_paper.txt    # There cannot be a leading slash for the
//                                  # first part of the path.
// :drive1:my_docs//my_paper.txt    # Only one '/' is allowed to separate parts.
// /usr/bin/helix                   # A mount tag needs to be specified.
// :usr:bin/helix/                  # No trailing /

file *first_file = NULL;
fsnode *first_fsnode = NULL;
mountpoint *first_mountpoint = NULL;

size_t next_node_vfs_id = 0;

#define FS_LIST_ADD(start, new)                                                \
  if (start)                                                                   \
    start->prev = new;                                                         \
  new->next = start;                                                           \
  new->prev = NULL;                                                            \
  start = new;

#define FS_LIST_REMOVE(start, del)                                             \
  if (del->next)                                                               \
    del->next->prev = del->prev;                                               \
  if (del->prev)                                                               \
    del->prev->next = del->next;                                               \
  if (!del->next && !del->prev)                                                \
    start = NULL;

#define FS_LIST_FIND(start, memb, val, res)                                    \
  for (typeof(start) m = start; m; m = m->next) {                              \
    if (m->memb == val) {                                                      \
      res = m;                                                                 \
      break;                                                                   \
    }                                                                          \
  }

#define FS_LIST_FINDSTR(start, str_memb, val, res)                             \
  for (typeof(start) m = start; m; m = m->next) {                              \
    if (strcmp(m->memb, val) == 0) {                                           \
      res = m;                                                                 \
      break;                                                                   \
    }                                                                          \
  }

bool is_canonical(const char *path) {
  const char *tag_end;
  size_t tag_len;

  if (!path)
    return false;

  // Check tag
  if (path[0] != ':')
    return false; // No starting : for tag
  tag_end = strchr(path + 1, ':');
  if (!tag_end)
    return false; // No closing : for tag
  tag_len = tag_end - path - 1;
  if (tag_len == 0 || tag_len > TAG_SIZE)
    return false; // No/too long tag

  path = tag_end + 1;
  if (path[0] == '/')
    return false; // Rule 3)
  for (; *path; path++) {
    if (path[0] == '/' && (path[1] == '\0' || path[1] == '/'))
      return false; // Rules 4,5
  }

  return true;
}

/// Attempt to make a path conform to canonical path notation.
/// This will:
///  - Remove whitespace between tag and path proper
///  - Remove leading '/'
///  - Shorten any runs of '/' down to a single one
///  - Remove trailing '/'
/// This is meant just to fix small stylistic errors though, and can't
/// fix everything.
//
// `path` : The path to try to make canonical.
//
// Returns 0 if path is now canonical, or -1 if it's (still) not
int try_make_canonical(char *path, size_t *len_res) {
  char *save_path = path;
  if (is_canonical(path)) {
    *len_res = strlen(path);
    return 0;
  }

  char *tag_end = strchr(path + 1, ':');
  if (tag_end == NULL)
    return -1;
  int tag_len = tag_end - path + 1;

  char *buff = kalloc(strlen(path) + 1);
  int buff_i = 0;

  // First, we assume that the tag is correct. If it's not, there's nothing
  // we can do about it anyway.
  strncpy(buff, path, tag_len);
  buff_i += tag_len;

  for (path = tag_end + 1; *path; path++) {
    if (path[0] != '/') {
      buff[buff_i++] = path[0];
    } else {
      // Don't add trailing or double(or more) slashes
      // or leading slashes
      if (path[1] == '\0')
        break;
      else if (path[1] == '/') {
        bool found_nonslash = false;
        for (char *c = path + 1; *c; c++) {
          if (*c != '/') {
            if (path != tag_end + 1)
              buff[buff_i++] = '/';
            path = c - 1; // Skip adjacent slashes
            found_nonslash = true;
            break;
          }
        }
        // printf("\n");
        if (!found_nonslash)
          break; // Got to end of path, just break
      } else {
        if (path != tag_end + 1)
          buff[buff_i++] = '/';
      }
    }
  }

  buff[buff_i++] = '\0';

  strcpy(save_path, buff);
  kfree(buff);

  return is_canonical(path);
}

/// Opens a file on behalf of a given process, creating a new `file` object to
/// represent this instance.
//
// `tag`   : The mountpoint tag where the desired file is, i.e. what's between
//           the ':'s at the start of the full path.
// `path`  : The path to the file within the specified mountpoint
// `pid`   : The process which opens the file. If this is -1, then it's opened
//         for the kernel only.
// `access`: Access mode for this file. Refer to `enum fileaccess_e`
//
// Returns the new file's `file.fd`, or if an error occurred, returns -1
int vfs_open(const char *tag, const char *path, int pid, fileaccess access) {
  static size_t next_fd = 0;
  mountpoint *mp = NULL;

  for (mountpoint *c = first_mountpoint; c; c = c->next) {
    if (strncmp(c->tag, tag, TAG_SIZE) == 0)
      mp = c;
  }

  if (!mp)
    return -1; // Mountpoint not found

  fsnode *node = mp->root;
  if (!node)
    return -1; // Mountpoint has no root node!

  while (1) {
    const char *delim = strchrnul(path, '/'); // Next slash OR string null term
    size_t part_len = delim - path;

    if (part_len == 0)
      break; // Must be fully empty path

    if (mp->fs->lookup(node, path, part_len, &node) < 0) {
      return -1; // fs failed lookup
    }

    if (*delim == '\0')
      break; // Got to end of path
    else
      path = delim + 1;
  }

  if (node->type == NT_DIR)
    return -1; // Can't open a directory

  file *new_file = kalloc(sizeof(file));
  new_file->access = access;
  new_file->pid = pid;
  new_file->node = node;
  new_file->fd = next_fd;

  if (mp->fs->open(new_file) < 0) {
    kfree(new_file);
    return -1;
  }

  FS_LIST_ADD(first_file, new_file);

  // TODO when per-process fd tables are implemented, this should add a local
  //  descriptor to the corresponding one.

  return next_fd++;
}

int vfs_close(size_t global_fd, int pid) {
  file *f = NULL;

  FS_LIST_FIND(first_file, fd, global_fd, f);
  if (!f)
    return -1; // Couldn't find file matching fd

  if (f->node->mount->fs->close(f, pid) < 0)
    return -1;

  FS_LIST_REMOVE(first_file, f);
  kfree(f);
  return 0;
}

int vfs_seek(size_t global_fd, int pid, size_t off, seek_anchor anc) {
  file *f = NULL;

  FS_LIST_FIND(first_file, fd, global_fd, f);
  if (!f)
    return -1;

  return f->node->mount->fs->seek(f, off, anc);
}

int vfs_read(size_t global_fd, int pid, size_t bytes, char *buff) {
  file *f = NULL;

  FS_LIST_FIND(first_file, fd, global_fd, f);
  if (!f)
    return -1;

  // TODO: Check that buffer is fully mapped in the process's address space

  return f->node->mount->fs->read(f, bytes, buff);
}

int vfs_write(size_t global_fd, int pid, size_t bytes, const char *buff) {
  file *f = NULL;

  FS_LIST_FIND(first_file, fd, global_fd, f);
  if (!f)
    return -1;

  // TODO: Check that buffer is fully mapped in the process's address space

  return f->node->mount->fs->write(f, bytes, buff);
}

int vfs_mount(const char *tag, int device, const char *param,
              mountpoint *mount_res, filesystem *fs) {
  if (strlen(tag) > TAG_SIZE)
    return -1;

  mountpoint *mnt = kalloc(sizeof(mountpoint));
  mnt->device = device;
  memcpy(mnt->tag, tag, strlen(tag));
  mnt->tag[strlen(tag)] = '\0';
  mnt->fs = fs;
  mnt->dirty = false;

  // This should fill in mnt->root
  if (fs->mount(tag, device, param, mnt) < 0) {
    kfree(mnt);
    return -1;
  }

  FS_LIST_ADD(first_mountpoint, mnt);
  return 0;
}

int vfs_read_node(mountpoint *mount, size_t fs_specific_id, fsnode *node_res) {
  fsnode *node = kalloc(sizeof(fsnode));
  node->mount = mount;
  node->dirty = false;
  node->vfs_id = next_node_vfs_id++;
  node->node_id = fs_specific_id;

  // Sets up link_count, type
  if (mount->fs->read_node(mount, node) < 0) {
    kfree(node);
    return -1;
  }

  FS_LIST_ADD(first_fsnode, node);
  return 0;
}

int vfs_new_node(mountpoint *mount, nodetype type) {
  if (!mount)
    return -1;

  fsnode *node = kalloc(sizeof(fsnode));
  node->vfs_id = next_node_vfs_id++;
  node->dirty = false;
  node->mount = mount;
  node->type = type;

  if (mount->fs->new_node(mount, type) < 0) {
    kfree(node);
    return -1;
  }

  FS_LIST_ADD(first_fsnode, node);
  return 0;
}

int vfs_unlink_node(fsnode *dir, const char *name) {
  if (!dir)
    return -1;

  fsnode *to_unlink;
  dir->mount->fs->lookup(dir, name, strlen(name), &to_unlink);

  if (!to_unlink)
    return -1; // No node called name in dir

  for (file *f = first_file; f; f = f->next) {
    if (f->node == to_unlink)
      return -1; // This node is in use
  }

  // It's expected that this call will decrease the node's reference counter.
  // And also, of course, change some internal representation so that the node
  // is not present in that directory anymore. It may still be in others.
  if (dir->mount->fs->unlink_node(dir, name) < 0)
    return -1;

  if (--to_unlink->link_count <= 0) {
    // Guaranteed that there are no files open referring to this node.
    FS_LIST_REMOVE(first_fsnode, to_unlink);
    kfree(to_unlink);
  }

  return 0;
}

int vfs_save_all(mountpoint *mount) { return mount->fs->save_all(mount); }
int vfs_save_node(fsnode *node) { return node->mount->fs->save_node(node); }
int vfs_link_node(fsnode *dir, const char *name, fsnode *to_link) {
  return dir->mount->fs->link_node(dir, name, to_link);
}
int vfs_lookup(fsnode *dir, const char *name, size_t name_len,
               fsnode **node_res) {
  return dir->mount->fs->lookup(dir, name, name_len, node_res);
}
int vfs_num_children(fsnode *dir) { return dir->mount->fs->num_children(dir); }
int vfs_list_dir(fsnode *dir, list_dir_entry *entries) {
  return dir->mount->fs->list_dir(dir, entries);
}
