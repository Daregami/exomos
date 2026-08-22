#ifndef KEYBOARD_H
#define KEYBOARD_H

#include <stdint.h>

#define KBUF_SIZE 256

void kbuf_put(char c);
char kbuf_get();     // возвращает 0 если пусто
uint8_t kbuf_empty();

#endif