#include <stdint.h>
#include "../drivers/exomfs.h"
#include "../drivers/vga.h"
#include "alloc.h"
#include "process.h"
#include "gdt.h"

#define PAGE_PRESENT 0x01
#define PAGE_WRITE   0x02
#define PAGE_USER    0x04

#define USER_CODE_ADDR  0x00400000 // виртуальный адрес кода программы
#define USER_STACK_ADDR 0x00800000 // виртуальный адрес стека программы
#define USER_STACK_TOP  (USER_STACK_ADDR + 4096) // стек растет вниз

// Маппим виртуальный адрес на физический в каталоге pd
static void map_page(uint32_t *pd, uint32_t virt, uint32_t phys, uint32_t flags) {
    uint32_t pd_index = virt >> 22;
    uint32_t pt_index = (virt >> 12) & 0x3FF;

    // Если таблицы страниц нет - создаем
    if (!(pd[pd_index] & PAGE_PRESENT)) {
        uint32_t pt_phys = pmm_alloc();
        uint32_t *pt = (uint32_t *)pt_phys;
        for (int i = 0; i < 1024; i++) {
            pt[i] = 0;
        }
        pd[pd_index] = pt_phys | PAGE_PRESENT | PAGE_WRITE | PAGE_USER;
    }

    uint32_t *pt = (uint32_t *)(pd[pd_index] & 0xFFFFF000);
    pt[pt_index] = phys | flags;
}

// Создаем каталог страниц для процесса
static uint32_t *create_user_page_dir() {
    uint32_t pd_phys = pmm_alloc();
    uint32_t *pd = (uint32_t *)pd_phys;

    for (int i = 0; i < 1024; i++) {
        pd[i] = 0;
    }

    // Копируем ядерные маппинги из текущего каталога
    uint32_t *kernel_pd = (uint32_t *)0x10000;

    // Higher-half ядро (0xC0000000+)
    pd[768] = kernel_pd[768];

    return pd;
}

// Прыжок в ring 3
extern void jump_to_usermode(uint32_t eip, uint32_t esp);

void process_exec(const char *name) {
    // Ищем файл
    int idx = exomfs_find(name);
    if (idx < 0) {
        kernel_log("File not found", VGA_ERROR);
        return;
    }

    // Создаем каталог страниц
    uint32_t *pd = create_user_page_dir();

    // Выделяем страницу под код и маппим
    uint32_t code_phys = pmm_alloc();
    map_page(pd, USER_CODE_ADDR, code_phys, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    // Выделяем страницу под стек и маппим
    uint32_t stack_phys = pmm_alloc();
    map_page(pd, USER_STACK_ADDR, stack_phys, PAGE_PRESENT | PAGE_WRITE | PAGE_USER);

    // Читаем файл прямо на физическую страницу кода
    exomfs_read(idx, (void *)code_phys);

    // Переключаем каталог страниц
    asm volatile("mov %0, %%cr3" : : "r"(pd));

    // Прыгаем в ring 3
    jump_to_usermode(USER_CODE_ADDR, USER_STACK_TOP);
}