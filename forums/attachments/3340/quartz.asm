;----------
;Name: Mac OS X Quartz (Core Graphics) Hello World Demo
;Author: Keith Kanios <keith@kanios.net>
;Purpose: Prints "Hello, World!" to the Screen and Terminal
;Requires: Mac OS X, NASM 2.07+ and XCode
;Tested on Mac OS X 10.5 (Leopard)
;----------
;For 32-bit version:
; assemble using: nasm -f macho quartz.asm -o quartz.o
; link using: ld -e _start -lc -framework ApplicationServices -o quartz quartz.o
;----------
;For 64-bit version:
; assemble using: nasm -f macho64 quartz.asm -o quartz.o
; link using: ld -e _start -lc -ldylib1.o -framework ApplicationServices -o quartz quartz.o
;----------

;##### Check NASM Version #####
%ifdef __NASM_VERSION_ID__
	%if __NASM_VERSION_ID__ >= 0x02070000
		%define _VALID_NASM_ 1
	%endif
%endif
%ifndef _VALID_NASM_
	%fatal "NASM 2.07 or later is required."
%endif

;##### External Functions #####
extern _CGMainDisplayID
extern _CGDisplayCapture
extern _CGDisplayGetDrawingContext
extern _CGContextSelectFont
extern _CGContextSetTextDrawingMode
extern _CGContextSetRGBFillColor
extern _CGContextSetRGBStrokeColor
extern _CGContextShowTextAtPoint
extern _CGContextFlush
extern _CGDisplayRelease
extern _printf
extern _sleep

;##### Select Variable Size #####
%if __BITS__ == 64
	default rel
	%idefine DS DQ
%else
	%idefine DS DD
%endif


;##### Initialized Data #####
[section .data align=16]
	msg DB 'Hello, World!',0x0A
	len equ ($ - msg)
	errstr DB 'Error Number: %i',0x0A,0x00
	font DB 'Times-Roman',0x00
	font_size DS 48.0
	red DS 1.0
	green DS 1.0
	blue DS 1.0
	opacity DS 0.75
	x DS 40.0
	y DS 40.0

;##### Uninitialized Data #####
[section .bss align=16]
	display RESQ 1
	context RESQ 1

;##### Program Entry Point #####
global _start			;Declare Entry Point
[section .text align=16]
_start:

;##### 64-bit Version #####
%if __BITS__ == 64

	;#### Get Main Display ID ####
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	call	_CGMainDisplayID
	mov	rsp,rbx
	mov	DWORD[display],eax

        ;#### Capture the Main Display ####
	mov     rbx,rsp
	and     rsp,0xFFFFFFFFFFFFFFF0
	mov     rdi,QWORD[display]
	call    _CGDisplayCapture
	mov     rsp,rbx
	test	eax,eax
	jnz     .errno_64

	;#### Get Display Drawing Context ####
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,QWORD[display]
	call	_CGDisplayGetDrawingContext
	mov	rsp,rbx
	test	eax,eax
	jz	.errno_64
	mov	DWORD[context],eax

	;#### Set the Font ####
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,QWORD[context]
	mov	rsi,font
	movq	xmm0,QWORD[font_size]
	mov	rdx,1
	call	_CGContextSelectFont
	mov	rsp,rbx

	;#### Set the Text Drawing Mode ####
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,QWORD[context]
	mov	rsi,2
	call	_CGContextSetTextDrawingMode
	mov	rsp,rbx

	;#### Set RGB Fill Color ####
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,QWORD[context]
	movq	xmm0,QWORD[red]
	movq	xmm1,QWORD[green]
	movq	xmm2,QWORD[blue]
	movq	xmm3,QWORD[opacity]
	call	_CGContextSetRGBFillColor
	mov	rsp,rbx

	;#### Set RGB Stroke Color ####
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,QWORD[context]
	movq	xmm0,QWORD[red]
	movq	xmm1,QWORD[green]
	movq	xmm2,QWORD[blue]
	movq	xmm3,QWORD[opacity]
	call	_CGContextSetRGBStrokeColor
	mov	rsp,rbx

	;#### Print Text to Screen ####
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,QWORD[context]
	movq	xmm0,QWORD[x]
	movq	xmm1,QWORD[y]
	mov	rsi,msg
	mov	rdx,len - 1
	call	_CGContextShowTextAtPoint
	mov	rsp,rbx

	;#### Flush the Current Context ####
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,QWORD[context]
	call	_CGContextFlush
	mov	rsp,rbx

	;#### Sleep for 5 seconds ####
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,5
	call	_sleep
	mov	rsp,rbx

	;#### Release the Main Display ####
	mov	rbp,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,QWORD[display]
	call	_CGDisplayRelease
	mov	rsp,rbx

	;#### syscall.write() to STDOUT ####
.write:
	mov	rdx,len
	mov	rsi,msg
	mov	rdi,1
	mov	rax,0x02000004
	syscall

	;#### syscall.exit() ####
.exit_64:
	xor	rdi,rdi
	mov	rax,0x02000001
	syscall
	ret

	;#### Print Specified Error Number ####
.errno_64:
	mov	rbx,rsp
	and	rsp,0xFFFFFFFFFFFFFFF0
	mov	rdi,errstr
	mov	rsi,rax
	call	_printf
	mov	rsp,rbx
jmp	.exit_64

;##### 32-bit Version #####
%else

	;#### Get Main Display ID ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	call	_CGMainDisplayID
	mov	esp,ebx
	mov	DWORD[display],eax

	;#### Capture the Main Display ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	sub	esp,12
	push	DWORD[display]
	call	_CGDisplayCapture
	mov	esp,ebx
	test	eax,eax
	jnz	near .errno_32

	;#### Get Display Drawing Context ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	sub	esp,12
	push	DWORD[display]
	call	_CGDisplayGetDrawingContext
	mov	esp,ebx
	test	eax,eax
	jz	near .errno_32
	mov	DWORD[context],eax	

	;#### Set the Font ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	push	DWORD 1
	push	DWORD[font_size]
	push	DWORD font
	push	DWORD[context]
	call	_CGContextSelectFont
	mov	esp,ebx

	;#### Set the Text Drawing Mode ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	sub	esp,8
	push	DWORD 2
	push	DWORD[context]
	call	_CGContextSetTextDrawingMode
	mov	esp,ebx

	;#### Set RGB Fill Color ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	sub	esp,12
	push	DWORD[opacity]
	push	DWORD[blue]
	push	DWORD[green]
	push	DWORD[red]
	push	DWORD[context]
	call	_CGContextSetRGBFillColor
	mov	esp,ebx

	;#### Set RGB Stroke Color ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	sub	esp,12
	push	DWORD[opacity]
	push	DWORD[blue]
	push	DWORD[green]
	push	DWORD[red]
	push	DWORD[context]
	call	_CGContextSetRGBStrokeColor
	mov	esp,ebx

	;#### Print Text to Screen ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	sub	esp,12
	push	DWORD len - 1
	push	DWORD msg
	push	DWORD[x]
	push	DWORD[y]
	push	DWORD[context]
	call	_CGContextShowTextAtPoint
	mov	esp,ebx

	;#### Sleep for 5 seconds ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	sub	esp,12
	push	DWORD 5
	call	_sleep
	mov	esp,ebx

	;#### Release the Main Display ####
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	sub	esp,12
	push	DWORD[display]
	call	_CGDisplayRelease
	mov	esp,ebx

	;#### syscall.write() to STDOUT ####
	push	DWORD len
	push	DWORD msg
	push	DWORD 1
	sub	esp,4
	mov	eax,4
	int	0x80
	add	esp,16

	;#### syscall.exit() ####
.exit_32:
	push	DWORD 0
	sub	esp,4
	mov	eax,1
	int	0x80
	add	esp,8
	ret

	;#### Print Specified Error Number ####
.errno_32:
	mov	ebx,esp
	and	esp,0xFFFFFFF0
	sub	esp,8
	push	eax
	push	DWORD errstr
	call	_printf
	mov	esp,ebx
jmp	.exit_32
%endif
