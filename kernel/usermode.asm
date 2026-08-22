[BITS 32]
global jump_to_usermode

section .text

jump_to_usermode:
    mov eax, [esp + 4] ; eip
    mov ecx, [esp + 8] ; esp юзера

    mov dx, 0x23 ; сегмент данных юзера (0x20 | 3)
    mov ds, dx
    mov es, dx
    mov fs, dx
    mov gs, dx

    push 0x23 ; ss юзера
    push ecx ; esp юзера
    pushf ; eflags
    push 0x1B ; cs юзера (0x18 | 3)
    push eax ; eip точка входа
    iret