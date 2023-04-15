.386
.model flat,stdcall

include \masm32\include\windows.inc
include \masm32\include\user32.inc
include \masm32\include\kernel32.inc
include \masm32\include\gdi32.inc
include \masm32\include\advapi32.inc

includelib \masm32\lib\advapi32.lib
includelib \masm32\lib\user32.lib
includelib \masm32\lib\kernel32.lib
includelib \masm32\lib\gdi32.lib
includelib \masm32\lib\advapi32.lib


.data

ClassName       db "SimpleWinClass",0
subkeyname      db "Software\"
AppName         db "Regi32",0
num             db "%lu",0

stringvalue     db "Written by",0
regtext1        db "John Lyons" ,0
regstring2      db "Number of times executed",0
regerror        db "Error accessing Registry",0

regkey          DWORD ?
counter         DWORD ?

times           DWORD ?


hInstance       HINSTANCE ?
CommandLine     LPSTR ?
hwnd            HWND ?

buff            db 200 dup(0)

.code

WinMain proc hInst:HINSTANCE,hPrevInst:HINSTANCE,CmdLine:LPSTR,CmdShow:SDWORD
   LOCAL wc:WNDCLASSEX
   LOCAL msg:MSG

   mov   wc.cbSize,SIZEOF WNDCLASSEX
   mov   wc.style, CS_HREDRAW or CS_VREDRAW
   mov   wc.lpfnWndProc, OFFSET WndProc
   mov   wc.cbClsExtra,NULL
   mov   wc.cbWndExtra,NULL
        push  hInstance
        pop   wc.hInstance
   mov   wc.hbrBackground,COLOR_WINDOW+1
   mov   wc.lpszMenuName,NULL
   mov   wc.lpszClassName,OFFSET ClassName
        invoke LoadIcon,NULL,IDI_APPLICATION

   mov   wc.hIcon,eax
        mov   wc.hIconSm,0
        invoke LoadCursor,NULL,IDC_ARROW

   mov   wc.hCursor,eax
        invoke RegisterClassEx, addr wc

        INVOKE CreateWindowEx,NULL,ADDR ClassName,ADDR AppName,\
           WS_OVERLAPPEDWINDOW,
           CW_USEDEFAULT,CW_USEDEFAULT,           ;postion
          300,100,            ;size
          NULL,NULL,\
           hInst,NULL
        mov   hwnd,eax


        INVOKE ShowWindow, hwnd,SW_SHOWNORMAL
        INVOKE UpdateWindow, hwnd

        .WHILE TRUE
                INVOKE GetMessage, ADDR msg,NULL,0,0
                .BREAK .IF (!eax)
                INVOKE TranslateMessage, ADDR msg
                INVOKE DispatchMessage, ADDR msg
        .ENDW
        mov     eax,msg.wParam
        ret
WinMain endp
WndProc proc hWnd:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
        LOCAL hdc:HDC
        LOCAL ps:PAINTSTRUCT
        LOCAL Disp  :DWORD
        LOCAL pKey  :DWORD
        LOCAL Temp  :DWORD

   mov   eax,uMsg
   .IF eax==WM_DESTROY
      invoke PostQuitMessage,NULL

        .ELSEIF eax==WM_CREATE
;----------------------------------------------------------------------------
                        ;Create first key, who wrote the program

                invoke RegCreateKeyEx, HKEY_LOCAL_MACHINE,
                                     addr subkeyname, NULL, NULL,
                                     REG_OPTION_NON_VOLATILE,
                                     KEY_ALL_ACCESS, NULL,
                                     addr pKey, addr Disp

                cmp eax,ERROR_SUCCESS
                jne okok

                invoke RegSetValueEx, pKey, ADDR stringvalue,
                                      NULL, REG_SZ,
                                      ADDR regtext1, SIZEOF stringvalue

                cmp eax,ERROR_SUCCESS
                jne okok

                invoke RegCloseKey, pKey

;----------------------------------------------------------------------------
                        ;Get the value of "number of time exectued" from the reg

                invoke RegCreateKeyEx, HKEY_LOCAL_MACHINE,
                                         ADDR subkeyname, NULL, NULL,
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS, NULL,
                                         addr pKey, addr Disp
                cmp eax,ERROR_SUCCESS
                jne okok

                mov Temp,4

                invoke RegQueryValueEx, pKey, ADDR regstring2,
                                         NULL, ADDR Disp,
                                         ADDR times, ADDR Temp
                cmp eax,ERROR_SUCCESS
                jne okok4

                invoke RegCloseKey, pKey

okok4:
                inc [times]

                        ;Set the new time to the key "number of times executed"

                invoke RegCreateKeyEx, HKEY_LOCAL_MACHINE,
                                         ADDR subkeyname, NULL, NULL,
                                         REG_OPTION_NON_VOLATILE,
                                         KEY_ALL_ACCESS, NULL,
                                         addr pKey, addr Disp
                cmp eax,ERROR_SUCCESS
                jne okok

                invoke RegSetValueEx, pKey, ADDR regstring2,
                                         NULL, REG_DWORD_LITTLE_ENDIAN,
                                         ADDR times, Temp
                cmp eax,ERROR_SUCCESS
                jne okok

                invoke RegCloseKey, pKey

                jmp okok2
okok:
                mov [times],-1
okok2:


        .ELSEIF eax==WM_PAINT
                invoke BeginPaint,hWnd, ADDR ps
                mov    hdc,eax

                cmp times,-1
                jne okok3

                invoke TextOut,hdc,0,0,ADDR regerror,(SIZEOF regerror) -1

                jmp endit
okok3:

                invoke TextOut,hdc,0,0,ADDR regstring2,(SIZEOF regstring2) -1

                push times
                push offset num
                push offset buff
                call wsprintfA
                add esp,0ch
                lea esi,buff
                call count
                invoke TextOut,hdc,175,0,esi,counter
endit:
                invoke EndPaint,hWnd, ADDR ps

        .ELSE
                invoke DefWindowProc,hWnd,uMsg,wParam,lParam
                ret
   .ENDIF
        xor    eax,eax
   ret
WndProc endp
count proc
        push esi
           mov eax,0
noc2:
          cmp [esi],byte ptr 0
          je noc1
          inc esi
          inc eax
          jmp noc2
noc1:
        mov [counter],eax

        pop esi

        ret
count endp

start:

   invoke GetModuleHandle, NULL
   mov    hInstance,eax
   invoke GetCommandLine
      invoke WinMain, hInstance,NULL,CommandLine, SW_SHOWDEFAULT
   invoke ExitProcess,eax

end start
 
 
 
