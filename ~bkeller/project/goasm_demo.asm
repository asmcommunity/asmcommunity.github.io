;::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::;
; GoASM Demo Version 0.0.1                                                 ;
;                                                                          ;
; Author: Bryant Keller                                                    ;
; Description:                                                             ;
;  A simple demo application in GoASM.                                     ;
;                                                                          ;
; Homepage: http://www.asmcommunity.net/~bkeller/                          ;
; Email: bkeller@asmcommunity.net                                          ;
;                                                                          ;
; Build With:                                                              ;
;  GoAsm /nw /fo $$$.obj goasm_demo.asm                                    ;
;  GoLink /entry Start /fo demo.exe $$$.obj && del *.obj                   ;
;::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::;

; ------------------------------------------------------------------------ ;
; Includes
; ------------------------------------------------------------------------ ;

#INCLUDE <WIN32A.INC>

; ------------------------------------------------------------------------ ;
; Equates
; ------------------------------------------------------------------------ ;

DEFAULT_WIDTH	EQU	400
DEFAULT_HEIGHT	EQU	300
DEFAULT_STYLE	EQU	WS_OVERLAPPEDWINDOW + WS_VISIBLE

; ------------------------------------------------------------------------ ;
; Initialized Data
; ------------------------------------------------------------------------ ;
DATA SECTION

	; ---------------------------------------------------------------- ;
	; Strings
	; ---------------------------------------------------------------- ;

	szClassName	DB	"MyProjectClass", 0
	szAppName	DB	"GoASM Demo", 0

	; ---------------------------------------------------------------- ;
	; Message Table
	; ---------------------------------------------------------------- ;

ALIGN 4
	MsgTable	DD	OnCreate,	WM_CREATE
			DD	OnCommand,	WM_COMMAND
			DD	OnClose,	WM_CLOSE
			DD	OnDestroy,	WM_DESTROY

	; ---------------------------------------------------------------- ;
	; Window Class
	; ---------------------------------------------------------------- ;
ALIGN 4
                                                                                         ; --------------------------------------
                                                                                         ; Error!
                                                                                         ; Line 56 of assembler source file (source.asm):-
                                                                                         ; Unexpected material:-
                                                                                         ; --------------------------------------
	wc		WNDCLASSEX < \
				SIZEOF WNDCLASSEX, \		; cbSize
				CS_HREDRAW + CS_VREDRAW, \	; style
				OFFSET WndProc, \		; lpfnWndProc
				0, \				; cbClsExtra
				0, \				; cbWndExtra
				0, \				; hInstance
				0, \				; hIcon
				0, \				; hCursor
				COLOR_BTNFACE + 1, \		; hbrBackground
				0, \				; lpszMenuName
				OFFSET szClassName, \		; lpszClassName
				0 \				; hIconSm
			>

; ------------------------------------------------------------------------ ;
; Executable Code
; ------------------------------------------------------------------------ ;
CODE SECTION

; ------------------------------------------------------------------------ ;
  Start:
; ------------------------------------------------------------------------ ;
; Routine Description:
;
;     The program entry point procedure initializes the applications
;     environment, loads the primary resources, registers the window
;     class with the operating system, creates and displays the main
;     window, preforms the message pump, then cleans up and exits to
;     the operating system.
;
; Arguments:
;
;     None.
;
; Return Value:
;
;     The value in Msg.wParam as set by the message pump.
;
; ------------------------------------------------------------------------ ;

FRAME
LOCAL	hWnd, Msg:MSG
                                                                                         ; --------------------------------------
                                                                                         ; Also, if I remove the above structure
                                                                                         ; it errors on 'Msg:MSG'
                                                                                         ; --------------------------------------
	; ---------------------------------------------------------------- ;
	; Initialize Environment
	; ---------------------------------------------------------------- ;

	_GetModuleHandleA ( NULL )
	mov	D[wc.hInstance], EAX

	; ---------------------------------------------------------------- ;
	; Load Resources
	; ---------------------------------------------------------------- ;

	_LoadIconA ( NULL, IDI_APPLICATION )
	mov	D[wc.hIcon], EAX
	mov	D[wc.hIconSm], EAX

	_LoadCursorA ( NULL, IDC_ARROW )
	mov	D[wc.hCursor], EAX

	; ---------------------------------------------------------------- ;
	; Initialize Main Window
	; ---------------------------------------------------------------- ;
	_RegisterClassExA ( ADDR wc )

	_GetSystemMetrics ( SM_CYSCREEN )
	mov	EBX, EAX
	_GetSystemMetrics ( SM_CXSCREEN )
	shr	EBX, 1
	shr	EAX, 1
	sub	EBX, DEFAULT_HEIGHT / 2
	sub	EAX, DEFAULT_WIDTH  / 2
	mov	ECX, DEFAULT_WIDTH
	mov	EDX, DEFAULT_HEIGHT

	; ---------------------------------------------------------------- ;
	; Create Main Window
	; ---------------------------------------------------------------- ;

	_CreateWindowExA ( 0, ADDR szClassName, ADDR szAppName, DEFAULT_STYLE, EAX, EBX, ECX, EDX, 0, 0, D[wc.hInstance], 0 )
	test	EAX, EAX
	jz	>.exit
	mov	D[hWnd], EAX

	; ---------------------------------------------------------------- ;
	; Display Main Window
	; ---------------------------------------------------------------- ;

	_ShowWindow ( D[hWnd], SW_SHOWNORMAL )
	_UpdateWindow ( D[hWnd] )

	; ---------------------------------------------------------------- ;
	; Process Messages
	; ---------------------------------------------------------------- ;

:	_GetMessageA ( ADDR Msg, NULL, NULL, NULL )
	test	EAX, EAX
	jz	>
	_TranslateMessage ( ADDR Msg )
	_DispatchMessageA ( ADDR Msg )
	jmp	<

	; ---------------------------------------------------------------- ;
	; Cleanup and Exit
	; ---------------------------------------------------------------- ;

:	_UnregisterClassA ( ADDR szClassName, D[wc.hInstance] )
.exit:	_ExitProcess ( D[Msg.wParam] )
	ret

ENDF

; ------------------------------------------------------------------------ ;
  WndProc:
; ------------------------------------------------------------------------ ;
; Routine Description:
;
;     This routine checks the message handler table for the current
;     message being recieved from the operating system. If found, the
;     routine calls the associated procedure. If not found, it calls
;     the default windows handler.
;
; Arguments:
;
;     hWnd - Handle of the current window.
;     uMsg - Message identifier.
;     wParam - First message parameter (contents depends on uMsg).
;     lParam - Second message parameter (contents depends on uMsg).
;
; Return Value:
;
;     The return value is the result of the message processing and
;     depends on the message sent.
;
; ------------------------------------------------------------------------ ;

FRAME hWnd, uMsg, wParam, lParam

	mov	EAX, D[uMsg]
	mov	ECX, SIZEOF MsgTable / 8
	mov	EDX, ADDR MsgTable

	; ---------------------------------------------------------------- ;
	; Search Message Table
	; ---------------------------------------------------------------- ;

:	dec	ECX
	js	>
	cmp	D[EDX + ECX * 8 + 4], EAX
	jnz	<
	call	D[EDX + ECX * 8 ]
	jnc	>.exit

	; ---------------------------------------------------------------- ;
	; Call Default Message Handler
	; ---------------------------------------------------------------- ;

:	_DefWindowProcA ( D[hWnd], D[uMsg], D[wParam], D[lParam] )
.exit:	ret

ENDF

; ------------------------------------------------------------------------ ;
  OnCreate:
; ------------------------------------------------------------------------ ;
; Routine Description:
;
;     This routine is called from the message handler table and is
;     responsible for the creation and initialization of the window
;     elements and controls.
;
; Arguments:
;
;     Arguments are barrowed from WndProc.
;
;     wParam - Unused.
;     lParam - Pointer to structure containing window creation data.
;
; Return Value:
;
;     On return, if the carry flag is set, DefWindowProc will be
;     called. If the carry flag is not set, EAX should be set to Zero
;     to continue processing the window creation and negative one to
;     destroy the window and exit.
;
; ------------------------------------------------------------------------ ;

USEDATA WndProc

.rct:	stc ; return carry: TRUE
.rcf:	ret ; return carry: FALSE

ENDU

; ------------------------------------------------------------------------ ;
  OnCommand:
; ------------------------------------------------------------------------ ;
; Routine Description:
;
;     This routine is called from the message handler table and is
;     responsible for processing menu commands, control notifications,
;     and accelerator keystrokes.
;
; Arguments:
;
;     Arguments are barrowed from WndProc.
;
;     High word of wParam - Notification code.
;     Low word of wParam - item, control, or accelerator identifier.
;     lParam - Handle of control sending the notification, or NULL.
;
; Return Value:
;
;     On return, if the carry flag is set, DefWindowProc will be
;     called. If the carry flag is not set, then WndProc returns
;     with the value in EAX.
;
; ------------------------------------------------------------------------ ;

USEDATA WndProc

.rct:	stc ; return carry: TRUE
.rcf:	ret ; return carry: FALSE

ENDU

; ------------------------------------------------------------------------ ;
  OnClose:
; ------------------------------------------------------------------------ ;
; Routine Description:
;
;     This routine is called from the message handler table and is
;     responsible for shutting down and for signaling that the application
;     should be terminated.
;
; Arguments:
;
;     Arguments are barrowed from WndProc.
;
;     wParam - Unused.
;     lParam - Unused.
;
; Return Value:
;
;     On return, if the carry flag is set, DefWindowProc will be
;     called. If the carry flag is not set, then WndProc returns
;     with the value in EAX.
;
; ------------------------------------------------------------------------ ;

USEDATA WndProc

.rct:	stc ; return carry: TRUE
.rcf:	ret ; return carry: FALSE

ENDU

; ------------------------------------------------------------------------ ;
  OnDestroy:
; ------------------------------------------------------------------------ ;
; Routine Description:
;
;     This routine is called from the message handler table and is
;     responsible posting a notification of closure to the operating
;     system.
;
; Arguments:
;
;     Arguments are barrowed from WndProc.
;
;     wParam - Unused.
;     lParam - Unused.
;
; Return Value:
;
;     On return, if the carry flag is set, DefWindowProc will be
;     called. If the carry flag is not set, then WndProc returns
;     with the value in EAX.
;
; ------------------------------------------------------------------------ ;

USEDATA WndProc

	_PostQuitMessage ( 0 )
	ret

ENDU

;::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::;
;                Copyright (c) Bryant Keller, 2006                         ;
;                       All rights reserved.                               ;
;::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::;