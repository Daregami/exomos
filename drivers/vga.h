#ifndef VGA_H
#define VGA_H

#include <stdint.h>

#define VGA_ADDR        0xB8000

#define VGA_ERROR       0x00
#define VGA_OK          0x01

#define COLOR_WHITE     0x0F

void vga_goto(uint8_t x, uint8_t y);
void kernel_log(char *msg, uint8_t type);

#endif
