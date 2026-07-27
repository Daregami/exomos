[BITS 16]
org 0x7E00

_load_start:
    mov si,msg_kload
    call print_str

copy_kernel:
    mov bp,32 ; количество итераций для загрузки ядра - 2мб / 64кб = 32

.loop:
    xor ax,ax
    mov ds,ax

    mov ah,0x42
    mov si,disk_dap
    int 0x13

    jc disk_error

    mov cx, 0x8000 ; счетчик пересылаемых 16-битовых слов
    mov si, bios_gdt
    xor ax, ax
    mov es, ax
    mov ax, 0x8700 ; функция переноса блока
    int 0x15

    jc bios_error

    add dword [disk_dap + 8],128 ; 128 * 512 = 64кб
    add byte [bios_gdt + 28], 0x0001 ; 24 байт до дескриптора приемника
                                     ; + 4 увеличиваем базовый адрес на 64кб

    dec bp
    jnz .loop ; считали нужное количество блоков - выходим

    mov si,msg_ok
    call print_str

    ;mov ax,0x1000
    ;mov ds,ax
    ;mov si,0x7E00
    ;call print_str

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

bios_error:
    mov si,error_bios
    call print_str
.halt:
    cli
    hlt
    jmp .halt

msg_kload db 'Load kernel... ',0
error_disk db 'Disk error... ',0
error_bios db 'Bios error... ',0
msg_ok db 'OK',0x0D,0x0A,0

align 4
disk_dap:
    db 0x10
    db 0
    dw 128 ; читаем 128 сектора за раз в буфер
    ; Адрес 0x00017E00 (буфер)
    dw 0x0000 ; Смещение буфера
    dw 0x2000 ; Сегмент буфера
dap_lba:
    dq 129

align 8
bios_gdt:
    dq 0 ; нулевой дескриптор
    dq 0 ; настраивается BIOS

    ; Источник данных данных из буфера
    dw 0xFFFF ; предел перемещаемого куска
    dw 0x0000 ; базовый адрес буфера
    db 0x02 ; средний базовый адрес
    db 0x93 ; права доступа чтение/запись
            ; находятся в памяти, данные, в Ring0
    db 0x00
    db 0x00
    
    ; Приемник данных данных из буфера
    dw 0xFFFF
    dw 0x0000 ; базовый адрес расположения ядра
    db 0x10 ; средний базовый адрес
    db 0x93 ; права доступа чтение/запись
            ; находятся в памяти, данные, в Ring0
    db 0x00
    db 0x00

    dq 0
    dq 0

times 65536-($-$$) db 0
