#include "../drivers/vga.h"
#include <stdint.h>
#include "ports.h"
#include "pic.h"

void keyboard_handler() {
    uint8_t scancode = inb(0x60);

    char *msg = "XYZ";    
    vga_print(msg, BACK_BLACK | COLOR_WHITE);

    outb(PIC_MASTER_CMD, PIC_EOI);  // Завершаем прерывание
}