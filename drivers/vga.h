#ifndef VGA_H
#define VGA_H

#define VGA_ADDR        0xB8000

#define VGA_ERROR       0x00
#define VGA_OK          0x01

#define COLOR_WHITE     0x0F

void kernel_log(char msg, int type);

#endif
