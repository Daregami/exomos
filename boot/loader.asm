[BITS 16]
org 0x7E00

_load_start:
    mov si,msg_kload
    call print_str

copy_kernel:
    mov si,msg_ok
    call print_str

stop_system:
    cli
    hlt
    jmp stop_system

%include "print.asm"

msg_kload db 'Load kernel... ',0
msg_ok db 'OK',0x0D,0x0A,0

bios_gdt:
    dq 0 ; нулевой дескриптор
    dq 0 ; настраивается BIOS

    ; Источник данных данных из буфера


times 65536-($-$$) db 0
