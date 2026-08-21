#include "vga.h"
#include <stdint.h>
#include "../kernel/ports.h"
#include "../kernel/pic.h"

void keyboard_handler(uint32_t vector) {
    uint8_t scancode = inb(0x60);

    char *msg = "XYZ";    
    vga_print(msg, BACK_BLACK | COLOR_WHITE);

    outb(PIC_MASTER_CMD, PIC_EOI);  // Завершаем прерывание
}