#ifndef PIC_H
#define PIC_H

// Порты PIC
#define PIC_MASTER_CMD      0x20 // Порт для приема команд
#define PIC_MASTER_DATA     0x21 // Порт для приема данных
#define PIC_SLAVE_CMD       0xA0
#define PIC_SLAVE_DATA      0xA1

// Команды инициализации
#define ICW1_INIT           0x11 // Инициализация с возможностью каскадирования
#define ICW4_8086           0x01 // Режим x86

// Базовые векторы после ремаппинга
#define ICW2_PIC_MASTER_OFFSET   32  // Меняем IRQ 0-7 в векторы 32-39
#define ICW2_PIC_SLAVE_OFFSET    40  // Меняем IRQ 8-15 в векторы 40-47

// Каскадирование
#define ICW3_MASTER         0b00000100   // Slave подключен на вход IR2
                                        // будет передавать на шину данных номер пина всем Slave
#define ICW3_SLAVE          0x02  // Каскадный номер Slave 2, он будет на него реагировать данными по шине

// Маски
#define PIC_MASK_ALL        0xFF  // Все прерывания замаскированы
#define PIC_MASK_KEYBOARD   0b11111101 // Только IRQ1 разрешён

// EOI
#define PIC_EOI             0x20 // Командное слово завершения прерывания

void pic_remap();

#endif