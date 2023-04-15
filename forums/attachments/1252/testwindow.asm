; Example.asm
; ===========
;
; example.asm, a part of the NASM Win32 Coding Toolkit.
; This File demonstrates the use of NASM to make win32 programs.
;
; Not very pretty but it demonstrates some importent points
; about coding with NASM.
;
; Coded By Gij
; If you need to contact me, my E-mail is: gij <at> bigfoot.com

%include "C:\NASM\inc\win32n.inc"

extern RegisterClassA
IMPORT RegisterClassA user32.dll
extern CreateWindowExA
IMPORT CreateWindowExA user32.dll
;extern CreateWindow
;IMPORT CreateWindow user32.dll
extern ShowWindow
IMPORT ShowWindow user32.dll
extern UpdateWindow
IMPORT UpdateWindow user32.dll
extern GetMessageA
IMPORT GetMessageA user32.dll
extern TranslateMessage
IMPORT TranslateMessage user32.dll
extern DispatchMessageA
IMPORT DispatchMessageA user32.dll
extern DefWindowProcA
IMPORT DefWindowProcA user32.dll
extern ExitProcess
IMPORT ExitProcess kernel32.dll
extern MessageBeep
IMPORT MessageBeep user32.dll
extern GetLastError
IMPORT GetLastError kernel32.dll
extern LoadIconA
IMPORT LoadIconA user32.dll
extern LoadCursorA
IMPORT LoadCursorA user32.dll
extern PostQuitMessage
IMPORT PostQuitMessage user32.dll
extern BeginPaint
IMPORT BeginPaint user32.dll
extern EndPaint
IMPORT EndPaint user32.dll
extern DrawTextA
IMPORT DrawTextA user32.dll

global _WinMain@16

SEGMENT .text USE32 class=code
_WinMain@16:

%define ebp_hInstance       ebp+8	; handle of current instance
%define ebp_hPrevInstance   ebp+0ch	; handle of previous instance
%define ebp_lpszCmdLine     ebp+10h	; pointer to command line
%define ebp_nCmdShow        ebp+14h	; show state of window

	push ebp
	mov ebp,esp

RegisterWindowClass:

	mov eax,[ebp_hInstance]
	mov [WindowClassStruc+WNDCLASS.hInstance],eax
	
	push LPCTSTR IDI_APPLICATION
	push HINSTANCE NULL
	call LoadIconA

	mov [WindowClassStruc+WNDCLASS.hIcon],eax

	push LPCTSTR IDC_CROSS
	push HINSTANCE NULL
	call LoadCursorA

	mov [WindowClassStruc+WNDCLASS.hCursor],eax

	push dword WindowClassStruc
	call RegisterClassA

	test eax,eax
	jnz .Success

.Fail:
	call GetLastError
	jmp FareWell

.Success:


MakeWindow:

	push LPVOID NULL
	push HINSTANCE [ebp_hInstance]
	push HMENU NULL
	push HWND NULL
	push INTEGER 50h
	push INTEGER 150h
	push INTEGER CW_USEDEFAULT  ; 
	push INTEGER CW_USEDEFAULT  ; 
	push DWORD WS_OVERLAPPEDWINDOW
	push LPCTSTR WindowTitle
	push LPCTSTR ClassName
	push DWORD WS_EX_OVERLAPPEDWINDOW
        call CreateWindowExA

	test eax,eax
	jnz .Success

.Fail:
	call GetLastError
	jmp FareWell

.Success:

	mov [WindowHandle],eax

	push INTEGER  0xa
	push HWND [WindowHandle]
	call ShowWindow

	push eax
	call UpdateWindow
	
MsgLoop:

	push UINT 0
	push UINT 0
	push HWND 0
	push LPMSG WindowMSG
	call GetMessageA

	or eax,eax
	jz FareWell

	push DWORD WindowMSG
	call TranslateMessage

	push DWORD WindowMSG
	call DispatchMessageA
	
	jmp MsgLoop

FareWell:

	push UINT 0
	call ExitProcess

WndProc:

%define ebp_hWnd   ebp+8		; handle of window
%define ebp_Msg    ebp+0ch		; message
%define ebp_wParam ebp+10h		; first message parameter
%define ebp_lParam ebp+14h		; second message parameter
%define ebp_DC	  ebp-4

	push ebp
	mov ebp,esp

	cmp dword [ebp_Msg],WM_DESTROY
	jz Destroy_Handler

	cmp dword [ebp_Msg],WM_PAINT
	jz Paint_Handler

.DefMsgHandler:

	push dword [ebp_lParam]
	push dword [ebp_wParam]
	push dword [ebp_Msg]
	push dword [ebp_hWnd]
	call DefWindowProcA
.Exit:

	mov esp,ebp
	pop ebp
	ret 0ch

Destroy_Handler:

	push INTEGER 0
	call PostQuitMessage
	jmp WndProc.Exit

Paint_Handler:

	push ebp
	mov ebp,esp
	sub esp,4

	push dword PaintSt
	push HWND [WindowHandle]
	call BeginPaint

	or eax,eax
	jz Paint_Handler.Exit

	mov [ebp_DC],eax	
	
	push UINT DT_CENTER
	push LPRECT RectSt
	push INTEGER BOXTEXTLEN
	push LPCTSTR BoxText
	push HDC [ebp_DC]
	call DrawTextA

	push dword PaintSt
	push HWND [WindowHandle]
	call EndPaint
.Exit:

	mov esp,ebp
	pop ebp

	jmp WndProc.DefMsgHandler

SEGMENT .data USE32 class=data

WindowTitle 		db "NASM Win32 Coding Example by Gij",0
ClassName 		db "NASM_WIN32",0
WindowHandle		dd 0

BoxText 		db "Got Milk?"
BOXTEXTLEN		equ $-BoxText

WindowClassStruc:
ISTRUC WNDCLASS
at WNDCLASS.style,            dd    0
at WNDCLASS.lpfnWndProc,      dd    WndProc
at WNDCLASS.cbClsExtra,       dd    0
at WNDCLASS.cbWndExtra,       dd    0
at WNDCLASS.hInstance,        dd    0
at WNDCLASS.hIcon,            dd    NULL
at WNDCLASS.hCursor,          dd    NULL
at WNDCLASS.hbrBackground,    dd    1
at WNDCLASS.lpszMenuName,     dd    NULL
at WNDCLASS.lpszClassName,    dd    ClassName
IEND

WindowMSG:
ISTRUC MSG
at MSG.hwnd,              	dd    0
at MSG.message, 	  	dd    0
at MSG.wParam,            	dd    0
at MSG.lParam,            	dd    0
at MSG.time,              	dd    0
;at MSG.x,                	dd    0
;at MSG.y,                	dd    0
IEND

PaintSt:
ISTRUC PAINTSTRUCT
IEND

RectSt:
ISTRUC RECT
at RECT.left, 			dd 0
at RECT.top, 			dd 0
at RECT.right, 			dd 150h
at RECT.bottom,			dd 50h
IEND