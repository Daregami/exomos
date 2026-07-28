[BITS 16]
org 0x7E00

_load_start:
    mov si,msg_kload
    call print_str

copy_kernel:
    mov bp,32 ; количество итераций для загрузки ядра - 2мб / 64кб = 32

.loop:
    push bp
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
    add word [bios_gdt + 28], 0x0001 ; 24 байт до дескриптора приемника
                                     ; + 4 увеличиваем базовый адрес на 64кб
    pop bp
    dec bp
    jnz .loop ; считали нужное количество блоков - выходим

    mov si,msg_ok
    call print_str

    ; Открываем доступ к памяти объемом больше 1мб
%include "a20_enable.asm"

    ; Переходим в Protect Mode
    cli ; намертво блокируем прерывания из реального режима
    lgdt [gdt_descriptor] ; загружаем указатель на GDT

    ; Включаем бит PE
    mov eax,cr0
    or eax,1
    mov cr0,eax ; в протект моде

    jmp 0x08:pm_entry ; обязательный дальний прыжок дабы сбросить конвейер из 16 битных инструкций
                      ; а также применение нового сегмента кода, что по факту просто смещение от GDT структуры

[BITS 32]
pm_entry:
    ; Настраиваем сегментные регистры на селектор данных
    mov ax,0x10
    mov ds,ax
    mov es,ax
    mov fs,ax
    mov gs,ax
    mov ss,ax

    ; Настройка стека чуть ниже ядра
    mov esp,0x90000

    ; Так как сегменты настроены в виде Flat Model (плоской модели) с базовым адресом 0
    ; то физический адрес 0x100000 доступен просто по указателю
    mov eax,0x100000
    jmp eax ; прыжок в ядро

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

%include "gdt_table.asm"

times 65536-($-$$) db 0
