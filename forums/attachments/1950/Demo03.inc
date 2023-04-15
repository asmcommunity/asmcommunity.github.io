; ==================================================================================================
; Title:   Demo03.inc
; Author:  G. Friedrich  @ February 2003
; Version: 1.0.0
; Purpose: Demonstration program 3 of ObjAsm32.
; ==================================================================================================


APPSTYLE    equ     CS_BYTEALIGNWINDOW or CS_BYTEALIGNCLIENT or CS_VREDRAW or CS_HREDRAW

TBSTYLE     equ     WS_CHILD or WS_VISIBLE or WS_CLIPCHILDREN or TBSTYLE_TOOLTIPS or \
                    CCS_NORESIZE or CCS_NODIVIDER or CCS_ADJUSTABLE or TBSTYLE_FLAT

SBSTYLE     equ     WS_CHILD or WS_VISIBLE or SBARS_TOOLTIPS or SBARS_SIZEGRIP

.const
ToolbarFile TbDef {IDC_TOOLBARFILE, TBSTYLE, 0, $OfsTStr("File operations"), 0, 0, 0, 0, 5}
  TBBUTTON {IDB_FILE_NEW,    IDM_FILE_NEW,    TBSTATE_ENABLED, TBSTYLE_BUTTON}
  TBBUTTON {0,               0,               TBSTATE_ENABLED, TBSTYLE_SEP}
  TBBUTTON {IDB_FILE_OPEN,   IDM_FILE_OPEN,   TBSTATE_ENABLED, TBSTYLE_BUTTON}
  TBBUTTON {IDB_FILE_SAVE,   IDM_FILE_SAVE,   TBSTATE_ENABLED, TBSTYLE_BUTTON}
  TBBUTTON {IDB_FILE_SAVEAS, IDM_FILE_SAVEAS, TBSTATE_ENABLED, TBSTYLE_BUTTON}

ToolbarEdit TbDef {IDC_TOOLBAREDIT, TBSTYLE, 0, $OfsTStr("Edit operations"), 0, 0, 0, 0, 3}
  TBBUTTON {IDB_EDIT_CUT,    IDM_EDIT_CUT,    TBSTATE_ENABLED, TBSTYLE_BUTTON}
  TBBUTTON {IDB_EDIT_COPY,   IDM_EDIT_COPY,   TBSTATE_ENABLED, TBSTYLE_BUTTON}
  TBBUTTON {IDB_EDIT_PASTE,  IDM_EDIT_PASTE,  TBSTATE_ENABLED, TBSTYLE_BUTTON}
  TBBUTTON {IDB_FILE_OPEN,   IDM_FILE_OPEN,   0,               TBSTYLE_BUTTON}

AppStatusbar SbDef {IDC_STATUSBAR, SBSTYLE, 0, NULL, 0, 0, 0, 0, 4}
  SbPartDef {120,  50, 3, SBT_NOBORDERS, $OfsTStr("Help")}
  SbPartDef {120,  20, 1, SBT_NOBORDERS or SBT_OWNERDRAW, NULL}
  SbPartDef {120,  -2, 0, SBT_SUNKEN, NULL}         ;Spring
  SbPartDef {120, 100, 2, SBT_SUNKEN, NULL}

; ——————————————————————————————————————————————————————————————————————————————————————————————————

CStr szDemoApp03, "DemoApp03"

Object DemoApp03, MySdiAppID, SdiApp                ;Single Document Interface App.
  RedefineMethod    Done
  RedefineMethod    Init
  StaticMethod      OnClose,        dword, dword
  StaticMethod      OnCommand,      dword, dword
  StaticMethod      OnCreate,       dword, dword
  StaticMethod      OnHelp,         dword, dword
  StaticMethod      OnPaint,        dword, dword
  StaticMethod      OnNotify,       dword, dword
  StaticMethod      OnSize,         dword, dword
  RedefineMethod    Run
  BoundMethod       Startup

  Event     OnClose,    WM_CLOSE, WM_QUERYENDSESSION
  Event     OnCommand,  WM_COMMAND
  Event     OnCreate,   WM_CREATE
  Event     OnHelp,     WM_HELP
  Event     OnNotify,   WM_NOTIFY
  Event     OnPaint,    WM_PAINT
  Event     OnSize,     WM_SIZE

  DefineVariable    pIconImageList, Pointer,  NULL
  DefineVariable    pModelessDlg,   Pointer,  NULL
  DefineVariable    pRebar,         Pointer,  NULL
  DefineVariable    pStatusbar,     Pointer,  NULL
  DefineVariable    pToolbar1,      Pointer,  NULL
  DefineVariable    pToolbar2,      Pointer,  NULL
  DefineVariable    pTooltip,       Pointer,  NULL
  DefineVariable    hIcon1,         Handle,   0
  DefineVariable    hIcon2,         Handle,   0
ObjectEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————

Object ModelessDlg, , DialogModeless
  RedefineMethod    CtlsGet
  RedefineMethod    CtlsSet
  RedefineMethod    OnCommand,      dword, dword
ObjectEnd


.code
; ==================================================================================================
;    DemoApp03 implementation
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.Done
; Purpose:   Finalizes the SDI application object.
; Arguments: None.
; Return:    Nothing.

Method DemoApp03.Done, uses esi
    SetObject esi
    .if [esi].hIcon1 != 0
      invoke DestroyIcon, [esi].hIcon1
    .endif
    .if [esi].hIcon2 != 0
      invoke DestroyIcon, [esi].hIcon2
    .endif
    Destroy [esi].pRebar
    Destroy [esi].pStatusbar
    Destroy [esi].pToolbar1
    Destroy [esi].pToolbar2
    Destroy [esi].pTooltip
    Destroy [esi].pIconImageList
    ACall esi.Done
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.Init
; Purpose:   Initalizes the SDI application object.
; Arguments: None.
; Return:    Nothing.

Method DemoApp03.Init, uses ebx esi
    local dWwd:dword, dWht:dword
    local dWtx:dword, dWty:dword

    SetObject esi
    invoke LoadCommonControls, ICC_COOL_CLASSES or ICC_BAR_CLASSES    ;Rebar & Statusbar

    mov ebx, $New(MaskedImageList, Init, esi, 16, 16, 1)
    mov [esi].pIconImageList, ebx
    OCall ebx::MaskedImageList.IconLoadResource, "ICON_NEW"
    OCall ebx::MaskedImageList.IconLoadResource, "ICON_OPEN"
    OCall ebx::MaskedImageList.IconLoadResource, "ICON_SAVE"
    OCall ebx::MaskedImageList.IconLoadResource, "ICON_SAVEAS"
    OCall ebx::MaskedImageList.IconLoadResource, "ICON_CUT"
    OCall ebx::MaskedImageList.IconLoadResource, "ICON_COPY"
    OCall ebx::MaskedImageList.IconLoadResource, "ICON_PASTE"

    mov dWwd, 500
    mov dWht, 350
    mov dWtx, $invoke(CenterForm, dWwd, $invoke(GetSystemMetrics, SM_CXSCREEN))
    mov dWty, $invoke(CenterForm, dWht, $invoke(GetSystemMetrics, SM_CYSCREEN))

    invoke CreateWindowEx, WS_EX_LEFT or WS_EX_APPWINDOW, \
           offset szDemoApp03, offset szAppTitle, WS_OVERLAPPEDWINDOW, \
           dWtx, dWty, dWwd, dWht, NULL, NULL, hInstance, esi

    invoke ShowWindow, [esi].hWnd, SW_SHOWNORMAL
    invoke UpdateWindow, [esi].hWnd
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.OnClose
; Purpose:   Event procedure for WM_CLOSE message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero if handled.

Method DemoApp03.OnClose, uses esi, wParam:dword, lParam:dword
    SetObject esi
    @invoke MessageBox, [esi].hWnd, "Are you sure ?", "Application exit", MB_YESNO or MB_ICONQUESTION
    .if eax == IDNO
      xor eax, eax
    .else
      invoke DefWindowProc, [esi].hWnd, WM_CLOSE, wParam, lParam
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.OnCommand
; Purpose:   Event procedure for WM_COMMAND message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero if handled.

Method DemoApp03.OnCommand, uses esi, wParam:dword, lParam:dword
    local Dlg:DialogAbout

    SetObject esi
    Switch wParam
    Case IDM_FILE_NEW
      OCall [esi].pStatusbar::Statusbar.SetPartText, 0, "New File", SBT_NOBORDERS
      .if [esi].hIcon1 != 0
        invoke DestroyIcon, [esi].hIcon1
      .endif
      mov [esi].hIcon1, $OCall([esi].pIconImageList::MaskedImageList.IconCreate, 0, ILD_TRANSPARENT)
      OCall [esi].pStatusbar::Statusbar.SetPartIcon, 0, eax
      xor eax, eax
    Case IDM_FILE_OPEN
      OCall [esi].pStatusbar::Statusbar.SetPartText, 0, "Open File", SBT_SUNKEN
      .if [esi].hIcon2 != 0
        invoke DestroyIcon, [esi].hIcon2
      .endif
      mov [esi].hIcon2, $OCall([esi].pIconImageList::MaskedImageList.IconCreate, 1, ILD_TRANSPARENT)
      OCall [esi].pStatusbar::Statusbar.SetPartIcon, 0, eax
      xor eax, eax
    Case IDM_EXIT
      invoke SendMessage, [esi].hWnd, WM_SYSCOMMAND, SC_CLOSE, NULL
      xor eax, eax
    Case IDM_HELLO
      .if [esi].pModelessDlg == NULL
        mov [esi].pModelessDlg, $New(ModelessDlg, Init, esi, [esi].hWnd, "DIALOG1")
        OCall [esi].pModelessDlg::ModelessDlg.Show
        mov eax, [esi].pModelessDlg
        invoke GetDlgItem, [eax].ModelessDlg.hWnd, IDC_CHECK1
        invoke SetWindowContextHelpId, eax, IDC_CHECK1
      .else
        mov eax, [esi].pModelessDlg
        invoke SetFocus, [eax].Dialog.hWnd
      .endif
      xor eax, eax
    Case IDM_ABOUT
      LNew Dlg, DialogAbout, Init, esi, [esi].hWnd
      OCall Dlg::DialogAbout.Show
      OCall Dlg::DialogAbout.Done
      xor eax, eax
    Default
      invoke DefWindowProc, [esi].hWnd, WM_COMMAND, wParam, lParam
    endsw
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.OnCreate
; Purpose:   Event procedure for WM_CREATE message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero.

Method DemoApp03.OnCreate, uses ebx esi, wParam:dword, lParam:dword
    SetObject esi

    mov [esi].pStatusbar, $New(Statusbar, Init, esi, [esi].hWnd, offset AppStatusbar)
    mov [esi].pRebar, $New(Rebar, Init, esi, [esi].hWnd)

    mov [esi].pToolbar2, $New(Toolbar)
    OCall [esi].pToolbar2::Toolbar.Init, esi, [esi].hWnd, offset ToolbarEdit, \
               [esi].pIconImageList, NULL, NULL

    mov ebx, $OCall([esi].pToolbar2::Toolbar.GetHeight)
    OCall [esi].pToolbar2::Toolbar.GetWidth
    mov ecx, [esi].pToolbar2
    OCall [esi].pRebar::Rebar.InsertBand, [ecx].Toolbar.hWnd, eax, ebx

    mov [esi].pToolbar1, $New(Toolbar)
    OCall [esi].pToolbar1::Toolbar.Init, esi, [esi].hWnd, offset ToolbarFile, \
               [esi].pIconImageList, NULL, NULL
    mov ebx, $OCall([esi].pToolbar1::Toolbar.GetHeight)
    OCall [esi].pToolbar1::Toolbar.GetWidth
    mov ecx, [esi].pToolbar1
    OCall [esi].pRebar::Rebar.InsertBand, [ecx].Toolbar.hWnd, eax, ebx

    mov eax, [esi].pToolbar1
    mov ebx, [eax].Toolbar.hWnd
    invoke SendMessage, ebx, TB_GETTOOLTIPS, 0, 0
    mov [esi].pTooltip, eax
    invoke SendMessage, [esi].pTooltip, TTM_SETDELAYTIME, TTDT_INITIAL, 0
    invoke SendMessage, [esi].pTooltip, TTM_SETDELAYTIME, TTDT_RESHOW, 0

    mov [esi].pTooltip, $New(Tooltip, Init, esi, [esi].hWnd)
    mov ecx, [esi].pStatusbar
    mov ecx, [esi].pStatusbar
    OCall [esi].pTooltip::Tooltip.AddWndTip, [ecx].WinControl.hWnd, "Hallo"

    xor eax, eax
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.OnHelp
; Purpose:   Event procedure for WM_HELP message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero.

Method DemoApp03.OnHelp, uses esi, wParam:dword, lParam:dword
    SetObject esi
    mov eax, lParam
    DbgHex [eax].HELPINFO.iContextType
    DbgDec [eax].HELPINFO.iCtrlId
    DbgHex [eax].HELPINFO.hItemHandle
    DbgDec [eax].HELPINFO.dwContextId
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.OnNotify
; Purpose:   Event procedure for WM_NOTIFY message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero if handled.

Method DemoApp03.OnNotify, uses esi, wParam:dword, lParam:dword
    SetObject esi
    mov eax, wParam
    .if (eax == [esi].pModelessDlg)
      mov eax, lParam
      .if [eax].NMHDR.code == DLGDESTROYED
        Destroy [esi].pModelessDlg
        xor eax, eax
        ExitMethod
      .endif
    .endif
    invoke DefWindowProc, [esi].hWnd, WM_NOTIFY, wParam, lParam
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.OnPaint
; Purpose:   Event procedure for WM_PAINT message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero if handled.

Method DemoApp03.OnPaint, uses esi, wParam:dword, lParam:dword
    local PS:PAINTSTRUCT, hDC:Handle, Rct:RECT

    SetObject esi
    invoke BeginPaint, [esi].hWnd, addr PS
    mov hDC, eax

    mov ecx, [esi].pRebar
    invoke GetWindowRect, [ecx].Rebar.hWnd, addr Rct
    mov eax, Rct.bottom
    sub eax, Rct.top
    push eax
    mov ecx, [esi].pStatusbar
    invoke GetWindowRect, [ecx].Statusbar.hWnd, addr Rct
    mov eax, Rct.bottom
    sub eax, Rct.top
    push eax
    invoke GetClientRect, [esi].hWnd, addr Rct
    pop eax
    sub Rct.bottom, eax
    pop Rct.top
    invoke DrawEdge, hDC, addr Rct, EDGE_SUNKEN, BF_RECT

    invoke EndPaint, [esi].hWnd, addr PS
    mov eax, TRUE
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.OnSize
; Purpose:   Event procedure for WM_SIZE message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Zero if handled.

Method DemoApp03.OnSize, uses ebx esi, wParam:dword, lParam:dword
    local Rct:RECT

    .if wParam != SIZE_MINIMIZED
      SetObject esi
      invoke DefWindowProc, [esi].hWnd, WM_SIZE, wParam, lParam
      invoke GetClientRect, [esi].hWnd, addr Rct
      mov ebx, [esi].pStatusbar
      invoke MoveWindow, [ebx].Statusbar.hWnd, 0, 0, 0, 0, TRUE
      invoke UpdateWindow, [ebx].Statusbar.hWnd

      mov ecx, [esi].pRebar
      invoke MoveWindow, [ecx].Rebar.hWnd, 0, 0, Rct.right, Rct.bottom, TRUE
    .endif

    xor eax, eax                ;Return zero
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.Run
; Purpose:   Launches the SDI application object starting the message loop.
; Arguments: None.
; Return:    Exit code passed to PostQuitMessage.

Method DemoApp03.Run, uses esi
    local Msg:MSG

    SetObject esi
    .repeat
      invoke GetMessage, addr Msg, NULL, 0, 0
      .break .if eax == 0
      .if [esi].pModelessDlg != NULL
        mov ecx, [esi].pModelessDlg
        invoke IsDialogMessage, [ecx].Dialog.hWnd, addr Msg
        .continue .if eax != 0
      .endif

      invoke TranslateAccelerator, [esi].hWnd, [esi].hAccelerators, addr Msg
      .if !eax
        invoke TranslateMessage, addr Msg
        invoke DispatchMessage, addr Msg
      .endif
    .until FALSE
    mov eax, Msg.wParam
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    DemoApp03.Startup
; Purpose:   Registers the object class with the OS.
; Arguments: None.
; Return:    Nothing.

Method DemoApp03.Startup
    local WC:WNDCLASSEX

    mov WC.cbSize, sizeof WNDCLASSEX
    mov WC.style, APPSTYLE
    m2m WC.lpfnWndProc, $MethodAddr(DemoApp03.WndProc)
    mov WC.cbClsExtra, 0
    mov WC.cbWndExtra, 0
    m2m WC.hInstance, hInstance
    @invoke LoadBitmap, hInstance, "BMP_BACKGROUND"
    mov WC.hbrBackground, $invoke(CreatePatternBrush, eax)
    mov WC.lpszMenuName, $OfsCStr("MENU_APP")
    mov WC.lpszClassName, offset szDemoApp03
    mov WC.hIcon, $invoke(LoadIcon, hInstance, "ICON_APP_32")
    mov WC.hCursor, $invoke(LoadCursor, 0, IDC_ARROW)
    mov WC.hIconSm, $invoke(LoadIcon, hInstance, "ICON_APP_16")

    invoke RegisterClassEx, addr WC
MethodEnd



; ==================================================================================================
;    ModelessDlg implementation
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    ModelessDlg.CtlsGet
; Purpose:   Reads the controls in the dialog.
; Arguments: None.
; Return:    Nothing.

Method ModelessDlg.CtlsGet
    SetObject ecx
    invoke IsDlgButtonChecked, [ecx].hWnd, IDC_CHECK1
    .if eax == BST_UNCHECKED
      invoke Beep, 1000, 500
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    ModelessDlg.CtlsSet
; Purpose:   Sets the controls in the dialog.
; Arguments: None.
; Return:    Nothing.

Method ModelessDlg.CtlsSet, uses esi
    SetObject esi
    invoke CheckRadioButton, [esi].hWnd, IDC_RADIO1, IDC_RADIO3, IDC_RADIO2
    invoke CheckDlgButton, [esi].hWnd, IDC_CHECK1, BST_CHECKED
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    ModelessDlg.OnCommand
; Purpose:   Event procedure for WM_COMMAND message.
; Arguments: Arg1: First message parameter.
;            Arg2: Second message parameter.
; Return:    Nothing.

Method ModelessDlg.OnCommand, uses esi, wParam:dword, lParam:dword
    SetObject esi
    LoWord(wParam)
    .if ax == IDOK
      OCall esi.CtlsGet
      OCall esi.DestroyDialog, wParam
    .elseif ax == IDCANCEL
      OCall esi.DestroyDialog, wParam
    .elseif ax == IDC_CHECK1
      invoke IsDlgButtonChecked, [esi].hWnd, IDC_CHECK1
      .if eax == BST_CHECKED
        EnableDlgControl [esi].hWnd, IDC_RADIO1
        EnableDlgControl [esi].hWnd, IDC_RADIO2
        EnableDlgControl [esi].hWnd, IDC_RADIO3
      .else
        EnableDlgControl [esi].hWnd, IDC_RADIO1, FALSE
        EnableDlgControl [esi].hWnd, IDC_RADIO2, FALSE
        EnableDlgControl [esi].hWnd, IDC_RADIO3, FALSE
      .endif
    .endif
MethodEnd
