#include "vga.h"
#include <stdint.h>
#include "../kernel/ports.h"

volatile char * const video_buf = (volatile char*)VGA_ADDR;
static uint8_t cursor_x = 0;
static uint8_t cursor_y = 0;

// Обновляем аппаратный курсор
static void vga_update_cursor() {
    uint16_t pos = cursor_y * VGA_WIDTH + cursor_x;
    outb(0x3D4, 14); // старший байт позиции
    outb(0x3D5, pos >> 8);
    outb(0x3D4, 15); // младший байт позиции
    outb(0x3D5, pos & 0xFF);
}

// Сдвигаем весь экран на одну строку вверх
static void vga_scroll() {
    // Копируем строки 1-24 на место 0-23
    for (int i = 0; i < (VGA_HEIGHT - 1) * VGA_WIDTH; i++) {
        video_buf[i * 2] = video_buf[(i + VGA_WIDTH) * 2];
        video_buf[i * 2 + 1] = video_buf[(i + VGA_WIDTH) * 2 + 1];
    }

    // Очищаем последнюю строку
    for (int i = 0; i < VGA_WIDTH; i++) {
        int pos = ((VGA_HEIGHT - 1) * VGA_WIDTH + i) * 2;
        video_buf[pos] = ' ';
        video_buf[pos + 1] = BACK_BLACK | COLOR_WHITE;
    }

    cursor_y = VGA_HEIGHT - 1;
}

void vga_goto(uint8_t x, uint8_t y) {
    cursor_x = x;
    cursor_y = y;
    vga_update_cursor();
}

void vga_putchar(char c, uint8_t color) {
    // Перенос строки
    if (c == '\n') {
        cursor_x = 0;
        cursor_y++;
        if (cursor_y >= VGA_HEIGHT) vga_scroll();
        vga_update_cursor();
        return;
    }

    // Backspace
    if (c == '\b') {
        if (cursor_x > 0) {
            cursor_x--;
            video_buf[(cursor_x + VGA_WIDTH * cursor_y) * 2] = ' ';
            video_buf[(cursor_x + VGA_WIDTH * cursor_y) * 2 + 1] = color;
        }
        vga_update_cursor();
        return;
    }

    video_buf[(cursor_x + VGA_WIDTH * cursor_y) * 2] = c;
    video_buf[(cursor_x + VGA_WIDTH * cursor_y) * 2 + 1] = color;

    cursor_x++;
    if (cursor_x >= VGA_WIDTH) {
        cursor_x = 0;
        cursor_y++;
    }

    // Очищаем позицию под курсором чтобы цвет был правильный
    video_buf[(cursor_x + VGA_WIDTH * cursor_y) * 2] = ' ';
    video_buf[(cursor_x + VGA_WIDTH * cursor_y) * 2 + 1] = BACK_BLACK | COLOR_WHITE;

    if (cursor_y >= VGA_HEIGHT) vga_scroll();
    vga_update_cursor();
}

void vga_print(char *str, uint8_t color) {
    while (*str != '\0') {
        vga_putchar(*str, color);
        str++;
    }
}

void vga_print_int(uint32_t num, uint8_t color) {
    char buf[10]; // Максимальное число int - 4294967295, всего 10 цифр
    int i = 0;

    if (num == 0) {
        vga_putchar('0', color);
        return;
    }

    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }

    while (--i >= 0) {
        vga_putchar(buf[i], color);
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
    vga_print("\n", white);
}
