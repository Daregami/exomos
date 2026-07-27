[BITS 16]
org 0x7E00

_load_start:
    mov si,msg_kload
    call print_str

copy_kernel:
    mov bp,64 ; количество итераций для загрузки ядра - 2мб / 32кб = 64

    mov ah,0x42
    mov si,disk_dap
    int 0x13

    jc disk_error

    mov si,msg_ok
    call print_str

stop_system:
    cli
    hlt
    jmp stop_system

%include "print.asm"

disk_error:
    mov si,error_disk
    call print_str
.halt:
    cli
    hlt
    jmp .halt

msg_kload db 'Load kernel... ',0
error_disk db 'Disk error... ',0
msg_ok db 'OK',0x0D,0x0A,0

align 4
disk_dap:
    db 0x10
    db 0
    dw 64 ; читаем 64 сектора за раз в буфер
    ; Адрес 0x00017E00 (буфер)
    dw 0x7E00       ; Смещение буфера
    dw 0x1000       ; Сегмент буфера
dap_lba:
    dq 129

align 8
bios_gdt:
    dq 0 ; нулевой дескриптор
    dq 0 ; настраивается BIOS

    ; Источник данных данных из буфера
    dw 0xFFFF ; предел перемещаемого куска
    dw 0x7E00 ; базовый адрес буфера
    dw 0x0001 ; средний базовый адрес
    db 0x93 ; права доступа чтение/запись
            ; находятся в памяти, данные, в Ring0
    db 0x00
    db 0x00
    
    ; Приемник данных данных из буфера
    dw 0xFFFF
    dw 0x0000 ; базовый адрес расположения ядра
    base_addr dw 0x0010 ; средний базовый адрес
    db 0x93 ; права доступа чтение/запись
            ; находятся в памяти, данные, в Ring0
    db 0x00
    db 0x00

    dq 0
    dq 0

times 65536-($-$$) db 0
