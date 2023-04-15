;EXPERIMENT 3
;MICRO-LAB TEACHER: DE SOUZA
;STUDENTS: JESSE N. CHANDLER & Tyler Reynolds
; multi-segment executable file template.

data segment
LLength db 00h,00h,00h,00h,00h ;allocating free memory
WWidth db 00h,00h,00h,00h,00h
HHeight db 00h,00h,00h,00h,00h
VVolume db 00h,00h,00h,00h,00h,"$"

prompt_1 db 0dh,0Ah,'Enter the length in meters: ',"$"
prompt_2 db 0dh,0Ah,'Enter the width in meters: ',"$"
prompt_3 db 0dh,0Ah,'Enter the height in meters: ',"$"
prompt_4 db 0dh,0ah, 'The volume is:',"$"

temp db 00h
factor db 03h
multiplicand db 0ah
ends

stack segment
    dw   128  dup(0)
ends

code segment

main proc far
    
                                    ; set segment registers:
    mov ax, data
    mov ds, ax
    mov es, ax

           
    xor ax,ax                       ; xor anything with itself sets self to 0
    xor bx,bx                       ; therefore this is an initialization
    xor dx,dx 
    cld                             ;sets autoincrement mode                         

do_again:
    
    mov ah, 09h
    mov DX, offset prompt_1         ;show string 1
    int 21h
    mov dx, offset LLength 
    mov DI, dx             
    
    call char_get
    mov dx, offset LLength 
    call conv_asc2bin
    
    mov ah, 09h
    mov DX, offset prompt_2         ;show string 2
    int 21h
    mov dx, offset WWidth           ;stores location of variable
    mov DI, dx                      ;moves address to ds since this can't 
                                    ;be done directly
    
    call char_get
    mov dx, offset WWidth 
    call conv_asc2bin
    
    mov ah, 09
    mov DX, offset prompt_3         ;show string 3
    int 21h
    mov dx, offset HHeight
    mov DI, dx
     
    call char_get
    mov dx, offset HHeight 
    call conv_asc2bin
    
    call Volume
    mov dx, offset VVolume
    call conv_bin2asc
    
    mov ah,09h
    mov dx,offset VVolume
    int 21h
    
    mov di,offset LLength
    mov cx,19
    xor ax,ax
    rep stosb
    
    loop do_again
    

ret     
main endp


;   place subroutines below the main function just as in c++
;****************************************************************
;----------------------------------------------------------------
;
;----------------------------------------------------------------
;****************************************************************


Volume proc near
;calculates the volume by updating al using following commands
;still not correct we need to find a way to operate in decimal        
        
        
        mov al, LLength
        xor ah,ah
        mul WWidth
        mul HHeight
        div factor
        mov di, offset VVolume
        stosb 
        mov ah,09h
        MOV Dx,offset prompt_4
        int 21H
 
    ret   
Volume endp




char_get proc near
;procedure to read the designated size of data 


    mov cx,5                   
until_done:                ;sets the label to loop until user presses enter

    
    mov ah, 00H
    int 16h                ;get a key, return in scan code in Ah
        

    cmp ah,10h             ;TEST FOR Q
    jz finish_1
    cmp ah,1CH             ;TEST FOR ENTER
        
    jz finish_2
    
    mov ah,06h
    MOV DL,AL
    int 21h   
    stosb
                
    loop until_done
    jmp finish_2 
    
finish_1:

        mov ah, 07          ;PARAMETERS TO CLEAR SCREEN
        mov al, 0
        mov bh, 07
        mov cx, 0
        mov dh, 24
        mov dl, 79
        int 10h
        mov ah, 00h         ;exit to program operating system.
        int 21h             ;user interface interrupt
                                             
    
finish_2: 
mov al,0dh
stosb

ret
char_get endp


conv_asc2bin proc near
;using this function requires a starting address value to be placed in dx
;must use this function after the variable has been written to mem

;the most effective way to use this function would be to place after variable has been received.
;however some of concepts of this function will need to be reproduced in the volume function to 
;correctly indicate the result.
    cld
two2one:
    mov di, dx              ;fills the contents of al with byte size 
                            ;peices of data that the user entered
                             
    mov cx,05               ;stops if scasb repeats greater than 5 times
    mov al,0dh
    repne scasb             ;auto increments di
    
    sub di,01h              ;scasb increments di too much
    
    sub di,dx               ;because dx contains address of variable
                            ; we want to know the displacement from the start
                            ;of the variable.
                            
                            
    mov cx,di               ;stores the number of values entered by user
    mov bx,dx
    mov di,dx               ;re-initializes di to address of variable
                            
place_value:                ;using cx we know the number of digits the user input
    cmp cx,01
    jz ones_place
    cmp cx,02
    jz tens_place
 
    ones_place:
    mov al, [bx]
    sub al,30h
    mov [di],al
    jmp finish_3
    
    tens_place:
    mov ax,[bx]
    sub ah,30h
    mov offset temp, ah
    sub al,30h
    mul multiplicand
    add al, temp
    xor ah,ah
    mov [di],ax
    inc di
    dec cx
    jmp finish_3           
    
    finish_3:
    loop place_value

    
ret    
conv_asc2bin endp

ends

conv_bin2asc proc near

mov di,dx
mov bx,dx
mov ax,[bx]
cmp ax, 0ah
jge double_digit
jmp single_digit

double_digit:
div multiplicand
add al,30h
add ah,30h
stosw
jmp finish_final



single_digit:
add al,30h
stosb
jmp finish_final

finish_final:


ret    
conv_bin2asc endp


end main ;set entry point and stop assembler
