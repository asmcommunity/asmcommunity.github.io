format ms coff

; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; DATA section
; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
section '.data' data
	_g_cmdline_block	dd 0
	_g_cmdline_args		dd 0
	_g_cmdline_argcount dd 0

	PUBLIC _g_cmdline_block
	PUBLIC _g_cmdline_args
	PUBLIC _g_cmdline_argcount
	

; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; CODE section
; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
section '.code' code

EXTRN _GetProcessHeap@0
EXTRN _HeapAlloc@12

EXTRN _malloc

; internal_alloc: allocates memory, amount of bytes passed in EAX.
internal_alloc:
	push	eax					; dwBytes
	call	_GetProcessHeap@0
	push	dword 0				; dwFlags
	push	eax					; hHeap
	call	_HeapAlloc@12
	ret



;
; void __stdcall cmdline_init(const char *input);
;   tokenizes `input', removing whitespace and handling double-quoted arguments.
;   Output: g_cmdline_block contains input in tokenized-and-terminated form,
;           g_cmdline_args is an array of pointers into g_cmdline_block,
;			g_cmdline_argcount is the number of pointers in g_cmdline_args.
;
PUBLIC _cmdline_init@4
_cmdline_init@4:
	_input$			equ 8
	_totalbytes$	equ (-16)
	_pushedstart$	equ (-24)

	push	ebp
	mov		ebp, esp

	push	ebx
	push	esi
	push	edi
	sub		esp, 4

	xor		esi, esi
	mov		[_g_cmdline_argcount], esi
	mov		[ebp + _totalbytes$], esi

	mov		ebx, [ebp + _input$]
	cmp		byte [ebx], 0
	je		.doneparse

	mov		eax, ebx

.mainloop:
	; skip whitespace
	cmp		byte [eax], ' '
	jne		.notws1
.skipws1:
	inc		esi
	cmp		byte [esi+ebx], ' '
	je		.skipws1
.notws1:

	; if we reached end of string, break
	mov		al, [esi+ebx]
	test	al, al
	je		.doneparse

	; handle quote-delimited args
	cmp		al, '"' 
	jne		.notquote

	; in quotes, consume until endquote
	inc		esi
	mov		edi, esi
	mov		al, [esi+ebx]
	test	al, al
	je		.next_iteration

.find_endquote:
	cmp		al, '"'
	je		.consume_done
	inc		edi
	mov		al, [edi+ebx]
	test	al, al
	jne		.find_endquote
	jmp		.consume_done

.notquote:
	; not in quotes, consume until quote or whitespace
	lea	edi, [esi+1]

	jmp		.consume_loop
.consume:
	cmp		al, '"'
	je		.consume_done
	cmp		al, ' '
	je		.consume_done
	inc		edi
.consume_loop:
	mov		al, [edi+ebx]
	test	al, al
	jne		.consume

.consume_done:
	; at this point we have a string to add, or an empty string in cases like
	; two doublequotes right next to eachother.
	cmp		edi, esi
	jbe		.next_iteration

	; push {start, end} pair to stack
	; !!! 
	push	edi
	push	esi
	; !!!

	; increase totalbytes... +1 because of NUL terminator
	mov		eax, edi
	sub		eax, esi
	inc		eax
	add		[ebp + _totalbytes$], eax

	inc		dword [_g_cmdline_argcount]

.next_iteration:
	lea		esi, [edi + 1]
	lea		eax, [esi + ebx]
	cmp		byte [eax - 1], 0
	jne		.mainloop

.doneparse:

	; Allocate memory for the pointers-to-arguments buffer. Exit if no args
	; were detected.
	mov		eax, [_g_cmdline_argcount]
	test	eax, eax
	jz		.exit
	
	shl		eax, 2
	call	internal_alloc
	mov		[_g_cmdline_args], eax

	; allocate memory for the tokenized commandline buffer
	mov		eax, [ebp + _totalbytes$]
	call	internal_alloc
	mov		[_g_cmdline_block], eax


	; now comes the easy part - filling the g_cmdline_args with pointers, and
	; at the same time g_cmdline_block with tokenized and NUL-terminated strings;
	; copied from the input buffer.
	;
	mov		ecx, [_g_cmdline_argcount]
	mov		edi, eax ; NOTE: EAX still equal to [_g_cmdline_block] from assignment above

	lea		ebx, [ebp + _pushedstart$]
	mov		edx, [_g_cmdline_args]
.fill_loop:
	; store arg pointer
	mov		[edx], edi
	add		edx, 4

	; copy string
	push	ecx
	mov		eax, [ebx + 0]	; start-index
	mov		ecx, [ebx + 4]	; end-index
	sub		ecx, eax

	mov		esi, [ebp + _input$]
	add		esi, eax
	rep		movsb
	xor		al, al
	stosb
	pop		ecx

	; process next item
	sub		ebx, 8
	dec		ecx
	jnz		.fill_loop
	
.exit:
	pop		edi
	pop		esi
	pop		ebx

	leave

	retn	4
