#include "../drivers/vga.h"
#include "syscalls.h"
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

void idt_set_entry_user(int index, uint32_t handler) {
    idt[index].offset_low = handler & 0xFFFF;
    idt[index].selector = 0x08; // обработчик все равно в ring 0
    idt[index].zero = 0;
    idt[index].type_attr = 0b11101110;  // бит 7 = 1 запись активна
                                        // биты 6-5 = DPL 11 - ring 3 может вызывать
                                        // бит 4 = 0
                                        // биты 3-0 = 1110 - 32-бит interrupt gate
    idt[index].offset_high = (handler >> 16) & 0xFFFF;
}

void default_handler(uint32_t vector) {
    vga_goto(0,3);
    kernel_log("Unhandled interrupt ", VGA_ERROR);
    vga_print_int(vector, BACK_BLACK | COLOR_GREEN);
    outb(PIC_MASTER_CMD, PIC_EOI);
}

void hello_world(uint32_t vector) {
    vga_goto(0,3);
    vga_print("Hello, world!", BACK_BLACK | COLOR_GREEN);
    outb(PIC_MASTER_CMD, PIC_EOI);
}

// Номера прерываний
extern uint32_t isr_stub_table[256];
// Таблица оброботчиков прерывания
void (*handler_table[256])(uint32_t) = {0};

void register_handler(uint32_t vector, void (*handler)(uint32_t)) {
    handler_table[vector] = handler;
}

extern void keyboard_handler(uint32_t vector);
extern void syscall_stub(); // Для сисколов свой стаб

void idt_init() {
    pic_remap();

    // Заполняем структуру idt
    idtp.limit = sizeof(idt) - 1;
    idtp.base  = (uint32_t)&idt;   

    // Заполняем вектора дефотным прерыванием
    for (volatile uint32_t i = 0; i < 256; i++) {
        idt_set_entry(i, isr_stub_table[i]); // записываем адреса стабов
        register_handler(i, default_handler);
    }

    // Вектор 0x80 - системный вызов, DPL=3 чтобы ring 3 мог вызывать
    idt_set_entry_user(0x80, (uint32_t)syscall_stub);

    // Вектор 33 клавиатура (IRQ1, база 32 + 1)
    register_handler(33, keyboard_handler);

    register_handler(60, hello_world);

    // Загружаем IDT
    asm volatile("lidt %0" : : "m"(idtp));
}