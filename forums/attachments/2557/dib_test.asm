.386
.model flat, stdcall
option casemap:none

include /masm32/include/windows.inc
include /masm32/include/kernel32.inc
include /masm32/include/user32.inc
include /masm32/include/gdi32.inc
include /masm32/fpulib/fpu.inc
include macros.inc

includelib /masm32/lib/kernel32.lib
includelib /masm32/lib/user32.lib
includelib /masm32/lib/gdi32.lib
includelib /masm32/fpulib/fpu.lib

DlgProc PROTO :DWORD,:DWORD,:DWORD,:DWORD
InitStars PROTO
MoveStars PROTO
DrawStars PROTO
PutPixel PROTO :DWORD,:DWORD,:DWORD
_random PROTO :DWORD

.const
ScreenWidth		equ 400
ScreenHeight		equ 300
NUM_OF_STARS		equ 50
SS_SIZE			equ sizeof STAR

.data
RandSeed		dd 0A2F59C2Eh
			dd 05B2A10E9h

.data?

STAR STRUCT
  x dd ?
  y dd ?
  plane dd ?
STAR ENDS

canvasDC		dd ?
canvasBmp		dd ?
hDC			dd ?
canvas_buffer		dd ?
ps			PAINTSTRUCT <>
canvas			BITMAPINFO <>
rct			RECT <>
stars			STAR NUM_OF_STARS dup(<?>)
TheValue		REAL10 ?
dummy			dd ?

.code
los:

invoke GetModuleHandle,0
invoke DialogBoxParam,eax,105,0,addr DlgProc,0

;######################################################################################
DlgProc PROC hwndDlg:DWORD,uMsg:DWORD,wParam:DWORD,lParam:DWORD

.if uMsg == WM_INITDIALOG
    mov canvas.bmiHeader.biSize,sizeof canvas.bmiHeader
    mov canvas.bmiHeader.biWidth,ScreenWidth
    mov canvas.bmiHeader.biHeight,ScreenHeight
    mov canvas.bmiHeader.biPlanes,1
    mov canvas.bmiHeader.biBitCount,32

    invoke GetDC,hwndDlg
    mov hDC,eax
    invoke CreateCompatibleDC,hDC
    mov canvasDC,eax
    invoke CreateDIBSection,hDC,addr canvas,DIB_RGB_COLORS,addr canvas_buffer,0,0
    mov canvasBmp,eax
    invoke SelectObject,canvasDC,canvasBmp
    invoke ReleaseDC,hDC,0

    invoke InitStars
    invoke SetTimer,hwndDlg,0,100,0

.elseif uMsg == WM_TIMER
    mov edi,canvas_buffer			;only for
    mov ecx,ScreenWidth * ScreenHeight		;wiping away
    xor eax,eax					;old frame
    rep stosd					;that was drawn

;    invoke _random,ScreenWidth
;    mov ebx,eax
;    invoke _random,ScreenHeight
;    invoke PutPixel,ebx,eax,00FFFFFFh

;    invoke MoveStars				;here goes the drawing
    invoke DrawStars

    invoke RedrawWindow,hwndDlg,0,0,RDW_INVALIDATE

.elseif uMsg == WM_PAINT
    invoke BeginPaint,hwndDlg,addr ps
    invoke BitBlt,eax,0,0,ScreenWidth,ScreenHeight,canvasDC,0,0,SRCCOPY
    invoke EndPaint,hwndDlg,addr ps

.elseif uMsg == WM_CLOSE
    xor eax,eax
    invoke ExitProcess,0

.else
    xor eax,eax
    ret

.endif
mov eax,TRUE
ret

DlgProc ENDP
;######################################################################################
InitStars PROC USES ECX EBX

xor ebx,ebx
xor ecx,ecx
.while ecx < NUM_OF_STARS
    invoke _random,ScreenWidth
    mov [ebx+stars.x],eax
    invoke _random,ScreenHeight
    mov [ebx+stars.y],eax
    invoke _random,16
    mov [ebx+stars.plane],eax
    add ebx,SS_SIZE
    inc ecx
.endw
ret

InitStars ENDP
;######################################################################################
MoveStars PROC USES ECX EAX EBX

xor ecx,ecx
.while ecx < NUM_OF_STARS
     mov ebx,2
     mov eax,stars[ecx].plane
     div ebx
     sub stars[ecx].x,eax

;    invoke FpuDiv,stars[ecx].plane,2,addr TheValue,SRC1_DIMM or SRC2_DIMM or DEST_MEM
;    invoke FpuSub,stars[ecx].x,addr TheValue,addr TheValue,SRC1_DIMM or SRC2_REAL or DEST_MEM
;    invoke FpuRound,addr TheValue,stars[ecx].x,SRC1_REAL or DEST_IMEM

    .if stars[ecx].x < 0
        mov stars[ecx].x,ScreenWidth
        invoke _random,16
        mov stars[ecx].plane,eax
    .endif
    inc ecx
.endw
ret

MoveStars ENDP
;######################################################################################
DrawStars PROC USES ECX EAX EBX EDX EDI

xor ebx,ebx
xor ecx,ecx
.while ecx < NUM_OF_STARS
    invoke PutPixel,[ebx+stars.x],[ebx+stars.y],00FFFFFFh
    add ebx,SS_SIZE
    inc ecx
.endw    
ret

DrawStars ENDP
;######################################################################################
PutPixel PROC x:DWORD,y:DWORD,color:DWORD

.if x>=0 && x<ScreenWidth && y>=0 && y<ScreenHeight
    mov edi,canvas_buffer

    mov ecx,ScreenWidth
    mov eax,y
    mul ecx
    add eax,x

    mov ebx,color
    mov dword ptr [edi+eax*4],ebx

.endif
ret

PutPixel ENDP
;######################################################################################
_random         PROC USES EDI ESI EBX ECX Range:DWORD

        mov     ecx,Range      
        mov     eax,dWord ptr [RandSeed+4]
        mov     ebx,dWord ptr [RandSeed]
        mov     esi,eax
        mov     edi,ebx        
        mov     dl,ah
        mov     ah,al
        mov     al,bh
        mov     bh,bl
        xor     bl,bl
        rcr     dl,1        
        rcr     eax,1
        rcr     ebx,1
        add     ebx,edi
        adc     eax,esi
        add     ebx,03b1c62e9h
        adc     eax,04d8f3619h
        mov     dword ptr [RandSeed],ebx
        mov     dword ptr [RandSeed+4],eax
        xor     edx,edx
        div     ecx
        mov     eax,edx           
        jnz     _quit
        inc     eax
_quit:        
        ret
_random         ENDP
;######################################################################################
END los