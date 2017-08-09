;******************************************************************************
;       Interrupt handle wrappers
;
;******************************************************************************

; CPU exception handlers
extern x86_divide_except
global x86_divide_handle
extern x86_single_step_debug_except
global x86_single_step_debug_handle
extern x86_nonmask_except
global x86_nonmask_handle
extern x86_breakpoint_except
global x86_breakpoint_handle
extern x86_overflow_except
global x86_overflow_handle
extern x86_bound_except
global x86_bound_handle
extern x86_invalid_opcode_except
global x86_invalid_opcode_handle
extern x86_busy_coproc_except
global x86_busy_coproc_handle
extern x86_double_fault_except
global x86_double_fault_handle
extern x86_coproc_overrun_except
global x86_coproc_overrun_handle
extern x86_invalid_tss_except
global x86_invalid_tss_handle
extern x86_no_segment_except
global x86_no_segment_handle
extern x86_stack_except
global x86_stack_handle
extern x86_gpf_except
global x86_gpf_handle
extern x86_page_fault_except
global x86_page_fault_handle
extern x86_coproc_except
global x86_coproc_handle

; PIC handlers
global x86_i8253_irq_handle
extern x86_i8253_irq_do_handle
global x86_kbr_irq_handle
extern x86_kbr_irq_do_handle
;global x86_floppy_irq_handle
;extern x86_floppy_irq_do_handle
global x86_id0_handle
extern x86_id0_do_handler
global x86_id1_handle
extern x86_id1_do_handler
global x86_ata_handle
extern x86_ata_do_handler

;owner interrupt start
global x86_resched_handle
extern x86_resched_do_handler


section .text
align 4

%macro HANDLE 1 

    pushad

    cli
    mov eax,esp
    push eax
    call %1
    pop eax
    sti 
    popad
    iret

%endmacro

%macro HANDLE_TIME 1 

    pushad

    cli
    ;mov eax,esp
    ;push eax
    call %1
    ;pop eax
    sti 
    popad
    iret
%endmacro

%macro HANDLE_PAGE 1 

    pushad

    cli
    mov eax,esp
    push eax
    call %1
    pop eax
    sti 
    popad
    add esp, 4

    iret

%endmacro

;-----------------------------
; first come the CPU handlers
;-----------------------------

x86_divide_handle:
    HANDLE x86_divide_except

x86_single_step_debug_handle:
    HANDLE x86_single_step_debug_except
    
x86_nonmask_handle:
    HANDLE x86_nonmask_except

x86_breakpoint_handle:
    HANDLE x86_breakpoint_except

x86_overflow_handle:
    HANDLE x86_overflow_except

x86_bound_handle:
    HANDLE x86_bound_except

x86_invalid_opcode_handle:
    HANDLE x86_invalid_opcode_except

x86_busy_coproc_handle:
    HANDLE x86_busy_coproc_except

x86_double_fault_handle:
    HANDLE x86_double_fault_except
    
x86_coproc_overrun_handle:
    HANDLE x86_coproc_overrun_except

x86_invalid_tss_handle:
    HANDLE x86_invalid_tss_except

x86_no_segment_handle:
    HANDLE x86_no_segment_except

x86_stack_handle:
    HANDLE x86_stack_except

x86_gpf_handle:
    HANDLE x86_gpf_except

x86_page_fault_handle:
    HANDLE_PAGE x86_page_fault_except

x86_coproc_handle:
    HANDLE x86_coproc_except


;-----------------------------
; PIC handlers
;-----------------------------

x86_i8253_irq_handle:
    HANDLE_TIME x86_i8253_irq_do_handle

x86_kbr_irq_handle:
    HANDLE x86_kbr_irq_do_handle

;x86_floppy_irq_handle:
;    HANDLE x86_floppy_irq_do_handle

x86_id0_handle:
    HANDLE x86_id0_do_handler

x86_id1_handle:
    HANDLE x86_id1_do_handler

x86_ata_handle:
    HANDLE x86_ata_do_handler

;-----------------------------
; my own interrupt
;-----------------------------
x86_resched_handle:
    HANDLE_TIME x86_resched_do_handler
