#ifndef ATA_H
#define ATA_H

#include <stdint.h>

void ata_read_sectors(uint32_t lba, uint8_t count, void *buf);

#endif