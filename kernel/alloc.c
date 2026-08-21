#include <stdint.h>
#include "alloc.h"

static uint32_t bitmap[BITMAP_SIZE];

void pmm_set(uint32_t page) {
    // Находи нужный индекс из битмапа, где находится нужная страница
    // Резерврируем в нужном смещении бит 1
    bitmap[page / 32] |= (1 << (page % 32));
}

void pmm_clear(uint32_t page) {
    // Применяем побитовую маску с инверсией, чтобы освободить страницу
    bitmap[page / 32] &= ~(1 << (page % 32));
}

uint8_t pmm_test(uint32_t page) {
    // Сдвигом проверяем, не занята ли страница
    return (bitmap[page / 32] >> (page % 32)) & 1;
}

uint32_t pmm_alloc() {
    // Уже перебираем все стнарицы от начала, пока не найдем свободную
    for (uint32_t i = 0; i < TOTAL_PAGES; i++) {
        // Если 0, значит свободна, можно использовать
        if (!pmm_test(i)) {
            pmm_set(i); // Устанавливаем занятость страницы
            return i * PAGE_SIZE; // Физический адрес полученной страницы
        }
    }
    return 0; // Нет свободных страниц
}

void pmm_free(uint32_t addr) {
    uint32_t page = addr / PAGE_SIZE; // Обратная операция из адресв в битмап страницы
    pmm_clear(page); // Освобождаем страницу
}

void pmm_init() {
    // Помечаем все как занятое
    for (volatile uint32_t i = 0; i < BITMAP_SIZE; i++) {
        bitmap[i] = 0xFFFFFFFF;
    }

    /// Вычисляем конец ядра в физических страницах
    // _kernel_end - виртуальный адрес
    // Вычитаем 0xC0000000, получаем физический
    // Делим на 4096, получаем номер страницы
    uint32_t kernel_end_phys = (uint32_t)&_kernel_end - 0xC0000000;
    uint32_t first_free = kernel_end_phys / PAGE_SIZE;

    for (uint32_t i = first_free; i < TOTAL_PAGES; i++) {
        pmm_clear(i);
    }
}