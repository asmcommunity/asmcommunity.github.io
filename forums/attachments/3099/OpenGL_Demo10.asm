;OpenGL/ObjAsm32 demo #10
;TextureMapping with Filtering and Simple Lighting
;
;ESCAPE to quit
;F1 to toggle window/fullscreen
;F to change between 3 sets of min/mag filters

; ---------------------------------------------------------------------------
%include @Environ(OA32_PATH)\\Code\\Macros\\Model.inc   ;Include & initialize standard modules
SysSetup OOP_WINDOWS,DEBUG(WND);, RESGUARD)             ;Loads OOP files and OS related objects
; ---------------------------------------------------------------------------
WinMain proto :DWORD,:DWORD,:DWORD,:DWORD				;Main program loop
; ---------------------------------------------------------------------------
$inv equ $invoke				;I am SO LAZY (plus it looks tidier)
IncludeAPIs Opengl32,glu32,oleaut32		;I corrected a couple of Opengl32 prototypes to use REAL8 as intended
; ---------------------------------------------------------------------------
include OpenGL_Demo1.inc		;Data for this application
; ---------------------------------------------------------------------------

;I am depending on Pixelmap to do my imagefile loading
LoadObjects Pixelmap

  ;This is the color format we'll use for Lighting and Material attributes
  Color4 struct
	r real4 ?
	g real4 ?
	b real4 ?
	a real4 ?
  Color4 ends

  ;Let's define a 2D Vector as a set of two floats:
  Vec2 struct
  	u real4 ?
  	v real4 ?
  Vec2 ends

  ;Let's define a 3D Vector as a set of three floats:
  Vec3 struct
  	x real4 ?
  	y real4 ?
  	z real4 ?
  Vec3 ends


.data
MyTextureID dd 0				;Our only texture id
FilterChanged BOOL FALSE		;Tell render thread to switch min/mag filters
CurrentFilter dd 0				;Which min/mag filter set we are using (0,1 or 2)
MinFilter dd GL_LINEAR			;Current min/mag filter set
MagFilter dd GL_LINEAR

AmbientLight Color4 <0.5f,0.5f,0.5f,1.0f>			;Ambient white light, of mid strength
DiffuseLight Color4 <1.0f,1.0f,1.0f,1.0f>			;Full intensity white light
SpecularLight Color4 <1.0f,1.0f,1.0f,0.0f>  		;Specular highlight
PositionLight Color4  <0.0f,0.0f,-2.0f,1.0f>		;Position light just behind the camera, facing outwards

  ;Data for our Triangle and Quad  
  Translate1 Vec3 <0.0f,0.0f,-4.0f>   ;Translation from (0,0,0) to origin of Triangle
  Translate2 Vec3 <+1.5f,0.0f,-6.0f>   ;Translation from (0,0,0) to origin of Square


;Coordinates for equilateral tatrahedron with edge length of 2 units
;I adjusted the Z coords to centralize the Origin
;    ( 0, Sqr(3) - 1/Sqr(3),            0)
;    (-1,        - 1/Sqr(3),            0)
;    ( 1,        - 1/Sqr(3),            0)
;    ( 0,        0,                     2 * Sqr(2/3))

  ;Vertices for a Tetrahedron centered on its Origin (ie rotates about its own centroid)
  TriPoint1  Vec3 <0.0f,  1.154700538379251529018297561003f, 0.0f> 
  TriPoint2  Vec3 <-1.0f,-0.57735026918962576450914878050225f, -0.57735026918962576450914878050225f> 
  TriPoint3  Vec3 <1.0f ,-0.57735026918962576450914878050225f, -0.57735026918962576450914878050225f>
  TriPoint4  Vec3 <0.0f,0.0f,1.154700538379251529018297561003f> 
  TriUV1     Vec2 <0.5f,0.0f>			;Texture Coords for each Vertex
  TriUV2     Vec2 <0.0f,1.0f>
  TriUV3     Vec2 <1.0f,1.0f>
  TriUV4     Vec2 <0.5f,0.5f>
  FaceNorm1  Vec3 <0.0f,0.0f,0.0f>	;we'll calculate these once
  FaceNorm2  Vec3 <0.0f,0.0f,0.0f>
  FaceNorm3  Vec3 <0.0f,0.0f,0.0f>
  FaceNorm4  Vec3 <0.0f,0.0f,0.0f>

  ;Rotation
  fRotation_Triangle real4 0.0f					;Angle to rotate, in degrees
  vAxis_Triangle     Vec3 <1.0f,1.0f,0.0f>		;Axis of Rotation
  fRotSpeed_Triangle real4 1.2f					;Amount to increment rotation angle per frame


.code


;Program EntryPoint
start:
	SysInit
	mov    CommandLine,$inv(GetCommandLine)	
	invoke WinMain, hInstance,NULL,CommandLine, SW_SHOWDEFAULT
	SysDone
	invoke ExitProcess,eax
	

;Calculate the CrossProduct of two Vectors.
;This macro will return a 3D Vector on the FPU stack
;It works for real4, real8 and real10 floats.
;X: ST(2) = v1y * v2z - v1z * v2y
;Y: ST(1) = v1z * v2x - v1x * v2z
;Z: ST(0) = v1x * v2y - v1y * v2x
CrossProduct macro v1,v2
	fld  v1.y
	fmul v2.z
	fld  v1.z
	fmul v2.y
	fsub
	
	fld  v1.z
	fmul v2.x
	fld  v1.x
	fmul v2.z
	fsub
	
	fld  v1.x
	fmul v2.y
	fld  v1.y
	fmul v2.x
	fsub
endm
Vector_Cross equ CrossProduct

;Vector Subtraction: fpu stack = vA - vB
Vector_Sub macro vA,vB
	fld vA.x
	fsub vB.x
	fld vA.y
	fsub vB.y
	fld vA.z
	fsub vB.z
endm

;Store a Vector from the FPU Stack, popping it from the stack also
Vector_Stow macro Output
	fstp Output.z
	fstp Output.y
	fstp Output.x
endm

;Function to calculate a SurfaceNormal using Vec3 precision
;Output = ptr to output vector
;PointA, PointB, PointC = 3 consecutive points from a Face 
CalcSurfaceNormal proc uses ecx Output,PointA, PointB, PointC
LOCAL vAB:Vec3	;Vector of Edge AB
LOCAL vBC:Vec3	;Vector of Edge AC
	mov eax,PointA
	mov ecx,PointB
	mov edx,PointC
	Vector_Sub  [ecx].Vec3,[eax].Vec3		;Calculate Edge Vector AB
	Vector_Stow vAB							;vAB = PointB - PointA
	Vector_Sub  [eax].Vec3,[edx].Vec3		;Calculate Edge Vector BC
	Vector_Stow vBC							;vAC = PointC - PointB
	CrossProduct vAB,vBC					;Calculate vAB x vBC
	mov edx,Output
	Vector_Stow [edx].Vec3					;Store result
	ret
CalcSurfaceNormal endp	
	
	
; ---------------------------------------------------------------------------
;Helper function to create OpenGL Texture from an Image File
;szFileName = ptr to namestring
;ptexid = ptr to dword to receive ID of tex resource
;bWantMipMaps = whether or not to generate mipmaps for this texture
;Returns FALSE or textureid

LoadTexture proc szFileName, bWantMipMaps:BOOL
LOCAL texid, pxm
	;Make sure path exists
	.if $inv(GetFileAttributes,szFileName) !=-1
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
			.if eax==32
				invoke glTexImage2D,GL_TEXTURE_2D, 0, 4, [edx].Pixelmap.dWidth, [edx].Pixelmap.dHeight, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, [edx].Pixelmap.pPixels
				.if bWantMipMaps
					mov edx,pxm
					invoke gluBuild2DMipmaps,GL_TEXTURE_2D, 4, [edx].Pixelmap.dWidth, [edx].Pixelmap.dHeight, GL_BGRA_EXT, GL_UNSIGNED_BYTE, [edx].Pixelmap.pPixels
				.endif		
			.elseif eax==24
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
	ret
LoadTexture endp

;Load all textures used by this demo
;Call this if we just (re)created the app window
ReloadTextures proc	
	mov MyTextureID,$inv(LoadTexture,"Data/Tile1.jpg",TRUE)
	DbgDec eax,"texture id"
	ret
ReloadTextures endp

;Release all textures used by this demo
;Call this if the app window is killed
ReleaseTextures proc	
	.if MyTextureID!=0
		invoke glDeleteTextures,1,addr MyTextureID
	.endif
	ret
ReleaseTextures endp

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
	;Initialize Lighting
    invoke glLightfv,GL_LIGHT1, GL_AMBIENT, addr AmbientLight
    invoke glLightfv,GL_LIGHT1, GL_DIFFUSE, addr DiffuseLight
    invoke glLightfv,GL_LIGHT1, GL_SPECULAR,addr SpecularLight
    invoke glLightfv,GL_LIGHT1, GL_POSITION, addr PositionLight
	invoke glEnable,GL_LIGHT1
	invoke glEnable,GL_LIGHTING
	;Calculate SurfaceNormals for tetrahedron
	invoke CalcSurfaceNormal,addr FaceNorm1,addr TriPoint1,addr TriPoint2, addr TriPoint3	;123
	invoke CalcSurfaceNormal,addr FaceNorm2,addr TriPoint1,addr TriPoint4, addr TriPoint2	;142
	invoke CalcSurfaceNormal,addr FaceNorm3,addr TriPoint1,addr TriPoint3, addr TriPoint4	;134
	invoke CalcSurfaceNormal,addr FaceNorm4,addr TriPoint4,addr TriPoint3, addr TriPoint2	;432
	lea eax,FaceNorm1
	DbgFloat FaceNorm4.x
	DbgFloat FaceNorm4.y
	DbgFloat FaceNorm4.z
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
DrawGLScene proc

	invoke glClear,GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT
	
	;If the texture min/mag filter was changed (by the user pressing F)
	.if FilterChanged
		invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, MinFilter
		invoke glTexParameteri,GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, MagFilter
		mov FilterChanged,FALSE
	.endif

	invoke glLoadIdentity										; Reset the current modelview matrix
	invoke glTranslatef,Translate1.x,Translate1.y,Translate1.z	; Move left 1.5 units and into the screen 6.0
	invoke glRotatef,fRotation_Triangle,vAxis_Triangle.x,vAxis_Triangle.y,vAxis_Triangle.z
	invoke glBegin,GL_TRIANGLES									; Drawing using triangles
	
		;123
		invoke glNormal3f,FaceNorm1.x,FaceNorm1.y,FaceNorm1.z
		invoke glTexCoord2f,TriUV1.u,TriUV1.v
		invoke glVertex3f,TriPoint1.x,TriPoint1.y,TriPoint1.z	; Top		
		invoke glTexCoord2f,TriUV2.u,TriUV2.v
		invoke glVertex3f,TriPoint2.x,TriPoint2.y,TriPoint2.z	; Bottom left
		invoke glTexCoord2f,TriUV3.u,TriUV3.v
		invoke glVertex3f,TriPoint3.x,TriPoint3.y,TriPoint3.z	; Bottom right
		;142
		invoke glNormal3f,FaceNorm2.x,FaceNorm2.y,FaceNorm2.z
		invoke glTexCoord2f,TriUV1.u,TriUV1.v
		invoke glVertex3f,TriPoint1.x,TriPoint1.y,TriPoint1.z	; Top
		invoke glTexCoord2f,TriUV4.u,TriUV4.v
		invoke glVertex3f,TriPoint4.x,TriPoint4.y,TriPoint4.z	; Bottom left
		invoke glTexCoord2f,TriUV2.u,TriUV2.v
		invoke glVertex3f,TriPoint2.x,TriPoint2.y,TriPoint2.z	; Bottom right
		;134
		invoke glNormal3f,FaceNorm3.x,FaceNorm3.y,FaceNorm3.z
		invoke glTexCoord2f,TriUV1.u,TriUV1.v
		invoke glVertex3f,TriPoint1.x,TriPoint1.y,TriPoint1.z	; Top
		invoke glTexCoord2f,TriUV3.u,TriUV3.v
		invoke glVertex3f,TriPoint3.x,TriPoint3.y,TriPoint3.z	; Bottom left
		invoke glTexCoord2f,TriUV4.u,TriUV4.v
		invoke glVertex3f,TriPoint4.x,TriPoint4.y,TriPoint4.z	; Bottom right
		;432
		invoke glNormal3f,FaceNorm4.x,FaceNorm4.y,FaceNorm4.z
		invoke glTexCoord2f,TriUV4.u,TriUV4.v
		invoke glVertex3f,TriPoint4.x,TriPoint4.y,TriPoint4.z	; Top
		invoke glTexCoord2f,TriUV3.u,TriUV3.v
		invoke glVertex3f,TriPoint3.x,TriPoint3.y,TriPoint3.z	; Bottom left
		invoke glTexCoord2f,TriUV2.u,TriUV2.v
		invoke glVertex3f,TriPoint2.x,TriPoint2.y,TriPoint2.z	; Bottom right
	invoke glEnd												; Finished drawing triangles

	fld fRotSpeed_Triangle
	fadd fRotation_Triangle
	fstp fRotation_Triangle

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
	
	
	invoke glGetString,GL_VERSION
	.if byte ptr[eax]<"3"
		DbgWarning "Warning - Your OpenGL driver is OLD - you should update it."
	.endif
	DbgStr eax,"Current OpenGL driver version"
		
	
	;Load textures
	invoke ReloadTextures

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
				invoke ReleaseTextures				
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
				invoke ReloadTextures
			.endif
			
		.endif
	.until 0

	; Shutdown
	invoke ReleaseTextures
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

			;Change filter if user released the F key
			;(we may get several keydown messages, but only one keyup message)
			.if edx==VK_F
				mov eax,CurrentFilter
				inc eax
				xor edx,edx
				mov ecx,3
				div ecx
				mov CurrentFilter,edx
				DbgDec CurrentFilter
				
				.if CurrentFilter==0
					mov MinFilter,GL_LINEAR
					mov MagFilter,GL_LINEAR
				.elseif CurrentFilter==1
					mov MinFilter,GL_NEAREST
					mov MagFilter,GL_NEAREST
				.else
					mov MinFilter,GL_LINEAR_MIPMAP_NEAREST
					mov MagFilter,GL_LINEAR
				.endif
				
				mov FilterChanged,TRUE
			.endif
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
