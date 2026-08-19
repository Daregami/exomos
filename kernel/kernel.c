#include "../drivers/vga.h"
#include <stdint.h> // Стандартные типы данных
#include "alloc.h"
#include "idt.h"

int kernel_main(void) {
    // Пишем строку с помощью vga драйвера
    vga_goto(0,2);
    char *msg = "Kernel loaded";    
    kernel_log(msg,VGA_OK);

    idt_init();
    pmm_init();

    // Разрешаем прерывания
    asm volatile("sti");

    // Прерывание выводит - Hello world!
    asm volatile("int $60");

    // Бесконечный цкил
    while (1) {
        // Вызываем ассемблерную инструкцию hlt
        __asm__ __volatile__("hlt"); 
    }
    return 0;
}
