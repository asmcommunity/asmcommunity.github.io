;XIX System 19 Bootsector - Version 1.00
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
;The purpose of this bootsector is simply to load
;the second stage of the XIX Bootloader, normally
;called the Bootloader.
;
;This is the case, in order to circumvent the now-
;restricting 512 (actually ~440) byte limit of the
;MBR, and to allow future implementations of error
;handling, error logging, hardware diagnostics,
;portability, and multi-boot support.
;
;For full documentation, read the source-enclosed
;bootsector.img.doc file.

[BITS 16]		;Assembler Directive denoting 16-bit Real-Mode
[ORG 7C00h]		;Assembler Directive denoting residence at 7C00h in memory

begin:		;function that jumps over the data section and into the boot sequence
	jmp 0000h:boot


;Section for Functions
prntstr:	;function that will print a string, loaded into SI, on screen
	mov ah, 0Eh	;place 0Eh (teletype function) into AH

	mov bh, 00h	;set page number to page 1
	mov bl, 0Ah	;set attribute to Green Text, Black Background, non-Flashing

	.nxtchar	;label to jump to if AL is not equal to Null Char
	lodsb		;load the string residing in SI, into AL for printing
	or al, al	;check to see if AL = Null Character

	jz .ret		;if zero, jump to label to return to main program

	int 10h		;call int 10h to print the string
	jmp .nxtchar

	.ret		;label to jump to if AL = Null Character
ret

readdsk:	;function that will reset the disk, then read it into 7E00h 
	jmp .rddsk		;jump over error function to read the disk

	.rderr			;label to jump to if error occurs
	lea si, [dskrderr]	;load the error sting into SI
	call prntstr		;print the string
	cli			;clear interupts
	hlt			;enter unrecoverable halt

	.rddsk
	mov ax, 7E00h		;load 7E00h into AX register (can't directly manipulate ES)
	mov es, ax		;set ES to contents of AX (location of memory to read to)
	xor bx, bx		;set offset to 0000h

	mov ah, 02h		;place 02h (read function) into AH
	mov al, 02h		;read 2 sectors
	mov ch, 00h		;read from cylinder 1
	mov cl, 02h		;read from sector 2
	mov dh, 00h		;read from head 0
	mov dl, [bootdrv]	;read from drive

	int 13h		;call int 13h to read from disk
	jc .rderr	;if carry flag is set (read unsuccessfull) go to error routine
ret


;Section for Strings
sysmess db 'XIX System 19 Bootsector - Version 1.00',13,10,0	;string to print system information
								;13 = character return
								;10 = new line
								;0 = null character (end string)

dsksucc db 'Disk Reading Successfull, Will now Load Stage 2.',13,10,10,0

dskrderr db 'ERROR - Disk Reading Unsuccessful - ABORT',13,10,'     ENTERING UNRECOVERABLE CPU HALT',0


;Section for Variables
bootdrv db 0	;variable to store drive number


;Main Program
boot:
	mov [bootdrv], dl	;load the boot-drive number into the bootdrv variable

	xor ax, ax		;refresh all segment registers
    	mov ds, ax
    	mov es, ax
	mov fs, ax
	mov gs, ax

	mov ax, 0000h		;set stack segment to grow down from 7C00h
    	mov ss, ax

    	mov sp, 7C00h		;stack is 0200h bytes in size

    	cld			;clear the direction flag

	lea si, [sysmess]	;load effective address of string sysmess into SI
	call prntstr		;call the function to print the string

	call readdsk		;call the function to load from disk into memory
	lea si, [dsksucc]	;dsksucc into SI
	call prntstr		;call the function to print the string

	jmp 7E00h:0000h		;jump to instruction residing in 7E00h (stage 2 bootloader)

	hlt			;halt the processor from executing this instruction


;Necessities for the Bootsector
times 510d-($-$$) db 0		;fill unused space upto 510 bytes (1FEh)
signature dw 0xAA55		;append the bootloader signature to last 2 bytes (1FEh-200h) 
