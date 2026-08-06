#include "../drivers/vga.h"
#include <stdint.h>
#include "ports.h"
#include "idt.h"
#include "pic.h"


struct idt_entry {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t  zero;
    uint8_t  type_attr;
    uint16_t offset_high;
} __attribute__((packed)); // Не потимизировать с выравниванием

struct idt_ptr {
    uint16_t limit; // Размер таблицы обработчиков прерываний
    uint32_t base; // Начало таблицы
} __attribute__((packed));

struct idt_entry idt[256];
struct idt_ptr   idtp;

void idt_set_entry(int index, uint32_t handler) {
    idt[index].offset_low = handler & 0xFFFF; // Младшая часть адреса хендлера
    idt[index].selector = 0x08; // Может быть выполнен только в Ring0
    idt[index].zero = 0; // Зарезервированно
    idt[index].type_attr = 0b10001110;  // бит 7 = 1 запись активна
                                        // биты 6-5 = DPL 00 - можно вызывать в Ring0
                                        // бит 4 = всегда ноль для interrupt
                                        // биты 3-0 = 1110 32 битный режим прерываний
                                        // процессор при выходе сбросит IF
    idt[index].offset_high = (handler >> 16) & 0xFFFF; // Старшая часть адреса хендлера
}

void default_handler() {
    vga_goto(0, 3);
    kernel_log("Unhandled interrupt", VGA_ERROR);
    outb(PIC_MASTER_CMD, PIC_EOI);
}

void idt_init() {
    pic_remap();

    // Заполняем структуру idt
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;   

    // Заполняем с 32 вектора дефотным прерыванием
    for (volatile int i = 32; i < 256; i++) {
        idt_set_entry(i, (uint32_t)default_interrupt);
    }

    // Вектор 33 клавиатура (IRQ1, база 32 + 1)
    idt_set_entry(33, (uint32_t)keyboard_interrupt);
    // Загружаем IDT
    asm volatile("lidt %0" : : "m"(idtp));
}