[BITS 32]

extern keyboard_handler
extern default_handler

global keyboard_interrupt
global default_interrupt

default_interrupt:
    pusha
    call default_handler
    popa
    iret

keyboard_interrupt:
    pusha
    call keyboard_handler
    popa
    iret