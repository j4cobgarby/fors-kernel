#ifndef __INCLUDE_ARCH_X64_TIMER_825X_H__
#define __INCLUDE_ARCH_X64_TIMER_825X_H__

#define PIT825X_DATA_0  0x40
#define PIT825X_DATA_1  0x41
#define PIT825X_DATA_2  0x42
#define PIT825X_CMD     0x43

#define PIT825X_CHN_0   (0x00 << 6)
#define PIT825X_CHN_1   (0x01 << 6)
#define PIT825X_CHN_2   (0x02 << 6)
#define PIT825X_RDBACK  (0x03 << 6)

#endif /* __INCLUDE_ARCH_X64_TIMER_825X_H__ */