#include <stdint.h>
#include "gdt.h"

struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0; // стек ядра
    uint32_t ss0; // сегмент стека ядра
    uint32_t unused[22]; // нужно заполнить до 102 байт
    uint16_t trap;
    uint16_t iomap;
} __attribute__((packed));

struct tss_entry tss;

void tss_init(uint32_t kernel_stack) {
    for (uint32_t i = 0; i < sizeof(tss); i++) {
        ((uint8_t*)&tss)[i] = 0;
    }

    tss.ss0 = 0x10;
    tss.esp0 = kernel_stack;
    tss.iomap = sizeof(tss);  // указываем что IO bitmap нет

    // TSS дескриптор: база = адрес структуры, лимит = 103
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss) - 1;

    // access = 0x89: присутствует, DPL=0, тип 32-бит TSS
    gdt_set_entry(5, base, limit, 0x89, 0x00);

    gdt_load_tss();
}