# Fors System Calls

## Filesystem Access

### Open File

Params:
 - File path relative to CWD (`const char *`)
 - Open mode
Returns:
 - Newly opened file descriptor

### Close File

Params:
 - File descriptor
Returns:
 - <0 on failure

### Read File

Params:
 - File descriptor
 - Max number of bytes to read
 - Pointer to buffer to read into
Returns:
 - Number of bytes actually read

### Write to File

Params:
 - File descriptor
 - Number of bytes to write
 - Pointer to buffer to read from
Returns:
 - Number of bytes written

### Seek in File

Params:
 - File descriptor
 - Seek offset
 - Seek type
Returns:
 - 0 on success, <0 on failure

### Get current cursor position in file

Params:
 - File descriptor
Returns:
 - cursor pos on success, <0 on failure

### Read Directory

Params:
 - File descriptor of dir
 - Number of entries to read
 - Array of `dir_entry_t` types to read into
Returns:
 - Number of entries read

### Make File (or Directory)

Params:
 - Relative path to create
 - Permissions
 - File type (File or Dir)
Returns:
 - 0 on success, <0 on failure

### Make Hard Link

Params:
 - Relative path to new file
 - Relative path to link to
Returns:
 - 0 on success, <0 on failure

### Mount Device

Params:
 - Relative path to mount directory
 - Device number
Returns:
 - 0 on success, <0 on failure

### Unmount

Params:
 - Relative path to mount directory
Returns:
 - 0 on success, <0 on failure

### Delete Node

Params:
 - Relative path to node to delete
Returns:
 - 0 on success, <0 on failure

## Process/Thread Management

### Fork

Spawns a new task as a copy of the one that issues this.

Params:
 _none_
Returns:
 - If successful:
    - To the new task: 0
    - To the original task: the ID of the new task
 - Otherwise, <0

### Quit

End the current task.

Params:
 - Exit code
Returns:
 _nothing, since the task no longer exists after the call_

### Execute

Make the current task change its image to an executable.

Params:
 - Relative path to the executable file
Returns:
 - On failure: <0
 - On success: N/A because different code will now be running.

## System Info

### CPU Info

Params:
 - Structure of CPU info, to fill in by the kernel.
Returns:
 _nothing_
