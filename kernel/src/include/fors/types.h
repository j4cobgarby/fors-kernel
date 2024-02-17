#ifndef INCLUDE_FORS_TYPES_H_
#define INCLUDE_FORS_TYPES_H_

typedef long dev_id_t;
typedef long fsn_id_t;
typedef long uid_t;
typedef long gid_t;
typedef long pid_t;
typedef long timestamp_t;

typedef unsigned fsn_perm_t;
#define FP_RUSR (1 << 0)
#define FP_WUSR (1 << 1)
#define FP_XUSR (1 << 2)
#define FP_RGRP (1 << 3)
#define FP_WGRP (1 << 4)
#define FP_XGRP (1 << 5)
#define FP_ROTH (1 << 6)
#define FP_WOTH (1 << 7)
#define FP_XOTH (1 << 8)

typedef unsigned of_mode_t;
#define OF_WRITE (1 << 0)
#define OF_READ  (1 << 1)

#endif // INCLUDE_FORS_TYPES_H_