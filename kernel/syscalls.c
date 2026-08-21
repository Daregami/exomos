#include <stdint.h>
#include "../drivers/vga.h"

void syscall_handler(uint32_t vector) {
    vga_goto(0, 4);
    vga_print("Syscall works!", BACK_BLACK | COLOR_GREEN);
}