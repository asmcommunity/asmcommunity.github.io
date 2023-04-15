;*****************************************************************************
; Test
;
;
;*****************************************************************************

			.586
			.MODEL flat, stdcall
			OPTION casemap :none

;*****************************************************************************
;			INCLUDE FILES
;*****************************************************************************
INCLUDE \MASM32\INCLUDE\windows.inc
INCLUDE \MASM32\INCLUDE\user32.inc
INCLUDE \MASM32\INCLUDE\kernel32.inc
INCLUDE \MASM32\INCLUDE\msvcrt.inc

INCLUDELIB \MASM32\LIB\user32.lib
INCLUDELIB \MASM32\LIB\kernel32.lib
INCLUDELIB \MASM32\LIB\msvcrt.lib

;*****************************************************************************
;			PROTOTYPES
;*****************************************************************************
Compare			PROTO	:DWORD, :DWORD

;*****************************************************************************
			.CONST
;*****************************************************************************

;*****************************************************************************
;			STRUCTURES
;*****************************************************************************
ItemStruct STRUCT
  item_id	DD	?
  item_value	DD	?
ItemStruct ENDS

;*****************************************************************************
;			MACROS
;*****************************************************************************
CTEXT MACRO text:VARARG
  LOCAL TxtName
  .DATA
    TxtName BYTE text, 0
  .CODE
  EXITM <ADDR TxtName>
ENDM

;*****************************************************************************
			.DATA
;*****************************************************************************
ItemArray		ItemStruct	10 DUP (<1,11>,<4,14>,<5,15>,<9,19>,<3,13>,<7,11>,<6,16>,<2,12>,<8,18>,<10,20>)

;*****************************************************************************
			.DATA?
;*****************************************************************************

;*****************************************************************************
			.CODE
;*****************************************************************************
Start:
	invoke	crt_qsort, ADDR ItemArray, 10, SIZEOF DWORD, OFFSET Compare

	invoke	MessageBox, 0, CTEXT("qsort done!"), CTEXT("qsort"), MB_OK

	invoke	ExitProcess, NULL

;*******************************************************************************
; Compare
;
;
;*******************************************************************************
Compare			PROC	USES esi edi, e1:DWORD, e2:DWORD

			LOCAL	szMessage[128]:BYTE

	mov	esi, e1
	assume	esi:PTR ItemStruct
	mov	esi, [esi].item_value
	mov	edi, e2
	assume	edi:PTR ItemStruct
	mov	edi, [edi].item_value

;	pushad
;	invoke	wsprintf, ADDR szMessage, CTEXT("1: %d     2: %d"), esi, edi
;	invoke	MessageBox, 0, ADDR szMessage, 0, 0
;	popad

	.if (esi > edi)
		mov	eax, 1
	.elseif (esi < edi)
		mov	eax, -1
	.else
		xor	eax, eax
	.endif

	assume	edi:NOTHING
	assume	esi:NOTHING

	ret

Compare			ENDP

;*******************************************************************************

			END	Start
