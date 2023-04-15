.586
.model flat, stdcall
option casemap:none

include \masm32\include\windows.inc
include \masm32\include\opengl32.inc
include \masm32\include\glu32.inc
include \masm32\include\ogl\kernel32.inc
include \masm32\include\ogl\user32.inc
include \masm32\include\ogl\gdi32.inc
include \masm32\include\WinExtra\winextra.def

includelib \masm32\lib\opengl32.lib
includelib \masm32\lib\glu32.lib
includelib \masm32\lib\kernel32.lib
includelib \masm32\lib\user32.lib
includelib \masm32\lib\gdi32.lib

CreateGLWindow  PROTO :DWORD,:DWORD,:DWORD,:DWORD,:BYTE
WinMain         PROTO :DWORD,:DWORD,:DWORD,:DWORD
WndProc         PROTO :DWORD,:DWORD,:DWORD,:DWORD
ReSizeGLScene   PROTO :DWORD,:DWORD
KillGLWindow    PROTO
InitGL          PROTO
DrawGLScene     PROTO

DM_BITSPERPEL equ 00040000h
PFD_DRAW_TO_WINDOW equ 00000004h
PFD_DOUBLEBUFFER equ 00000001h
PFD_TYPE_RGBA equ 0
PFD_MAIN_PLANE equ 0
DM_PELSWIDTH equ 00080000h
PFD_SUPPORT_OPENGL equ 00000020h
DM_PELSHEIGHT equ 00100000h

.DATA
our_title       db "Masm32 conversion of Nehe's tuts, (s)",0
ClassName       db "asmOpenGL",0
error           db "Error",0
noDC_RC         db "Release of DC and RC failed.",0
RCrelease       db "Release rendering context failed.",0
DCrelease       db "Release device context failed.",0
hWndRelease     db "Could not release hWnd.",0
register_error  db "Failed to register the window class.",0
not_full        db "The requested fullscreen mode is not supported by",13,10
                db "your video card. Use windowed mode instead ?",0
create_device   db "Can't create a GL device context.",0
create_pixel    db "Can't find a suitable pixelformat.",0
close           db "Program will now close",0
create_error    db "Window creation error.",0
pixel_error     db "Can't set the pixelformat.",0
rend_error      db "Can't create a GL rendering context.",0
act_error       db "Can't activate the GL rendering context.",0
init_error      db "Initialization Failed.",0
start_full      db "Would you like to run in fullscreen mode ?",0
class_error     db "Could not unregister class.",0

_45d0	equ 40468000h ;45.0
_45d1   equ 0	
_01d0	equ 1069128089		 
_01d1   equ -1717986918 ;0.1
_100d0	equ 1079574528
_100d1  equ 0           ;100.0
_1d0	equ 1072693248
_1d1    equ 0           ;1.0
_05	    equ 1056964608  ; 0.5
_1	    equ 1065353216  ; 1.0
_m1	    equ -1082130432 ;-1.0
_3	    equ 1077936128  ; 3.0
_m1d5   equ -1077936128 ;-1.5
_m6	    equ -1061158912 ;-6.0

.data?
    hDC         HDC ?
    hRC         DWORD ?
    hInstance   HANDLE ?
    keys        db 256 dup (?)
    active      db ?
    fullscreen  db ?
    hWnd        dd ?
.code
start:
    invoke GetModuleHandle, 0
    mov hInstance, eax
    invoke WinMain, eax, 0, 0, 0
    invoke ExitProcess, eax
    
WinMain proc hInst:HWND, hPrevInst:UINT, CmdLine:WPARAM, CmdShow:LPARAM
  LOCAL msg:MSG
    invoke MessageBox, 0, ADDR start_full, ADDR our_title, MB_YESNO or MB_ICONQUESTION
    .if eax == IDNO
        mov fullscreen, 0
    .else
        mov fullscreen, 1
    .endif
    invoke CreateGLWindow, ADDR our_title, 640, 480, 16, fullscreen
    .if (!eax)
        ret
    .endif
    .while TRUE
        invoke PeekMessage, ADDR msg,NULL,0,0,PM_REMOVE
        .if eax
            .if msg.message == WM_QUIT
                .break
            .else
                invoke TranslateMessage, ADDR msg
                invoke DispatchMessage, ADDR msg
            .endif
        .else
            .if keys[VK_ESCAPE]
                .break
            .elseif keys[VK_F1]
                mov keys[VK_F1], 0
                invoke KillGLWindow
                .if fullscreen
                    mov fullscreen, 0
                .else
                    mov fullscreen, 1
                .endif
                invoke CreateGLWindow, ADDR our_title, 640,480,16,fullscreen
                .if (!eax)
                    ret
                .endif
            .else
                invoke DrawGLScene
                invoke SwapBuffers, hDC
            .endif
        .endif
    .endw
    invoke KillGLWindow
    mov eax, msg.wParam
    ret
WinMain endp

WndProc proc hWin:DWORD, uMsg:DWORD, wParam:DWORD, lParam:DWORD
    .if uMsg == WM_ACTIVATE
        mov eax, wParam
        .if !ah
            mov active, 1
        .else
            mov active, 0
        .endif
    .elseIF uMsg == WM_SYSCOMMAND
        .if (wParam == SC_SCREENSAVE) || (wParam == SC_MONITORPOWER)
            xor eax, eax
            ret
        .endif
    .elseIF uMsg == WM_KEYDOWN
        mov eax, wParam
        mov keys[eax], 1
    .elseIF uMsg == WM_KEYUP
        mov eax, wParam
        mov keys[eax], 0
    .elseIF uMsg == WM_SIZE
        mov eax, lParam
        mov ebx, eax
        and eax, 0ffffh
        shr ebx, 16
        invoke ReSizeGLScene, eax, ebx
    .elseIF uMsg == WM_CLOSE
        invoke PostQuitMessage, 0
        xor eax, eax
        ret
    .endif
    invoke DefWindowProc, hWin, uMsg, wParam, lParam
    ret
WndProc endp

InitGL proc         ;opengl setup (like color, etc...)
    invoke glShadeModel, GL_SMOOTH              ;enable smooth shading
    invoke glClearDepth, _1d1, _1d0             ;depth buffer setup
    invoke glEnable, GL_DEPTH_TEST              ;enable depth testing
    invoke glDepthFunc, GL_LEQUAL               ;type of depth test
    invoke glHint, GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST ;set really nice perspective calculations
    mov eax, TRUE
    ret
InitGL endp

CreateGLWindow proc letitle:DWORD,w:DWORD,h:DWORD,bits:DWORD,flag:BYTE
  LOCAL PixelFormat:DWORD, wc:WNDCLASS
  LOCAL dwExStyle:DWORD, dwStyle:DWORD
  LOCAL rect:RECT, dmScreenSettings:DEVMODE
  LOCAL pfd:PIXELFORMATDESCRIPTOR
    mov rect.left, 0
    push w
    pop rect.right
    mov rect.top, 0
    push h
    pop rect.bottom
    mov al, flag
    mov fullscreen, al
    mov wc.style, CS_HREDRAW or CS_VREDRAW or CS_OWNDC
    mov wc.lpfnWndProc, offset WndProc
    mov wc.cbClsExtra, 0
    mov wc.cbWndExtra, 0
    push hInstance
    pop wc.hInstance
    mov wc.hIcon, 0
    invoke LoadCursor, 0, IDC_ARROW
    mov wc.hCursor, eax
    mov wc.lpszMenuName, 0
    mov wc.lpszClassName, offset ClassName
    mov wc.hbrBackground, 0
    invoke RegisterClass, ADDR wc
    .if (!eax)
        invoke MessageBox, 0, ADDR register_error, ADDR error, MB_OK or MB_ICONEXCLAMATION
        xor eax, eax
        ret
    .endif
    .if (fullscreen)
        ZeroMemory &dmScreenSettings, sizeof(dmScreenSettings)
        mov dmScreenSettings.dmSize, sizeof dmScreenSettings
        push w
        pop dmScreenSettings.dmPelsWidth
        push h
        pop dmScreenSettings.dmPelsHeight
        push bits
        pop dmScreenSettings.dmBitsPerPel
        mov dmScreenSettings.dmFields, DM_BITSPERPEL or DM_PELSWIDTH or DM_PELSHEIGHT
        invoke ChangeDisplaySettings, ADDR dmScreenSettings, CDS_FULLSCREEN
        .if eax != DISP_CHANGE_SUCCESSFUL
            invoke MessageBox, 0, ADDR not_full, ADDR error, MB_YESNO or MB_ICONEXCLAMATION
            .if eax == IDYES
                mov fullscreen, 0
            .else
                invoke MessageBox, 0, ADDR close, ADDR error, MB_OK or MB_ICONSTOP
                xor eax, eax
                ret
            .endif
        .endif
    .endif
    .if (fullscreen)
        mov dwExStyle, WS_EX_APPWINDOW
        mov dwStyle, WS_POPUP or WS_CLIPSIBLINGS or WS_CLIPCHILDREN
        invoke ShowCursor, 0
    .else
        mov dwExStyle, WS_EX_APPWINDOW or WS_EX_WINDOWEDGE
        mov dwStyle, WS_OVERLAPPEDWINDOW or WS_CLIPSIBLINGS or WS_CLIPCHILDREN
    .endif
    invoke AdjustWindowRectEx, ADDR rect, dwStyle, 0, dwExStyle
    mov eax, rect.left
    sub rect.right, eax
    mov eax, rect.top
    sub rect.bottom, eax
    invoke CreateWindowEx, dwExStyle, ADDR ClassName, ADDR our_title, \
                           dwStyle,0,0, rect.right, rect.bottom, 0, 0, hInstance, 0
    mov hWnd, eax
    .if (!eax)
        invoke KillGLWindow
        invoke MessageBox, 0, ADDR create_error, ADDR error, MB_OK or MB_ICONEXCLAMATION
        xor eax, eax
        ret
    .endif
    ZeroMemory &pfd, sizeof(PIXELFORMATDESCRIPTOR)
    mov pfd.nSize, sizeof PIXELFORMATDESCRIPTOR
    mov pfd.nVersion, 1
    mov pfd.dwFlags, PFD_DRAW_TO_WINDOW or PFD_SUPPORT_OPENGL or PFD_DOUBLEBUFFER
    mov pfd.iPixelType, PFD_TYPE_RGBA
    mov eax, bits
    mov pfd.cColorBits, al
    mov pfd.cDepthBits, 16
    mov pfd.dwLayerMask, PFD_MAIN_PLANE
    invoke GetDC, hWnd
    mov hDC, eax
    .if (!eax)
        invoke KillGLWindow
        invoke MessageBox, 0, ADDR create_device, ADDR error, MB_OK or MB_ICONEXCLAMATION
        xor eax, eax
        ret
    .endif
    invoke ChoosePixelFormat, hDC, ADDR pfd
    mov PixelFormat, eax
    .if (!eax)
        invoke KillGLWindow
        invoke MessageBox, 0, ADDR create_pixel, ADDR error, MB_OK or MB_ICONEXCLAMATION
        xor eax, eax
        ret
    .endif
    invoke SetPixelFormat, hDC, PixelFormat, ADDR pfd
    .if (!eax)
        invoke KillGLWindow
        invoke MessageBox, 0, ADDR pixel_error, ADDR error, MB_OK or MB_ICONEXCLAMATION
        xor eax, eax
        ret
    .endif
    invoke wglCreateContext, hDC
    mov hRC, eax
    .if (!eax)
        invoke KillGLWindow
        invoke MessageBox, 0, ADDR rend_error, ADDR error, MB_OK or MB_ICONEXCLAMATION
        xor eax, eax
        ret
    .endif
    invoke wglMakeCurrent, hDC, hRC
    .if (!eax)
        invoke KillGLWindow
        invoke MessageBox, 0, ADDR act_error, ADDR error, MB_OK or MB_ICONEXCLAMATION
        xor eax, eax
        ret
    .endif
    invoke ShowWindow, hWnd, SW_SHOW
    invoke SetForegroundWindow, hWnd
    invoke SetFocus, hWnd
    invoke ReSizeGLScene, w, h
    invoke InitGL
    .if (!eax)
        invoke KillGLWindow
        invoke MessageBox, 0, ADDR init_error, ADDR error, MB_OK or MB_ICONEXCLAMATION
        xor eax, eax
        ret
    .endif
    mov eax, TRUE
    ret
CreateGLWindow endp

DrawGLScene proc                                        ;drawing proc
    invoke glClear, GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT ;clear screen and depth buffer
    invoke glLoadIdentity                                      ;reset modelview matrix
    invoke glTranslatef, _m1d5, 0, _m6
    invoke glBegin, GL_TRIANGLES
        invoke glVertex3f, 0, _1, 0
        invoke glVertex3f, _m1, _m1, 0
        invoke glVertex3f, _1, _m1, 0
    invoke glEnd
    invoke glTranslatef, _3, 0, 0
    invoke glBegin, GL_QUADS
        invoke glVertex3f, _m1, _1, 0
        invoke glVertex3f, _1, _1, 0
        invoke glVertex3f, _1, _m1, 0
        invoke glVertex3f, _m1, _m1, 0
    invoke glEnd
    mov eax, TRUE
    ret
DrawGLScene endp

ReSizeGLScene proc w:DWORD, h:DWORD             ;resize scene
  LOCAL ratio:DWORD
    .if h == 0
        mov h, 1                                ;prevent div by zero
    .endif
    invoke glViewport, 0, 0, w, h               ;reset viewport
    invoke glMatrixMode, GL_PROJECTION          ;select projection matrix
    invoke glLoadIdentity                       ;reset it
    fild w
    fild h
    fdivp st(1), st ;width/height
    fstp ratio
    invoke gluPerspective,_45d1,_45d0,dword ptr [ratio],dword ptr [ratio+4],_01d1,_01d0,_100d1,_100d0
    invoke glMatrixMode, GL_MODELVIEW           ;select modelview matrix
    invoke glLoadIdentity                       ;reset it
    ret
ReSizeGLScene endp

KillGLWindow proc
    .if fullscreen
        invoke ChangeDisplaySettings, 0, 0          ;get back to window
        invoke ShowCursor, 1                        ;and show mouse ptr
    .endif
    .if hRC
        invoke wglMakeCurrent, 0, 0
        .if !eax
            invoke MessageBox, 0, ADDR noDC_RC, ADDR error, MB_OK or MB_ICONINFORMATION
        .endif
        invoke wglDeleteContext, hRC
        .if !eax
            invoke MessageBox, 0, ADDR RCrelease, ADDR error, MB_OK or MB_ICONINFORMATION
        .endif
        mov hRC, NULL
    .endif
    invoke ReleaseDC, hWnd, hDC
    .if (hDC && (!eax))
        invoke MessageBox, 0, ADDR DCrelease, ADDR error, MB_OK or MB_ICONINFORMATION
        mov hDC, NULL
    .endif
    invoke DestroyWindow, hWnd
    .if (hWnd && (!eax))
        invoke MessageBox, 0, ADDR hWndRelease, ADDR error, MB_OK or MB_ICONINFORMATION
        mov hWnd, NULL
    .endif
    invoke UnregisterClass, ADDR ClassName, hInstance
    .if (!eax)
        invoke MessageBox, 0, ADDR class_error, ADDR error, MB_OK or MB_ICONINFORMATION
    .endif
    ret
KillGLWindow endp
END start