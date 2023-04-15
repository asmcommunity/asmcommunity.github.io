%include '\nasm32\inc\win32\windows.inc'

segment .data
szClassName	db	"window_class",0

wc:
    	istruc WNDCLASSEX
        	at WNDCLASSEX.cbSize,          dd    WNDCLASSEX_size
        	at WNDCLASSEX.style,           dd    NULL
        	at WNDCLASSEX.lpfnWndProc,     dd    NULL
        	at WNDCLASSEX.cbClsExtra,      dd    NULL
        	at WNDCLASSEX.cbWndExtra,      dd    NULL
        	at WNDCLASSEX.hInstance,       dd    NULL
        	at WNDCLASSEX.hIcon,           dd    NULL
        	at WNDCLASSEX.hCursor,         dd    NULL
        	at WNDCLASSEX.hbrBackground,   dd    COLOR_WINDOW+1
        	at WNDCLASSEX.lpszMenuName,    dd    NULL
        	at WNDCLASSEX.lpszClassName,   dd    NULL
        	at WNDCLASSEX.hIconSm,         dd    NULL
    	iend

message:
    	istruc MSG
        	at MSG.hwnd,                   dd    NULL
        	at MSG.message,                dd    NULL
        	at MSG.wParam,                 dd    NULL
        	at MSG.lParam,                 dd    NULL
        	at MSG.time,                   dd    NULL
        	at MSG.pt,                     dd    NULL
    	iend

regFailCaption	db	"Registration Error!",0
regFailText	db	"Registration of Window Class failed!",0

createFailCap	db	"Window Error!",0
createFailTxt	db	"Creation of Window Failed!",0

windowTitle	db	"Window Title",0


segment .bss
hWnd		resd	1
hInstance	resd	1

%define	GetModuleHandleA _GetModuleHandleA@4
%define	ExitProcess _ExitProcess@4
%define	LoadIconA _LoadIconA@8
%define LoadCursorA _LoadCursorA@8
%define RegisterClassExA _RegisterClassExA@4
%define CreateWindowExA _CreateWindowExA@48
%define ShowWindow _ShowWindow@8
%define UpdateWindow _UpdateWindow@4
%define GetMessageA _GetMessageA@16
%define TranslateMessage _TranslateMessage@4
%define DispatchMessageA _DispatchMessageA@4
%define MessageBoxA _MessageBoxA@16
%define DefWindowProcA _DefWindowProcA@16
%define DestroyWindow _DestroyWindow@4
%define PostQuitMessage _PostQuitMessage@4

segment	.text
	extern	GetModuleHandleA
	extern	ExitProcess
	extern  LoadIconA
	extern	LoadCursorA
	extern	RegisterClassExA
	extern	CreateWindowExA
	extern	ShowWindow
	extern	UpdateWindow
	extern  GetMessageA
	extern	TranslateMessage
	extern  DispatchMessageA
	extern  MessageBoxA
	extern  DefWindowProcA
	extern	DestroyWindow
	extern  PostQuitMessage

	global	_WinMainCRTStartup
	

_WinMainCRTStartup:
	push	NULL
	call	GetModuleHandleA
	mov	[hInstance], eax

	push	SW_SHOWNORMAL
	push	NULL
	push	NULL
	push	dword [hInstance]
	call	WinMain

	push	NULL
	call	ExitProcess

	ret


%define hinst	[ebp+8]
%define hpinst  [ebp+12]
%define cmdl	[ebp+16]
%define	cmds	[ebp+20]
WinMain:
	push	ebp
	mov	ebp, esp

	mov	dword [wc + WNDCLASSEX.lpfnWndProc], WndProc

	mov	eax, hinst
	mov	[wc + WNDCLASSEX.hInstance], eax

	push	IDI_APPLICATION
	push	NULL
	call	LoadIconA
	
	mov	dword [wc + WNDCLASSEX.hIcon], eax
	mov	dword [wc + WNDCLASSEX.hIconSm], eax

	push	IDC_ARROW
	push	NULL
	call	LoadCursorA
	
	mov	dword [wc + WNDCLASSEX.hCursor], eax
	mov	dword [wc + WNDCLASSEX.lpszClassName], szClassName

	push	wc
	call	RegisterClassExA
	cmp	eax, 0
	jz	near regfail

	push	NULL
	push	dword [wc + WNDCLASSEX.hInstance]
	push	NULL
	push	NULL
	push	120
	push	240
	push	CW_USEDEFAULT
	push	CW_USEDEFAULT
	push	WS_OVERLAPPEDWINDOW
	push	windowTitle
	push	szClassName
	push	WS_EX_CLIENTEDGE
	call	CreateWindowExA

	cmp	eax, 0
	jz	near winfail
	
	mov	[hWnd], eax

	push	dword cmds
	push	dword [hWnd]
	call	ShowWindow

	push	dword [hWnd]
	call	UpdateWindow

msgLoop:
	push	0
	push	0
	push	NULL
	push	message
	call	GetMessageA

	cmp	eax, 0
	jle	near end_WinMain

	push	message
	call	TranslateMessage

	push	message
	call	DispatchMessageA

	jmp	msgLoop

	jmp	end_WinMain

regfail:
	mov	eax, MB_OK
	or	eax, MB_ICONEXCLAMATION

	push	eax
	push	regFailCaption
	push	regFailText
	push	NULL
	call	MessageBoxA

	jmp	end_WinMain

winfail:
	mov	eax, MB_OK
	or	eax, MB_ICONEXCLAMATION
	
	push	eax
	push	createFailCap
	push	createFailTxt
	push	NULL
	call	MessageBoxA

	jmp	end_WinMain

end_WinMain:
	mov	dword eax, [message + MSG.wParam]

	mov	esp, ebp
	pop	ebp
	ret	16

%define	hwnd	[ebp+8]
%define	msg	[ebp+12]
%define	wparam	[ebp+16]
%define lparam	[ebp+20]
WndProc:
	push	ebp
	mov	ebp, esp

	cmp	dword msg, WM_CLOSE
	jz	wm_close
	cmp	dword msg, WM_DESTROY
	jz	wm_destroy

	push	dword lparam
	push	dword wparam
	push	dword msg
	push	dword hwnd
	call	DefWindowProcA
	jmp	end_WndProc2


wm_close:
	push	dword hwnd
	call	DestroyWindow
	jmp	end_WndProc

wm_destroy:
	push	0
	call	PostQuitMessage

end_WndProc:
	xor	eax, eax
end_WndProc2:
	mov	esp, ebp
	pop	ebp
	ret	16
