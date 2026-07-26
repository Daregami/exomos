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
    
    mov si,msg
    call print_str

    jmp $ ; бесконечный цикл

; Функция печати строки
print_str:
.loop:
    lodsb ; считать символ в регистр al
    or al,al ; если не ноль продолжаем цикл
    jz .done
    mov ah,0x0E ; функция режима телетайпа
    int 0x10
    jmp .loop

.done:
    ret

msg db 'Hello, world!',0

times 510-($-$$) db 0
dw 0xAA55
