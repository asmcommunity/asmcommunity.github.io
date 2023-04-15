; ==================================================================================================
; Title:   StopWatch.inc
; Author:  G. Friedrich
; Version: 1.0.0
; Purpose: Support of performance monitoring objects.
; Link:    http://www.agner.org/assem/
; Notes:   Version 1.0.0, December 2002
;            - First release.
; ==================================================================================================


SWH_STOPPED    equ    0
SWH_RUNNING    equ    1

Object StopWatch, StopWatchID, Primer
  StaticMethod      GetTime
  StaticMethod      GetTimeStr,     Pointer
  RedefineMethod    Init,           Pointer              ;-> Owner
  StaticMethod      Reset
  StaticMethod      Start
  StaticMethod      Stop

  DefineVariable    qFrequency,     qword,      0        ;Perf. timer frequency
  DefineVariable    qStartCount,    qword,      0        ;Starting Count
  DefineVariable    qAccCount,      qword,      0        ;Accumulated Count
  DefineVariable    dSubCount,      dword,      0        ;Overhead (Start/Stop)
  DefineVariable    dStatus,        dword,      SWH_STOPPED
ObjectEnd


; ==================================================================================================

if IMPLEMENT

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    StopWatch.GetTime
; Purpose:   Returns the elapsed time in miliseconds.
; Arguments: None.
; Return:    Accumulated time in edx::eax.

Method StopWatch.GetTime
    local dMultiplicator:dword, qTime:qword

    SetObject ecx
    mov dMultiplicator, 1000
    fild [ecx].qAccCount
    fild [ecx].qFrequency
    fdivp st(1), st(0)
    fimul dMultiplicator
    fistp qword ptr qTime
    qmov edx::eax, qTime
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    StopWatch.GetTimeStr
; Purpose:   Returns the elapsed time in seconds.
; Arguments: Pointer to a buffer where the string will be located (at least 20 bytes!).
; Return:    Nothing.

Method StopWatch.GetTimeStr,, pResult:Pointer
    SetObject ecx
    fild [ecx].qAccCount
    fild [ecx].qFrequency
    fdivp st(1), st(0)
    invoke St0ToStr, pResult, 0, 6, f_NOR
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    StopWatch.Init
; Purpose:   Initializes and calibrates the object.
; Arguments: Pointer to an owner object.
; Return:    Nothing.

Method StopWatch.Init, uses ebx esi, pOwner:Pointer
    SetObject esi
    ACall esi.Init, pOwner
    invoke QueryPerformanceFrequency, addr [esi].qFrequency
    mov [esi].dSubCount, 0
    mov ebx, 256
    OCall esi::StopWatch.Reset
    .repeat
      OCall esi.Start
      OCall esi.Stop
      dec ebx
    .until ebx == 0
    mov eax, dword ptr [esi].qAccCount
    shr eax, 8
    mov [esi].dSubCount, eax
    OCall esi.Reset
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    StopWatch.Reset
; Purpose:   Resets the time count.
; Arguments: None.
; Return:    Nothing.

Method StopWatch.Reset
    SetObject ecx
    .if [ecx].dStatus == SWH_STOPPED
      xor eax, eax
      mov dword ptr [ecx].qAccCount, eax
      mov dword ptr [ecx + 4].qAccCount, eax
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    StopWatch.Start
; Purpose:   Starts the time measuring.
; Arguments: None.
; Return:    Nothing.

Method StopWatch.Start
    local dTimeStartHi:dword, dTimeStartLo:dword

    invoke QueryPerformanceCounter, addr dTimeStartLo
    SetObject ecx
    .if [ecx].dStatus == SWH_STOPPED
      mov [ecx].dStatus, SWH_RUNNING
      mov eax, dTimeStartLo
      mov dword ptr [ecx].qStartCount, eax
      mov edx, dTimeStartHi
      mov dword ptr [ecx + 4].qStartCount, edx
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    StopWatch.Stop
; Purpose:   Stops the time measuring.
; Arguments: None.
; Return:    Nothing.

Method StopWatch.Stop
    local dCountStopHi:dword, dCountStopLo:dword

    invoke QueryPerformanceCounter, addr dCountStopLo
    SetObject ecx
    .if [ecx].dStatus == SWH_RUNNING
      mov eax, dCountStopLo
      mov edx, dCountStopHi
      sub eax, dword ptr [ecx].qStartCount
      sbb edx, dword ptr [ecx + 4].qStartCount
      sub eax, [ecx].dSubCount
      sbb edx, 0
      add dword ptr [ecx].qAccCount, eax
      adc dword ptr [ecx + 4].qAccCount, edx
      mov [ecx].dStatus, SWH_STOPPED
    .endif
MethodEnd

endif