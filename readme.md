# Fors Kernel

## TODO

 - ~~Virtual file system interface~~
 - Buffer cache
    - ~~ATA Hard Disk Controller~~
    - EXT2 Filesystem
 - PCI Controller
    - Then, network interface card
    - PCI storage devices
 - Loading programs from disk

## Features

Since it doesn't do what I want yet, the following are what I consider might be nice features to implement once the base is there.

### Inter-process communication

To share messages between separate processes, fors uses a topic based publish/subscribe model in addition to more standard process-process signals. 

#### Pub/sub

Topic names follow a hierarchical naming scheme. A process can publish to a number of topics, and can simultaneously subscribe to some as well.

Below is a list of some proposed use cases of this pub/sub IPC mechanism. It is of course not comprehensive.

 - Each process could publish a topic for its status updates. For example, a process with id `proc0` could publish a topic called `pstatus/proc0`. If this process encounters any errors during running, or wishes to provide any extra status information, it could publish that to this topic. This way, any other process that wants to can keep track of the status of any other set of processes.

 - The kernel could publish a topic for keyboard keypresses, for example called `device/kbd0/key`. Then, any process which wants direct and immediate access to keypresses could set up a subscriber to this topic. This would be useful for realtime keypress handling; most command line applications would read line-by-line input.

 - A printer could subscribe to a topic like `device/printer0/print_queue`, which a number of processes could publish to. This is an easy way of allowing sharing of printing resources.

---

 - **Decoupled processes**: Unlike a traditional signal interface, processes do not need to know each others IDs to communicate with each other. A subscriber doesn't need to care about which specific processes are publishing to a topic, and a publisher doesn't need to care about which processes are subscribing to a topic that it's publishing to (however, there is access control in place so that sensitive information can be safely published).
 - **Transparent interfaces to other pub/sub systems**: There are several widely used pub/sub messaging systems, for instance MQTT and ROS. These could be implemented as layers on top of the fors pub/sub system (in a similar manner to other filesystems mounted at certain points in a Linux virtual filesystem). This could make these other systems simpler to integrate with applications.

 - **Granular events**: An arbitrary amount of topics can exist, which means there can be an arbitrary amount of different events that a process can respond to in a similar fashion to responding to traditional UNIX signals. For instance, there could be a topic for system wide events relating to device status, which could contain messages such as "New USB drive connected", or "Mouse disconnected", etc. This would be a kernel-published topic. A userspace published topic could be for general user-facing notifications. All sorts of applications could publish to this topic, for example: email and messaging clients, to notify users when they receive a communication; compilers, renderers, and other long-processing-time applications, to notify users when a job is done; and power/battery managers and other system-type jobs, to notify users if they're on low battery, or if perhaps a disk failure was detected.
