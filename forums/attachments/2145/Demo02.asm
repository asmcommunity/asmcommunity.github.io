; ==================================================================================================
; Title:   Demo02.asm
; Author:  G. Friedrich  @ July 2003
; Version: 1.0.0
; Purpose: Demonstration program 2 of ObjAsm32.
; ==================================================================================================


%include @Environ(OA32_PATH)\\Code\\Macros\\Model.inc       ;Include & initialize standard modules
SysSetup OOP_WINDOWS, DEBUG(WND);, RESGUARD)                 ;Loads OOP files and OS related objects

include Demo02_Globals.inc                                  ;Includes application globals
IncludeBoth HtmlHelp

LoadObjects Dialog, DialogModal, DialogAbout                ;Loads or builds the following objects
LoadObjects SdiApp

.code
include Demo02.inc                                          ;Includes DemoApp02 object

start:                                                      ;Program entry point
    SysInit                                                 ;Runtime initialization of OOP model

    ResGuard_Start                                          ;Activates ResGuard
    OCall @DemoApp02::DemoApp02.Init                        ;Initializes the object data
    OCall @DemoApp02::DemoApp02.Run                         ;Executes the application
    OCall @DemoApp02::DemoApp02.Done                        ;Finalizes it
    ResGuard_Show                                           ;Shows ResGuard results

    SysDone                                                 ;Runtime finalization of the OOP model
    invoke ExitProcess, 0                                   ;Exits program returning 0 to the OS
end start                                                   ;Code end and defines prg entry point
