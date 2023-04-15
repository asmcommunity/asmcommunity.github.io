;OpenGL/ObjAsm32 demo #1
;'The black window of doom'
;
;ESCAPE to quit
;F1 to toggle window/fullscreen

; ---------------------------------------------------------------------------
%include @Environ(OA32_PATH)\\Code\\Macros\\Model.inc   ;Include & initialize standard modules
SysSetup OOP_WINDOWS,DEBUG(WND);, RESGUARD)             ;Loads OOP files and OS related objects
; ---------------------------------------------------------------------------
WinMain proto :DWORD,:DWORD,:DWORD,:DWORD				;Main program loop
; ---------------------------------------------------------------------------
$inv equ $invoke				;I am SO LAZY (plus it looks tidier)
IncludeAPIs Opengl32,glu32		;I corrected a couple of Opengl32 prototypes to use REAL8 as intended
; ---------------------------------------------------------------------------
include OpenGL_Demo1.inc		;Data for this application
; ---------------------------------------------------------------------------



;Program EntryPoint
start:
	SysInit
	mov    CommandLine,$inv(GetCommandLine)	
	invoke WinMain, hInstance,NULL,CommandLine, SW_SHOWDEFAULT
	SysDone
	invoke ExitProcess,eax

;Call this whenever the window size changes
ReSizeGLScene proc  dwidth, dheight
LOCAL fAspectRatio:real8
	.if dheight==0				; Prevent A Divide By Zero By	
		mov dheight,1			; Making Height Equal One
	.endif

	invoke glViewport,0,0,dwidth,dheight	; Reset The Current Viewport
	invoke glMatrixMode,GL_PROJECTION		; Select The Projection Matrix
	invoke glLoadIdentity					; Reset The Projection Matrix

	; Calculate The Aspect Ratio Of The Window
	fild dwidth
	fidiv dheight
	fstp fAspectRatio
	invoke gluPerspective,r8_45,fAspectRatio,r8_1,r8_1000
	invoke glMatrixMode,GL_MODELVIEW
	invoke glLoadIdentity
	ret
ReSizeGLScene endp

;Release our RenderDC, WindowDC, Window and Class
KillGLWindow proc
	.if fullscreen
		invoke ChangeDisplaySettings,NULL,0
		invoke ShowCursor,TRUE	
	.endif

	.if hRC	
		.if !$invoke (wglMakeCurrent,NULL,NULL)		
			@invoke MessageBox,NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK or MB_ICONINFORMATION
		.endif

		.if !$invoke(wglDeleteContext,hRC)		
			@invoke MessageBox,NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK or MB_ICONINFORMATION
		.endif
		mov hRC,NULL
	.endif

	.if hDC && ($invoke(ReleaseDC,hWnd,hDC)==0)
		@invoke MessageBox,NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK or MB_ICONINFORMATION
		mov hDC,0
	.endif

	.if hWnd && ($invoke(DestroyWindow,hWnd)==0)
		@invoke MessageBox,NULL,"Could Not Release hWnd.","SHUTDOWN ERROR",MB_OK or MB_ICONINFORMATION
		mov hWnd,0
	.else
		DbgWarning "Window Closed"
	.endif

	@invoke UnregisterClass,"OpenGL",hInstance
	.if !eax	
		@invoke MessageBox,NULL,"Could Not Unregister Class.","SHUTDOWN ERROR",MB_OK or MB_ICONINFORMATION
		mov hInstance,0
	.endif
	ret
KillGLWindow endp

;Initialize our most basic render settings
InitGL proc
	invoke glEnable,GL_TEXTURE_2D;									 Enable Texture Mapping
	invoke glShadeModel,GL_SMOOTH;									 Enable Smooth Shading
	@invoke glClearColor,0,0,0, r4_half;							 Black Background
	invoke glClearDepth,r8_1;										 Depth Buffer Setup
	invoke glEnable,GL_DEPTH_TEST;									 Enables Depth Testing
	invoke glDepthFunc,GL_LEQUAL;									 The Type Of Depth Testing To Do
	invoke glHint,GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST;		 Really Nice Perspective Calculations
	return TRUE;													 Initialization Went OK
InitGL endp

;/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
; *	title			- Title To Appear At The Top Of The Window				*
; *	width			- Width Of The GL Window Or Fullscreen Mode				*
; *	height			- Height Of The GL Window Or Fullscreen Mode			*
; *	bits			- Number Of Bits To Use For Color (8/16/24/32)			*
; *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
CreateGLWindow proc psztitle, dwidth, dheight,  dbits,  fullscreenflag
LOCAL PixelFormat
local wc:WNDCLASS
local dwStyle,dwExStyle
LOCAL rc:RECT
LOCAL dmScreenSettings:DEVMODE
LOCAL pfd:PIXELFORMATDESCRIPTOR
	mov rc.left,0
	m2m rc.right,dwidth
	mov rc.top,0
	m2m rc.bottom,dheight
	m2m fullscreen,fullscreenflag

	invoke RtlZeroMemory,addr wc,sizeof wc
	mov wc.style, CS_HREDRAW or CS_VREDRAW or CS_OWNDC;		// Redraw On Size, And Own DC For Window.
	mov wc.lpfnWndProc	,offset WndProc
	mov wc.cbClsExtra	,0
	mov wc.cbWndExtra	,0
	m2m wc.hInstance	,hInstance
	mov wc.hIcon		,$inv(LoadIcon,NULL, IDI_WINLOGO);			// Load The Default Icon
	mov wc.hCursor		,$inv(LoadCursor,NULL, IDC_ARROW);			// Load The Arrow Pointer
	mov wc.hbrBackground,0
	mov wc.lpszMenuName	,0
	mov wc.lpszClassName,$OfsCStr("OpenGL")

	.if !$inv(RegisterClass,addr wc)
		@invoke MessageBox,NULL,"Failed To Register The Window Class.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE;												// Return FALSE
	.endif
		
	.if fullscreen	
		DbgText "yes"
		invoke RtlZeroMemory,addr dmScreenSettings,sizeof dmScreenSettings
		mov dmScreenSettings.dmSize,sizeof dmScreenSettings
		m2m dmScreenSettings.dmPelsWidth,dwidth
		m2m dmScreenSettings.dmPelsHeight,dheight
		m2m dmScreenSettings.dmBitsPerPel,dbits
		mov dmScreenSettings.dmFields,(DM_BITSPERPEL or DM_PELSWIDTH or DM_PELSHEIGHT)
		; Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		.if $inv(ChangeDisplaySettings,addr dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL		
			; If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			@invoke MessageBox,NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO or MB_ICONEXCLAMATION
			.if eax==IDYES
				mov fullscreen,FALSE
			.else
				; Pop Up A Message Box Letting User Know The Program Is Closing.
				@invoke MessageBox,NULL,"Program Will Now Close.","ERROR",MB_OK or MB_ICONSTOP
				return FALSE
			.endif
		.endif
	.endif
	
	.if (fullscreen)		; Are We Still In Fullscreen Mode?
		mov dwExStyle,WS_EX_APPWINDOW;									// Window Extended Style
		mov dwStyle,WS_POPUP;											// Windows Style
		invoke ShowCursor,FALSE;		 Hide Mouse Pointer
	.else
		mov dwExStyle,WS_EX_APPWINDOW or WS_EX_WINDOWEDGE;				// Window Extended Style
		mov dwStyle,WS_OVERLAPPEDWINDOW;								// Windows Style
	.endif

	invoke AdjustWindowRectEx,addr rc, dwStyle, FALSE, dwExStyle;		// Adjust Window To True Requested Size

	; Create The Window
	
	mov edx,dwStyle
	or edx,WS_CLIPSIBLINGS or WS_CLIPSIBLINGS
	invoke CreateWindowEx,	dwExStyle,$OfsCStr("OpenGL"),psztitle,edx,
								0, 0,
								dwidth,;rc.right-rc.left,	;// Calculate Window Width
								dheight,;rc.bottom-rc.top,	;// Calculate Window Height
								0,0,hInstance,0
	mov hWnd,eax							
	.if !eax
		invoke KillGLWindow
		@invoke MessageBox,NULL,"Window Creation Error.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif
	
	invoke RtlZeroMemory,addr pfd,sizeof pfd
	mov pfd.nSize,sizeof pfd
	mov pfd.nVersion,1
	mov pfd.dwFlags,PFD_DRAW_TO_WINDOW or PFD_SUPPORT_OPENGL or PFD_DOUBLEBUFFER
	mov pfd.iPixelType,PFD_TYPE_RGBA
	mov edx,dbits
	mov pfd.cColorBits,dl
	mov pfd.cDepthBits,16
	mov pfd.iLayerType,PFD_MAIN_PLANE
	
	mov hDC,$inv(GetDC,hWnd)
	.if !eax
		invoke KillGLWindow
		int 3
		@invoke MessageBox,NULL,"Can't Create A GL Device Context.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	mov PixelFormat,$inv(ChoosePixelFormat,hDC,addr pfd)
	.if !eax
		invoke KillGLWindow
		@invoke MessageBox,NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif
	
	.if !$inv(SetPixelFormat,hDC,PixelFormat,addr pfd)
		invoke KillGLWindow
		@invoke MessageBox,NULL,"Can't Set The PixelFormat.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	mov hRC,$inv(wglCreateContext,hDC)
	.if eax==0
		invoke KillGLWindow
		@invoke MessageBox,NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	.if $inv(wglMakeCurrent,hDC,hRC)==0
		invoke KillGLWindow
		@invoke MessageBox,NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	invoke ShowWindow,hWnd,SW_SHOW
	invoke SetForegroundWindow,hWnd
	invoke SetFocus,hWnd
	invoke ReSizeGLScene,dwidth, dheight

	.if !$inv(InitGL)
		invoke KillGLWindow
		@invoke MessageBox,NULL,"Initialization Failed.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	return TRUE
CreateGLWindow endp

;Render the Scene (one frame)
;We are going to draw a Triangle, and a Rectangle ('quad').
DrawGLScene proc
.data
  ;Let's define a 3D Vector as a set of three floats:
  Vec3 struct
  	x real4 ?
  	y real4 ?
  	z real4 ?
  Vec3 ends
  ;Lets define a Color with 3 components (RGB)
  RGB struct
  	red   real4 ?
  	green real4 ?
  	blue  real4 ?
  RGB ends
  ;Data for our Triangle and Quad  
  Translate1 Vec3 <-1.5f,0.0f,-6.0f>   ;Translation from (0,0,0) to origin of Triangle
  Translate2 Vec3 <3.0f,0.0f,0.0f>     ;Translation from origin of Triangle to origin of Square
  TriPoint1  Vec3 <0.0f, 1.0f, 0.0f>	;Three points make a Triangle
  TriPoint2  Vec3 <-1.0f,-1.0f, 0.0f>
  TriPoint3  Vec3 <1.0f,-1.0f, 0.0f>
  QuadPoint1 Vec3 <-1.0f, 1.0f, 0.0f>   ;Four points make a Quad
  QuadPoint2 Vec3 <1.0f, 1.0f, 0.0f>
  QuadPoint3 Vec3 <1.0f,-1.0f, 0.0f>
  QuadPoint4 Vec3 <-1.0f,-1.0f, 0.0f>
  ;Some colors
  Color1 RGB <1.0f,0.0f,0.0f>
  Color2 RGB <0.0f,1.0f,0.0f>
  Color3 RGB <0.0f,0.0f,1.0f>
  .code
	invoke glClear,GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT
	invoke glLoadIdentity										; Reset the current modelview matrix
	invoke glTranslatef,Translate1.x,Translate1.y,Translate1.z	; Move left 1.5 units and into the screen 6.0
	invoke glBegin,GL_TRIANGLES									; Drawing using triangles
		invoke glColor3f,Color1.red,Color1.green,Color1.blue
		invoke glVertex3f,TriPoint1.x,TriPoint1.y,TriPoint1.z	; Top
		invoke glColor3f,Color2.red,Color2.green,Color2.blue
		invoke glVertex3f,TriPoint2.x,TriPoint2.y,TriPoint2.z	; Bottom left
		invoke glColor3f,Color3.red,Color3.green,Color3.blue
		invoke glVertex3f,TriPoint3.x,TriPoint3.y,TriPoint3.z	; Bottom right
	invoke glEnd												; Finished Drawing The Triangle
	invoke glTranslatef,Translate2.x,Translate2.y,Translate2.z	; Move right 3 units
	invoke glBegin,GL_QUADS    									; Draw a quad
		invoke glColor3f,Color1.red,Color2.green,Color3.blue
		invoke glVertex3f,QuadPoint1.x,QuadPoint1.y,QuadPoint1.z; Top left
		invoke glColor3f,Color2.red,Color2.green,Color3.blue
		invoke glVertex3f,QuadPoint2.x,QuadPoint2.y,QuadPoint2.z; Top right
		invoke glColor3f,Color1.red,Color1.green,Color3.blue
		invoke glVertex3f,QuadPoint3.x,QuadPoint3.y,QuadPoint3.z; Bottom right
		invoke glColor3f,Color1.red,Color2.green,Color2.blue
		invoke glVertex3f,QuadPoint4.x,QuadPoint4.y,QuadPoint4.z; Bottom left
	invoke glEnd												; Finished Drawing The Quad
	return TRUE
DrawGLScene endp

;Our main program loop.. windows message pump !!
;Create our App Window for OpenGL Rendering..
;Then start pumping WM's until its time to die.
WinMain proc hInst:HINSTANCE,hPrevInst:HINSTANCE,CmdLine:LPSTR,CmdShow:DWORD
LOCAL msg:MSG


	
	; Ask The User Which Screen Mode They Prefer
	@invoke MessageBox,NULL,"Would You Like To Run In Fullscreen Mode?", "Start FullScreen?",MB_YESNOCANCEL or MB_ICONQUESTION
	.if eax==IDNO
		mov fullscreen,FALSE
	.elseif eax==IDCANCEL
		return FALSE
	.endif

	; Create Our OpenGL Window
	@invoke CreateGLWindow,"OpenGL Demo #1",640,480,16,fullscreen
	.if !eax
		DbgWarning "Window Create failed"
		return FALSE
	.endif
	
	.repeat
		.if $invoke(PeekMessage,addr msg,NULL,0,0,PM_REMOVE)		
			.if msg.message==WM_QUIT			
				.break
			.else
				invoke TranslateMessage,addr msg
				invoke DispatchMessage,addr msg
			.endif
		.else		
			; Draw The Scene.  Watch For ESC Key And Quit Messages From DrawGLScene()
			.if (active && !$invoke(DrawGLScene)) || keys[VK_ESCAPE]
				.break
			.else
				invoke SwapBuffers,hDC
			.endif

			.if keys[VK_F1]			
				mov keys[VK_F1],FALSE
				;Destroy the Window
				invoke KillGLWindow
				;Toggle fullscreen mode flag
				xor fullscreen,TRUE
				;Recreate the Window with new screen mode
				@invoke CreateGLWindow,"OpenGL Demo #1",640,480,16,fullscreen
				.if !eax
					DbgWarning "CreateGLWindow failed"
					.break
				.endif
			.endif
		.endif
	.until 0

	; Shutdown
	invoke KillGLWindow
	return msg.wParam
WinMain endp

;Application Window's Message Handler
;No suprises here, we watch for some keypresses and make sure nothing turns the screen off
WndProc proc hWin:HWND, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
	.switch uMsg
		.case WM_ACTIVATE
			mov eax,wParam
			shr eax,16
			.if !ax
				mov active,TRUE			
			.else			
				mov active,FALSE
			.endif
			return 0
		

		.case WM_SYSCOMMAND
			mov eax,wParam
			.if eax==SC_SCREENSAVE || eax==SC_MONITORPOWER
				return 0;Prevent From Happening
			.endif

		.case WM_CLOSE		
			invoke PostQuitMessage,0
			return 0

		.case WM_KEYDOWN			
			mov edx,wParam
			mov keys [edx] , TRUE
			return 0
		
		.case WM_KEYUP
			mov edx,wParam
			mov keys[edx],FALSE
			return 0

		.case WM_SIZE
			mov eax,lParam
			mov edx,eax
			shr edx,16
			and eax,0FFFFh
			invoke ReSizeGLScene,eax,edx
			return 0
	.endsw
	

	; Pass All Unhandled Messages To DefWindowProc
	invoke DefWindowProc,hWin,uMsg,wParam,lParam
	ret
WndProc endp

end start
