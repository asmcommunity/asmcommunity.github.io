;AUTHOR: DANIEL LUCAS

Include Irvine32.inc  ;includes Master Irvine's Library

.data
message1 BYTE "Please type the file name of the file to be compressed",0     ;initiates the variable message1 and assigns it "Please type the file name of the file to be compressed", 0
message2 BYTE "Please type the filename to save the compression to(must end in .drl):",0       ;initiates the variable message2 and assigns it "Please type the filename to save the compression to:", 0
message3 BYTE "Cannot compress an empty file!",0                             ;initiates the variable message3 and assigns it "Cannot compress and empty file", 0
message4 BYTE "Please select one of the options:",0dh, 0Ah,
				"1. Compress a file",0dh,0ah,
				"2. DeCompress a file",0dh,0ah,0               ;initiates the variable message4 and assigns it "PLease select one 1. Compress 2. DeCompress", 0
message5 BYTE "Please type the file name of the file to be DeCompressed(must be a .drl file):",0     ;initiates the variable message1 and assigns it "Please type the file name of the file to be Decompressed", 0
message6 BYTE "Please type the filename to save the DeCompressed File to:",0       ;initiates the variable message2 and assigns it "Please type the filename to save the DeCompression to:", 0
message7 BYTE "Cannot compress an empty file!",0                             ;initiates the variable message7 and assigns it "Cannot compress and empty file", 0
message12 BYTE "File error has occurred",0                                   ;initiates the variable message6 and assigns it "File error has occurred", 0
message11 BYTE "Here3", 0dh, 0Ah,0                                                 ;initiates the variable message8 and assigns it "Complete!", 0
message9 BYTE "Here1", 0dh, 0Ah,0   
message10 BYTE "Here2", 0dh, 0Ah,0   
message8 BYTE "Complete!",0   
handle1 DWORD ?								    ;Uninitialized variable for handle1
handle2 DWORD ?								    ;Uninitialized variable for handle2
handle3 DWORD ?								    ;Uninitialized variable for handle3
handle4 DWORD ?								    ;Uninitialized variable for handle4
BUFFER_SIZE = 1									;sets buffer size to 1
buffer BYTE BUFFER_SIZE DUP(0)							;Duplicates buffer to buffer size and sets to 0
bitBuffer BYTE BUFFER_SIZE DUP(0)							;Duplicates buffer to buffer size and sets to 0
bitsBuffer DWORD BUFFER_SIZE DUP(0)							;Duplicates buffer to buffer size and sets to 0
OutBitBuffer DWORD BUFFER_SIZE DUP(0)							;Duplicates buffer to buffer size and sets to 0
bytesRead DWORD ?								;Unintialized variable for bytesread
bytesWritten DWORD ?								;Unintialized variable for byteswritten
sourceFile BYTE 20 DUP(0),0							;initiates the variable for the sourceFile
destinationFile BYTE 20 DUP(0),0						;initiates the variable for the destinationFile
newword byte 46 DUP(?),0
fresh byte 46 DUP(0),0
store dword ?
wordArray DWORD ?
wordNum dword 1
wordNum2 dword 1
tempWord dword 15 dup(?),0
bitCount DWORD 0
outBitCount DWORD 0
outBitMask DWORD 0
nBits DWORD 8
currentPrefix DWORD ?
nextCodeWord DWORD ?
codeWordLength DWORD ?
suffix	BYTE 8192 DUP(?)
prefix DWORD 8192 DUP(?)
oldFileSize	DWORD ?
codeWord DWORD ?
cORd BYTE 0

.code
main PROC					;beginning of main proc
call ShowPrompt				;calls showprompt proc
call checkForEmptyFile		;calls checkforemptyfile proc
call getFileHandles			;calls getfilehandles proc
cmp cORd, 2
je here
call  compressFile
jmp here2
here:
call decompressFile
here2:
call flushBits
;call compressFile
;call readWrite				;calls the readWrite Proc
;call ReadFromFile1			;calls read from file1 proc
;call makeList
call closeFiles				;calls the closeFiles proc

exit						;properly exits
main ENDP					;end of main proc

;--------------------------------------------------------------------------------------------------------------;
readWrite Proc
	mov		nbits, 8				;the number of bits to be read
copy:								;beginning of the copy label
	call	readBits				;calls the readBits proc
	cmp		edx,0					;checks to see if anything was read
	jz		done					;if not: jump to done label
	mov		ecx, eax				;else, take what was read(eax) and put it into ecx to pass
	call	writeBits				;calls writeBits proc with ecx and edx from readBits
	jmp		copy					;continues until no bits were read
done:								;the done label
	call	flushBits				;calls the flushBits proc
	ret
readWrite ENDP

;--------------------------------------------------------------------------------------------------------------;
showPrompt PROC				;beginning of showPrompt proc
L1:							;loop one
	mov		edx, OFFSET message4	;moves the offset of message 4 into edx
	call	WriteString			;displays message4
	call	crlf					;new line
	call	ReadChar				;reads user input of a char
	cmp		al, "1"					;compares input to the char '1'
	je		Compress					;if equal then jump to Compress
	cmp		al, "2"					;compares user input to the char '2'
	je		DeCompress				;if equal then jump to DeCompress
	jmp		L1						;start loop over again if neither '1' nor '2'

Compress:					;beginning of Compress label
	mov		cORd, 1
	mov		edx, OFFSET message1			;moves the offset of message1 into edx
	call	WriteString						;displays message1
	call	crlf							;new line
	mov		edx, OFFSET sourceFile			;moves the offset of sourceFile into edx
	mov		ecx, SIZEOF sourceFile			;moves the sizeOf sourceFile into ecx
	call	ReadString						;reads the user input of a string
	call	crlf							;new line
Compressb:									;destination
	mov		edx, OFFSET message2			;moves the offset of message2 into edx
	call	WriteString						;displays message2
	call	crlf							;new line
	mov		edx, OFFSET destinationFile		;moves the offset of destinationFile into edx
	mov		ecx, SIZEOF destinationFile		;moves the sizeOf destinationFile into ecx
	call	ReadString						;reads the user input of a string
	call	crlf							;new line
	push	eax								;push eax onto stack
	mov		edi, OFFSET destinationFile		;puts the offset of destination file into edi
	mov		eax, ".drl"						;puts ".drl" into eax
	mov		ecx, SIZEOF destinationFile		;puts the SIZEOF destin file into ecx
	cld										;forward direction
	repne scasb								;search for .drl in the destination file name
	jnz		Compressb						;if not there, try again
	;dec	edi								;decrement edi
	pop		eax								;pop eax off the stack
	ret										;return

DeCompress:									;beginning of DeCompress Label
	mov		cORd, 2							;mov into cORd 2
	mov		edx, OFFSET message5			;moves the offset of message5 into ex
	call	WriteString						;displays message5
	call	crlf							;new line
	mov		edx, OFFSET sourceFile			;moves the offset of sourceFile into edx
	mov		ecx, SIZEOF sourceFile			;moves the sizeOf sourceFile into ecx
	call	ReadString						;reads the user input of a string
	call	crlf							;new line

	mov		edi, OFFSET sourceFile			;puts the offset of source file into edi
	mov		eax, ".drl"						;puts ".drl" into eax
	mov		ecx, SIZEOF sourceFile			;puts the SIZEOF source file into ecx
	cld										;forward direction
	repne scasb								;search for .drl in the source file name
	jnz		DeCompress						;if not there, try again

	mov		edx, OFFSET message6			;moves the offset of message6 into edx
	call	WriteString						;displays message6
	call	crlf							;new line
	mov		edx, OFFSET destinationFile		;moves the offset of destinationFile into edx
	mov		ecx, SIZEOF destinationFile		;moves the sizeOf destinationFile into ecx
	call	ReadString						;reads the user input of a string
	call	crlf							;new line
	ret										;return
showPrompt ENDP						;end of ShowPrompt proc

;--------------------------------------------------------------------------------------------------------------;
checkForEmptyFile PROC
	mov		edx, OFFSET sourceFIle				; moves the offset of sourceFile into edx
	call	OpenInputFile						; calls openinputfile proc
	cmp		eax, INVALID_HANDLE_VALUE			;check validity of file handle
	je		Quit2
	mov		handle3, eax						;jumps to quit label if invalid
	mov		edx, OFFSET buffer					;moves the offset of buffer into edx
	mov		ecx, BUFFER_SIZE
	mov		eax, handle3						;moves the buffer_Size into ecx
	call	ReadFromFile						;calls ReadFromFile
	mov		bytesRead, eax						;move eax into bytesRead
	cmp		eax, 0								;compare eax to 0
	je		QUIT								;if eax=0 then jump to quit
	mov		eax, handle3						;move handle3 into eax
	call	Closefile							;calls closeFiles proc		
	ret											;return

QUIT:										;beginning of QUIT label
	mov		edx, OFFSET message7			;moves the offset of message7 into edx
	call	WriteString						;displays message7				
	call	crlf							;new line
	exit									;properly exits

QUIT2:
	mov		edx, OFFSET message12			;moves the offset of message6 into edx
	call	WriteString						;calls writestring
	call	crlf							;new line
	exit									;exits
checkForEmptyFile ENDP						;end of checkForEmptyFile proc

;--------------------------------------------------------------------------------------------------------------;
getFileHandles PROC								;beginning of getfilehandles proc
	mov		edx, OFFSET sourceFIle				; moves the offset of sourceFile into edx
	call	OpenInputFile						; calls openinputfile proc
	cmp		eax, INVALID_HANDLE_VALUE			;check validity of file handle
	je		Quit								;jumps to quit label if invalid
	mov		handle1, eax						;moves eax into handle1
	mov		edx, OFFSET destinationFile			;moves the offset of the destination file into edx
	call	CreateOutputFile					;calls the CreatOutputFile proc
	cmp		eax, INVALID_HANDLE_VALUE			;check validity of file handle
	je		Quit								;jumps to quit label if invalid
	mov		handle2, eax						;moves eax into handle2
	ret											;return

QUIT:											;beginning of quit label
	mov		edx, OFFSET message12				;moves the offset of message6 into edx
	call	WriteString							;writes edx
	call	crlf								;new line
	call	crlf								;new line
	call	main								;calls main proc
getFileHandles ENDP								;end of getFileHandles Proc

;--------------------------------------------------------------------------------------------------------------;
closeFiles PROC									;beginning of closeFiles proc
	mov		eax, handle1						;moves handle1 into eax
	call	CloseFile							;closes the file
	mov		eax, handle2						;moves handle2 into eax
	call	CloseFile							;close the file
	ret											;return
closeFiles ENDP									;end of the closeFiles proc

;--------------------------------------------------------------------------------------------------------------;
writeBit Proc
	push	ecx					;preserve ecx
	push	edx					;preserve edx
	
	cmp		outBitCount, 0		;checks to see if bit count is 0
	jne		RunThru				;if not: go to the runThrough
	mov		outBitBuffer, 0		;if so: reset buffer
	mov		outBitMask, 1		;if so: reset mask
RunThru:						;runThru label
	cmp		ecx, 0				;checks to see if ecx is 0
	je		DOWN				;if so: jump to down label
	mov		eax, outBitMask		;if not: put mask into eax
	or		outBitBuffer, eax	;	   : and or buffer with eax
DOWN:							;down label
	shl		outBitMask,1		;shift mask left 1 bit
	inc		outBitCount			;inc bit count
	
	cmp		outBitCount, 8				;checks to see if full byte
	jne		Quit						;if not: jump to quit
	mov		eax, handle2				;else: print the byte				
	mov		edx, OFFSET OutBitBuffer	;"
	mov		ecx, BUFFER_SIZE			;"
	call	WriteToFile					;"
	mov		outBitCount,0				;reset bit count to 0
	
Quit:							;quit label
	pop		edx					;preserve edx
	pop		ecx					;preserve ecx
	ret							;return to caller
	
writeBit ENDP

;--------------------------------------------------------------------------------------------------------------;
writeBits Proc
	push	edi			;preserve edi
	push	ebx			;preserve ebx
	
	mov		edi, edx	;put edx into edi(the count)
	mov		ebx, ecx	;put ecx into ebx(whats to be written)
	
NextOut:				;next out label
	cmp		edi, 0		;is the count to 0?
	je		Quit		;if so: jump to Quit
	mov		ecx, 1		;make ecx 1
	and		ecx, ebx	;and it with ebx
	call	writeBit	;call the writeBit proc
	shr		ebx,1		;shift ebx right
	dec		edi			;dec the count
	jmp		NextOut		;go back to NextOut
	
Quit:					;Quit label
	pop		ebx			;preserve ebx
	pop		edi			;preserve edi
	ret					;return to caller
writeBits ENDP

;--------------------------------------------------------------------------------------------------------------; 
readBits PROC
	push	ebx				;preserve ebx
	push	esi				;preserve esi
	push	edi				;preserve edi
	
	mov		edx,0			;make edx 0
	mov		edi, nbits		;move the number of bits to be read into edi
	mov		ebx, 0			;make ebx 0
	mov		esi, 1			;make esi 1
NextOne:					;nextOne label
	cmp		edi, 0			;compare edi to 0
	je		Quit			;if equal: jump to Quit
	call	readBit			;else: call readBit proc
	cmp		eax, -1			;compare result to -1(nothing read)
	je		Quit			; if nothing read: jump to Quit
	inc		edx				;inc edx
	cmp		eax, 0			;compare what was read to 0
	je		Skip			;if 0: SKIP
	or		ebx, esi		;or's ebx with esi
Skip:						;Skip label
	shl		esi, 1			;shift esi left by one bit
	dec		edi				;dec edi
	jmp		NextOne			;jump back to nextOne

Quit:						;quit label
	mov		edx, nbits		;move nbits into edx
	sub		edx, edi		;subtracts bits read from bits to be read
	mov		eax, ebx		;puts result in eax to return
	pop		edi				;preserve edi
	pop		esi				;preserve esi
	pop		ebx				;preserve ebx

	ret						;return to caller
 
 readBits ENDP

;--------------------------------------------------------------------------------------------------------------;
readBit Proc
	cmp		bitCount, 0				;compare the bitCount with 0
	jne		NoNewByte				;if not 0: send to noNewByte
	mov		eax, handle1			;read from this file				
	mov		edx, OFFSET bitBuffer	;put it into this variable
	mov		ecx, BUFFER_SIZE		;read this much
	call	ReadFromFile			;calls readfromfile
	cmp		eax,0					;compare result with 0
	je		EOF						;if 0: go to EOF
	mov		bitCount, 8				;else: make bitCount 8
NoNewByte:							;noNewByte
	movzx	eax, [bitBuffer]		;move the bit into eax
	and		eax, 1					;and bit with 1
	shr		bitBuffer,1				;shift buffer right by 1 bit
	dec		bitCount				;dec bitCount
	ret								;return to caller
EOF:								;EOF label
	mov		eax,-1					;move -1 into eax to return if end of file
	ret								;return to caller
readBit EndP

;--------------------------------------------------------------------------------------------------------------;
flushBits Proc
	cmp		outBitCount,0			;compare the outBitCount to 0
	ja		Print					;if above 0: jump to Print
	ret								;else:return
Print:								;Print label
	mov eax, handle2				;write to this file
	mov edx, OFFSET OutBitBuffer	;from this buffer
	mov ecx, BUFFER_SIZE			;of this size
	call WriteToFile				;writes to the file
	ret								;return to caller
flushBits ENDP

;--------------------------------------------------------------------------------------------------------------;
initModel	PROC
	mov		currentPrefix,-1		;make currentPrefix -1
	mov		nextCodeWord, 256		;make the nextCodeWord 256
	mov		codeWordLength, 13		;make the codeWordLength 13
	mov 	ecx,0					;put 0 into ecx
SetPreSuf:							;sets up the first 255 values
	cmp		ecx,256					;if ecx=256
	je		Done					;then you are done
	mov		suffix[ecx], cl			;else: move cl into suffix at ecx
	mov		prefix[ecx*4], -1		;move -1 into every prefix
	inc		ecx						;increment ecx
	jmp		SetPreSuf				;do it again
Done:								;Done: label
	ret								;return
initModel ENDP

;--------------------------------------------------------------------------------------------------------------;
compressFile Proc
	push	edi				;preserve edi
	push	ebx				;preserve ebx
	push	esi				;preserve esi
	

	GetFileSize		PROTO STDCALL hFile:DWORD, lpFileSizeHigh:DWORD		;procedure for getting the size of a file
	invoke			GetFileSize, handle1, NULL							;call getFileSize with the sourceFile
	mov		ecx, eax			;put the size of the file into ecx
	mov		edx, 16				;put the size into edx
	call	writeBits			;call writeBits
	call	initModel			;call the model initiator
	mov		ebx, nextCodeWord	;put the nextCodeWord into ebx
RepeatMe:

	call	readBits				;calls the readBits proc
	cmp		edx,0					;checks to see if anything was read
	jz		done					;if not: jump to done label
	mov		edi,-1					;move -1 into edi
Traverse:							;traverse label
	inc		edi						;increment edi
	cmp		edi, nextCodeWord		;compare edi to nextCodeWord
	je		noMatchFound			;if equal, then we did not find a match
	
	mov		esi, currentPrefix		;else: move the currentPrefix into esi
	cmp		esi, prefix[edi*4]		;compare esi with prefix at edi*4
	jne		Traverse				;if not the same, then go back to Traverse
	cmp		al, suffix[edi]			;else: compare the suffix at edi to al
	jne		Traverse				;if not the same then go back to Traverse
			
	mov		currentPrefix,edi		;else: move edi into currentPrefix	
	jmp		repeatMe				;jump to repeatMe

noMatchFound:						;noMatchFoundLabel
	cmp		nextCodeWord, 8192		;DICTIONARY TOO BIG?
	je		NoAdd					;jump to NoAdd
	mov		prefix[ebx*4], esi		;else: move esi into prefix at the nextCodeWord*4
	mov		suffix[ebx], al			;move al into suffix at nextCodeWord
	inc		ebx						;increment ebx
	mov		nextCodeWord,ebx		;move ebx into nextCodeWord
NoAdd:								;noAdd label
	mov		ecx, currentPrefix		;move the currentPrefix into ecx
	mov		currentPrefix,eax		;move eax into currentPrefix
	mov		edx, codeWordLength		;put codeWordLength into edx
	call	writeBits				;calls writeBits proc
	jmp RepeatMe					;jump to repeatMe
Done:								;done label
	mov		ecx, currentPrefix		;move the currentPrefix into ecx
	mov		edx, codeWordLength		;move the codeWordLength into edx
	call	writeBits				;calls writeBits proc
	pop		edi						;preserve edi
	pop		ebx						;preserve ebx
	pop		esi						;preserve esi
	call	flushBits				;call flushBits to write any stragglers 
	ret								;return to caller
compressFile ENDP


;--------------------------------------------------------------------------------------------------------------;
deCompressFile Proc
	push	edi						;preserve edi
	push	ebx						;preserve ebx
	push	esi						;preserve esi
	mov		esi,0					;make esi 0
	call	initModel				;initialize the model
	mov		nBits, 16				;move into nBits 16
	call	readBits				;read 16 bits
	mov		oldFileSize, eax		;put eax into oldFileSize
	mov		eax, codeWordLength		;put codeWordLength into eax
	mov		nBits, eax				;put codeWordLength into nBits

RepeatMe:
	call	readBits				;calls the readBits proc
	cmp		edx,0					;checks to see if anything was read
	jz		done					;if not: jump to done label
	mov		esi, nextCodeWord		;put the value of the nextCodeWord into esi
	cmp		nextCodeWord, 8192		;compare nextCodeWord to 8192
	je		here					;if above: skip adding new codeWord
	cmp		currentPrefix,-1		;compare the currentPrefix to -1
	je		here					;if equal: jump the the here label
	mov		ebx, currentPrefix		;else: add currentPrefix to ebx
	mov		prefix[esi*4], ebx		;put ebx into the prefix at esi*4
	mov		currentPrefix,eax		;move eax into currentPrefix
	cmp		eax, esi				;compare esi with eax
	jne		here2					;if not equal: jump to here2
	mov		dl,suffix[esi-1]		;else: put the suffix at esi-1 into dl
	mov		suffix[esi],dl			;and put dl into the suffix at esi
here2:	
	inc		nextCodeWord			;increment the nextCodeWord
here:
	mov		ecx,eax					;put eax into ecx
	mov		currentPrefix,eax		;put eax into currentPrefix
	call	outputString			;call outputString
	mov		suffix[esi],dl			;move dl into suffix at esi
	sub		oldFileSize,eax			;sub eax from the oldFileSize
	jne		RepeatMe				;jump to repeatMe if oldFileSize isnt complete
Done:								;done label
	pop		edi						;preserve edi
	pop		ebx						;preserve ebx
	pop		esi						;preserve esi
	call	flushBits				;call flushBits to print any stragglers
	ret								;return to user

deCompressFile ENDP

;--------------------------------------------------------------------------------------------------------------;
outputString proc
	cmp		ecx,-1					;base case
	je		returnMe				;jump to returnMe
	push	ecx						;push ecx onto stack
	cmp		ecx,256					;compare ecx with 256
	jae		here					;if above or equal to: jump to here
	mov		edx,ecx					;else, move ecx into edx
	here:							;here Label
	mov		ecx, prefix[ecx*4]		;move the prefix at ecx*4 into ecx
	call	outputString			;recursion
	pop		ecx						;pops ecx from stack
	push	eax						;push eax onto the stack
	mov		cl, suffix[ecx]			;mov the suffix at ecx into cl
	push	edx						;push edx onto stack
	mov		edx, 8					;move 8 into edx(amount of bits to be written)
	call	writeBits				;call writeBits
	pop		edx						;pop edx back off the stack
	pop		eax						;pop eax back off the stack
	inc		eax						;increment eax
	ret								;return

returnMe:
	mov		eax, 0					;put 0 into eax
	ret								;return 0
	
outputString endP

end MAIN