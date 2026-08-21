#include <stdint.h>
#include "../kernel/ports.h"
#include "ata.h"

#define ATA_DATA       0x1F0
#define ATA_ERROR      0x1F1
#define ATA_SECT_COUNT 0x1F2
#define ATA_LBA_LO     0x1F3
#define ATA_LBA_MID    0x1F4
#define ATA_LBA_HI     0x1F5
#define ATA_DRIVE      0x1F6
#define ATA_STATUS     0x1F7
#define ATA_COMMAND    0x1F7

#define ATA_STATUS_BSY  0x80
#define ATA_STATUS_DRQ  0x08
#define ATA_CMD_READ    0x20

// Ждем пока диск освободится
static void ata_wait() {
    while (inb(ATA_STATUS) & ATA_STATUS_BSY);
}

// Ждем пока данные будут готовы
static void ata_wait_drq() {
    while (!(inb(ATA_STATUS) & ATA_STATUS_DRQ));
}

// Читаем count секторов начиная с lba в буфер buf
void ata_read_sectors(uint32_t lba, uint8_t count, void *buf) {
    ata_wait();

    // LBA28 старшие 4 бита в регистре drive, остальные 24 в трех регистрах
    // 0 - master/ 1 - slave
    // 1 - всегда 1
    // 1 - режим LBA
    // 1 - всегда 1
    outb(ATA_DRIVE, 0b11100000 | ((lba >> 24) & 0x0F)); // master, LBA mode
    outb(ATA_SECT_COUNT, count);
    outb(ATA_LBA_LO, lba & 0xFF);
    outb(ATA_LBA_MID, (lba >> 8) & 0xFF);
    outb(ATA_LBA_HI, (lba >> 16) & 0xFF);
    outb(ATA_COMMAND, ATA_CMD_READ);

    uint16_t *ptr = (uint16_t *)buf;

    for (uint8_t i = 0; i < count; i++) {
        ata_wait_drq();

        // Один сектор = 256 слов (512 байт)
        for (int j = 0; j < 256; j++) {
            ptr[j] = inw(ATA_DATA);
        }

        ptr += 256; // сдвигаем указатель на следующий сектор
    }
}