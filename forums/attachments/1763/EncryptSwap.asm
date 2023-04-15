EncryptSwap proc inputfile:DWORD,inputpass:DWORD, inputtable:DWORD,encrypttrue:DWORD
    LOCAL hFile:DWORD
    LOCAL inputfilesize:DWORD
    LOCAL hMap:DWORD
    LOCAL myoffset:DWORD
    LOCAL PassHashBuf[32]:BYTE
    LOCAL lastbytehash:DWORD
    LOCAL lastbytefile:DWORD
    LOCAL hTable:DWORD
    LOCAL tablesize:DWORD
    LOCAL TableBuf[1024*10+3]:BYTE
    LOCAL BytesRead:DWORD
    LOCAL tableoffset:DWORD
    LOCAL number_of_rounds:DWORD
    LOCAL remainder:DWORD
    LOCAL cryptrounds:DWORD
	  

    invoke CreateFile,inputfile,GENERIC_WRITE + GENERIC_READ , 0, NULL, OPEN_EXISTING,\
	    FILE_ATTRIBUTE_NORMAL, NULL
    mov hFile,eax
    invoke GetFileSize,eax,NULL
    mov inputfilesize,eax
    invoke CreateFileMapping,hFile,NULL,PAGE_READWRITE,0,0,NULL
    .if eax==NULL
@@:
        invoke CloseHandle,hFile
        xor eax,eax
        dec eax
        ret
    .endif    
    mov hMap,eax
    invoke MapViewOfFile,hMap,FILE_MAP_WRITE,0,0,0
    .if eax==NULL
        invoke CloseHandle,hMap
        jmp @B
    .endif
    mov myoffset,eax
    invoke CreateFile,inputtable,GENERIC_READ , 0, NULL, OPEN_EXISTING,\
	    FILE_ATTRIBUTE_NORMAL, NULL
    mov hTable,eax
    invoke GetFileSize,eax,NULL
    mov tablesize,eax
    invoke ReadFile,hTable,addr TableBuf,1024*10+3,addr BytesRead,NULL
    invoke CloseHandle,hTable
    invoke HashPassword,inputpass,addr PassHashBuf    
	mov eax,inputfilesize
	xor edx,edx
	mov ebx,8
	div ebx
	mov number_of_rounds,eax
	mov remainder,edx
	mov eax,1024*10+3
	mov ebx,4
	mul ebx
	mov ebx,eax
	mov eax,inputfilesize
	.if eax>ebx
		xor edx,edx
		div ebx
		mov cryptrounds,eax
	.else
		mov cryptrounds,1
	.endif		
    .if encrypttrue==TRUE  
encloop:		
		.if remainder!=0
			push myoffset
			pop esi
			mov edi,esi
			add edi,inputfilesize
			dec edi
			;edi now contains pointer to last byte
			;esi contains pointer to first byte
			mov ecx,remainder
@@:			
			mov al,byte ptr [esi]
			mov ah,byte ptr [edi]
			mov byte ptr [esi],ah
			mov byte ptr [edi],al
			inc esi
			dec edi
			dec ecx
			jnz @B
		.endif	
		
;---------------------------------		
		mov ecx,number_of_rounds
		test ecx,ecx
		jz hashswapenc
		push myoffset
		pop esi		
@@:	
		mov ax,word ptr [esi]
		mov bx,word ptr [esi+2]
		mov dx,word ptr [esi+4]
		mov word ptr [esi+2],ax
		mov word ptr [esi+4],bx
		mov ax,word ptr [esi+6]
		mov word ptr [esi+6],dx
		mov word ptr [esi],ax
		add esi,8
		dec ecx
		jnz @B    
hashswapenc:
;--------------------------------
    	push myoffset
    	pop esi 
    	lea edi,PassHashBuf
		mov ecx,sizeof TableBuf
		sub ecx,3
		lea eax,TableBuf
    	add eax,3
    	mov tableoffset,eax	
@@:
		mov eax,dword ptr [edi]				
		xor ebx,ebx
		mov edx,tableoffset
		mov bl,byte ptr [edx]	
		inc edx
		mov tableoffset,edx
		dec ecx
		jz @F
		mul ebx
		xor edx,edx
		div inputfilesize
		push edx
		mov eax,dword ptr [edi+4]		
		xor ebx,ebx
		mov edx,tableoffset
        mov bl,byte ptr [edx]       
        inc edx
        mov tableoffset,edx
        mul ebx
        xor edx,edx
        div inputfilesize
        xor eax,eax
        pop ebx         
        mov al,byte ptr [esi+edx]
        mov ah,byte ptr [esi+ebx]
        mov byte ptr [esi+edx],ah
        mov byte ptr [esi+ebx],al
        dec ecx
        jz @F
        
		mov eax,dword ptr [edi+8]			
		xor ebx,ebx
		mov edx,tableoffset
		mov bl,byte ptr [edx]	
		inc edx
		mov tableoffset,edx
		dec ecx
		jz @F
		mul ebx
		xor edx,edx
		div inputfilesize
		push edx
		mov eax,dword ptr [edi+12]
		xor ebx,ebx
		mov edx,tableoffset
        mov bl,byte ptr [edx]	
        inc edx
        mov tableoffset,edx
        mul ebx
        xor edx,edx
        div inputfilesize
        xor eax,eax
        pop ebx  
        mov al,byte ptr [esi+edx]
        mov ah,byte ptr [esi+ebx]
        mov byte ptr [esi+edx],ah
        mov byte ptr [esi+ebx],al
        dec ecx
        jz @F
        
		mov eax,dword ptr [edi+16]			
		xor ebx,ebx
		mov edx,tableoffset
		mov bl,byte ptr [edx]
		inc edx
		mov tableoffset,edx
		dec ecx
		jz @F
		mul ebx
		xor edx,edx
		div inputfilesize
		push edx
		mov eax,dword ptr [edi+20]	
		xor ebx,ebx
		mov edx,tableoffset
        mov bl,byte ptr [edx]  
        inc edx
        mov tableoffset,edx
        mul ebx
        xor edx,edx
        div inputfilesize
        xor eax,eax
        pop ebx         
        mov al,byte ptr [esi+edx]
        mov ah,byte ptr [esi+ebx]
        mov byte ptr [esi+edx],ah
        mov byte ptr [esi+ebx],al
        dec ecx
        jz @F
        
		mov eax,dword ptr [edi+24]				
		xor ebx,ebx
		mov edx,tableoffset
		mov bl,byte ptr [edx]	
		inc edx
		mov tableoffset,edx
		dec ecx
		jz @F
		mul ebx
		xor edx,edx
		div inputfilesize
		push edx
		mov eax,dword ptr [edi+28]		
		xor ebx,ebx
		mov edx,tableoffset
        mov bl,byte ptr [edx]	  
        inc edx
        mov tableoffset,edx
        mul ebx
        xor edx,edx
        div inputfilesize
        xor eax,eax
        pop ebx       
        mov al,byte ptr [esi+edx]
        mov ah,byte ptr [esi+ebx]
        mov byte ptr [esi+edx],ah
        mov byte ptr [esi+ebx],al
        dec ecx
        jz @F
        jmp @B    
@@:
		mov eax,cryptrounds
		dec eax
		mov cryptrounds,eax
		jnz encloop
   .else
   		
decloop:   		
    	push myoffset
    	pop esi 
    	lea edi,PassHashBuf
		mov ecx,sizeof TableBuf
		sub ecx,3		
		lea eax,TableBuf
		add eax,sizeof TableBuf
		dec eax
		mov tableoffset,eax
@@:
		mov eax,dword ptr [edi+28]				
		xor ebx,ebx
		mov edx,tableoffset
		mov bl,byte ptr [edx]	
		dec edx
		mov tableoffset,edx
		dec ecx
		jz @F
		mul ebx
		xor edx,edx
		div inputfilesize
		push edx
		mov eax,dword ptr [edi+24]
		xor ebx,ebx
		mov edx,tableoffset
        mov bl,byte ptr [edx]	      
        dec edx
        mov tableoffset,edx
        mul ebx
        xor edx,edx
        div inputfilesize
        xor eax,eax
        pop ebx             
        mov al,byte ptr [esi+edx]
        mov ah,byte ptr [esi+ebx]
        mov byte ptr [esi+edx],ah
        mov byte ptr [esi+ebx],al
        dec ecx
        jz @F
        
		mov eax,dword ptr [edi+20]
		xor ebx,ebx
		mov edx,tableoffset
		mov bl,byte ptr [edx]		
		dec edx
		mov tableoffset,edx
		dec ecx
		jz @F
		mul ebx
		xor edx,edx
		div inputfilesize
		push edx
		mov eax,dword ptr [edi+16]
		xor ebx,ebx
		mov edx,tableoffset
        mov bl,byte ptr [edx]          
        dec edx
        mov tableoffset,edx
        mul ebx
        xor edx,edx
        div inputfilesize
        xor eax,eax
        pop ebx             
        mov al,byte ptr [esi+edx]
        mov ah,byte ptr [esi+ebx]
        mov byte ptr [esi+edx],ah
        mov byte ptr [esi+ebx],al
        dec ecx
        jz @F
        
		mov eax,dword ptr [edi+12]
		xor ebx,ebx
		mov edx,tableoffset
		mov bl,byte ptr [edx]		
		dec edx
		mov tableoffset,edx
		dec ecx
		jz @F
		mul ebx
		xor edx,edx
		div inputfilesize
		push edx
		mov eax,dword ptr [edi+8]
		xor ebx,ebx
		mov edx,tableoffset
        mov bl,byte ptr [edx]	      
        dec edx
        mov tableoffset,edx
        mul ebx
        xor edx,edx
        div inputfilesize
        xor eax,eax
        pop ebx          
        mov al,byte ptr [esi+edx]
        mov ah,byte ptr [esi+ebx]
        mov byte ptr [esi+edx],ah
        mov byte ptr [esi+ebx],al
        dec ecx
        jz @F
        
		mov eax,dword ptr [edi+4]
		xor ebx,ebx
		mov edx,tableoffset
		mov bl,byte ptr [edx]		
		dec edx
		mov tableoffset,edx
		dec ecx
		jz @F
		mul ebx
		xor edx,edx
		div inputfilesize
		push edx
		mov eax,dword ptr [edi]
		xor ebx,ebx
		mov edx,tableoffset
        mov bl,byte ptr [edx]        
        dec edx
        mov tableoffset,edx
        mul ebx
        xor edx,edx
        div inputfilesize
        xor eax,eax
        pop ebx              
        mov al,byte ptr [esi+edx]
        mov ah,byte ptr [esi+ebx]
        mov byte ptr [esi+edx],ah
        mov byte ptr [esi+ebx],al
        dec ecx
        jz @F
        jmp @B
@@:
;--------------------------------
		mov ecx,number_of_rounds	
		test ecx,ecx
		jz hashswapdec	
		push myoffset
		pop esi	
		add esi,inputfilesize
		sub esi,remainder
		sub esi,2
@@:		
		mov ax,word ptr [esi-6]
		mov bx,word ptr [esi]
		mov word ptr [esi],ax
		mov dx,word ptr [esi-2]
		mov word ptr [esi-2],bx
		mov ax,word ptr [esi-4]
		mov word ptr [esi-4],dx
		mov word ptr [esi-6],ax
		sub esi,8
		dec ecx
		jnz @B    
hashswapdec:	
;-------------------------------

		.if remainder!=0
			push myoffset
			pop esi
			mov edi,esi
			add edi,inputfilesize
			dec edi
			;edi now contains pointer to last byte
			;esi contains pointer to first byte
			mov ecx,remainder
@@:			
			mov al,byte ptr [esi]
			mov ah,byte ptr [edi]
			mov byte ptr [esi],ah
			mov byte ptr [edi],al
			inc esi
			dec edi
			dec ecx
			jnz @B
		.endif	
		mov eax,cryptrounds
		dec eax
		mov cryptrounds,eax
		jnz decloop

   .endif

    invoke FlushViewOfFile,myoffset,inputfilesize
    invoke UnmapViewOfFile,myoffset
    invoke CloseHandle,hMap
    invoke CloseHandle,hFile
    xor eax,eax
    ret
EncryptSwap endp