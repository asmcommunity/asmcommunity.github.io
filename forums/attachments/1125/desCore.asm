COMMENT	* Copyright © 2004 John Weeks johnweeks@prodigy.net                                                                 #
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*


					TITLE			Des Encription
					SUBTITLE		Main Functions
					.NOLISTMACRO
					.386
					.MODEL		FLAT,STDCALL
					OPTION		CASEMAP:NONE

;INCLUDE	../common/des.inc

FirstPermuteFunc1	MACRO	SrcReq:REQ,DestReg:REQ,Bit:REQ,Value:REQ
					LOCAL	P10
					mov		ebx,Bit
					xor		DestReg,DestReg
					test	SrcReq,Value
					jz		P10
					mov		DestReg,ebx
P10:
ENDM
PermuteFunc1		MACRO	SrcReq:REQ,DestReg:REQ,Value:REQ
					LOCAL	P10
					add		ebx,ebx
					test	SrcReq,Value
					jz		P10
					or		DestReg,ebx
P10:
ENDM
FirstPermuteFunc2	MACRO	SrcReq:REQ,DestReg:REQ,Value:REQ
					LOCAL	P10
					xor		DestReg,DestReg
					test	SrcReq,Value
					jz		P10
					mov		DestReg,ebx
P10:
ENDM
PermuteFunc2			MACRO	SrcReq:REQ,DestReg:REQ,Value:REQ
					LOCAL	P10
					test	SrcReq,Value
					jz		P10
					or		DestReg,ebx
P10:
ENDM

; The cmov instruction is slightly faster on intel but a lot slower on amd
; and so it wasn't worth the effort to lose 386 instruction compatibility

;FirstPermuteFunc1	MACRO	SrcReq:REQ,DestReg:REQ,Bit:REQ,Value:REQ
;					xor		ecx,ecx
;					mov		ebx,Bit
;					test	SrcReq,Value
;					cmovnz	ecx,ebx
;					mov		DestReg,ecx
;ENDM
;PermuteFunc1		MACRO	SrcReq:REQ,DestReg:REQ,Value:REQ
;					xor		ecx,ecx
;					add		ebx,ebx
;					test	SrcReq,Value
;					cmovnz	ecx,ebx
;					or		DestReg,ecx
;ENDM
;FirstPermuteFunc2	MACRO	SrcReq:REQ,DestReg:REQ,Value:REQ
;					xor		ecx,ecx
;					test	SrcReq,Value
;					cmovnz	ecx,ebx
;					mov		DestReg,ecx
;ENDM
;PermuteFunc2		MACRO	SrcReq:REQ,DestReg:REQ,Value:REQ
;					xor		ecx,ecx
;					test	SrcReq,Value
;					cmovnz	ecx,ebx
;					or		DestReg,ecx
;ENDM



InitalPermute			MACRO
					FirstPermuteFunc1		<edx>,<esi>,01h,00000001000000000000000000000000b
					FirstPermuteFunc2		<edx>,<ebp>,	00000010000000000000000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000000010000000000000000b
					PermuteFunc2			<edx>,<ebp>,	00000000000000100000000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000000000000000100000000b
					PermuteFunc2			<edx>,<ebp>,	00000000000000000000001000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000000000000000000000001b
					PermuteFunc2			<edx>,<ebp>,	00000000000000000000000000000010b

					PermuteFunc1			<eax>,<esi>,	00000001000000000000000000000000b
					PermuteFunc2			<eax>,<ebp>,	00000010000000000000000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000000010000000000000000b
					PermuteFunc2			<eax>,<ebp>,	00000000000000100000000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000000000000000100000000b
					PermuteFunc2			<eax>,<ebp>,	00000000000000000000001000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000000000000000000000001b
					PermuteFunc2			<eax>,<ebp>,	00000000000000000000000000000010b

					
					PermuteFunc1			<edx>,<esi>,	00000100000000000000000000000000b
					PermuteFunc2			<edx>,<ebp>,	00001000000000000000000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000001000000000000000000b
					PermuteFunc2			<edx>,<ebp>,	00000000000010000000000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000000000000010000000000b
					PermuteFunc2			<edx>,<ebp>,	00000000000000000000100000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000000000000000000000100b
					PermuteFunc2			<edx>,<ebp>,	00000000000000000000000000001000b

					PermuteFunc1			<eax>,<esi>,	00000100000000000000000000000000b
					PermuteFunc2			<eax>,<ebp>,	00001000000000000000000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000001000000000000000000b
					PermuteFunc2			<eax>,<ebp>,	00000000000010000000000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000000000000010000000000b
					PermuteFunc2			<eax>,<ebp>,	00000000000000000000100000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000000000000000000000100b
					PermuteFunc2			<eax>,<ebp>,	00000000000000000000000000001000b

					PermuteFunc1			<edx>,<esi>,	00010000000000000000000000000000b
					PermuteFunc2			<edx>,<ebp>,	00100000000000000000000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000100000000000000000000b
					PermuteFunc2			<edx>,<ebp>,	00000000001000000000000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000000000001000000000000b
					PermuteFunc2			<edx>,<ebp>,	00000000000000000010000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000000000000000000010000b
					PermuteFunc2			<edx>,<ebp>,	00000000000000000000000000100000b

					PermuteFunc1			<eax>,<esi>,	00010000000000000000000000000000b
					PermuteFunc2			<eax>,<ebp>,	00100000000000000000000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000100000000000000000000b
					PermuteFunc2			<eax>,<ebp>,	00000000001000000000000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000000000001000000000000b
					PermuteFunc2			<eax>,<ebp>,	00000000000000000010000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000000000000000000010000b
					PermuteFunc2			<eax>,<ebp>,	00000000000000000000000000100000b

					PermuteFunc1			<edx>,<esi>,	01000000000000000000000000000000b
					PermuteFunc2			<edx>,<ebp>,	10000000000000000000000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000010000000000000000000000b
					PermuteFunc2			<edx>,<ebp>,	00000000100000000000000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000000000100000000000000b
					PermuteFunc2			<edx>,<ebp>,	00000000000000001000000000000000b
					PermuteFunc1			<edx>,<esi>,	00000000000000000000000001000000b
					PermuteFunc2			<edx>,<ebp>,	00000000000000000000000010000000b

					PermuteFunc1			<eax>,<esi>,	01000000000000000000000000000000b
					PermuteFunc2			<eax>,<ebp>,	10000000000000000000000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000010000000000000000000000b
					PermuteFunc2			<eax>,<ebp>,	00000000100000000000000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000000000100000000000000b
					PermuteFunc2			<eax>,<ebp>,	00000000000000001000000000000000b
					PermuteFunc1			<eax>,<esi>,	00000000000000000000000001000000b
					PermuteFunc2			<eax>,<ebp>,	00000000000000000000000010000000b
ENDM
Expand				MACRO
					mov	edx,ebp								;L=0.5	T=0.5	E=ALU
					rol ebp,1								;L=4	T=1		E=
					ror edx,15
					mov	eax,ebp								;L=0.5	T=0.5	E=ALU
					add	eax,eax								;L=0.5	T=0.5	E=ALU
					add eax,eax								;L=0.5	T=0.5	E=ALU
					mov	ebx,eax								;L=0.5	T=0.5	E=ALU
					and	ebp,111111b
					and	eax,111111000000b				

					or	eax,ebp								;L=0.5	T=0.5	E=ALU
					add	ebx,ebx								;L=0.5	T=0.5	E=ALU
					add	ebx,ebx								;L=0.5	T=0.5	E=ALU
					mov ebp,ebx								;L=0.5	T=0.5	E=ALU
					add	ebp,ebp								;L=0.5	T=0.5	E=ALU
					add	ebp,ebp								;L=0.5	T=0.5	E=ALU
					and	ebx,111111000000000000b
					and	ebp,111111000000000000000000b

					or	eax,ebx								;L=0.5	T=0.5	E=ALU
					or	eax,ebp								;L=0.5	T=0.5	E=ALU

					mov	ebx,edx
					add	ebx,ebx
					add ebx,ebx
					mov	ebp,ebx
					and	edx,111111b
					and	ebx,111111000000b
					or	edx,ebx
					add	ebp,ebp
					add	ebp,ebp
					mov	ebx,ebp
					add	ebx,ebx
					add	ebx,ebx
					and	ebp,111111000000000000b
					and	ebx,111111000000000000000000b
					or	edx,ebp
					or	edx,ebx
ENDM
Subtitution			MACRO
					TableOffset=0

					mov	ebx,edx
					shr	ebx,18
					and	ebx,0111111b
					mov	ebp,[SLoc+ebx*4+TableOffset]
					TableOffset=TableOffset+256

					mov	ebx,edx
					shr	ebx,12
					and	ebx,0111111b
					or	ebp,[SLoc+ebx*4+TableOffset]
					TableOffset=TableOffset+256

					mov	ebx,edx
					shr	ebx,6
					and	ebx,0111111b
					or	ebp,[SLoc+ebx*4+TableOffset]
					TableOffset=TableOffset+256

					and	edx,0111111b
					or	ebp,[SLoc+edx*4+TableOffset]
					TableOffset=TableOffset+256

					mov	ebx,eax
					shr	ebx,18
					and	ebx,0111111b
					or	ebp,[SLoc+ebx*4+TableOffset]
					TableOffset=TableOffset+256

					mov	ebx,eax
					shr	ebx,12
					and	ebx,0111111b
					or	ebp,[SLoc+ebx*4+TableOffset]
					TableOffset=TableOffset+256

					mov	ebx,eax
					shr	ebx,6
					and	ebx,0111111b
					or	ebp,[SLoc+ebx*4+TableOffset]
					TableOffset=TableOffset+256

					and	eax,0111111b
					or	ebp,[SLoc+eax*4+TableOffset]
ENDM
FinalPermute			MACRO
					FirstPermuteFunc1		<ebp>,<edx>,1b	,1000b
					FirstPermuteFunc2		<ebp>,<eax>,10000000b
					PermuteFunc1			<ecx>,<edx>,1000b
					PermuteFunc2			<ecx>,<eax>,10000000b
					PermuteFunc1			<ebp>,<edx>,100000000000b
					PermuteFunc2			<ebp>,<eax>,1000000000000000b
					PermuteFunc1			<ecx>,<edx>,100000000000b
					PermuteFunc2			<ecx>,<eax>,1000000000000000b
					PermuteFunc1			<ebp>,<edx>,10000000000000000000b
					PermuteFunc2			<ebp>,<eax>,100000000000000000000000b
					PermuteFunc1			<ecx>,<edx>,10000000000000000000b
					PermuteFunc2			<ecx>,<eax>,100000000000000000000000b
					PermuteFunc1			<ebp>,<edx>,1000000000000000000000000000b
					PermuteFunc2			<ebp>,<eax>,10000000000000000000000000000000b
					PermuteFunc1			<ecx>,<edx>,1000000000000000000000000000b
					PermuteFunc2			<ecx>,<eax>,10000000000000000000000000000000b
					PermuteFunc1			<ebp>,<edx>,100b
					PermuteFunc2			<ebp>,<eax>,1000000b
					PermuteFunc1			<ecx>,<edx>,100b
					PermuteFunc2			<ecx>,<eax>,1000000b
					PermuteFunc1			<ebp>,<edx>,10000000000b
					PermuteFunc2			<ebp>,<eax>,100000000000000b
					PermuteFunc1			<ecx>,<edx>,10000000000b
					PermuteFunc2			<ecx>,<eax>,100000000000000b
					PermuteFunc1			<ebp>,<edx>,1000000000000000000b
					PermuteFunc2			<ebp>,<eax>,10000000000000000000000b
					PermuteFunc1			<ecx>,<edx>,1000000000000000000b
					PermuteFunc2			<ecx>,<eax>,10000000000000000000000b
					PermuteFunc1			<ebp>,<edx>,100000000000000000000000000b
					PermuteFunc2			<ebp>,<eax>,1000000000000000000000000000000b
					PermuteFunc1			<ecx>,<edx>,100000000000000000000000000b
					PermuteFunc2			<ecx>,<eax>,1000000000000000000000000000000b
					PermuteFunc1			<ebp>,<edx>,10b
					PermuteFunc2			<ebp>,<eax>,100000b
					PermuteFunc1			<ecx>,<edx>,10b
					PermuteFunc2			<ecx>,<eax>,100000b
					PermuteFunc1			<ebp>,<edx>,1000000000b
					PermuteFunc2			<ebp>,<eax>,10000000000000b
					PermuteFunc1			<ecx>,<edx>,1000000000b
					PermuteFunc2			<ecx>,<eax>,10000000000000b
					PermuteFunc1			<ebp>,<edx>,100000000000000000b
					PermuteFunc2			<ebp>,<eax>,1000000000000000000000b
					PermuteFunc1			<ecx>,<edx>,100000000000000000b
					PermuteFunc2			<ecx>,<eax>,1000000000000000000000b
					PermuteFunc1			<ebp>,<edx>,10000000000000000000000000b
					PermuteFunc2			<ebp>,<eax>,100000000000000000000000000000b
					PermuteFunc1			<ecx>,<edx>,10000000000000000000000000b
					PermuteFunc2			<ecx>,<eax>,100000000000000000000000000000b
					PermuteFunc1			<ebp>,<edx>,1b
					PermuteFunc2			<ebp>,<eax>,10000b
					PermuteFunc1			<ecx>,<edx>,1b
					PermuteFunc2			<ecx>,<eax>,10000b
					PermuteFunc1			<ebp>,<edx>,100000000b
					PermuteFunc2			<ebp>,<eax>,1000000000000b
					PermuteFunc1			<ecx>,<edx>,100000000b
					PermuteFunc2			<ecx>,<eax>,1000000000000b
					PermuteFunc1			<ebp>,<edx>,10000000000000000b
					PermuteFunc2			<ebp>,<eax>,100000000000000000000b
					PermuteFunc1			<ecx>,<edx>,10000000000000000b
					PermuteFunc2			<ecx>,<eax>,100000000000000000000b
					PermuteFunc1			<ebp>,<edx>,1000000000000000000000000b
					PermuteFunc2			<ebp>,<eax>,10000000000000000000000000000b
					PermuteFunc1			<ecx>,<edx>,1000000000000000000000000b
					PermuteFunc2			<ecx>,<eax>,10000000000000000000000000000b
ENDM

.CONST
align 4	
SLoc	equ	$
S	dd	0808200h,00h,08000h,0808202h,0808002h,08202h,02h,08000h
	dd	0200h,0808200h,0808202h,0200h,0800202h,0808002h,0800000h,02h
	dd	0202h,0800200h,0800200h,08200h,08200h,0808000h,0808000h,0800202h
	dd	08002h,0800002h,0800002h,08002h,00h,0202h,08202h,0800000h
	dd	08000h,0808202h,02h,0808000h,0808200h,0800000h,0800000h,0200h
	dd	0808002h,08000h,08200h,0800002h,0200h,02h,0800202h,08202h
	dd	0808202h,08002h,0808000h,0800202h,0800002h,0202h,08202h,0808200h
	dd	0202h,0800200h,0800200h,00h,08002h,08200h,00h,0808002h
	dd	040084010h,040004000h,04000h,084010h,080000h,010h,040080010h,040004010h
	dd	040000010h,040084010h,040084000h,040000000h,040004000h,080000h,010h,040080010h
	dd	084000h,080010h,040004010h,00h,040000000h,04000h,084010h,040080000h
	dd	080010h,040000010h,00h,084000h,04010h,040084000h,040080000h,04010h
	dd	00h,084010h,040080010h,080000h,040004010h,040080000h,040084000h,04000h
	dd	040080000h,040004000h,010h,040084010h,084010h,010h,04000h,040000000h
	dd	04010h,040084000h,080000h,040000010h,080010h,040004010h,040000010h,080010h
	dd	084000h,00h,040004000h,04010h,040000000h,040080010h,040084010h,084000h
	dd	0104h,04010100h,00h,04010004h,04000100h,00h,010104h,04000100h
	dd	010004h,04000004h,04000004h,010000h,04010104h,010004h,04010000h,0104h
	dd	04000000h,04h,04010100h,0100h,010100h,04010000h,04010004h,010104h
	dd	04000104h,010100h,010000h,04000104h,04h,04010104h,0100h,04000000h
	dd	04010100h,04000000h,010004h,0104h,010000h,04010100h,04000100h,00h
	dd	0100h,010004h,04010104h,04000100h,04000004h,0100h,00h,04010004h
	dd	04000104h,010000h,04000000h,04010104h,04h,010104h,010100h,04000004h
	dd	04010000h,04000104h,0104h,04010000h,010104h,04h,04010004h,010100h
	dd	080401000h,080001040h,080001040h,040h,0401040h,080400040h,080400000h,080001000h
	dd	00h,0401000h,0401000h,080401040h,080000040h,00h,0400040h,080400000h
	dd	080000000h,01000h,0400000h,080401000h,040h,0400000h,080001000h,01040h
	dd	080400040h,080000000h,01040h,0400040h,01000h,0401040h,080401040h,080000040h
	dd	0400040h,080400000h,0401000h,080401040h,080000040h,00h,00h,0401000h
	dd	01040h,0400040h,080400040h,080000000h,080401000h,080001040h,080001040h,040h
	dd	080401040h,080000040h,080000000h,01000h,080400000h,080001000h,0401040h,080400040h
	dd	080001000h,01040h,0400000h,080401000h,040h,0400000h,01000h,0401040h
	dd	080h,01040080h,01040000h,021000080h,040000h,080h,020000000h,01040000h
	dd	020040080h,040000h,01000080h,020040080h,021000080h,021040000h,040080h,020000000h
	dd	01000000h,020040000h,020040000h,00h,020000080h,021040080h,021040080h,01000080h
	dd	021040000h,020000080h,00h,021000000h,01040080h,01000000h,021000000h,040080h
	dd	040000h,021000080h,080h,01000000h,020000000h,01040000h,021000080h,020040080h
	dd	01000080h,020000000h,021040000h,01040080h,020040080h,080h,01000000h,021040000h
	dd	021040080h,040080h,021000000h,021040080h,01040000h,00h,020040000h,021000000h
	dd	040080h,01000080h,020000080h,040000h,00h,020040000h,01040080h,020000080h
	dd	010000008h,010200000h,02000h,010202008h,010200000h,08h,010202008h,0200000h
	dd	010002000h,0202008h,0200000h,010000008h,0200008h,010002000h,010000000h,02008h
	dd	00h,0200008h,010002008h,02000h,0202000h,010002008h,08h,010200008h
	dd	010200008h,00h,0202008h,010202000h,02008h,0202000h,010202000h,010000000h
	dd	010002000h,08h,010200008h,0202000h,010202008h,0200000h,02008h,010000008h
	dd	0200000h,010002000h,010000000h,02008h,010000008h,010202008h,0202000h,010200000h
	dd	0202008h,010202000h,00h,010200008h,08h,02000h,010200000h,0202008h
	dd	02000h,0200008h,010002008h,00h,010202000h,010000000h,0200008h,010002008h
	dd	0100000h,02100001h,02000401h,00h,0400h,02000401h,0100401h,02100400h
	dd	02100401h,0100000h,00h,02000001h,01h,02000000h,02100001h,0401h
	dd	02000400h,0100401h,0100001h,02000400h,02000001h,02100000h,02100400h,0100001h
	dd	02100000h,0400h,0401h,02100401h,0100400h,01h,02000000h,0100400h
	dd	02000000h,0100400h,0100000h,02000401h,02000401h,02100001h,02100001h,01h
	dd	0100001h,02000000h,02000400h,0100000h,02100400h,0401h,0100401h,02100400h
	dd	0401h,02000001h,02100401h,02100000h,0100400h,00h,01h,02100401h
	dd	00h,0100401h,02100000h,0400h,02000001h,02000400h,0400h,0100001h
	dd	08000820h,0800h,020000h,08020820h,08000000h,08000820h,020h,08000000h
	dd	020020h,08020000h,08020820h,020800h,08020800h,020820h,0800h,020h
	dd	08020000h,08000020h,08000800h,0820h,020800h,020020h,08020020h,08020800h
	dd	0820h,00h,00h,08020020h,08000020h,08000800h,020820h,020000h
	dd	020820h,020000h,08020800h,0800h,020h,08020020h,0800h,020820h
	dd	08000800h,020h,08000020h,08020000h,08020020h,08000000h,020000h,08000820h
	dd	00h,08020820h,020020h,08000020h,08020000h,08000800h,08000820h,00h
	dd	08020820h,020800h,020800h,0820h,0820h,020020h,08000000h,08020800h
.DATA
DataBit		dq	1
.CODE
;	Entry
;		edi=Subkey Table
;		edx:eaxPlain text Message
;	Returns
;		edx:eax Cyphertest  
;	Notes
;		All registers including ebp are used
ALIGN 4
					KeyOffset=0
DesCoreASM:			InitalPermute
REPEAT 16
					mov	ecx,ebp
					Expand
					xor	edx,[edi+4+KeyOffset]	
					xor	eax,[edi+KeyOffset]
					Subtitution
					xor ebp,esi
					mov esi,ecx
					KeyOffset=KeyOffset+8
ENDM
					FinalPermute
					ret





ALIGN 4
					KeyOffset=0
Des3CoreASM:		InitalPermute
REPEAT 2
REPEAT 16
					mov	ecx,ebp
					Expand
					xor	edx,[edi+4+KeyOffset]	
					xor	eax,[edi+KeyOffset]
					Subtitution
					xor ebp,esi
					mov esi,ecx
					KeyOffset=KeyOffset+8
ENDM
					xchg	esi,ebp
ENDM
REPEAT 16
					mov	ecx,ebp
					Expand
					xor	edx,[edi+4+KeyOffset]	
					xor	eax,[edi+KeyOffset]
					Subtitution
					xor ebp,esi
					mov esi,ecx
					KeyOffset=KeyOffset+8
ENDM
					FinalPermute
					ret
END					
