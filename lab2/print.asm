SECTION .DATA
color_red:		db	1Bh, '[31;2m', 0
.len		equ $ - color_red
color_blue:		db	1Bh, '[34;1m', 0
.len		equ $ - color_blue
color_default:  db  1Bh, '[37;0m', 0
.len        equ $ - color_default

message			db	'Hello World!!!', 0Ah

SECTION	.text
global _start

_start:
    mov 	eax, 4
    mov 	ebx, 1
    mov 	ecx, color_blue
    mov 	edx, color_blue.len
    int 	80h

    mov     ecx, message
    mov     edx, 14
    mov     eax, 4
    mov     ebx, 1
    int     80h

    mov 	ebx, 0
	mov 	eax, 1
    int 	80h

