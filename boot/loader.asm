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
    xor ax, ax
    mov ds, ax

    ; Если bios выдал нулевой диск, меняем на стандартный
    mov al,dl
    test al,al
    jnz .drive_ok
    mov dl, 0x80 ; захардкоженный номер диска
.drive_ok:
    mov [boot_drive], dl

    ; Читаем первый чанк в 32кб - ELF заголовок
    mov ah, 0x42
    mov si, disk_dap
    int 0x13
    jc disk_error

    ; Проверяем ELF магию
    mov ax, 0x2000
    mov es, ax
    cmp dword [es:0x0000], 0x464C457F
    jne elf_error

    mov eax, [es:0x18] ; e_entry - виртуальный адрес, куда прыгать
    mov [elf_entry], eax

    mov eax, [es:0x1C] ; e_phoff - смещение таблицы program headers
    mov [elf_phoff], eax ; от начала файла в байтах

    movzx ax, word [es:0x2A] ; e_phentsize - размер одной записи phdr
    mov [elf_phentsize], ax

    movzx ax, word [es:0x2C] ; e_phnum - сколько program headers в файле
    mov [elf_phnum], ax

    xor cx,cx
    mov di, seg_table ; массив PT_LOAD записей
    mov word [seg_count], 0 ; сколько PT_LOAD нашли

.parse_phdr:
    cmp cx, [elf_phnum] ; пока не пройдем все записи
    jge .parse_done
    push cx

    ; Вычисляем где в буфере лежит текущий program header
    ; смещение = e_phoff + индекс * e_phentsize
    movzx eax, cx
    movzx ebx, word [elf_phentsize]
    imul eax, ebx ; eax = индекс * размер_записи
    add eax, [elf_phoff] ; eax = смещение phdr от начала файла
    mov si, ax ; si = смещение внутри буфера

    ; Проверяем PD_LOAD по первым 4 байтам
    ; эта секция говорит о том, что нужно положить байты по адресу
    cmp dword [es:si], 1
    jne .skip_phdr

    mov eax, [es:si + 0x04] ; p_offset - откуда в файле считывать
    mov [di + 0], eax

    mov eax, [es:si + 0x0C] ; p_paddr - куда копировать в физическую память
    mov [di + 4], eax

    mov eax, [es:si + 0x10] ; p_filesz - байт данных в файле
    mov [di + 8], eax

    mov eax, [es:si + 0x14] ; p_memsz - размер сегмента в памяти
    mov [di + 12], eax      ; нужен, чтобы знать, как обнулять .bss

    add di, 16
    inc word [seg_count]

.skip_phdr:
    pop cx
    inc cx ; следующий phdr
    jmp .parse_phdr

.parse_done:
    mov si, seg_table
    xor cx, cx

; Грузим каждый PT_LOAD сегмент с диска
.load_segment:
    cmp cx, [seg_count] ; пока не загрузим все сегменты
    jge .segments_done
    push cx
    push si

    mov eax, [si + 0] ; p_offset
    mov [seg_offset], eax

    mov eax, [si + 4] ; p_paddr
    mov [seg_paddr], eax

    mov eax, [si + 8] ; p_filesz
    mov [seg_filesz], eax

    ; p_offset смещение в байтах от начала ELF-файла
    ; shr eax, 9 - это деление на 512 (размер сектора)
    ; 65 LBA, с которого ELF записан на диск
    ; LBA = начало_elf_на_диске + p_offset / 512
    mov eax, [seg_offset]
    shr eax, 9 ; / 512
    add eax, 65 ; + стартовый LBA ядра
    mov [cur_lba], eax

    ; Сколько секторов читать (p_filesz + 511) / 512
    mov eax, [seg_filesz]
    add eax, 511
    shr eax, 9
    mov [seg_sectors], eax

    mov eax, [seg_paddr]
    mov [cur_dest], eax

.chunk_loop:
    cmp dword [seg_sectors], 0
    je .next_segment

    mov eax, [seg_sectors]
    cmp eax, 64
    jbe .chunk_ok
    mov eax, 64
.chunk_ok:
    mov [cur_chunk], eax ; запоминаем размер этой порции

    ; Читаем порцию с диска в буфер
    mov word [disk_dap + 2], ax ; записываем кол-во секторов в DAP
    mov eax, [cur_lba]
    mov [disk_dap + 8], eax ; записываем LBA в DAP
    mov dword [disk_dap + 12], 0 ; старшие 32 бита LBA = 0

    xor ax, ax
    mov ds, ax
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, disk_dap
    int 0x13 ; читаем все в буфер
    jc disk_error

    ; Обновляем адрес приемника
    mov eax, [cur_dest]
    mov [bios_gdt + 26], ax
    shr eax, 16
    mov [bios_gdt + 28], al
    mov [bios_gdt + 31], ah

    ; cx для int 0x15 = количество 16-битных слов
    ; один сектор = 512 байт = 256 слов.
    ; значит секторов * 256 = слов.
    movzx ecx, word [cur_chunk]
    shl ecx, 8

    xor ax, ax
    mov es, ax
    mov si, bios_gdt
    mov ax, 0x8700
    int 0x15
    jc bios_error

    ; Сдвигаем счётчики для следующей порции
    mov eax, [cur_chunk]
    sub [seg_sectors], eax
    add [cur_lba], eax ; LBA сдвигается вперёд на диске

    shl eax, 9
    add [cur_dest], eax

    jmp .chunk_loop

.next_segment:
    pop si ; восстанавливаем указатель на массив
    pop cx ; восстанавливаем индекс сегмента
    add si, 16 ; переходим к следующей записи (16 байт)
    inc cx
    jmp .load_segment

.segments_done:
    mov si, msg_ok
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

; Для каждого PT_LOAD сегмента проверяем
; если p_memsz > p_filesz значит хвост нужно забить нулями
.zero_bss:
    cmp dword [bss_count], 0
    je .zero_done

    ; edi = p_paddr + p_filesz - первый байт после данных из файла
    mov edi, [si + 4]            ; p_paddr
    add edi, [si + 8]            ; + p_filesz

    ; ecx = p_memsz - p_filesz - сколько байт обнулить
    mov ecx, [si + 12] ; p_memsz
    sub ecx, [si + 8] ; - p_filesz
    jz .zero_next ; если 0 обнулять нечего

    xor al, al
    rep stosb ; забиваем нулями

.zero_next:
    add si, 16 ; следующая запись в массиве
    dec dword [bss_count]
    jmp .zero_bss

.zero_done:
    ; Прыжок в ядро по виртуальному адресу higher-half
    mov esp, 0xC0090000 ; стек тоже через higher-half
    mov eax, [elf_entry] ; ядро по виртуальному адресу
    jmp eax

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

[BITS 16]

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

elf_error:
    mov si, error_elf
    call print_str
.halt:
    cli
    hlt
    jmp .halt

msg_kload db 'Load kernel... ',0
error_disk db 'Disk error... ',0
error_bios db 'Bios error... ',0
msg_ok db 'OK',0x0D,0x0A,0
error_elf db 'Not ELF',0

boot_drive      db 0 ; номер диска от BIOS

elf_entry       dd 0 ; e_entry куда прыгать после загрузки
elf_phoff       dd 0 ; e_phoff где в файле таблица phdr
elf_phentsize   dw 0 ; e_phentsize размер одной записи phdr
elf_phnum       dw 0 ; e_phnum количество записей phdr

seg_count       dw 0 ; сколько PT_LOAD сегментов нашли
seg_table       times 16 * 8 db 0 ; массив: до 8 записей по 16 байт
                                 ; (p_offset, p_paddr, p_filesz, p_memsz)

seg_offset      dd 0 ; p_offset текущего сегмента
seg_paddr       dd 0 ; p_paddr физический адрес назначения
seg_filesz      dd 0 ; p_filesz байт данных в файле
seg_sectors     dd 0 ; сколько секторов осталось прочитать
cur_lba         dd 0 ; текущий LBA на диске
cur_dest        dd 0 ; текущий адрес назначения в памяти
cur_chunk       dd 0 ; секторов в текущей порции

bss_count       dd 0             ; счётчик для цикла обнуления

align 4
disk_dap:
    db 0x10
    db 0
    dw 64 ; читаем 64 сектора за раз в буфер
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
