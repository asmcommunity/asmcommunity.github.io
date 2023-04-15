.586p
.model flat,stdcall
option casemap:none
option proc:private

incAPI MACRO files:VARARG
	FOR file, <files>
		include		file&.inc
		includelib	file&.lib
	ENDM
ENDM

include		<windows.inc>
incAPI		<kernel32,user32>

ASSUME FS:NOTHING

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; DATA? section
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.data?
	szBuffer	BYTE	128 dup (?)

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; DATA section
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.data
	szString	BYTE	" 1234       5678   9      ", 0


;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; CODE section
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.code

; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; void trimspace(char *dst, char *src, unsigned dstlen)
; ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
;
; Removes duplicate spaces in string, and makes sure not to produce any
; buffer overflows. Safety over speed, can probably be optimized plenty.
; 
; dstlen includes trailing NUL char, and destination will always be zero
; terminated - ie, a 2-byte buffer will hold a string of 1 char.
;
OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE

trimspace proc stdcall a_dst:dword, a_src:dword, a_len:dword
arg_dst		= 8
arg_src		= 12
arg_len		= 16

	push	esi
	mov		esi, [esp + arg_len]
	test	esi, esi
	jz		short @@out
	cmp		esi, 1
	jnz		short @@longer_than_1
	mov		eax, [esp + arg_dst]
	mov		byte ptr [eax],	0
	pop		esi
	retn	12

@@longer_than_1:
	mov		eax, [esp + arg_src]
	mov		edx, [esp + arg_dst]
	dec		esi
	push	ebx

@@mainloop:
	mov		cl, [eax]
	inc		eax
	cmp		cl, ' '
	jnz		short @@space_done
	cmp		[eax], cl
	jnz		short @@space_done
	nop

@@skipspace:
	mov		bl, [eax + 1]
	inc		eax
	cmp		bl, ' '
	jz		short @@skipspace

@@space_done:
	mov		[edx], cl
	inc		edx
	dec		esi
	test	cl, cl
	jz		short @@zterm_out
	test	esi, esi
	jnz		short @@mainloop

@@zterm_out:
	mov		byte ptr [edx],	0
	pop		ebx

@@out:
	pop		esi
	retn	12
trimspace	endp

OPTION PROLOGUE:PROLOGUEDEF
OPTION EPILOGUE:EPILOGUEDEF

ENTRY32:
	invoke	trimspace, addr szBuffer, addr szString, sizeof szBuffer
	invoke	MessageBox, 0, addr szBuffer, addr szString, MB_OK
	invoke	ExitProcess, 0	

END ENTRY32
