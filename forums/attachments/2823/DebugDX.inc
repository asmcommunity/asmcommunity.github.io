 ; ==================================================================================================
; Title:   DebugDX.inc
; Author:  G. Friedrich
; Version: 1.0.1
; Purpose: Debug macros for DirectX.
; Notes:   Version 1.0.0, September 2005
;            - First release.
;          Version 1.0.1, March 2006
;            - Local macro variables redefined as global symbols.
;          Version 1.0.1, February 2007
;            - DbgMaterial added.
;            - FPU stack corruption corrected.
;            - DbgMatrix4 added.
;            - Symbolic arguments are now allowed.
; ==================================================================================================

;Supported debugging macros:
;———————————————————————————
;  - DbgVec2, DbgVec3, DbgVec4
;  - DbgPlane, DbgQuaternion, DbgMatrix4
;  - DbgMaterial


DbgPlane      equ DbgVec4
DbgQuaternion equ DbgVec4


; ==================================================================================================
;   DirectX debugging macros
; ==================================================================================================


; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgVec2
; Purpose:   Outputs the components of the D3DXVECTOR2 vector.
; Arguments: Arg1: -> D3DXVECTOR2
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgVec2 macro Var:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      DbgSetDestWnd Dest
      if type(Var) eq 4
        mov ebp, Var
        @invoke DbgOutText, "&Var -\] [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        lea ebp, Var
        @invoke DbgOutText, "&Var = [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif
      sub esp, 108                  ;;Reserve FPU storage. Keep stack aligned!
      mov edi, esp
      fnsave [edi]
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp

      fld [ebx].D3DXVECTOR2.x
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXVECTOR2.y
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

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
; Macro:     DbgVec3
; Purpose:   Outputs the components of the D3DXVECTOR3 vector.
; Arguments: Arg1: -> D3DXVECTOR3
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgVec3 macro Var:req, InfoText, Dest
    if DEBUGGING
      push edi
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      DbgSetDestWnd Dest
      if type(Var) eq 4
        mov ebp, Var
        @invoke DbgOutText, "&Var -\] [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        lea ebp, Var
        @invoke DbgOutText, "&Var = [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif

      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp

      fld [ebp].D3DXVECTOR3.x
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebp].D3DXVECTOR3.y
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebp].D3DXVECTOR3.z
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
            
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
      pop edi
    endif
endm

DbgVec3_Double macro Var:req, InfoText, Dest
    if DEBUGGING
      push edi
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      DbgSetDestWnd Dest
      if type(Var) eq 4
        mov ebp, Var
        @invoke DbgOutText, "&Var -\] [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        lea ebp, Var
        @invoke DbgOutText, "&Var = [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif

      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp

      fld real8 ptr[ebp]
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld real8 ptr[ebp+8]
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld real8 ptr [ebp+16]
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
            
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
      pop edi
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgVec4
; Purpose:   Outputs the components of the D3DXVECTOR4 vector.
; Arguments: Arg1: -> D3DXVECTOR4
;            Arg2: Additional information.
;            Arg3: Destination Window name.
DbgVec4 macro Var:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      DbgSetDestWnd Dest
      if type(Var) eq 4
        mov ebp, Var
        @invoke DbgOutText, "&Var -\] [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        lea ebp, Var
        @invoke DbgOutText, "&Var = [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif     
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp

      fld [ebp].D3DXVECTOR3.x
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebp].D3DXVECTOR3.y
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebp].D3DXVECTOR3.z
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebp].D3DXVECTOR4.w
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
             
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgMaterial
; Purpose:   Outputs the components of the D3DMATERIAL9 structure.
; Arguments: Arg1: -> Vector4
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgMaterial macro Var:req, InfoText, Dest
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
      if type(Var) eq 4
        mov ebx, Var
      else
        lea ebx, Var
      endif
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp

      @invoke DbgOutText, "Material values:", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd

      if type(Var) eq 4
        @invoke DbgOutText, "&Var -\] D3DMATERIAL9.Diffuse  [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        @invoke DbgOutText, "&Var = D3DMATERIAL9.Diffuse  [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif
      fld [ebx].D3DMATERIAL9.Diffuse.r
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Diffuse.g
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fstp st(0)
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Diffuse.b
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Diffuse.a
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd

      if type(Var) eq 4
        @invoke DbgOutText, "&Var -\] D3DMATERIAL9.Ambient  [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        @invoke DbgOutText, "&Var = D3DMATERIAL9.Ambient  [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif
      fld [ebx].D3DMATERIAL9.Ambient.r
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Ambient.g
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Ambient.b
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Ambient.a
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd

      if type(Var) eq 4
        @invoke DbgOutText, "&Var -\] D3DMATERIAL9.Specular [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        @invoke DbgOutText, "&Var = D3DMATERIAL9.Specular [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif
      fld [ebx].D3DMATERIAL9.Specular.r
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Specular.g
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Specular.b
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Specular.a
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd

      if type(Var) eq 4
        @invoke DbgOutText, "&Var -\] D3DMATERIAL9.Emissive [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        @invoke DbgOutText, "&Var = D3DMATERIAL9.Emissive [", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif
      fld [ebx].D3DMATERIAL9.Emissive.r
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(255,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Emissive.g
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,128,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Emissive.b
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,255), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DMATERIAL9.Emissive.a
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd

      if type(Var) eq 4
        @invoke DbgOutText, "&Var -\] D3DMATERIAL9.Power =", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      else
        @invoke DbgOutText, "&Var = D3DMATERIAL9.Power =", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      endif
      fld [ebx].D3DMATERIAL9.Power
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgMatrix4
; Purpose:   Outputs the components of the D3DXMATRIX.
; Arguments: Arg1: -> D3DXMATRIX
;            Arg2: Additional information.
;            Arg3: Destination Window name.

DbgMatrix4 macro Var:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      PushAll eax, ecx, edx
      invoke EnterCriticalSection, addr DbgCritSect
      PopAll edx, ecx, eax
      DbgSetDestWnd Dest

      @invoke DbgOutText, "Matrix4 values:", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd

      if type(Var) eq 4
        mov ebx, Var
        @invoke DbgOutText, "&Var -\]", $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd
      else
        lea ebx, Var
        @invoke DbgOutText, "&Var =", $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd
      endif
      sub esp, DBG_FLOAT_BUFFERSIZE ;;Reserve a buffer for the number. Keep stack aligned!
      mov esi, esp

      ;-------- 0 --------

      @invoke DbgOutText, offset szDbgBracketLeft, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DXMATRIX.m00
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m01
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m02
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m03
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd

      ;-------- 1 --------

      @invoke DbgOutText, offset szDbgBracketLeft, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DXMATRIX.m10
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m11
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m12
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m13
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd

      ;-------- 2 --------

      @invoke DbgOutText, offset szDbgBracketLeft, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DXMATRIX.m20
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m21
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m22
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m23
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd

      ;-------- 3 --------

      @invoke DbgOutText, offset szDbgBracketLeft, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      fld [ebx].D3DXMATRIX.m30
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m31
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m32
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgComma, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd

      fld [ebx].D3DXMATRIX.m33
      invoke St0ToStr, esi, 0, DBG_FLOAT_RESOLUTION, 1
      fstp st(0)
      @invoke DbgOutText, esi, $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      @invoke DbgOutText, offset szDbgBracketRight, $RGB(0,0,0), DBG_EFFECT_NEWLINE, %??DbgDstWnd

      add esp, DBG_FLOAT_BUFFERSIZE ;;Restore stack
      invoke LeaveCriticalSection, addr DbgCritSect
      DbgLoadContext
    endif
endm

DbgMat4 equ DbgMatrix4

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgD3DShowRS
; Purpose:   Outputs all render states.
; Arguments: Arg1: -> Device
;            Arg2: Text information.
;            Arg3: Destination Window name.

DbgD3DShowRS macro pDevice:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      DbgSetDestWnd Dest
      ifdifi <pDevice>, <ebx>
        mov ebx, pDevice
      endif
      invoke EnterCriticalSection, offset DbgCritSect
      @invoke DbgOutText, "Current Render States", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      @invoke DbgD3DOutRS, ebx, %??DbgDstWnd
      invoke LeaveCriticalSection, offset DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgD3DShowTSS
; Purpose:   Outputs all texture stage states.
; Arguments: Arg1: -> Device
;            Arg2: Texture stage.
;            Arg3: Text information.
;            Arg4: Destination Window name.

DbgD3DShowTSS macro pDevice:req, dStage:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      DbgSetDestWnd Dest
      ifdifi <pDevice>, <ebx>
        mov ebx, pDevice
      endif
      ifdifi <dStage>, <edi>
        mov edi, dStage
      endif
      invoke EnterCriticalSection, offset DbgCritSect
      @invoke DbgOutText, "Current Texture Stage States", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      @invoke DbgD3DOutTSS, ebx, edi, %??DbgDstWnd
      invoke LeaveCriticalSection, offset DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgD3DShowSS
; Purpose:   Outputs all sampler states.
; Arguments: Arg1: -> Device
;            Arg2: Sampler stage.
;            Arg3: Text information.
;            Arg4: Destination Window name.

DbgD3DShowSS macro pDevice:req, dStage:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      DbgSetDestWnd Dest
      ifdifi <pDevice>, <ebx>
        mov ebx, pDevice
      endif
      ifdifi <dStage>, <edi>
        mov edi, dStage
      endif
      invoke EnterCriticalSection, offset DbgCritSect
      @invoke DbgOutText, "Current Sampler States", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      @invoke DbgD3DOutSS, ebx, edi, %??DbgDstWnd
      invoke LeaveCriticalSection, offset DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgD3DSaveRSTS
; Purpose:   Saves all render, sampler and texture stage states.
; Arguments: Arg1: -> Device
;            Arg2: -> Buffer.
;            Arg3: Text information.
;            Arg4: Destination Window name.

DbgD3DSaveRSTS macro pDevice:req, pBuffer:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      DbgSetDestWnd Dest
      ifdifi <pDevice>, <ebx>
        mov ebx, pDevice
      endif
      ifdifi <pBuffer>, <edi>
        mov edi, pBuffer
      endif
      invoke EnterCriticalSection, offset DbgCritSect
      @invoke DbgOutText, "Saving Render, Sampler and Texture Stage States", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      @invoke DbgD3DStoreRSTS, ebx, edi, %??DbgDstWnd
      invoke LeaveCriticalSection, offset DbgCritSect
      DbgLoadContext
    endif
endm

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Macro:     DbgD3DChangedRSTS
; Purpose:   Shows all render, sampler and texture stage states that changed.
; Arguments: Arg1: -> Device
;            Arg2: -> Buffer.
;            Arg3: Text information.
;            Arg4: Destination Window name.

DbgD3DChangedRSTS macro pDevice:req, pBuffer:req, InfoText, Dest
    if DEBUGGING
      DbgSaveContext
      DbgSetDestWnd Dest
      ifdifi <pDevice>, <ebx>
        mov ebx, pDevice
      endif
      ifdifi <pBuffer>, <esi>
        mov esi, pBuffer
      endif
      invoke EnterCriticalSection, offset DbgCritSect
      @invoke DbgOutText, "Comparing Render, Sampler and Texture Stage States", $RGB(0,0,0), DBG_EFFECT_NORMAL, %??DbgDstWnd
      DbgShowTxtInfo InfoText, %??DbgDstWnd
      DbgShowSrcInfo %??DbgDstWnd
      @invoke DbgD3DCompareRSTS, ebx, esi, %??DbgDstWnd
      invoke LeaveCriticalSection, offset DbgCritSect
      DbgLoadContext
    endif
endm
