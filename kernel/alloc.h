#ifndef ALLOC_H
#define ALLOC_H

#define PAGE_SIZE       4096 // Размер одной страницы - 4кб
#define TOTAL_MEMORY    0x8000000   // в qemu выделяем 128 МБ ОЗУ
#define TOTAL_PAGES     (TOTAL_MEMORY / PAGE_SIZE) // Сколько страниц получим из всей памяти
#define BITMAP_SIZE     (TOTAL_PAGES / 32) // Массив

void pmm_set(uint32_t page);
void pmm_clear(uint32_t page);
uint8_t pmm_test(uint32_t page);
uint32_t pmm_alloc();
void pmm_free(uint32_t addr);
void pmm_init();

extern uint32_t _kernel_end;

#endif