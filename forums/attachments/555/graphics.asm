;a program that obtains a digit from 1-7 from the user and then
;draws 5 squares to the screen each digit characters long.
;
;Author: Josh Drayton
;		sub methods drawChar1 and 
;					getValidDigitFromKeyboard - Ross Hayward
;Version: 1.0 
;Realease date: 17/04/05
 
.model small
.stack 100
.data
	left_corner_Black dw  0C28H ;12 * 40 (the centre co-ord of the screen)
	left_corner_left dw   0H
	left_corner_right dw  0H
	left_corner_top dw    0H
	left_corner_bottom dw 0H
	attr db 70H ;16*2(white background)+ 0 (black text)
	Digit dw 0H
	; Some constants for the top of the file
	ZERO_ASCII 			EQU 30H
	SEVEN_ASCII			EQU 37H
	VIDEO_SEG_START	EQU 0B800H
;…
.code

main proc
	mov ax,@data
	mov ds,ax
	mov ax,0003H 
	int 10H			;ensure the correct graphics mode is selected
					;eventually wish to devise a way to save the contents of video
					;memory first then restore on termination of program
	push ax
	call getValidDigitFromKeyboard
	mov Digit, ax
	pop ax
	mov ax, Digit
	
	jnp DIVISION
	sub al, 1D	;take digit - 1
	
	DIVISION:
	shr al, 1D ;divide digit by 2

	mov dx, left_corner_Black
	sub dh, al	;take the digit/2 from centre row
	sub dl, al	;take the digit/2 from centre column
	mov left_corner_Black, dx	; put the co-ord for the black square in dx
    mov ax, Digit
	sub dh, al	;sub digit from centre row
	mov left_corner_left, dx

	add dh, al
	add dh, al	;add 2* digit
	mov left_corner_right, dx

	sub dh, al ; sub the digit from location of the top left corner of the right square(row)
	sub dl,al  ; sub the digit from current column location
	mov left_corner_top, dx
	
	add dl,al ; add the digit to current column
	add dl,al ; add the digit to current column
	mov left_corner_bottom, dx
	
	mov dx, left_corner_black
	mov ax, 0H
	mov bx, Digit
	mov bh, attr
	push dx
	push bx
	call drawSquare ; dx:row,column  bx:70H, digit
	pop bx
	pop dx
	mov dx, left_corner_left
	mov bh, 72H	;16*7(white background) + 2(green text)
	push dx
	push bx
	call drawSquare ; dx:row,column  bx:72H
	pop bx
	pop dx
	
	;SPACEWAIT:		having trouble implementing a way to finish 
					;the program by pressing a certain key
	;int 16
	;cmp al, 34H
	;jne SPACEWAIT
	;mov ax, 0H
	;int 20
;------------------------------------------------	
;getValidDigitFromKeyboard ()
; get a numeric digit between 1 and 7 from the keyboard
;
; Pre: True
; Post: al is value of digit, ah is ascii value of digit
;	30H<ah<38H AND 0<al<08H
;------------------------------------------------
getValidDigitFromKeyboard  proc near
	
	; call service 16/00 repeatedly if ASCII value is not in
	; correct range
	getNumLoop:		;loop start
        mov ax,00H	;get keyboard character using BIOS Service
        int 16H

	;check loop guard
	;if ASCII value not between 1 and 9 re-enter loop
	cmp al, ZERO_ASCII	
	jle getNumLoop
	cmp al, SEVEN_ASCII
    jg getNumLoop
    
    
	;assert 30H<al<3AH
	mov ah, al
	sub al, 30H   ;subtract 30H to get character value from ASCII
        
	retn
getValidDigitFromKeyboard  endp
;------------------------------------------------
; drawChar1(word position, word character)
; Draws a single digit at a given location on the screen.
; The character to be drawn and the position are provided as
; parameters on the stack.
; Subroutine uses no BIOS services
; Pre: 	position high byte is row 0>=row>=24D
;		position low byte is column 0>=column>=79D
;		character low byte is value of digit
;		character high byte character attribute
; Post: Video memory changed according to row, column
;-----------------------------------------------
drawChar1 proc near
        push BP
        mov BP,SP
        push ax
        push bx
        push dx
        push es

        ;position cursor is equivalent to finding memory location
        ;so convert row/column to offset from video memory segment
        ;offset = 2*(row*80+column)
        
        mov dx,[bp+4]
        ;assert row (in dx) <40D, 40*160<FFH so result in al
        mov al, 80D
        mul dh         ;dh*al result in ax
        mov dh,0
        add  ax,dx
        shl ax,1       ;mult by 2 (2 bytes for each char)
        mov bx,ax 

        ; write digit is then copy ASCII code and attribute 
        ; value to video memory
        mov dx,[bp+6]
        mov dl,ch
        mov ax,VIDEO_SEG_START
        mov es,ax
        mov es:[bx], dx

        pop es
        pop dx
        pop bx
        pop ax
        pop bp
        ret
drawChar1 endp

drawSquare proc near
;------------------------------------------------
; drawSquare(word position, word character)
; Loops through the calls of drawing the squares of
;side digit long.
; Pre: 	position high byte is row 0>=row>=24D and is top 
;       left corner of square
;		position low byte is column 0>=column>=79D and is top 
;       left corner of square
;		character low byte is value of digit
;		character high byte character attribute
; Post: Video memory changed having a square drawn
;-----------------------------------------------
	push bp
	mov BP,SP
    push ax
    push bx
    push cx
    push dx
    mov dx,[bp+4]
    mov bx, [bp+6]
    STARTLOOP:
    push bx
	push dx
	call drawChar1 ; bx:attribute digit		dx: location to draw row, column
	pop dx
	pop bx
	add dl,1	;increase the column co ordinate by 1
	add ch,1	; increase the count of runs trough this part of loop by 1
	cmp ch, bl  ; compare number of runs trought loop to digit
	jne STARTLOOP ; jump if digit not = number of runs through loop
	sub dl, bl	;sub digit from row result:left most side of square
	add dh, 1	; add one to row
	mov ch, 0	;reset the number of loops in column to 0
	add cl, 1   ;increase the number of loops in row by 1
	mov al, cl	
	add al,1	;add one to number of loops in row
	cmp al , bl ; compare number of loops in row
	jne STARTLOOP  ;jump if digit not = number of loops in row
    pop dx
    pop cx
    pop bx
    pop ax
    pop bp
    ret
drawSquare endp
main endp
end main	 