#include <stdint.h>
#include "ata.h"
#include "exomfs.h"

#define EXOMFS_MAGIC 0x45584F4D // EXOM
#define EXOMFS_START 2048 // сектор начала ФС на диске

static struct exomfs_header header;
static struct exomfs_entry entries[16]; // максимум 16 файлов

uint8_t exomfs_init() {
    uint8_t buf[512];
    ata_read_sectors(EXOMFS_START, 1, buf);

    // Проверяем магию
    header = *(struct exomfs_header *)buf;
    if (header.magic != EXOMFS_MAGIC) return 0;

    // Копируем записи файлов из того же сектора
    uint8_t *ptr = buf + sizeof(struct exomfs_header);
    for (uint32_t i = 0; i < header.file_count && i < 16; i++) {
        entries[i] = *(struct exomfs_entry *)(ptr + i * sizeof(struct exomfs_entry));
    }

    return 1;
}

// Ищем файл по имени, возвращаем индекс или -1
int exomfs_find(const char *name) {
    for (uint32_t i = 0; i < header.file_count; i++) {
        // Простое сравнение строк
        const char *a = name;
        const char *b = entries[i].name;
        while (*a && *a == *b) { a++; b++; }
        if (*a == 0 && *b == 0) return i;
    }
    return -1;
}

// Читаем файл в буфер, возвращаем размер
uint32_t exomfs_read(int index, void *buf) {
    if (index < 0 || (uint32_t)index >= header.file_count) return 0;

    // С какого lba начинаем считать
    uint32_t lba = EXOMFS_START + entries[index].offset;
    // Сколько секторов читать
    uint32_t sectors = (entries[index].size + 511) / 512;

    // Читаем порциями по 255 секторов (максимум для LBA28 count=uint8_t)
    uint8_t *ptr = (uint8_t *)buf;
    while (sectors > 0) {
        uint8_t chunk;
        if (sectors > 255) {
            chunk = 255;
        } else {
            chunk = sectors;
        }
        ata_read_sectors(lba, chunk, ptr);
        lba += chunk;
        ptr += chunk * 512;
        sectors -= chunk;
    }

    return entries[index].size;
}