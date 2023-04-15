;Super Random Number Generator
;Written by Homer in December, 2008
;Based on code by Agner Fog and NaN

;This object is Contextual...
;That makes it suitable for ENCRYPTION purposes!!

;Modified April 2009
;Added various Access Methods + fpu support

;=======================================================

; define constants for MT11213A:
N       = 351
M       = 175
R       = 19
MATRIX_A = 0E4BD75F5H
TEMU    = 11
TEMS    = 7
TEMT    = 15
TEML    = 17
TEMB    = 655E5280H
TEMC    = 0FFD58000H

LOWER_MASK = (1 SHL R) - 1     ; lower R bits
UPPER_MASK = -1 SHL R          ; upper 32-R bits
;=======================================================


RID equ 18273645
Object RNG, RID, Primer
    RedefineMethod Init, dword
    VirtualMethod  Get                      ;Random 32bit binary
    VirtualMethod  Int_Ranged,dword,dword   ;Integer in Range dMin thru dMax
    VirtualMethod  Real_Ranged,real4,real4  ;Float in Range fMin thru fMax
    VirtualMethod  Real_Signed_Normal       ;Float in Range -1.0 thru +1.0
    VirtualMethod  Real_Unsigned_Normal     ;Float in Range 0.0 thru +1.0
    Private
        VirtualMethod  Mersenne         ;Random 32bit from Mersenne Twister
    PrivateEnd
    DefineVariable seed, dword
    DefineVariable MT, dword, N dup <(?)>   ; history buffer
    DefineVariable MTI,dword                ; index into MT buffer
    DefineVariable PENDINGX,QWORD,0
    DefineVariable ReseedAt,byte,0
    DefineVariable PENDXOK,byte,FALSE
ObjectEnd

;=======================================================

.data
TEMP    DQ      0               ; dual purpose: conversion to float and MT buffer overrun
LMASK   DD      LOWER_MASK      ; constants
UMASK   DD      UPPER_MASK
MATA    DD      MATRIX_A
TMB     LABEL   QWORD           ; constants
        DD      TEMB, TEMB
TMC     LABEL   QWORD
        DD      TEMC, TEMC
USE_MMX db      -1

.code
;===================================================

;Given an INITIAL RANDOM SEED, this method initializes
;the internal table used by the Mersenne Twister algorithm,
;and a counter used to periodically generate calls to this method,
;which forces the Mersenne table to be RESEEDED.
Method RNG.Init, uses esi ecx, Seed
    SetObject esi
       finit
       
       ;Set the random seed
       MOV EAX, Seed
       mov [esi].seed,eax
       
       XOR     ECX, ECX

@@:    IMUL    EAX, 29943829        ; make random numbers and put them into buffer
       DEC     EAX
       MOV     [esi].MT[ECX], EAX        
       ADD     ECX, 4
       CMP     ECX, N*4
       JB      @B       
       MOV     [esi].MTI, ECX

       ;check microprocessor support for MMX instructions, but only do it once      
       ;Only perform 
       .if USE_MMX == -1
    
           PUSHAD
           ; detect if CPUID instruction supported by microprocessor:
           PUSHFD
           POP     EAX
           MOV     EBX, EAX
           XOR     EAX, 1 SHL 21
           PUSH    EAX
           POPFD
           PUSHFD
           POP     EAX
           XOR     EAX, EBX
           XOR     EDX, EDX
           AND     EAX, 1 SHL 21
           JZ      SHORT NOCPUID        ; CPUID instruction not supported
           XOR     EAX, EAX
           CPUID
           TEST    EAX, EAX
           JZ      SHORT NOCPUID        ; CPUID function 1 not supported
           MOV     EAX, 1
           CPUID
           SHR     EDX, 23              ; bit 32 = MMX support
           AND     EDX, 1
    NOCPUID:MOV    USE_MMX, DL
           POPAD
       .endif
       MOV     [esi].PENDXOK, 0   
       ;Generate one random to initialize the Reseed counter
       OCall esi.Mersenne
       ror eax,13
       mov [esi].ReseedAt,al 
MethodEnd

;===============================================================================

Method RNG.Mersenne,uses esi ecx 
    SetObject esi
    
       CMP     [USE_MMX], 0         ; can we use MMX registers?
       MOV     ECX, [esi].MTI
       JE      R20

; this version uses MMX registers
       CMP     ECX, N*4
       JNB     M70                  ; buffer is empty, fill it
M40:    

       CMP     [esi].PENDXOK, 0
       JE      SHORT M50
       MOV     EAX, DWORD PTR [esi+4].PENDINGX ; a tempered random number is allready pending
       MOV     [esi].PENDXOK, 0        
       RET

M50:    ; tempering of two random numbers in parallel

       MOVQ    MM0, qword ptr [esi+ecx].MT
       MOVQ    MM1, MM0
       PSRLD   MM0, TEMU
       PXOR    MM0, MM1
       MOVQ    MM1, MM0        
       PSLLD   MM0, TEMS
       PAND    MM0, TMB
       PXOR    MM0, MM1
       MOVQ    MM1, MM0        
       PSLLD   MM0, TEMT
       PAND    MM0, TMC
       PXOR    MM0, MM1
       MOVQ    MM1, MM0        
       PSRLD   MM0, TEML
       PXOR    MM0, MM1
       MOVD    EAX, MM0                ; return low number
       MOVQ    [esi].PENDINGX, MM0         ; save high number for next time
       EMMS
    IF N AND 1                         ; check for the last odd one if N is odd
           CMP     ECX, (N-1)*4
           SETB    [esi].PENDXOK
    ELSE        
           MOV     [esi].PENDXOK, 1
    ENDIF        
       ADD     ECX, 8
       MOV     [esi].MTI, ECX        
       RET
       


M70:    ; buffer is empty. Fill it up
       MOVD    MM3, UMASK        ; load constants
       MOVD    MM4, LMASK
       MOVD    MM5, MATA
       lea     ECX, [esi].MT       ; kk
       lea     EDX, [esi+M*4].MT   ; km
       PUNPCKLDQ MM3, MM3          ; duplicate constants
       PUNPCKLDQ MM4, MM4
       PUNPCKLDQ MM5, MM5

M80:    ; kk loop, first part
       MOVQ    MM1, [ECX]          ; mt[kk+1] : mt[kk]
       ; get misaligned pair (mt[kk+2] : mt[kk+1])
       MOVD    MM2, [ECX+4]        ; 0 : mt[kk+1]
       PUNPCKLDQ MM2, [ECX+8]      ; mt[kk+2] : mt[kk+1]
IF      M AND 1                     ; km is misaligned if M is odd. Get misaligned pair
       MOVD    MM0, [EDX]          ; 0 : mt[km]
       PUNPCKLDQ MM0, [EDX+4]      ; mt[km+1] : mt[km]
ELSE
       MOVQ    MM0, [EDX]          ; mt[km+1] : mt[km]
ENDIF        
       PAND    MM1, MM3            ; & UPPER_MASK
       PAND    MM2, MM4            ; & LOWER_MASK
       POR     MM1, MM2            ; y1 : y0
       MOVQ    MM2, MM1            ; y1 : y0
       PSLLD   MM1, 31             ; copy bit 0 into all bits
       PSRAD   MM1, 31             ; -(y & 1)
       PAND    MM1, MM5            ; & MATRIX_A
       PSRLD   MM2, 1              ; y >> 1
       PXOR    MM2, MM1
       PXOR    MM0, MM2
       MOVQ    [ECX], MM0          ; result into mt[kk+1] : mt[kk]
       ADD     EDX, 8
       ADD     ECX, 8
       lea     eax,[esi + (N-1)*4].MT
       CMP     EDX, eax
       JB      M80                 ; loop until km wraparound
       
       MOV     EAX, [esi].MT        ; copy beginning to end for kk wraparound
IF      (M-N) AND 1                 ; do the split km pair if M-N is odd
       MOV     [EDX+4], EAX       
       MOVD    MM0, [EDX]
       PUNPCKLDQ MM0, [esi].MT     ; mt[km+1] : mt[km]
       SUB     EDX, N*4            ; km wraparound
       JMP     SHORT M95
ELSE        
       MOV     [EDX], EAX       
       SUB     EDX, N*4            ; km wraparound
ENDIF

M90:    ; second part of loop, where km has wrapped around
IF      (M-N) AND 1                 ; km is misaligned if M-N is odd. Get misaligned pair
       MOVD    MM0, [EDX]          ; 0 : mt[km]
       PUNPCKLDQ MM0, [EDX+4]      ; mt[km+1] : mt[km]
ELSE
       MOVQ    MM0, [EDX]          ; mt[km+1] : mt[km]
ENDIF        
M95:    MOVQ    MM1, [ECX]          ; mt[kk+1] : mt[kk]
       ; get misaligned pair mt[kk+2] : mt[kk+1]
       MOVD    MM2, [ECX+4]        ; 0 : mt[kk+1]
       PUNPCKLDQ MM2, [ECX+8]      ; mt[kk+2] : mt[kk+1]
       PAND    MM1, MM3            ; & UPPER_MASK
       PAND    MM2, MM4            ; & LOWER_MASK
       POR     MM1, MM2            ; y1 : y0
       MOVQ    MM2, MM1            ; y1 : y0
       PSLLD   MM1, 31             ; copy bit 0 into all bits
       PSRAD   MM1, 31             ; -(y & 1)
       PAND    MM1, MM5            ; & MATRIX_A
       PSRLD   MM2, 1              ; y >> 1
       PXOR    MM2, MM1
       PXOR    MM0, MM2
       MOVQ    [ECX], MM0          ; result into mt[kk+1] : mt[kk]
       ; loop epilog
       ADD     ECX, 8
       ADD     EDX, 8
       lea     eax,[esi+N*4].MT
       CMP     ECX,eax
       JB      M90

       XOR     ECX, ECX

       MOV     [esi].MTI, ECX        
       EMMS
       JMP     M40
      

       
; this version for old processors without MMX support:
R20:   CMP     ECX, N*4
       JNB     SHORT R50            ; buffer is empty, fill it   
R40:   MOV     EAX, [esi+ecx].MT
       ADD     ECX, 4
       MOV     [esi].MTI, ECX
       
       MOV     EDX, EAX
       SHR     EAX, TEMU
       XOR     EAX, EDX
       MOV     EDX, EAX
       SHL     EAX, TEMS
       AND     EAX, TEMB
       XOR     EAX, EDX
       MOV     EDX, EAX
       SHL     EAX, TEMT
       AND     EAX, TEMC
       XOR     EAX, EDX
       MOV     EDX, EAX
       SHR     EAX, TEML
       XOR     EAX, EDX            
       RET

       ; fill buffer with random numbers
R50:    PUSH    EBX        
       lea     ECX, [esi].MT
       lea     EDX, [esi+M*4].MT
       ; kk loop
R60:    MOV     EAX, [ECX]
       MOV     EBX, [ECX+4]
       AND     EAX, UPPER_MASK
       AND     EBX, LOWER_MASK
       OR      EAX, EBX
       SHR     EAX, 1
       SBB     EBX, EBX
       AND     EBX, MATRIX_A
       XOR     EAX, EBX
       XOR     EAX, [EDX]
       MOV     [ECX], EAX
       ADD     EDX, 4
       lea     eax,[esi+ N*4].MT
       CMP     EDX, eax
       JB      SHORT R70
       MOV     EAX, [esi].MT
       MOV     [EDX], EAX ; copy begin of table to after end to simplify kk+1 wraparound
       lea     EDX, [esi].MT
R70:    ADD     ECX, 4
       lea     eax,[esi+ N*4].MT
       CMP     ECX, eax
       JB      R60                  ; loop end        
       XOR     ECX, ECX
       MOV     [esi].MTI, ECX        
       POP     EBX
       JMP     R40        
       
MethodEnd

Method RNG.Get,uses esi ecx
LOCAL state [200] : byte

    SetObject esi
    
    ;Preserve the FPU state (protect from MMX code)
    lea edx,state
    fsave byte ptr [edx]
    
    ;If needed, reseed Mersenne Table
    .while [esi].ReseedAt==0
        DbgWarning "Reseeding Mersenne Table"
        OCall esi.Mersenne
        OCall esi.Init,eax
    .endw
    dec [esi].ReseedAt
    
    
   ;kludge the random seed using park miller
   mov eax, [esi].seed 
   xor edx, edx 
   mov ecx, 127773 
   div ecx 
   mov ecx, eax 
   mov eax, 16807 
   mul edx 
   mov edx, ecx 
   mov ecx, eax 
   mov eax, 2836 
   mul edx 
   sub ecx, eax 
   xor edx, edx 
   mov [esi].seed, eax 
   ;generate a random binary number
   OCall esi.Mersenne
   ;kludge seed with mersenne result
   ror eax,3
   xor [esi].seed,eax
   ;kludge result with the random seed
   mov ecx,[esi].seed
   add eax,ecx
   rol eax,cl
   
   ;Restore fpu state
   lea edx,state
   frstor  byte ptr [edx]
MethodEnd

;Return random Integer in given range
Method RNG.Int_Ranged,uses esi,dMin,dMax
    SetObject esi
    OCall esi.Get
    MOV     EDX, dMax             ; max
    MOV     ECX, dMin             ; min
    SUB     EDX, ECX
    JS      RERROR               ; max < min
    INC     EDX                  ; max - min + 1
    MUL     EDX                  ; multiply random number by interval and truncate
    LEA     EAX, [EDX+ECX]       ; add min
    RET
RERROR: MOV     EAX, 80000000H       ; error exit   
MethodEnd



;Return (on FPU) a random value in the range 0 thru +1.0
Method RNG.Real_Unsigned_Normal,uses esi
    SetObject esi
    OCall esi.Get
    and  eax,3FFFFFFFh     ;instead of 7FFFFFFFh
    or   eax,3F800000h     ; fix the exponent to 1
    fldReg eax
    fld1
    fsubp st(1),st
MethodEnd

;Return (on FPU) a random value in the range -1.0 thru +1.0
Method RNG.Real_Signed_Normal,uses esi
    SetObject esi
    OCall esi.Real_Unsigned_Normal
    OCall esi.Real_Unsigned_Normal
    fsub
MethodEnd

Method RNG.Real_Ranged,uses esi,fmin:real4,fmax:real4
    SetObject esi
    fld fmax
    fsub fmin
    OCall esi.Real_Unsigned_Normal
    fmul
    fadd fmin
MethodEnd
    