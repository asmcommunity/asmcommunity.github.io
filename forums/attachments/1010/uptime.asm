.686p
.model flat,stdcall
option casemap:none

include <windows.inc>
include <kernel32.inc>
include <user32.inc>
includelib <kernel32.lib>
includelib <user32.lib>

.data?
	r_h		DWORD ?
	r_m		DWORD ?
	r_s		DWORD ?

	buffer	BYTE 128 dup (?)
	
.data
	div_h	DWORD	(1000*60*60)
	div_m	DWORD	(1000*60)
	div_s	DWORD	(1000)

	szFormat	BYTE	"%d hours, %d minutes and %d seconds.", 0
	szUptime	BYTE	"Windows uptime:", 0
	
.code

START:
	invoke	GetTickCount

	xor		edx, edx
	div		[div_h]
	mov		[r_h], eax

	mov		eax, edx
	xor		edx, edx
	div		[div_m]
	mov		[r_m], eax

	mov		eax, edx
	xor		edx, edx
	div		[div_s]
	mov		[r_s], eax

	invoke	wsprintf, addr buffer, addr szFormat, [r_h], [r_m], [r_s]

	invoke	MessageBox, 0, addr buffer, addr szUptime, MB_OK
	
	
	invoke	ExitProcess, 0	


end START
