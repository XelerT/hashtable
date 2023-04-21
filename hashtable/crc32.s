section .text

global asm_get_crc32_hash

;===============================================
; Entry: rdi - pointer on the word_t

asm_get_crc32_hash:

        mov rax, 0xFFFFFFFF
        mov rcx, qword [rdi + 8]
        mov rsi, qword [rdi]
.hash:
        crc32 eax, byte [rsi]
        add rsi, 0x1
        loop .hash

        ret

