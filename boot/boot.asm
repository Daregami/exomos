[BITS 16]
org 0x7C00

_start:
    mov [number_disk],dl ; запоминаем номер диска

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
    
    ; Копирование с помощью lba
    mov ah,0x42
    mov dl,[number_disk]
    mov si,disk_dap
    int 0x13

    jc disk_error

    mov si,msg_ok
    call print_str
    
    mov dl,[number_disk]
    jmp 0x7E00 ; если ошибок не было, прыгаем в загрузчик

%include "print.asm"

disk_error:
    mov si,error_disk
    call print_str
.halt:
    cli ; блокируем прерывания
    hlt ; гасим процессор
    jmp .halt ; если случится немаскированное прерывание


msg_load db 'Loading bootloader... ',0
msg_ok db 'OK',0x0D,0x0A,0
error_disk db 'Disk error...',0
number_disk db 0

align 4
disk_dap:
    db 0x10 ; размер пакета
    db 0x00
    dw 128 ; количество секторов под загрузчик
    dw 0x7E00 ; смещение bx
    dw 0x0000 ; сегмент es
.lba_sector:
    dq 1 ; начальный сектор загрузчика

times 510-($-$$) db 0
dw 0xAA55
