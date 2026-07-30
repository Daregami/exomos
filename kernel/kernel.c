#include "../drivers/vga.h"
#include <stdint.h> // Стандартные типы данных

int kernel_main(void) {
    // Пишем строку с помощью vga драйвера
    vga_goto(0,2);
    char *msg = "Kernel started!";    
    kernel_log(msg,VGA_ERROR);

    // Бесконечный цкил
    while (1) {
        // Вызываем ассемблерную инструкцию hlt
        __asm__ __volatile__("hlt"); 
    }
    return 0;
}
