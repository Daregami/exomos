[BITS 16]
org 0x7E00

_load_start:
    mov si,msg_kload
    call print_str

    jmp $

%include "print.asm"

msg_kload db 'Load kernel... ',0
msg_ok db 'OK',0x0D,0x0A,0

times 65536-($-$$) db 0
