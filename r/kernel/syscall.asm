
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                               syscall.asm
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
;                                                     Forrest Yu, 2005
; ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

%include "sconst.inc"

_NR_get_ticks           equ 0 ; 要跟 global.c 中 sys_call_table 的定义相对应！
_NR_disp_str_1          equ 1 ;
_NR_disp_color_str_1    equ 2 ;
_NR_process_sleep       equ 3 ;
_NR_sem_p               equ 4 ;
_NR_sem_v               equ 5 ;
_NR_process_wakeup      equ 6 ;

INT_VECTOR_SYS_CALL equ 0x90

; 导入全局变量
extern	disp_pos

; 导出符号
global	get_ticks
global  disp_str_1
global  disp_color_str_1
global  process_sleep
global  sem_p
global  sem_v
global  process_wakeup

bits 32
[section .text]

; ====================================================================
;                              get_ticks
; ====================================================================
get_ticks:
	mov 	eax, _NR_get_ticks
	int 	INT_VECTOR_SYS_CALL
	ret

; ========================================================================
;                             disp_str_1
; ========================================================================
disp_str_1:
    mov 	eax, _NR_disp_str_1
   	mov 	ebx, [esp + 4]
   	int 	INT_VECTOR_SYS_CALL
   	ret


; ========================================================================
;                             disp_color_str_1
; ========================================================================
disp_color_str_1:
    mov 	eax, _NR_disp_color_str_1
   	mov 	ebx, [esp + 4]
   	mov     ecx, [esp + 8]
   	int 	INT_VECTOR_SYS_CALL
   	ret


; ========================================================================
;                             process_sleep
; ========================================================================
process_sleep:
    mov 	eax, _NR_process_sleep
   	mov 	ebx, [esp + 4]
   	int 	INT_VECTOR_SYS_CALL
   	ret


; ========================================================================
;                             process_wakeup
; ========================================================================
process_wakeup:
    mov 	eax, _NR_process_wakeup
   	mov 	ebx, [esp + 4]
   	int 	INT_VECTOR_SYS_CALL
   	ret


; ========================================================================
;                             sem_p
; ========================================================================
sem_p:
    mov 	eax, _NR_sem_p
   	mov 	ebx, [esp + 4]
   	int 	INT_VECTOR_SYS_CALL
   	ret



; ========================================================================
;                             sem_v
; ========================================================================
sem_v:
    mov 	eax, _NR_sem_v
   	mov 	ebx, [esp + 4]
   	int 	INT_VECTOR_SYS_CALL
   	ret