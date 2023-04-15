; uses Herbert's "direct to elf executable" code
; nasm -f bin myprog.asm > myprog.map
; chmod +x myprog

; "tune" this so an empty loop gives zero, most often,
; on your machine.
THIS_EMPTY_LOOP equ 384

; define this on the command line to Nasm,
; or uncomment it here, if you prefer...
; REPT_COUNT equ 1 ; 10000

;[map all]
;===========================================================================
bits 32
    ORIGIN equ 8048000h
org ORIGIN
section .text
    code_offset equ 0
    code_addr:
;--------------------------- ELF header -----------------------------------
        dd $464c457f,$00010101,0,0,$00030002,1,main,$34,0,0,$00200034,2,0
        dd 1,code_offset,code_addr,code_addr,code_filez,code_memsz,5,4096
        dd 1,data_offset,data_addr,data_addr,data_filez,data_memsz,6,4096
main:
;--------- your code goes here ------------------------------------------

    xor eax, eax
    cpuid
    rdtsc
    push edx
    push eax

;------------------
; code to time

;times REPT_COUNT mov eax, 0
;times REPT_COUNT xor eax, eax

;times REPT_COUNT dec eax
; times REPT_COUNT sub eax, byte 1


%rep REPT_COUNT
;push eax
;pop eax

;mov eax, [esp]
;mov [esp], eax

;    das
xor eax, eax
aam

%endrep

;-------------------

    xor eax, eax
    cpuid
    rdtsc
    
    pop ebx
    sub eax, ebx
    pop ecx
    sbb edx, ecx
    sub eax, THIS_EMPTY_LOOP
    sbb edx, 0

    mov edi, ascbuf
    call u64toda

; will need a suitable "printstring" and "exit" for 'doze

    mov al, 10       ; terminate buffer with LF
    stosb
    mov ecx, ascbuf
    mov edx, edi
    sub edx, ecx
    mov ebx, 1
    mov eax, 4
    int 80h


exit:
    mov eax, 1
    int 80h
;----------------

;--------------------------------------------------------------
; u64toda - converts (64 bit) integer in edx:eax
; to (comma delimited) decimal representation in
; string in buffer pointed to by edi
; returns edi -> next position in buffer
;-----------------------------------------------------------------
u64toda:
                push eax
                push ebx
                push ecx
                push edx
                push esi

                mov ebx, edx    ; stash high dword
                mov esi, 0Ah    ; prepare to divide by 10
                xor ecx, ecx    ; zero the digit count
                jmp highleft    ; check is high word 0 ?
highword:
                xchg eax, ebx   ; swap high & low words
                xor edx, edx    ; zero edx for the divide!
                div esi         ; divide high word by 10
                xchg eax, ebx   ; swap 'em back
                div esi         ; divide low word including remainder
                push edx        ; remainder is our digit - save it
                inc ecx         ; count digits
highleft:
                test ebx, ebx
                jnz highword
lowleft:
                xor edx, edx     ; zero high word
                div esi          ; divide low word by 10
                push edx         ; our digit
                inc ecx          ; count it
                or eax, eax      ; 0 yet ?
                jne lowleft
                cmp ecx, 4       ; commas needed ?
                jl write2buf     ; nope
                xor edx, edx     ; zero high word for divide
                mov eax, ecx     ; number of digits
                mov ebx, 3
                div ebx
                mov esi, edx     ; remainder = number digits before comma
                test edx, edx
                jnz write2buf    ; no remainder?
                mov esi, 3       ; we can write 3 digits, then.
write2buf:
                pop eax          ; get digit back - in right order
                add al, '0'      ; convert to ascii character
                stosb            ; write it to our buffer
                dec esi          ; digits before comma needed
                jnz moredigits   ; no comma needed yet
                cmp ecx, 2       ; we at the end?
                jl moredigits    ; don't need comma
                mov al, ','      ; write a comma
                stosb
                mov esi, 3       ; we're good for another 3 digits
moredigits:
                loop write2buf   ; write more digits - cx of 'em

                pop esi
                pop edx
                pop ecx
                pop ebx
                pop eax
                ret
;-------------------------------------------------------------



;------------ constant data ---------------------------------
; (note that we're in .text, not .rdata)
        align 4

; we have none

;---------------------------------------------------------------------------
        align 4
        code_memsz equ $ - $$
        code_filez equ code_memsz
        data_addr equ (ORIGIN+code_memsz+4095)/4096*4096 + (code_filez % 4096)
        data_offset equ code_filez
section .data vstart=data_addr
;------------ initialized data ------------------------------

; we have none

;---------------------------------------------------------------------------
    idat_memsz equ $ - $$
    bss_addr equ data_addr + ($ - $$)
section .bss  vstart=bss_addr
;--------------------------- uninitialized data ----------------------------

    ascbuf resb 20h

;---------------------------------------------------------------------------
    udat_memsz equ $ - $$
    data_memsz equ  idat_memsz + udat_memsz
    data_filez equ  idat_memsz
;===========================================================================

