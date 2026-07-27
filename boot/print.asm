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
