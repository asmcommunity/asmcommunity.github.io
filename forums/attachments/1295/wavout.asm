.486
.model flat,stdcall
option casemap:none

include /masm32/include/windows.inc
include /masm32/include/user32.inc
include /masm32/include/kernel32.inc
include /masm32/include/gdi32.inc
include /masm32/include/winmm.inc

includelib /masm32/lib/user32.lib
includelib /masm32/lib/kernel32.lib
includelib /masm32/lib/gdi32.lib
includelib /masm32/lib/winmm.lib


WinMain PROTO :DWORD,:DWORD,:DWORD,:DWORD
waud PROTO :DWORD, :LPSTR, :DWORD

.data
wav db "\\somefilename\\",0
cn db "SimpleWinClass",0
an db "Our First Window",0
rb dd 0

.data?
hInstance dd ?
cmdl LPSTR ?
wh dd ?
wfx WAVEFORMATEX <>
wdh WAVEHDR <>  

.code
start:

invoke GetModuleHandle, 0
mov hInstance, eax
invoke GetCommandLine
mov cmdl, eax

invoke WinMain, hInstance,NULL, cmdl, SW_SHOWDEFAULT
invoke ExitProcess, eax

WinMain proc hInst:HINSTANCE,hPrevInst:HINSTANCE,CmdLine:LPSTR,CmdShow:DWORD 
    LOCAL wc:WNDCLASSEX                                          
    LOCAL msg:MSG 
    LOCAL hwnd:HWND 

    mov   wc.cbSize,SIZEOF WNDCLASSEX                 
    mov   wc.style, CS_HREDRAW or CS_VREDRAW 
    mov   wc.lpfnWndProc, OFFSET WndProc 
    mov   wc.cbClsExtra,NULL 
    mov   wc.cbWndExtra,NULL 
    push  hInstance 
    pop   wc.hInstance 
    mov   wc.hbrBackground,COLOR_WINDOW+1 
    mov   wc.lpszMenuName,NULL 
    mov   wc.lpszClassName,OFFSET cn 
    invoke LoadIcon,NULL,IDI_APPLICATION 
    mov   wc.hIcon,eax 
    mov   wc.hIconSm,eax 
    invoke LoadCursor,NULL,IDC_ARROW 
    mov   wc.hCursor,eax 
    invoke RegisterClassEx, addr wc                      
    invoke CreateWindowEx,NULL,\ 
                ADDR cn,\ 
                0,\ 
                WS_OVERLAPPEDWINDOW,\ 
                CW_USEDEFAULT,\ 
                CW_USEDEFAULT,\ 
                CW_USEDEFAULT,\ 
                CW_USEDEFAULT,\ 
                NULL,\ 
                NULL,\ 
                hInst,\ 
                NULL 
    mov   hwnd,eax 
    invoke ShowWindow, hwnd,CmdShow               
    invoke UpdateWindow, hwnd                                 

    .WHILE TRUE                                                       
                invoke GetMessage, ADDR msg,NULL,0,0 
                .BREAK .IF (!eax) 
                invoke TranslateMessage, ADDR msg 
                invoke DispatchMessage, ADDR msg 
   .ENDW 
    mov     eax,msg.wParam                                            
    ret 
WinMain endp 


WndProc proc hWnd:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
LOCAL hf:DWORD
LOCAL siz1:DWORD
LOCAL hea:DWORD
LOCAL blo:DWORD

.IF uMsg==WM_DESTROY
invoke PostQuitMessage,NULL

.ELSEIF uMsg==WM_CREATE
mov wfx.nSamplesPerSec, 44100d
mov wfx.nChannels, 2
mov wfx.cbSize,0
mov wfx.wBitsPerSample, 16
mov eax, 16
shr eax,3
shl eax,1
mov wfx.nBlockAlign, ax
mov wfx.wFormatTag, WAVE_FORMAT_PCM
shl eax,4
mov wfx.nAvgBytesPerSec, eax
invoke waveOutOpen, ADDR wh, WAVE_MAPPER, ADDR wfx, 0, 0, CALLBACK_NULL

.if eax!=MMSYSERR_NOERROR 
invoke MessageBox,hWnd, ADDR an, ADDR an,0
.endif

invoke CreateFile, ADDR wav, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_EXISTING, 0,0
mov hf, eax
.if eax==ERROR_INVALID_HANDLE
invoke PostQuitMessage,0
.endif
.while(0)
invoke GetFileSize, hf, 0
mov siz1, eax
.if eax==0
invoke PostQuitMessage,0
.endif
invoke GetProcessHeap
mov hea, eax
invoke HeapAlloc, hea, 0, siz1
mov blo, eax
.if eax==0
invoke PostQuitMessage,0
.endif
invoke ReadFile, hf, blo, siz1, addr rb, 0
.endw
invoke CloseHandle, hf

invoke waud, wh, blo, siz1
invoke waveOutClose, wh

.ELSE
invoke DefWindowProc, hWnd, uMsg, wParam, lParam
ret
.ENDIF

xor eax,eax
ret
WndProc endp

waud proc wh1:DWORD, block:LPSTR, siz:DWORD
invoke RtlZeroMemory, ADDR wdh, sizeof wdh
push block
pop wdh.dwBufferLength
push siz
pop wdh.lpData
invoke waveOutPrepareHeader, wh, ADDR wdh, sizeof wdh
invoke waveOutWrite, wh, ADDR wdh, sizeof wdh
invoke Sleep, 500
invoke waveOutUnprepareHeader, wh, ADDR wdh, sizeof wdh

.if eax==WAVERR_STILLPLAYING
invoke Sleep, 100
.endif
ret
waud endp
end start