#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_ADDR            0xC00B8000

#define VGA_ERROR           0x00
#define VGA_OK              0x01

#define COLOR_BLACK         0x00
#define COLOR_BLUE          0x01
#define COLOR_GREEN         0x02
#define COLOR_CYAN          0x03
#define COLOR_RED           0x04
#define COLOR_MAGENTA       0x05
#define COLOR_BROWN         0x06
#define COLOR_LIGHT_GRAY    0x07

#define COLOR_DARK_GRAY     0x08
#define COLOR_LIGHT_BLUE    0x09
#define COLOR_LIGHT_GREEN   0x0A
#define COLOR_LIGHT_CYAN    0x0B
#define COLOR_LIGHT_RED     0x0C
#define COLOR_LIGHT_MAGENTA 0x0D
#define COLOR_YELLOW        0x0E
#define COLOR_WHITE         0x0F

#define BACK_BLACK          0x00
#define BACK_BLUE           0x10
#define BACK_GREEN          0x20
#define BACK_CYAN           0x30
#define BACK_RED            0x40
#define BACK_MAGENTA        0x50
#define BACK_BROWN          0x60
#define BACK_LIGHT_GRAY     0x70

#define BACK_DARK_GRAY      0x80
#define BACK_LIGHT_BLUE     0x90
#define BACK_LIGHT_GREEN    0xA0
#define BACK_LIGHT_CYAN     0xB0
#define BACK_LIGHT_RED      0xC0
#define BACK_LIGHT_MAGENTA  0xD0
#define BACK_YELLOW         0xE0
#define BACK_WHITE          0xF0

#define VGA_WIDTH           80
#define VGA_HEIGHT          25

void vga_goto(uint8_t x, uint8_t y);
void vga_print(char *str, uint8_t color);
void kernel_log(char *msg, uint8_t type);

#endif
