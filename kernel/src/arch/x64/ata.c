#include "fors/ata.h"
#include "arch/x64/io.h"
#include "arch/x64/pic.h"
#include "fors/printk.h"

static int st_rd(void *dev, size_t addr, char *buf)
{
    return ATA_OK == ata_read_sector((ata_device_t *)dev, addr, buf);
}

static int st_wr(void *dev, size_t addr, const char *buf)
{
    return ATA_OK == ata_write_sector((ata_device_t *)dev, addr, buf);
}

static size_t st_nb(void *dev)
{
    return ata_num_blocks((ata_device_t *)dev);
}

static int st_init(void *dev, const char *cfg)
{
    // int primary = va_arg(args, int);
    // int master = va_arg(args, int);
    int primary = cfg[0] == 'p';
    int master = cfg[1] == 'm';
    printk("Initialising device with primary=%d, master=%d\n", primary, master);
    ata_init_device((ata_device_t *)dev, primary, master);
    return 1;
}

store_type_t ata_store_type = {
    .name = "atapio",
    .block_sz = 512,
    .init = &st_init,
    .rd = &st_rd,
    .wr = &st_wr,
    .nblocks = &st_nb,
};

// Initialize ATA device structure
void ata_init_device(ata_device_t *dev, bool primary, bool master)
{
    if (primary) {
        dev->base_port = ATA_PRIMARY_DATA;
        dev->ctrl_port = ATA_PRIMARY_CTRL;
    } else {
        dev->base_port = ATA_SECONDARY_DATA;
        dev->ctrl_port = ATA_SECONDARY_CTRL;
    }
    dev->drive = master ? 0 : 1;

    // Ensure that the PIC IRQ is open for the corresponding ATA bus
    pic_unblock_irq(master ? PIC_IRQ_ATABUS_MASTER : PIC_IRQ_ATABUS_SLAVE);
}

// Wait for drive to be ready (not busy)
int ata_wait_ready(ata_device_t *dev)
{
    uint16_t status_port = dev->base_port + 7; // Status register
    int timeout = 1000000;                     // Adjust timeout as needed

    while (timeout-- > 0) {
        uint8_t status = inb(status_port);
        if (!(status & ATA_STATUS_BSY)) {
            return ATA_OK;
        }
    }
    return ATA_ERR_TIMEOUT;
}

// Wait for data request
int ata_wait_drq(ata_device_t *dev)
{
    uint16_t status_port = dev->base_port + 7;
    int timeout = 1000000;

    while (timeout-- > 0) {
        uint8_t status = inb(status_port);
        if (status & ATA_STATUS_DRQ) {
            return ATA_OK;
        }
        if (status & ATA_STATUS_ERR) {
            return ATA_ERR_DRIVE_ERROR;
        }
    }
    return ATA_ERR_TIMEOUT;
}

// Select drive and set up LBA addressing
int ata_select_drive(ata_device_t *dev, size_t lba)
{
    uint8_t drive_select
        = (dev->drive == 0 ? ATA_DRIVE_MASTER : ATA_DRIVE_SLAVE) | ATA_DRIVE_LBA
        | ((lba >> 24) & 0x0F);

    outb(dev->base_port + 6, drive_select); // Drive/Head register

    // Small delay after drive selection
    for (int i = 0; i < 4; i++) {
        inb(dev->ctrl_port); // Delay by reading alternate status
    }

    return ata_wait_ready(dev);
}

// Read a single 512-byte sector
int ata_read_sector(ata_device_t *dev, size_t lba, char *buffer)
{
    uint16_t *buf = (uint16_t *)buffer;
    int ret;

    // Wait for drive to be ready
    ret = ata_wait_ready(dev);
    if (ret != ATA_OK) return ret;

    // Select drive and set LBA
    ret = ata_select_drive(dev, lba);
    if (ret != ATA_OK) return ret;

    // Set up the command
    outb(dev->base_port + 1, 0x00);               // Features
    outb(dev->base_port + 2, 1);                  // Sector count (1 sector)
    outb(dev->base_port + 3, lba & 0xFF);         // LBA low
    outb(dev->base_port + 4, (lba >> 8) & 0xFF);  // LBA mid
    outb(dev->base_port + 5, (lba >> 16) & 0xFF); // LBA high

    // Send read command
    outb(dev->base_port + 7, ATA_CMD_READ_SECTORS);

    // Wait for data to be ready
    ret = ata_wait_drq(dev);
    if (ret != ATA_OK) return ret;

    // Read 256 words (512 bytes)
    for (int i = 0; i < 256; i++) {
        buf[i] = inw(dev->base_port);
    }

    return ATA_OK;
}

// Write a single 512-byte sector
int ata_write_sector(ata_device_t *dev, size_t lba, const char *buffer)
{
    const uint16_t *buf = (const uint16_t *)buffer;
    int ret;

    // Wait for drive to be ready
    ret = ata_wait_ready(dev);
    if (ret != ATA_OK) return ret;

    // Select drive and set LBA
    ret = ata_select_drive(dev, lba);
    if (ret != ATA_OK) return ret;

    // Set up the command
    outb(dev->base_port + 1, 0x00);               // Features
    outb(dev->base_port + 2, 1);                  // Sector count (1 sector)
    outb(dev->base_port + 3, lba & 0xFF);         // LBA low
    outb(dev->base_port + 4, (lba >> 8) & 0xFF);  // LBA mid
    outb(dev->base_port + 5, (lba >> 16) & 0xFF); // LBA high

    // Send write command
    outb(dev->base_port + 7, ATA_CMD_WRITE_SECTORS);

    // Wait for data request
    ret = ata_wait_drq(dev);
    if (ret != ATA_OK) return ret;

    // Write 256 words (512 bytes)
    for (int i = 0; i < 256; i++) {
        outw(dev->base_port, buf[i]);
    }

    // Wait for completion
    ret = ata_wait_ready(dev);
    if (ret != ATA_OK) return ret;

    // Check for errors
    uint8_t status = inb(dev->base_port + 7);
    if (status & ATA_STATUS_ERR) {
        return ATA_ERR_DRIVE_ERROR;
    }

    return ATA_OK;
}

// Read multiple sectors
int ata_read_sectors(
    ata_device_t *dev, size_t start_lba, size_t count, char *buffer)
{
    for (size_t i = 0; i < count; i++) {
        int ret = ata_read_sector(dev, start_lba + i, buffer + (i * 512));
        if (ret != ATA_OK) {
            return ret;
        }
    }

    return ATA_OK;
}

// Write multiple sectors
int ata_write_sectors(
    ata_device_t *dev, size_t start_lba, size_t count, const char *buffer)
{
    for (size_t i = 0; i < count; i++) {
        int ret = ata_write_sector(dev, start_lba + i, buffer + (i * 512));
        if (ret != ATA_OK) {
            return ret;
        }
    }

    return ATA_OK;
}

int ata_identify(ata_device_t *dev, uint16_t *identify_data)
{
    int ret;

    ret = ata_wait_ready(dev);
    if (ret != ATA_OK) return ret;

    ret = ata_select_drive(dev, 0);
    if (ret != ATA_OK) return ret;

    outb(dev->base_port + 7, ATA_CMD_IDENTIFY);

    // Check if drive exists
    uint8_t status = inb(dev->base_port + 7);
    if (status == 0) {
        return ATA_ERR_INVALID_DRIVE; // No drive
    }

    ret = ata_wait_drq(dev);
    if (ret != ATA_OK) return ret;

    for (int i = 0; i < 256; i++) {
        identify_data[i] = inw(dev->base_port);
    }

    return ATA_OK;
}

size_t ata_num_blocks(ata_device_t *dev)
{
    return 0; // TODO
}
