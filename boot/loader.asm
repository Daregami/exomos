[BITS 16]
org 0x7E00

; Главный каталог (1024 записи)
PAGE_DIRECTORY      equ 0x10000 ; 4 КБ: 0x10000–0x10FFF
; Таблица для идентичного маппинга адресов
PAGE_TABLE_IDENTITY equ 0x11000 ; 4 КБ: 0x11000–0x11FFF
; Таблица для старших адресов ядра
PAGE_TABLE_HIGH     equ 0x12000 ; 4 КБ: 0x12000–0x12FFF

_load_start:
    mov si,msg_kload
    call print_str

copy_kernel:
    mov bp,64 ; количество итераций для загрузки ядра - 2мб / 32кб = 64

.loop:
    push bp
    xor ax,ax
    mov ds,ax

    mov ah,0x42
    mov dl, 0x80 ; Захардкоженный номер диска
    mov si,disk_dap
    int 0x13

    jc disk_error

    mov cx, 0x4000 ; счетчик пересылаемых 16-битовых слов
    mov si, bios_gdt
    xor ax, ax
    mov es, ax
    mov ax, 0x8700 ; функция переноса блока
    int 0x15

    jc bios_error

    add dword [disk_dap + 8],64 ; 64 * 512 = 32кб
    add word [bios_gdt + 26], 0x8000
    adc byte [bios_gdt + 28], 0
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

    ; Обнуляем область под таблицы (3 * 4096 = 12288 байт = 3072 двойных слова)
    mov edi, PAGE_DIRECTORY
    xor eax, eax
    mov ecx, 3072
    rep stosd

    ; Заполняем идентичный маппинг, чтобы после смены режима
    ; процессор смог считать сдедующие инструкции из памяти
    ; маппим первые 4 МБ (1024 страницы по 4 КБ)
    mov edi, PAGE_TABLE_IDENTITY
    mov eax, 0x00000003  ; физ. адрес 0, биты P + R/W (существует в памяти и можно писать и читать)
    mov ecx, 1024
.fill_identity:
    mov [edi], eax
    add eax, 0x1000
    add edi, 4
    dec ecx
    jnz .fill_identity

    ; Маппим 0xC0000000+ по физическим адресам 0x00000000+ (первые 4 МБ)
    mov edi, PAGE_TABLE_HIGH
    mov eax, 0x00000003
    mov ecx, 1024
.fill_high:
    mov [edi], eax
    add eax, 0x1000
    add edi, 4
    dec ecx
    jnz .fill_high

    ; Заполняем каталог страниц
    mov dword [PAGE_DIRECTORY + 0*4], PAGE_TABLE_IDENTITY + 0x03
    mov dword [PAGE_DIRECTORY + 768*4], PAGE_TABLE_HIGH + 0x03

    ; Загружаем каталов в регистр CR3
    mov eax, PAGE_DIRECTORY
    mov cr3, eax

    ; Включаем пагинацию (31 бит PG в CR0)
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    ; Прыжок в ядро по виртуальному адресу higher-half
    mov esp, 0xC0090000           ; стек тоже через higher-half
    jmp 0x08:0xC0100000           ; ядро по виртуальному адресу

    ; Настройка стека чуть ниже ядра
    ; mov esp,0x90000

    ; Так как сегменты настроены в виде Flat Model (плоской модели) с базовым адресом 0
    ; то физический адрес 0x100000 доступен просто по указателю
    ; mov eax,0x100000
    ; jmp eax ; прыжок в ядро

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
    dw 64 ; читаем 128 сектора за раз в буфер
    ; Адрес 0x00017E00 (буфер)
    dw 0x0000 ; Смещение буфера
    dw 0x2000 ; Сегмент буфера
dap_lba:
    dq 65

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

times 32768-($-$$) db 0
