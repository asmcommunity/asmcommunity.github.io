.586

.model FLAT, STDCALL
option casemap:none
include windows.inc
include user32.inc
includelib user32.lib
include kernel32.inc
includelib kernel32.lib

WinMain proto :DWORD,:DWORD,:DWORD,:DWORD
GetDesktopSize proto

;RECT Struct
;left	LONG	?
;top		LONG	?
;right	LONG	?
;bottom	LONG	?
;RECT ends

.data
	ClassName	db	"AssemblyWindow!", 0
	AppName		db	"Simple Window Application", 0
	Text		db	"Assembly is fun!", 0
.data?
	hInstance		HINSTANCE	? ; instance handle of the program
	CommandLine		LPSTR		?
	XScreen			db			?
	YScreen			db			?
.code
start:	
	invoke	GetDesktopSize
	invoke	GetModuleHandle,NULL
	mov		hInstance,eax
	invoke	GetCommandLine
	mov		CommandLine,eax
	invoke	WinMain, hInstance,NULL,CommandLine,SW_SHOWDEFAULT
	invoke	ExitProcess,0
	WinMain proc hInst:HINSTANCE,hPrevInst:HINSTANCE,CmdLine:LPSTR,CmdShow:DWORD
	LOCAL	wc:WNDCLASSEX
	LOCAL	msg:MSG
	LOCAL	hwnd:HWND
	mov		wc.cbSize,SIZEOF WNDCLASSEX ;fill values in members of wc
	mov		wc.style, CS_HREDRAW or CS_VREDRAW
	mov 	wc.lpfnWndProc, OFFSET WndProc
	mov 	wc.cbClsExtra,NULL
	mov 	wc.cbWndExtra,NULL
	push 	hInstance
	pop 	wc.hInstance
	mov 	wc.hbrBackground,COLOR_WINDOW+1
	mov 	wc.lpszMenuName,NULL
	mov 	wc.lpszClassName,OFFSET ClassName
	invoke 	LoadIcon,NULL,IDI_APPLICATION
	mov 	wc.hIcon,eax
	mov 	wc.hIconSm,eax
	invoke 	LoadCursor,NULL,IDC_ARROW
	mov 	wc.hCursor,eax
	invoke 	RegisterClassEx, addr wc ;register our window class
	invoke 	CreateWindowEx, NULL,addr ClassName,addr AppName,WS_POPUP or WS_EX_TOPMOST,0,0,XScreen,YScreen,NULL,NULL,hInst,NULL
	mov 	hwnd,eax
	invoke 	ShowWindow, hwnd,CmdShow ;display our window on desktop
	invoke 	UpdateWindow, hwnd ;refresh the client area 
	.WHILE 	TRUE ;Enter message loop
	invoke 	GetMessage, ADDR msg,NULL,0,0
	.BREAK .IF (!eax)
	invoke 	TranslateMessage, ADDR msg
	invoke 	DispatchMessage, ADDR msg
	.ENDW
	mov 	eax,msg.wParam ;return exit code in eax
	ret
	WinMain endp
	WndProc proc hWnd:HWND, uMsg:UINT,wParam:WPARAM, lParam:LPARAM
	LOCAL	hdc:HDC
	LOCAL	ps:PAINTSTRUCT
	LOCAL	rect:RECT
	.IF 	uMsg==WM_DESTROY ;if the user closes our window
	invoke 	PostQuitMessage,NULL ;quit our application
	.ELSEIF uMsg==WM_PAINT
	invoke 	BeginPaint,hWnd,addr ps
	mov		hdc,eax	
	invoke	GetClientRect,hWnd, addr rect
	invoke	DrawText,hdc, addr Text,-1,addr rect, DT_SINGLELINE or DT_CENTER or DT_VCENTER
	invoke	EndPaint,hWnd, addr rect
	.ELSE
	invoke DefWindowProc,hWnd,uMsg,wParam,lParam ;Default message processing
	ret
	.ENDIF
	xor eax,eax
	ret
	WndProc endp
	GetDesktopSize proc
	LOCAL	DesktopRect:RECT
	LOCAL	DesktopWindow:HWND
	invoke GetDesktopWindow
	mov		DesktopWindow, eax
	invoke GetWindowRect,DesktopWindow,addr DesktopRect
	;width = right - left
	sub		eax,[DesktopRect.right],[DesktopRect.left]
	mov		XScreen,eax
	;height = bottom - top
	sub		eax,[DesktopRect.bottom],[DesktopRect.top]
	mov		YScreen, eax
	Ret
	GetDesktopSize endp
end start