; ==================================================================================================
; Title:   WinApp.inc
; Author:  G. Friedrich
; Version: 1.0.1
; Purpose: Support of WinApp objects for ObjAsm32.
; Notes:   WinApp is the base object for all SDI and MDI derivated objects.
;          It implements basic features like modeless dialog processing.
;          Version 1.0.0, August 2008
;            - First release.
;          Version 1.0.1, October 2009
;            - OnIdle moved from MdiApp and SdiApp.
; ==================================================================================================


Object WinApp, WinAppID, WinPrimer
  RedefineMethod    Init
  VirtualMethod     OnIdle,         dword

  DefineVariable    hActiveMlsDlg,  Handle,     0
ObjectEnd

; ==================================================================================================

if IMPLEMENT

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    WinApp.Init
; Purpose:   Initializes WinApp object.
; Arguments: None.
; Return:    Nothing.

Method WinApp.Init, NOFRAME
    SetObject ecx,, [esp + 4]
    ACall ecx.Init, NULL, 0
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    WinApp.OnIdle
; Purpose:   rocedure called when the app is idle.
; Arguments: Arg1: Number of times OnIdle was called since the last processed message.
; Return:    eax = Zero to stop getting more OnIdle calls.

Method WinApp.OnIdle, NOFRAME, dCount:dword
    xor eax, eax
MethodEnd


endif