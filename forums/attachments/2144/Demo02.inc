; ==================================================================================================
; Title:   Demo02.inc
; Author:  G. Friedrich  @ July 2003
; Version: 1.0.0
; Purpose: Demonstration program 2 of ObjAsm32.
; ==================================================================================================


CLSSTYLE equ CS_BYTEALIGNWINDOW or CS_BYTEALIGNCLIENT or CS_VREDRAW or CS_HREDRAW; or CS_DROPSHADOW

CStr szDemoApp02, "DemoApp02"                           ;Creates szDemoApp02 string in .const

MyStruc struc
  a   dword   ?
  b   dword   ?
MyStruc ends

Object DemoApp02, MySdiAppID, SdiApp                    ;Single Document Interface App.
  RedefineMethod    Init                                ;Init method refefinition
  StaticMethod      OnCommand,  dword, dword            ;WM_COMMAND message handler
  StaticMethod      OnClose,    dword, dword            ;WM_CLOSE and WM_QUERYENDSESSION handler
  StaticMethod      OnPaint,    dword, dword            ;WM_PAINT message handler
  BoundMethod       Startup

  Event     OnCommand,  WM_COMMAND                      ;Links OnCommand method with WM_COMMAND
  Event     OnClose,    WM_CLOSE, WM_QUERYENDSESSION    ;Links OnClose
  Event     OnPaint,    WM_PAINT                        ;Links OnPaint method with WM_PAINT

  Count = 0
  repeat 64
  DefineVariable    MyVar%Count,  MyStruc,  {5h,2h}
  Count = Count + 1
  endm  
ObjectEnd                                               ;Ends object definition


.code
; ==================================================================================================
;    DemoApp02 implementation
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp02.Init
; Purpose:   Initalizes the SDI application object.
; Arguments: None.
; Return:    Nothing.

Method DemoApp02.Init, uses esi
    local dWwd:dword, dWht:dword
    local dWtx:dword, dWty:dword

    SetObject esi
    
    DbgHex [esi].MyVar0.MyStruc.a
    DbgHex [esi].MyVar0.MyStruc.b
    DbgHex [esi].MyVar1.MyStruc.a
    DbgHex [esi].MyVar1.MyStruc.b
    DbgHex [esi].MyVar63.MyStruc.a
    DbgHex [esi].MyVar63.MyStruc.b
    

    mov dWwd, 500                                       ;Default creation width
    mov dWht, 350                                       ;Default creation height
    mov dWtx, $invoke(CenterForm, dWwd, $invoke(GetSystemMetrics, SM_CXSCREEN)) ;Center the form
    mov dWty, $invoke(CenterForm, dWht, $invoke(GetSystemMetrics, SM_CYSCREEN)) ;Center the form

    invoke CreateWindowEx, WS_EX_LEFT or WS_EX_APPWINDOW, \
           offset szDemoApp02, offset szAppTitle, WS_OVERLAPPEDWINDOW, \
           dWtx, dWty, dWwd, dWht, NULL, NULL, hInstance, esi ;Pass the instance pointer

    invoke ShowWindow, [esi].hWnd, SW_SHOWNORMAL        ;Show the window
    invoke UpdateWindow, [esi].hWnd                     ;Update if necessary

MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp02.OnCommand
; Purpose:   Event procedure for WM_COMMAND message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero if handled.

Method DemoApp02.OnCommand, uses esi, wParam:dword, lParam:dword
    local bBuffer[MAX_PATH]:byte, Dlg:DialogAbout

    SetObject esi
    Switch wParam
    Case IDM_EXIT
      invoke SendMessage, [esi].hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL
      xor eax, eax

    Case IDM_ABOUT
      LNew Dlg, DialogAbout, Init, esi, [esi].hWnd
      OCall Dlg::DialogAbout.Show
      OCall Dlg::DialogAbout.Done
      xor eax, eax

    Case IDM_HELP
      @invoke GetEnvironmentVariable, "OA32_PATH", addr bBuffer, sizeof bBuffer
      @invoke StrLCat, addr bBuffer, "\Help\ObjAsm32.chm", sizeof bBuffer
      invoke HtmlHelp, [esi].hWnd, addr bBuffer, HH_DISPLAY_TOPIC, 0
      xor eax, eax
    Default
      invoke DefWindowProc, [esi].hWnd, WM_COMMAND, wParam, lParam
    endsw
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp02.OnClose
; Purpose:   Event procedure for WM_CLOSE message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero if handled.

Method DemoApp02.OnClose, uses esi, wParam:dword, lParam:dword
    SetObject esi
    @invoke MessageBox, [esi].hWnd, "Are you sure ?", "Application exit", MB_YESNO or MB_ICONQUESTION
    .if eax == IDNO
      xor eax, eax
    .else
      invoke DefWindowProc, [esi].hWnd, WM_CLOSE, wParam, lParam
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp02.OnPaint
; Purpose:   Event procedure for WM_PAINT message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero if handled.

Method DemoApp02.OnPaint, uses esi, wParam:dword, lParam:dword
    local PS:PAINTSTRUCT, hDC:Handle, Rct:RECT

    SetObject esi
    mov hDC, $invoke(BeginPaint, [esi].hWnd, addr PS)

    invoke GetClientRect, [esi].hWnd, addr Rct
    invoke DrawEdge, hDC, addr Rct, EDGE_SUNKEN, BF_RECT

    invoke EndPaint, [esi].hWnd, addr PS
    mov eax, TRUE
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp02.Startup
; Purpose:   Registers the object class with the OS.
; Arguments: None.
; Return:    Nothing.

Method DemoApp02.Startup
    local WC:WNDCLASSEX

    mov WC.cbSize, sizeof WNDCLASSEX
    mov WC.style, CLSSTYLE
    m2m WC.lpfnWndProc, $MethodAddr(DemoApp02.WndProc)
    mov WC.cbClsExtra, 0
    mov WC.cbWndExtra, 0
    m2m WC.hInstance, hInstance
    @invoke LoadBitmap, hInstance, "BMP_BACKGROUND"
    mov WC.hbrBackground, $invoke(CreatePatternBrush, eax)
    mov WC.lpszMenuName, $OfsCStr("MENU_APP")
    mov WC.lpszClassName, offset szDemoApp02
    mov WC.hIcon, $invoke(LoadIcon, hInstance, "ICON_APP_32")
    mov WC.hCursor, $invoke(LoadCursor, NULL, IDC_ARROW)
    mov WC.hIconSm, $invoke(LoadIcon, hInstance, "ICON_APP_16")

    invoke RegisterClassEx, addr WC
MethodEnd
