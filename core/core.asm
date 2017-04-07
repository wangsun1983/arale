global _start; 导出 _start


[section .text]
;************************************************;
;	Prints a string
;	DS=>SI: 0 terminated string
;************************************************;
_start:       
       extern start_core
       call start_core
       cli
       hlt

;************************************************;
;because we defined the entry address 0x100000
;so we should include all the inc after the main
;entry.
;************************************************;
%include "debug.inc" 

[section .data]

LoadingMsg db "Happy new year", 0

[SECTION .bss]
;StackSpace		resb	2 * 1024 * 1024
;StackTop:		; 栈顶




