.386
.model flat,stdcall
option casemap:none
include \masm32\include\windows.inc
include \masm32\include\user32.inc
include \masm32\include\kernel32.inc
include \masm32\include\shell32.inc
includelib \masm32\lib\user32.lib
includelib \masm32\lib\kernel32.lib
includelib \masm32\lib\shell32.lib

WM_SHELLNOTIFY equ WM_USER+5
IDI_TRAY equ 0
IDM_RESTORE equ 1000
WinMain proto :DWORD,:DWORD,:DWORD,:DWORD

.const
IDR_MAINMENU	    equ 101
IDR_CHILDMENU	   equ 102
IDM_EXIT		      equ  40001
IDM_TILEHORZ	equ 40002
IDM_TILEVERT	equ 40003
IDM_CASCADE		equ 40004
IDM_NEW 		equ 40005
IDM_CLOSE		  equ 40006

.data
ClassName db "MDIASMClass",0
MDIClientName db "MDICLIENT",0
MDIChildClassName db "Win32asmMDIChild",0
MDIChildTitle db "MDI Child",0
AppName  db "Win32asm MDI Demo",0
ClosePromptMessage db "Are you sure you want to close this window?",0
RestoreString	db "&Restore",0
ExitString		db "E&xit Program",0

.data?
hInstance dd ?
hMainMenu dd ?
hwndClient dd ?
hChildMenu dd ?
mdicreate MDICREATESTRUCT <>
hwndFrame dd ?
note NOTIFYICONDATA <>
hPopupMenu	dd ?

.code
start:
	invoke GetModuleHandle, NULL
	mov    hInstance,eax
	invoke WinMain, hInstance,NULL,NULL, SW_SHOWDEFAULT
	invoke ExitProcess,eax

WinMain proc hInst:HINSTANCE,hPrevInst:HINSTANCE,CmdLine:LPSTR,CmdShow:DWORD
	LOCAL wc:WNDCLASSEX
	LOCAL msg:MSG
	;=============================================
	; Register the frame window class
	;=============================================
	mov   wc.cbSize,SIZEOF WNDCLASSEX
	mov   wc.style, CS_HREDRAW or CS_VREDRAW or CS_DBLCLKS
	mov   wc.lpfnWndProc, OFFSET WndProc
	mov   wc.cbClsExtra,NULL
	mov   wc.cbWndExtra,NULL
	push  hInstance
	pop   wc.hInstance
	mov   wc.hbrBackground,COLOR_APPWORKSPACE+1
	mov   wc.lpszMenuName,IDR_MAINMENU
	mov   wc.lpszClassName,OFFSET ClassName
	invoke LoadIcon,NULL,IDI_APPLICATION
	mov   wc.hIcon,eax
	mov   wc.hIconSm,eax
	invoke LoadCursor,NULL,IDC_ARROW
	mov   wc.hCursor,eax
	invoke RegisterClassEx, addr wc
	;================================================
	; Register the MDI child window class
	;================================================
	mov  wc.style, CS_HREDRAW or CS_VREDRAW or CS_BYTEALIGNWINDOW
	mov wc.lpfnWndProc,offset ChildProc
	mov wc.hbrBackground,COLOR_WINDOW+1
	mov wc.lpszClassName,offset MDIChildClassName
	invoke RegisterClassEx,addr wc
	invoke CreateWindowEx,NULL,ADDR ClassName,ADDR AppName,\
	   WS_OVERLAPPEDWINDOW or WS_CLIPCHILDREN,CW_USEDEFAULT,\
	   CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,0,\
	   hInst,NULL
	mov   hwndFrame,eax
	invoke LoadMenu,hInstance, IDR_CHILDMENU
	mov hChildMenu,eax
	invoke ShowWindow, hwndFrame,SW_SHOWNORMAL
	invoke UpdateWindow, hwndFrame
	.while TRUE
		invoke GetMessage, ADDR msg,NULL,0,0
		.break .if (!eax)
		invoke TranslateMDISysAccel,hwndClient,addr msg
		.if eax==0
			invoke TranslateMessage, ADDR msg
			invoke DispatchMessage, ADDR msg
		.endif
	.endw
	invoke DestroyMenu, hChildMenu
	mov	eax,msg.wParam
	ret
WinMain endp

WndProc proc hWnd:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
	LOCAL pt:POINT
	LOCAL ClientStruct:CLIENTCREATESTRUCT
	.if uMsg==WM_CREATE
		invoke GetMenu,hWnd
		mov hMainMenu,eax
		invoke GetSubMenu,hMainMenu,1
		mov ClientStruct.hWindowMenu,eax
		mov ClientStruct.idFirstChild,100
		INVOKE CreateWindowEx,NULL,ADDR MDIClientName,NULL,\
			WS_CHILD or WS_VISIBLE or WS_CLIPCHILDREN,1,\
			1,320,200,hWnd,NULL,\
			hInstance,addr ClientStruct
		   mov hwndClient,eax
		   ;=======================================
		   ; Initialize the MDICREATESTRUCT
		   ;=======================================
		mov mdicreate.szClass,offset MDIChildClassName
		mov mdicreate.szTitle,offset MDIChildTitle
		push hInstance
		pop mdicreate.hOwner
		mov mdicreate.x,CW_USEDEFAULT
		mov mdicreate.y,CW_USEDEFAULT
		mov mdicreate.lx,CW_USEDEFAULT
		mov mdicreate.ly,CW_USEDEFAULT
	    ;Set up the Tray Icon Popup Menu
		invoke CreatePopupMenu
		mov hPopupMenu,eax
		invoke AppendMenu,hPopupMenu,MF_STRING,IDM_RESTORE,addr RestoreString
		invoke AppendMenu,hPopupMenu,MF_STRING,IDM_EXIT,addr ExitString
      .elseif uMsg==WM_SIZE
		.if wParam==SIZE_MINIMIZED
			mov note.cbSize,sizeof NOTIFYICONDATA
			push hWnd
			pop note.hwnd
			mov note.uID,IDI_TRAY
			mov note.uFlags,NIF_ICON+NIF_MESSAGE+NIF_TIP
			mov note.uCallbackMessage,WM_SHELLNOTIFY
			invoke LoadIcon,NULL,IDI_WINLOGO
			mov note.hIcon,eax
			invoke lstrcpy,addr note.szTip,addr AppName
			invoke ShowWindow,hWnd,SW_HIDE
			invoke Shell_NotifyIcon,NIM_ADD,addr note
		.endif
	.elseif uMsg==WM_COMMAND
		.if lParam==0
			invoke Shell_NotifyIcon,NIM_DELETE,addr note
			mov eax,wParam
			.if ax==IDM_RESTORE
				invoke ShowWindow,hWnd,SW_RESTORE
			.elseif ax==IDM_EXIT
				invoke SendMessage,hWnd,WM_CLOSE,0,0
			.elseif ax==IDM_TILEHORZ
				invoke SendMessage,hwndClient,WM_MDITILE,MDITILE_HORIZONTAL,0
			.elseif ax==IDM_TILEVERT
				invoke SendMessage,hwndClient,WM_MDITILE,MDITILE_VERTICAL,0
			.elseif ax==IDM_CASCADE
				invoke SendMessage,hwndClient,WM_MDICASCADE,MDITILE_SKIPDISABLED,0
			.elseif ax==IDM_NEW
				invoke SendMessage,hwndClient,WM_MDICREATE,0,addr mdicreate
			.elseif ax==IDM_CLOSE
				invoke SendMessage,hwndClient,WM_MDIGETACTIVE,0,0
				invoke SendMessage,eax,WM_CLOSE,0,0
			.else
				invoke DefFrameProc,hWnd,hwndClient,uMsg,wParam,lParam
				ret
			.endif
		.endif
	.elseif uMsg==WM_SHELLNOTIFY
		.if wParam==IDI_TRAY
			.if lParam==WM_RBUTTONDOWN
				invoke GetCursorPos,addr pt
				invoke TrackPopupMenu,hPopupMenu,TPM_RIGHTALIGN,pt.x,pt.y,NULL,hWnd,NULL
			.elseif lParam==WM_LBUTTONDBLCLK
				invoke SendMessage,hWnd,WM_COMMAND,IDM_RESTORE,0
			.endif
		.endif
	.elseif uMsg==WM_DESTROY
		invoke PostQuitMessage,NULL
	.else
		invoke DefFrameProc,hWnd,hwndClient,uMsg,wParam,lParam
		ret
	.endif
	xor eax,eax
	ret
WndProc endp
ChildProc proc hChild:DWORD,uMsg:DWORD,wParam:DWORD,lParam:DWORD
	.if uMsg==WM_MDIACTIVATE
		mov eax,lParam
		.if eax==hChild
			invoke GetSubMenu,hChildMenu,1
			mov edx,eax
			invoke SendMessage,hwndClient,WM_MDISETMENU,hChildMenu,edx
		.else
			invoke GetSubMenu,hMainMenu,1
			mov edx,eax
			invoke SendMessage,hwndClient,WM_MDISETMENU,hMainMenu,edx
		.endif
		invoke DrawMenuBar,hwndFrame
	.elseif uMsg==WM_CLOSE
		invoke MessageBox,hChild,addr ClosePromptMessage,addr AppName,MB_YESNO
		.if eax==IDYES
			invoke SendMessage,hwndClient,WM_MDIDESTROY,hChild,0
		.endif
	.else
		invoke DefMDIChildProc,hChild,uMsg,wParam,lParam
		ret
	.endif
	xor eax,eax
	ret
ChildProc endp
end start
