
.data
   ClassName db "MainWinClass",0
   AppName   db "Main Window",0
   fullscreen BOOL TRUE
   active	  BOOL FALSE
   hRC dd 0				;OGL Render device context
   hDC dd 0				;Typical Window device context
   
   ;Some constants, because passing floats isnt always easy
   r4_half real4 0.5f
   r4_1    real4 1.0f
   r4_1000 real4 1000.0f
   
   r8_0	   real8 0.0f
   r8_1    real8 1.0f
   r8_45   real8 45.0f
   r8_75   real8 75.0f
   r8_1000 real8 1000.0f
   
   ;Somewhere to put keyboard state
   keys db 256 dup (0)
   
.data?
   CommandLine LPSTR ?
   hWnd dd ?
.code