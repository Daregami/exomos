#include "pic.h"
#include "ports.h"

void pic_remap() {
    // Настройка Master PIC
    outb(PIC_MASTER_CMD,  ICW1_INIT);
    outb(PIC_MASTER_DATA, ICW2_PIC_MASTER_OFFSET);
    outb(PIC_MASTER_DATA, ICW3_MASTER);
    outb(PIC_MASTER_DATA, ICW4_8086);

    // Настройка Slave PIC
    outb(PIC_SLAVE_CMD,  ICW1_INIT);
    outb(PIC_SLAVE_DATA, ICW2_PIC_SLAVE_OFFSET);
    outb(PIC_SLAVE_DATA, ICW3_SLAVE);
    outb(PIC_SLAVE_DATA, ICW4_8086);

    // Маски
    outb(PIC_MASTER_DATA, PIC_MASK_KEYBOARD);
    outb(PIC_SLAVE_DATA,  PIC_MASK_ALL);
}