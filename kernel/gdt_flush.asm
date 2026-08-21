[BITS 32]
global gdt_flush
global gdt_load_tss

section .text
gdt_flush:
    mov eax, [esp + 4] ; адрес структуры gdt_ptr
    lgdt [eax]
    mov ax, 0x10 ; сегмент данных ядра
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    jmp 0x08:.flush ; перезагружаем cs
.flush:
    ret

; Загрузка .tss
gdt_load_tss:
    mov ax, 0x28 ; селектор TSS
    ltr ax
    ret