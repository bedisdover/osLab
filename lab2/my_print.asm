SECTION .DATA
color_red:		db	1Bh, '[31;1m', 0
.len		equ $ - color_red
color_blue:		db	1Bh, '[34;1m', 0
.len		equ $ - color_blue
color_default:  db  1Bh, '[37;0m', 0
.len        equ $ - color_default

SECTION	.text
global  my_printFile
global  my_printDirectory

my_printFile:
    mov 	eax, 4
    mov 	ebx, 1
    mov 	ecx, color_blue
    mov 	edx, color_blue.len
    int 	80h

    mov     ecx, [esp + 4]
    mov     edx, [esp + 8]
    mov     eax, 4
    mov     ebx, 1
    int     80h

    mov     eax, 4
    mov     ebx, 1
    mov     ecx, color_default
    mov     edx, color_default.len
    int     80h

    ret

my_printDirectory:
	mov 	eax, 4
	mov 	ebx, 1
	mov 	ecx, color_red
	mov 	edx, color_red.len
	int 	80h

	mov     ecx, [esp + 4]
    mov     edx, [esp + 8]
    mov     ebx, 1
    mov     eax, 4
    int     80h

    mov     eax, 4
    mov     ebx, 1
    mov     ecx, color_default
    mov     edx, color_default.len
    int     80h

    ret


