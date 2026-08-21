#ifndef GDT_H
#define GDT_H

#include <stdint.h>

void gdt_init();
void gdt_set_entry(int i, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void tss_init(uint32_t kernel_stack);
extern void gdt_flush(uint32_t gdtp_addr);
extern void gdt_load_tss();

#endif