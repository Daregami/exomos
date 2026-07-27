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

    ; Чтение секторов загрузчика
    mov ah,0x02
    mov al,128 ; количество секторов - 64кб на загрузчик
    mov cx,2 ; ch=0 - номер цилиндра; cl=2 - со второго сектора
    mov dh,0 ; номер головки
    mov dl,[number_disk]
    mov bx,0x7E00 ; пишем сразу после MBR
    int 0x13

    jc disk_error

    mov si,msg_ok
    call print_str

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

times 510-($-$$) db 0
dw 0xAA55
