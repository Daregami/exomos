#include "../drivers/vga.h"
#include <stdint.h> // Стандартные типы данных
#include "alloc.h"
#include "idt.h"
#include "gdt.h"
#include "tss.h"

int kernel_main(void) {
    // Пишем строку с помощью vga драйвера
    vga_goto(0,2);
    char *msg = "Kernel loaded";    
    kernel_log(msg,VGA_OK);

    idt_init();
kernel_log("IDT ok", VGA_OK);

gdt_init();
kernel_log("GDT ok", VGA_OK);

tss_init(0xC0090000);
kernel_log("TSS ok", VGA_OK);
    
    pmm_init();

    // Разрешаем прерывания
    asm volatile("sti");

    // Прерывание выводит - Hello world!
    asm volatile("int $60");

    // Тест сискола из ядра
    asm volatile("int $0x80");

    // Бесконечный цкил
    while (1) {
        // Вызываем ассемблерную инструкцию hlt
        __asm__ __volatile__("hlt"); 
    }
    return 0;
}
