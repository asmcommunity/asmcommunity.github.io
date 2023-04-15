;######################################################################################################################
;#                                                                                                                    #
;#	Copyright � 2004 John Weeks johnweeks@prodigy.net                                                                 #
;#                                                                                                                    #
;######################################################################################################################
					TITLE			Des Encription
					SUBTITLE		Main Functions
					.NOLISTMACRO
					.386
					.MODEL		FLAT,STDCALL
					OPTION		CASEMAP:NONE
include ../common/des.inc
.CODE

DesKeyDecryptASM:
;	Entry
;		edx:eax=Key
;		edi=Subkey Table
;	Returns
;		Nothing  
;	Notes
;		edi is unchanged		
;		All other registers including ebp are used
	call	DesKeyEncryptASM	

StartCount=0
EndCount=120
REPEAT 8
	mov	eax,[edi+StartCount]
	mov	ebx,[edi+StartCount+4]
	mov	ecx,[edi+EndCount]
	mov	edx,[edi+EndCount+4]
	mov	[edi+EndCount],eax
	mov	[edi+EndCount+4],ebx
	mov	[edi+StartCount],ecx
	mov	[edi+StartCount+4],edx
StartCount=StartCount+8
EndCount=EndCount-8
ENDM
	ret


DesKeyEncryptASM:
;	Entry
;		edx:eax=Key
;		edi=Subkey Table
;	Returns
;		Nothing  
;	Notes
;		edi is unchanged		
;		All other registers including ebp are used
;	Key Table must be Zeroed out
	test	eax,	02h
	jz	@F
	mov	ebx,	0100h
	or	[edi+0],	ebx
	mov	ebx,	040000h
	or	[edi+8],	ebx
	mov	ebx,	040h
	or	[edi+16],	ebx
	mov	ebx,	08000h
	or	[edi+24],	ebx
	mov	ebx,	0400h
	or	[edi+32],	ebx
	mov	ebx,	080000h
	or	[edi+40],	ebx
	mov	ebx,	04000h
	or	[edi+48],	ebx
	mov	ebx,	010h
	or	[edi+64],	ebx
	mov	ebx,	010000h
	or	[edi+72],	ebx
	mov	ebx,	04h
	or	[edi+88],	ebx
	mov	ebx,	080h
	or	[edi+96],	ebx
	mov	ebx,	01h
	or	[edi+104],	ebx
	mov	ebx,	020000h
	or	[edi+112],	ebx
	mov	ebx,	02h
	or	[edi+120],	ebx
@@:	test	eax,	04h
	jz	@F
	mov	ebx,	04h
	or	[edi+0],	ebx
	mov	ebx,	02000h
	or	[edi+16],	ebx
	mov	ebx,	0200000h
	or	[edi+24],	ebx
	mov	ebx,	02h
	or	[edi+32],	ebx
	mov	ebx,	040000h
	or	[edi+40],	ebx
	mov	ebx,	040h
	or	[edi+48],	ebx
	mov	ebx,	08000h
	or	[edi+56],	ebx
	mov	ebx,	08h
	or	[edi+64],	ebx
	mov	ebx,	01000h
	or	[edi+72],	ebx
	mov	ebx,	020h
	or	[edi+80],	ebx
	mov	ebx,	0800h
	or	[edi+88],	ebx
	mov	ebx,	010h
	or	[edi+96],	ebx
	mov	ebx,	010000h
	or	[edi+104],	ebx
	mov	ebx,	0100000h
	or	[edi+120],	ebx
@@:	test	eax,	08h
	jz	@F
	mov	ebx,	0800h
	or	[edi+0],	ebx
	mov	ebx,	0800000h
	or	[edi+16],	ebx
	mov	ebx,	0200h
	or	[edi+24],	ebx
	mov	ebx,	0100000h
	or	[edi+32],	ebx
	mov	ebx,	02000h
	or	[edi+48],	ebx
	mov	ebx,	0200000h
	or	[edi+56],	ebx
	mov	ebx,	020000h
	or	[edi+64],	ebx
	mov	ebx,	0100h
	or	[edi+72],	ebx
	mov	ebx,	0400000h
	or	[edi+88],	ebx
	mov	ebx,	08h
	or	[edi+96],	ebx
	mov	ebx,	01000h
	or	[edi+104],	ebx
	mov	ebx,	020h
	or	[edi+112],	ebx
	mov	ebx,	04000h
	or	[edi+120],	ebx
@@:	test	eax,	010h
	jz	@F
	mov	ebx,	0400000h
	or	[edi+0],	ebx
	mov	ebx,	0100000h
	or	[edi+4],	ebx
	mov	ebx,	08000h
	or	[edi+8],	ebx
	mov	ebx,	0800h
	or	[edi+12],	ebx
	mov	ebx,	0400h
	or	[edi+16],	ebx
	mov	ebx,	02000h
	or	[edi+20],	ebx
	mov	ebx,	080000h
	or	[edi+24],	ebx
	mov	ebx,	0400h
	or	[edi+28],	ebx
	mov	ebx,	04000h
	or	[edi+32],	ebx
	mov	ebx,	0400000h
	or	[edi+36],	ebx
	mov	ebx,	08000h
	or	[edi+44],	ebx
	mov	ebx,	0800000h
	or	[edi+48],	ebx
	mov	ebx,	02h
	or	[edi+52],	ebx
	mov	ebx,	0200h
	or	[edi+56],	ebx
	mov	ebx,	0200000h
	or	[edi+60],	ebx
	mov	ebx,	01000h
	or	[edi+68],	ebx
	mov	ebx,	04h
	or	[edi+72],	ebx
	mov	ebx,	040h
	or	[edi+76],	ebx
	mov	ebx,	080h
	or	[edi+80],	ebx
	mov	ebx,	04000h
	or	[edi+84],	ebx
	mov	ebx,	01h
	or	[edi+88],	ebx
	mov	ebx,	0100h
	or	[edi+92],	ebx
	mov	ebx,	020000h
	or	[edi+96],	ebx
	mov	ebx,	01h
	or	[edi+100],	ebx
	mov	ebx,	0100h
	or	[edi+104],	ebx
	mov	ebx,	010000h
	or	[edi+108],	ebx
	mov	ebx,	080h
	or	[edi+116],	ebx
	mov	ebx,	040h
	or	[edi+120],	ebx
@@:	test	eax,	020h
	jz	@F
	mov	ebx,	020h
	or	[edi+4],	ebx
	mov	ebx,	08000h
	or	[edi+12],	ebx
	mov	ebx,	02h
	or	[edi+20],	ebx
	mov	ebx,	0200000h
	or	[edi+28],	ebx
	mov	ebx,	010h
	or	[edi+44],	ebx
	mov	ebx,	040000h
	or	[edi+52],	ebx
	mov	ebx,	020000h
	or	[edi+60],	ebx
	mov	ebx,	01h
	or	[edi+68],	ebx
	mov	ebx,	010000h
	or	[edi+76],	ebx
	mov	ebx,	080h
	or	[edi+84],	ebx
	mov	ebx,	0100000h
	or	[edi+92],	ebx
	mov	ebx,	04h
	or	[edi+108],	ebx
	mov	ebx,	0400000h
	or	[edi+124],	ebx
@@:	test	eax,	040h
	jz	@F
	mov	ebx,	040h
	or	[edi+4],	ebx
	mov	ebx,	010h
	or	[edi+12],	ebx
	mov	ebx,	040000h
	or	[edi+20],	ebx
	mov	ebx,	020000h
	or	[edi+28],	ebx
	mov	ebx,	080000h
	or	[edi+36],	ebx
	mov	ebx,	08h
	or	[edi+44],	ebx
	mov	ebx,	0800h
	or	[edi+60],	ebx
	mov	ebx,	04h
	or	[edi+76],	ebx
	mov	ebx,	020h
	or	[edi+92],	ebx
	mov	ebx,	0800000h
	or	[edi+100],	ebx
	mov	ebx,	0200h
	or	[edi+108],	ebx
	mov	ebx,	01000h
	or	[edi+116],	ebx
@@:	test	eax,	080h
	jz	@F
	mov	ebx,	010000h
	or	[edi+4],	ebx
	mov	ebx,	08h
	or	[edi+12],	ebx
	mov	ebx,	0800h
	or	[edi+28],	ebx
	mov	ebx,	02000h
	or	[edi+36],	ebx
	mov	ebx,	0400h
	or	[edi+44],	ebx
	mov	ebx,	0400000h
	or	[edi+52],	ebx
	mov	ebx,	08000h
	or	[edi+60],	ebx
	mov	ebx,	0800000h
	or	[edi+68],	ebx
	mov	ebx,	0200h
	or	[edi+76],	ebx
	mov	ebx,	01000h
	or	[edi+84],	ebx
	mov	ebx,	040h
	or	[edi+92],	ebx
	mov	ebx,	04000h
	or	[edi+100],	ebx
	mov	ebx,	0100h
	or	[edi+108],	ebx
	mov	ebx,	01h
	or	[edi+116],	ebx
	mov	ebx,	080000h
	or	[edi+124],	ebx
@@:	test	eax,	0200h
	jz	@F
	mov	ebx,	02h
	or	[edi+0],	ebx
	mov	ebx,	0100h
	or	[edi+8],	ebx
	mov	ebx,	0400000h
	or	[edi+24],	ebx
	mov	ebx,	08h
	or	[edi+32],	ebx
	mov	ebx,	01000h
	or	[edi+40],	ebx
	mov	ebx,	020h
	or	[edi+48],	ebx
	mov	ebx,	0800h
	or	[edi+56],	ebx
	mov	ebx,	0800000h
	or	[edi+72],	ebx
	mov	ebx,	0200h
	or	[edi+80],	ebx
	mov	ebx,	0100000h
	or	[edi+88],	ebx
	mov	ebx,	02000h
	or	[edi+104],	ebx
	mov	ebx,	0200000h
	or	[edi+112],	ebx
	mov	ebx,	020000h
	or	[edi+120],	ebx
@@:	test	eax,	0400h
	jz	@F
	mov	ebx,	0100000h
	or	[edi+0],	ebx
	mov	ebx,	04h
	or	[edi+8],	ebx
	mov	ebx,	080h
	or	[edi+16],	ebx
	mov	ebx,	01h
	or	[edi+24],	ebx
	mov	ebx,	020000h
	or	[edi+32],	ebx
	mov	ebx,	0100h
	or	[edi+40],	ebx
	mov	ebx,	0400000h
	or	[edi+56],	ebx
	mov	ebx,	08000h
	or	[edi+64],	ebx
	mov	ebx,	0400h
	or	[edi+72],	ebx
	mov	ebx,	080000h
	or	[edi+80],	ebx
	mov	ebx,	04000h
	or	[edi+88],	ebx
	mov	ebx,	0800000h
	or	[edi+104],	ebx
	mov	ebx,	0200h
	or	[edi+112],	ebx
@@:	test	eax,	0800h
	jz	@F
	mov	ebx,	04000h
	or	[edi+0],	ebx
	mov	ebx,	0800h
	or	[edi+8],	ebx
	mov	ebx,	010h
	or	[edi+16],	ebx
	mov	ebx,	010000h
	or	[edi+24],	ebx
	mov	ebx,	04h
	or	[edi+40],	ebx
	mov	ebx,	080h
	or	[edi+48],	ebx
	mov	ebx,	01h
	or	[edi+56],	ebx
	mov	ebx,	0200000h
	or	[edi+64],	ebx
	mov	ebx,	02h
	or	[edi+72],	ebx
	mov	ebx,	040000h
	or	[edi+80],	ebx
	mov	ebx,	040h
	or	[edi+88],	ebx
	mov	ebx,	08000h
	or	[edi+96],	ebx
	mov	ebx,	0400h
	or	[edi+104],	ebx
	mov	ebx,	080000h
	or	[edi+112],	ebx
	mov	ebx,	020h
	or	[edi+120],	ebx
@@:	test	eax,	01000h
	jz	@F
	mov	ebx,	040h
	or	[edi+0],	ebx
	mov	ebx,	0400000h
	or	[edi+8],	ebx
	mov	ebx,	0100000h
	or	[edi+12],	ebx
	mov	ebx,	08h
	or	[edi+16],	ebx
	mov	ebx,	01000h
	or	[edi+24],	ebx
	mov	ebx,	04h
	or	[edi+28],	ebx
	mov	ebx,	020h
	or	[edi+32],	ebx
	mov	ebx,	0800h
	or	[edi+40],	ebx
	mov	ebx,	020h
	or	[edi+44],	ebx
	mov	ebx,	010h
	or	[edi+48],	ebx
	mov	ebx,	0800000h
	or	[edi+52],	ebx
	mov	ebx,	010000h
	or	[edi+56],	ebx
	mov	ebx,	0200h
	or	[edi+60],	ebx
	mov	ebx,	0200h
	or	[edi+64],	ebx
	mov	ebx,	0200000h
	or	[edi+68],	ebx
	mov	ebx,	0100000h
	or	[edi+72],	ebx
	mov	ebx,	010h
	or	[edi+84],	ebx
	mov	ebx,	02000h
	or	[edi+88],	ebx
	mov	ebx,	040000h
	or	[edi+92],	ebx
	mov	ebx,	0200000h
	or	[edi+96],	ebx
	mov	ebx,	020000h
	or	[edi+100],	ebx
	mov	ebx,	02h
	or	[edi+104],	ebx
	mov	ebx,	080000h
	or	[edi+108],	ebx
	mov	ebx,	040000h
	or	[edi+112],	ebx
	mov	ebx,	08h
	or	[edi+116],	ebx
	mov	ebx,	080h
	or	[edi+124],	ebx
@@:	test	eax,	02000h
	jz	@F
	mov	ebx,	0400000h
	or	[edi+4],	ebx
	mov	ebx,	020h
	or	[edi+12],	ebx
	mov	ebx,	0800000h
	or	[edi+20],	ebx
	mov	ebx,	0200h
	or	[edi+28],	ebx
	mov	ebx,	01000h
	or	[edi+36],	ebx
	mov	ebx,	040h
	or	[edi+44],	ebx
	mov	ebx,	04000h
	or	[edi+52],	ebx
	mov	ebx,	0100h
	or	[edi+60],	ebx
	mov	ebx,	020000h
	or	[edi+68],	ebx
	mov	ebx,	080000h
	or	[edi+76],	ebx
	mov	ebx,	08h
	or	[edi+84],	ebx
	mov	ebx,	0800h
	or	[edi+100],	ebx
	mov	ebx,	02000h
	or	[edi+108],	ebx
	mov	ebx,	0400h
	or	[edi+116],	ebx
@@:	test	eax,	04000h
	jz	@F
	mov	ebx,	040h
	or	[edi+12],	ebx
	mov	ebx,	04000h
	or	[edi+20],	ebx
	mov	ebx,	0100h
	or	[edi+28],	ebx
	mov	ebx,	01h
	or	[edi+36],	ebx
	mov	ebx,	010000h
	or	[edi+44],	ebx
	mov	ebx,	080h
	or	[edi+52],	ebx
	mov	ebx,	0100000h
	or	[edi+60],	ebx
	mov	ebx,	0800h
	or	[edi+68],	ebx
	mov	ebx,	02000h
	or	[edi+76],	ebx
	mov	ebx,	0400h
	or	[edi+84],	ebx
	mov	ebx,	0400000h
	or	[edi+92],	ebx
	mov	ebx,	08000h
	or	[edi+100],	ebx
	mov	ebx,	02h
	or	[edi+108],	ebx
	mov	ebx,	0200000h
	or	[edi+116],	ebx
	mov	ebx,	01000h
	or	[edi+124],	ebx
@@:	test	eax,	08000h
	jz	@F
	mov	ebx,	080000h
	or	[edi+4],	ebx
	mov	ebx,	010000h
	or	[edi+12],	ebx
	mov	ebx,	080h
	or	[edi+20],	ebx
	mov	ebx,	0100000h
	or	[edi+28],	ebx
	mov	ebx,	04h
	or	[edi+44],	ebx
	mov	ebx,	020h
	or	[edi+60],	ebx
	mov	ebx,	08000h
	or	[edi+68],	ebx
	mov	ebx,	02h
	or	[edi+76],	ebx
	mov	ebx,	0200000h
	or	[edi+84],	ebx
	mov	ebx,	010h
	or	[edi+100],	ebx
	mov	ebx,	040000h
	or	[edi+108],	ebx
	mov	ebx,	020000h
	or	[edi+116],	ebx
	mov	ebx,	01h
	or	[edi+124],	ebx
@@:	test	eax,	020000h
	jz	@F
	mov	ebx,	020000h
	or	[edi+0],	ebx
	mov	ebx,	02h
	or	[edi+8],	ebx
	mov	ebx,	040000h
	or	[edi+16],	ebx
	mov	ebx,	040h
	or	[edi+24],	ebx
	mov	ebx,	08000h
	or	[edi+32],	ebx
	mov	ebx,	0400h
	or	[edi+40],	ebx
	mov	ebx,	080000h
	or	[edi+48],	ebx
	mov	ebx,	04000h
	or	[edi+56],	ebx
	mov	ebx,	0800h
	or	[edi+64],	ebx
	mov	ebx,	010h
	or	[edi+72],	ebx
	mov	ebx,	010000h
	or	[edi+80],	ebx
	mov	ebx,	04h
	or	[edi+96],	ebx
	mov	ebx,	080h
	or	[edi+104],	ebx
	mov	ebx,	01h
	or	[edi+112],	ebx
	mov	ebx,	0200000h
	or	[edi+120],	ebx
@@:	test	eax,	040000h
	jz	@F
	mov	ebx,	0100000h
	or	[edi+8],	ebx
	mov	ebx,	02000h
	or	[edi+24],	ebx
	mov	ebx,	0200000h
	or	[edi+32],	ebx
	mov	ebx,	02h
	or	[edi+40],	ebx
	mov	ebx,	040000h
	or	[edi+48],	ebx
	mov	ebx,	040h
	or	[edi+56],	ebx
	mov	ebx,	0400000h
	or	[edi+64],	ebx
	mov	ebx,	08h
	or	[edi+72],	ebx
	mov	ebx,	01000h
	or	[edi+80],	ebx
	mov	ebx,	020h
	or	[edi+88],	ebx
	mov	ebx,	0800h
	or	[edi+96],	ebx
	mov	ebx,	010h
	or	[edi+104],	ebx
	mov	ebx,	010000h
	or	[edi+112],	ebx
	mov	ebx,	0200h
	or	[edi+120],	ebx
@@:	test	eax,	080000h
	jz	@F
	mov	ebx,	020h
	or	[edi+0],	ebx
	mov	ebx,	04000h
	or	[edi+8],	ebx
	mov	ebx,	0800000h
	or	[edi+24],	ebx
	mov	ebx,	0200h
	or	[edi+32],	ebx
	mov	ebx,	0100000h
	or	[edi+40],	ebx
	mov	ebx,	02000h
	or	[edi+56],	ebx
	mov	ebx,	01h
	or	[edi+64],	ebx
	mov	ebx,	020000h
	or	[edi+72],	ebx
	mov	ebx,	0100h
	or	[edi+80],	ebx
	mov	ebx,	0400000h
	or	[edi+96],	ebx
	mov	ebx,	08h
	or	[edi+104],	ebx
	mov	ebx,	01000h
	or	[edi+112],	ebx
	mov	ebx,	080000h
	or	[edi+120],	ebx
@@:	test	eax,	0100000h
	jz	@F
	mov	ebx,	080h
	or	[edi+4],	ebx
	mov	ebx,	040h
	or	[edi+8],	ebx
	mov	ebx,	08000h
	or	[edi+16],	ebx
	mov	ebx,	0800h
	or	[edi+20],	ebx
	mov	ebx,	0400h
	or	[edi+24],	ebx
	mov	ebx,	02000h
	or	[edi+28],	ebx
	mov	ebx,	080000h
	or	[edi+32],	ebx
	mov	ebx,	0400h
	or	[edi+36],	ebx
	mov	ebx,	04000h
	or	[edi+40],	ebx
	mov	ebx,	0400000h
	or	[edi+44],	ebx
	mov	ebx,	08000h
	or	[edi+52],	ebx
	mov	ebx,	0800000h
	or	[edi+56],	ebx
	mov	ebx,	02h
	or	[edi+60],	ebx
	mov	ebx,	010000h
	or	[edi+64],	ebx
	mov	ebx,	0200h
	or	[edi+68],	ebx
	mov	ebx,	01000h
	or	[edi+76],	ebx
	mov	ebx,	04h
	or	[edi+80],	ebx
	mov	ebx,	040h
	or	[edi+84],	ebx
	mov	ebx,	080h
	or	[edi+88],	ebx
	mov	ebx,	04000h
	or	[edi+92],	ebx
	mov	ebx,	01h
	or	[edi+96],	ebx
	mov	ebx,	0100h
	or	[edi+100],	ebx
	mov	ebx,	020000h
	or	[edi+104],	ebx
	mov	ebx,	01h
	or	[edi+108],	ebx
	mov	ebx,	0100h
	or	[edi+112],	ebx
	mov	ebx,	010000h
	or	[edi+116],	ebx
	mov	ebx,	040000h
	or	[edi+120],	ebx
	mov	ebx,	08h
	or	[edi+124],	ebx
@@:	test	eax,	0200000h
	jz	@F
	mov	ebx,	0400000h
	or	[edi+12],	ebx
	mov	ebx,	08000h
	or	[edi+20],	ebx
	mov	ebx,	02h
	or	[edi+28],	ebx
	mov	ebx,	0200000h
	or	[edi+36],	ebx
	mov	ebx,	010h
	or	[edi+52],	ebx
	mov	ebx,	040000h
	or	[edi+60],	ebx
	mov	ebx,	0100h
	or	[edi+68],	ebx
	mov	ebx,	01h
	or	[edi+76],	ebx
	mov	ebx,	010000h
	or	[edi+84],	ebx
	mov	ebx,	080h
	or	[edi+92],	ebx
	mov	ebx,	0100000h
	or	[edi+100],	ebx
	mov	ebx,	04h
	or	[edi+116],	ebx
	mov	ebx,	0400h
	or	[edi+124],	ebx
@@:	test	eax,	0400000h
	jz	@F
	mov	ebx,	01000h
	or	[edi+4],	ebx
	mov	ebx,	010h
	or	[edi+20],	ebx
	mov	ebx,	040000h
	or	[edi+28],	ebx
	mov	ebx,	020000h
	or	[edi+36],	ebx
	mov	ebx,	080000h
	or	[edi+44],	ebx
	mov	ebx,	08h
	or	[edi+52],	ebx
	mov	ebx,	0100000h
	or	[edi+68],	ebx
	mov	ebx,	04h
	or	[edi+84],	ebx
	mov	ebx,	020h
	or	[edi+100],	ebx
	mov	ebx,	0800000h
	or	[edi+108],	ebx
	mov	ebx,	0200h
	or	[edi+116],	ebx
	mov	ebx,	0200000h
	or	[edi+124],	ebx
@@:	test	eax,	0800000h
	jz	@F
	mov	ebx,	01h
	or	[edi+4],	ebx
	mov	ebx,	080000h
	or	[edi+12],	ebx
	mov	ebx,	08h
	or	[edi+20],	ebx
	mov	ebx,	0800h
	or	[edi+36],	ebx
	mov	ebx,	02000h
	or	[edi+44],	ebx
	mov	ebx,	0400h
	or	[edi+52],	ebx
	mov	ebx,	0400000h
	or	[edi+60],	ebx
	mov	ebx,	020h
	or	[edi+68],	ebx
	mov	ebx,	0800000h
	or	[edi+76],	ebx
	mov	ebx,	0200h
	or	[edi+84],	ebx
	mov	ebx,	01000h
	or	[edi+92],	ebx
	mov	ebx,	040h
	or	[edi+100],	ebx
	mov	ebx,	04000h
	or	[edi+108],	ebx
	mov	ebx,	0100h
	or	[edi+116],	ebx
	mov	ebx,	020000h
	or	[edi+124],	ebx
@@:	test	eax,	02000000h
	jz	@F
	mov	ebx,	0200000h
	or	[edi+0],	ebx
	mov	ebx,	020000h
	or	[edi+8],	ebx
	mov	ebx,	0100h
	or	[edi+16],	ebx
	mov	ebx,	0400000h
	or	[edi+32],	ebx
	mov	ebx,	08h
	or	[edi+40],	ebx
	mov	ebx,	01000h
	or	[edi+48],	ebx
	mov	ebx,	020h
	or	[edi+56],	ebx
	mov	ebx,	04000h
	or	[edi+64],	ebx
	mov	ebx,	0800000h
	or	[edi+80],	ebx
	mov	ebx,	0200h
	or	[edi+88],	ebx
	mov	ebx,	0100000h
	or	[edi+96],	ebx
	mov	ebx,	02000h
	or	[edi+112],	ebx
	mov	ebx,	01h
	or	[edi+120],	ebx
@@:	test	eax,	04000000h
	jz	@F
	mov	ebx,	0200h
	or	[edi+0],	ebx
	mov	ebx,	04h
	or	[edi+16],	ebx
	mov	ebx,	080h
	or	[edi+24],	ebx
	mov	ebx,	01h
	or	[edi+32],	ebx
	mov	ebx,	020000h
	or	[edi+40],	ebx
	mov	ebx,	0100h
	or	[edi+48],	ebx
	mov	ebx,	040h
	or	[edi+64],	ebx
	mov	ebx,	08000h
	or	[edi+72],	ebx
	mov	ebx,	0400h
	or	[edi+80],	ebx
	mov	ebx,	080000h
	or	[edi+88],	ebx
	mov	ebx,	04000h
	or	[edi+96],	ebx
	mov	ebx,	0800000h
	or	[edi+112],	ebx
	mov	ebx,	010000h
	or	[edi+120],	ebx
@@:	test	eax,	08000000h
	jz	@F
	mov	ebx,	080000h
	or	[edi+0],	ebx
	mov	ebx,	020h
	or	[edi+8],	ebx
	mov	ebx,	0800h
	or	[edi+16],	ebx
	mov	ebx,	010h
	or	[edi+24],	ebx
	mov	ebx,	010000h
	or	[edi+32],	ebx
	mov	ebx,	04h
	or	[edi+48],	ebx
	mov	ebx,	080h
	or	[edi+56],	ebx
	mov	ebx,	02000h
	or	[edi+64],	ebx
	mov	ebx,	0200000h
	or	[edi+72],	ebx
	mov	ebx,	02h
	or	[edi+80],	ebx
	mov	ebx,	040000h
	or	[edi+88],	ebx
	mov	ebx,	040h
	or	[edi+96],	ebx
	mov	ebx,	08000h
	or	[edi+104],	ebx
	mov	ebx,	0400h
	or	[edi+112],	ebx
	mov	ebx,	01000h
	or	[edi+120],	ebx
@@:	test	eax,	010000000h
	jz	@F
	mov	ebx,	040000h
	or	[edi+0],	ebx
	mov	ebx,	08h
	or	[edi+4],	ebx
	mov	ebx,	080h
	or	[edi+12],	ebx
	mov	ebx,	0400000h
	or	[edi+16],	ebx
	mov	ebx,	0100000h
	or	[edi+20],	ebx
	mov	ebx,	08h
	or	[edi+24],	ebx
	mov	ebx,	01000h
	or	[edi+32],	ebx
	mov	ebx,	04h
	or	[edi+36],	ebx
	mov	ebx,	020h
	or	[edi+40],	ebx
	mov	ebx,	0800h
	or	[edi+48],	ebx
	mov	ebx,	020h
	or	[edi+52],	ebx
	mov	ebx,	010h
	or	[edi+56],	ebx
	mov	ebx,	0800000h
	or	[edi+60],	ebx
	mov	ebx,	0800000h
	or	[edi+64],	ebx
	mov	ebx,	02h
	or	[edi+68],	ebx
	mov	ebx,	0200h
	or	[edi+72],	ebx
	mov	ebx,	0200000h
	or	[edi+76],	ebx
	mov	ebx,	0100000h
	or	[edi+80],	ebx
	mov	ebx,	010h
	or	[edi+92],	ebx
	mov	ebx,	02000h
	or	[edi+96],	ebx
	mov	ebx,	040000h
	or	[edi+100],	ebx
	mov	ebx,	0200000h
	or	[edi+104],	ebx
	mov	ebx,	020000h
	or	[edi+108],	ebx
	mov	ebx,	02h
	or	[edi+112],	ebx
	mov	ebx,	080000h
	or	[edi+116],	ebx
	mov	ebx,	0100h
	or	[edi+120],	ebx
	mov	ebx,	010000h
	or	[edi+124],	ebx
@@:	test	eax,	020000000h
	jz	@F
	mov	ebx,	0400h
	or	[edi+4],	ebx
	mov	ebx,	020h
	or	[edi+20],	ebx
	mov	ebx,	0800000h
	or	[edi+28],	ebx
	mov	ebx,	0200h
	or	[edi+36],	ebx
	mov	ebx,	01000h
	or	[edi+44],	ebx
	mov	ebx,	040h
	or	[edi+52],	ebx
	mov	ebx,	04000h
	or	[edi+60],	ebx
	mov	ebx,	040000h
	or	[edi+68],	ebx
	mov	ebx,	020000h
	or	[edi+76],	ebx
	mov	ebx,	080000h
	or	[edi+84],	ebx
	mov	ebx,	08h
	or	[edi+92],	ebx
	mov	ebx,	0800h
	or	[edi+108],	ebx
	mov	ebx,	02000h
	or	[edi+116],	ebx
	mov	ebx,	04h
	or	[edi+124],	ebx
@@:	test	eax,	040000000h
	jz	@F
	mov	ebx,	0200000h
	or	[edi+4],	ebx
	mov	ebx,	01000h
	or	[edi+12],	ebx
	mov	ebx,	040h
	or	[edi+20],	ebx
	mov	ebx,	04000h
	or	[edi+28],	ebx
	mov	ebx,	0100h
	or	[edi+36],	ebx
	mov	ebx,	01h
	or	[edi+44],	ebx
	mov	ebx,	010000h
	or	[edi+52],	ebx
	mov	ebx,	080h
	or	[edi+60],	ebx
	mov	ebx,	0800h
	or	[edi+76],	ebx
	mov	ebx,	02000h
	or	[edi+84],	ebx
	mov	ebx,	0400h
	or	[edi+92],	ebx
	mov	ebx,	0400000h
	or	[edi+100],	ebx
	mov	ebx,	08000h
	or	[edi+108],	ebx
	mov	ebx,	02h
	or	[edi+116],	ebx
	mov	ebx,	0200h
	or	[edi+124],	ebx
@@:	test	eax,	080000000h
	jz	@F
	mov	ebx,	020000h
	or	[edi+4],	ebx
	mov	ebx,	01h
	or	[edi+12],	ebx
	mov	ebx,	010000h
	or	[edi+20],	ebx
	mov	ebx,	080h
	or	[edi+28],	ebx
	mov	ebx,	0100000h
	or	[edi+36],	ebx
	mov	ebx,	04h
	or	[edi+52],	ebx
	mov	ebx,	0400000h
	or	[edi+68],	ebx
	mov	ebx,	08000h
	or	[edi+76],	ebx
	mov	ebx,	02h
	or	[edi+84],	ebx
	mov	ebx,	0200000h
	or	[edi+92],	ebx
	mov	ebx,	010h
	or	[edi+108],	ebx
	mov	ebx,	040000h
	or	[edi+116],	ebx
	mov	ebx,	0100h
	or	[edi+124],	ebx


@@:	test	edx,	02h
	jz	@F
	mov	ebx,	01h
	or	[edi+0],	ebx
	mov	ebx,	0200000h
	or	[edi+8],	ebx
	mov	ebx,	02h
	or	[edi+16],	ebx
	mov	ebx,	040000h
	or	[edi+24],	ebx
	mov	ebx,	040h
	or	[edi+32],	ebx
	mov	ebx,	08000h
	or	[edi+40],	ebx
	mov	ebx,	0400h
	or	[edi+48],	ebx
	mov	ebx,	080000h
	or	[edi+56],	ebx
	mov	ebx,	020h
	or	[edi+64],	ebx
	mov	ebx,	0800h
	or	[edi+72],	ebx
	mov	ebx,	010h
	or	[edi+80],	ebx
	mov	ebx,	010000h
	or	[edi+88],	ebx
	mov	ebx,	04h
	or	[edi+104],	ebx
	mov	ebx,	080h
	or	[edi+112],	ebx
	mov	ebx,	02000h
	or	[edi+120],	ebx
@@:	test	edx,	04h
	jz	@F
	mov	ebx,	010000h
	or	[edi+0],	ebx
	mov	ebx,	0200h
	or	[edi+8],	ebx
	mov	ebx,	0100000h
	or	[edi+16],	ebx
	mov	ebx,	02000h
	or	[edi+32],	ebx
	mov	ebx,	0200000h
	or	[edi+40],	ebx
	mov	ebx,	02h
	or	[edi+48],	ebx
	mov	ebx,	040000h
	or	[edi+56],	ebx
	mov	ebx,	0400000h
	or	[edi+72],	ebx
	mov	ebx,	08h
	or	[edi+80],	ebx
	mov	ebx,	01000h
	or	[edi+88],	ebx
	mov	ebx,	020h
	or	[edi+96],	ebx
	mov	ebx,	0800h
	or	[edi+104],	ebx
	mov	ebx,	010h
	or	[edi+112],	ebx
	mov	ebx,	0800000h
	or	[edi+120],	ebx
@@:	test	edx,	08h
	jz	@F
	mov	ebx,	01000h
	or	[edi+0],	ebx
	mov	ebx,	080000h
	or	[edi+8],	ebx
	mov	ebx,	04000h
	or	[edi+16],	ebx
	mov	ebx,	0800000h
	or	[edi+32],	ebx
	mov	ebx,	0200h
	or	[edi+40],	ebx
	mov	ebx,	0100000h
	or	[edi+48],	ebx
	mov	ebx,	080h
	or	[edi+64],	ebx
	mov	ebx,	01h
	or	[edi+72],	ebx
	mov	ebx,	020000h
	or	[edi+80],	ebx
	mov	ebx,	0100h
	or	[edi+88],	ebx
	mov	ebx,	0400000h
	or	[edi+104],	ebx
	mov	ebx,	08h
	or	[edi+112],	ebx
	mov	ebx,	0400h
	or	[edi+120],	ebx
@@:	test	edx,	010h
	jz	@F
	mov	ebx,	010000h
	or	[edi+4],	ebx
	mov	ebx,	08h
	or	[edi+12],	ebx
	mov	ebx,	0800h
	or	[edi+28],	ebx
	mov	ebx,	02000h
	or	[edi+36],	ebx
	mov	ebx,	0400h
	or	[edi+44],	ebx
	mov	ebx,	0400000h
	or	[edi+52],	ebx
	mov	ebx,	08000h
	or	[edi+60],	ebx
	mov	ebx,	0800000h
	or	[edi+68],	ebx
	mov	ebx,	0200h
	or	[edi+76],	ebx
	mov	ebx,	01000h
	or	[edi+84],	ebx
	mov	ebx,	040h
	or	[edi+92],	ebx
	mov	ebx,	04000h
	or	[edi+100],	ebx
	mov	ebx,	0100h
	or	[edi+108],	ebx
	mov	ebx,	01h
	or	[edi+116],	ebx
	mov	ebx,	080000h
	or	[edi+124],	ebx
@@:	test	edx,	020h
	jz	@F
	mov	ebx,	04h
	or	[edi+4],	ebx
	mov	ebx,	0400h
	or	[edi+12],	ebx
	mov	ebx,	0400000h
	or	[edi+20],	ebx
	mov	ebx,	08000h
	or	[edi+28],	ebx
	mov	ebx,	02h
	or	[edi+36],	ebx
	mov	ebx,	0200000h
	or	[edi+44],	ebx
	mov	ebx,	010h
	or	[edi+60],	ebx
	mov	ebx,	04000h
	or	[edi+68],	ebx
	mov	ebx,	0100h
	or	[edi+76],	ebx
	mov	ebx,	01h
	or	[edi+84],	ebx
	mov	ebx,	010000h
	or	[edi+92],	ebx
	mov	ebx,	080h
	or	[edi+100],	ebx
	mov	ebx,	0100000h
	or	[edi+108],	ebx
	mov	ebx,	02000h
	or	[edi+124],	ebx
@@:	test	edx,	040h
	jz	@F
	mov	ebx,	0200h
	or	[edi+4],	ebx
	mov	ebx,	0200000h
	or	[edi+12],	ebx
	mov	ebx,	010h
	or	[edi+28],	ebx
	mov	ebx,	040000h
	or	[edi+36],	ebx
	mov	ebx,	020000h
	or	[edi+44],	ebx
	mov	ebx,	080000h
	or	[edi+52],	ebx
	mov	ebx,	08h
	or	[edi+60],	ebx
	mov	ebx,	080h
	or	[edi+68],	ebx
	mov	ebx,	0100000h
	or	[edi+76],	ebx
	mov	ebx,	04h
	or	[edi+92],	ebx
	mov	ebx,	020h
	or	[edi+108],	ebx
	mov	ebx,	0800000h
	or	[edi+116],	ebx
	mov	ebx,	02h
	or	[edi+124],	ebx
@@:	test	edx,	080h
	jz	@F
	mov	ebx,	0100h
	or	[edi+4],	ebx
	mov	ebx,	020000h
	or	[edi+12],	ebx
	mov	ebx,	080000h
	or	[edi+20],	ebx
	mov	ebx,	08h
	or	[edi+28],	ebx
	mov	ebx,	0800h
	or	[edi+44],	ebx
	mov	ebx,	02000h
	or	[edi+52],	ebx
	mov	ebx,	0400h
	or	[edi+60],	ebx
	mov	ebx,	020h
	or	[edi+76],	ebx
	mov	ebx,	0800000h
	or	[edi+84],	ebx
	mov	ebx,	0200h
	or	[edi+92],	ebx
	mov	ebx,	01000h
	or	[edi+100],	ebx
	mov	ebx,	040h
	or	[edi+108],	ebx
	mov	ebx,	04000h
	or	[edi+116],	ebx
	mov	ebx,	040000h
	or	[edi+124],	ebx
@@:	test	edx,	0200h
	jz	@F
	mov	ebx,	02000h
	or	[edi+0],	ebx
	mov	ebx,	01h
	or	[edi+8],	ebx
	mov	ebx,	020000h
	or	[edi+16],	ebx
	mov	ebx,	0100h
	or	[edi+24],	ebx
	mov	ebx,	0400000h
	or	[edi+40],	ebx
	mov	ebx,	08h
	or	[edi+48],	ebx
	mov	ebx,	01000h
	or	[edi+56],	ebx
	mov	ebx,	080000h
	or	[edi+64],	ebx
	mov	ebx,	04000h
	or	[edi+72],	ebx
	mov	ebx,	0800000h
	or	[edi+88],	ebx
	mov	ebx,	0200h
	or	[edi+96],	ebx
	mov	ebx,	0100000h
	or	[edi+104],	ebx
	mov	ebx,	080h
	or	[edi+120],	ebx
@@:	test	edx,	0400h
	jz	@F
	mov	ebx,	0800000h
	or	[edi+0],	ebx
	mov	ebx,	010000h
	or	[edi+8],	ebx
	mov	ebx,	04h
	or	[edi+24],	ebx
	mov	ebx,	080h
	or	[edi+32],	ebx
	mov	ebx,	01h
	or	[edi+40],	ebx
	mov	ebx,	020000h
	or	[edi+48],	ebx
	mov	ebx,	0100h
	or	[edi+56],	ebx
	mov	ebx,	040000h
	or	[edi+64],	ebx
	mov	ebx,	040h
	or	[edi+72],	ebx
	mov	ebx,	08000h
	or	[edi+80],	ebx
	mov	ebx,	0400h
	or	[edi+88],	ebx
	mov	ebx,	080000h
	or	[edi+96],	ebx
	mov	ebx,	04000h
	or	[edi+104],	ebx
	mov	ebx,	010h
	or	[edi+120],	ebx
@@:	test	edx,	0800h
	jz	@F
	mov	ebx,	0400h
	or	[edi+0],	ebx
	mov	ebx,	01000h
	or	[edi+8],	ebx
	mov	ebx,	020h
	or	[edi+16],	ebx
	mov	ebx,	0800h
	or	[edi+24],	ebx
	mov	ebx,	010h
	or	[edi+32],	ebx
	mov	ebx,	010000h
	or	[edi+40],	ebx
	mov	ebx,	04h
	or	[edi+56],	ebx
	mov	ebx,	02000h
	or	[edi+72],	ebx
	mov	ebx,	0200000h
	or	[edi+80],	ebx
	mov	ebx,	02h
	or	[edi+88],	ebx
	mov	ebx,	040000h
	or	[edi+96],	ebx
	mov	ebx,	040h
	or	[edi+104],	ebx
	mov	ebx,	08000h
	or	[edi+112],	ebx
	mov	ebx,	08h
	or	[edi+120],	ebx
@@:	test	edx,	01000h
	jz	@F
	mov	ebx,	010000h
	or	[edi+12],	ebx
	mov	ebx,	080h
	or	[edi+20],	ebx
	mov	ebx,	0100000h
	or	[edi+28],	ebx
	mov	ebx,	04h
	or	[edi+44],	ebx
	mov	ebx,	020h
	or	[edi+60],	ebx
	mov	ebx,	08000h
	or	[edi+68],	ebx
	mov	ebx,	02h
	or	[edi+76],	ebx
	mov	ebx,	0200000h
	or	[edi+84],	ebx
	mov	ebx,	010h
	or	[edi+100],	ebx
	mov	ebx,	040000h
	or	[edi+108],	ebx
	mov	ebx,	020000h
	or	[edi+116],	ebx
	mov	ebx,	01h
	or	[edi+124],	ebx
@@:	test	edx,	02000h
	jz	@F
	mov	ebx,	02000h
	or	[edi+4],	ebx
	mov	ebx,	04h
	or	[edi+12],	ebx
	mov	ebx,	020h
	or	[edi+28],	ebx
	mov	ebx,	0800000h
	or	[edi+36],	ebx
	mov	ebx,	0200h
	or	[edi+44],	ebx
	mov	ebx,	01000h
	or	[edi+52],	ebx
	mov	ebx,	040h
	or	[edi+60],	ebx
	mov	ebx,	010h
	or	[edi+68],	ebx
	mov	ebx,	040000h
	or	[edi+76],	ebx
	mov	ebx,	020000h
	or	[edi+84],	ebx
	mov	ebx,	080000h
	or	[edi+92],	ebx
	mov	ebx,	08h
	or	[edi+100],	ebx
	mov	ebx,	0800h
	or	[edi+116],	ebx
@@:	test	edx,	04000h
	jz	@F
	mov	ebx,	02h
	or	[edi+4],	ebx
	mov	ebx,	0200h
	or	[edi+12],	ebx
	mov	ebx,	01000h
	or	[edi+20],	ebx
	mov	ebx,	040h
	or	[edi+28],	ebx
	mov	ebx,	04000h
	or	[edi+36],	ebx
	mov	ebx,	0100h
	or	[edi+44],	ebx
	mov	ebx,	01h
	or	[edi+52],	ebx
	mov	ebx,	010000h
	or	[edi+60],	ebx
	mov	ebx,	08h
	or	[edi+68],	ebx
	mov	ebx,	0800h
	or	[edi+84],	ebx
	mov	ebx,	02000h
	or	[edi+92],	ebx
	mov	ebx,	0400h
	or	[edi+100],	ebx
	mov	ebx,	0400000h
	or	[edi+108],	ebx
	mov	ebx,	08000h
	or	[edi+116],	ebx
	mov	ebx,	0800000h
	or	[edi+124],	ebx
@@:	test	edx,	08000h
	jz	@F
	mov	ebx,	040000h
	or	[edi+4],	ebx
	mov	ebx,	0100h
	or	[edi+12],	ebx
	mov	ebx,	01h
	or	[edi+20],	ebx
	mov	ebx,	010000h
	or	[edi+28],	ebx
	mov	ebx,	080h
	or	[edi+36],	ebx
	mov	ebx,	0100000h
	or	[edi+44],	ebx
	mov	ebx,	04h
	or	[edi+60],	ebx
	mov	ebx,	0400h
	or	[edi+68],	ebx
	mov	ebx,	0400000h
	or	[edi+76],	ebx
	mov	ebx,	08000h
	or	[edi+84],	ebx
	mov	ebx,	02h
	or	[edi+92],	ebx
	mov	ebx,	0200000h
	or	[edi+100],	ebx
	mov	ebx,	010h
	or	[edi+116],	ebx
	mov	ebx,	04000h
	or	[edi+124],	ebx
@@:	test	edx,	020000h
	jz	@F
	mov	ebx,	080h
	or	[edi+0],	ebx
	mov	ebx,	02000h
	or	[edi+8],	ebx
	mov	ebx,	0200000h
	or	[edi+16],	ebx
	mov	ebx,	02h
	or	[edi+24],	ebx
	mov	ebx,	040000h
	or	[edi+32],	ebx
	mov	ebx,	040h
	or	[edi+40],	ebx
	mov	ebx,	08000h
	or	[edi+48],	ebx
	mov	ebx,	0400h
	or	[edi+56],	ebx
	mov	ebx,	01000h
	or	[edi+64],	ebx
	mov	ebx,	020h
	or	[edi+72],	ebx
	mov	ebx,	0800h
	or	[edi+80],	ebx
	mov	ebx,	010h
	or	[edi+88],	ebx
	mov	ebx,	010000h
	or	[edi+96],	ebx
	mov	ebx,	04h
	or	[edi+112],	ebx
@@:	test	edx,	040000h
	jz	@F
	mov	ebx,	010h
	or	[edi+0],	ebx
	mov	ebx,	0800000h
	or	[edi+8],	ebx
	mov	ebx,	0200h
	or	[edi+16],	ebx
	mov	ebx,	0100000h
	or	[edi+24],	ebx
	mov	ebx,	02000h
	or	[edi+40],	ebx
	mov	ebx,	0200000h
	or	[edi+48],	ebx
	mov	ebx,	02h
	or	[edi+56],	ebx
	mov	ebx,	0100h
	or	[edi+64],	ebx
	mov	ebx,	0400000h
	or	[edi+80],	ebx
	mov	ebx,	08h
	or	[edi+88],	ebx
	mov	ebx,	01000h
	or	[edi+96],	ebx
	mov	ebx,	020h
	or	[edi+104],	ebx
	mov	ebx,	0800h
	or	[edi+112],	ebx
@@:	test	edx,	080000h
	jz	@F
	mov	ebx,	08h
	or	[edi+0],	ebx
	mov	ebx,	0400h
	or	[edi+8],	ebx
	mov	ebx,	080000h
	or	[edi+16],	ebx
	mov	ebx,	04000h
	or	[edi+24],	ebx
	mov	ebx,	0800000h
	or	[edi+40],	ebx
	mov	ebx,	0200h
	or	[edi+48],	ebx
	mov	ebx,	0100000h
	or	[edi+56],	ebx
	mov	ebx,	04h
	or	[edi+64],	ebx
	mov	ebx,	080h
	or	[edi+72],	ebx
	mov	ebx,	01h
	or	[edi+80],	ebx
	mov	ebx,	020000h
	or	[edi+88],	ebx
	mov	ebx,	0100h
	or	[edi+96],	ebx
	mov	ebx,	0400000h
	or	[edi+112],	ebx
	mov	ebx,	08000h
	or	[edi+120],	ebx
@@:	test	edx,	0100000h
	jz	@F
	mov	ebx,	08h
	or	[edi+20],	ebx
	mov	ebx,	0800h
	or	[edi+36],	ebx
	mov	ebx,	02000h
	or	[edi+44],	ebx
	mov	ebx,	0400h
	or	[edi+52],	ebx
	mov	ebx,	0400000h
	or	[edi+60],	ebx
	mov	ebx,	020h
	or	[edi+68],	ebx
	mov	ebx,	0800000h
	or	[edi+76],	ebx
	mov	ebx,	0200h
	or	[edi+84],	ebx
	mov	ebx,	01000h
	or	[edi+92],	ebx
	mov	ebx,	040h
	or	[edi+100],	ebx
	mov	ebx,	04000h
	or	[edi+108],	ebx
	mov	ebx,	0100h
	or	[edi+116],	ebx
	mov	ebx,	020000h
	or	[edi+124],	ebx
@@:	test	edx,	0200000h
	jz	@F
	mov	ebx,	02000h
	or	[edi+12],	ebx
	mov	ebx,	0400h
	or	[edi+20],	ebx
	mov	ebx,	0400000h
	or	[edi+28],	ebx
	mov	ebx,	08000h
	or	[edi+36],	ebx
	mov	ebx,	02h
	or	[edi+44],	ebx
	mov	ebx,	0200000h
	or	[edi+52],	ebx
	mov	ebx,	040h
	or	[edi+68],	ebx
	mov	ebx,	04000h
	or	[edi+76],	ebx
	mov	ebx,	0100h
	or	[edi+84],	ebx
	mov	ebx,	01h
	or	[edi+92],	ebx
	mov	ebx,	010000h
	or	[edi+100],	ebx
	mov	ebx,	080h
	or	[edi+108],	ebx
	mov	ebx,	0100000h
	or	[edi+116],	ebx
	mov	ebx,	0800h
	or	[edi+124],	ebx
@@:	test	edx,	0400000h
	jz	@F
	mov	ebx,	0800000h
	or	[edi+4],	ebx
	mov	ebx,	02h
	or	[edi+12],	ebx
	mov	ebx,	0200000h
	or	[edi+20],	ebx
	mov	ebx,	010h
	or	[edi+36],	ebx
	mov	ebx,	040000h
	or	[edi+44],	ebx
	mov	ebx,	020000h
	or	[edi+52],	ebx
	mov	ebx,	080000h
	or	[edi+60],	ebx
	mov	ebx,	010000h
	or	[edi+68],	ebx
	mov	ebx,	080h
	or	[edi+76],	ebx
	mov	ebx,	0100000h
	or	[edi+84],	ebx
	mov	ebx,	04h
	or	[edi+100],	ebx
	mov	ebx,	020h
	or	[edi+116],	ebx
	mov	ebx,	08000h
	or	[edi+124],	ebx
@@:	test	edx,	0800000h
	jz	@F
	mov	ebx,	04000h
	or	[edi+4],	ebx
	mov	ebx,	040000h
	or	[edi+12],	ebx
	mov	ebx,	020000h
	or	[edi+20],	ebx
	mov	ebx,	080000h
	or	[edi+28],	ebx
	mov	ebx,	08h
	or	[edi+36],	ebx
	mov	ebx,	0800h
	or	[edi+52],	ebx
	mov	ebx,	02000h
	or	[edi+60],	ebx
	mov	ebx,	04h
	or	[edi+68],	ebx
	mov	ebx,	020h
	or	[edi+84],	ebx
	mov	ebx,	0800000h
	or	[edi+92],	ebx
	mov	ebx,	0200h
	or	[edi+100],	ebx
	mov	ebx,	01000h
	or	[edi+108],	ebx
	mov	ebx,	040h
	or	[edi+116],	ebx
	mov	ebx,	010h
	or	[edi+124],	ebx
@@:	test	edx,	02000000h
	jz	@F
	mov	ebx,	080h
	or	[edi+8],	ebx
	mov	ebx,	01h
	or	[edi+16],	ebx
	mov	ebx,	020000h
	or	[edi+24],	ebx
	mov	ebx,	0100h
	or	[edi+32],	ebx
	mov	ebx,	0400000h
	or	[edi+48],	ebx
	mov	ebx,	08h
	or	[edi+56],	ebx
	mov	ebx,	0400h
	or	[edi+64],	ebx
	mov	ebx,	080000h
	or	[edi+72],	ebx
	mov	ebx,	04000h
	or	[edi+80],	ebx
	mov	ebx,	0800000h
	or	[edi+96],	ebx
	mov	ebx,	0200h
	or	[edi+104],	ebx
	mov	ebx,	0100000h
	or	[edi+112],	ebx
	mov	ebx,	04h
	or	[edi+120],	ebx
@@:	test	edx,	04000000h
	jz	@F
	mov	ebx,	010h
	or	[edi+8],	ebx
	mov	ebx,	010000h
	or	[edi+16],	ebx
	mov	ebx,	04h
	or	[edi+32],	ebx
	mov	ebx,	080h
	or	[edi+40],	ebx
	mov	ebx,	01h
	or	[edi+48],	ebx
	mov	ebx,	020000h
	or	[edi+56],	ebx
	mov	ebx,	02h
	or	[edi+64],	ebx
	mov	ebx,	040000h
	or	[edi+72],	ebx
	mov	ebx,	040h
	or	[edi+80],	ebx
	mov	ebx,	08000h
	or	[edi+88],	ebx
	mov	ebx,	0400h
	or	[edi+96],	ebx
	mov	ebx,	080000h
	or	[edi+104],	ebx
	mov	ebx,	04000h
	or	[edi+112],	ebx
	mov	ebx,	0800h
	or	[edi+120],	ebx
@@:	test	edx,	08000000h
	jz	@F
	mov	ebx,	08000h
	or	[edi+0],	ebx
	mov	ebx,	08h
	or	[edi+8],	ebx
	mov	ebx,	01000h
	or	[edi+16],	ebx
	mov	ebx,	020h
	or	[edi+24],	ebx
	mov	ebx,	0800h
	or	[edi+32],	ebx
	mov	ebx,	010h
	or	[edi+40],	ebx
	mov	ebx,	010000h
	or	[edi+48],	ebx
	mov	ebx,	0100000h
	or	[edi+64],	ebx
	mov	ebx,	02000h
	or	[edi+80],	ebx
	mov	ebx,	0200000h
	or	[edi+88],	ebx
	mov	ebx,	02h
	or	[edi+96],	ebx
	mov	ebx,	040000h
	or	[edi+104],	ebx
	mov	ebx,	040h
	or	[edi+112],	ebx
	mov	ebx,	0400000h
	or	[edi+120],	ebx
@@:	test	edx,	010000000h
	jz	@F
	mov	ebx,	010000h
	or	[edi+20],	ebx
	mov	ebx,	080h
	or	[edi+28],	ebx
	mov	ebx,	0100000h
	or	[edi+36],	ebx
	mov	ebx,	04h
	or	[edi+52],	ebx
	mov	ebx,	0400000h
	or	[edi+68],	ebx
	mov	ebx,	08000h
	or	[edi+76],	ebx
	mov	ebx,	02h
	or	[edi+84],	ebx
	mov	ebx,	0200000h
	or	[edi+92],	ebx
	mov	ebx,	010h
	or	[edi+108],	ebx
	mov	ebx,	040000h
	or	[edi+116],	ebx
	mov	ebx,	0100h
	or	[edi+124],	ebx
@@:	test	edx,	020000000h
	jz	@F
	mov	ebx,	0800h
	or	[edi+4],	ebx
	mov	ebx,	04h
	or	[edi+20],	ebx
	mov	ebx,	020h
	or	[edi+36],	ebx
	mov	ebx,	0800000h
	or	[edi+44],	ebx
	mov	ebx,	0200h
	or	[edi+52],	ebx
	mov	ebx,	01000h
	or	[edi+60],	ebx
	mov	ebx,	010h
	or	[edi+76],	ebx
	mov	ebx,	040000h
	or	[edi+84],	ebx
	mov	ebx,	020000h
	or	[edi+92],	ebx
	mov	ebx,	080000h
	or	[edi+100],	ebx
	mov	ebx,	08h
	or	[edi+108],	ebx
	mov	ebx,	0100000h
	or	[edi+124],	ebx
@@:	test	edx,	040000000h
	jz	@F
	mov	ebx,	08000h
	or	[edi+4],	ebx
	mov	ebx,	0800000h
	or	[edi+12],	ebx
	mov	ebx,	0200h
	or	[edi+20],	ebx
	mov	ebx,	01000h
	or	[edi+28],	ebx
	mov	ebx,	040h
	or	[edi+36],	ebx
	mov	ebx,	04000h
	or	[edi+44],	ebx
	mov	ebx,	0100h
	or	[edi+52],	ebx
	mov	ebx,	01h
	or	[edi+60],	ebx
	mov	ebx,	080000h
	or	[edi+68],	ebx
	mov	ebx,	08h
	or	[edi+76],	ebx
	mov	ebx,	0800h
	or	[edi+92],	ebx
	mov	ebx,	02000h
	or	[edi+100],	ebx
	mov	ebx,	0400h
	or	[edi+108],	ebx
	mov	ebx,	0400000h
	or	[edi+116],	ebx
	mov	ebx,	020h
	or	[edi+124],	ebx
@@:	test	edx,	080000000h
	jz	@F
	mov	ebx,	010h
	or	[edi+4],	ebx
	mov	ebx,	04000h
	or	[edi+12],	ebx
	mov	ebx,	0100h
	or	[edi+20],	ebx
	mov	ebx,	01h
	or	[edi+28],	ebx
	mov	ebx,	010000h
	or	[edi+36],	ebx
	mov	ebx,	080h
	or	[edi+44],	ebx
	mov	ebx,	0100000h
	or	[edi+52],	ebx
	mov	ebx,	02000h
	or	[edi+68],	ebx
	mov	ebx,	0400h
	or	[edi+76],	ebx
	mov	ebx,	0400000h
	or	[edi+84],	ebx
	mov	ebx,	08000h
	or	[edi+92],	ebx
	mov	ebx,	02h
	or	[edi+100],	ebx
	mov	ebx,	0200000h
	or	[edi+108],	ebx
	mov	ebx,	040h
	or	[edi+124],	ebx
@@:	ret

END