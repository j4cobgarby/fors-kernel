# Filesystem

## Overview

### Filesystem Nodes (`fsnode_t`)

In memory, the information and data about files from any filesystem type (e.g. ext2, fat, virtual ones) are represented by the same structure -- the `fsnode_t`. This provides a seamless interface to various types. fsnodes are conceptually very similar to inodes found in UNIX. They are an **in-core representation of the metadata and actual data about a file or directory on a real disk, or of that in a virtual filesystem**.

Memory for fsnodes is not actually allocated dynamically. There is a fixed maximum number of fsnodes, `NUM_FSNODES`. All fsnodes are stored in `fsnodes`.

**fsnodes are not files** -- multiple different files with different names and locations can refer to the same fsnode. **files** (name-fsnode mappings) are represented by entries in directories, and **fsnodes** (filesystem node metadata objects) are _pointed to_ by files.

There isn't necessarily (or usually) an fsnode in the system for every single node on the filesystem media.

So, what happens when the `fsnodes` array does fill up? Basically, when no more open files refer to a specific fsnode, it is seen as "free", so when a filesystem implementation is trying to read in a disk-node to an fsnode, they will see a free entry in the array and reallocate it.

### Open Files (`openfile_t`)

As hinted at, when a process opens a file, an `openfile_t` structure is used to represent this. I think of open files as "cursors" on an fsnode, where there can be many all referring to the same actual file (or directory), but reading and writing at different locations.

When a file is opened by a process, the fsnode that it refers to has its `ref_count` increased by one. This is true of every "file" type, including directories. It's important to note that a directory's fsnode doesn't need to exist (in memory) for its children to have fsnodes.

## How it's used

Below are some rough pseudocode implementations of some high level filesystem functions.
They assume the existence of the following procedures:

 - `get_node(parent fsnode, name) => fsnode at parent/name` queries the fs implementation to make a new fsnode for an existing fsnode named name, directly inside the parent node.
 - `put_node(node)` frees up the entry in `fsnodes` of a given node.

 - `find_node(root fsnode, path) => fsnode at path` recurses the tree structure from root until path's parent's node is got (using `get_node`), and then uses `get_node` again to attempt to find the required node. Basically, an extended version of `get_node`.

 - `add_child(parent fsnode, new fsnode)` appends a new child node to a children list.

 - `add_child_byname(parent fsnode, new node's name)` = `add_child(parent, get_node(parent, new node's name))`

### Opening a file

Parameters:
 - path to open
 - access mode
 - process id

 1) node = find_node(system root, path to open)
 2) file = {node = node, cursor = 0, mode = mode, proc = process id}
 3) find available slot in `open_files`, and insert file
 4) increment node's reference count
 5) call the fs implementation's open method in case it wants to do anything extra.
 4) return insertion index

### Closing a file

Parameters:
 - file to close

 1) call the fs implementation's close method in case it wants to do anything extra.
 2) decrement file->node's reference counter
 3) if it is now at 0, call put_node on it
 4) mark the file's entry in `open_files` as free

### Reading a file
 - open file index
 - number of bytes to read
 - buffer in kernel address space 

 1) simply tell the fs implementation to perform this read.
 2) update access_time in the file's fsnode.

### Writing to a file

Parameters:
 - open file index
 - number of bytes to write
 - buffer in kernel address space

 1) tell the fs implementation to perform this write.
 2) update access_time and mod_time in the fsnode.

### Seeking in a file

Parameters:
 - open file index
 - seek offset
 - seek anchor type (relative, absolute, from start, from end)
 
 1) update cursor in the open file.
 2) call the fs implementation's seek method in case it needs to do anything extra.

### Creating a new file

Parameters:
 - new file whole path
 - access permissions

 1) find parent node of given path
 2) call fs implementation's mknode method, which will try and create a new fsnode (importantly, this generates a new internal_id for the fsnode)
 3) add the new fsnode to the fsnodes array.

### Creating a hard-link

Parameters:
 - the node to link to
 - the path to the new link file

 1) find the parent node of the given path (which will be a directory)
 2) call the fs implementation to add a directory entry to the parent node, pointing to the node to link to.

### Creating a directory

Parameters:
 - new directory whole path
 - access permissions

 1) find the parent node of the given path
 2) call the fs implementation's mknode method, which will try and create a new fsnode. In this case, we will specify to mknode that it should be a directory.
 3) add the new directory node to the fsnodes array.

### Deleting a directory or regular file

Parameters:
 - path to object to delete

 1) if object is a directory and it is not empty, then return here.
 2) call fs implementation either for rmdir or rmnode.
 2) remove object from its sibling linked list, thereby also making its parent not point to it.

### Reading entries from a directory
### Mounting a filesystem
