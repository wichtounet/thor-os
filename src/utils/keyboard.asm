; Return the keyboard input into al
key_wait:
    mov al, 0xD2
    out 0x64, al

    mov al, 0x80
    out 0x60, al

    .key_up:
        in al, 0x60
        and al, 10000000b
    jnz .key_up

    ; key_down
        in al, 0x60

    ret

; In: key in al
; Out: ascci key in al
key_to_ascii:
    and eax, 0xFF
    mov al, [eax + azerty]

    ret

; Qwertz table

azerty:
    db '0',0xF,'1234567890',0xF,0xF,0xF,0xF
    db 'qwertzuiop'
    db '^$',0xD,0x11
    db 'asdfghjklÃ©'
    db 'ù²','*'
    db 'yxcvbnm,;:!'
    db 0xF,'*',0x12,0x20,0xF,0xF
