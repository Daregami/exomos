#include "../drivers/vga.h"
#include <stdint.h> // Стандартные типы данных
#include "alloc.h"
#include "idt.h"
#include "gdt.h"
#include "tss.h"
#include "../drivers/ata.h"

int kernel_main(void) {
    // Пишем строку с помощью vga драйвера
    vga_goto(0,2);
    char *msg = "Kernel loaded";    
    kernel_log(msg,VGA_OK);

    idt_init();
    gdt_init();
    tss_init(0xC0090000);
    pmm_init();

    // Разрешаем прерывания
    asm volatile("sti");

    // Прерывание выводит - Hello world!
    asm volatile("int $60");

    // Тест сискола из ядра
    asm volatile("int $0x80");

    uint8_t buf[512];

    ata_read_sectors(0, 1, buf); // читаем первый сектор (MBR)

    vga_goto(0, 5);
    // Первые два байта MBR - это начало загрузчика
    // Последние два байта - сигнатура 0x55 0xAA
    vga_print("ATA: ", BACK_BLACK | COLOR_GREEN);
    vga_print_int(buf[0], BACK_BLACK | COLOR_GREEN);
    vga_print(" ", BACK_BLACK | COLOR_GREEN);
    vga_print_int(buf[1], BACK_BLACK | COLOR_GREEN);
    vga_print(" ... ", BACK_BLACK | COLOR_GREEN);
    vga_print_int(buf[510], BACK_BLACK | COLOR_GREEN);
    vga_print(" ", BACK_BLACK | COLOR_GREEN);
    vga_print_int(buf[511], BACK_BLACK | COLOR_GREEN);

    // Бесконечный цкил
    while (1) {
        // Вызываем ассемблерную инструкцию hlt
        __asm__ __volatile__("hlt"); 
    }
    return 0;
}
