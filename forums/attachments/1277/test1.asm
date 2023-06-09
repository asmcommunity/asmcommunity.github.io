;Compiler Declaration Section

format PE GUI 4.0

entry !Start

macro align value { rb (value-1) - (rva $ + value-1) mod value }

_BufSize=1000

; GUI Equates

HandleOffs equ 0
IDOffs equ 4
OwnerOffs equ 8
ArrayOffs equ 12
LeftOffs equ 16
TopOffs equ 20
RightOffs equ 24
BottomOffs equ 28
BackColorOffs equ 32
ForeColorOffs equ 36
TypeOffs equ 40
StatusOffs equ 44
evActivateOffs equ 48
evCommandOffs equ 52
evMouseActivateOffs equ 56
evPaintOffs equ 60
HDCOffs equ 64
evMouseMoveOffs equ 68
evMouseWheelOffs equ 72
evScrollOffs equ 76
evCreateOffs equ 80
evDestroy equ 84
evKeyDownOffs equ 88
evKeyUpOffs equ 92
evDblClickOffs equ 96
cdDblClickOffs equ 100
evClickOffs equ 104
cdClickOffs equ 108
evChangeOffs equ 112
cdChangeOffs equ 116
evSelectOffs equ 120
cdSelectOffs equ 124
evNotify1Offs equ 128
cdNotify1Offs equ 132
evNotify2Offs equ 136
cdNotify2Offs equ 140
evNotify3Offs equ 144
cdNotify3Offs equ 148
evNotify4Offs equ 152
cdNotify4Offs equ 156
evNotify5Offs equ 160
cdNotify5Offs equ 164
evNotify6Offs equ 168
cdNotify6Offs equ 172
evNotify7Offs equ 176
cdNotify7Offs equ 180
evNotify8Offs equ 184
cdNotify8Offs equ 188
evNotify9Offs equ 192
cdNotify9Offs equ 196
evNotify10Offs equ 200
cdNotify10Offs equ 204
evNotify11Offs equ 208
cdNotify11Offs equ 212
evNotify12Offs equ 216
cdNotify12Offs equ 220
evNotify13Offs equ 224
cdNotify13Offs equ 228
evNotify14Offs equ 232
cdNotify14Offs equ 236
evNotify15Offs equ 240
cdNotify15Offs equ 244
evNotify16Offs equ 248
cdNotify16Offs equ 252
evNotify17Offs equ 256
cdNotify17Offs equ 260
evNotify18Offs equ 264
cdNotify18Offs equ 268
evNotify19Offs equ 272
cdNotify19Offs equ 276
evNotify20Offs equ 280
cdNotify20Offs equ 284
evNotify21Offs equ 288
cdNotify21Offs equ 292
evNotify22Offs equ 296
cdNotify22Offs equ 300
evNotify23Offs equ 304
cdNotify23Offs equ 308
evNotify24Offs equ 312
cdNotify24Offs equ 316
evNotify25Offs equ 320
cdNotify25Offs equ 324
evNotify26Offs equ 328
cdNotify26Offs equ 332
evNotify27Offs equ 336
cdNotify27Offs equ 340
evNotify28Offs equ 344
cdNotify28Offs equ 348
evNotify29Offs equ 352
cdNotify29Offs equ 356
evNotify30Offs equ 360
cdNotify30Offs equ 364
evNotify31Offs equ 368
cdNotify31Offs equ 372
evNotify32Offs equ 376
cdNotify32Offs equ 380
evSysCommandOffs equ 384
evResizeOffs equ 388
evMoveOffs equ 392
evLButtonDownOffs equ 396
evLButtonUpOffs equ 400
evRButtonDownOffs equ 404
evRButtonUpOffs equ 408
ToolTipHandleOffs equ 412
evContextMenuOffs equ 416
evCommEventOffs equ 420
evReserved3Offs equ 424
evReserved4Offs equ 428
evReserved5Offs equ 432
evReserved6Offs equ 436
evReserved7Offs equ 440
Reserved1Offs equ 444
Reserved2Offs equ 448
Reserved3Offs equ 452
Reserved4Offs equ 456
Reserved5Offs equ 460
Reserved6Offs equ 464
Reserved7Offs equ 468
Reserved8Offs equ 472
Reserved9Offs equ 476; Sequential update number
WidthOffs equ 480
HeightOffs equ 484
ControlBrushOffs equ 488
InterceptSafeOffs equ 492
InterceptProcOffs equ 496
Style1Offs equ 500
Extra2Offs equ 504
DoNotUseOffs equ 508
WinConstructed equ 0
WinEnabled equ 1
WinVisible equ 2
WinChangeBackColor equ 3
WinChangeForeColor equ 4
WinArray equ 5
WinSubClassed equ 6
WinLoaded equ 7
WinPlaced equ 8
; End of Declaration Section

; Library Section

include 'include\kernel.inc'
include 'include\user.inc'
include 'include\gdi.inc'
include 'include\comctl.inc'
include 'include\comdlg.inc'
include 'include\shell.inc'
include 'include\cproc.inc'
include 'include\riched.inc'
include 'include\raedit.inc'
include 'include\ragrid.inc'
include 'include\macro\import.inc'
include 'include\macro\stdcall.inc'
include 'include\macro\resource.inc'

section '.idata' import data readable writeable

library kernel32,"kernel32.dll",\
        user32,"User32.dll",\
        gdi,"GDI32.DLL",\
        comctl,"COMCTL32.DLL",\
        msvcrt,"msvcrt.dll"

kernel32:
	import	ExitProcess,"ExitProcess",\
		GetCommandLine,"GetCommandLineA",\
		GetModuleHandle,"GetModuleHandleA",\
		GetStdHandle,"GetStdHandle",\
		LoadLibrary,"LoadLibraryA"
user32:
	import	GetDC,"GetDC",\
		ReleaseDC,"ReleaseDC",\
		CreateWindowEx,"CreateWindowExA",\
		DefWindowProc,"DefWindowProcA",\
		DispatchMessage,"DispatchMessageA",\
		GetMessage,"GetMessageA",\
		LoadCursor,"LoadCursorA",\
		LoadIcon,"LoadIconA",\
		PostMessage,"PostMessageA",\
		PostQuitMessage,"PostQuitMessage",\
		RegisterClass,"RegisterClassA",\
		GetWindowLong,"GetWindowLongA",\
		SetWindowLong,"SetWindowLongA",\
		TranslateMessage,"TranslateMessage"
gdi:
	import	CreateDIBSection,"CreateDIBSection",\
		CreateSolidBrush,"CreateSolidBrush",\
		DeleteObject,"DeleteObject",\
		GetPixel,"GetPixel",\
		GetBkColor,"GetBkColor",\
		SetBkColor,"SetBkColor",\
		SetBkMode,"SetBkMode",\
		GetStockObject,"GetStockObject",\
		SetTextColor,"SetTextColor"
comctl:
	import	InitCommonControlsEx,"InitCommonControlsEx"
msvcrt:
	import	puts,"puts"

; End of Library Section

section '.data' data readable writeable

align 4

ERR dd 0
!ArrayBase dd 1
_ErrVec dd 0
STATUS dd 0
POS dd 0
Ticks dd 0
XferCount dd 0
ArrNdx dd 0
EventNotify dw 0
EventID dw 0
_TrueStr db "TRUE",0
_FalseStr db "FALSE",0
!ControlType1 db 'Havis',0
!ControlType2 db 'BUTTON',0
!ControlType3 db 'STATIC',0
!ControlType4 db 'COMBOBOX',0
!ControlType5 db 'EDIT',0
!ControlType6 db 'BUTTON',0
!ControlType7 db 'LISTBOX',0
!ControlType8 db 'msctls_trackbar32',0
!ControlType9 db 'msctls_updown32',0
!ControlType10 db 'msctls_progress32',0
!ControlType11 db 'SysMonthCal32',0
!ControlType12 db 'SysHeader32',0
!ControlType13 db 'SysTreeView32',0
!ControlType14 db 'SysTabControl32',0
!ControlType15 db 'SysAnimate32',0
!ControlType16 db 'ToolbarWindow32',0
!ControlType17 db 'msctls_statusbar32',0
!ControlType18 db 'BUTTON',0
!ControlType19 db 'BUTTON',0
!ControlType20 db 'TIMER',0
!ControlType21 db 'RICHEDIT20A',0
!ControlType22 db 'RAEdit',0
!ControlType23 db 'RAGrid',0
!ControlType24 db 'Havis',0
!ControlType25 db 'SysListView32',0
!ControlType26 db 'STATIC',0
!ControlType27 db 'BUTTON',0
!ControlType28 db 'STATIC',0
!ControlType29 db 'OBCOMM',0
!ControlType30 db 'PROGRESS3D',0
!ControlType31 db 'SCROLLBAR',0
!ControlType32 db 'SysDateTimePick32',0

!OBMainWindowClass WNDCLASS
msg MSG
!TrapColorChange dd 0
!OBMainClass db 'OBMain',0
!icc dd $00000008,$00003FFF
!!RichEd db 'RichEd20.dll',0
!!RAEdit db 'RAEdit.dll',0
!!RAGrid db 'RAGrid.dll',0
!!Csmdll db 'Csmdll.dll',0
!title db 'OmniBasic',0

align 4
MenuEVPtr dd 0
MenuID dd 0
FindEVPtr dd 0
FindMsgID dd 0
FINDMSGSTRING db 'commdlg_FindReplace',0
FLAGS dd 0
nmhdr_hwndFrom dd 0
nmhdr_idFrom dd 0
nmhdr_code dd 0

cr_cpMin dd 0
cr_cpMax dd 0
ofn_lStructSize dd 76
ofn_hwndOwner dd 0
ofn_hInstance dd 0
ofn_lpstrFilter dd 0
ofn_lpstrCustomFilter dd 0
ofn_nMaxCustFilter dd 0
ofn_nFilterIndex dd 1
ofn_lpstrFile dd 0
ofn_nMaxFile dd 0
ofn_lpstrFileTitle dd 0
ofn_nMaxFileTitle dd 0
ofn_lpstrInitialDir dd 0
ofn_lpstrTitle dd 0
ofn_Flags dd 0
ofn_nFileOffset dw 0
ofn_nFileExtension dw 0
ofn_lpstrDefExt dd 0
ofn_lCustData dd 0
ofn_lpfnHook dd 0
ofn_lpTemplateName dd 0

cc_lStructSize dd 36
cc_hwndOwner dd 0
cc_hInstance dd 0
cc_rgbResult dd 0
cc_lpCustColors dd _CustColors
cc_Flags dd 0
cc_lCustData dd 0
cc_lpfnHook dd 0
cc_lpTemplateName dd 0

_CustColors rd 16

fr_lStructSize dd 40
fr_hwndOwner dd 0
fr_hInstance dd 0
fr_Flags dd 0
fr_lpstrFindWhat dd 0
fr_lpstrReplaceWith dd _PrintBuf+12
fr_wFindWhatLen dw 0
fr_wReplaceWithLen dw 0
fr_lCustData dd 0
fr_lpfnHook dd 0
fr_lpTemplateName dd 0
ft_SearchMin rd 1
ft_SearchMax rd 1
ft_SearchTextPtr rd 1
ft_FoundMin rd 1
ft_FoundMax rd 1

cf_lStructSize dd 58
cf_hwndOwner dd 0
cf_hDC dd 0
cf_lpLogFont dd 0
cf_iPointSize dd 0
cf_Flags dd 0
cf_rgbColors dd 0
cf_lCustData dd 0
cf_lpfnHook dd 0
cf_lpTemplateName dd 0
cf_hInstance dd 0
cf_lpszStyle dd 0
cf_nFontType dw 0
cf_nSizeMin dd 0
cf_nSizeMax dd 0

pr_lStructSize dd 66
pr_hwndOwner dd 0
pr_hDevMode dd 0
pr_hDevNames dd 0
pr_hDC dd 0
pr_Flags dd 0
pr_nFromPage dw 0
pr_nToPage dw 0
pr_nMinPage dw 0
pr_nMaxPage dw 0
pr_nCopies dw 0
pr_hInstance dd 0
pr_lCustData dd 0
pr_lpfnPrintHook dd 0
pr_lpfnSetupHook dd 0
pr_lpPrintTemplateName dd 0
pr_lpSetupTemplateName dd 0
pr_hPrintTemplate dd 0
pr_hSetupTemplate dd 0

!RA_BCKCOLOR dd $C0F0F0
!RA_TXTCOLOR dd $000000
!RA_SELBCKCOLOR dd $800000
!RA_SELTXTCOLOR dd $ffffff
!RA_CMNTCOLOR dd $008000
!RA_STRCOLOR dd $ff0000
!RA_OPRCOLOR dd $0000a0
!RA_HILITE1 dd $F0C0C0
!RA_HILITE2 dd $C0F0C0
!RA_HILITE3 dd $C0C0F0
!RA_SELBARCOLOR dd $c0c0c0
!RA_SELBARPEN dd $808080
!RA_LNRCOLOR dd $800000

!cbSize dd 52
!rcItem dd 0,0,0,0
!rcButton dd 0,0,0,0
!stateButton dd 0
!hwndCombo dd 0
!hwndItem dd 0
!hwndList dd 0

_ErrExitMsg db 'Error exit',0
align 4
_IOBuffer dd _IOBuffer+12
 dd 1000
 dd 0
 rb 1000
align 4
_PrintBuf dd _PrintBuf+12
 dd 1000
 dd 0
 rb 1000
align 4
_NullStr dd _NullStr+12
 dd 1
 dd 0
 rb 1
align 4
_CRLF dd _CRLF+12
 dd 2
 dd 0
 rb 2
align 4
_Prompt dd _Prompt+12
 dd 1
 dd 0
 rb 1
align 4
_ConvBuf dd _ConvBuf+12
 dd 32
 dd 0
 rb 32
align 4
_ConvBuf1 dd _ConvBuf1+12
 dd 32
 dd 0
 rb 32
align 4
ProgramName dd ProgramName+12
 dd 28
 dd 0
 rb 28
align 4
_SwitchStr dd _SwitchStr+12
 dd 1000
 dd 0
 rb 1000
; LN:1 ; Dimension Variables
!bm1:
 db $42,$4d,$f6,$00,$00,$00,$00,$00,$00,$00,$76,$00,$00,$00,$28,$00,$00,$00,$10,$00,$00,$00,$10,$00,$00,$00,$01,$00,$04,$00,$00,$00
 db $00,$00,$80,$00,$00,$00,$12,$0b,$00,$00,$12,$0b,$00,$00,$10,$00,$00,$00,$00,$00,$00,$00,$ff,$ff,$ff,$00,$c0,$c0,$c0,$00,$00,$00
 db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
 db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$11,$11,$11,$11,$11,$11,$11,$11,$11,$11
 db $11,$11,$11,$11,$11,$11,$11,$22,$22,$22,$22,$22,$21,$11,$11,$20,$00,$00,$00,$00,$21,$11,$11,$20,$00,$00,$00,$00,$21,$11,$11,$20
 db $00,$00,$00,$00,$21,$11,$11,$20,$00,$00,$00,$00,$21,$11,$11,$20,$00,$00,$00,$00,$21,$11,$11,$20,$00,$00,$00,$00,$21,$11,$11,$20
 db $00,$00,$00,$00,$21,$11,$11,$20,$00,$00,$00,$00,$21,$11,$11,$20,$00,$00,$02,$22,$21,$11,$11,$20,$00,$00,$02,$02,$11,$11,$11,$20
 db $00,$00,$02,$21,$11,$11,$11,$22,$22,$22,$22,$11,$11,$11,$11,$11,$11,$11,$11,$11,$11,$11
!bm2:
 db $42,$4d,$f6,$00,$00,$00,$00,$00,$00,$00,$76,$00,$00,$00,$28,$00,$00,$00,$10,$00,$00,$00,$10,$00,$00,$00,$01,$00,$04,$00,$00,$00
 db $00,$00,$80,$00,$00,$00,$12,$0b,$00,$00,$12,$0b,$00,$00,$10,$00,$00,$00,$00,$00,$00,$00,$00,$ff,$ff,$00,$00,$80,$80,$00,$c0,$c0
 db $c0,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
 db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$22,$22,$22,$22,$22,$22,$22,$22,$22,$22
 db $22,$22,$22,$22,$22,$22,$23,$33,$33,$33,$33,$33,$22,$22,$23,$31,$11,$11,$11,$11,$32,$22,$23,$23,$11,$11,$11,$11,$13,$22,$23,$02
 db $31,$11,$11,$11,$11,$32,$23,$20,$23,$11,$11,$11,$11,$13,$23,$02,$02,$33,$33,$33,$33,$33,$23,$20,$20,$20,$20,$23,$22,$22,$23,$02
 db $02,$02,$02,$03,$22,$22,$23,$20,$23,$33,$33,$33,$22,$22,$22,$33,$32,$22,$22,$22,$23,$33,$22,$22,$22,$22,$22,$22,$22,$33,$22,$22
 db $22,$22,$23,$22,$23,$23,$22,$22,$22,$22,$22,$33,$32,$22,$22,$22,$22,$22,$22,$22,$22,$22
!bm3:
 db $42,$4d,$f6,$00,$00,$00,$00,$00,$00,$00,$76,$00,$00,$00,$28,$00,$00,$00,$10,$00,$00,$00,$10,$00,$00,$00,$01,$00,$04,$00,$00,$00
 db $00,$00,$80,$00,$00,$00,$12,$0b,$00,$00,$12,$0b,$00,$00,$10,$00,$00,$00,$00,$00,$00,$00,$ff,$ff,$00,$00,$ff,$ff,$ff,$00,$c0,$c0
 db $c0,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00
 db $00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$00,$22,$22,$22,$22,$22,$22,$22,$22,$22,$33
 db $33,$33,$33,$33,$33,$32,$23,$00,$33,$33,$33,$22,$30,$32,$23,$00,$33,$33,$33,$22,$30,$32,$23,$00,$33,$33,$33,$22,$30,$32,$23,$00
 db $33,$33,$33,$33,$30,$32,$23,$00,$00,$00,$00,$00,$00,$32,$23,$00,$33,$33,$33,$33,$00,$32,$23,$03,$11,$11,$11,$11,$30,$32,$23,$03
 db $11,$11,$11,$11,$30,$32,$23,$03,$11,$11,$11,$11,$30,$32,$23,$03,$11,$11,$11,$11,$30,$32,$23,$03,$11,$11,$11,$11,$33,$32,$23,$03
 db $11,$11,$11,$11,$31,$32,$23,$33,$33,$33,$33,$33,$33,$32,$22,$22,$22,$22,$22,$22,$22,$22
!ObjectCount dd 1
; End of Initialized Data Section

; Start of Uninitialized Data Section

!CT_FORM equ 100
!CT_BUTTON equ 101
!CT_STATICICON equ 102
!CT_COMBOBOX equ 103
!CT_TEXTBOX equ 104
!CT_FRAME equ 105
!CT_LISTBOX equ 106
!CT_SLIDER equ 107
!CT_SPINNER equ 108
!CT_PROGRESSBAR equ 109
!CT_CALENDAR equ 110
!CT_HEADER equ 111
!CT_TREEVIEW equ 112
!CT_TABFOLDER equ 113
!CT_ANIMATE equ 114
!CT_TOOLBAR equ 115
!CT_STATUSBAR equ 116
!CT_CHECKBOX equ 117
!CT_RADIOBUTTON equ 118
!CT_TIMER equ 119
!CT_RICHEDIT equ 120
!CT_RAEdit equ 121
!CT_RAGrid equ 122
!CT_TOOLWINDOW equ 123
!CT_LISTVIEW equ 124
!CT_STATICTEXT equ 125
!CT_ICONBUTTON equ 126
!CT_RECTANGLE equ 127
!CT_OBCOMM equ 128
!CT_PROGRESS3D equ 129
!CT_SCROLLBAR equ 130
!CT_DATETIMEPICK equ 131

align 4

; Timer Handles
!Timer rd 1
Timer1 rd 1
Timer2 rd 1
Timer3 rd 1
Timer4 rd 1
Timer5 rd 1
Timer6 rd 1
Timer7 rd 1
Timer8 rd 1
Timer9 rd 1
Timer10 rd 1
Timer11 rd 1
Timer12 rd 1
Timer13 rd 1
Timer14 rd 1
Timer15 rd 1
Timer16 rd 1
Timer17 rd 1
Timer18 rd 1
Timer19 rd 1
Timer20 rd 1
Timer21 rd 1
Timer22 rd 1
Timer23 rd 1
Timer24 rd 1
Timer25 rd 1
Timer26 rd 1
Timer27 rd 1
Timer28 rd 1
Timer29 rd 1
Timer30 rd 1
Timer31 rd 1
Timer32 rd 1

_FltArgSafe0 rf 1
_FltArgSafe1 rf 1
_FloatRet rf 1
_LoopCtr rd 10
!hinstance rd 1
!dc rd 1
_InHandle rd 1
_OutHandle rd 1
_IOPthNum rd 1
_XferAddr rd 1
_PUArgs rd 15
_ArgList rd 1
_ArgNum rd 1
_Poker rd 1
_DummyArg rd 1
_FcnArg1 rd 1
_FcnArg2 rd 1
_FcnArg3 rd 1
_FcnArg4 rd 1
_FcnArg5 rd 1
_FcnArg6 rd 1
_FcnArg7 rd 1
_LongRet rd 1
_StringRet rd 3
_ArgSafe0 rd 1
_ArgSafe1 rd 1
_ArgSafe2 rd 1
_ArgSafe3 rd 1
!FileHandle rd 1
!FilBufAdr rd 1
!FilBufSiz rd 1
_SwitchInt rd 1
__ByteCounter rd 1
_PrtPthNum rd 1
_DataPtr rd 1
_Decimals rw 1
_Digits rw 1
_BoolRet rb 1
!PassSystemEvent rb 1
!PassInterceptEvent rb 1
!PassColorChangeEvent rb 1

; COMM DCB Structure
!DCBlengthOffs equ 0; rd 1
!BaudRate equ 4; rd 1
!fBinary equ 8; rd 1
!fParity equ 12; rd 1
!fOutxCtsFlow equ 16; rd 1
!fOutxDsrFlow equ 20; rd 1
!fDtrControl equ 24; rd 1
!fDsrSensitivity equ 28; rd 1
!fTXContinueOnXoff equ 32; rd 1
!fOutX equ 36; rd 1
!fInX equ 40; rd 1
!fErrorChar equ 44; rd 1
!fNull equ 48; rd 1
!fRtsControl equ 52; rd 1
!fAbortOnError equ 56; rd 1
!fDummy2 equ 60; rd 1
!wReserved equ 64; rw 1
!XonLim equ 66; rw 1
!XoffLim equ 68; rw 1
!ByteSize equ 70; rb 1
!Parity equ 71;  rb 1
!StopBits equ 72; rb 1
!XonChar equ 73; rb 1
!XoffChar equ 74; rb 1
!ErrorChar equ 75; rb 1
!EofChar equ 76; rb 1
!EvtChar equ 77; rb 1
!wReserved1 equ 78; rw 1

align 4
!CommBase:
!Comm1 rb 80
!Comm2 rb 80
!Comm3 rb 80
!Comm4 rb 80
!Comm5 rb 80
!Comm6 rb 80
!Comm7 rb 80
!Comm8 rb 80
!Comm9 rb 80
!Comm10 rb 80
!Comm11 rb 80
!Comm12 rb 80
!Comm13 rb 80
!Comm14 rb 80
!Comm15 rb 80
!Comm16 rb 80
!Comm17 rb 80
!Comm18 rb 80
!Comm19 rb 80
!Comm20 rb 80
!Comm21 rb 80
!Comm22 rb 80
!Comm23 rb 80
!Comm24 rb 80
!Comm25 rb 80
!Comm26 rb 80
!Comm27 rb 80
!Comm28 rb 80
!Comm29 rb 80
!Comm30 rb 80
!Comm31 rb 80
!Comm32 rb 80

align 4

;_SYSTEMTIME Structure
wYear rw 1 
wMonth rw 1 
wDayOfWeek rw 1 
wDay rw 1 
wHour rw 1 
wMinute rw 1 
wSecond rw 1 
wMilliseconds rw 1

;_FILETIME Structure
!dwLowDateTime rd 1
!dwHighDateTime rd 1


; RAGrid Column Structure

gc_colwt rd 1
gc_lpszhdrtext rd 1
gc_halign rd 1
gc_calign rd 1
gc_ctype rd 1
gc_ctextmax rd 1
gc_lpszformat rd 1
gc_himl rd 1
gc_hdrflag rd 1
gc_colxp rd 1
gc_edthwnd rd 1

CursorPosX rd 1
CursorPosY rd 1

Rect: ;Structure
RectLeft rd 1
RectTop rd 1
RectRight rd 1
RectBottom rd 1

; Paint Structure
PaintStruct rd 16

!TargetGUIDesc rd 1
!SourceGUIDesc rd 1

; RAGrid 
GridRow rd 1
GridCol rd 1

;TC_ITEM structure
!TC_ITEMmask rd 1
!TC_ITEMState rd 1
!TC_ITEMStateMask rd 1
!TC_ITEMText rd 1
!TC_ITEMTextMax rd 1
!TC_ITEMImage rd 1
!TC_ITEMlParam rd 1

; RAEdit Vars
EditcpMin rd 1
EditcpMax rd 1
EditSelType rw 1
EditLine rd 1
EditcpLine rd 1
EditlpLine rd 1
EditnLines rd 1
EditnHidden rd 1
EditfChanged rd 1
EditnPage rd 1
EditnWordGroup rd 1
OldFrameProc rd 1

!from equ 0; rd 1
!to equ 4; rd 1

; _TBBUTTON Structure
!iBitmap equ 0; rd 1
!idCommand equ 4; rd 1
!fsState equ 8; rb 1
!fsStyle equ 9; rb 1
!dwData equ 10; rd 1
!iString equ 14; rd 1

!Brush rd 1
!Desc rd 1
NMHDR rd 1
!dsStatus rd 1
ArrayIndex rd 1
!dsBackColor rd 1
!dsForeColor rd 1
!dsControlType rd 1

align 4
; LN:2 dim bm1 as bitmap
bm1 rd 1
align 4
; LN:3 dim bm2 as bitmap
bm2 rd 1
align 4
; LN:4 dim bm3 as bitmap
bm3 rd 1
; End of Uninitialized Data Section

; Start of Control Descriptor Section
!OBMain rd 128
section '.code' code readable executable
; Main Code Section
align 4
!Start:
call _Init
invoke GetModuleHandle, NULL
mov dword [!hinstance], eax
invoke GetCommandLine
mov dword [_ArgList],eax
mov dword edi,ProgramName
call __MovArg
invoke GetStdHandle, STD_INPUT_HANDLE
mov dword [_InHandle], eax
invoke GetStdHandle, STD_OUTPUT_HANDLE
mov dword [_OutHandle], eax
mov byte [_CRLF+12],13
mov byte [_CRLF+13],10
mov byte [_CRLF+14],0
mov byte [_Prompt+12],63
mov byte [_Prompt+13],0
mov byte [_NullStr+12],0
mov dword [_ErrVec],_ErrExit
invoke InitCommonControlsEx,!icc
invoke LoadIcon,0,IDI_APPLICATION
mov [!OBMainWindowClass.hIcon],eax
invoke LoadCursor,0,IDC_ARROW
mov [!OBMainWindowClass.hCursor],eax
mov [!OBMainWindowClass.style],0
mov [!OBMainWindowClass.lpfnWndProc],!WindowProc
mov [!OBMainWindowClass.cbClsExtra],0
mov [!OBMainWindowClass.cbWndExtra],0
mov eax,[!hinstance]
mov [!OBMainWindowClass.hInstance],eax
mov [!OBMainWindowClass.hbrBackground],COLOR_BTNFACE+1
mov [!OBMainWindowClass.lpszMenuName],0
mov [!OBMainWindowClass.lpszClassName],!OBMainClass
invoke RegisterClass,!OBMainWindowClass
mov dword [!OBMain+StatusOffs],0
mov dword [!OBMain+IDOffs],1
mov dword [!OBMain+ArrayOffs],0
mov dword [!OBMain+LeftOffs],0
mov dword [!OBMain+TopOffs],0
mov dword [!OBMain+WidthOffs],400
mov dword [!OBMain+HeightOffs],300
mov dword [!OBMain+ForeColorOffs],$ffffff
mov dword [!OBMain+BackColorOffs],$ffffffff
mov dword [!OBMain+TypeOffs],!CT_FORM
mov dword [!OBMain+ControlBrushOffs],0
invoke CreateWindowEx,0,!OBMainClass,!title,WS_VISIBLE+WS_OVERLAPPEDWINDOW,0,0,400,300,NULL,NULL,[!hinstance],!OBMain
invoke SetWindowLong,[!OBMain],GWL_USERDATA,!OBMain

!MsgLoop:
invoke GetMessage,msg,NULL,0,0
or eax,eax
jz !EndMsgLoop
invoke TranslateMessage,msg
invoke DispatchMessage,msg
jmp !MsgLoop
!EndMsgLoop:
invoke ExitProcess,[msg.wParam]

proc !WindowProc,!hwnd,wmsg,wparam,lparam
enter
push ebx esi edi
cmp [wmsg],WM_GETMINMAXINFO
jne NotwmGetMaxInfo
mov dword edi,[!hwnd]
mov dword [!OBMain+HandleOffs],edi
mov dword [!OBMain+OwnerOffs],0
mov dword [!OBMain+InterceptSafeOffs],!WindowProc
jmp !DefWndProc
NotwmGetMaxInfo:
mov esi,!OBMain
cmp [wmsg],WM_DESTROY
je !wmDestroy
!WinProcCommon:
cmp [wmsg],WM_SYSCOMMAND
je !wmSysCommand
cmp [wmsg],WM_SIZE
je !wmSize
cmp [wmsg],WM_MOVE
je !wmMove
cmp [wmsg],WM_TIMER
je !wmTimer
cmp [wmsg],WM_CREATE
je !wmCreate
cmp [wmsg],WM_PAINT
je !wmPaint
cmp [wmsg],WM_CONTEXTMENU
je !wmContextMenu
mov eax,[wparam]
mov [EventID],ax
shr eax,16
mov [EventNotify],ax
invoke GetWindowLong,[lparam],GWL_USERDATA
mov [!Desc],eax
cmp [wmsg],WM_CTLCOLORSTATIC
je !CtlColorChange
cmp [wmsg],WM_CTLCOLOREDIT
je !CtlColorChange
cmp [wmsg],WM_CTLCOLORBTN
je !CtlColorChange
cmp [wmsg],WM_CTLCOLORSCROLLBAR
je !CtlColorChange
cmp [wmsg],WM_CTLCOLORLISTBOX
je !CtlColorChange
cmp [wmsg],WM_CTLCOLORDLG
je !CtlColorChange
cmp [wmsg],WM_CTLCOLORMSGBOX
je !CtlColorChange
cmp [wmsg],WM_COMMAND
je !wmCommand
cmp [wmsg],WM_NOTIFY
je !wmNotify
cmp [wmsg],WM_HSCROLL
je !wmScroll
cmp [wmsg],WM_VSCROLL
je !wmScroll
cmp [wmsg],WM_MOUSEMOVE
je !wmMouseMove
cmp [wmsg],WM_LBUTTONDOWN
je !wmLButtonDown
cmp [wmsg],WM_LBUTTONUP
je !wmLButtonUp
cmp [wmsg],WM_RBUTTONDOWN
je !wmRButtonDown
cmp [wmsg],WM_RBUTTONUP
je !wmRButtonUp
mov eax,[wmsg]
cmp [FindMsgID],eax
je !FindMsg

!DefWndProc:
invoke DefWindowProc,[!hwnd],[wmsg],[wparam],[lparam]
jmp !Finish

!wmSysCommand:
mov [!PassSystemEvent],0
mov eax,[esi+evSysCommandOffs]
or eax,eax
je !DefWndProc
mov edx,[wparam]
cmp edx,$f020
je !DoSysCmd
cmp edx,$f030
je !DoSysCmd
cmp edx,$f060
je !DoSysCmd
cmp edx,$f120
je !DoSysCmd
jmp !DefWndProc
!DoSysCmd:
mov [STATUS],edx
call dword eax
cmp [!PassSystemEvent],0
jne !DefWndProc
xor eax,eax
jmp !Finish

!wmSize:; need to update RightOffs and BottomOffs
mov ecx,[lparam]
mov edx,ecx
and ecx,$ffff
shr edx,16
mov [esi+WidthOffs],ecx
mov [esi+HeightOffs],edx
mov eax,[esi+evResizeOffs]
or eax,eax
je !DefWndProc
mov edx,[wparam]
mov [STATUS],edx
call dword eax
xor eax,eax
jmp !Finish

!wmMove:; need to update RightOffs and BottomOffs
mov ecx,[lparam]
mov edx,ecx
and ecx,$ffff
shr edx,16
mov [esi+LeftOffs],ecx
mov [esi+TopOffs],edx
mov eax,[esi+evMoveOffs]
or eax,eax
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!FindMsg:
mov edx,[FindEVPtr]
or edx,edx
je !DefWndProc
mov eax,[fr_Flags]
mov [FLAGS],eax
call dword edx
je !DefWndProc

!wmTimer:
mov edx,[wparam]
shl edx,2
add edx,!Timer
mov eax,[edx]
cmp eax,0
je !Finish
call dword eax
xor eax,eax
jmp !Finish

!wmNotify:
mov edi,[lparam]; Address of NMHDR
mov [NMHDR],edi; Save NMHDR for event processing
mov esi,[edi]; Handle
invoke GetWindowLong,esi,GWL_USERDATA
cmp eax,0
je !DefWndProc
mov esi,eax; Address of Desc
mov ebx,[edi+8]; Code
mov eax,[esi+ArrayOffs]
mov [ArrayIndex],eax
mov eax,[esi+TypeOffs]
mov [!dsControlType],eax
mov ecx,cdNotify32Offs+4; Points past last Notify Offs
add ecx,esi; Add Offs to base
add esi,evNotify1Offs; Points to first Notify Offs
!NotifyLoop:
cmp ebx,[esi+4]
jne !NotNotifyCode
mov eax,[esi]
cmp eax,0
jne !ProcNotify
jmp !DefWndProc
!NotNotifyCode:
add esi,8; Point to next Notify
cmp esi,ecx; Past last Notify?
jne !NotifyLoop; No, Try next
jmp !DefWndProc; Give up, no Notify match

!ProcNotify:
mov edi,[NMHDR]
add edi,12
cmp [!dsControlType],!CT_RAGrid
jne !RAEditCheck
mov ebx,[edi]
add ebx,[!ArrayBase]
mov [GridCol],ebx
mov ebx,[edi+4]
add ebx,[!ArrayBase]
mov [GridRow],ebx
jmp !NotifyDispatch
!RAEditCheck:
cmp [!dsControlType],!CT_RAEdit
jne !TabFolderCheck
mov ebx,[edi]
add ebx,[!ArrayBase]
mov [EditcpMin],ebx
sub ebx,[edi+14]
mov [EditcpLine],ebx
mov ebx,[edi+4]
add ebx,[!ArrayBase]
mov [EditcpMax],ebx
mov bx,[edi+8]
mov [EditSelType],bx
mov ebx,[edi+10]
add ebx,[!ArrayBase]
mov [EditLine],ebx
mov ebx,[edi+18]
add ebx,[!ArrayBase]
mov [EditlpLine],ebx
mov ebx,[edi+22]
add ebx,[!ArrayBase]
mov [EditnLines],ebx
mov ebx,[edi+26]
mov [EditnHidden],ebx
mov ebx,[edi+30]
mov [EditfChanged],ebx
mov ebx,[edi+34]
add ebx,[!ArrayBase]
mov [EditnPage],ebx
mov ebx,[edi+38]
add ebx,[!ArrayBase]
mov [EditnWordGroup],ebx
jmp !NotifyDispatch
!TabFolderCheck:
cmp [!dsControlType],!CT_TABFOLDER
jne !DefWndProc
!NotifyDispatch:
jmp !CommandDispatch

!wmMouseMove:
call !GetMousePos
mov eax,[esi+evMouseMoveOffs]
or eax,eax
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!wmLButtonDown:
call !GetMousePos
mov eax,[esi+evLButtonDownOffs]
or eax,eax
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!wmLButtonUp:
call !GetMousePos
mov eax,[esi+evLButtonUpOffs]
or eax,eax
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!wmRButtonDown:
call !GetMousePos 
mov eax,[esi+evRButtonDownOffs]
or eax,eax
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!wmRButtonUp:
call !GetMousePos 
mov eax,[esi+evRButtonUpOffs]
or eax,eax
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!GetMousePos:
mov ebx,[lparam]
and ebx,$ffff
mov [CursorPosX],ebx
mov ebx,[lparam]
shr ebx,16
mov [CursorPosY],ebx
ret

!wmCreate:
mov eax,[esi+evCreateOffs]
cmp eax,0
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!wmPaint:
mov eax,[esi+evPaintOffs]
cmp eax,0
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!CtlColorChange:
mov [!PassColorChangeEvent],0
cmp [!TrapColorChange],0
je !NotColorChangeTrap
mov eax,[!TrapColorChange]
call dword eax
cmp [!PassColorChangeEvent],0
jne !NotColorChangeTrap
xor eax,eax
jmp !Finish
!NotColorChangeTrap:
invoke GetWindowLong,[lparam],GWL_USERDATA
cmp [!Desc],0
je !DefWndProc
mov esi,[!Desc]
call !GetDesc
bt [!dsStatus],WinChangeBackColor
jc !CtlBackcolorChange
bt [!dsStatus],WinChangeForeColor
jc !CtlForeColorChange
jmp !DefWndProc

!CtlBackcolorChange:
bt [!dsStatus],WinChangeForeColor
jc !CtlBothColorChange
cmp [!dsBackColor],$ffffffff
jne !NotTransparent
invoke SetBkMode,[wparam],TRANSPARENT
invoke GetStockObject,NULL_BRUSH
jmp !Finish
!NotTransparent:
call !BackColorChange
jmp !Finish

!BackColorChange:
invoke DeleteObject,[!Brush]
invoke CreateSolidBrush,[!dsBackColor]
mov [!Brush],eax
invoke SetBkColor,[wparam],[!dsBackColor]
mov eax,[!Brush]
ret

!CtlBothColorChange:
call !BothColorChange
jmp !Finish

!BothColorChange:
invoke DeleteObject,[!Brush]
invoke CreateSolidBrush,[!dsBackColor]
mov [!Brush],eax
invoke SetBkColor,[wparam],[!dsBackColor]
invoke SetTextColor,[wparam],[!dsForeColor]
mov eax,[!Brush]
ret

!CtlForeColorChange:
call !ForeColorChange
jmp !Finish

!ForeColorChange:
invoke DeleteObject,[!Brush]
invoke GetBkColor,[wparam]
cmp [!dsControlType],!CT_TEXTBOX
je !ListOrTextOrCombo
cmp [!dsControlType],!CT_LISTBOX
je !ListOrTextOrCombo
cmp [!dsControlType],!CT_COMBOBOX
je !ListOrTextOrCombo
invoke GetPixel,[wparam],0,0
!ListOrTextOrCombo:
mov esi,[!Desc]
bts dword [esi+StatusOffs],WinChangeBackColor
mov [esi+BackColorOffs],eax
invoke CreateSolidBrush,eax
mov [!Brush],eax
invoke SetTextColor,[wparam],[!dsForeColor]
invoke SetBkMode,[wparam],TRANSPARENT
mov eax,[!Brush]
ret

!wmCommand:
mov ax,[EventID]
cmp ax,999
jg !wmMenu
cmp [!Desc],0
je !DefWndProc
mov esi,[!Desc]
mov eax,[esi+ArrayOffs]
mov [ArrayIndex],eax
mov eax,[esi+evCommandOffs]
cmp eax,0
jne !CommandDispatch
xor ebx,ebx
mov bx,[EventNotify]
; Click
cmp ebx,[esi+cdClickOffs]
jne !evChange
mov eax,[esi+evClickOffs]
cmp eax,0
jne !CommandDispatch
jmp !DefWndProc
!evChange:
cmp ebx,[esi+cdChangeOffs]
jne !evSelect
mov eax,[esi+evChangeOffs]
cmp eax,0
jne !CommandDispatch
jmp !DefWndProc
!evSelect:
cmp ebx,[esi+cdSelectOffs]
jne !evDblClick
mov eax,[esi+evSelectOffs]
cmp eax,0
jne !CommandDispatch
jmp !DefWndProc
!evDblClick:
cmp ebx,[esi+cdDblClickOffs]
jne !DefWndProc
mov eax,[esi+evDblClickOffs]
cmp eax,0
je !DefWndProc
!CommandDispatch:
call dword eax
xor eax,eax
jmp !Finish

!wmMenu:
mov edx,[MenuEVPtr]
or edx,edx
je !DefWndProc
and eax,$ffff
mov [MenuID],eax
call dword edx
jmp !Finish

!wmScroll:
cmp [!Desc],0
je !DefWndProc
mov esi,[!Desc]
mov eax,[esi+ArrayOffs]
mov [ArrayIndex],eax
mov eax,[esi+evScrollOffs]
cmp eax,0
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!wmContextMenu:
invoke GetWindowLong,[wparam],GWL_USERDATA
or eax,eax
je !DefWndProc
mov esi,eax
mov eax,[esi+ArrayOffs]
mov [ArrayIndex],eax
call !GetMousePos
mov eax,[esi+evContextMenuOffs]
cmp eax,0
je !DefWndProc
call dword eax
xor eax,eax
jmp !Finish

!GetDesc:
mov eax,[esi+StatusOffs]
mov [!dsStatus],eax
mov eax,[esi+BackColorOffs]
mov [!dsBackColor],eax
mov eax,[esi+ForeColorOffs]
mov [!dsForeColor],eax
mov eax,[esi+TypeOffs]
mov [!dsControlType],eax
ret

!wmDestroy:
invoke DeleteObject,[!Brush]
invoke PostQuitMessage,0
xor eax,eax

!Finish:
pop edi esi ebx
return

; LN:6 OBMain.CREATE
mov edi,!OBMain
mov [!TargetGUIDesc],edi
OBMain_CREATE:
; LN:7 createbitmap bm1,OBCNew
invoke GetDC,NULL
mov [_ArgSafe0],eax
mov ebx,!bm1+14
invoke CreateDIBSection,eax,ebx,0,_ArgSafe1,0,0
mov [bm1],eax
mov edi,[_ArgSafe1]
mov esi,!bm1
mov ecx,[esi+2]
mov edx,[esi+10]
sub ecx,edx
add esi,edx
_Lbl1:
mov eax,[esi]
mov [edi],eax
inc esi
inc edi
dec ecx
jne _Lbl1
invoke ReleaseDC,0,[_ArgSafe0]
; LN:8 createbitmap bm2,OBCOpen
invoke GetDC,NULL
mov [_ArgSafe0],eax
mov ebx,!bm2+14
invoke CreateDIBSection,eax,ebx,0,_ArgSafe1,0,0
mov [bm2],eax
mov edi,[_ArgSafe1]
mov esi,!bm2
mov ecx,[esi+2]
mov edx,[esi+10]
sub ecx,edx
add esi,edx
_Lbl2:
mov eax,[esi]
mov [edi],eax
inc esi
inc edi
dec ecx
jne _Lbl2
invoke ReleaseDC,0,[_ArgSafe0]
; LN:9 createbitmap bm3,OBCSave1
invoke GetDC,NULL
mov [_ArgSafe0],eax
mov ebx,!bm3+14
invoke CreateDIBSection,eax,ebx,0,_ArgSafe1,0,0
mov [bm3],eax
mov edi,[_ArgSafe1]
mov esi,!bm3
mov ecx,[esi+2]
mov edx,[esi+10]
sub ecx,edx
add esi,edx
_Lbl3:
mov eax,[esi]
mov [edi],eax
inc esi
inc edi
dec ecx
jne _Lbl3
invoke ReleaseDC,0,[_ArgSafe0]
; LN:10 END EVENT
ret

; Automatic END statement
invoke DeleteObject,[!Brush]
invoke PostMessage,[!OBMain],WM_CLOSE,0,0
_ErrExit:
cinvoke puts,_ErrExitMsg
mov eax,[ERR]
invoke ExitProcess, eax
; End of Code Section

; Init Section
_Init:

mov [!OBMain+80],OBMain_CREATE
ret
; End of Init Section

; Function Code Section

__MovArg:
mov esi,[_ArgList]
mov edx,[edi+4]
mov edi,[edi]
__MovArg1:
mov al,[esi]
cmp al,32
je __MovArgSpace
mov [edi],al
or al,al
je __MovArgDone
inc esi
inc edi
dec edx
je __MovArgDone
jmp __MovArg1
__MovArgSpace:
xor al,al
mov [edi],al
inc esi
__MovArgDone:
mov [_ArgList],esi
ret
; End of Function Section

