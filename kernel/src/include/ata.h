#ifndef ATA_H
#define ATA_H

#include <stdint.h>

// ATA-PIO port definitions for primary and secondary controllers
#define ATA_PRIMARY_DATA        0x1F0
#define ATA_PRIMARY_ERROR       0x1F1
#define ATA_PRIMARY_FEATURES    0x1F1
#define ATA_PRIMARY_SECCOUNT    0x1F2
#define ATA_PRIMARY_LBA_LOW     0x1F3
#define ATA_PRIMARY_LBA_MID     0x1F4
#define ATA_PRIMARY_LBA_HIGH    0x1F5
#define ATA_PRIMARY_DRIVE       0x1F6
#define ATA_PRIMARY_STATUS      0x1F7
#define ATA_PRIMARY_COMMAND     0x1F7
#define ATA_PRIMARY_CTRL        0x3F6

#define ATA_SECONDARY_DATA      0x170
#define ATA_SECONDARY_ERROR     0x171
#define ATA_SECONDARY_FEATURES  0x171
#define ATA_SECONDARY_SECCOUNT  0x172
#define ATA_SECONDARY_LBA_LOW   0x173
#define ATA_SECONDARY_LBA_MID   0x174
#define ATA_SECONDARY_LBA_HIGH  0x175
#define ATA_SECONDARY_DRIVE     0x176
#define ATA_SECONDARY_STATUS    0x177
#define ATA_SECONDARY_COMMAND   0x177
#define ATA_SECONDARY_CTRL      0x376

// ATA status register bits
#define ATA_STATUS_BSY   0x80  // Busy
#define ATA_STATUS_DRDY  0x40  // Drive ready
#define ATA_STATUS_DRQ   0x08  // Data request ready
#define ATA_STATUS_ERR   0x01  // Error

// ATA commands
#define ATA_CMD_READ_SECTORS    0x20
#define ATA_CMD_WRITE_SECTORS   0x30
#define ATA_CMD_IDENTIFY        0xEC

// Drive selection bits
#define ATA_DRIVE_MASTER        0xA0
#define ATA_DRIVE_SLAVE         0xB0
#define ATA_DRIVE_LBA           0x40

// Error codes
#define ATA_OK                  0
#define ATA_ERR_TIMEOUT        -1
#define ATA_ERR_DRIVE_ERROR    -2
#define ATA_ERR_INVALID_DRIVE  -3

typedef struct {
    uint16_t base_port;
    uint16_t ctrl_port;
    uint8_t drive;  // 0 for master, 1 for slave
} ata_device_t;

// Initialize ATA device structure
void ata_init_device(ata_device_t *dev, bool primary, bool master);

// Wait for drive to be ready (not busy);
int ata_wait_ready(ata_device_t *dev);

// Wait for data request
int ata_wait_drq(ata_device_t *dev);

// Select drive and set up LBA addressing
int ata_select_drive(ata_device_t *dev, uint32_t lba);

// Read a single 512-byte sector
int ata_read_sector(ata_device_t *dev, uint32_t lba, void *buffer);

// Write a single 512-byte sector
int ata_write_sector(ata_device_t *dev, uint32_t lba, const void *buffer);

// Read multiple sectors
int ata_read_sectors(ata_device_t *dev, uint32_t start_lba, uint32_t count, void *buffer);

// Write multiple sectors
int ata_write_sectors(ata_device_t *dev, uint32_t start_lba, uint32_t count, const void *buffer);

// Simple drive identification (optional);
int ata_identify(ata_device_t *dev, uint16_t *identify_data);

#endif // ATA_H
