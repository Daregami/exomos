#include "vga.h"
#include <stdint.h>

volatile char * const video_buf = (volatile char*)VGA_ADDR;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

void vga_goto(uint8_t x, uint8_t y) {
    cursor_x = x;
    cursor_y = y;
}

void vga_print(char *str) {

    while (*str != '\0') {
        // Экран переполнен, пока скролла нет выходим
        if (cursor_y >= 25) {
            break;
        }
        
        // Печатаем символ в текущих координатах курсора
        video_buf[(cursor_x + 80 * cursor_y) * 2] = *str;
        video_buf[(cursor_x + 80 * cursor_y) * 2 + 1] = COLOR_WHITE;

        cursor_x++; // Смещаем курсор по строке
        
        //Дошли до конца строки
        if (cursor_x >= 80 ) {
            cursor_x = 0;
            cursor_y += 1;
        }
        str++;
    }
}


void kernel_log(char *msg, uint8_t type) {
    switch (type) {
        case VGA_OK:
            vga_print(msg);
            break;
        case VGA_ERROR:
            vga_print(msg);
            break;
    }
}
