[BITS 16]
org 0x7C00

_start:
    ; Начальная инициализация сегментных регистров
    xor ax,ax
    mov es,ax
    mov ds,ax
    
    ; Инициализация стека
    mov ss,ax
    mov ax,0x7C00
    mov sp,ax

    ; Очистка экрана
    mov ax,0x0003
    int 0x10
    
    mov si,msg_load
    call print_str

    ;;;

    mov si,msg_ok
    call print_str

    jmp $ ; бесконечный цикл

%include "print.asm"

msg_load db 'Loading bootloader... ',0
msg_ok db 'OK',0x0D,0x0A,0

times 510-($-$$) db 0
dw 0xAA55
