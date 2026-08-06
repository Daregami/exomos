#include "vga.h"
#include <stdint.h>

volatile char * const video_buf = (volatile char*)VGA_ADDR;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

void vga_goto(uint8_t x, uint8_t y) {
    cursor_x = x;
    cursor_y = y;
}

void vga_print(char *str, uint8_t color) {

    while (*str != '\0') {
        // Экран переполнен, пока скролла нет выходим
        if (cursor_y >= 25) {
            break;
        }
        
        // Печатаем символ в текущих координатах курсора
        video_buf[(cursor_x + VGA_WIDTH * cursor_y) * 2] = *str;
        video_buf[(cursor_x + VGA_WIDTH * cursor_y) * 2 + 1] = color;

        cursor_x++; // Смещаем курсор по строке
        
        //Дошли до конца строки
        if (cursor_x >= VGA_WIDTH) {
            cursor_x = 0;
            cursor_y += 1;
        }
        str++;
    }
}


void kernel_log(char *msg, uint8_t type) {
    uint8_t green = BACK_BLACK | COLOR_GREEN;
    uint8_t red = BACK_BLACK | COLOR_RED;
    uint8_t white = BACK_BLACK | COLOR_WHITE;

    vga_print("[ ", white);
    switch (type) {
        case VGA_OK:
            vga_print("OK", green);
            break;
        case VGA_ERROR:
            vga_print("ERROR", red);
            break;
    }
    vga_print(" ] ", white);
    vga_print(msg, white);
}
