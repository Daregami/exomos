#include "../drivers/vga.h"
#include <stdint.h> // Стандартные типы данных

extern void idt_init();

int kernel_main(void) {
    // Пишем строку с помощью vga драйвера
    vga_goto(0,2);
    char *msg = "Kernel loaded";    
    kernel_log(msg,VGA_OK);

    idt_init();

    // Разрешаем прерывания
    asm volatile("sti");

    asm volatile("int $0x60");

    // Бесконечный цкил
    while (1) {
        // Вызываем ассемблерную инструкцию hlt
        __asm__ __volatile__("hlt"); 
    }
    return 0;
}
