[bits 32]
section .text
global switch_to
switch_to:
    mov eax, [esp + 4]  ; old
    mov edx, [esp + 8]  ; new

    ; eip has been save when call context_switch
    push ebp
    push ebx
    push esi
    push edi

    ; switch stack
    mov [eax], esp      ; save esp
    mov esp, edx

    pop edi
    pop esi
    pop ebx
    pop ebp
    ret

