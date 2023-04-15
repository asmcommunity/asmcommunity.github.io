; REM SNAP.ASM 12:45 PM 5/24/2006
     .386
      .model flat, stdcall
      option casemap :none   ; case sensitive

; #########################################################################

      include \masm32\include\windows.inc
      include \masm32\include\user32.inc
      include \masm32\include\kernel32.inc

      includelib \masm32\lib\user32.lib
      includelib \masm32\lib\kernel32.lib
VIDEO	segment at 0B800H
SCREEN_BUFFER	DB 4000 dup (?)
VIDEO	ends
snap	segment 'CODE'
        assume  cs:snap,ds:snap,es:snap
; Routine saves screen to disk
; file(only text bytes) also
; removes trailing blanks
; and Blank Lines
video_int	equ	10h
keyboard_int	equ	16h
dos_int	equ	21h
mouse_int	equ	33h
	org	100h
_screen	proc	near
	jmp	program_start
buffer	db	2050 dup (?)
filename	db 'SCREEN.ASC',0
end_of_buffer	dw 0
handle          dw      0
program_start:
; ES > Video Memory
	push	cs
	pop	ds
	mov	ax,0B800h
	mov	es,ax
	lea	si,filename
	mov	[end_of_buffer],si
	lea     dx,filename
	call    create_file
	jc	screen_c
        mov     handle,ax
; Move Video to Disk Buffer
	call    save_screen
	push	ds
        pop     es
; Remove Trailing Blanks
; From Disk Buffer
        call    remove_blanks 
        mov     bx,[handle]
	lea     dx,buffer
	call    write_file
	jc	screen_c
; CLOSE FILE WITH HANDLE
	mov     ah,3Eh
	int     21h
        jc      screen_c
        mov     al,0
screen_c:
	mov     ah,4Ch
	int     21h
_screen	endp
find_ascii	proc	near
; This routine finds last
; ASCII byte > space
; SI > End of ASCII string
; On Exit:
; Carry Set if ASCII byte
; pass start of Buffer
; Carry Cleared if ASCII
; byte is in Buffer
; SI > End of ASCII string
	push	bx
	push	cx
; Find Next Byte > Space
find_ascii_space:
	lodsb	; look for space
	cmp	al,' '
	jbe	find_ascii_space
; stop on > space
	inc	si
	inc	si; > Pass End of Ascii String
; Is ASCII byte found still in Buffer
	lea	cx,buffer
	cmp	ecx,esi; start > end
	jc	find_ascii_x
; end < start  set carry Not in Buffer
; for Error Exit 
	mov	cx,si
	stc
	pop	cx
	pop	bx
	ret
find_ascii_x:
; Okay clear carry
; ASCII byte is in Buffer
	clc
	pop	cx
	pop	bx
	ret
find_ascii	endp
remove_blanks	proc	near
	mov	cx,2050
	lea	di,buffer
find_next_line:
	mov	al,13
	cld; UP
; Find Next Line <CR>
	call	scan_byte
	jc	remove_blanks_x
; <CR> Found DI > <CR>
	mov	si,di; > <CR>
	mov	dx,si
	std; down
; Find Last Char > Space
; Before <CR>	
	call	find_ascii
; SI > Blank after Last Char > Space
; SI > Insertion Point for Next Line
; DI > Start of Next Line
	xchg	si,di
	mov	dx,si
	mov	bx,di
; Find No. of Spaces at End
; of Line
	sub	dx,di
; Find No. of Bytes to
; Move up in Buffer
	lea	cx,end_of_buffer
	sub	cx,si; Calculate length
	cld; up
; Move Buffer Up
; to clear Trailing Blanks
	rep	movsb; was repz
; Clear End of Buffer
; by No. of Spaces Removed
	std; Down
	xor	al,al
	mov	cx,dx; No. of Spaces	
	lea	di,end_of_buffer
	dec	di; > Byte before end
	rep	stosb; Clear ** was repz
	mov	cx,di
; Find End of Buffer Byte
; After last <LF>
	inc	cx
	mov	end_of_buffer,cx
; Caculate New Length
	push	dx
	lea	dx,buffer
	sub	cx,dx; was offset buffer
	pop	dx
	mov	di,bx
	inc	di; > byte after <CR> 
	jmp	find_next_line
remove_blanks_x:
; Calculate New Buffer length
	lea	cx,end_of_buffer
	push	dx
	lea	dx,buffer
	sub	cx,dx; was offset buffer
	pop	dx
	ret
remove_blanks	endp
	if	0
INT 21 - DOS 2+ - CREATE A FILE WITH HANDLE (CREAT)
	AH = 3Ch
	CX = attributes for file
	DS:DX = address of ASCIZ filename
Return: CF = 1 if error
	    AX = Error Code
	CF = 0 successful
	    AX = file handle
	endif
create_file	proc	near
	push	cx		
	sub	cx,cx 		
 	mov	ah,3Ch		
	int	dos_int		
	pop	cx 		
	ret 
create_file	endp
	if	0
INT 21 - DOS 2+ - WRITE TO FILE WITH HANDLE
	AH = 40h
	BX = file handle
	CX = number of bytes to write
	DS:DX = pointer to buffer
Return: CF = 1 if error
	    AX = Error Code
	CF = 0 successful
	    AX = number of bytes written
Note: if CX is zero, no data is written, and
      the file is truncated or extended to
      the current position.
	endif
write_file	proc	near
	mov	ah,40h      
	int	dos_int         
	ret
write_file	endp
save_screen	proc	near
	push	ds
	push	es
;  ES:DI > Video Memory
;  DS:DS > Disk Buffer 
; Can not use over ride
; Prefixs on LODSB and
; STOSB	
; swap DS and ES for
; LODSB and STOSB
; instructions
	push	es
	mov	ax,ds
	mov	es,ax
	pop	ds
	xor	si,si		
	lea	di,buffer
; # LINES ON SCREEN		
	mov     cx,25
	cld; up
next_line:
	push	cx
; # BYTES ON A LINE
	mov     cx,80
next_byte:
	lodsb	; get text from Video
	stosb	; save char in Disk Buffer
	lodsb	; skip attribute
	loop	next_byte
; CR+LF after each Line
	mov	al,13
	stosb	; INSERT <CR>
	mov	al,10
	stosb	; INSERT <LF>
	pop	cx
	loop	next_line
; restore segment registers
	pop	es
	pop	ds
	ret			
save_screen	endp
scan_byte	proc	near
; ES:DI > Data to be searched
; AL = byte to be found
; CX = length of search data
; Exit: Carry ON byte AL not found
;       Carry OFF byte AL found
;	
; Routines called none
	repnz	scasb
	jnz	scan_byte_x
	dec	di
	clc
	ret
scan_byte_x:
	stc
	ret
scan_byte	endp
snap	ends
	end	_screen


