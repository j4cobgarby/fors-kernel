#ifndef __INCLUDE_FORS_FILESYSTEM_H__
#define __INCLUDE_FORS_FILESYSTEM_H__

#include <stdbool.h>
#include <stddef.h>

// Flags for mountpoint.flags
#define MF_RDONLY (1 << 0) // Is this mountpoint read only?
#define MF_EXEC   (1 << 1) // Are programs on this mountpoint executable?

typedef struct mountpoint {
    char *tag; // Name used to refer to this mountpoint
    int device; // Which device (if any) does this mountpoint store its data on?
    struct filesystem_type *type;
    unsigned flags;
} mountpoint;

typedef struct filesystem_type {
    char *name;

    // Set up a mountpoint type, m, taking into account parameters in opt. The mountpoint
    // gets tagged with the string in tag.
    // m.device and m.flags should already be set, so that this function knows
    // which device to use and how to mountpoint it.
    // The string tag is copied, so it doesn't need to stay in memory.
    int (*setup_mountpoint)(mountpoint *m, char *opt, const char *tag);

    
} filesystem_type;

typedef struct inode {
    // FS specific ID. In some cases, this will be the inode number within the
    // FS, or it could be anything really. 
    unsigned long id; 
    
    unsigned long blksize;
    unsigned long blks;

    bool dirty; // Has this inode been modified since last saved?

    mountpoint *mount; // Which mountpoint is this inode within?
} inode;

// Methods which act on a mountpoint as a whole.
typedef struct mountpoint_methods {
    // Sets up a given inode.
    // Similar to filesystem_type.setup_mountpoint. The inode should already
    int (*setup_inode)(struct inode *);
} mountpoint_methods;

typedef struct inode_methods {
    
} inode_methods;

typedef struct file_methods {
    
} file_methods;

/// Filesystem Design
//
// A mountpointed filesystem is associated with a 'tag' which used used when
// referring to files in it. For example, if the user mountpoints a filesystem
// with the tag 'root', then files on it could be accessed as so:
//  :root/home/jacob/my_document.txt
// And if the current working directory is already in :root, then this same
// path could equally be referred to as:
//  /home/jacob/my_document.txt
//

#endif /* __INCLUDE_FOSR_FILESYSTEM_H__ */
