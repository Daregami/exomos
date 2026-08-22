#include "vga.h"
#include <stdint.h>
#include "../kernel/ports.h"
#include "../kernel/pic.h"
#include "keyboard.h"

static char buffer[KBUF_SIZE];
static uint32_t head = 0; // куда пишем
static uint32_t tail = 0; // откуда читаем

// Таблица для преобразования сканкодов в символы
static const char scancode_map[128] = {
    0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b',
    '\t', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',
    0, 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`',
    0, '\\', 'z', 'x', 'c', 'v', 'b', 'n', 'm', ',', '.', '/', 0,
    '*', 0, ' '
};

char scancode_to_char(uint8_t scancode) {
    if (scancode >= 128) return 0;
    return scancode_map[scancode];
}

void kbuf_put(char c) {
    uint32_t next = (head + 1) % KBUF_SIZE;
    if (next == tail) return; // буфер полон, теряем символ
    buffer[head] = c;
    head = next;
}

char kbuf_get() {
    if (head == tail) return 0; // пусто
    char c = buffer[tail];
    tail = (tail + 1) % KBUF_SIZE;
    return c;
}

uint8_t kbuf_empty() {
    return head == tail;
}

void keyboard_handler(uint32_t vector) {
    uint8_t scancode = inb(0x60);
    
    // Игнорируем отпускание (старший бит = 1)
    if (scancode & 0x80) {
        outb(PIC_MASTER_CMD, PIC_EOI);
        return;
    }

    char c = scancode_to_char(scancode);
    if (c) {
        kbuf_put(c);
    }

    outb(PIC_MASTER_CMD, PIC_EOI);  // Завершаем прерывание
}