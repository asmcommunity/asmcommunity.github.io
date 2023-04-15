.486                      
.model flat, stdcall      
option casemap :none    

include \masm32\include\windows.inc
include \masm32\include\masm32.inc
include \masm32\include\kernel32.inc
include \masm32\include\user32.inc

includelib \masm32\lib\masm32.lib
includelib \masm32\lib\kernel32.lib
includelib \masm32\lib\user32.lib

HashPassword    PROTO :DWORD, :DWORD


.data
usage       db 'Usage: hashpassword.exe <password>',13,10,0
format      db '%s-%.8x:%.8x:%.8x:%.8x:%.8x:%.8x:%.8x:%.8x',13,10,0

.data?
mypass      db 256 dup (?)
printbuf    db 512 dup (?)
HashBuf     db 32 dup (?)

.code
start:
invoke GetCL,1,addr mypass
invoke lstrlen,addr mypass
.if eax<1
    invoke StdOut,addr usage
    invoke ExitProcess,1
.endif
invoke HashPassword,addr mypass,addr HashBuf
push offset HashBuf
pop esi
push dword ptr [esi+28]
push dword ptr [esi+24]
push dword ptr [esi+20]
push dword ptr [esi+16]
push dword ptr [esi+12]
push dword ptr [esi+8]
push dword ptr [esi+4]
push dword ptr [esi]
push offset mypass
push offset format
push offset printbuf
call wsprintf
invoke StdOut,addr printbuf
 
invoke ExitProcess,0

HashPassword proc inputpassword:DWORD, outputbuffer:DWORD
    LOCAL pwsize:DWORD

    invoke lstrlen,inputpassword
    .if eax==0
        dec eax
        ret
    .endif
    mov pwsize,eax
    xor eax,eax
    push inputpassword
    pop esi
    mov al,byte ptr [esi]
    mov ah,byte ptr [esi]
    bswap eax
    mov al,byte ptr [esi]
    mov ah,byte ptr [esi]
    mov ecx,78277783
    mul ecx
    xor eax,2742303
    mov ecx,20743729
    xor edx,edx
    div ecx
    bswap eax
    mul edx
    push outputbuffer
    pop edi
    mov dword ptr [edi],eax
    mov ecx,10757
    xor edx,edx
    div ecx
    bswap eax
    mul edx
    mov dword ptr [edi+4],eax
    mov ecx,77392473
    mul ecx
    mov cx,word ptr [edi+2]
    xor eax,ecx
    mov dword ptr [edi+8],eax
    bswap eax
    mov ecx,3297329
    mul ecx
    shl eax,3
    mov ecx,579362
    xor edx,edx
    div ecx
    xor ecx,ecx
    mov cl,byte ptr [esi]
    add eax,ecx
    bswap eax
    add eax,edx
    bswap eax
    mov dword ptr [edi+12],eax
    xor eax,24704739
    mov ecx,137
    mul ecx
    xor edx,edx
    bswap eax
    div ecx
    add eax,edx
    mov dword ptr [edi+16],eax
    mov ecx,13362972
    xor edx,edx
    div ecx
    add eax,2739384
    xor eax,edx
    rol eax,3
    sub eax,ecx
    mov ecx,dword ptr [edi+11]
    bswap ecx
    xor eax,ecx
    mov dword ptr [edi+20],eax
    mov ecx,dword ptr [edi+9]
    sub eax,ecx
    mov byte ptr [edi+3],al
    xor byte ptr [edi],ah
    bswap eax
    mov ecx,1337927
    mul ecx
    xchg eax,ecx
    xor edx,edx
    div cx
    add eax,ecx
    mov dword ptr [edi+24],eax
    mov ecx,dword ptr [edi+19]
    xor eax,ecx
    mul edx
    bswap eax
    and eax,208343
    mov dword ptr [edi+28],eax
    mov ecx,dword ptr [edi]
    xor eax,ecx
    bswap eax
    mov dword ptr [edi],eax
    mov ecx,dword ptr [edi+3]
    xor ecx,12237492
    mul ecx
    mov dword ptr [edi+3],eax
    mov ecx,dword ptr [edi+7]
    xchg eax,ecx
    xor edx,edx
    mov ebx,231773
    div ebx
    shl edx,4
    add eax,edx
    xor eax,ecx
    mov dword ptr [edi+7],eax
    mov ecx,dword ptr [edi+27]
    bswap ecx
    mul ecx
    shr ecx,8
    xor edx,edx
    div ecx
    xor ah,dl
    xor al,dh
    mov ecx,379
    mul ecx
    add eax,dword ptr [edi+13]
    xor eax,ebx
    mov dword ptr [edi+12],eax
    mov ebx,dword ptr [edi+25]
    xor eax,ebx
    mov dword ptr [edi+28],eax
    mov ebx,dword ptr [edi]
    xor edx,edx
    mul ebx
    add dword ptr [edi+18],eax
    xor dword ptr [edi+24],eax
    mov ecx,2023855
    mul ecx
    xor dword ptr [edi],eax
    mov ecx,dword ptr [edi+14]
    shl ecx,2
    xor eax,ecx
    mov dword ptr [edi+14],eax
    mov ecx,pwsize
    .if ecx<2
        xor eax,eax
        ret
    .endif
    inc esi
    dec ecx

hashloop:
    xor ebx,ebx
    mov bl,byte ptr [esi]
    mul ebx
    mov ebx,dword ptr [edi]
    xor eax,ebx
    mov dword ptr [edi],eax
    push eax
    mov eax,dword ptr [edi+4]
    mov ebx,3027347
    xor edx,edx
    div ebx
    pop ebx
    add ebx,edx
    xor eax,ebx
    bswap eax
    add dword ptr [edi+4],eax
    sub eax,27239
    mov edx,dword ptr [edi+8]
    bswap edx
    mul edx
    xor eax,edx
    mov edx,dword ptr [edi+10]
    xor eax,edx
    mov dword ptr[edi+8],eax
    mov edx,dword ptr [edi+20]
    xor eax,edx
    mov ebx,2037275
    xor edx,edx
    div ebx
    xor edx,dword ptr [edi+12]
    xor eax,edx
    bswap eax
    shr edx,4
    mul edx
    mov ebx,dword ptr [edi]
    bswap ebx
    xor eax,ebx
    mov dword ptr [edi+12],eax
    mov ebx,dword ptr [edi+12]
    xor eax,ebx
    xor edx,edx
    mov ebx,984379
    div ebx
    mov ebx,dword ptr [edi+9]
    add ebx,edx
    xor eax,ebx
    bswap eax
    sub eax,ecx
    mov ebx,dword ptr [edi+4]
    xor eax,ebx
    mov dword  ptr [edi+16],eax
    mov ebx,dword ptr [edi+20]
    push ebx
    xor eax,ebx
    xor ebx,ebx
    mov bx,word ptr [edi+7]
    xor edx,edx
    div ebx
    pop ebx
    xor ebx,edx
    bswap ebx
    xor eax,ebx
    mov dword ptr [edi+20],eax
    add ebx,dword ptr [edi+24]
    xchg ebx,eax
    mul ecx
    bswap ebx
    mul ecx
    xor eax,ebx
    mov ebx,dword ptr [edi+1]
    xor eax,ebx
    mov dword ptr [edi+24],eax
    sub ebx,dword ptr [edi+28]
    add ebx,edx
    xor edx,edx
    mov dx,word ptr [edi+3]
    mul edx
    bswap eax
    xor eax,ebx
    mov dword ptr [edi+28],eax   
    inc esi
    dec ecx
    jnz hashloop
    xor eax,eax
    ret
HashPassword endp

end start 