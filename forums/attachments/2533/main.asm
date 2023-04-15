
SCREEN_WIDTH equ 800
SCREEN_HEIGHT equ 600
SCREEN_BPP equ 16

.data
	g_WindowName db "main",0
	_Dbl1	real8	1.0

.code
	; Desc: Perform the various initializations
	Initialize		proc
		invoke	glClearDepth, dword ptr [_Dbl1], dword ptr [_Dbl1 + 4]
		invoke	glHint, GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST
		invoke	glClearColor, 0, 0, 0, CFLT(1.0)
		invoke	glEnable, GL_NORMALIZE
		invoke	glEnable, GL_CULL_FACE
		mov	eax,TRUE
		ret
	Initialize		endp

	; Desc: Free allocated resources
	Deinitialize		proc
		ret
	Deinitialize		endp

	; Desc: Update the scene
	Update			proc	MilliSeconds:real4, TotalTime:real4
		; Update scene here
		mov	eax,[g_keys]
		mov	al,[eax + VK_ESCAPE]
		test	al,al
		jz	Esc_Pressed
		invoke	Terminate_Application
		Esc_Pressed:		ret
	Update			endp

	; Desc: Draw the scene
	Draw			proc
		invoke glClear, GL_COLOR_BUFFER_BIT or GL_DEPTH_BUFFER_BIT or GL_STENCIL_BUFFER_BIT
		invoke glBegin,GL_TRIANGLES
			invoke glColor3f, CFLT(0.5), CFLT(1.0), CFLT(0.5)
			invoke glVertex3f,CFLT(0.0),CFLT(1.0),CFLT(0.0)		// Top
			invoke glVertex3f,CFLT(-1.0),CFLT(-1.0),CFLT(0.0)	// Bottom Left
			invoke glVertex3f,CFLT(1.0),CFLT(-1.0),CFLT(0.0)		// Bottom Right
		invoke glEnd
		ret
	Draw			endp