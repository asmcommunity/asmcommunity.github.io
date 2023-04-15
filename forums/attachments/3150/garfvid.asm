;"TB" <NO_acr872k_SPAM@HERE_hotmail.com>
;Wed, 1 Jan 2003 16:31:00 -0500
;alt.lang.asm
org 100h

section .text
    mov ax, 0A000h
    mov es, ax

    mov ax, $4F02            ; 4F02H=SET VESA VIDEO MODE
    mov bx, $0107            ; 107H=1280x1024x256
    int $10                  ; VIDEO INTERRUPT

    call set_palette

    xor ecx, ecx
    mov bh, 1 ; cl ; $0F

LoopBG:
    inc ecx
    mov eax,ecx
    call pp2 ; PutPixel
    cmp ecx, 1280 * 1024   ;0FFFFh
    jne LoopBG

    mov ah, 10h
    int 16h
    cmp al, 1Bh
    jz egress

    xor ecx, ecx
;    cmp bh, 4
;    jnz top

;    mov bh, 3
    inc bh
    jmp short LoopBG

egress:
;    mov ah, 0
;    int 16h
    mov ax, 3
    int 10h
    ret


PutPixel:                    ; EAX=LONG ADDRESS BH=COLOR

    push eax ; push used register
    push ecx ; push argument
    push edx ; push used register

    push ebx ; push used register

    xor ebx, ebx ; clear extended part of ebx
    mov bx, $0FFFF ; divisor

    xor edx,edx  ; clear extended part of edx

    div ebx         ; divide: remainder (local bank addr) in edx and
                    ; quotient (bank number) in eax
    xchg edx, eax  ; exchange them because SwitchBank needs bank number in
                    ; edx
    call SwitchBank ; switch bank, bank number=dx

    mov di,ax  ; prepare offset
    mov ax,$0a000       ; prepare segment
    mov es,ax  ; set segment
    pop ebx  ; pop argument
    mov [es:di], byte bh ; draw!

    pop edx
    pop ecx
    pop eax

    ret

pp2:
    push eax
    pop ax
    pop dx
    mov di, ax
    mov al, bh
    cmp [cur_bank], dx
    jz bank_okay
    mov [cur_bank], dx
    call SwitchBank
bank_okay:
    stosb
    ret


SwitchBank:
    push ax
    push bx
    mov ax,4f05h
    xor bx,bx
    int $10
    pop bx
    pop ax
    ret

;---------------
set_palette:
    mov si, my_pal
    xor cx, cx
.top:
    mov dx, 3c8h
    mov al, cl
    out dx, al
    inc dx
    lodsb
    shr al, 2
    out dx, al
    lodsb
    shr al, 2
    out dx, al
    lodsb
    shr al, 2
    out dx, al
    inc cx
    cmp cx, 100h
    jnz .top
    ret

;--------------
section .data
    cur_bank dw 0

my_pal:
    db 0, 0 ,0
    db 0, 255, 0
    db 0, 0, 255
    db 255, 0, 0
    db 128, 128, 0
    db 128, 0, 128
    db 0, 128, 128
    db 128, 0, 0
    db 0, 128, 0
    db 0, 0, 128
    db 255, 128, 0
    db 255, 0, 128
    db 255, 128, 128
    db 128, 255, 128
    db 128, 128, 255
    db 240, 240, 0
    db 240, 0, 240
    db 240, 100, 240
    db 240, 100, 64
    db 240, 0, 64
    db 230, 64, 64
    db 230, 128, 64
    db 230, 64, 128
    db 230, 230, 64
    db 230, 64, 230
    db 230, 180, 64
    db 230, 64, 180
    db 230, 0, 180
    db 230, 0, 64
    db 230, 255, 0
    db 220, 255, 64
    db 200, 255, 64
    db 155, 255, 0
    db 128, 255, 64
    db 100, 255, 128
    db 180, 0, 0
    db 220, 220, 64
    db 180, 220, 0
    db 180, 255, 128
    db 180, 0, 128
    db 180, 128, 64
    db 240, 0, 128
    db 240, 128, 0
    db 220, 240, 160
    db 255, 140, 120
    db 220, 128, 255
    db 200, 64, 255
    db 200, 255, 64
    db 240, 255, 0
    db 100, 0, 255
    db 64, 64, 240
    db 255, 0, 140
    db 220, 180, 100
    db 240, 200, 180
    db 64, 255, 255
    db 100, 255, 100
    db 100, 100, 255
    db 230, 200, 0
    db 180, 255, 64
    db 0, 255, 240
    db 200, 255, 180
    db 64, 0, 0
    db 200, 0, 100
    db 0, 220, 100
    db 0, 64, 0
    db 64, 240, 64
    db 64, 255, 64
    db 0, 64, 100
    db 220, 120, 120
    db 64, 255, 255
    db 255, 240, 0
    db 0, 240, 30
    db 128, 255, 64
    db 240, 0, 128
    db 255, 120, 255
    db 120, 0, 255
    db 64, 128, 255
    db 100, 0, 64
    db 0, 64, 128
    db 220, 55, 155
    db 64, 255, 220
    db 0, 230, 200
    db 128, 64, 255
    db 240, 64, 0
    db 128, 64, 255
    db 0, 64, 85
    db 80, 100, 0
    db 220, 200, 60
    db 160, 64, 40
    db 100, 240, 0
    db 100, 64, 255
    db 40, 100, 40
    db 220, 40, 255
    db 60, 155, 55
    db 205, 55, 105
    db 155, 85, 55
    db 85, 85, 135
    db 175, 155, 75
    db 85, 45, 55
    db 205, 55, 25
    db 25, 155, 185
    db 55, 185, 135
    db 205, 25, 25
    db 195, 55, 35
    db 25, 205, 155
    db 85, 55, 145
    db 125, 55, 155
    db 55, 155, 25
    db 225, 25, 25
    db 205, 125, 55
    db 25, 215, 55
    db 25, 225, 25
    db 55, 25, 155
    db 100, 55, 205
    db 0, 0, 255
    db 0, 0, 245
    db 0, 0, 235
    db 0, 0, 225
    db 0, 0, 215
    db 0, 0, 205
    db 0, 0, 195
    db 0, 0, 185
    db 0, 0, 175
    db 0, 0, 165
    db 0, 0, 155
    db 0, 0, 145
    db 0, 0, 135
    db 0, 0, 125
    db 0, 0, 115
    db 0, 0, 105
    db 0, 0, 95
    db 0, 0, 85
    db 0, 0, 75
    db 0, 0, 65
    db 0, 0, 55
    db 0, 0, 45
    db 0, 0, 35
    db 0, 0, 25
    db 0, 255, 0
    db 0, 245, 0
    db 0, 235, 0
    db 0, 225, 0
    db 0, 215, 0
    db 0, 205, 0
    db 0, 195, 0
    db 0, 185, 0
    db 0, 175, 0
    db 0, 165, 0
    db 0, 155, 0
    db 0, 145, 0
    db 0, 135, 0
    db 0, 125, 0
    db 0, 115, 0
    db 0, 105, 0
    db 0, 95, 0
    db 0, 85, 0
    db 0, 75, 0
    db 0, 65, 0
    db 0, 55, 0
    db 0, 45, 0
    db 0, 35, 0
    db 0, 25, 0
    db 255, 0, 0
    db 245, 0, 0
    db 235, 0, 0
    db 225, 0, 0
    db 215, 0, 0
    db 205, 0, 0
    db 195, 0, 0
    db 185, 0, 0
    db 175, 0, 0
    db 165, 0, 0
    db 155, 0, 0
    db 145, 0, 0
    db 135, 0, 0
    db 125, 0, 0
    db 115, 0, 0
    db 105, 0, 0
    db 95, 0, 0
    db 85, 0, 0
    db 75, 0, 0
    db 65, 0, 0
    db 55, 0, 0
    db 45, 0, 0
    db 35, 0, 0
    db 25, 0, 0
    db 25, 25, 25
    db 45, 35, 35
    db 65, 45, 45
    db 85, 55, 55
    db 105, 65, 65
    db 125, 75, 75
    db 145, 85, 85
    db 165, 95, 95
    db 185, 105, 105
    db 205, 115, 115
    db 225, 125, 125
    db 245, 135, 135
    db 205, 145, 145
    db 165, 155, 155
    db 125, 165, 165
    db 85, 175, 175
    db 75, 185, 165
    db 65, 195, 155
    db 55, 205, 145
    db 45, 215, 135
    db 35, 225, 125
    db 25, 235, 115
    db 15, 245, 105
    db 25, 255, 95
    db 25, 255, 85
    db 25, 255, 75
    db 25, 255, 65
    db 25, 255, 55
    db 25, 255, 45
    db 25, 255, 35
    db 35, 255, 25
    db 45, 245, 35
    db 55, 235, 45
    db 65, 225, 55
    db 75, 215, 65
    db 85, 205, 75
    db 95, 195, 85
    db 105, 185, 95
    db 115, 175, 85
    db 125, 165, 75
    db 135, 155, 65
    db 145, 145, 55
    db 155, 135, 45
    db 165, 125, 35
    db 175, 115, 25
    db 185, 105, 25
    db 195, 95, 25
    db 205, 85, 25
    db 215, 75, 25
    db 225, 65, 25
    db 235, 55, 25
    db 245, 45, 25
    db 255, 35, 25
    db 255, 25, 35
    db 255, 25, 45
    db 255, 25, 55
    db 255, 25, 65
    db 255, 25, 75
    db 255, 25, 85
    db 255, 25, 95
    db 255, 25, 105
    db 255, 25, 125
    db 255, 25, 135
    db 255, 25, 145
    db 255, 25, 155
    db 255, 25, 175
    db 225, 25, 195
    db 205, 25, 225
    db 165, 25, 245
    db 255, 255, 255
