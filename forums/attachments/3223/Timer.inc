;Use these commands with the GET Method
;Note that GET always leaves a time value on the FPU: real8 , in Seconds
ABSOLUTETIME EQU 0 ;time since windows started
APPTIME EQU 1 ;zerobased time since Reset was called - can be paused/unpaused
ELAPSEDTIME EQU 2 ;time elapsed between 

.code


Object Timer,345345,Primer

RedefineMethod Init
VirtualMethod Get, dword
VirtualMethod Reset
VirtualMethod Start
VirtualMethod Stop
VirtualMethod Advance
DefineVariable Period , real8, 0.0f
DefineVariable StopTime, real8, 0.0f
DefineVariable LastElapsedTime, real8, 0.0f
DefineVariable BaseTime, real8, 0.0f
DefineVariable AppTime, real8, 0.0f
DefineVariable fElapsedTime, real8, 0.0f
DefineVariable IsRunning, BOOL, FALSE
ObjectEnd

Method Timer.Reset,uses esi
LOCAL qTime:QWORD
SetObject esi
INVOKE QueryPerformanceCounter, ADDR qTime
fild qTime
fmul [esi].Period
fst [esi].BaseTime
fst [esi].StopTime
fstp [esi].LastElapsedTime
MethodEnd

Method Timer.Start, uses esi
LOCAL qTime:QWORD
SetObject esi
.if [esi].IsRunning==FALSE
fld [esi].BaseTime
INVOKE QueryPerformanceCounter, ADDR qTime
fild qTime
fmul [esi].Period
fst [esi].LastElapsedTime
fsub [esi].StopTime
fadd [esi].BaseTime
fstp [esi].BaseTime
fldz
fst [esi].StopTime 
fstp [esi].AppTime 
mov [esi].IsRunning,TRUE
.endif
MethodEnd

;Advance timer by 1/10th of the period (normally 0.1 seconds)
Method Timer.Advance, uses esi
SetObject esi
.if [esi].IsRunning==FALSE
mov eax, 10
fildReg eax
fmul [esi].Period
fadd [esi].StopTime
fstp [esi].StopTime
.endif
MethodEnd

Method Timer.Stop, uses esi
LOCAL qTime:QWORD
SetObject esi
.if [esi].IsRunning==TRUE
;fld [esi].BaseTime
INVOKE QueryPerformanceCounter, ADDR qTime
fild qTime
fmul [esi].Period
fst [esi].LastElapsedTime
fstp [esi].StopTime 
mov [esi].IsRunning,FALSE
.endif
MethodEnd

Method Timer.Init,uses esi
LOCAL qTPS:QWORD
SetObject esi
INVOKE QueryPerformanceFrequency, ADDR qTPS
fld1
fild qTPS
fdiv
fstp [esi].Period 
OCall esi.Reset
MethodEnd

Method Timer.Get,uses esi,command
LOCAL qTime:QWORD
SetObject esi
.if [esi].IsRunning==FALSE && command==ABSOLUTETIME
fld [esi].StopTime
fst [esi].AppTime 
ExitMethod 
     .endif

INVOKE QueryPerformanceCounter, ADDR qTime
fild qTime ;get integer time
fmul [esi].Period ;convert to float

.switch command

.case ELAPSEDTIME
fld st(0) ;duplicate
fsub [esi].LastElapsedTime ;find elapsed
fstp [esi].fElapsedTime ;store elapsed
fstp [esi].LastElapsedTime ;store float
fld [esi].fElapsedTime

.case APPTIME
fsub [esi].BaseTime
fst [esi].AppTime

.case ABSOLUTETIME
fst [esi].AppTime

.default
fUnload
DbgWarning "Error - Unknown Command in Timer.Get"
int 3

.endsw

MethodEnd
