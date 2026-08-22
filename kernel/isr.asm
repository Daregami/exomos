[BITS 32]

extern handler_table

%macro isr_stub 1
isr_stub_%1:
    pusha
    push dword %1 ; передаем номер вектора
    call dispatch
    add esp, 4
    popa
    iret
%endmacro

%assign i 0
%rep 256
    isr_stub i
%assign i i+1
%endrep

dispatch:
    mov eax, [esp + 4]
    mov eax, [handler_table + eax * 4]
    test eax, eax
    jz .skip
    push dword [esp + 4]
    call eax
    add esp, 4
.skip:
    ret

; Таблица адресов стабов, которые ссылаются на обработчики
global isr_stub_table
isr_stub_table:
%assign i 0
%rep 256
    dd isr_stub_%+i
%assign i i+1
%endrep

global syscall_stub
extern syscall_dispatch

syscall_stub:
    pusha
    push esp              ; передаем указатель на сохраненные регистры
    call syscall_dispatch
    add esp, 4
    popa
    iret