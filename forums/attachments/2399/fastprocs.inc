
;------------------------------------------------------------------------------
; Name:		FASTCALL calling convention support for 64-bit MASM
;		(ML64.EXE 8.00.50727.42)
; Version:	0.1ß
; Author:	MazeGen
; Credits:	Dr. Manhattan, tofu-sensei, jorgon, Feryno, Tomasz Grysztar
; Note:		The code is based on Petroizki's pmacros
;------------------------------------------------------------------------------

; settings switches (you can override them in your source)

 FAST_SPILL = 1		; set means always spill register arguments on the stack

; Assumption: all identifiers are case sensitive.

; dependencies:	ErrorEcho macro
;		@ArgI macro
;		@ArgRev macro from general macro set
;		reghandling macro set and its dependencies
;		typehandling macro set and its dependencies

; These features are not supported in this beta version:
; - hot-patchability
; - VARARG
; - floating-point parameters

; Syntax:
;
; FASTPROC label [,USES reglist] [,parameter:tag]... [,FRAME [:ehandler-address]
;  [FASTLOCAL varlist]
;  statements
; FASTENDP label
;
; When addressing procedure parameters and locals, square brackets ([]) have
; to be always used, or an error arises (such as "invalid use od register").
; That's because ML64 doesn't accept operands like [[rbp+xyz]] in contrast
; to 16-bit ML.

COMMENT #

Charts to explain how INVOKE macro works
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Stack layouts for even and odd number of stack parameters:

 A. Total 6 integer parameters -> 2 stack parameters (uses AND method):

  a) a layout for inital 8-byte alignment (e.g., RSP = 88dec)

				|      xyz	|	88
 1. PUSH RSP			|      88	|	80
 2. PUSH [RSP]			|      88	|	72 (used by POP RSP)
 3. AND RSP, NOT (16t-1)	|     hole	|	64
 5. MOV [RSP + 2*8 + 4*8], arg6	|     arg6	|	56
 6. MOV [RSP + 1*8 + 4*8], arg5	|     arg5	|	48
				|    R9 home	|
				|    R8 home	|
				|    RDX home	|
 4. SUB RSP, 4*8 + 2*8		|    RCX home	|	16
 7. CALL func			|  RETURN LINK	|	(8 at the entry of the function)
 8. ADD RSP, 4*8 + 2*8 + 8	|      88	|	72
 9. POP RSP			|      xyz	|	88

  b) a layout for inital 16-byte alignment (e.g., RSP = 80dec)

				|      xyz	|	80
 1. PUSH RSP			|      80	|	72 (used by POP RSP)
 2. PUSH [RSP]			|      80	|	64
 3. AND RSP, NOT (16t-1)	-   no change	-	64
 5. MOV [RSP + 2*8 + 4*8], arg6	|     arg6	|	56
 6. MOV [RSP + 1*8 + 4*8], arg5	|     arg5	|	48
				|    R9 home	|
				|    R8 home	|
				|    RDX home	|
 4. SUB RSP, 4*8 + 2*8		|    RCX home	|	16
 7. CALL func			|  RETURN LINK	|	(8 at the entry of the function)
 8. ADD RSP, 4*8 + 2*8 + 8	|      80	|	72
 9. POP RSP			|      xyz	|	80

 B. Total 7 integer parameters -> 3 stack parameters (uses OR method):

  a) a layout for inital 8-byte alignment (e.g., RSP = 88dec)

				|      xyz	|	88
 1. PUSH RSP			|      88	|	80
 2. PUSH [RSP]			|      88	|	72 (used by POP RSP)
 3. OR RSP, 16t/2		-   no change	-	72
 5. MOV [RSP + 3*8 + 4*8], arg7	|     arg7	|	64
 6. MOV [RSP + 2*8 + 4*8], arg6	|     arg6	|	56
 7. MOV [RSP + 1*8 + 4*8], arg5	|     arg5	|	48
				|    R9 home	|
				|    R8 home	|
				|    RDX home	|
 4. SUB RSP, 4*8 + 3*8		|    RCX home	|	16
 8. CALL func			|  RETURN LINK	|	(8 at the entry of the function)
 9. ADD RSP, 4*8 + 3*8		|      88	|	72
 10. POP RSP			|      xyz	|	88

  b) a layout for inital 16-byte alignment (e.g., RSP = 80dec)

				|      xyz	|	80
 1. PUSH RSP			|      80	|	72 (used by POP RSP)
 2. PUSH [RSP]			|      80	|	64
 3. OR RSP, 16t/2	       - removes prev one -     72
 5. MOV [RSP + 3*8 + 4*8], arg7	|     arg6	|	64
 6. MOV [RSP + 2*8 + 4*8], arg6	|     arg6	|	56
 7. MOV [RSP + 1*8 + 4*8], arg5	|     arg5	|	48
				|    R9 home	|
				|    R8 home	|
				|    RDX home	|
 4. SUB RSP, 4*8 + 3*8		|    RCX home	|	16
 8. CALL func			|  RETURN LINK	|	(8 at the entry of the function)
 9. ADD RSP, 4*8 + 3*8		|      80	|	72
 10. POP RSP			|      xyz	|	80

Charts to explain how is stack used during procedure prologue
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 Note: RBP is always aligned to 16 bytes. RBP always points at the last
 local. It is not possible to make 32-bit-like prologue since the stack
 frame pointer offset has to be less than or equal to 240 relative to
 the end of fixed allocation size (end of locals).

 RSP = 296dec at the entry of the procedure (never 16-byte aligned)
 USES rbx r12 r13 xmm6 xmm7
 FASTLOCAL byte_array[3]:BYTE
 FASTLOCAL word_array[3]:WORD
 FASTLOCAL dword_array[3]:DWORD
 FASTLOCAL loc_byte:BYTE
 FASTLOCAL loc_qword:QWORD
 FASTLOCAL loc_xmmword:XMMWORD

 1. PUSH RBP			|----|
      (RSP 16-byte aligned now)	|----|	288
 2. PUSH RBX			|----|
				|----|	280
 3. PUSH R12			|----|
				|----|	272
 4. PUSH R13			|----|
				|----|	264
  ALIGN 16 (+8)			|xxxx|
				|xxxx|	256
 6. MOVDQA			|----|
      [RBP + 64 + 16*1], XMM6	|----|
				|----|
				|----|	240	[RBP+80] (64 + 16 + align 0)
 7. MOVDQA			|----|
      [RBP + 64 + 16*0], XMM6	|----|
				|----|
				|----|	224	[RBP+64] (48 + 16 + align 0)
		loc_xmmword	|XMMW|
				|----|
				|----|
  ALIGN 16 (+8)			|----|	208	[RBP+48] (32 + 8  + align 8)
				|xxxx|
				|xxxx|
		loc_qword	|QW--|
  ALIGN 8 (+7)			|----|	192	[RBP+32] (24 + 1  + align 7)
				|xxxx|
		loc_byte	|xxxB|	184	[RBP+24] (12 + 12 + align 0)
		dword_array[3]	|DW--|
				|DW--|
  ALIGN 4 (+2)			|DW--|	172	[RBP+12] (4  + 6  + align 2)
		word_array[3]	|xxW-|
  ALIGN 2 (+1)			|W-W-|	164	[RBP+04] (0  + 3  + align 1)
		byte_array[3]	|xBBB|	160	[RBP]
 5. SUB RSP, 64 + 16*2 + 8

;;;;;;;;; old

				|  RETURN LINK	|	296
				|     		|
 1. PUSH RBP			|      RBP	|
      (RSP 16-byte aligned now)	|		|	288
 2. PUSH RBX			|      RBX	|
				|		|	280
 3. PUSH R12			|      R12	|
				|		|	272
 4. PUSH R13			|      R13	|
				|		|	264
    ALIGN 16 (-8)		|      hole	|
				|		|	256
 6. MOVDQA			|      XMM7	|
      [RSP + 64 + 16*1], XMM7	|		|
				|		|
				|		|	240		[RBP+80] (80+16-16)
 7. MOVDQA			|      XMM6	|
      [RSP + 64 + 16*0], XMM6	|		|
				|		|
				|		|	224		[RBP+64] (64+16-16)
				| byte_array[3] |	220		[RBP+61] (60+4-3)
    ALIGN 2 (-1)		| word_array[3]	|
				|    		|	212	(214)	[RBP+54] (52+8-6)
    ALIGN 4 (-2)		| dword_array[3]|
				|     		|
				|     		|	200		[RBP+40] (40+12-12)
				|    loc_byte   |
				|		|	192	(199)	[RBP+39] (32+8-1)
    ALIGN 8 (-7)		|   loc_qword   |
				|		|
				|		|
				|		|	176	(184)	[RBP+24] (16+16-8)
    ALIGN 16 (-8)		|  loc_xmmword	|
				|     		|
				|		|
 5. SUB RSP, 64 + 16*2 + 8	|		|	160		[RBP]

;;;;;;;;;;;;;;;;; old

				|  RETURN LINK	|	296
				|     		|
 1. PUSH RBP			|      RBP	|
      (RSP 16-byte aligned now)	|		|	288
 2. PUSH RBX			|      RBX	|
				|		|	280
 3. PUSH R12			|      R12	|
				|		|	272
 4. PUSH R13			|      R13	|
				|		|	264
    ALIGN 16 (-8)		|      hole	|
				|		|	256
 6. MOVDQA			|      XMM7	|
      [RSP + 64 + 16*1], XMM7	|		|
				|		|
				|		|	240		[RBP+80] (80+16-16)
 7. MOVDQA			|      XMM6	|
      [RSP + 64 + 16*0], XMM6	|		|
				|		|
				|		|	224		[RBP+64] (64+16-16)
				| byte_array[3] |	220		[RBP+61] (60+4-3)
    ALIGN 2 (-1)		| word_array[3]	|
				|    		|	212	(214)	[RBP+54] (52+8-6)
    ALIGN 4 (-2)		| dword_array[3]|
				|     		|
				|     		|	200		[RBP+40] (40+12-12)
				|    loc_byte   |
				|		|	192	(199)	[RBP+39] (32+8-1)
    ALIGN 8 (-7)		|   loc_qword   |
				|		|
				|		|
				|		|	176	(184)	[RBP+24] (16+16-8)
    ALIGN 16 (-8)		|  loc_xmmword	|
				|     		|
				|     		|
 5. SUB RSP, 64 + 16*2 + 8	|     		|	160		[RBP]




					|  RETURN LINK	|	296
					|     		|
 1. PUSH RBP (16-byte aligned now)	|      RBP	|
					|		|	288
 2. PUSH RBX				|      RBX	|		[RBP-8]
					|		|	280
 3. PUSH R12				|      R12	|		[RBP-16]
					|		|	272
 4. PUSH R13				|      R13	|		[RBP-24]
					|		|	264
    ALIGN 16 (-8)			|      hole	|
					|		|	256
 6. MOVDQA				|      XMM7	|		[RBP-32]
      [RSP + 64 + 16*1], XMM7		|		|
					|		|
					|		|	240
 7. MOVDQA				|      XMM6	|		[RBP-48]
      [RSP + 64 + 16*0], XMM6		|		|
					|		|
					|		|	224
    ALIGN 2 (-1)			| byte_array[3] |	220	[RBP-67]
					| word_array[3]	|		[RBP-74]
    ALIGN 4 (-2)			|    		|	212	(214)
					| dword_array[3]|		[RBP-88]
					|     		|
					|     		|	200
					|    loc_byte   |		[RBP-89]
    ALIGN 8 (-7)			|		|	192	(199)
					|   loc_qword   |		[RBP-104]
					|		|
					|		|
    ALIGN 16 (-8)			|		|	176	(184)
					|  loc_xmmword	|		[RBP-128]
					|     		|
					|     		|
 5. SUB RSP, 64 + 16*2 + 8		|     		|	160




					|  RETURN LINK	|	296
 1. PUSH RBP (16-byte aligned now)	|	RBP	|	288
 2. PUSH RBX				|	RBX	|	280
 3. PUSH R12				|	R12	|	272
 4. PUSH R13				|	R13	|	264
    ALIGN 16				|	hole	|	256
 6. MOVDQA				|    XMM7 high	|
      [RSP +  + 16*1], XMM7		|    XMM7 low	|	240
 7. MOVDQA				|    XMM6 high	|
      [RSP +  + 16*0], XMM6		|    XMM6 low	|	224
					|   byte_array  |

#

IFNDEF FASTPROCS

FASTPROCS EQU 001

 ; global constants and variables

 fast_nonvol_gen TEXTEQU <rbx,edi,rsi,rbp,r3,r7,r8,r5,r12,r13,r14,r15>
 fast_nonvol_xmm TEXTEQU <xmm6,xmm7,xmm8,xmm9,xmm10,xmm11,xmm12,xmm13,xmm14,xmm15>

 fast_param_subst_postfix = 0	;; global parameters counter
 fast_local_subst_postfix = 0	;; global locals counter

 ; why is the following done? See
 ; http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=156645

 OPTION NOKEYWORD:<ret>
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; x64 general macro set; collected and written by MazeGen

; ErrorEcho
;
; replacement for .ERRcond directive, prints user-friendly message with
; current file name and current file line
; 
; place:	required: string; description of place where the error arose
; message:	required: string; additional message
;
; example: ErrorEcho <Print macro>, <no text given>

ErrorEcho MACRO place:REQ, message:REQ
LOCAL line

 PUSHCONTEXT RADIX
.RADIX 10
 line TEXTEQU %@Line		;; print the line number in decimal radix
 POPCONTEXT RADIX

 ECHO
%ECHO * Error occured in @FileCur(line):
 ECHO * &place:
 ECHO *  &message
 ECHO *
 ECHO * compilation interrupted by forcing error...
 ECHO
 .ERR
ENDM

@ArgI MACRO index:REQ, arglist:VARARG
LOCAL arg
LOCAL count, retstr

 retstr TEXTEQU <>		;; return blank if index out of the list

 count = 0

%FOR arg, <arglist>
   count = count + 1

   IF count EQ index
     retstr TEXTEQU <arg>
     EXITM
   ENDIF
 ENDM

 EXITM <retstr>
ENDM

@ArgRev MACRO arglist:VARARG
LOCAL txt, arg

 txt TEXTEQU <>
%FOR arg, <arglist>
   txt CATSTR <arg>, <!,>, txt
 ENDM
 
 txt SUBSTR txt, 1, @SizeStr (%txt) - 1

 txt CATSTR <!<>, txt, <!>>

 EXITM txt
ENDM

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; x64 reghandling macro set; written by MazeGen

; note: no macro checks whether is the given argument valid register operand

; Is it general register?

IsGeneralReg MACRO arg:REQ
local curarg, f

 f = 0

 ;; note: to avoid error "string or text literal too long", the list
 ;; has to be divided among more loops

 FOR curarg, <al,cl,dl,bl,spl,bpl,sil,dil,ah,ch,dh,bh,r0b,r1b,r2b,r3b,r4b,r5b,r6b,r7b,r8b,r9b,r10b,r11b,r12b,r13b,r14b,r15b,ax,cx,dx,bx,sp,bp,si,di,r0w,r1w,r2w,r3w,r4w,r5w,r6w,r7w,r8w,r9w,r10w,r11w,r12w,r13w,r14w,r15w>
 %IFIDNI <curarg>, <arg>
     f = 1
     EXITM 
  ENDIF
 ENDM

 IF f
   EXITM %f
 ENDIF

 FOR curarg, <eax,ecx,edx,ebx,esp,ebp,esi,edi,r0d,r1d,r2d,r3d,r4d,r5d,r6d,r7d,r8d,r9d,r10d,r11d,r12d,r13d,r14d,r15d,rax,rcx,rdx,rbx,rsp,rbp,rsi,rdi,r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15>
 %IFIDNI <curarg>, <arg>
     f = 1
     EXITM 
  ENDIF
 ENDM

 EXITM %f
ENDM

; Register to its 16/32/64-bit hardware number

Reg216bitNr MACRO arg:REQ
local curarg, count, exitstr

 exitstr TEXTEQU <-1>

 ;; 8-bit register mnemonics

 count = 1

 FOR curarg, <al,cl,dl,bl,spl,bpl,sil,dil,ah,ch,dh,bh,r0b,r1b,r2b,r3b,r4b,r5b,r6b,r7b,r8b,r9b,r10b,r11b,r12b,r13b,r14b,r15b>
  %IFIDNI <curarg>, <arg>
     exitstr SUBSTR <00010203040506070001020300010203040506070809101112131415>, count, 2
     EXITM
   ENDIF
   count = count + 2
 ENDM

 IFDIFI exitstr, <-1>
   EXITM @CatStr (%exitstr, <t>)
 ENDIF

 ;; 16-bit register mnemonics

 count = 1

 FOR curarg, <ax,cx,dx,bx,sp,bp,si,di,r0w,r1w,r2w,r3w,r4w,r5w,r6w,r7w,r8w,r9w,r10w,r11w,r12w,r13w,r14w,r15w>
  %IFIDNI <curarg>, <arg>
     exitstr SUBSTR <000102030405060700010203040506070809101112131415>, count, 2
     EXITM
   ENDIF
   count = count + 2
 ENDM

 IFDIFI exitstr, <-1>
   EXITM @CatStr (%exitstr, <t>)
 ENDIF

 ;; 32-bit register mnemonics

 count = 1

 FOR curarg, <eax,ecx,edx,ebx,esp,ebp,esi,edi,r0d,r1d,r2d,r3d,r4d,r5d,r6d,r7d,r8d,r9d,r10d,r11d,r12d,r13d,r14d,r15d>
  %IFIDNI <curarg>, <arg>
     exitstr SUBSTR <000102030405060700010203040506070809101112131415>, count, 2
     EXITM
   ENDIF
   count = count + 2
 ENDM

 IFDIFI exitstr, <-1>
   EXITM @CatStr (%exitstr, <t>)
 ENDIF

 ;; 64-bit general register mnemonics

 count = 1

 FOR curarg, <rax,rcx,rdx,rbx,rsp,rbp,rsi,rdi,r0,r1,r2,r3,r4,r5,r6,r7,r8,r9,r10,r11,r12,r13,r14,r15>
  %IFIDNI <curarg>, <arg>
     exitstr SUBSTR <000102030405060700010203040506070809101112131415>, count, 2
     EXITM
   ENDIF
   count = count + 2
 ENDM

 IFDIFI exitstr, <-1>
   EXITM @CatStr (%exitstr, <t>)
 ENDIF

 ;; 64-bit mmx register mnemonics

 count = 1

 FOR curarg, <mm0,mm1,mm2,mm3,mm4,mm5,mm6,mm7>
  %IFIDNI <curarg>, <arg>
     exitstr SUBSTR <01234567>, count, 1
     EXITM
   ENDIF
   count = count + 1
 ENDM

 IFDIFI exitstr, <-1>
   EXITM @CatStr (%exitstr, <t>)
 ENDIF

 ;; 80-bit fp register mnemonics

 count = 1

 FOR curarg, <st,st(0),st(1),st(2),st(3),st(4),st(5),st(6),st(7)>
  %IFIDNI <curarg>, <arg>
     exitstr SUBSTR <001234567>, count, 1
     EXITM
   ENDIF
   count = count + 1
 ENDM

 IFDIFI exitstr, <-1>
   EXITM @CatStr (%exitstr, <t>)
 ENDIF

 ;; 128-bit xmm register mnemonics

 count = 1

 FOR curarg, <xmm0,xmm1,xmm2,xmm3,xmm4,xmm5,xmm6,xmm7,xmm8,xmm9,xmm10,xmm11,xmm12,xmm13,xmm14,xmm15>
  %IFIDNI <curarg>, <arg>
     exitstr SUBSTR <00010203040506070809101112131415>, count, 2
     EXITM
   ENDIF
   count = count + 2
 ENDM

 IFDIFI exitstr, <-1>
   EXITM @CatStr (%exitstr, <t>)
 ENDIF

 ;; segment selector mnemonics

 count = 1

 FOR curarg, <es,cs,ss,ds,fs,gs>
  %IFIDNI <curarg>, <arg>
     exitstr SUBSTR <012345>, count, 1
     EXITM
   ENDIF
   count = count + 1
 ENDM

 EXITM %exitstr
ENDM

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; x64 typehandling macro set; collected and written by MazeGen

; note: some of Is... macros originally written by Four-F
; these macros were renamed, edited and fixed

IsCodeLabel MACRO op:REQ
 EXITM %(OPATTR (op)) AND 00000001y	;; references a code label
ENDM

IsDirMemAddr MACRO op:REQ 
 EXITM %(OPATTR (op)) AND 00001000y	;; uses direct memory addressing
ENDM

IsExtLabel MACRO op:REQ
 EXITM %(OPATTR (op)) AND 10000000y	;; references an external label
ENDM

IsImm MACRO op:REQ
 EXITM %(OPATTR (op)) AND 00000100y	;; is an immediate value
ENDM

IsNoError MACRO op:REQ
 EXITM %(OPATTR (op)) AND 00100000y
 ;; references no undefined symbols and is without error 
ENDM

IsReg MACRO op:REQ
 EXITM %(OPATTR (op)) AND 00010000y	;; is a register value
ENDM

; FASTENDP

fastendp MACRO procname:REQ
LOCAL tmp

 procname ENDP

 ;; replace parameter and local identifiers with unique redefinable ones.

%FOR tmp, <fast_paramnames>
   @SubStr (tmp, 2) TEXTEQU @CatStr (<fast_param_subst>, %fast_param_subst_postfix)
   fast_param_subst_postfix = fast_param_subst_postfix + 1
 ENDM

%FOR tmp, <fast_localnames>
   @SubStr (tmp, 2) TEXTEQU @CatStr (<fast_local_subst>, %fast_local_subst_postfix)
   fast_local_subst_postfix = fast_local_subst_postfix + 1
 ENDM

ENDM

;fastendp TEXTEQU <fastendp>
Fastendp TEXTEQU <fastendp>
FAstendp TEXTEQU <fastendp>
FastEndp TEXTEQU <fastendp>
FAstEndp TEXTEQU <fastendp>
FAstENdp TEXTEQU <fastendp>
FASTENDP TEXTEQU <fastendp>

; FASTCALL EPILOG macro procedure

; why is the following done? See
; http://connect.microsoft.com/VisualStudio/feedback/ViewFeedback.aspx?FeedbackID=156645

ret TEXTEQU <fastepilog>
Ret TEXTEQU <fastepilog>
rEt TEXTEQU <fastepilog>
REt TEXTEQU <fastepilog>
reT TEXTEQU <fastepilog>
ReT TEXTEQU <fastepilog>
rET TEXTEQU <fastepilog>
RET TEXTEQU <fastepilog>

fastepilog MACRO procname, flag, parmbytes, localbytes, reglist, userparms
LOCAL rbpdist
LOCAL usesreg

 rbpdist = fast_usesxmmbase

%FOR usesreg, <fast_usesxmmlist>
   movdqa usesreg, [rbp+rbpdist]
   rbpdist = rbpdist + SIZEOF XMMWORD
 ENDM

 ;; expected, official epilog starts here

 lea rsp, [rbp+fast_fixedallocsize]

 IFNB fast_usesgenlist
  %FOR usesreg, @ArgRev (fast_usesgenlist)
     pop usesreg
   ENDM
 ENDIF

 pop rbp

 BYTE 0C3h		; RETN (hardcoded because RET is in NOKEYWORD list)
ENDM

; FASTLOCAL

fastlocal MACRO variables:VARARG
LOCAL variable
LOCAL tmp, checktype
LOCAL count, paramname, paramnr, $paramnr, pos0, pos1, separator
LOCAL localsize, localtype
	
 IFB <variables>
   ErrorEcho <local macro>, <syntax error: locals must be defined with name and type>
   EXITM
 ENDIF
	
 ;; go trough the locals

 FOR variable, <variables>

   separator INSTR <variable>, <:>	;; seek for an ':'

   IF separator EQ 0
     ErrorEcho <local macro>, <syntax error: locals must be defined with name and type>
     EXITM
   ENDIF

   ;; local definition can be accessible only through: @SubStr (<variable>, 1, separator - 1)

   ;; get type of the local

   localtype SUBSTR <variable>, separator + 1, @SizeStr (<variable>) - separator

   ;; seek for "[" (an array definition of a local)

   pos0 INSTR @SubStr (<variable>, 1, separator - 1), <[>

   ;; see if we found an array definition

   IF pos0 GT 1

     pos1 INSTR @SubStr (<variable>, 1, separator - 1), <]>
			
     ;; catch "[]blah" and "][" and "[]", show the error string starting at "]"

     IF (pos1 NE @SizeStr (@SubStr (<variable>, 1, separator - 1))) OR (pos0 GT pos1) OR (pos0 + 1 EQ pos1)
%       ErrorEcho <local macro>, <syntax error in array definition: @SubStr (@SubStr (<variable>, 1, separator - 1), pos1)>
       EXITM
     ENDIF
			
     ;; set correct array count

     count SUBSTR @SubStr (<variable>, 1, separator - 1), pos0 + 1, pos1 - pos0 - 1
			
     IF count EQ 0
%      ErrorEcho <local macro>, <syntax error: the number of elements cannot be zero>
       EXITM
     ENDIF
			
     ;; adjust local name pointer

     separator = pos0

   ELSE

     ;; catch "blah]", show the error string starting at "]"

     pos1 INSTR @SubStr (<variable>, 1, separator - 1), <]>

     IF pos1
%      ErrorEcho <local macro>, <syntax error in array definition: @SubStr (@SubStr (<variable>, 1, separator - 1), pos1)>
       EXITM
     ENDIF

     count TEXTEQU <1>
     pos0 = separator		;; set the end of the name

   ENDIF
		
   ;; make local counts list

   fast_localcounts CATSTR fast_localcounts, count, <,>

   ;; check if the local is not already defined as a parameter

   paramnr = 0

%  FOR paramname, <fast_paramnames>
     paramnr = paramnr + 1
     IFIDN <paramname>, @CatStr (<$>, @SubStr (<variable>, 1, pos0 - 1))
       PUSHCONTEXT RADIX
      .RADIX 10
       $paramnr TEXTEQU %paramnr
%      ErrorEcho <local macro>, <parameter $paramnr redefined as a local>
       POPCONTEXT RADIX
       EXITM
     ENDIF
   ENDM
		
   ;; make local names list
   ;; note: to prevent substitution of the name in the next FOR pass,
   ;; the name is decorated with "$" prefix

   fast_localnames CATSTR fast_localnames, <$>, @SubStr (<variable>, 1, pos0 - 1), <,>
		
   ;; check if the type is PTR

   checktype TEXTEQU <>

%  FORC tmp, localtype
     IFIDNI <tmp>, <! >
       EXITM			;; space found - type string was loaded
     ENDIF
     checktype CATSTR checktype, <tmp>
   ENDM

   IFIDNI checktype, <PTR>
     localtype TEXTEQU <QWORD>
   ENDIF
				
   ;; make local types list

   fast_localtypes CATSTR fast_localtypes, localtype, <,>

   ;; consider the size
   ;; note: it has to be done this way because the type can
   ;; be also a structure or an array

%  localsize = SIZEOF (TYPE (localtype))

   ;; make local sizes list

   fast_localsizes CATSTR fast_localsizes, %localsize, <,>

 ENDM

ENDM

;fastlocal TEXTEQU <fastlocal>
Fastlocal TEXTEQU <fastlocal>
FAstlocal TEXTEQU <fastlocal>
FastLocal TEXTEQU <fastlocal>
FAstLocal TEXTEQU <fastlocal>
FAstLOcal TEXTEQU <fastlocal>
FASTLOCAL TEXTEQU <fastlocal>

; FASTPROC: PROC replacement

; poznámky (pøeložit do eng):
; - pokud procedura používá volbu FRAME, není možné dál deklarovat žádné
;   další volby (jako USES) a ani žádné argumenty.

COMMENT #

fastproc
fastendp
fastprolog
fastepilog

fast_localnames
fast_localtypes
fast_localsizes
fast_localcounts

fast_paramnames
fast_paramtypes
fast_paramsizes

fast_usesgenlist
fast_usesgencount
fast_usesxmmlist
fast_usesxmmcount

fast_fixedallocsize
fast_usesxmmbase
#

fastproc MACRO procname:REQ, args:VARARG
LOCAL procdef
LOCAL arg, tmp, separator
LOCAL optionname
LOCAL $paramsize, paramsize, paramname, paramtype, checktype
LOCAL usestmplist, useslist, usestmpreg, usesreg, useslistchar
		
 ;; set fastprocs prologue and epilogue macros

 OPTION PROLOGUE:fastprolog
 OPTION EPILOGUE:fastepilog

 ;; fastprocs globals init

 fast_usesgenlist TEXTEQU <>	;; USES general registers
 fast_usesgencount = 0		;; number of USES general registers
 fast_usesxmmlist TEXTEQU <>
 fast_usesxmmcount = 0

 fast_paramnames TEXTEQU <>
 fast_paramtypes TEXTEQU <>
 fast_paramsizes TEXTEQU <>

 fast_localnames TEXTEQU <>
 fast_localtypes TEXTEQU <>
 fast_localsizes TEXTEQU <>
 fast_localcounts TEXTEQU <>

 fast_frame = 0

 ;; locals init

 procdef TEXTEQU <>	;; definition string to be sent for 'proc'
		
 ;; go trough the function parameters

 FOR arg, <args>

   IF fast_frame
     ErrorEcho <fastproc macro>, <there cannot be any other options or arguments after FRAME option: arg>
     EXITM
   ENDIF

   separator INSTR <arg>, <:>	;; seek for ':'
			
   IF separator GT 1		;; a parameter or FRAME:ehandler-address option

     paramname SUBSTR <arg>, 1, separator

     IFIDNI <FRAME:>, paramname
       fast_frame = 1

       ;; add this option to the final string that is send to the 'proc'

       procdef CATSTR procdef, <! >, <arg>

     ELSE

       ;; param name can be accessible only through: @SubStr (<arg>, 1, separator - 1)

       ;; get parameter type

       paramtype SUBSTR <arg>, separator + 1, @SizeStr (arg) - separator
				
       ;; make param names list
       ;; note: to prevent substitution of the name in the next FOR pass,
       ;; the name is decorated with "$" prefix

       fast_paramnames CATSTR fast_paramnames, <$>, @SubStr (<arg>, 1, separator - 1), <,>
				
       ;; set parameter size, align all to 8 bytes

       IFDIFI paramtype, <VARARG>
					
         ;; check if the type is PTR

         checktype TEXTEQU <>

%        FORC tmp, paramtype
           IFIDNI <tmp>, <! >
             EXITM			;; space found - type string was loaded
           ENDIF
           checktype CATSTR checktype, <tmp>
         ENDM

         IFIDNI checktype, <PTR>
           paramtype TEXTEQU <QWORD>
         ENDIF
					
         ;; make param types list

         fast_paramtypes CATSTR fast_paramtypes, paramtype, <,>

         ;; consider the size
         ;; note: it has to be done this way because the type can
         ;; be also a structure or an array

%        paramsize = SIZEOF (TYPE (paramtype))
					
         ;; make params sizes list

         fast_paramsizes CATSTR fast_paramsizes, %paramsize, <,>

       ELSE
         ErrorEcho <fastproc macro>, <VARARG is not supported>
         EXITM

       ENDIF
     ENDIF
    
   ELSE
     ;; there was no ':', it could be only a FRAME or USES option;
     ;; note: ML64 (8.00.50727.42) disable any other options than FRAME
     ;; if FRAME is used and therefore USES option is processed manually
     ;; (in contrast to 32-bit ML, where the prologue and epilogue macros
     ;; can provide USES list automatically through one of their arguments)

     optionname SUBSTR <arg>, 1, 5		;; only "FRAME" or "USES "

     IFIDNI <USES >, optionname

       usestmplist SUBSTR <arg>, 6		;; skip "USES "

       ;; make uses list with entries delimited with commas

       useslist TEXTEQU <>
       usestmpreg TEXTEQU <>

%      FORC useslistchar, <usestmplist>
         IFDIFI <useslistchar>, <! >		;; skip spaces
           usestmpreg CATSTR usestmpreg, <useslistchar>
         ELSE
           useslist CATSTR useslist, usestmpreg, <!,>
           usestmpreg TEXTEQU <>
         ENDIF
       ENDM

       useslist CATSTR useslist, usestmpreg, <!,> ;; add also the latest one

%      FOR usesreg, <useslist>
         IF IsReg (usesreg)
           IF SIZEOF (TYPE (usesreg)) EQ SIZEOF (TYPE (QWORD))
             IF IsGeneralReg (usesreg)
               fast_usesgenlist CATSTR fast_usesgenlist, <usesreg>, <!,>
               fast_usesgencount = fast_usesgencount + 1
             ELSE
               ErrorEcho <fastproc macro>, <USES accepts only 64-bit general or xmm register: usesreg>
             ENDIF
           ELSEIF SIZEOF (TYPE (usesreg)) EQ SIZEOF (TYPE (XMMWORD))
             fast_usesxmmlist CATSTR fast_usesxmmlist, <usesreg>, <!,>
             fast_usesxmmcount = fast_usesxmmcount + 1
           ELSE
             ErrorEcho <fastproc macro>, <USES accepts only 64-bit general or xmm register: usesreg>
           ENDIF
         ELSE
           ErrorEcho <fastproc macro>, <syntax error in USES: usesreg>
         ENDIF
       ENDM

     ELSEIFIDNI <FRAME>, optionname
       fast_frame = 1

       ;; add this option to the final string that is send to the 'proc'

       procdef CATSTR procdef, <! >, <arg>

     ELSE
       ErrorEcho <fastproc macro>, <syntax error: arg> ;; print whole argument
     ENDIF

   ENDIF
 ENDM
  
  ;; create the real 'proc' with option FRAME, if any, without any other
  ;; options or arguments

  procname PROC procdef

ENDM

;fastproc TEXTEQU <fastproc>
Fastproc TEXTEQU <fastproc>
FAstproc TEXTEQU <fastproc>
FastProc TEXTEQU <fastproc>
FAstProc TEXTEQU <fastproc>
FAstPRoc TEXTEQU <fastproc>
FASTPROC TEXTEQU <fastproc>

;pridat
;- varovani, pokud by procedura frame mit mela a nema
;
;- pokud je xmm volatile, ma se pri jeho ukladani taky pouzit savexmm128,
;  nebo allocstack? protoze sem allocstack v tomto pripade nikde zmineny
;  nebo pouzity nevidel, pouzivam vzdycky savexmm128

; FASTCALL PROLOG macro function

; note: both parmbytes, localbytes, reglist and userparms don't contain
; any useful informations because of broken PROC directive; these
; informations are provided by global variables

; note: procedure's arguments are processed lastly because the size
; of locals and USES xmm registers has to be known first.

fastprolog MACRO procname, flag, parmbytes, localbytes, reglist, userparms
LOCAL count, $count
LOCAL rbpdist
LOCAL name, type, size, $size, isreal
LOCAL localcnt, alignsize
LOCAL usesreg, nonvolreg, isnonvolreg
LOCAL paramreg

 IF FAST_SPILL
   count = 0

  %FOR type, <fast_paramtypes>
     count = count + 1

     ;; a note: XMMWORD is not allowed as a type of a parameter

     IF (TYPE (type) EQ TYPE REAL4) OR (TYPE (type) EQ TYPE REAL8)
       paramreg TEXTEQU @ArgI (count, <xmm0,xmm1,xmm2,xmm3>)
       movq qword ptr [rsp + count * SIZEOF QWORD], paramreg
     ELSE
       paramreg TEXTEQU @ArgI (count, <rcx,rdx,r8,r9>)
       mov [rsp + count * SIZEOF QWORD], paramreg
     ENDIF

     IF count EQ 4	;; there are 4 register parameters
       EXITM
     ENDIF
   ENDM

 ENDIF

 rbpdist = 0		;; the distance to stack frame pointer

 push rbp
 IF fast_frame
  .PUSHREG rbp
 ENDIF

%FOR usesreg, <fast_usesgenlist>
   push usesreg
   IF fast_frame
     IF @InStr (1, fast_nonvol_gen, usesreg)
        .PUSHREG usesreg
     ELSE
        .ALLOCSTACK SIZEOF QWORD
     ENDIF
   ENDIF
 ENDM

 ;; assign memory operands to locals

;nebylo by lepší místo použití $size (která je stejnì použita jedinì pokud
;nejde o strukturu) udìlat pøímo "type ptr", když mám seznam typù? mìlo by
;to normálnì projít a bylo by to èistìjší

 count = 0

%FOR name, <fast_localnames>
   count = count + 1
   type TEXTEQU @ArgI (count, fast_localtypes)
   size TEXTEQU @ArgI (count, fast_localsizes)
   localcnt TEXTEQU @ArgI (count, fast_localcounts)

   IF size EQ 2
     $size TEXTEQU <word>
     alignsize = 2
   ELSEIF size EQ 4
     $size TEXTEQU <dword>
     alignsize = 4
   ELSEIF size EQ 6
     $size TEXTEQU <fword>
     alignsize = 8
   ELSEIF size EQ 8
     $size TEXTEQU <qword>
     alignsize = 8
   ELSEIF size EQ 10t		;; REAL10 and TBYTE types
     $size TEXTEQU <tbyte>	;; align to 16 bytes
     alignsize = 16t
   ELSEIF size EQ 16t
     $size TEXTEQU <xmmword>
     alignsize = 16t
   ELSEIF size GT 16t		;; structures and arrays greater than 16 bytes
     $size TEXTEQU <byte>	;; align to 16 bytes
     alignsize = 16t
   ELSE				;; structures and arrays of sizes
     $size TEXTEQU <byte>	;; 3, 5, 7, 9, 11, 12, 13, 14, and 15
     alignsize = 1		;; align to 1 byte
   ENDIF

   IFIDNI type, <REAL4>
     isreal = 1
   ELSEIFIDNI type, <REAL8>
     isreal = 1
   ELSEIFIDNI type, <REAL10>
     isreal = 1
   ELSE
     isreal = 0
   ENDIF

   ;; align the local to its size and adjust the relative offset to RBP

   rbpdist = (rbpdist + alignsize - 1) AND (NOT (alignsize - 1))

   ;; set local EQU
   ;; if a structure is declared (i.e. TYPE of a paramater is not equal
   ;; to TYPE of its size), concatenate also structure name

   IF (TYPE (type)) EQ (TYPE ($size)) OR isreal
;    @SubStr (name, 2) TEXTEQU @CatStr (<[>, %$size, < ptr rbp+>, %rbpdist, <]>) ;; this method of accessing memory operands is no longer available in ML64
     @SubStr (name, 2) TEXTEQU @CatStr (%$size, < ptr (rbp+>, %rbpdist, <)>)
   ELSE
;    @SubStr (name, 2) TEXTEQU @CatStr (<[rbp+>, %rbpdist, <].>, %type)
     @SubStr (name, 2) TEXTEQU @CatStr (<(rbp+>, %rbpdist, <).>, %type)
   ENDIF

   rbpdist = rbpdist + size*localcnt
 ENDM

 ;; the base of the locals has to be always aligned to 16 bytes;
 ;; if odd number of USES general registers, align to 8 bytes,
 ;; otherwise to 16 bytes

 IF fast_usesgencount AND 1
   rbpdist = (rbpdist + SIZEOF QWORD - 1) AND (NOT (SIZEOF QWORD - 1))
 ELSE
   rbpdist = (rbpdist + SIZEOF XMMWORD - 1) AND (NOT (SIZEOF XMMWORD - 1))
 ENDIF

 ;; allocate the stack for xmm USES registers and locals and align
 ;; the allocation to 16 bytes (if odd number of USES general registers,
 ;; add one QWORD)

 fast_fixedallocsize = rbpdist + fast_usesxmmcount*SIZEOF XMMWORD + ((fast_usesgencount AND 1) * SIZEOF QWORD)

 sub rsp, fast_fixedallocsize
 IF fast_frame
  .ALLOCSTACK fast_fixedallocsize
 ENDIF

 ;; estabilish the stack frame

 mov rbp, rsp
 IF fast_frame
  .SETFRAME rbp, 0
 ENDIF

 ;; save USES xmm registers, if any

 fast_usesxmmbase = rbpdist		;; remember their base for the epilogue

%FOR usesreg, <fast_usesxmmlist>
   movdqa [rbp+rbpdist], usesreg
   IF fast_frame
    .SAVEXMM128 usesreg, rbpdist
   ENDIF
   rbpdist = rbpdist + SIZEOF XMMWORD
 ENDM

 ;; get offset relative to rbp of the first parameter
 ;; (2x QWORD means pushed RBP and return link pushed by CALL)

 rbpdist = fast_fixedallocsize + fast_usesgencount * SIZEOF QWORD + 2 * SIZEOF QWORD

 ;; assign memory operands to parameters

 count = 0

%FOR name, <fast_paramnames>
   count = count + 1
   type TEXTEQU @ArgI (count, fast_paramtypes)
   size TEXTEQU @ArgI (count, fast_paramsizes)

   IF size EQ 2
     $size TEXTEQU <word>
   ELSEIF size EQ 4
     $size TEXTEQU <dword>
   ELSEIF size EQ 6
     $size TEXTEQU <fword>
   ELSEIF size EQ 8
     $size TEXTEQU <qword>
   ELSEIF size GT 8
     PUSHCONTEXT RADIX
    .RADIX 10		;; print the parameter number in decimal radix
     $count TEXTEQU %count
%    ErrorEcho <fastproc macro>, <the !size of the parameter $count cannot be greater than QWORD: type>
     POPCONTEXT RADIX
   ELSE
     $size TEXTEQU <byte>
   ENDIF

   IFIDNI type, <REAL4>
     isreal = 1
   ELSEIFIDNI type, <REAL8>
     isreal = 1
   ELSE
     isreal = 0
   ENDIF

   IF (TYPE (type)) EQ (TYPE ($size)) OR isreal
;    @SubStr (name, 2) TEXTEQU @CatStr (<[>,%$size, < ptr rbp+>, %rbpdist, <]>)
     @SubStr (name, 2) TEXTEQU @CatStr (%$size, < ptr (rbp+>, %rbpdist, <)>)
   ELSE
;    @SubStr (name, 2) TEXTEQU @CatStr (<[rbp+>, %rbpdist, <].>, %type)
     @SubStr (name, 2) TEXTEQU @CatStr (<(rbp+>, %rbpdist, <).>, %type)
   ENDIF

   rbpdist = rbpdist + SIZEOF QWORD ;; any parameter allocates QWORD in stack
 ENDM

 IF fast_frame
  .ENDPROLOG
 ENDIF

 EXITM %0	;; from the assembler's point of view, there are no parameters
ENDM

; INVOKE

; Contrary to MSDN documentation, you can pass arrays by immediate value
; using this macro.

; A note about sign-extending:
; Quote from Matt Pietrek's article "Everything You Need To Know To Start
; Programming 64-Bit Windows Systems":
; "integer parameters that are less than 64-bits are sign extended"
; Because MSDN doesn't bear to that, the parameters are not sign extended.

invoke MACRO name:REQ, args:VARARG
LOCAL argnr, $argnr, count, stackcount
LOCAL arg, arglist
LOCAL acc, acc_used
LOCAL addrchars, paddr, localname;, islocal
LOCAL regarg8, regarg8alt
LOCAL regarg16, regarg16alt
LOCAL regarg32, regarg32alt
LOCAL regarg64, regarg64alt
LOCAL regarg128
;;;LOCAL isinteger

 ;; a note: @ArgRev doesn't return empty string on empty input because
 ;; this way it is defined by Microsoft. If I would modify it, user's
 ;; includes maight contain the original @ArgRev macro which would redefine
 ;; the one modified by me back.

 IFNB <args>
   arglist TEXTEQU @ArgRev (args)	; reversed list of arguments
 ELSE
   arglist TEXTEQU <!<!>>
 ENDIF

 count = 0
 FOR arg, <args>			; count the arguments
   count = count + 1
 ENDM

 argnr = count				; number of last argument

 ; count contains real number of arguments passed on the stack

 IF (count-4) GE 0
   count = count - 4
 ELSE
   count = 0
 ENDIF

 stackcount = count

 ; note: when calling INVOKE, the stack has to be aligned at least to 8 bytes
 ; note: at least WinAPI calls always need 8-byte stack alignment at the entry

 ; note: for explanation of aligning the stack see stack layouts at the top
 ; if odd number of arguments, align the stack to 8 bytes using OR
 ; if even number of arguments, align the stack to 16 bytes using AND
 ; credit: the idea about how to align the stack comes from GoAsm

 push rsp
 push [rsp]
 IF stackcount AND 1
   or rsp, 16t/2
 ELSE
   and rsp, NOT (16t-1)
 ENDIF

 ;; reserve the space for arguments and spilling registers

 sub rsp, 4*8 + stackcount*8

 acc_used = 0

%FOR arg, arglist
   IFE count			;; no more arguments passed through the stack
     EXITM
   ENDIF

   count = count - 1		;; zero-based index
   argnr = argnr - 1		;; number of current argument

   ;; catch ADDR operator

   IF @SizeStr (<arg>) LT 5	;; "ADDR" has 4 chars
     addrchars TEXTEQU <>
   ELSE
     addrchars SUBSTR <arg>, 1, 5
   ENDIF

   IFIDNI <ADDR >, addrchars
     paddr SUBSTR <arg>, 6

;     islocal = 0
;
;    %FOR localname, <fast_localnames>
;       IFIDNI @SubStr (localname, 2), paddr
;         islocal = 1
;         EXITM
;       ENDIF
;     ENDM

;     IF islocal
;       lea rax, [paddr]				;; local variable
;     ELSE
;       ;; the following can't be used in ML64
;       ;; mov [rsp + count*8 + 4*8], OFFSET paddr
;       lea rax, paddr
;     ENDIF

     acc_used = 1

     IF IsCodeLabel (paddr) OR IsDirMemAddr (paddr) OR IsExtLabel (paddr)
       ;; the following can't be used in ML64
       ;; mov [rsp + count*8 + 4*8], OFFSET paddr
       lea rax, paddr
     ELSE
       lea rax, [paddr]				;; local variable
     ENDIF

     mov [rsp + count*8 + 4*8], rax

   ELSE

     IF IsNoError (arg)

       IF acc_used
         FOR acc, <al,ah,ax,eax,rax>
%          IFIDNI <arg>, <acc>
%            ErrorEcho <invoke macro>, <register value of arg overwritten> ;; arg can be printed directly in this case
             EXITM
           ENDIF
         ENDM
       ENDIF

       IF IsImm (arg)
         mov qword ptr [rsp + count*8 + 4*8], arg

       ELSEIF IsReg (arg)
         IF SIZEOF (TYPE (arg)) NE SIZEOF XMMWORD
           mov [rsp + count*8 + 4*8], arg
         ELSE
           movq [rsp + count*8 + 4*8], arg	;; pass low 64 bits of xmmx
         ENDIF

       ELSE					;; memory operand
         acc_used = 1

         IF SIZEOF (TYPE (arg)) EQ SIZEOF BYTE
           mov al, arg
           mov [rsp + count*8 + 4*8], al

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF WORD
           mov ax, arg
           mov [rsp + count*8 + 4*8], ax

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF DWORD
           mov eax, arg
           mov [rsp + count*8 + 4*8], eax

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF QWORD
           mov rax, arg
           mov [rsp + count*8 + 4*8], rax

         ELSE
           PUSHCONTEXT RADIX
          .RADIX 10
           $argnr TEXTEQU %argnr
%          ErrorEcho <invoke macro>, <invalid size of parameter $argnr>
           POPCONTEXT RADIX
         ENDIF
       ENDIF

     ELSE
       IF IsNoError ([arg])
         PUSHCONTEXT RADIX
        .RADIX 10		;; print the parameter number in decimal radix
         $argnr TEXTEQU %argnr
%        ErrorEcho <invoke macro>, <syntax error on parameter $argnr: use square brackets>
         POPCONTEXT RADIX
       ELSE
         ErrorEcho <invoke macro>, <invalid parameter: arg> ;; arg can be printed directly in this case
       ENDIF
     ENDIF

   ENDIF

 ENDM

 ;; pass 1st argument to RCX (R1) or XMM0
 ;; 2nd to RDX (R2) or XMM1
 ;; 3rd to R8 or XMM2
 ;; 4th to R9 or XMM3

 count = 0

%FOR arg, <args>

   IF acc_used
     FOR acc, <al,ah,ax,eax,rax>
%      IFIDNI <arg>, <acc>
%        ErrorEcho <invoke macro>, <register value of arg overwritten>  ;; arg can be printed directly in this case
         EXITM
       ENDIF
     ENDM
   ENDIF

   count = count + 1
   argnr = count

   regarg8	TEXTEQU @ArgI (count, <cl,dl,r8b,r9b>)
   regarg8alt	TEXTEQU @ArgI (count, <r1b,r2b,r8b,r9b>)
   regarg16	TEXTEQU @ArgI (count, <cx,dx,r8w,r9w>)
   regarg16alt	TEXTEQU @ArgI (count, <r1w,r2w,r8w,r9w>)
   regarg32	TEXTEQU @ArgI (count, <ecx,edx,r8d,r9d>)
   regarg32alt	TEXTEQU @ArgI (count, <r1d,r2d,r8d,r9d>)
   regarg64	TEXTEQU @ArgI (count, <rcx,rdx,r8,r9>)
   regarg64alt	TEXTEQU @ArgI (count, <r1,r2,r8,r9>)
   regarg128	TEXTEQU @ArgI (count, <xmm0,xmm1,xmm2,xmm3>)

   IF @SizeStr (<arg>) LT 5
     addrchars TEXTEQU <>
   ELSE
     addrchars SUBSTR <arg>, 1, 5
   ENDIF

   ;;;isinteger = 1			;; assume an integer parameter

   IFIDNI <ADDR >, addrchars
     paddr SUBSTR <arg>, 6

;     islocal = 0
;
;    %FOR localname, <fast_localnames>
;       IFIDNI @SubStr (localname, 2), paddr
;         islocal = 1
;         EXITM
;       ENDIF
;     ENDM
;
;     IF islocal
;       lea regarg64, [paddr]
;     ELSE
;       lea regarg64, paddr
;     ENDIF

     IF IsCodeLabel (paddr) OR IsDirMemAddr (paddr) OR IsExtLabel (paddr)
       lea regarg64, paddr
     ELSE
       lea regarg64, [paddr]
     ENDIF

   ELSE

     IF IsNoError (arg)

       IF IsImm (arg)
         mov regarg64, arg

       ELSE

         IF SIZEOF (TYPE (arg)) EQ SIZEOF BYTE
           IFDIFI regarg8, <arg>
             IFDIFI regarg8alt, <arg>
               mov regarg8, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF WORD
           IFDIFI regarg16, <arg>
             IFDIFI regarg16alt, <arg>
               mov regarg16, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF DWORD
           IFDIFI regarg32, <arg>
             IFDIFI regarg32alt, <arg>
               mov regarg32, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF QWORD
           IFDIFI regarg64, <arg>
             IFDIFI regarg64alt, <arg>
               mov regarg64, arg
             ENDIF
           ENDIF

         ELSE
           PUSHCONTEXT RADIX
          .RADIX 10
           $argnr TEXTEQU %argnr
%          ErrorEcho <invoke macro>, <invalid size of parameter $argnr>
           POPCONTEXT RADIX
         ENDIF
       ENDIF

     ELSE
       IF IsNoError ([arg])
         PUSHCONTEXT RADIX
        .RADIX 10
         $argnr TEXTEQU %argnr
%        ErrorEcho <invoke macro>, <syntax error on parameter $argnr: use square brackets>
         POPCONTEXT RADIX
       ELSE
         ErrorEcho <invoke macro>, <invalid parameter: arg> ;; arg can be printed directly in this case
       ENDIF
     ENDIF

   ENDIF

   IF count EQ 4	;; there are 4 register parameters
     EXITM
   ENDIF

 ENDM

 call name			; call the function

 ;; if even number of arguments, remove one more entry
 ;; note: for explanation of restoring the stack, see stack layouts at the top

 IF stackcount AND 1
   add rsp, 4*8 + stackcount*8
 ELSE
   add rsp, 4*8 + stackcount*8 + 8
 ENDIF

 pop rsp
ENDM

;invoke TEXTEQU <invoke>
Invoke TEXTEQU <invoke>
iNvoke TEXTEQU <invoke>
INvoke TEXTEQU <invoke>
inVoke TEXTEQU <invoke>
InVoke TEXTEQU <invoke>
iNVoke TEXTEQU <invoke>
INVoke TEXTEQU <invoke>
invOke TEXTEQU <invoke>
InvOke TEXTEQU <invoke>
iNvOke TEXTEQU <invoke>
INvOke TEXTEQU <invoke>
inVOke TEXTEQU <invoke>
InVOke TEXTEQU <invoke>
iNVOke TEXTEQU <invoke>
INVOke TEXTEQU <invoke>
invoKe TEXTEQU <invoke>
InvoKe TEXTEQU <invoke>
iNvoKe TEXTEQU <invoke>
INvoKe TEXTEQU <invoke>
inVoKe TEXTEQU <invoke>
InVoKe TEXTEQU <invoke>
iNVoKe TEXTEQU <invoke>
INVoKe TEXTEQU <invoke>
invOKe TEXTEQU <invoke>
InvOKe TEXTEQU <invoke>
iNvOKe TEXTEQU <invoke>
INvOKe TEXTEQU <invoke>
inVOKe TEXTEQU <invoke>
InVOKe TEXTEQU <invoke>
iNVOKe TEXTEQU <invoke>
INVOKe TEXTEQU <invoke>
invokE TEXTEQU <invoke>
InvokE TEXTEQU <invoke>
iNvokE TEXTEQU <invoke>
INvokE TEXTEQU <invoke>
inVokE TEXTEQU <invoke>
InVokE TEXTEQU <invoke>
iNVokE TEXTEQU <invoke>
INVokE TEXTEQU <invoke>
invOkE TEXTEQU <invoke>
InvOkE TEXTEQU <invoke>
iNvOkE TEXTEQU <invoke>
INvOkE TEXTEQU <invoke>
inVOkE TEXTEQU <invoke>
InVOkE TEXTEQU <invoke>
iNVOkE TEXTEQU <invoke>
INVOkE TEXTEQU <invoke>
invoKE TEXTEQU <invoke>
InvoKE TEXTEQU <invoke>
iNvoKE TEXTEQU <invoke>
INvoKE TEXTEQU <invoke>
inVoKE TEXTEQU <invoke>
InVoKE TEXTEQU <invoke>
iNVoKE TEXTEQU <invoke>
INVoKE TEXTEQU <invoke>
invOKE TEXTEQU <invoke>
InvOKE TEXTEQU <invoke>
iNvOKE TEXTEQU <invoke>
INvOKE TEXTEQU <invoke>
inVOKE TEXTEQU <invoke>
InVOKE TEXTEQU <invoke>
iNVOKE TEXTEQU <invoke>
INVOKE TEXTEQU <invoke>

; INVOKE

; Contrary to MSDN documentation, you can pass arrays by immediate value
; using this macro.

; A note about sign-extending:
; Quote from Matt Pietrek's article "Everything You Need To Know To Start
; Programming 64-Bit Windows Systems":
; "integer parameters that are less than 64-bits are sign extended"
; Because MSDN doesn't bear to that, the parameters are not sign extended.

invoke MACRO name:REQ, args:VARARG
LOCAL argnr, $argnr, count, stackcount
LOCAL arg, arglist
LOCAL acc, acc_used
LOCAL addrchars, paddr, localname;, islocal
LOCAL regarg8, regarg8alt
LOCAL regarg16, regarg16alt
LOCAL regarg32, regarg32alt
LOCAL regarg64, regarg64alt
LOCAL regarg128
LOCAL isinteger

 ;; a note: @ArgRev doesn't return empty string on empty input because
 ;; this way it is defined by Microsoft. If I would modify it, user's
 ;; includes maight contain the original @ArgRev macro which would redefine
 ;; the one modified by me back.

 IFNB <args>
   arglist TEXTEQU @ArgRev (args)	; reversed list of arguments
 ELSE
   arglist TEXTEQU <!<!>>
 ENDIF

 count = 0
 FOR arg, <args>			; count the arguments
   count = count + 1
 ENDM

 argnr = count				; number of last argument

 ; count contains real number of arguments passed on the stack

 IF (count-4) GE 0
   count = count - 4
 ELSE
   count = 0
 ENDIF

 stackcount = count

 ; note: when calling INVOKE, the stack has to be aligned at least to 8 bytes
 ; note: at least WinAPI calls always need 8-byte stack alignment at the entry

 ; note: for explanation of aligning the stack see stack layouts at the top
 ; if odd number of arguments, align the stack to 8 bytes using OR
 ; if even number of arguments, align the stack to 16 bytes using AND
 ; credit: the idea about how to align the stack comes from GoAsm

 push rsp
 push [rsp]
 IF stackcount AND 1
   or rsp, 16t/2
 ELSE
   and rsp, NOT (16t-1)
 ENDIF

 ;; reserve the space for arguments and spilling registers

 sub rsp, 4*8 + stackcount*8

 acc_used = 0

%FOR arg, arglist
   IFE count			;; no more arguments passed through the stack
     EXITM
   ENDIF

   count = count - 1		;; zero-based index
   argnr = argnr - 1		;; number of current argument

   ;; catch ADDR operator

   IF @SizeStr (<arg>) LT 5	;; "ADDR" has 4 chars
     addrchars TEXTEQU <>
   ELSE
     addrchars SUBSTR <arg>, 1, 5
   ENDIF

   IFIDNI <ADDR >, addrchars
     paddr SUBSTR <arg>, 6

;     islocal = 0
;
;    %FOR localname, <fast_localnames>
;       IFIDNI @SubStr (localname, 2), paddr
;         islocal = 1
;         EXITM
;       ENDIF
;     ENDM

;     IF islocal
;       lea rax, [paddr]				;; local variable
;     ELSE
;       ;; the following can't be used in ML64
;       ;; mov [rsp + count*8 + 4*8], OFFSET paddr
;       lea rax, paddr
;     ENDIF

     acc_used = 1

     IF IsCodeLabel (paddr) OR IsDirMemAddr (paddr) OR IsExtLabel (paddr)
       ;; the following can't be used in ML64
       ;; mov [rsp + count*8 + 4*8], OFFSET paddr
       lea rax, paddr
     ELSE
       lea rax, [paddr]				;; local variable
     ENDIF

     mov [rsp + count*8 + 4*8], rax

   ELSE

     IF IsNoError (arg)

       IF acc_used
         FOR acc, <al,ah,ax,eax,rax>
%          IFIDNI <arg>, <acc>
%            ErrorEcho <invoke macro>, <register value of arg overwritten>
             EXITM
           ENDIF
         ENDM
       ENDIF

       IF IsImm (arg)
         mov qword ptr [rsp + count*8 + 4*8], arg

       ELSEIF IsReg (arg)
         IF SIZEOF (TYPE (arg)) NE SIZEOF XMMWORD
           mov [rsp + count*8 + 4*8], arg
         ELSE
           movq [rsp + count*8 + 4*8], arg		;; pass low 64 bits
         ENDIF

       ELSE						;; memory operand
         acc_used = 1

         IF SIZEOF (TYPE (arg)) EQ SIZEOF BYTE
           mov al, arg
           mov [rsp + count*8 + 4*8], al

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF WORD
           mov ax, arg
           mov [rsp + count*8 + 4*8], ax

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF DWORD
           mov eax, arg
           mov [rsp + count*8 + 4*8], eax

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF QWORD
           mov rax, arg
           mov [rsp + count*8 + 4*8], rax

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF XMMWORD
           mov rax, qword ptr arg		; pass low 64-bit part
           mov [rsp + count*8 + 4*8], rax

         ELSE
           ErrorEcho <invoke macro>, <invalid size of arg parameter>
         ENDIF
       ENDIF

     ELSE
       IF IsNoError ([arg])
         PUSHCONTEXT RADIX
        .RADIX 10		;; print the parameter number in decimal radix
         $argnr TEXTEQU %argnr
%        ErrorEcho <invoke macro>, <syntax error on parameter $argnr: use square brackets>
         POPCONTEXT RADIX
       ELSE
         ErrorEcho <invoke macro>, <invalid parameter: arg>
       ENDIF
     ENDIF

   ENDIF

 ENDM

 ;; pass 1st argument to RCX (R1) or XMM0
 ;; 2nd to RDX (R2) or XMM1
 ;; 3rd to R8 or XMM2
 ;; 4th to R9 or XMM3

 count = 0

%FOR arg, <args>

   IF acc_used
     FOR acc, <al,ah,ax,eax,rax>
%      IFIDNI <arg>, <acc>
%        ErrorEcho <invoke macro>, <register value of arg overwritten>
         EXITM
       ENDIF
     ENDM
   ENDIF

   count = count + 1
   argnr = count

   regarg8	TEXTEQU @ArgI (count, <cl,dl,r8b,r9b>)
   regarg8alt	TEXTEQU @ArgI (count, <r1b,r2b,r8b,r9b>)
   regarg16	TEXTEQU @ArgI (count, <cx,dx,r8w,r9w>)
   regarg16alt	TEXTEQU @ArgI (count, <r1w,r2w,r8w,r9w>)
   regarg32	TEXTEQU @ArgI (count, <ecx,edx,r8d,r9d>)
   regarg32alt	TEXTEQU @ArgI (count, <r1d,r2d,r8d,r9d>)
   regarg64	TEXTEQU @ArgI (count, <rcx,rdx,r8,r9>)
   regarg64alt	TEXTEQU @ArgI (count, <r1,r2,r8,r9>)
   regarg128	TEXTEQU @ArgI (count, <xmm0,xmm1,xmm2,xmm3>)

   IF @SizeStr (<arg>) LT 5
     addrchars TEXTEQU <>
   ELSE
     addrchars SUBSTR <arg>, 1, 5
   ENDIF

   isinteger = 1			;; assume an integer parameter

   IFIDNI <ADDR >, addrchars
     paddr SUBSTR <arg>, 6

;     islocal = 0
;
;    %FOR localname, <fast_localnames>
;       IFIDNI @SubStr (localname, 2), paddr
;         islocal = 1
;         EXITM
;       ENDIF
;     ENDM
;
;     IF islocal
;       lea regarg64, [paddr]
;     ELSE
;       lea regarg64, paddr
;     ENDIF

     IF IsCodeLabel (paddr) OR IsDirMemAddr (paddr) OR IsExtLabel (paddr)
       lea regarg64, paddr
     ELSE
       lea regarg64, [paddr]
     ENDIF

   ELSE

     IF IsNoError (arg)

       IF IsImm (arg)
         mov regarg64, arg

       ELSE

         IF SIZEOF (TYPE (arg)) EQ SIZEOF BYTE
           IFDIFI regarg8, <arg>
             IFDIFI regarg8alt, <arg>
               mov regarg8, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF WORD
           IFDIFI regarg16, <arg>
             IFDIFI regarg16alt, <arg>
               mov regarg16, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF DWORD
           IFDIFI regarg32, <arg>
             IFDIFI regarg32alt, <arg>
               mov regarg32, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF QWORD
           IFDIFI regarg64, <arg>
             IFDIFI regarg64alt, <arg>
               mov regarg64, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF XMMWORD
           isinteger = 0

           IF TYPE (arg) EQ TYPE XMMWORD
             movq regarg128, qword ptr arg	; pass low 64-bit part
           ELSE
             IFDIFI regarg128, <arg>
               movq regarg128, arg		; pass low 64-bit part
             ENDIF
           ENDIF
         ELSE
           ErrorEcho <invoke macro>, <invalid size of arg parameter>
         ENDIF
       ENDIF

     ELSE
       IF IsNoError ([arg])
         PUSHCONTEXT RADIX
        .RADIX 10
         $argnr TEXTEQU %argnr
%        ErrorEcho <invoke macro>, <syntax error on parameter $argnr: use square brackets>
         POPCONTEXT RADIX
       ELSE
         ErrorEcho <invoke macro>, <invalid parameter: arg>
       ENDIF
     ENDIF

   ENDIF

   IF FAST_SPILL
     IF isinteger
       mov [rsp+(count-1)*8], regarg64
     ELSE
       movq qword ptr [rsp+(count-1)*8], regarg128
     ENDIF
   ENDIF

   IF count EQ 4	;; there are 4 register parameters
     EXITM
   ENDIF

 ENDM

 call name			; call the function

 ;; if even number of arguments, remove one more entry
 ;; note: for explanation of restoring the stack, see stack layouts at the top

 IF stackcount AND 1
   add rsp, 4*8 + stackcount*8
 ELSE
   add rsp, 4*8 + stackcount*8 + 8
 ENDIF

 pop rsp
ENDM

;invoke TEXTEQU <invoke>
Invoke TEXTEQU <invoke>
iNvoke TEXTEQU <invoke>
INvoke TEXTEQU <invoke>
inVoke TEXTEQU <invoke>
InVoke TEXTEQU <invoke>
iNVoke TEXTEQU <invoke>
INVoke TEXTEQU <invoke>
invOke TEXTEQU <invoke>
InvOke TEXTEQU <invoke>
iNvOke TEXTEQU <invoke>
INvOke TEXTEQU <invoke>
inVOke TEXTEQU <invoke>
InVOke TEXTEQU <invoke>
iNVOke TEXTEQU <invoke>
INVOke TEXTEQU <invoke>
invoKe TEXTEQU <invoke>
InvoKe TEXTEQU <invoke>
iNvoKe TEXTEQU <invoke>
INvoKe TEXTEQU <invoke>
inVoKe TEXTEQU <invoke>
InVoKe TEXTEQU <invoke>
iNVoKe TEXTEQU <invoke>
INVoKe TEXTEQU <invoke>
invOKe TEXTEQU <invoke>
InvOKe TEXTEQU <invoke>
iNvOKe TEXTEQU <invoke>
INvOKe TEXTEQU <invoke>
inVOKe TEXTEQU <invoke>
InVOKe TEXTEQU <invoke>
iNVOKe TEXTEQU <invoke>
INVOKe TEXTEQU <invoke>
invokE TEXTEQU <invoke>
InvokE TEXTEQU <invoke>
iNvokE TEXTEQU <invoke>
INvokE TEXTEQU <invoke>
inVokE TEXTEQU <invoke>
InVokE TEXTEQU <invoke>
iNVokE TEXTEQU <invoke>
INVokE TEXTEQU <invoke>
invOkE TEXTEQU <invoke>
InvOkE TEXTEQU <invoke>
iNvOkE TEXTEQU <invoke>
INvOkE TEXTEQU <invoke>
inVOkE TEXTEQU <invoke>
InVOkE TEXTEQU <invoke>
iNVOkE TEXTEQU <invoke>
INVOkE TEXTEQU <invoke>
invoKE TEXTEQU <invoke>
InvoKE TEXTEQU <invoke>
iNvoKE TEXTEQU <invoke>
INvoKE TEXTEQU <invoke>
inVoKE TEXTEQU <invoke>
InVoKE TEXTEQU <invoke>
iNVoKE TEXTEQU <invoke>
INVoKE TEXTEQU <invoke>
invOKE TEXTEQU <invoke>
InvOKE TEXTEQU <invoke>
iNvOKE TEXTEQU <invoke>
INvOKE TEXTEQU <invoke>
inVOKE TEXTEQU <invoke>
InVOKE TEXTEQU <invoke>
iNVOKE TEXTEQU <invoke>
INVOKE TEXTEQU <invoke>

; INVOKE

; Contrary to MSDN documentation, you can pass arrays by immediate value
; using this macro.

; A note about sign-extending:
; Quote from Matt Pietrek's article "Everything You Need To Know To Start
; Programming 64-Bit Windows Systems":
; "integer parameters that are less than 64-bits are sign extended"
; Because MSDN doesn't bear to that, the parameters are not sign extended.

invoke MACRO name:REQ, args:VARARG
LOCAL argnr, $argnr, count, stackcount
LOCAL arg, arglist
LOCAL acc, acc_used
LOCAL addrchars, paddr, localname;, islocal
LOCAL regarg8, regarg8alt
LOCAL regarg16, regarg16alt
LOCAL regarg32, regarg32alt
LOCAL regarg64, regarg64alt
LOCAL regarg128
;;;LOCAL isinteger

 ;; a note: @ArgRev doesn't return empty string on empty input because
 ;; this way it is defined by Microsoft. If I would modify it, user's
 ;; includes maight contain the original @ArgRev macro which would redefine
 ;; the one modified by me back.

 IFNB <args>
   arglist TEXTEQU @ArgRev (args)	; reversed list of arguments
 ELSE
   arglist TEXTEQU <!<!>>
 ENDIF

 count = 0
 FOR arg, <args>			; count the arguments
   count = count + 1
 ENDM

 argnr = count				; number of last argument

 ; count contains real number of arguments passed on the stack

 IF (count-4) GE 0
   count = count - 4
 ELSE
   count = 0
 ENDIF

 stackcount = count

 ; note: when calling INVOKE, the stack has to be aligned at least to 8 bytes
 ; note: at least WinAPI calls always need 8-byte stack alignment at the entry

 ; note: for explanation of aligning the stack see stack layouts at the top
 ; if odd number of arguments, align the stack to 8 bytes using OR
 ; if even number of arguments, align the stack to 16 bytes using AND
 ; credit: the idea about how to align the stack comes from GoAsm

 push rsp
 push [rsp]
 IF stackcount AND 1
   or rsp, 16t/2
 ELSE
   and rsp, NOT (16t-1)
 ENDIF

 ;; reserve the space for arguments and spilling registers

 sub rsp, 4*8 + stackcount*8

 acc_used = 0

%FOR arg, arglist
   IFE count			;; no more arguments passed through the stack
     EXITM
   ENDIF

   count = count - 1		;; zero-based index
   argnr = argnr - 1		;; number of current argument

   ;; catch ADDR operator

   IF @SizeStr (<arg>) LT 5	;; "ADDR" has 4 chars
     addrchars TEXTEQU <>
   ELSE
     addrchars SUBSTR <arg>, 1, 5
   ENDIF

   IFIDNI <ADDR >, addrchars
     paddr SUBSTR <arg>, 6

;     islocal = 0
;
;    %FOR localname, <fast_localnames>
;       IFIDNI @SubStr (localname, 2), paddr
;         islocal = 1
;         EXITM
;       ENDIF
;     ENDM

;     IF islocal
;       lea rax, [paddr]				;; local variable
;     ELSE
;       ;; the following can't be used in ML64
;       ;; mov [rsp + count*8 + 4*8], OFFSET paddr
;       lea rax, paddr
;     ENDIF

     acc_used = 1

     IF IsCodeLabel (paddr) OR IsDirMemAddr (paddr) OR IsExtLabel (paddr)
       ;; the following can't be used in ML64
       ;; mov [rsp + count*8 + 4*8], OFFSET paddr
       lea rax, paddr
     ELSE
       lea rax, [paddr]				;; local variable
     ENDIF

     mov [rsp + count*8 + 4*8], rax

   ELSE

     IF IsNoError (arg)

       IF acc_used
         FOR acc, <al,ah,ax,eax,rax>
%          IFIDNI <arg>, <acc>
%            ErrorEcho <invoke macro>, <register value of arg overwritten>
             EXITM
           ENDIF
         ENDM
       ENDIF

       IF IsImm (arg)
         mov qword ptr [rsp + count*8 + 4*8], arg

       ELSEIF IsReg (arg)
         IF SIZEOF (TYPE (arg)) NE SIZEOF XMMWORD
           mov [rsp + count*8 + 4*8], arg
         ELSE
           movq [rsp + count*8 + 4*8], arg		;; pass low 64 bits
         ENDIF

       ELSE						;; memory operand
         acc_used = 1

         IF SIZEOF (TYPE (arg)) EQ SIZEOF BYTE
           mov al, arg
           mov [rsp + count*8 + 4*8], al

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF WORD
           mov ax, arg
           mov [rsp + count*8 + 4*8], ax

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF DWORD
           mov eax, arg
           mov [rsp + count*8 + 4*8], eax

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF QWORD
           mov rax, arg
           mov [rsp + count*8 + 4*8], rax

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF XMMWORD
           mov rax, qword ptr arg		; pass low 64-bit part
           mov [rsp + count*8 + 4*8], rax

         ELSE
           PUSHCONTEXT RADIX
          .RADIX 10
           $argnr TEXTEQU %argnr
%          ErrorEcho <invoke macro>, <invalid size of parameter $argnr>
           POPCONTEXT RADIX
         ENDIF
       ENDIF

     ELSE
       IF IsNoError ([arg])
         PUSHCONTEXT RADIX
        .RADIX 10		;; print the parameter number in decimal radix
         $argnr TEXTEQU %argnr
%        ErrorEcho <invoke macro>, <syntax error on parameter $argnr: use square brackets>
         POPCONTEXT RADIX
       ELSE
         ErrorEcho <invoke macro>, <invalid parameter: arg>
       ENDIF
     ENDIF

   ENDIF

 ENDM

 ;; pass 1st argument to RCX (R1) or XMM0
 ;; 2nd to RDX (R2) or XMM1
 ;; 3rd to R8 or XMM2
 ;; 4th to R9 or XMM3

 count = 0

%FOR arg, <args>

   IF acc_used
     FOR acc, <al,ah,ax,eax,rax>
%      IFIDNI <arg>, <acc>
%        ErrorEcho <invoke macro>, <register value of arg overwritten>
         EXITM
       ENDIF
     ENDM
   ENDIF

   count = count + 1
   argnr = count

   regarg8	TEXTEQU @ArgI (count, <cl,dl,r8b,r9b>)
   regarg8alt	TEXTEQU @ArgI (count, <r1b,r2b,r8b,r9b>)
   regarg16	TEXTEQU @ArgI (count, <cx,dx,r8w,r9w>)
   regarg16alt	TEXTEQU @ArgI (count, <r1w,r2w,r8w,r9w>)
   regarg32	TEXTEQU @ArgI (count, <ecx,edx,r8d,r9d>)
   regarg32alt	TEXTEQU @ArgI (count, <r1d,r2d,r8d,r9d>)
   regarg64	TEXTEQU @ArgI (count, <rcx,rdx,r8,r9>)
   regarg64alt	TEXTEQU @ArgI (count, <r1,r2,r8,r9>)
   regarg128	TEXTEQU @ArgI (count, <xmm0,xmm1,xmm2,xmm3>)

   IF @SizeStr (<arg>) LT 5
     addrchars TEXTEQU <>
   ELSE
     addrchars SUBSTR <arg>, 1, 5
   ENDIF

   ;;;isinteger = 1			;; assume an integer parameter

   IFIDNI <ADDR >, addrchars
     paddr SUBSTR <arg>, 6

;     islocal = 0
;
;    %FOR localname, <fast_localnames>
;       IFIDNI @SubStr (localname, 2), paddr
;         islocal = 1
;         EXITM
;       ENDIF
;     ENDM
;
;     IF islocal
;       lea regarg64, [paddr]
;     ELSE
;       lea regarg64, paddr
;     ENDIF

     IF IsCodeLabel (paddr) OR IsDirMemAddr (paddr) OR IsExtLabel (paddr)
       lea regarg64, paddr
     ELSE
       lea regarg64, [paddr]
     ENDIF

   ELSE

     IF IsNoError (arg)

       IF IsImm (arg)
         mov regarg64, arg

       ELSE

         IF SIZEOF (TYPE (arg)) EQ SIZEOF BYTE
           IFDIFI regarg8, <arg>
             IFDIFI regarg8alt, <arg>
               mov regarg8, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF WORD
           IFDIFI regarg16, <arg>
             IFDIFI regarg16alt, <arg>
               mov regarg16, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF DWORD
           IFDIFI regarg32, <arg>
             IFDIFI regarg32alt, <arg>
               mov regarg32, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF QWORD
           IFDIFI regarg64, <arg>
             IFDIFI regarg64alt, <arg>
               mov regarg64, arg
             ENDIF
           ENDIF

         ELSEIF SIZEOF (TYPE (arg)) EQ SIZEOF XMMWORD
           ;;;isinteger = 0

           IF TYPE (arg) EQ TYPE XMMWORD
             movq regarg128, qword ptr arg	; pass low 64-bit part
           ELSE
             IFDIFI regarg128, <arg>
               movq regarg128, arg		; pass low 64-bit part
             ENDIF
           ENDIF
         ELSE
           ErrorEcho <invoke macro>, <invalid size of arg parameter>
         ENDIF
       ENDIF

     ELSE
       IF IsNoError ([arg])
         PUSHCONTEXT RADIX
        .RADIX 10
         $argnr TEXTEQU %argnr
%        ErrorEcho <invoke macro>, <syntax error on parameter $argnr: use square brackets>
         POPCONTEXT RADIX
       ELSE
         ErrorEcho <invoke macro>, <invalid parameter: arg>
       ENDIF
     ENDIF

   ENDIF

   IF count EQ 4	;; there are 4 register parameters
     EXITM
   ENDIF

 ENDM

 call name			; call the function

 ;; if even number of arguments, remove one more entry
 ;; note: for explanation of restoring the stack, see stack layouts at the top

 IF stackcount AND 1
   add rsp, 4*8 + stackcount*8
 ELSE
   add rsp, 4*8 + stackcount*8 + 8
 ENDIF

 pop rsp
ENDM

;invoke TEXTEQU <invoke>
Invoke TEXTEQU <invoke>
iNvoke TEXTEQU <invoke>
INvoke TEXTEQU <invoke>
inVoke TEXTEQU <invoke>
InVoke TEXTEQU <invoke>
iNVoke TEXTEQU <invoke>
INVoke TEXTEQU <invoke>
invOke TEXTEQU <invoke>
InvOke TEXTEQU <invoke>
iNvOke TEXTEQU <invoke>
INvOke TEXTEQU <invoke>
inVOke TEXTEQU <invoke>
InVOke TEXTEQU <invoke>
iNVOke TEXTEQU <invoke>
INVOke TEXTEQU <invoke>
invoKe TEXTEQU <invoke>
InvoKe TEXTEQU <invoke>
iNvoKe TEXTEQU <invoke>
INvoKe TEXTEQU <invoke>
inVoKe TEXTEQU <invoke>
InVoKe TEXTEQU <invoke>
iNVoKe TEXTEQU <invoke>
INVoKe TEXTEQU <invoke>
invOKe TEXTEQU <invoke>
InvOKe TEXTEQU <invoke>
iNvOKe TEXTEQU <invoke>
INvOKe TEXTEQU <invoke>
inVOKe TEXTEQU <invoke>
InVOKe TEXTEQU <invoke>
iNVOKe TEXTEQU <invoke>
INVOKe TEXTEQU <invoke>
invokE TEXTEQU <invoke>
InvokE TEXTEQU <invoke>
iNvokE TEXTEQU <invoke>
INvokE TEXTEQU <invoke>
inVokE TEXTEQU <invoke>
InVokE TEXTEQU <invoke>
iNVokE TEXTEQU <invoke>
INVokE TEXTEQU <invoke>
invOkE TEXTEQU <invoke>
InvOkE TEXTEQU <invoke>
iNvOkE TEXTEQU <invoke>
INvOkE TEXTEQU <invoke>
inVOkE TEXTEQU <invoke>
InVOkE TEXTEQU <invoke>
iNVOkE TEXTEQU <invoke>
INVOkE TEXTEQU <invoke>
invoKE TEXTEQU <invoke>
InvoKE TEXTEQU <invoke>
iNvoKE TEXTEQU <invoke>
INvoKE TEXTEQU <invoke>
inVoKE TEXTEQU <invoke>
InVoKE TEXTEQU <invoke>
iNVoKE TEXTEQU <invoke>
INVoKE TEXTEQU <invoke>
invOKE TEXTEQU <invoke>
InvOKE TEXTEQU <invoke>
iNvOKE TEXTEQU <invoke>
INvOKE TEXTEQU <invoke>
inVOKE TEXTEQU <invoke>
InVOKE TEXTEQU <invoke>
iNVOKE TEXTEQU <invoke>
INVOKE TEXTEQU <invoke>

ENDIF	; NDEF FASTPROCS
