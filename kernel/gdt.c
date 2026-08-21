#include <stdint.h>
#include "gdt.h"

struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_mid;
    uint8_t access;
    uint8_t flags_limit;
    uint8_t base_high;
} __attribute__((packed));

struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

struct gdt_entry gdt[6]; // 0-нулевой, 1-код ядра, 2-данные ядра,
                         // 3-код юзера, 4-данные юзера, 5-TSS

struct gdt_ptr gdtp;

void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[i].limit_low = limit & 0xFFFF;
    gdt[i].base_low = base & 0xFFFF;
    gdt[i].base_mid = (base >> 16) & 0xFF;
    gdt[i].access = access;
    gdt[i].flags_limit = (flags << 4) | ((limit >> 16) & 0x0F);
    gdt[i].base_high = (base >> 24) & 0xFF;
}

void gdt_init() {
    gdt_set_entry(0, 0, 0, 0, 0);              // нулевой
    gdt_set_entry(1, 0, 0xFFFFF, 0x9A, 0x0C);  // код ядра 0x08
    gdt_set_entry(2, 0, 0xFFFFF, 0x92, 0x0C);  // данные ядра 0x10
    gdt_set_entry(3, 0, 0xFFFFF, 0xFA, 0x0C);  // код юзера 0x18
    gdt_set_entry(4, 0, 0xFFFFF, 0xF2, 0x0C);  // данные юзера 0x20
    // gdt[5] заполнит tss_init()

    gdtp.limit = sizeof(gdt) - 1;
    gdtp.base  = (uint32_t)&gdt;

    gdt_flush((uint32_t)&gdtp);
}