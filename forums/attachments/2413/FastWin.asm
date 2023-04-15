; Assemble with "GoAsm <file.asm>"

; Link with "GoLink <file.asm> kernel32.dll user32.dll"
; Append "-debug coff" to GoLine command to link with symbols

data section

	szClsNm		db "FastererWin", 0x0
	szAppNm		db "Attempt at a ", 0x22, "Fasterer", 0x22, " Window Implementation", 0x0
	szText		db "This is my attempt at a ", 0x22, "fasterer", 0x22, " window ", \
			   "implementation. Rather than managing a ", \
			   "static stack frame manually through EBP, ", \
			   "I manage a dynamic stack frame through ESP.", \
			   0xA, 0xA, \
			   "To alleviate the headache that ensues from ", \
			   "trying to keep track of params and locals ", \
			   "on the stack, especially between intervening ", \
			   "PUSHes and POPs, I did away with them all ", \
			   "together. Instead, I SUBtract from ESP the ", \
			   "number of bytes I need for passing params. ", \
			   "This allows me to deal with a fixed number ", \
			   "of intervening bytes throughout the entire ", \
			   "process of putting params on the stack, as ", \
			   "opposed to the number changing with each ", \
			   "PUSH. An ideal side effect of using this ", \
			   "method is that now I can place params on the ", \
			   "stack in left-to-right order, at increasing ", \
			   "offsets from ESP, instead of right-to-left, ", \
			   "at decreasing offsets from EBP.", \
			   0xA, 0xA, \
			   "Well, any speed advantage will surely not be ", \
			   "human-perceptible, but at least I have the ", \
			   "satisfaction of know that this implementation ", \
			   "is ", 0x22, "fasterer", 0x22, " than my last one. Also, I have ", \
			   "discovered an opportunity for additional speed ", \
			   "gain. About 99.999% of windows messages, at ", \
			   "least in this program, are sent to DefWindowProc. ", \
			   "Instead of WndProc coping its params on the stack ", \
			   "before calling DefWindowProc, it just discards ", \
			   "its local storage and JuMPs to it instead. ", \
			   "This eliminates two copies of the same data ", \
			   "on the stack and one whole procedure RETurn."
			   0x0

	hInst		dd 0x0

code section

Start:

	; Get hInst

	sub esp, 0x4			; Prep 1 Arg
	mov d[esp], 0x0			; Calling App hInst
	call GetModuleHandleA		; Get hInst
	mov [hInst], eax		; Save hInst

	; Call FastererWin

	sub esp, 0x10			; Prep 4 Args
	mov [esp], eax			; hInst
	mov d[esp+0x4], 0x0		; No Prev Inst
	mov d[esp+0x8], 0x0		; No Cmd Ln
	mov d[esp+0xC], 0xA		; Show Def
	call WinMain			; FastererWin

	; Exit to Win

	ret 				; Ret Val in EAX

WinMain:

	; Reserve Auto Store

	sub esp, 0x4C			; 0x30+0x18+0x4

	; No Crit Regs on Stack

	; Auto Store on Stack

	; WndClsEx:	esp		; 0x0
	; cbSize	+0x0
	; style		+0x4
	; lpfnWndProc	+0x8
	; cbClsEx	+0xC
	; cbWndEx	+0x10
	; hInst		+0x14
	; hIcon		+0x18
	; hCurs		+0x1C
	; hbrBckGrnd	+0x20
	; lpszMnuNm	+0x24
	; lpszClsNm	+0x28
	; hIconSm	+0x2C

	; Msg:		esp+0x30	; 0x0+0x30
        ; hWnd		+0x0
	; msg		+0x4
	; wParam	+0x8
	; lParam	+0xC
	; time		+0x10
	; pt		+0x14

        ; hWnd:		esp+0x48	; 0x30+0x18

	; Ret Addr on Stack

	; Ret EIP:	esp+0x4C	; 0x48+0x4

	; Params on Stack

        ; hInst		esp+0x50	; 0x4C+0x4
        ; hPrevInst	esp+0x54	; 0x50+0x4
	; lpCmdLn	esp+0x58	; 0x54+0x4
	; nCmdShow	esp+0x5C	; 0x58+0x4

	; Do Not Preserve Crit Regs

	; Fill WinClsEx

	mov d[esp], 0x30		; WinClsEx Size
	mov d[esp+0x4], 0x1|0x2		; Horiz and Vert Redraw
	mov d[esp+0x8], addr WndProc	; FastererWin Wndproc
	mov d[esp+0xC], 0x0		; No Extra Cls Bytes
	mov d[esp+0x10], 0x0		; No Extra Wnd Bytes
	mov [esp+0x14], eax		; EAX Still hInst
	mov d[esp+0x20], 0x5		; Window Color
	mov d[esp+0x24], 0x0		; No Menu
	mov d[esp+0x28], addr szClsNm	; FastererWin Cls Nm

	; Get WinClsEx hIcon and hIconSm

	sub esp, 0x8			; Prep 2 Args
	mov d[esp], 0x0			; Sys hIcon
	mov d[esp+0x4], 0x7F00		; Std App hIcon
	call LoadIconA			; Get hIcon
	mov [esp+0x18], eax		; Save WinClsEx hIcon
	mov [esp+0x2c], eax		; Save WinClsEx hIconSm

	; Get WinClsEx hCurs

	sub esp, 0x8			; Prep 2 Args
	; WOW! --->			; Correct Args Still on Stack
	call LoadCursorA		; Get hCurs
	mov [esp+0x1C], eax		; Save WinClsEx hCurs

	; Register FastererWin WinCls

	lea eax, [esp]			; Load WinClsEx Addr
	sub esp, 0x4			; Prep 1 Arg
	mov [esp], eax			; WinClsEx Addr
	call RegisterClassExA		; Register FastererWin WinCls

	; Create FastererWin Win

	sub esp, 0x30			; Prep 12 Args
	mov d[esp], 0x0			; No Ex Style
	mov [esp+0x4], addr szClsNm	; FastererWin
	mov [esp+0x8], addr szAppNm
	mov d[esp+0xC], 0x0|0x40000|0x80000|0xC00000
					; Overlapped|ThickFrame|SysMnu|Caption
	mov d[esp+0x10], 0x80000000	; Def X Coord
	mov d[esp+0x14], 0x80000000	; Def Y Coord
	mov d[esp+0x18], 0x80000000	; Def Width
	mov d[esp+0x1C], 0x80000000	; Def Height
	mov d[esp+0x20], 0x0		; DeskTop Parent Win
	mov d[esp+0x24], 0x0		; No Mnu
	mov eax, [esp+0x50+0x30]	; Get hInst Param
	mov [esp+0x28], eax		; hInst Param
	mov d[esp+0x2C], 0x0		; No Ex Info
	call CreateWindowExA		; Create FastererWin Win
	mov [esp+0x48], eax		; Save FastererWin hWnd

	; Show FastererWin Win

	sub esp, 0x8			; Prep 2 Args
	mov [esp], eax			; FastererWin hWnd
	mov eax, [esp+0x5C+0x8]		; Get nCmdShow
	mov [esp+0x4], eax		; nCmdShow
	call ShowWindow			; Show FastererWin Win

	; Update FastererWin Win

	sub esp, 0x4			; Prep 1 Arg
	mov eax, [esp+0x48+0x4]		; Get FastererWin hWnd
	mov [esp], eax			; FastererWin hWnd
	call UpdateWindow		; Update FastererWin Win

	MsgLp:

		; Get Msg

		lea eax, [esp+0x30]		; Get Msg Addr
		sub esp, 0x10			; Prep 4 Args
		mov [esp], eax			; Msg Addr
		mov d[esp+0x4], 0x0		; No hWnd for All Msgs
		mov d[esp+0x8], 0x0		; No Min Msg Filter
		mov d[esp+0xC], 0x0		; No Max Msg Filter
		call GetMessageA		; Get Msg

		; Exit Msg Lp on Null Msg

		or eax, eax			; Check Null Msg
		jz >EndMsgLp			; Exit Msg Lp on Null Msg

		; Do Not Trans Msg

		; Dispatch Msg

		lea eax, [esp+0x30]		; Get Msg Addr
		sub esp, 0x4			; Prep 1 Arg
		mov [esp], eax			; Msg Addr
		call DispatchMessageA		; Dispatch Msg

	jmp <MsgLp			; Keep Pumping Msgs

	EndMsgLp:

	mov eax, [esp+0x30+0x8]		; Ret Msg wParam

	; Do Not Restore Crit Regs

	; Discard Auto Store

	add esp, 0x4C			; 0x30+0x18+0x4

	; Exit to Start

	ret 0x10			; Discard 4 Params

WndProc:

	; Reserve Auto Store

	sub esp, 0x48			; 0x34+0x10+0x4

	; No Crit Regs on Stack

	; Auto Store on Stack

	; PaintStruct:	esp		; 0x0
	; hdc		+0x0
	; fErase	+0x4
	; rcPaint	+0x8
	; fRestore	+0xC
	; fIncUpdate	+0x10
	; rgbReserved	+0x14

	; Rect:		esp+0x34	; 0x0+0x34
	; left		+0x0
	; top		+0x4
	; right		+0x8
	; bottom	+0xC

	; hdc:		esp+0x44	; 0x34+0x10

	; Ret Addr on Stack

	; Ret EIP:	esp+0x48	; 0x44+0x4

	; Params on Stack

        ; hWnd		esp+0x4C	; 0x48+0x4
        ; uMsg		esp+0x50	; 0x4C+0x4
	; wParam	esp+0x54	; 0x50+0x4
	; lParam	esp+0x58	; 0x54+0x4

	; Do Not Preserve Crit Regs

	ProcMsgs:

		; Check for Msgs of Interest

		mov eax, [esp+0x50]		; Get Msg Num

		; Something buggy with OnPaint...

		;cmp eax, 0xF			; Paint Msg?
		;jz >OnPaint			; Paint FastererWin Win

		cmp eax, 0x2			; Destroy Msg?
		jz >OnDestroy			; Destroy FastererWin Win
	
		; Discard Auto Store...

		add esp, 0x48			; 0x34+0x10+0x4

		; Fwd Msg to Windows

		jmp DefWindowProcA		; DefWindowProcA Will
						; Ret to WndProc's Caller

	OnPaint:

		; Paint FastererWin Win

		lea eax, [esp]			; Get PaintStruct Addr
		sub esp, 0x8			; Prep 2 Args
		mov [esp+0x4], eax		; PaintStruct Addr
		mov eax, [esp+0x4C+0x8]		; Get hWnd
		mov [esp], eax			; hWnd
		call BeginPaint			; Begin Painting
		mov [esp+0x48], eax		; Save hdc

		; Get FastererWin Win Rect

		lea eax, [esp+0x34]		; Get Rect Addr
		sub esp, 0x8			; Prep 2 Args
		mov [esp+0x4], eax		; Rect Addr
		mov eax, [esp+0x4C+0x8]		; Get hWnd
		mov [esp], eax			; hWnd
		call GetClientRect		; Get FastererWin Win Rect

		; Draw Text to FastererWin Win

		mov eax, [esp+0x48]		; Get hdc
		sub esp, 0x14			; Prep 5 Args
		mov [esp], eax			; hdc
		mov [esp+0x4], addr szText	; szText Addr
		mov d[esp+0x8], -1		; Assume szText Zero Terminated
		lea eax, [esp+0x34+0x14]	; Get Rect Addr
		mov [esp+0xC], eax		; Rect Addr
		mov d[esp+0x10], 0x10|0x40000	; Word Break and Ellipsis
		call DrawTextA			; Draw Text

		; End Paint FastererWin Win

		lea eax, [esp]			; Get PaintStruct Addr
		sub esp, 0x8			; Prep 2 Args
		mov [esp+0x4], eax		; PaintStruct Addr
		mov eax, [esp+0x4C+0x8]		; Get hWnd
		mov [esp], eax			; hWnd
		call EndPaint			; End Painting

		; Set Ret Val

		xor eax, eax			; ???
		
		; Exit ProcMsgs

		jmp >EndProcMsgs

	OnDestroy:

		; Destroy FastererWin Win

		sub esp, 0x4			; Prep 1 Arg
		mov d[esp], 0x0
		call PostQuitMessage		; Destroy FastererWin Win

		; Set Ret Val

		xor eax, eax			; ???

	EndProcMsgs:

	; Do Not Restore Crit Regs

	; Discard Auto Store

	add esp, 0x48			; 0x34+0x10+0x4

	; Exit to Dispatch/SendMessage

	ret 0x10			; Discard 4 Params