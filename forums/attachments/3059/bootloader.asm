;XIX System 19 Bootloader - Version pre-Alpha
;Copyright (C) 2010 Brian D. Knopp
;
;This program is free software: you can redistribute it and/or modify
;it under the terms of the GNU General Public License as published by
;the Free Software Foundation, using version 3 of the License.
;
;This program is distributed in the hope that it will be useful,
;but WITHOUT ANY WARRANTY; without even the implied warranty of
;MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
;GNU General Public License for more details.
;
;You should have received a copy of the GNU General Public License
;along with this program.  If not, see <http://www.gnu.org/licenses/>.
;
;
;This version of the XIX System 19 Bootloader is used
;specifically to setup a suitable environment to load
;the XIX Kernel and execute it.  Future features
;include the ability to detect the best route to perform
;its appointed duty by BIOS ID, error handling/reporting
;and multi-boot features.
;
;For full documentation, read the source-enclosed
;bootloader.ldr.doc file

[BITS 16]	;Assembler Directive denoting this as 16-bet Real Mode
[ORG 7E00h]	;Assembler Directive denoting this as existing at 7E00h

begin:		;function to jump over the data section and into the main boot sequence
	jmp 7E00h:boot


;Section for Functions
prntstr:	;function to print a string loaded into AL
	mov ah, 0Eh		;load 0Eh (teletype) into AH

	mov bh, 00h		;set page number to 0 (page 1)
	mov bl, 0Ah		;set attribute to Green Text, Black Background, non-flashing

	.nxtchar		;label to repeat int 10h
	cld			;clear direction flag
	lodsb			;load string in SI, into AL
	or al, al		;check to see if AL = null char (set zero if true)

	jz .ret			;jump over int 10h and exit function

	int 10h			;call int 10h to print the string
	jmp .nxtchar		;repeat the routine if null character is not yet present

	.ret			;label to jump to, to get out of the loop
ret

a20en:		;function to enable the A20 Gate
	cli			;clear interrupts to disallow interuption of procedure

	in al, 92h		;open AL to port 92h
	or al, 02h		;send value 02h to AL register
	out 92h, al		;close AL port 92h

	sti			;re-enable interupts

	lea si, [a20mess] 	;load the A20 Message into SI register
	call prntstr		;print the string
ret

krnlod:		;function to load the kernel into memory
	jmp .rstdsk		;jump over error messages

	.rsterr
	lea si, [dskrsterr]	;load the error string into SI
	call prntstr		;print the string
	cli			;clear interupts
	hlt			;enter unrecoverable halt

	.rstdsk
	mov ah, 00h		;function to reset disk controller
	mov dl, [bootdrv]	;drive to reset controller
	
	int 13h			;call int 13h to reset disk
	jc.rsterr		;if carry flag is set (reset unsuccessfull) go to error routine
	
	jmp .rddsk		;jump over error function to read the disk

	.rderr			;label to jump to if error occurs
	lea si, [krnloderr]	;load the error sting into SI
	call prntstr		;print the string
	cli			;clear interupts
	hlt			;enter unrecoverable halt

	.rddsk
	mov ax, 9200h		;load 9200h into AX register (can't directly manipulate ES)
	mov es, ax		;set ES to contents of AX (location of memory to read to)
	xor bx, bx		;set offset to 0000h

	mov ah, 02h		;place 02h (read function) into AH
	mov al, 12h		;read 18 sectors
	mov ch, 00h		;read from cylinder 1
	mov cl, 04h		;read from sector 4
	mov dh, 00h		;read from head 0
	mov dl, [bootdrv]	;read from drive

	int 13h			;call int 13h to read from disk
	jc .rderr		;if carry flag is set (read unsuccessfull) go to error routine

	lea si, [krnmess]	;load the address of the Kernel Load Message into SI
	call prntstr		;print the string	
ret

gdtset:		;function to setup and load the Global Descriptor Table
	xor ax, ax		;clear ax register		
	mov es, ax
	mov si, gdt		;start of GDT table into SI register
	mov di, [gdtbse]	;locate GDT at 500h in memory
	mov cx, [gdtsze]	;size of the GDT (defined my fancy footwork)
	push ds			;remember pre-modified DS register
	mov ds, ax

	cld			;clear the direction flag
	rep movsb		;move byte from DS:SI to ES:DI

	pop ds			;restore pre-modified DS register

	lgdt[gdtr]		;load the Global Descriptor Table

	lea si, [gdtmess]	;load the address of the GDT Setup Message into SI
	call prntstr		;print the string
ret


;Section for Strings
sysmess db 'XIX System 19 Bootloader - Version pre-Alpha',13,10,0	;System Information String
									;13 = char return
									;10 = new line
									;0 = null character (terminate)

a20mess db 'A20 Gate Successfully Enabled.',13,10,0

krnmess db 'Kernel Loading Successful.',13,10,0

dskrsterr db 'ERROR - Reset of Disk Unsuccessfull - ABORT',13,10,10,0

krnloderr db 'ERROR - Kernel Loading Unsuccessfull - ABORT',13,10,10,0

gdtmess db 'Global Descriptor Table Successfully Setup.',13,10,0

pmodmess db 'Will now enable Protected Mode, and boot the Kernel...',13,10,10,0


;Section for Variables
bootdrv db 0		;variable to hold boot drive number

;Section for Descriptor Tables
gdtr:
gdtsze dw gdt-gdtend-1	;size of GDT is C0h (192 bytes), each descriptor is 64-bytes long
gdtbse dd 500h		;where GDT is located at (500h)

gdt:
null	equ $-gdt	;this is the null descriptor
	dd 0x0000	;fill the null selector with 00h properties
	dd 0x0000

code	equ $-gdt  	;code segment with 4GB flat memory model
	dw 0xFFFF	;first word of limit field (use with second to denote 4GB size)
	dw 0x0000	;first word of the base field (Descriptor begins at 00h)
	db 0x00		;third byte of the base field
	db 0x9A		;first byte of attribute flags: (present,ring0,predef,nonconforming,read/execute,not accessed)
	db 0xCF		;second word of the limit/attribute field: (Fh limit2, 4kb granularity,32bit,zero, available)
	db 0x00		;forth byte of the base field

data	equ $-gdt	;data segment with 4GB flat memory model
	dw 0xFFFF	;first word of limit field (use with second to denote 4GB size)
	dw 0x0000	;first word of the base field (Descriptor begins at 00h)
	db 0x00		;third byte of the base field
	db 0x92		;first byte of attribute flags: (present,ring0,predef,nonexpdown,read/write,not accessed)
	db 0xCF		;second word of the limit/attribute field: (Fh limit2, 4kb granularity,32bit,zero, available)
	db 0x00		;forth byte of the base field
gdtend:		;empty function to denote end of GDT in memory


;Main Program
boot:
	mov [bootdrv], dl	;save starting drive into variable [bootdrv]

	mov ax, 07E0h		;setup segment registers
    	mov ds, ax
    	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ax, 0000h		;stack grows down from 7C00h
    	mov ss, ax

    	mov sp, 7C00h		;stack size is 0200h bytes
	mov bp, 7C00h
 
    	cld			;clear the direction flag

	lea si, [sysmess]	;load effective address of string sysmess into SI
	call prntstr		;call function to print the string

	call a20en		;call the function to enable the A20 Gate

	call krnlod		;call the function to load the kernel

	call gdtset		;call the function to setup the Global Descripter Table	

	lea si, [pmodmess]	;load the PMode enabling message
	call prntstr		;print the string
	
	pmoden:			;inline function to enable Protected Mode
		cli			;permanently clear interrupts
		mov eax, cr0		;move contents of ControlRegister0 into ExtendedAX register
		or al, 1		;compare AL to value 1
		mov cr0, eax		;move CR0 into EAX register

	jmp code:.pmode		;jump to 32 bit code to straighten out EIP

	[bits 32]
	.pmode
		mov ax, data		;align all data segments with the data descriptor
		mov ds, ax
		mov es, ax
		mov fs, ax
		mov gs, ax

		mov ss, ax		;align stack segment with data descriptor
		mov eax, 7C00h		;align base and stack pointers
		mov esp, eax
		mov ebp, eax

		jmp code:9200h	;jump to residence of Kernel
		hlt		;halt processor from executing this binary

;set bootloader to 1024 bytes (2 segments) long
times 1024d-($-$$) db 0		;fill unused space upto 1024 bytes with 0