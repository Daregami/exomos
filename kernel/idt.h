#ifndef IDT_H
#define IDT_H

// Обёртка для вызова обработчиков прерываний
extern void default_interrupt();
extern void keyboard_interrupt();

// Ремаппинг контроллера прерываний
void pic_remap();

#endif