[BITS 32]
org 0x00400000

; Печатаем приглашение
mov eax, 1
mov ebx, 1
mov ecx, prompt
mov edx, 2
int 0x80

.loop:
    ; Читаем символ с клавиатуры
    mov eax, 2
    mov ebx, 0
    mov ecx, buf
    mov edx, 1
    int 0x80

    ; Печатаем символ на экран (эхо)
    mov eax, 1
    mov ebx, 1
    mov ecx, buf
    mov edx, 1
    int 0x80

    jmp .loop

prompt db '> '
buf db 0