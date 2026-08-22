#include <stdint.h>
#include "../drivers/vga.h"
#include "../drivers/keyboard.h"

struct regs {
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
};

void syscall_dispatch(struct regs *r) {
    switch (r->eax) {
    case 0: // exit
        vga_goto(0, 8);
        vga_print("Process exited", BACK_BLACK | COLOR_WHITE);
        while (1) { asm volatile("hlt"); }
        break;
    case 1: // write
        // r->ebx = fd, r->ecx = buf, r->edx = len
        if (r->ebx == 1) { // stdout
            vga_print((char *)r->ecx, BACK_BLACK | COLOR_WHITE);
        }
        break;
    case 2: // read
        if (r->ebx == 0) { // stdin
            char *buf = (char *)r->ecx;
            // Ждем пока не появится символ
            while (kbuf_empty()) {
                asm volatile("sti; hlt");
            }
            buf[0] = kbuf_get();
            r->eax = 1; // прочитали 1 байт
        }
        break;
    }
}