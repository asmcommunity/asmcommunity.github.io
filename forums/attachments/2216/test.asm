.686
.MMX
.model flat,stdcall
option casemap:none
option proc:private

incAPI MACRO files:VARARG
	FOR file, <files>
		include		file&.inc
		includelib	file&.lib
	ENDM
ENDM

CTEXT MACRO y:VARARG
	LOCAL sym

	CONST segment
		IFIDNI <y>,<>
			sym db 0
		ELSE
			sym db y,0
		ENDIF
	CONST ends

	EXITM <OFFSET sym>
ENDM

include		<windows.inc>
incAPI		<kernel32,user32>

ASSUME FS:NOTHING


;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; DATA? section
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.data?


;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; DATA section
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.data
str1 db "abc-abcd-abcde-abcdef-abcdefg-abcdefgh-abcdefghi,abc-labcd-abcde-abcdef-abcdefg-abcdefgh-abcdefghi,abc-abcd-abcde-abcdef-abcdefg-abcdefgh-abcdefghi",9,"//Comment",0
str2 db 256 dup (0)


;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; CODE section
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.code


;------------------------------------
; if (no Term found) Returns (NULL)
; else Returns (Pointer after Term)
;------------------------------------
;; char *p
;; p = NNStrCpy(&Dest,&Source,',');
;; if (p) {
;; }
; extern "C" char * __stdcall NNStrCpy(char *pszDest,char *pszSource,char chTerm);
OPTION PROLOGUE:NONE
OPTION EPILOGUE:NONE
NNStrCpy proc pszDest:PTR BYTE,pszSource:PTR BYTE,chTerm:BYTE
	$pszDest EQU <[esp+1*4]>
	$pszSource EQU <[esp+2*4]>
	$chTerm EQU <byte ptr [esp+3*4]>
	movzx ecx,$chTerm
	mov eax,$pszSource
	mov edx,$pszDest
	pxor mm2,mm2
	movd MM3,ecx
	punpcklbw MM3,MM3
	punpcklwd MM3,MM3
	punpckldq MM3,MM3
@@TestAlignment:
	test eax,7
	jnz @@UnAlignedOrDoByte
@@AlignedAt8:
	movq MM0,[eax]
	movq MM1,MM0
	pcmpeqb MM0,MM3
	packsswb MM0,MM0
	movd ecx,MM0
	movq MM0,MM1
	test ecx,ecx; Found Term byte?
	jnz @@UnAlignedOrDoByte
	pcmpeqb MM0,mm2
	packsswb MM0,MM0
	movd ecx,MM0
	test ecx,ecx; Found NULL byte?
	jnz @@UnAlignedOrDoByte
	movq [edx],MM1
	add eax,8
	add edx,8
	jmp @@AlignedAt8
@@UnAlignedOrDoByte:	
	mov cl,[eax]
	inc eax
	cmp cl,$chTerm
	je @@Done 
	mov [edx],cl
	inc edx
	test cl,cl
	jnz @@TestAlignment
	dec edx
	xor eax,eax 
@@Done:
	mov byte ptr [edx],0
	ret 3*4
NNStrCpy endp
OPTION PROLOGUE:PrologueDef
OPTION EPILOGUE:EpilogueDef


ENTRY32:
	invoke	NNStrCpy, addr str1, addr str2, ','
	invoke	MessageBox, 0, addr str2, CTEXT("Drizz NNStrCpy"), MB_OK
	invoke	ExitProcess, 0


END ENTRY32
