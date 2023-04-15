; ==================================================================================================
; Title:   DebugDX.inc
; Author:  G. Friedrich
; Version: 1.0.1
; Purpose: Debug macros for DirectX.
; Notes:   Version 1.0.0, September 2005
;            - First release.
;          Version 1.0.1, March 2006
;            - Local macro variables redefined as global symbols.
; ==================================================================================================

;Supported debugging macros:
;———————————————————————————
;  - DbgVec2, DbgVec3, DbgVec4
;  - DbgPlane, DbgQuaternion


DbgPlane      equ DbgVec4
DbgQuaternion equ DbgVec4


; ==================================================================================================
;   DirectX debugging macros
; ==================================================================================================

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgVec2
; Purpose:   Outputs the components of the D3DXVECTOR2 vector.
; Arguments: Arg1: -> Vector2
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgVec2 macro Var:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      mov ebx, Var
      ifb <Dest>
        ??DstWnd textequ <offset szDbgSrc>
      else
        ??DstWnd textequ <Dest>
      endif
      @invoke DbgOutText, "&Var -\] [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd
      sub esp, 108                  ;;Reserve FPU storage. Keep stack aligned!
      mov edi, esp
      fnsave [edi]
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp

      fld [ebx].D3DXVECTOR2.x
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DstWnd
      @invoke DbgOutText, ",", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd

      fld [ebx].D3DXVECTOR2.y
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DstWnd
      @invoke DbgOutText, "]", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd

      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
      frstor [edi]
      add esp, 108                  ;;Restore stack
      DbgShowTxtInfo InfoText, %??DstWnd
      DbgShowSrcInfo %??DstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgVec3
; Purpose:   Outputs the components of the D3DXVECTOR3 vector.
; Arguments: Arg1: -> Vector3
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgVec3 macro Var:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      mov ebx, Var
      ifb <Dest>
        ??DstWnd textequ <offset szDbgSrc>
      else
        ??DstWnd textequ <Dest>
      endif
      @invoke DbgOutText, "&Var -\] [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd
      sub esp, 108                  ;;Reserve FPU storage. Keep stack aligned!
      mov edi, esp
      fnsave [edi]
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp

      fld [ebx].D3DXVECTOR3.x
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DstWnd
      @invoke DbgOutText, ",", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd

      fld [ebx].D3DXVECTOR3.y
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DstWnd
      @invoke DbgOutText, ",", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd

      fld [ebx].D3DXVECTOR3.z
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DstWnd
      @invoke DbgOutText, "]", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd

      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
      frstor [edi]
      add esp, 108                  ;;Restore stack
      DbgShowTxtInfo InfoText, %??DstWnd
      DbgShowSrcInfo %??DstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgVec4
; Purpose:   Outputs the components of the D3DXVECTOR4 vector.
; Arguments: Arg1: -> Vector4
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgVec4 macro Var:req, InfoText, Dest
    if DEBUGGING
      pushfd
      pushad
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      mov ebx, Var
      ifb <Dest>
        ??DstWnd textequ <offset szDbgSrc>
      else
        ??DstWnd textequ <Dest>
      endif
      @invoke DbgOutText, "&Var -\] [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd
      sub esp, 108                  ;;Reserve FPU storage. Keep stack aligned!
      mov edi, esp
      fnsave [edi]
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp

      fld [ebx].D3DXVECTOR4.x
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DstWnd
      @invoke DbgOutText, ",", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd

      fld [ebx].D3DXVECTOR4.y
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DstWnd
      @invoke DbgOutText, ",", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd

      fld [ebx].D3DXVECTOR4.z
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DstWnd
      @invoke DbgOutText, ",", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd

      fld [ebx].D3DXVECTOR4.w
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd
      @invoke DbgOutText, "]", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DstWnd

      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
      frstor [edi]
      add esp, 108                  ;;Restore stack
      DbgShowTxtInfo InfoText, %??DstWnd
      DbgShowSrcInfo %??DstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      popad
      popfd
    endif
endm
