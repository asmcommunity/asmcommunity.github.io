;Simple RK4 integration framework
;Original Copyright (c) 2004, Glenn Fiedler
;http://www.gaffer.org/articles
;
;This version by EvilHomer, Jan. 2006
;Updated from 1D to 3D by EvilHomer, Dec. 2006
;(Still no Rotational code)
;Whoops, there goes another year
;Whoops, there goes another pint of beer

.data


State struct
	vPosition Vec3 <> ;Position
	vVelocity Vec3 <> ;Velocity
State ends

Derivative struct
	_dVel Vec3 <> ;derivative of position: velocity
	_dAcc Vec3 <> ;derivative of velocity: acceleration
Derivative ends

;Our 'Spring Constants'...
	;k is our Coefficient of Elasticity
	;0 = very 'stretchy', larger to make the spring 'more strong' (apply more force)
	;b is our Coeffient of Dampening
	;0 = infinitely Bouncey, larger to make the spring 'more stiff'
k REAL4 10.0f
b REAL4 1.0f

fp0pt1 REAL4 0.1f
fp2 REAL4 2.0f
fp0 REAL4 0.0f
fhalf REAL4 0.5f
fp1Over6 REAL4 0.16666666666666666666666666666667f
fp100 REAL4 100.0f

.code
;Calculate the acceleration force due to our demo Spring
acceleration proc uses edi pDerivOut, pinitialstate, ft:real8	
	;return - k * state.x - b * state.v;
	mov eax,pinitialstate
	mov edi,pDerivOut
	fld k
	fchs
	fmul [eax].State.vPosition.x
	fld b
	fmul [eax].State.vVelocity.x
	fsub	
	fstp [edi].Derivative._dAcc.x
	;
	fld k
	fchs
	fmul [eax].State.vPosition.y
	fld b
	fmul [eax].State.vVelocity.y
	fsub	
	fstp [edi].Derivative._dAcc.y
	;
	fld k
	fchs
	fmul [eax].State.vPosition.z
	fld b
	fmul [eax].State.vVelocity.z
	fsub	
	fstp [edi].Derivative._dAcc.z
	ret
acceleration endp

evaluate1 proc uses esi edi,pDerivOut,pStateIn, ft:real8
	mov esi,pStateIn
	mov edi,pDerivOut
	m2m [edi].Derivative._dVel.x, [esi].State.vVelocity.x
	m2m [edi].Derivative._dVel.y, [esi].State.vVelocity.y
	m2m [edi].Derivative._dVel.z, [esi].State.vVelocity.z
	invoke acceleration, edi, pStateIn, ft
	ret
evaluate1 endp

;It is absolutely critical that you understand what this method is doing.
;First it takes the current state of the object (its position and velocity)
;and advances it ahead dt seconds using an Euler Integration step 
;with the derivatives that were passed in (velocity and acceleration). 
;Once this new position and velocity are calculated, it calculates 
;new derivatives at this point in time using the integrated state. 
;These derivatives will be different to the derivatives that were 
;initially passed in to the method if the derivatives are not 
;constant over the timestep. 
;===========================================================
evaluate2 proc uses esi edi,pDerivOut,pStateIn, ft:real8, fdt:real8, pDerivIn
LOCAL state:State
LOCAL ff:REAL8

	mov esi,pStateIn
	mov edi,pDerivIn

	fld [edi].Derivative._dVel.x
	fmul fdt
	fadd [esi].State.vPosition.x
	fstp state.vPosition.x
	fld [edi].Derivative._dVel.y
	fmul fdt
	fadd [esi].State.vPosition.y
	fstp state.vPosition.y
	fld [edi].Derivative._dVel.z
	fmul fdt
	fadd [esi].State.vPosition.z
	fstp state.vPosition.z
	;
	fld [edi].Derivative._dAcc.x
	fmul fdt
	fadd [esi].State.vVelocity.x
	fstp state.vVelocity.x
	fld [edi].Derivative._dAcc.y
	fmul fdt
	fadd [esi].State.vVelocity.y
	fstp state.vVelocity.y
	fld [edi].Derivative._dAcc.z
	fmul fdt
	fadd [esi].State.vVelocity.z
	fstp state.vVelocity.z		
	;
	mov edi,pDerivOut
	m2m [edi].Derivative._dVel.x,state.vVelocity.x
	m2m [edi].Derivative._dVel.y,state.vVelocity.y
	m2m [edi].Derivative._dVel.z,state.vVelocity.z
	;
	fld ft
	fadd fdt
	fstp ff
	invoke acceleration, edi, addr state, ff
	ret
evaluate2 endp

integrate proc uses esi,pstate, ft:real8, fdt:real8
LOCAL _a:Derivative
LOCAL _b:Derivative
LOCAL _c:Derivative
LOCAL _d:Derivative
LOCAL fdtOver2:REAL8

	;Calculate the halfway point in the timeframe
	fld fdt
	fmul fhalf
	fstp fdtOver2

	;Evaluate the Derivatives at partial timesteps
	invoke evaluate1,addr _a,pstate,ft
	invoke evaluate2,addr _b,pstate,ft,fdtOver2,addr _a
	invoke evaluate2,addr _c,pstate,ft,fdtOver2,addr _b
	invoke evaluate2,addr _d,pstate,ft,fdt,addr _c

	mov esi,pstate
	;
	fld _b._dVel.x
	fadd _c._dVel.x
	fmul fp2
	fadd _a._dVel.x
	fadd _d._dVel.x
    fmul fp1Over6
    fmul fdt
	fadd [esi].State.vPosition.x
	fstp [esi].State.vPosition.x
	fld _b._dVel.y
	fadd _c._dVel.y
	fmul fp2
	fadd _a._dVel.y
	fadd _d._dVel.y
    fmul fp1Over6
    fmul fdt
	fadd [esi].State.vPosition.y
	fstp [esi].State.vPosition.y
	fld _b._dVel.z
	fadd _c._dVel.z
	fmul fp2
	fadd _a._dVel.z
	fadd _d._dVel.z
    fmul fp1Over6
    fmul fdt
	fadd [esi].State.vPosition.z
	fstp [esi].State.vPosition.z	
	;
	fld _b._dAcc.x
	fadd _c._dAcc.x
	fmul fp2
	fadd _a._dAcc.x
	fadd _d._dAcc.x
    fmul fp1Over6
    fmul fdt
	fadd [esi].State.vVelocity.x
	fstp [esi].State.vVelocity.x
	fld _b._dAcc.y
	fadd _c._dAcc.y
	fmul fp2
	fadd _a._dAcc.y
	fadd _d._dAcc.y
    fmul fp1Over6
    fmul fdt
	fadd [esi].State.vVelocity.y
	fstp [esi].State.vVelocity.y
	fld _b._dAcc.z
	fadd _c._dAcc.z
	fmul fp2
	fadd _a._dAcc.z
	fadd _d._dAcc.z
    fmul fp1Over6
    fmul fdt
	fadd [esi].State.vVelocity.z
	fstp [esi].State.vVelocity.z	
	ret
integrate endp

.data
fEpsilon REAL4 0.001f
.code

testing proc
LOCAL state:State
LOCAL ft:REAL8
LOCAL fdt:REAL8
LOCAL cnt

      mov cnt,0
      
	fld fp100
	fstp state.vPosition.x
	mov state.vPosition.y,0
	fld fp100
	fmul fhalf
	fstp state.vPosition.z

	mov state.vVelocity.x,0
	mov state.vVelocity.y,0
	mov state.vVelocity.z,0
	
	fld fp0
	fstp ft
	fld fp0pt1
	fstp fdt
	
	.repeat ;while (fabs(state.x)>0.001f || fabs(state.v)>0.001f)
		lea eax,state.vPosition
        DbgVec3 eax
		invoke integrate,addr state, ft, fdt
		;increment time by fixed delta
		fld fdt
		fadd ft
		fstp ft
        ;Who is bigger - state.N or fEpsilon?
        fAbsMax state.vPosition.x,fEpsilon 
        fstpReg eax
        fAbsMax state.vVelocity.x,fEpsilon 
        fstpReg ebx
        ;Are both FABS(State.N) less than Epsilon?
        .if eax==fEpsilon && ebx==fEpsilon 
            .break 
        .endif
        inc cnt
	.until 0
      lea eax,state
      DbgVec2 eax
      DbgDec cnt,"Number of iterations"
	ret
testing endp

