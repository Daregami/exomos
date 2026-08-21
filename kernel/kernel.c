#include "../drivers/vga.h"
#include "../drivers/ata.h"
#include "../drivers/exomfs.h"
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
    gdt_init();
    tss_init(0xC0090000);
    pmm_init();

    // Разрешаем прерывания
    asm volatile("sti");

    // Прерывание выводит - Hello world!
    asm volatile("int $60");

    // Тест сискола из ядра
    asm volatile("int $0x80");

    if (!exomfs_init()) {
        kernel_log("exomfs not found", VGA_ERROR);
    } else {
        kernel_log("exomfs OK", VGA_OK);

        int idx = exomfs_find("hello.txt");
        if (idx >= 0) {
            uint8_t buf[512];
            exomfs_read(idx, buf);
            vga_goto(0, 6);
            vga_print((char *)buf, BACK_BLACK | COLOR_GREEN);
        }
    }

    // Бесконечный цкил
    while (1) {
        // Вызываем ассемблерную инструкцию hlt
        __asm__ __volatile__("hlt"); 
    }
    return 0;
}
