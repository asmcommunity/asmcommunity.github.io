; prints the value of ax as decimal, hex, and binary ascii
; nasm -f bin -o showax.com showax.asm

org 100h

section .text
    mov ax, 12345

    call ax2dec
    call newline
    call ax2hex
    call newline
    call ax2bin

    mov ah, 4Ch
    int 21h
;--------------

;--------------
ax2dec:
    push ax
    push bx
    push cx
    push dx

    mov bx, 10          ; divide by ten
    xor cx, cx          ; zero our counter
.push_digit:
    xor dx, dx          ; clear dx for the div
    div bx              ; dx:ax/bx -> ax quotient, dx remainder
    push dx             ; save remainder
    inc cx              ; bump digit counter
    or ax, ax           ; is quotient zero?
    jnz .push_digit     ; no, do more

    mov ah, 2           ; print character subfunction
.pop_digit:
    pop dx              ; get remainder back
    add dl, '0'         ; convert to ascii character
    int 21h             ; print it
    loop .pop_digit     ; cx times

    pop dx
    pop cx
    pop bx
    pop ax
    ret
;-------------------

;-------------------
ax2hex:
    push cx
    push dx

    mov cx, 4           ; four digits to show

.top
    rol ax, 4           ; rotate one digit into position
    mov dl, al          ; make a copy to process
    and dl, 0Fh         ; mask off a single (hex) digit
    cmp dl, 9           ; is it in the 'A' to 'F' range?
    jbe .dec_dig        ; no, skip it
    add dl, 7           ; adjust
.dec_dig:
    add dl, 30h         ; convert to character

    push ax
    mov ah, 2           ; print the character
    int 21h
    pop ax

    loop .top

    pop dx
    pop cx
    ret
;--------------------------

;--------------------------
ax2bin:
    push cx
    push dx
    mov cx, 16
.top
    rcl ax, 1     ; rotate and set/clear carry
    mov dl, '0'
    adc dl, 0     ; make it '1' if carry set

    push ax
    mov ah, 2     ; print it
    int 21h
    pop ax

    loop .top

    pop dx
    pop cx
    ret
;----------------------------

;----------------------------
newline:
    push ax
    push dx

    mov ah, 2       ; print character in dl
    mov dl, 13      ; carriage return
    int 21h
    mov dl, 10      ; and linefeed
    int 21h

    pop dx
    pop ax
    ret
;----------------------------


