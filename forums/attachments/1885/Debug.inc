; ==================================================================================================
; Title:   Debug.inc
; Author:  G. Friedrich
; Version: 1.0.12
; Purpose: Debug macros for ObjAsm32.
; Notes:   Version 1.0.0, January 2005
;            - First release.
;          Version 1.0.7 March 2005
;            - ResGuard macros added.
;          Version 1.0.8 May 2005
;            - DbgReal4Ptr and DbgReal8Ptr macros added by NaN.
;          Version 1.0.9 July 2005
;           - Synchronization problems with fast messages solved using SendMessageTimeout to block
;             parallel processing of new entering messages.
;           - Thread syncronization problem solved using critical sections.
;          Version 1.0.10 December 2005
;           - DbgMem dSize bug found by EvilHomer2K.
;           - DbgUStr added.
;           - DbgGUID corrected.
;          Version 1.0.11, March 2006
;            - Local macro variables redefined as global symbols.
;          Version 1.0.12, July 2006
;            - Stack alignment before each Dbgxxx macro (See DbgSaveContext & DbgLoadContext).
; ==================================================================================================

;Supported debugging macros:
;———————————————————————————
;  - DbgLine, DbgText, DbgWarning, DbgStr, DbgUStr
;  - DbgBin, DbgDec, DbgHex, DbgFloat, DbgReal4Ptr, DbgReal8Ptr, DbgGUID
;  - DbgMem, DbgGlobalMemUsage, DbgFpu
;  - DbgBmp
;  - DbgApiError, DbgComError, DbgMessage
;  - DbgObject, DbgTraceObject, DbgTraceShow
;  - ResGuard_Show, ResGuard_Start, ResGuard_Stop

;DEBUGGING  = TRUE / FALSE   ;Turns debugging output ON or OFF.
;TRACING    = TRUE / FALSE   ;Turns trace output ON or OFF.

; ==================================================================================================

DBG_DEV_WND         equ   00h
DBG_DEV_CON         equ   01h
DBG_DEV_LOG         equ   02h

DBG_OPT_NONE        equ   00h
DBG_OPT_SHOWINFO    equ   01h

DBG_MEM_STR         equ   100
DBG_MEM_UI8         equ   101
DBG_MEM_UI16        equ   102
DBG_MEM_UI32        equ   103
DBG_MEM_I8          equ   104
DBG_MEM_I16         equ   105
DBG_MEM_I32         equ   106
DBG_MEM_R4          equ   107
DBG_MEM_R8          equ   108

DBG_FLOAT_RESOLUTION  equ 12
DBG_FLOAT_BUFFERSIZE  equ 24      ;(DBG_FLOAT_RESOLUTION + 9) roundup to the next 4 boundary

if DEBUGGING
    CStr szDbgErr, "Debug Error"
    CStr szObjTrcErr, "Object Error"

    CStr szDbgFPU1, "Conditional: ST !> Source"     ;Floating point messages
    CStr szDbgFPU2, "Conditional: ST !< Source"
    CStr szDbgFPU3, "Conditional: ST = Source"
    CStr szDbgFPU4, "Conditional: Undefined"
    CStr szDbgFPU5, "FPU Levels : %d "
    CStr szDbgFPU6, "Exceptions : e s p u o z d i"  ;FPU exception bits
    CStr szDbgFPU7, "  st(%d) = %s"

    CStr szDbgMemFmt, "%0.8X: %0.2X %0.2X %0.2X %0.2X - %0.2X %0.2X %0.2X %0.2X - ", \
                      "%0.2X %0.2X %0.2X %0.2X - %0.2X %0.2X %0.2X %0.2X"

    CStr szDbgElipsis, "..."
    CStr szDbgInvalid, "????"
    CStr szDbgPoint2, " -!>"
    CStr szDbgComma, ", "

    CStr szDbgCtrCls, DEBUG_CENTER_CLASS

    externdef dDbgDev:dword       ;Exports dDbgDev symbol, defined in SetupSys.inc
    externdef hDbgDev:Handle

    .data
      hDbgDev             Handle    0
      pObjIDTableStart    Pointer   NULL
      dObjIDTableCount    dword     0
      pObjErrTableStart   Pointer   NULL
      dObjErrTableCount   dword     0
endif


; ==================================================================================================
;    Helper macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgOutHex
; Purpose:   Outputs a number in hexadecimal on the debug output device.
; Arguments: Arg1: Number: register or symbol(16 bytes).
;            Arg2: Output color (default = black)
;            Arg3: Destination Window Name.
; Note:      If register addressing is used, a size prefix is required.
;            i.e.:  DbgHexHex dword ptr [ecx]
;            This macro trashes ebx and edi!

DbgOutHex macro Var:req, Color:=<0>, Dest:req
    ??DbgDataSize = $GetSize(Var)
    if (.type Var) and 00010000b          ;;Is Var a register?
      if ??DbgDataSize eq 4
        ifdifi <Var>, <ebx>
          mov ebx, Var
        endif
      elseif ??DbgDataSize eq 2
        ifdifi <Var>, <bx>
          mov bx, Var
        endif
        shl ebx, 16
      else
        ifdifi <Var>, <bl>
          mov bl, Var
        endif
        shl ebx, 24
      endif
      sub esp, 12                         ;;Reserve stack space = 8 chars + 1 zero + 3 padding bytes
      mov edi, esp
      invoke dword2hex, edi, ebx
      if ??DbgDataSize eq 2
        mov byte ptr [edi + 4], 0
      elseif ??DbgDataSize eq 1
        mov byte ptr [edi + 2], 0
      endif
      @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
      add esp, 12
    else
      lea ebx, Var
      invoke IsBadReadPtr, ebx, ??DbgDataSize
      .if eax != 0
        @invoke DbgOutText, offset szDbgInvalid, Color, DBG_EFFECT_NORMAL, Dest
      .else
      sub esp, 12                         ;;Reserve stack space = 8 chars + 1 zero + 3 padding bytes
        mov edi, esp
        if ??DbgDataSize ge 16
          if ??DbgDataSize ne 16
            @invoke DbgOutText, offset szDbgElipsis, Color, DBG_EFFECT_NORMAL, Dest
          endif
          invoke dword2hex, edi, [ebx + 12]
          @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
          invoke dword2hex, edi, [ebx + 08]
          @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
          invoke dword2hex, edi, [ebx + 04]
          @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
          invoke dword2hex, edi, [ebx]
          @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
        elseif ??DbgDataSize ge 8
          if ??DbgDataSize ne 8
            @invoke DbgOutText, offset szDbgElipsis, Color, DBG_EFFECT_NORMAL, Dest
          endif
          invoke dword2hex, edi, [ebx + 4]
          @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
          invoke dword2hex, edi, [ebx]
          @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
        elseif ??DbgDataSize ge 4
          if ??DbgDataSize ne 4
            @invoke DbgOutText, offset szDbgElipsis, Color, DBG_EFFECT_NORMAL, Dest
          endif
          invoke dword2hex, edi, [ebx]
          @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
        elseif ??DbgDataSize ge 2
          if ??DbgDataSize ne 2
            @invoke DbgOutText, offset szDbgElipsis, Color, DBG_EFFECT_NORMAL, Dest
          endif
          mov cx, word ptr [ebx]
          shl ecx, 16
          invoke dword2hex, edi, ecx
          mov byte ptr [edi + 4], 0
          @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
        else
          if ??DbgDataSize ne 1
            @invoke DbgOutText, offset szDbgElipsis, Color, DBG_EFFECT_NORMAL, Dest
          endif
          mov cl, byte ptr [ebx]
          shl ecx, 24
          invoke dword2hex, edi, ecx
          mov byte ptr [edi + 2], 0
          @invoke DbgOutText, edi, Color, DBG_EFFECT_NORMAL, Dest
        endif
        add esp, 12
      .endif
    endif
    @invoke DbgOutText, "h", Color, DBG_EFFECT_NORMAL, Dest
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgShowSrcInfo
; Purpose:   Outputs source line info on the Debug Device.
; Arguments: Destination Window Name.
; Returns:   Nothing.

DbgShowSrcInfo macro Dest:req
    ??DbgIndex = 0
    ??DbgPos = 0
    %forc ??DbgChar, @FileCur      ;;Search last "\" and return is string position
      ??DbgIndex = ??DbgIndex + 1
      ifidni <&??DbgChar>, <\>
        ??DbgPos = ??DbgIndex
      endif
    endm

    .ifBitSet dDbgOpt, DBG_OPT_SHOWINFO
      @invoke DbgOutText, @CatStr(<!" [>, @SubStr(%@FileCur, ??DbgPos + 1), <, >, %@Line, <]!">), \
                      $RGB(128,128,128), DBG_EFFECT_NORMAL, Dest
    .endif
    @invoke DbgOutText, offset szNULL, $RGB(0,0,0), DBG_EFFECT_NEWLINE, Dest
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgShowTxtInfo
; Purpose:   Outputs additional text info on the Debug Device.
; Arguments: Arg1: Text
;            Arg2: Destination Window Name.
; Returns:   Nothing.

DbgShowTxtInfo macro Text, Dest
    ifnb <&Text>
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, Dest
      @invoke DbgOutText, Text, $RGB(0,0,0), DBG_EFFECT_NORMAL, Dest
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgSaveContext
; Purpose:   Saves CPU registers and flags onto stack and aligns it.
; Arguments: Mone.
; Returns:   Nothing.

DbgSaveContext macro
    pushfd
    pushad
    mov eax, esp                ;Save current esp value in eax
    and esp, 0FFFFFFFCh         ;Align stack on a 4 byte boundary
    push eax                    ;Save old stack value to restore registers and flags
    mov eax, [eax + 7 * 4]      ;Restore old eax value
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgLoadContext
; Purpose:   Loads CPU registers and flags from stack and restores stack alignment.
; Arguments: Mone.
; Returns:   Nothing.

DbgLoadContext macro
    pop esp                     ;Restore old esp
    popad
    popfd
endm

; ==================================================================================================
;   String debugging macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgLine
; Purpose:   Draws a line on the Debug Device.
; Arguments: Destination Window Name.

DbgLine macro Dest
    if DEBUGGING
      DbgSaveContext
      invoke EnterCriticalSection, addr DbgCritSect
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      @invoke DbgOutText, "————————————————————————————————————————————————————————————", \
              $RGB(0,0,0), DBG_EFFECT_NORMAL or DBG_EFFECT_NEWLINE, %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgText
; Purpose:   Outputs a text on the debug device.
; Arguments: Arg1: Text.
;            Arg2: Destination Window Name.

DbgText macro Text, Dest
    if DEBUGGING
      DbgSaveContext
      invoke EnterCriticalSection, addr DbgCritSect
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      ifb <&Text>
        @invoke DbgOutText, @CatStr(<!">,<%@FileCur>,<!">), $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        @invoke DbgOutText, <Text>, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgWarning
; Purpose:   Outputs a text on the debug device on red color.
; Arguments: Arg1: Text.
;            Arg2: Destination Window Name.

DbgWarning macro Text, Dest
    if DEBUGGING
      DbgSaveContext
      invoke EnterCriticalSection, addr DbgCritSect
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      ifb <&Text>
        @invoke DbgOutText, "WARNING", $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        @invoke DbgOutText, <Text>, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgStr
; Purpose:   Outputs a given string on the debug device.
; Arguments: Arg1: -> String.
;            Arg2: Destination Window Name.

DbgStr macro pAddr:req, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
;      @invoke DbgOutText, "&pAddr& -\] ", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, pAddr, $RGB(64,64,64), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgUStr
; Purpose:   Outputs a given UNICODE string on the debug device.
; Arguments: Arg1: -> String.
;            Arg2: Destination Window Name.

DbgUStr macro pAddr:req, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      @invoke DbgOutUText, pAddr, $RGB(64,64,64), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ==================================================================================================
;   Number debugging macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DgbHex
; Purpose:   Displays a number in hexadecimal format on the Debug Device.
; Arguments: Arg1: Number (register or symbol).
;            Arg2: Additional information.
;            Arg3: Destination Window name.
; Note:      If register addressing is used, a size prefix is required.
;            i.e.:  DbgHex dword ptr [ecx]

DbgHex macro Var:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      pushad
      @invoke DbgOutText, "&Var = ", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      popad
      DbgOutHex Var, 0, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgBin
; Purpose:   Outputs a number in binary format  on the Debug Device.
; Arguments: Arg1: Number (register or symbol). Max 32 bits.
;            Arg2: Additional information.
;            Arg3: Destination Window name.
; Note:      If register addressing is used, a size prefix is required.
;            i.e.:  DbgBin dword ptr [ecx]

DbgBin macro Var:req, InfoText, Dest
    local @@0, @@1, @@2

    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      ??DbgDataSize = $GetSize(Var)
      if ??DbgDataSize eq 1
        ifdifi <Var>, <bl>
          mov bl, Var
        endif
        shl ebx, 24
      elseif ??DbgDataSize eq 2
        ifdifi <Var>, <bx>
          mov bx, Var
        endif
        shl ebx, 16
      elseif ??DbgDataSize eq 4
        ifdifi <Var>, <ebx>
          mov ebx, Var
        endif
      endif
      @invoke DbgOutText, "&Var = ", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      sub esp, 40                   ;;Reserve a buffer on the stack. Keep stack aligned!
      mov edi, esp                  ;;Get a pointer to the buffer
      push edi                      ;;Save it
      mov ecx, ??DbgDataSize * 8
@@0:
      rcl ebx, 1                    ;;Set bit in carry flag
      jc @@1                        ;;Test carry flag
      mov byte ptr [edi], "0"
      jmp @@2
@@1:
      mov byte ptr [edi], "1"
@@2:
      inc edi
      loop @@0
      mov word ptr [edi], "y"       ;;Set terminator zero and "y" specifier
      pop edi
      @invoke DbgOutText, edi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      add esp, 40                   ;;Restore stack
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgDec
; Purpose:   Outputs a number in decimal format on the Debug Device.
; Arguments: Arg1: Number (register or symbol). Max 32 bits
;            Arg2: Additional information.
;            Arg3: Destination Window name.
; Note:      If register addressing is used, a size prefix is required.
;            i.e.:  DbgDec dword ptr [ecx]

DbgDec macro Var:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      ??DbgDataSize = $GetSize(Var)
      if ??DbgDataSize eq 1
        ifdifi <Var>, <al>
          mov al, Var
        endif
        cbw                         ;;al -> ax
        cwde                        ;;ax -> eax
        mov ebx, eax
      elseif ??DbgDataSize eq 2
        ifdifi <Var>, <ax>
          mov ax, Var
        endif
        cwde                        ;;ax -> eax
        mov ebx, eax
      elseif ??DbgDataSize eq 4
        ifdifi <Var>, <ebx>
          mov ebx, Var
        endif
      endif

      @invoke DbgOutText, "&Var = ", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      sub esp, 32                   ;;Reserve a buffer on the stack. Keep stack aligned!
      mov edi, esp                  ;;Get a pointer to the buffer
      @invoke wsprintf, edi, "%li", ebx
      @invoke DbgOutText, edi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, "t", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd  ;;Output "t" specifier
      add esp, 32                   ;;Restore stack
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgFloat
; Purpose:   Displays a floating point number in decimal format on the Debug Device.
; Arguments: Arg1: Floating point number (real4 or real8)
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgFloat macro Var: req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      sub esp, 108                  ;;Reserve FPU storage. Keep stack aligned!
      mov edi, esp
      fnsave [edi]
      fld Var
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, "&Var = ", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
      frstor [edi]
      add esp, 108                  ;;Restore stack
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgReal8Ptr
; Purpose:   Displays a floating point number in decimal format on the Debug Device.
; Arguments: Arg1: Floating point number pointer to a Real8 number in any register
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgReal8Ptr macro Var: req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      sub esp, 108                  ;;Reserve FPU storage. Keep stack aligned!
      mov edi, esp
      fnsave [edi]
      fld QWORD PTR [Var]
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, "&Var = ", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
      frstor [edi]
      add esp, 108                  ;;Restore stack
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgReal4Ptr
; Purpose:   Displays a floating point number in decimal format on the Debug Device.
; Arguments: Arg1: Floating point number pointer to a Real8 number in any register
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgReal4Ptr macro Var: req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      sub esp, 108                  ;;Reserve FPU storage. Keep stack aligned!
      mov edi, esp
      fnsave [edi]
      fld dword ptr [Var]
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, "&Var = ", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
      frstor [edi]
      add esp, 108                  ;;Restore stack
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Procedure: DbgGUID
; Purpose:   Outputs the a GUID on the debug device.
; Arguments: Arg1: -> GUID.
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgGUID macro pGUID:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      sub esp, 40                   ;;Get a buffer for 36 characters + zero + 3 filler bytes to
      mov edi, esp                  ;;Keep the stack aligned
      invoke GUID2Str, edi, pGUID
;      @invoke DbgOutText, "&pGUID& -\] ", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, edi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      add esp, 40                   ;;Restore stack
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm



; ==================================================================================================
;   Bitmap debugging macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgBmp
; Purpose:   Outputs a given bitmap on the debug device.
; Arguments: Arg1: -> Bitmap handle.
;            Arg2: Destination Window Name.

DbgBmp macro hBmp:req, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      @invoke DbgOutBitmap, eax, %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm


; ==================================================================================================
;   Memory debugging macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgMem
; Purpose:   Outputs a memory block on the Debug Device.
; Arguments: Arg1: Pointer to memory block.
;            Arg2: Size of memory block.
;            Arg3: Additional information.
;            Arg4: Output format (DBG_MEM_STR...DBG_MEM_R8).
;            Arg4: Destination Window Name.

DbgMem macro pData:req, dSize:req, InfoText:=<>, vCode:=<100>, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      ifdifi <pData>, <ebx>
        mov ebx, pData
      endif
      ifdifi <dSize>, <edi>
        mov edi, dSize
      endif
      @invoke DbgOutText, "MEMORY DUMP", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      @invoke DbgOutMem, ebx, edi, vCode, %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgGlobalMemUsage
; Purpose:   Displays global memory usage on the debug window.
; Arguments: Arg1: Additional information.
;            Arg2: Destination Window Name.

DbgGlobalMemUsage macro InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      invoke EnterCriticalSection, addr DbgCritSect
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      sub esp, 100                        ;Keep stack aligned!
      mov esi, esp
      sub esp, sizeof MEMORYSTATUS        ;Keep stack aligned!
      mov edi, esp
      mov [edi].MEMORYSTATUS.dwLength, sizeof MEMORYSTATUS
      invoke GlobalMemoryStatus, edi

      @invoke wsprintf, esi, "Memory load: %li%%", [edi].MEMORYSTATUS.dwMemoryLoad
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_UNDERLINE or DBG_EFFECT_BOLD, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd

      @invoke wsprintf, esi, "  Physical: %li of %li bytes.", \
              [edi].MEMORYSTATUS.dwAvailPhys, [edi].MEMORYSTATUS.dwTotalPhys
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd
      @invoke wsprintf, esi, "  PageFile: %li of %li bytes.", \
              [edi].MEMORYSTATUS.dwAvailPageFile, [edi].MEMORYSTATUS.dwTotalPageFile
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd
      @invoke wsprintf, esi, "  Virtual : %li of %li bytes.", \
              [edi].MEMORYSTATUS.dwAvailVirtual, [edi].MEMORYSTATUS.dwTotalVirtual
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd
      add esp, 100 + sizeof MEMORYSTATUS
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm


; ==================================================================================================
;   Hardware debugging macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgFPU
; Purpose:   Outputs the content of the FPU registers on the Debug Device.
; Arguments: Arg1: Additional information.
;            Arg2: Destination Window Name.

DbgFPU macro InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      invoke EnterCriticalSection, addr DbgCritSect
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      @invoke DbgOutText, "FPU DUMP", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      @invoke DbgOutFpu, %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm


; ==================================================================================================
;   OS debugging macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgMessage
; Purpose:   Identifies a windows message with a string.
; Arguments: Arg1: Windows message.
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgMessage macro Msg:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      @invoke DbgOutMsg, Msg, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgApiError
; Purpose:   Identifies an API error with a string.
; Arguments: Arg1: COM error code.
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgApiError macro InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      invoke GetLastError
      push eax
      invoke EnterCriticalSection, addr DbgCritSect
      pop eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      @invoke DbgOutApiErr, eax, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgComError
; Purpose:   Identifies a COM error with a string
; Arguments: Arg1: COM error code.
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgComError macro dComErr:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      @invoke DbgOutComErr, dComErr, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm


; ==================================================================================================
;   Object debugging macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgObject
; Purpose:   Outputs internal object variables to the Debug Device.
; Arguments: Arg1: InstanceHandle::ClassName.
;            Arg2: Additional information.
;            Arg3: Destination Window name.
; Return:    Nothing.

DbgObject macro Cast, InfoText, Dest
    if DEBUGGING
      ??DbgPos1 InStr <Cast>, <::>
      if ??DbgPos1 ne 0
        ifb <Dest>
          ??DbgDstWnd textequ <offset szDbgSrc>
        else
          ??DbgDstWnd textequ <Dest>
        endif
        ??IName SubStr <Cast>, 1, ??DbgPos1 - 1
        ??OName SubStr <Cast>, ??DbgPos1 + 2
        DbgSaveContext
        mov esi, ??IName
        invoke EnterCriticalSection, addr DbgCritSect
%       @invoke DbgShowObjectHeader, "&??OName", esi, ??DbgDstWnd
        DbgShowTxtInfo InfoText, %??DbgDstWnd
        DbgShowSrcInfo %??DbgDstWnd
%       DbgSpyClass ??OName, ??DbgDstWnd
        invoke LeaveCriticalSection, addr DbgCritSect
        DbgLoadContext
      else
        .err <DbgObject format error. Double Colon missing.>
      endif
    endif
endm

DbgSpyClass macro OName:req, Dest    ;;esi should point to the instance
    if DEBUGGING
%     for ??TestObject, <&OName&_InherForPath>
        ??ACount = 0
        repeat @CatStr(<??TestObject>, <_VCount>)
          ??ACount = ??ACount + 1
          ??sz1 CatStr <??TestObject>, <_Var_Name_>, %??ACount
          ??sz2 CatStr <??TestObject>, <_Var_Type_>, %??ACount
%         @invoke DbgOutText, @CatStr(<!"  >, %??sz1, <: !">), $RGB(64,64,255), DBG_EFFECT_NORMAL, Dest
          DbgOutHex @CatStr(<[esi].>, ??TestObject, <.>, %??sz1), 0, Dest    ;;This macro trashes ebx and edi
%         @invoke DbgOutText, @CatStr(<!" (>, %??sz2, < at >, <??TestObject>, <)!">), \
                  $RGB(64,64,255), DBG_EFFECT_NEWLINE, Dest
        endm
      endm
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgTraceObject
; Purpose:   Begins the tracing of an object instance.
; Arguments: InstanceHandle::ClassName

DbgTraceObject macro Cast:req
    if DEBUGGING
      if TRACING
        ??DbgPos1 InStr <Cast>, <::>
        if ??DbgPos1 ne 0
          ??DbgIName SubStr <Cast>, 1, ??DbgPos1 - 1
          ??DbgTraceOName SubStr <Cast>, ??DbgPos1 + 2
          DbgSaveContext
          PushAll eax, ecx, edx
          invoke EnterCriticalSection, addr DbgCritSect
          PopAll edx, ecx, eax
          if (opattr ??DbgIName) and 00010000b                   ;;Is ??DbgIName a register?
            mov ??pDbgTraceInstance, ??DbgIName
          else
            m2m ??pDbgTraceInstance, ??DbgIName
          endif
          invoke LeaveCriticalSection, addr DbgCritSect
          DbgLoadContext
        else
          .err <DbgTraceObject format error. Double Colon missing.>
        endif
      else
        %echo @CatStr(<WARNING: TRACE debug option is not active (>, %@FileCur, <, >, %@Line, <).>)
      endif
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgTraceShow
; Purpose:   Begins the tracing of an object instance.
; Arguments: Arg1: Additional information.
;            Arg2: Destination Window name.

DbgTraceShow macro InfoText, Dest
    if DEBUGGING
      if TRACING
        DbgSaveContext
        invoke EnterCriticalSection, addr DbgCritSect
        ifb <Dest>
          ??DbgDstWnd textequ <offset szDbgSrc>
        else
          ??DbgDstWnd textequ <Dest>
        endif
        @invoke DbgShowObjectHeader, "&??DbgTraceOName", ??pDbgTraceInstance, %??DbgDstWnd
        DbgShowTxtInfo InfoText, %??DbgDstWnd
        DbgShowSrcInfo %??DbgDstWnd
%       DbgShowTraceResult ??DbgTraceOName, ??DbgDstWnd          ;;Begin recursive display of results
        invoke LeaveCriticalSection, addr DbgCritSect
        DbgLoadContext
      else
        %echo @CatStr(<WARNING: TRACE debug option is not active (>, %@FileCur, <, >, %@Line, <).>)
      endif
    endif
endm

DbgShowTraceResult macro OName:req, Dest
%   for ??TestObject, <&OName&_InherForPath>
      ??ACount = 0
      repeat @CatStr(<??TestObject>, <_StcMtdCount>)
        ??ACount = ??ACount + 1
        ??sz1 CatStr <??TestObject>, <_StcMethod_Name_>, %??ACount

        ??DbgTraceMtdCount CatStr <MtdCnt_>, <??TestObject>, <_>, ??sz1
        ??DbgTraceMtdTicks CatStr <MtdTck_>, <??TestObject>, <_>, ??sz1

%       ifdef ??DbgTraceMtdCount
%         ifidn <??sz1>, <WndProc>
            ;;WndProc can't be traced due to Self is stablished after the prologue!
%           @invoke DbgOutText, @CatStr(<!"  >, ??TestObject, <.>, <!">), $RGB(64,64,255), \
                    DBG_EFFECT_NORMAL, Dest
%           @invoke DbgOutText, @CatStr(<!">, %??sz1, <: !">), $RGB(64,64,255), \
                    DBG_EFFECT_NORMAL, Dest
            @invoke DbgOutText, "can not be traced... !", $RGB(64,64,255), \
                    DBG_EFFECT_ITALIC or DBG_EFFECT_NEWLINE, Dest
          else
%           DbgShowTraceVar @CatStr(<??TestObject>, <.>, %??sz1), ??DbgTraceMtdCount, ??DbgTraceMtdTicks, Dest
          endif
        else                                                ;;It is an abstract method!
%         @invoke DbgOutText, @CatStr(<!"  >, ??TestObject, <.>, <!">), $RGB(64,64,255), \
                  DBG_EFFECT_NORMAL, Dest
%         @invoke DbgOutText, @CatStr(<!">, %??sz1, <: !">), $RGB(64,64,255), \
                  DBG_EFFECT_NORMAL, Dest
          @invoke DbgOutText, "is an abstract method...", $RGB(64,64,255), \
                  DBG_EFFECT_ITALIC or DBG_EFFECT_NEWLINE, Dest
        endif
      endm

      ??ACount = 0
      repeat @CatStr(<??TestObject>, <_IfcMtdCount>)
        ??ACount = ??ACount + 1
        ??sz1 CatStr <??TestObject>, <_IfcMethod_Name_>, %??ACount

        ??DbgTraceMtdCount CatStr <MtdCnt_>, <??TestObject>, <_>, ??sz1
        ??DbgTraceMtdTicks CatStr <MtdTck_>, <??TestObject>, <_>, ??sz1

%       ifdef ??DbgTraceMtdCount
%         DbgShowTraceVar @CatStr(<??TestObject>, <.>, %??sz1), ??DbgTraceMtdCount, ??DbgTraceMtdTicks, Dest
        endif
      endm

      ??ACount = 0
      repeat @CatStr(<??TestObject>, <_OMCount>)
        ??ACount = ??ACount + 1
        ??sz1 CatStr <??TestObject>, <_OvrMtd_Name_>, %??ACount

        ??DbgTraceMtdCount CatStr <MtdCnt_>, <??TestObject>, <_>, ??sz1
        ??DbgTraceMtdTicks CatStr <MtdTck_>, <??TestObject>, <_>, ??sz1

%       ifdef ??DbgTraceMtdCount
%         ifidn <??sz1>, <WndProc>
            ;;WndProc can't be traced due to Self is stablished after the prologue!
%           @invoke DbgOutText, @CatStr(<!"  >, ??TestObject, <.>, <!">), $RGB(64,64,255), \
                    DBG_EFFECT_NORMAL, Dest
%           @invoke DbgOutText, @CatStr(<!">, %??sz1, <: !">), $RGB(64,64,255), \
                    DBG_EFFECT_NORMAL, Dest
            @invoke DbgOutText, "can not be traced... !", $RGB(64,64,255), \
                    DBG_EFFECT_ITALIC or DBG_EFFECT_NEWLINE, Dest
          else
%           DbgShowTraceVar @CatStr(<??TestObject>, <.>, %??sz1), ??DbgTraceMtdCount, ??DbgTraceMtdTicks, Dest
          endif
        endif
      endm
    endm
endm

DbgShowTraceVar macro MName:req, Count:req, Ticks:req, Dest
    mov eax, @SizeStr(MName) + 1
    MemAlloc eax
    push eax
    push eax
    FillMem eax, &MName
    pop ecx
    @invoke DbgShowTraceMethod, ecx, Count, offset Ticks, Dest
    pop eax
    MemFree eax
endm


; ==================================================================================================
;   Debugging helper macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     Fix
; Purpose:   Reminder text echoed at compile time.
; Arguments: Text.

Fix macro Txt:=<Fix this later!!>
%   echo @CatStr(<Fix:> @FileCur, < (>, %@Line, <) - >, Txt)
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     ASSERT
; Purpose:   Displays a message if an argument value is FALSE.
; Arguments: Arg1: Value.
;            Arg2: Additional information.
;            Arg3: Destination Window name.

ASSERT macro Var:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      ifb <Dest>
        ??DbgDstWnd textequ <offset szDbgSrc>
      else
        ??DbgDstWnd textequ <Dest>
      endif
      .if !Var
        @invoke DbgOutText, "ASSERT ", $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
        @invoke DbgOutText, "&Var", $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
        DbgShowTxtInfo InfoText, %??DbgDstWnd
        DbgShowSrcInfo %??DbgDstWnd
      .endif
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgClear
; Purpose:   Clears the content of all or an specific Debug window.
; Arguments: Arg1: Target Debug Window name.

DbgClear macro TargetWnd
    if DEBUGGING
      DbgSaveContext
      invoke EnterCriticalSection, addr DbgCritSect
      ifb <TargetWnd>
        ??DbgDstWnd textequ <NULL>
      else
        ??DbgDstWnd textequ <TargetWnd>
      endif
      @invoke DbgOutClear, %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm


; ==================================================================================================
;   ResGuard macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     ResGuard_Show
; Purpose:   Shows the result of ResGuard system activity.
; Arguments: None.

ResGuard_Show macro
    if DEBUGGING and RESGUARD_LOADED
      invoke ResGuardShow
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     ResGuard_Start
; Purpose:   Starts activity of the ResGuard system.
; Arguments: None.

ResGuard_Start macro
    if DEBUGGING and RESGUARD_LOADED
      invoke ResGuardStart
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     ResGuard_Stop
; Purpose:   Stops activity of the ResGuard system.
; Arguments: None.

ResGuard_Stop macro
    if DEBUGGING and RESGUARD_LOADED
      invoke ResGuardStop
    endif
endm


; ==================================================================================================
;   DirectX macros
; ==================================================================================================

include DebugDX.inc
