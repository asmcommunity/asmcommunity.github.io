;OpenGL Application Framework 2.0
;Leith Ketchell
;July, 2010
;
;Last Edit July 2010:	Embedded a Timer class object ('Clock'), and some real8 variables to track elapsed and app time
;						Implemented a FrameRate Capping scheme which distributes rendering over time.
ifndef Timer
	MakeObjects .\Timer
endif

.data
   
   ;Some constants, because passing floats isnt always easy
   r4_half real4 0.5f
   r4_1    real4 1.0f
   r4_15 real4 15.0f	;constant for spotlight cone radius
   r4_1000 real4 1000.0f
   
   r8_0	   real8 0.0f
   r8_1    real8 1.0f
   r8_45   real8 45.0f
   r8_75   real8 75.0f
   r8_1000 real8 1000.0f
   
   hWnd dd 0
   pGLW dd 0 ; Global Pointer to main application class : "GLWindow"

.code
  
Object GLWindow,345345345,Primer
	RedefineMethod Init, Pointer
	RedefineMethod Done
	
	StaticMethod Kill
	StaticMethod Create,LPSTR,dword,dword,dword,BOOL
	StaticMethod LoadTexture, LPSTR, BOOL, DWORD
	StaticMethod Run,BOOL
	
	StaticMethod LT_Print,HANDLE,LPSTR,dword,dword
	
	Private
	StaticMethod DrawScene
	StaticMethod ReloadTextures
	StaticMethod ReleaseTextures
	StaticMethod WndProc,dword,dword,dword	
	StaticMethod Resize,dword,dword
	PrivateEnd

	DynamicMethod User_DrawScene
	DynamicMethod User_Shutdown
	DynamicMethod User_ReloadTextures
	DynamicMethod User_ReleaseTextures
	
	;Appliction Window
	DefineVariable pszWindowName, 		LPSTR,  NULL
	DefineVariable dWindowWidth, 		dword,	1024
	DefineVariable dWindowHeight, 		dword,	768
	DefineVariable dColorBits, 			dword,	32
	DefineVariable hRC, 				HANDLE, NULL
	DefineVariable hDC, 				HANDLE, NULL
	DefineVariable fullscreen,			BOOL, 	FALSE
	DefineVariable active, 	   			BOOL, 	TRUE
	DefineVariable dUse_2D_Projection,	BOOL, 	FALSE		;Ortho2D or Persp3D

	DefineVariable DefaultTextureID,dword,NULL

	;Texture Filtering	
	DefineVariable CurrentFilter, dword, 0				;Which min/mag filter set we are using (0,1 or 2)
	DefineVariable MinFilter, dword, GL_LINEAR			;Current min/mag filter set
	DefineVariable MagFilter, dword, GL_LINEAR
	
	;Lighting
	DefineVariable AmbientLight, Color4, {<0.5f,0.5f,0.5f,1.0f>}			;Ambient white light, of mid strength
	DefineVariable DiffuseLight ,Color4, {<1.0f,1.0f,1.0f,1.0f>}			;Full intensity white light
	DefineVariable SpecularLight, Color4, {<1.0f,1.0f,1.0f,0.0f>}  			;Specular highlight
	DefineVariable PositionLight, Color4,  {<0.0f,0.0f,4.0f,1.0f>}			;Position light just behind the camera, facing outwards
	DefineVariable DirectionLight, Vec3, {<0.0f,0.0f,-1.0f>}
	DefineVariable fConeAngleLight, real4, 15.0
	
	
	DefineVariable keys,byte,256 dup <(?)>		;Keyboard array
	
	DefineVariable r8_ElapsedTime,real8,0.0		;time elapsed between iterations of main loop
	DefineVariable r8_AppTime,real8,0.0			;total application running time
	DefineVariable r8_FrameAccum,real8,0.0		;time accumulated between rendered frames
	DefineVariable dFPS_Clamp,dword,60			;Cap the rendering FPS (frames per second) to this rate
	Embed Clock, Timer							;High Performance Timer object
	
ObjectEnd

Method GLWindow.Done, uses esi
	SetObject esi
	OCall esi.ReleaseTextures
	OCall esi.Kill	
	OCall esi.User_Shutdown
	ACall Done
MethodEnd

Method GLWindow.User_Shutdown, uses esi
	SetObject esi
	DbgWarning "Dummy User_Shutdown method called"
MethodEnd

;Initialize our most basic render settings
Method GLWindow.Init,uses esi,pOwner
	SetObject esi

	ACall Init, pOwner
	OCall [esi].Clock::Timer.Init
	
	invoke glEnable,GL_TEXTURE_2D;									 Enable Texture Mapping
	invoke glShadeModel,GL_SMOOTH;									 Enable Smooth Shading
	@invoke glClearColor,0,0,0, r4_half;							 Black Background
	invoke glClearDepth,r8_1;										 Depth Buffer Setup
	invoke glEnable,GL_DEPTH_TEST;									 Enables Depth Testing
	invoke glDepthFunc,GL_LEQUAL;									 The Type Of Depth Testing To Do
	invoke glHint,GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST;		 Really Nice Perspective Calculations

	;Initialize Lighting
    invoke glLightfv,GL_LIGHT1, GL_AMBIENT, addr [esi].AmbientLight
    invoke glLightfv,GL_LIGHT1, GL_DIFFUSE, addr [esi].DiffuseLight
    invoke glLightfv,GL_LIGHT1, GL_SPECULAR,addr [esi].SpecularLight
    invoke glLightfv,GL_LIGHT1, GL_POSITION, addr [esi].PositionLight
    invoke glLightfv,GL_LIGHT1, GL_SPOT_DIRECTION, addr [esi].DirectionLight

    ;angle of the cone light emitted by the spot : value between 0 to 180 (degrees)
    invoke glLightf,GL_LIGHT1, GL_SPOT_CUTOFF, [esi].fConeAngleLight

    ;SpotLight Attenuation Properties:
    ;exponent property defines the concentration of the light
    invoke glLightf,GL_LIGHT1, GL_SPOT_EXPONENT, r4_15       
    ;light attenuation (default values used here : no attenuation with the distance)
    invoke glLightf,GL_LIGHT1, GL_CONSTANT_ATTENUATION, r4_1
    invoke glLightf,GL_LIGHT1, GL_LINEAR_ATTENUATION, 0
    invoke glLightf,GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0    
    
	invoke glEnable,GL_LIGHT1
	invoke glEnable,GL_LIGHTING
	
	mov eax, TRUE;	 Initialization Went OK
MethodEnd




;/*	This Code Creates Our OpenGL Window.  Parameters Are:					*
; *	title			- Title To Appear At The Top Of The Window				*
; *	width			- Width Of The GL Window Or Fullscreen Mode				*
; *	height			- Height Of The GL Window Or Fullscreen Mode			*
; *	dColorbits		- Number Of Bits To Use For Color (8/16/24/32)			*
; *	fullscreenflag	- Use Fullscreen Mode (TRUE) Or Windowed Mode (FALSE)	*/
 
Method GLWindow.Create,uses esi,psztitle:LPSTR, dwidth, dheight,  dColorbits,  fullscreenflag:BOOL
LOCAL PixelFormat
local wc:WNDCLASS
local dwStyle,dwExStyle
LOCAL rc:RECT
LOCAL dmScreenSettings:DEVMODE
LOCAL pfd:PIXELFORMATDESCRIPTOR

	SetObject esi
	m2m [esi].pszWindowName, psztitle, edx
	m2m [esi].dColorBits,dColorbits,edx
	m2m [esi].dWindowWidth,dwidth,edx
	m2m [esi].dWindowHeight,dheight,edx
	m2m [esi].fullscreen,fullscreenflag

	mov rc.left,0
	m2m rc.right,dwidth
	mov rc.top,0
	m2m rc.bottom,dheight

	invoke RtlZeroMemory,addr wc,sizeof wc
	mov wc.style, CS_HREDRAW or CS_VREDRAW or CS_OWNDC;		// Redraw On Size, And Own DC For Window.
	mov wc.lpfnWndProc	,$MethodAddr(GLWindow.WndProc)
	mov wc.cbClsExtra	,0
	mov wc.cbWndExtra	,0
	m2m wc.hInstance	,hInstance
	mov wc.hIcon		,$invoke(LoadIcon,  NULL, IDI_WINLOGO);			// Load The Default Icon
	mov wc.hCursor		,$invoke(LoadCursor,NULL, IDC_ARROW);			// Load The Arrow Pointer
	mov wc.hbrBackground,0
	mov wc.lpszMenuName	,0
	mov wc.lpszClassName,$OfsCStr("OpenGL")

	.if !$invoke(RegisterClass,addr wc)
		@invoke MessageBox,NULL,"Failed To Register The Window Class.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE;												// Return FALSE
	.endif
		
	.if [esi].fullscreen	
		;Adopt screen dimensions
		invoke GetDesktopWindow
		lea edx,rc
		invoke GetWindowRect,eax,edx
		m2m dwidth, rc.right
		m2m dheight, rc.bottom
		
		invoke RtlZeroMemory,addr dmScreenSettings,sizeof dmScreenSettings
		mov dmScreenSettings.dmSize,sizeof dmScreenSettings
		m2m dmScreenSettings.dmPelsWidth,dwidth
		m2m dmScreenSettings.dmPelsHeight,dheight
		m2m dmScreenSettings.dmBitsPerPel,dColorbits
		mov dmScreenSettings.dmFields,(DM_BITSPERPEL or DM_PELSWIDTH or DM_PELSHEIGHT)
		; Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
		.if $invoke(ChangeDisplaySettings,addr dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL		
			; If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
			@invoke MessageBox,NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?","NeHe GL",MB_YESNO or MB_ICONEXCLAMATION
			.if eax==IDYES
				mov [esi].fullscreen,FALSE
			.else
				; Pop Up A Message Box Letting User Know The Program Is Closing.
				@invoke MessageBox,NULL,"Program Will Now Close.","ERROR",MB_OK or MB_ICONSTOP
				return FALSE
			.endif
		.endif
	.endif
		
	.if ([esi].fullscreen)		; Are We Still In Fullscreen Mode?
		mov dwExStyle,WS_EX_APPWINDOW;									// Window Extended Style
		mov dwStyle,WS_POPUP;											// Windows Style
;		invoke ShowCursor,FALSE;		 Hide Mouse Pointer
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
		OCall esi.Kill
		@invoke MessageBox,NULL,"Window Creation Error.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif	
	
	invoke RtlZeroMemory,addr pfd,sizeof pfd
	mov pfd.nSize,sizeof pfd
	mov pfd.nVersion,1
	mov pfd.dwFlags,PFD_DRAW_TO_WINDOW or PFD_SUPPORT_OPENGL or PFD_DOUBLEBUFFER
	mov pfd.iPixelType,PFD_TYPE_RGBA
	mov edx,dColorbits
	mov pfd.cColorBits,dl
	mov pfd.cDepthBits,16
	mov pfd.iLayerType,PFD_MAIN_PLANE
	
	mov [esi].hDC,$invoke(GetDC,hWnd)
	.if !eax
		OCall esi.Kill
		@invoke MessageBox,NULL,"Can't Create A GL Device Context.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	mov PixelFormat,$invoke(ChoosePixelFormat,[esi].hDC,addr pfd)
	.if !eax
		OCall esi.Kill
		@invoke MessageBox,NULL,"Can't Find A Suitable PixelFormat.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif
	
	.if !$invoke(SetPixelFormat,[esi].hDC,PixelFormat,addr pfd)
		OCall esi.Kill
		@invoke MessageBox,NULL,"Can't Set The PixelFormat.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	mov [esi].hRC,$invoke(wglCreateContext,[esi].hDC)
	.if eax==0
		OCall esi.Kill
		@invoke MessageBox,NULL,"Can't Create A GL Rendering Context.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	.if $invoke(wglMakeCurrent,[esi].hDC,[esi].hRC)==0
		OCall esi.Kill
		@invoke MessageBox,NULL,"Can't Activate The GL Rendering Context.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	invoke ShowWindow,hWnd,SW_SHOW
	invoke SetForegroundWindow,hWnd
	invoke SetFocus,hWnd
	OCall esi.Resize,dwidth, dheight

	.if !$OCall(esi.Init,NULL)
		OCall esi.Kill
		@invoke MessageBox,NULL,"Initialization Failed.","ERROR",MB_OK or MB_ICONEXCLAMATION
		return FALSE
	.endif

	mov eax, TRUE
MethodEnd

;Release our RenderDC, WindowDC, Window and Class
Method GLWindow.Kill,uses esi
	SetObject esi
	.if [esi].fullscreen
		invoke ChangeDisplaySettings,NULL,0
		invoke ShowCursor,TRUE	
	.endif

	.if [esi].hRC	
		.if !$invoke (wglMakeCurrent,NULL,NULL)		
			@invoke MessageBox,NULL,"Release Of DC And RC Failed.","SHUTDOWN ERROR",MB_OK or MB_ICONINFORMATION
		.endif

		.if !$invoke(wglDeleteContext,[esi].hRC)		
			@invoke MessageBox,NULL,"Release Rendering Context Failed.","SHUTDOWN ERROR",MB_OK or MB_ICONINFORMATION
		.endif
		mov [esi].hRC,NULL
	.endif

	.if [esi].hDC && ($invoke(ReleaseDC,hWnd,[esi].hDC)==0)
		@invoke MessageBox,NULL,"Release Device Context Failed.","SHUTDOWN ERROR",MB_OK or MB_ICONINFORMATION
		mov [esi].hDC,0
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
MethodEnd

;Call this whenever the window size changes
Method GLWindow.Resize,uses esi,  dwidth, dheight
LOCAL fAspectRatio:real8
LOCAL fTop:real8, fRight:real8
	SetObject esi
	
	.if dheight==0				; Prevent A Divide By Zero By	
		mov dheight,1			; Making Height Equal One
	.endif

	invoke glViewport,0,0,dwidth,dheight	; Reset The Current Viewport
	invoke glMatrixMode,GL_PROJECTION		; Select The Projection Matrix
	invoke glLoadIdentity					; Reset The Projection Matrix

	fild dwidth
	fidiv dheight
	fstp fAspectRatio
	.if [esi].dUse_2D_Projection
		fild dwidth
		fstp fRight
		fild dheight
		fstp fTop
		invoke gluOrtho2D,r8_0, fRight, r8_0,fTop
		.data
		r8_m1 real8 -1.0f
		r8_50 real8 50.0f
		.code
	;	invoke glOrtho, r8_0,fRight  ,r8_0,fTop,   r8_m1,r8_1
	.else
		; Calculate The Aspect Ratio Of The Window
		invoke gluPerspective,r8_45,fAspectRatio,r8_0,r8_1000
		invoke glMatrixMode,GL_MODELVIEW
		invoke glLoadIdentity
		
	.endif
	

MethodEnd

; ---------------------------------------------------------------------------
;Helper function to create OpenGL Texture from an Image File
;szFileName = ptr to namestring
;ptexid = ptr to dword to receive ID of tex resource
;bWantMipMaps = whether or not to generate mipmaps for this texture
;dTransparencyType = 0 (none), 1 (black colorkey) or 2 (alpha brightness)
;Returns FALSE or textureid

Method GLWindow.LoadTexture,uses esi ebx, szFileName:LPSTR, bWantMipMaps:BOOL, dTransparencyType
LOCAL texid, pxm
	SetObject esi
	;Make sure path exists
	.if $invoke(GetFileAttributes,szFileName) !=-1
		DbgStr szFileName
		mov texid,0
		invoke glGenTextures,1, addr texid
		.if texid!=0		
			;Bind to (ie 'select') the newly-generated Texture immediately
			;All subsequent texture operations apply to the currently bound texture
			invoke glBindTexture,GL_TEXTURE_2D, texid
			
			;Create a temporary Pixelmap object
			mov pxm,$New(Pixelmap)
			;Load image file
			OCall pxm::Pixelmap.LoadFile,NULL,szFileName			

			;Some debug
			mov edx,pxm
			DbgDec [edx].Pixelmap.dWidth
			DbgDec [edx].Pixelmap.dHeight
			DbgHex [edx].Pixelmap.pPixels
			DbgDec [edx].Pixelmap.BmpInfo.bmiHeader.biBitCount
								
			;Set up texture parameters
			invoke glPixelStorei,GL_UNPACK_ALIGNMENT, 4; Pixel Storage Mode (Word Alignment / 4 Bytes)
			invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR;	// Linear Min Filter
			invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR;	// Linear Mag Filter

			;Copy pixel data to texture, and generate mipmaps if required
			mov edx,pxm
			movzx eax,[edx].Pixelmap.BmpInfo.bmiHeader.biBitCount
			.if eax==32						;If image is 32 bits 
				.if dTransparencyType!=0	;If User wants some kind of alpha channel data generated
					mov eax,[edx].Pixelmap.pPixels
					xor ebx,ebx
					.while ebx<[edx].Pixelmap.dWidth
						push ebx
						xor ebx,ebx
						.while ebx<[edx].Pixelmap.dHeight
							;If the pixel is not totally black
							.if dword ptr[eax]!=0
								.if dTransparencyType==1
									;Make pixel completely opaque
									or dword ptr[eax],0FF000000h
								.else
									;Make pixel opacity = average rgb intensity (darker pixels are more transparent)
									push eax
									
									mov eax,[eax]
									xor edx,edx
									mov dl,al
									shr eax,8
									push eax
									and eax,255
									add edx,eax
									pop eax
									shr eax,8								
									add eax,edx								
									xor edx,edx
									mov ecx,3
									div ecx
									mov edx,eax
									shl edx,24
									
									pop eax
									or dword ptr[eax],edx
								.endif
							.endif
							add eax,4
							inc ebx	
							mov edx,pxm				
						.endw
						pop ebx
						inc ebx
					.endw
				.endif			
				;Generate OpenGL texture from pixel data
				invoke glTexImage2D,GL_TEXTURE_2D, 0, 4, [edx].Pixelmap.dWidth, [edx].Pixelmap.dHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, [edx].Pixelmap.pPixels
				.if bWantMipMaps
					mov edx,pxm
					invoke gluBuild2DMipmaps,GL_TEXTURE_2D, 4, [edx].Pixelmap.dWidth, [edx].Pixelmap.dHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, [edx].Pixelmap.pPixels
				.endif
				
			.elseif eax==24
				.if dTransparencyType!=0
					DbgWarning "Error - cannot generate alpha data for 24bit image - need more code"
					int 3
				.endif
				invoke glTexImage2D,GL_TEXTURE_2D, 0, 3, [edx].Pixelmap.dWidth, [edx].Pixelmap.dHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, [edx].Pixelmap.pPixels
				.if bWantMipMaps
					mov edx,pxm
					invoke gluBuild2DMipmaps,GL_TEXTURE_2D, 3, [edx].Pixelmap.dWidth, [edx].Pixelmap.dHeight, GL_RGB, GL_UNSIGNED_BYTE, [edx].Pixelmap.pPixels
				.endif		
			.else
				DbgWarning "Pixel format not supported"
				Destroy pxm
				return FALSE
			.endif
			
			;Cleanup pixelmap
			Destroy pxm
			
			;Return texture id
			mov eax,texid
		.else
			@invoke MessageBox,0,"Failed glGenTextures","Error",MB_OK+MB_ICONERROR
			mov eax, FALSE
		.endif
	.else
		DbgWarning "Error - Failed to locate imagefile"
		DbgStr szFileName
		mov eax,FALSE
	.endif
MethodEnd

;Modified version of LoadTexture
;Create a texture which contains text, printed using a FONT (possibly TrueType !!)
;The quality depends on the font size (chosen when loaded).
;Size of output texture depends on text content and font quality.
Method GLWindow.LT_Print,uses esi ebx, hFont, pText, dColor, dTransparencyType
LOCAL texid, hBmp
LOCAL rc:RECT, pt:POINT
LOCAL hDc,pm	

	SetObject esi
	mov texid,0
	invoke glGenTextures,1, addr texid
	.if texid!=0		

		;Bind to (ie 'select') the newly-generated Texture immediately
		;All subsequent texture operations apply to the currently bound texture
		invoke glBindTexture,GL_TEXTURE_2D, texid
		
		;---
		;Obtain the dimensions of the printed text, in the context of the main device:		
		;-Temporarily select the font into the main DC (preserve old font)
		push $invoke  (SelectObject,[esi].hDC,hFont)
		;-Measure the rect size of the printed text, IF we printed it to the main DC
		invoke SetBkMode,[esi].hDC,TRANSPARENT
		invoke DrawTextEx,[esi].hDC,pText,-1,addr rc,DT_CALCRECT,0
		;-Restore the old font
		pop eax
		invoke  SelectObject,[esi].hDC,eax
		;---
		
		
		;Create a temp DC, and a bitmap of RECT size
		mov hDc,$invoke  (CreateCompatibleDC,[esi].hDC)
		mov hBmp,$invoke (CreateCompatibleBitmap,[esi].hDC,rc.right,rc.bottom)

		;Select the Font and the BMP into the temp DC
		invoke  SelectObject,hDc,hBmp
		invoke  SelectObject,hDc,hFont	
		
		;Render the text to the DC (and thus into the BMP)
		invoke  SetBkMode,hDc,TRANSPARENT
		invoke SetBkColor,hDc,0
		invoke SetTextColor,hDc,dColor
		invoke  DrawText,hDc,pText,-1,addr rc,DT_SINGLELINE

		;Load the BMP into a Pixelmap
		mov pm, $New(Pixelmap,LoadBmp,NULL,hDc,hBmp)
		
		;Release temp DC and BMP
		invoke  DeleteDC,hDc
		invoke  DeleteObject,hBmp
		
		;From now on, perform 'texture loading' as usual
		;(all remaining code is duplicated from regular texture load method)
		
		;Some debug
		DbgWarning "Texture created from Printed Text"
		mov edx,pm
		DbgDec [edx].Pixelmap.dWidth
		DbgDec [edx].Pixelmap.dHeight
		DbgHex [edx].Pixelmap.pPixels
		DbgDec [edx].Pixelmap.BmpInfo.bmiHeader.biBitCount
							
		;Set up texture parameters
		invoke glPixelStorei,GL_UNPACK_ALIGNMENT, 4; Pixel Storage Mode (Word Alignment / 4 Bytes)
		invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR;	// Linear Min Filter
		invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR;	// Linear Mag Filter

		;Copy pixel data to texture, and generate mipmaps if required
		mov edx,pm
		movzx eax,[edx].Pixelmap.BmpInfo.bmiHeader.biBitCount
		.if eax==32						;If image is 32 bits 
			.if dTransparencyType!=0	;If User wants some kind of alpha channel data generated
				mov eax,[edx].Pixelmap.pPixels
				xor ebx,ebx
				.while ebx<[edx].Pixelmap.dWidth
					push ebx
					xor ebx,ebx
					.while ebx<[edx].Pixelmap.dHeight
						;If the pixel is not totally black
						.if dword ptr[eax]!=0
							.if dTransparencyType==1
								;Make pixel completely opaque
								or dword ptr[eax],0FF000000h
							.else
								;Make pixel opacity = average rgb intensity (darker pixels are more transparent)
								push eax
								
								mov eax,[eax]
								xor edx,edx
								mov dl,al
								shr eax,8
								push eax
								and eax,255
								add edx,eax
								pop eax
								shr eax,8								
								add eax,edx								
								xor edx,edx
								mov ecx,3
								div ecx
								mov edx,eax
								shl edx,24
								
								pop eax
								or dword ptr[eax],edx
							.endif
						.endif
						add eax,4
						inc ebx	
						mov edx,pm				
					.endw
					pop ebx
					inc ebx
				.endw
			.endif			
			;Generate OpenGL texture from pixel data
			invoke glTexImage2D,GL_TEXTURE_2D, 0, 4, [edx].Pixelmap.dWidth, [edx].Pixelmap.dHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, [edx].Pixelmap.pPixels
			
		.elseif eax==24
			.if dTransparencyType!=0
				DbgWarning "Error - cannot generate alpha data for 24bit image - need more code"
				int 3
			.endif
			invoke glTexImage2D,GL_TEXTURE_2D, 0, 3, [edx].Pixelmap.dWidth, [edx].Pixelmap.dHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, [edx].Pixelmap.pPixels
		.else
			DbgWarning "Pixel format not supported"
			Destroy pm
			return FALSE
		.endif
		
		;Cleanup pixelmap
		Destroy pm
		
		;Return texture id
		mov eax,texid
	.else
		@invoke MessageBox,0,"Failed glGenTextures","Error",MB_OK+MB_ICONERROR
		mov eax, FALSE
	.endif

MethodEnd


;Load all textures used by this demo
;Call this if we just (re)created the app window
Method GLWindow.ReloadTextures,uses esi
	SetObject esi
	OCall esi.User_ReloadTextures
MethodEnd

;Release all textures used by this demo
;Call this if the app window is killed
Method GLWindow.ReleaseTextures,uses esi	
	SetObject esi
	OCall esi.User_ReleaseTextures
MethodEnd

Method GLWindow.User_ReloadTextures,,
MethodEnd

Method GLWindow.User_ReleaseTextures,,
MethodEnd

Method GLWindow.DrawScene, uses esi
	SetObject esi
	invoke glClear,GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT
	invoke glMatrixMode,GL_MODELVIEW
	invoke glLoadIdentity	; Reset the modelview matrix
	OCall esi.User_DrawScene
MethodEnd

;This dummy method is overridden with user's callback render routine
Method GLWindow.User_DrawScene, uses esi
	SetObject esi
	DbgWarning "Dummy User_DrawScene method"
MethodEnd

;hWin is being passed silently in ESI, so 'this' is not available :|
Method GLWindow.WndProc,uses esi, uMsg:UINT, wParam:WPARAM, lParam:LPARAM
	SetObject esi
	.switch uMsg
		.case WM_ACTIVATE
			mov eax,wParam
			shr eax,16
			mov edx,pGLW
			.if !ax
				mov [edx].GLWindow.active,TRUE			
			.else			
				mov [edx].GLWindow.active,FALSE
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
			mov eax,pGLW
			mov [eax].GLWindow.keys [edx] , TRUE
			return 0
		
		.case WM_KEYUP
			mov edx,wParam
			mov eax,pGLW
			mov [eax].GLWindow.keys[edx],FALSE

			;Change filter if user released the F key
			;(we may get several keydown messages, but only one keyup message)
			.if edx==VK_F
				mov edx,pGLW				
				mov eax,[edx].GLWindow.CurrentFilter
				inc eax
				xor edx,edx
				mov ecx,3
				div ecx
				mov eax,pGLW
				mov [eax].GLWindow.CurrentFilter,edx
				.if edx==0
					mov [eax].GLWindow.MinFilter,GL_LINEAR
					mov [eax].GLWindow.MagFilter,GL_LINEAR
				.elseif edx==1
					mov [eax].GLWindow.MinFilter,GL_NEAREST
					mov [eax].GLWindow.MagFilter,GL_NEAREST
				.else
					mov [eax].GLWindow.MinFilter,GL_LINEAR_MIPMAP_NEAREST
					mov [eax].GLWindow.MagFilter,GL_LINEAR
				.endif				
				invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, [eax].GLWindow.MinFilter
				mov eax,pGLW
				invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, [eax].GLWindow.MagFilter
			.endif
			return 0

		.case WM_SIZE
			mov eax,lParam
			mov edx,eax
			shr edx,16
			and eax,0FFFFh
			OCall pGLW::GLWindow.Resize,eax,edx
			return 0
		.case WM_PAINT
			OCall pGLW::GLWindow.DrawScene
	.endsw
	

	; Pass All Unhandled Messages To DefWindowProc
	invoke DefWindowProc,esi,uMsg,wParam,lParam
MethodEnd

;Implements the Window MessageLoop
;dStayOnTop flag determines App Window behavior - see comments
Method GLWindow.Run, uses esi, dStayOnTop:BOOL
LOCAL msg:MSG
	SetObject esi
	
	.if dStayOnTop==TRUE			;Our application will remain 'on top' (at all times)
		mov eax,HWND_TOPMOST
	.else
		mov eax,HWND_TOP			;Our application will start 'on top' (but might not remain there)
	.endif
	invoke SetWindowPos,hWnd,eax,0,0,0,0,SWP_NOSIZE or SWP_NOMOVE
	
	invoke glGetString,GL_VERSION
	.if byte ptr[eax]<"3"
		DbgWarning "Warning - Your OpenGL driver is OLD - you should update it."
		DbgStr eax,"Current OpenGL driver version"
		@invoke MessageBox,0,"WARNING","Your OpenGL driver is OLD - you should update it.",MB_ICONEXCLAMATION
	.endif


	;Make sure we start by loading our default texture(s)
	OCall esi.ReloadTextures
	
	;Now enter the Main Loop (MessagePump)
	.repeat
		.if $invoke(PeekMessage,addr msg,NULL,0,0,PM_REMOVE)		
			.if msg.message==WM_QUIT			
				.break
			.else
				invoke TranslateMessage,addr msg
				invoke DispatchMessage,addr msg
			.endif
		.else

			;Note the application total time
			OCall [esi].Clock::Timer.Get,APPTIME
			fstp [esi].r8_AppTime
			
			;Note the elapsed time between loop iterations
			;Here we will implement "framerate cap"
			OCall [esi].Clock::Timer.Get,ELAPSEDTIME
			fst [esi].r8_ElapsedTime
			fadd [esi].r8_FrameAccum						;Accumulate elapsed time in this counter
			fst [esi].r8_FrameAccum
			fld1
			fidiv [esi].dFPS_Clamp
			fsubr
			fstpReg eax
			.ifBitSet eax,BIT31 ;Has enough time passed since we last rendered a frame?	
				
				;Yes - subtract one timestep from Accum, and render one Frame			
				fld [esi].r8_FrameAccum
				fld1
				fidiv [esi].dFPS_Clamp	
				fsub			
				fstp [esi].r8_FrameAccum
		

				; Draw The Scene.  Watch For Quit Messages from DrawScene
				.if ([esi].active && !$OCall(esi.DrawScene))
					.break
				.else
					invoke SwapBuffers,[esi].hDC
				.endif
	
			.else
				invoke Sleep,20
			.endif

			;Check for escape key
			.break .if [esi].keys[VK_ESCAPE]
			
			;Check for F1 key(toggle screen mode)
			.if [esi].keys[VK_F1]
				mov [esi].keys[VK_F1],FALSE
				OCall esi.ReleaseTextures
				OCall esi.Kill
				;Toggle fullscreen mode flag
				xor [esi].fullscreen,TRUE
				;Recreate the Window with new screen mode
				OCall esi.Create,[esi].pszWindowName,[esi].dWindowWidth,[esi].dWindowHeight,[esi].dColorBits,[esi].fullscreen
				.if !eax
					DbgWarning "CreateGLWindow failed"
					.break
				.endif
				OCall esi.ReloadTextures
			.endif
			
			
		.endif
	.until 0	
MethodEnd


