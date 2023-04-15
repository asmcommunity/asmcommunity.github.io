a
mov dx,b800
mov ds,dx
mov ah,02
mov al,61
mov cl,[bx]
mov ch,[bx+1]
mov [bx+a0],cl
mov [bx+a1],ch
mov [bx],al
mov [bx+1],ah
add bx,1a
cmp bx,0f00
jb 0128
sub bx,0f00
add al,01
cmp al,fd
jb 0130
sub al,dc
add ah,08
cmp ah,0b
jb 013b
sub ah,10
push ax
mov ah,01
int 16
jnz 0145
pop ax
jmp 0109
mov ah,00
int 16
mov ah,01
int 16
jnz 0145
mov ah,07
mov al,20
mov cx,fa0
mov bx,cx
sub bx,02
mov [bx],al
dec cx
dec bx
mov [bx],ah
loop 0156
mov ah,02
mov dx,0000
mov bh,00
int 10
mov al,00
mov ah,4c
int 21
ret

rcx
80
n matrix_3.com
w
q
q
q

