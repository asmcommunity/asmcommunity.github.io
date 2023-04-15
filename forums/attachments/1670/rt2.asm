.686
.model flat,stdcall
option casemap:none

include windows.inc
include kernel32.inc
include user32.inc

includelib kernel32.lib
includelib user32.lib

GenerateTableFile PROTO :DWORD
prand			  PROTO :DWORD
pseed			  PROTO :DWORD,:DWORD,:DWORD,:DWORD

.data
AppName 			db "Random tester",0
TableCreateSuccess	db "Table created successfully!",0
NoCreate			db "Unable to create table!",0
MyFileName			db "file.dat",0


.code

start:

	invoke GenerateTableFile,addr MyFileName
	invoke ExitProcess,0




GenerateTableFile PROC FileName:DWORD
    LOCAL hFile:DWORD
    LOCAL BytesWr:DWORD
    LOCAL TableBuf[1024]:BYTE    
    
;---------------------------------------  
	LOCAL count:DWORD 
    invoke CreateFile,FileName,GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,\
		FILE_ATTRIBUTE_NORMAL, NULL
	mov hFile,eax
	mov count,1024
	
	invoke pseed,0AAAABBBBh,0CCCCDDDDh,0EEEEFFFFh,11112222h
loop1000:	
    lea esi,TableBuf 
    mov ecx,esi
    add ecx,sizeof TableBuf
    @@:
    push ecx
    invoke prand,256
    mov byte ptr [esi],al
    inc esi
    pop ecx
    cmp esi,ecx
    jle @B
	invoke WriteFile,hFile,addr TableBuf,sizeof TableBuf,addr BytesWr,NULL
	mov eax,count
	dec eax
	mov count,eax
	test eax,eax
	jnz loop1000

    invoke CloseHandle,hFile
    invoke MessageBoxA,NULL,addr TableCreateSuccess,addr AppName,MB_OK
    ret
GenerateTableFile ENDP

pseed PROC s1:DWORD,s2:DWORD,s3:DWORD,s4:DWORD
	.data
	seed1  dd 0AAAABBBBh
	seed2  dd 0CCCCDDDDh
	seed3  dd 0EEEEFFFFh
	seed4  dd 11112222h
	
	.code
	mov eax,s1 ;if s1 = 0 then use default value
	.if eax!=0
		mov seed1,eax
	.endif
	mov eax,s2 ;if s2 = 0 then use default value
	.if eax!=0
		mov seed2,eax
	.endif
	mov eax,s3 ;if s3 = 0 then use default value
	.if eax!=0
		mov seed3,eax
	.endif
	mov eax,s4 ;if s4 = 0 then use default value
	.if eax!=0
		mov seed4,eax
	.endif
	ret

pseed ENDP

prand PROC base:DWORD
	;seed1 = AAAABBBB
	;seed2 = CCCCDDDD
	;seed3 = EEEEFFFF
	;seed4 = 11112222
	
	mov eax,seed1 ;AAAABBBB
	mov ebx,seed2 ;CCCCDDDD
	mov ecx,seed3 ;EEEEFFFF
	mov edx,seed4 ;11112222
;start shifting	
	xchg ax,bx    ;eax = AAAADDDD, ebx = CCCCBBBB
	xchg cx,dx	  ;ecx = EEEE2222, edx = 1111FFFF	
	xchg al,cl	  ;eax = AAAADD22, ecx = EEEE22DD
	xchg bl,dl	  ;ebx = CCCCBBFF, edx = 1111FFBB
	push eax	  ;AAAADD22
	push ecx      ;EEEE22DD
	shl eax,8	  ;AADD2200
	shr ecx,24	  ;000000EE
	add eax,ecx   ;AADD22EE
	mov seed1,eax	  ;s1 = AADD22EE
	pop ecx		  ;EEEE22DD
	pop eax 	  ;AAAADD22
	push ecx	  ;EEEE22DD
	shr eax,24	  ;000000AA
	push edx	  ;1111FFBB
	shl edx,8	  ;11FFBB00
	add edx,eax	  ;11FFBBAA
	mov seed2,edx    ;s2 = 11FFBBAA
	pop edx		  ;1111FFBB
	shr edx,24	  ;00000011
	push ebx	  ;CCCCBBFF
	shl ebx,8	  ;CCBBFF11
	mov seed3,ebx	  ;s3 = CCBBFF11
	pop ebx		  ;CCCCBBFF
	shr ebx,24	  ;000000CC
	pop ecx		  ;EEEE22DD
	shl ecx,8	  ;EE22DD00
	add ecx,ebx   ;EE22DDCC
	mov seed4,ecx    ;s4 = EE22DDCC
;start calculating
	mov eax,seed1
	mov ecx,137773
	xor edx,edx
	div ecx
	mov ebx,seed2
	add eax,ebx
	xor eax,edx
	mul edx
	mov seed2,eax
	mov ebx,seed1
	xor eax,ebx
	sub ebx,30721
	xor edx,edx
	div ebx
	add eax,seed3
	add ebx,edx
	xor edx,edx
	div ebx
	add eax,seed1
	mov ebx,20823
	mul ebx
	mov seed1,eax
	mov ebx,seed4
	xor ebx,eax
	add eax,1366
	mov ecx,37783
	xor edx,edx
	div ecx
	add ebx,edx
	add ebx,eax
	mov seed3,ebx
;use every last byte of each new seed
	mov eax,seed1
	shl eax,24
	mov ebx,seed2
	shl ebx,24
	shr ebx,8
	add eax,ebx
	mov ebx,seed3
	shl ebx,24
	shr ebx,16
	add eax,ebx
	mov ebx,seed4
	add al,bl	
	xor edx,edx
	div base	
    mov eax,edx
    ret

prand ENDP

end start