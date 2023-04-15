.586p
.model flat,stdcall
option casemap:none
option proc:private

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

incAPI MACRO files:VARARG
	FOR file, <files>
		include		file&.inc
		includelib	file&.lib
	ENDM
ENDM

include		<windows.inc>
incAPI		<kernel32,user32>

ASSUME FS:NOTHING

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; DATA? section
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.data?
	dwBytes		DWORD	? 

;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; DATA section
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.data
	message1	BYTE	"String 1",13,10
	message2	BYTE	"String 2",13,10
	message3	BYTE	"String 3",13,10


;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
; CODE section
;~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.code

ENTRY32:
	invoke	CreateFile, CTEXT("result.txt"), GENERIC_WRITE, FILE_SHARE_READ or FILE_SHARE_WRITE, \
				0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0
	.IF (eax==INVALID_HANDLE_VALUE)
		invoke	MessageBox, 0, CTEXT("Error!"), CTEXT("Can't create file"), MB_OK
		invoke	ExitProcess, -1
	.ENDIF

	; file handle is saved in EBX, since that's one of the registers that aren't trashed by
	; API calls. SIZEOF is used instead of strlen, since SIZEOF is done at assemble-time instead
	; of a runtime call to lstrlen.
	mov		ebx, eax
	invoke	WriteFile, ebx, ADDR message1, SIZEOF message1, addr dwBytes, 0
	invoke	WriteFile, ebx, ADDR message2, SIZEOF message2, addr dwBytes, 0
	invoke	WriteFile, ebx, ADDR message3, SIZEOF message3, addr dwBytes, 0

	invoke	CloseHandle, ebx

	invoke	ExitProcess, 0

END ENTRY32
