include a2fp.inc
include fp2a.inc

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fExp2
; Purpose:   Computes 2^x = 2^st0.
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fExp2 macro                       ;;st0      st1      st2
    fld st(0)                     ;;x        x        ?         Duplicate st0
    frndint                       ;;int(x)   x        ?         Round to integer
    fsub st(1), st(0)             ;;int(x)   frac(x)  ?         Compute fractional part of x
    fld1                          ;;1        int(x)   frac(x)   Load 1
    fscale                        ;;2^int(x) int(x)   frac(x)   Scale 1 by int(x)
    fxch                          ;;int(x)   2^int(x) frac(x)   Exchange st0 and st1
    fstp st(0)                    ;;2^int(x) frac(x)  ?         Eliminate st0 and pop
    fxch                          ;;frac(x)  2^int(x) ?         Exchange st0 and st1
    f2xm1                         ;;2^frac-1 2^int(x) ?         Compute 2xm1
    fld1                          ;;1        2^frac-1 2^int(x)  Load 1
    faddp st(1), st(0)            ;;2^frac   2^int(x) ?         Add st0 and st1 and pop
    fmulp st(1), st(0)            ;;2^x      ?        ?         Multiplay st1 by st0 and pop
endm
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     fPower
; Purpose:   Computes st0 = st0^st1 (x^y).
; Arguments: None.
; Returns:   Nothing.
; Note:      st5, st6 and st7 must be empty.

fPower macro
    fld1                          ;;Put 1 onto the stack
    fxch                          ;;Swap st0 and st1
    fyl2x                         ;;Compute 1*Log2(x)
    fmulp st(1),st(0)             ;;Compute y*Log2(x)
    fExp2                         ;;Comput2 2^(x*Log2(y)) = x^y
endm

a2dw proc uses ecx edi edx esi pString:DWORD
      ;----------------------------------------
      ; Convert decimal string into dword value
      ; return value in eax
      ;----------------------------------------
      mov edi, pString
      invoke lstrlen, pString
      xor ecx, ecx
      .while eax != 0
        xor edx, edx
        mov dl, byte ptr [edi]
        sub dl, "0" ; subtract each digit with "0" to convert it to hex value
        mov esi, eax
        dec esi
        push eax
        mov eax, edx
        push ebx
        mov ebx, 10
          .while esi > 0
            mul ebx
            dec esi
          .endw
        pop ebx
        add ecx, eax
        pop eax
        inc edi
        dec eax
      .endw
      mov eax, ecx
	ret
a2dw endp

h2dw proc uses ebx esi edi pString:DWORD

  ; -----------------------------------
  ; Convert hex string into dword value
  ; Return value in eax
  ; -----------------------------------
    mov edx,pString
    .if word ptr[edx]=='x0'
    	add pString,2
    .endif

    mov edi, pString
    mov esi, pString 

     ALIGN 4

  again:  
    mov al,[edi]
    inc edi
    or  al,al
    jnz again
    sub esi,edi
    xor ebx,ebx
    add edi,esi
    xor edx,edx
    not esi             ;esi = lenth

  .while esi != 0
    mov al, [edi]
    cmp al,'A'
    jb figure
    sub al,'A'-10
    adc dl,0
    shl dl,5            ;if cf set we get it bl 20h else - 0
    add al,dl
    jmp next
  figure: 
    sub al,'0'
  next:  
    lea ecx,[esi-1]
    and eax, 0Fh
    shl ecx,2           ;mul ecx by log 16(2)
    shl eax,cl          ;eax * 2^ecx
    add ebx, eax
    inc edi
    dec esi
  .endw

    mov eax,ebx
    ret

h2dw endp

dw2h proc source:DWORD, lpBuffer:DWORD

    push esi

    mov edx, lpBuffer
    mov esi, source
    
    mov word ptr[edx],"x0"	; mark string as Hex
    add edx,2

    xor eax, eax
    xor ecx, ecx

    mov [edx+8], al         ; put terminator at correct length
    mov cl, 7               ; length of hexstring - 1

  @@:
    mov eax, esi            ; we're going to work on AL
    and al, 00001111b       ; mask out high nibble

    cmp al,10
    sbb al,69h
    das

    mov [edx + ecx], al     ; store the asciihex(AL) in the string
    shr esi, 4              ; next nibble
    dec ecx                 ; decrease counter (one byte less than dec cl :-)
    jns @B                  ; eat them if there's any more

    pop esi

    ret

dw2h endp


dw2a proc dwValue:DWORD, lpBuffer:DWORD

    ; -------------------------------------------------------------
    ; convert DWORD to ascii string
    ; dwValue is passed as a value, direct, indirect or in register
    ; lpBuffer is the ADDRESS of the receiving buffer
    ; EXAMPLE:
    ; invoke dw2a,edx,ADDR buffer
    ; -------------------------------------------------------------
    .data
    szlu db "%lu",0
    .code
    invoke wsprintf,lpBuffer,ADDR szlu,dwValue

    ret

dw2a endp


;This proc determines whether a given Term is Hex.
;Accepts std asm format (ends with h or H ie C0B6h)
;Also accepts std C format (begins with 0x ie 0xC0B6)
IsTermHex proc pTerm
LOCAL ProllyHex
	DbgText "check for Hex"
	mov ProllyHex,FALSE
	mov eax,pTerm
	;Check for C-style hex string
	.if word ptr[eax]=="x0"
		mov ProllyHex,TRUE
		add eax,2
	.endif
	;Verify that the rest of the string is valid hex
	.while byte ptr[eax]!=0
		;If we reach the last character, and its h or H, 
		;then the Term is HEX
		.if byte ptr[eax+1]==0 
			.if ProllyHex==FALSE ;No Prefix - Check for Suffix
				.if (byte ptr[eax]=="h" || byte ptr[eax]=="H")
					;Ensure theres atleast one byte of value data
					mov edx,eax
					sub edx,pTerm
					.if edx!=0
						mov byte ptr[eax],0	;lose the suffix
						return TRUE
					.endif
				.endif
			.else
				return TRUE		;Prefixed - we're golden
			.endif
		;If we are NOT at the last character yet,
		;and the character is legal HEX,
		;then continue
		.elseif (byte ptr[eax]>='0' && byte ptr[eax]<='9') ||\
				(byte ptr[eax]>="a" && byte ptr[eax]<="f") ||\
				(byte ptr[eax]>="A" && byte ptr[eax]<="F")
			inc eax
			.continue
		.endif
		.break
	.endw
	return FALSE
IsTermHex endp

;This proc determines whether a given Term is Float.
;Accepts std asm format (ends with f or F ie 0.75f or .75f)
;Also accepts variant format (no trailing char ie 0.75 or .75)
IsTermFloat proc pTerm
LOCAL ProllyFloat
	DbgText "check for Float"
	mov ProllyFloat,FALSE
	mov eax,pTerm
	.while byte ptr[eax]!=0
		;If we reach the last character, and its f or F, 
		;then the Term is FLOAT
		.if byte ptr[eax+1]==0 
			.if (byte ptr[eax]=="f" || byte ptr[eax]=="F") || ProllyFloat==TRUE
				;Ensure atleast one byte of value data
				sub eax,pTerm
				.if eax!=0
					return TRUE
				.endif
			.endif
		;If we are NOT at the last character yet,
		;and the character is legal (numeric),
		;then continue
		.elseif (byte ptr[eax]>='0' && byte ptr[eax]<='9') ||\
				byte ptr[eax]=="."
			.if byte ptr[eax]=="."
				mov ProllyFloat,TRUE
			.endif
			inc eax
			.continue
		.endif
		.break
	.endw
	return FALSE
IsTermFloat endp

;This proc determines whether a given Term is Binary.
;Accepts std asm format (ends with b or B ie 10010010b)
;Also accepts variant format (starts with % ie %10010010)
IsTermBinary proc pTerm
LOCAL ProllyBin
	DbgText "check for Bin"
	mov ProllyBin,FALSE	
	mov eax,pTerm
	.if byte ptr[eax]=='%'	;Handle special case
		mov ProllyBin,TRUE
		inc eax
	.endif
	.while byte ptr[eax]!=0
		;If we reach the last character, and its b or B, 
		;then the Term is BINARY
		.if byte ptr[eax+1]==0 
			.if ProllyBin==FALSE
				.if (byte ptr[eax]=="b" || byte ptr[eax]=="B")
					;Make sure theres at least one byte of value data
					sub eax,pTerm
					.if eax!=0
						return TRUE
					.endif
				.endif
			.else
				return TRUE
			.endif
		;If we are NOT at the last character yet,
		;and the character is legal (numeric),
		;then continue
		.elseif (byte ptr[eax]=='0' || byte ptr[eax]=='1')
			inc eax
			.continue
		.endif
		.break
	.endw
	return FALSE
IsTermBinary endp

;This method determines whether a given Term is Integer.
;Accepts ONLY pure numerical values (eg 31337)
IsTermInteger proc pTerm
	DbgText "check for Integer"
	mov eax,pTerm
	.while byte ptr[eax]!=0
		.if byte ptr[eax]<'0' || byte ptr[eax]>'9'
			return FALSE
		.endif
		inc eax
	.endw
	return TRUE
IsTermInteger endp

IsTermAlpha proc pTerm
	DbgText "check for Alpha"
	mov eax,pTerm
	.while byte ptr[eax]!=0
		.if (byte ptr[eax]>='a' && byte ptr[eax]<='z') ||\
			(byte ptr[eax]>='A' && byte ptr[eax]<='Z')
			return TRUE
		.endif
		inc eax
	.endw
	return FALSE
IsTermAlpha endp


$StrNew macro pString
	invoke lstrlen,pString
	.if eax!=0
		inc eax
		MemAlloc eax
		push eax
		invoke lstrcpy,eax,pString
		pop eax
	.else
		DbgWarning "Error : Attempt to duplicate String of Zero Length"
	.endif
	exitm <eax>
endm