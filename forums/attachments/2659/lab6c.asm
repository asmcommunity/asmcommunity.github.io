.model small
.stack 100h
.data


count	DW 0
year	DW 0
input 	DB ?,?,?,?

prompt	DB "Enter a four digit number for year: ",0dh,0ah,'$'

space	DB 0dh,0ah
msg		DB "The result is:     "
r2d2	DB ?,?,?,?
		DB 0dh,0ah,'$'

.code
main proc

	mov ax,@data
	mov ds,ax
	
	;display prompt
	mov ah,9
	mov dx,offset prompt
	int 21h
	
	mov si,offset input
	
	;enter values for input
L1:	mov ah,01h
	int 21h
	CMP al,0dh
	JE L2
	mov [si],al
	inc si
	inc count
	Loop L1
	
L2:	mov cx,count
	mov si,offset input
	mov bx,10
	mov ax,0
	mov dx,0
	
	;convert input to decimal
L3: mul bx
	add al,[si]
	sub al,30h
	inc si
	Loop L3
	
	mov year,ax
	inc year
	
	;perform calculation
	mov ax,year
	mov bx,3			
	mul bx
	mov bx,7
	div bx
	mov ax,0
	add ax,36
	add ax,year
	sub ax,2000
	add ax,dx
	
	mov cx,count
	mov bl,10
	mov si,offset r2d2
	add si,count
	sub si,1
	
	;convert answer to decimal
L4: mov dx,0
	div bx
	add dx,30h
	mov [si],dx
	dec si
	dec si
	Loop L4
	
	mov si,offset r2d2
	
	;remove leading zeros
L5: mov ax,[si]
	CMP ax,0
	JG L6
	mov [si],21h
	inc si
	JMP L5
	
	;display result
L6:	mov ah,9
	mov dx,offset space
	int 21h
	
	mov ax,4c00h
	int 21h
	
main endp
end main
	