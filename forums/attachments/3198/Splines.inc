; Class  : Spline
; Purpose: Support class implementing two kinds of Spline Interpolation, in 2D and 3D
; Author : Leith Ketchell
; Date   : July 2, 2010

; Call Init_BSpline (or Init_Catmull_Rom), passing 4 adjacent points
; Now call BSpline_Lerp (or Catmull_Lerp), passing incremental values from 0.0 to 1.0
; Each Lerp call will return a point on the curve between the middle two points
;
;		OR
;
; Shove a bunch of Vectors into the Ancestor DwordCollection (as n*real4 vectors)
; Call WalkPoints_First
; Now in a Loop:
; -Call WalkPoints_Next, passing incremental values from 0.0 (to 1.0+)
; -We should get EAX=TRUE, and a Vector on the FPU stack.
; -Calling with a value above 1.0 should return EAX = FALSE.
; -Now check the value of EDX
; -EDX=0 means we just moved to the next Point, so reset your counter to 0.0
; -EDX=1 means we have ran out of Points.. its safe to Loop if we reset counter.
; Until EAX=FALSE, EDX=1


;              - - - 
;        B- - /      \ - -C      
; A                                 D

; ——————————————————————————————————————————————————————————————————————————————————————————————————

.data
r4_3 real4 3.0f
r4_4 real4 4.0f
r4_5 real4 5.0f
r4_6 real4 6.0f

.code

; ——————————————————————————————————————————————————————————————————————————————————————————————————


; ——————————————————————————————————————————————————————————————————————————————————————————————————

Object Spline, 345343, DwordCollection
	StaticMethod Init_BSpline_2D,Pointer,Pointer,Pointer,Pointer
	StaticMethod Init_BSpline_3D,Pointer,Pointer,Pointer,Pointer
	StaticMethod Init_Catmull_Rom_2D,Pointer,Pointer,Pointer,Pointer
	StaticMethod Init_Catmull_Rom_3D,Pointer,Pointer,Pointer,Pointer
	StaticMethod BSpline_Lerp_2D,real8
	StaticMethod BSpline_Lerp_3D,real8
	StaticMethod Catmull_Lerp_2D,real8
	StaticMethod Catmull_Lerp_3D,real8
	StaticMethod WalkPoints_First_2D
	StaticMethod WalkPoints_First_3D
	StaticMethod BSpline_WalkPoints_Next_2D, real8
	StaticMethod BSpline_WalkPoints_Next_3D, real8
	DefineVariable bLooping,BOOL,FALSE		;TRUE = treat points as cyclic buffer
	DefineVariable a,real8,4 dup (<?>)
	DefineVariable b,real8,4 dup (<?>)
	DefineVariable _c,real8,4 dup (<?>)
	DefineVariable Position,dword,0
	DefineVariable p1,Pointer,NULL
	DefineVariable p2,Pointer,NULL
	DefineVariable p3,Pointer,NULL
	DefineVariable p4,Pointer,NULL
ObjectEnd

Method Spline.Init_BSpline_2D,uses esi, p1, p2, p3,p4
	SetObject esi

	;a[0] = ( -p1.x + 3*p2.x - 3*p3.x + p4.x)/6.0;
	mov edx,p1
	fld [edx].Vec3.x
	fchs
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.x
	fadd
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.x
	mov edx,p4
	fadd [edx].Vec3.x
	fsub
	fdiv r4_6
	fstp [esi].a

	;a[1] = ( 3*p1.x - 6*p2.x + 3*p3.x )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.x
	fld r4_6
	mov edx,p2
	fmul [edx].Vec3.x
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.x
	fadd
	fsub
	fdiv r4_6
	fstp [esi+sizeof real8].a

	;a[2] = (-3*p1.x + 3*p3.x )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.x
	fchs
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.x
	fadd
	fdiv r4_6
	fstp [esi+(2*sizeof real8)].a

	;a[3] = ( p1.x + 4*p2.x + p3.x )/6.0;
	mov edx,p1
	fld [edx].Vec3.x
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.x
	fadd
	mov edx,p3
	fadd [edx].Vec3.x
	fdiv r4_6
	fstp [esi+(3*sizeof real8)].a


	;b[0] = ( -p1.y + 3*p2.y - 3*p3.y + p4.y)/6.0;
	mov edx,p1
	fld [edx].Vec3.y
	fchs
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.y
	fadd
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.y
	mov edx,p4
	fadd [edx].Vec3.y
	fsub
	fdiv r4_6
	fstp [esi].b

	;b[1] = ( 3*p1.y - 6*p2.y + 3*p3.y )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.y
	fld r4_6
	mov edx,p2
	fmul [edx].Vec3.y
	fsub
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.y
	fadd
	fdiv r4_6
	fstp [esi+sizeof real8].b

	;b[2] = (-3*p1.y + 3*p3.y )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.y
	fchs
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.y
	fadd
	fdiv r4_6
	fstp [esi+(2*sizeof real8)].b

	;b[3] = ( p1.y + 4*p2.y + p3.y )/6.0;
	mov edx,p1
	fld [edx].Vec3.y
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.y
	fadd
	mov edx,p3
	fadd [edx].Vec3.y
	fdiv r4_6
	fstp [esi+(3*sizeof real8)].b

MethodEnd

Method Spline.Init_BSpline_3D,uses esi, p1, p2, p3,p4
	SetObject esi

	;a[0] = ( -p1.x + 3*p2.x - 3*p3.x + p4.x)/6.0;
	mov edx,p1
	fld [edx].Vec3.x
	fchs
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.x
	fadd
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.x
	mov edx,p4
	fadd [edx].Vec3.x
	fsub
	fdiv r4_6
	fstp [esi].a

	;a[1] = ( 3*p1.x - 6*p2.x + 3*p3.x )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.x
	fld r4_6
	mov edx,p2
	fmul [edx].Vec3.x
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.x
	fadd
	fsub
	fdiv r4_6
	fstp [esi+sizeof real8].a

	;a[2] = (-3*p1.x + 3*p3.x )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.x
	fchs
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.x
	fadd
	fdiv r4_6
	fstp [esi+(2*sizeof real8)].a

	;a[3] = ( p1.x + 4*p2.x + p3.x )/6.0;
	mov edx,p1
	fld [edx].Vec3.x
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.x
	fadd
	mov edx,p3
	fadd [edx].Vec3.x
	fdiv r4_6
	fstp [esi+(3*sizeof real8)].a


	;b[0] = ( -p1.y + 3*p2.y - 3*p3.y + p4.y)/6.0;
	mov edx,p1
	fld [edx].Vec3.y
	fchs
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.y
	fadd
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.y
	mov edx,p4
	fadd [edx].Vec3.y
	fsub
	fdiv r4_6
	fstp [esi].b

	;b[1] = ( 3*p1.y - 6*p2.y + 3*p3.y )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.y
	fld r4_6
	mov edx,p2
	fmul [edx].Vec3.y
	fsub
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.y
	fadd
	fdiv r4_6
	fstp [esi+sizeof real8].b

	;b[2] = (-3*p1.y + 3*p3.y )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.y
	fchs
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.y
	fadd
	fdiv r4_6
	fstp [esi+(2*sizeof real8)].b

	;b[3] = ( p1.y + 4*p2.y + p3.y )/6.0;
	mov edx,p1
	fld [edx].Vec3.y
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.y
	fadd
	mov edx,p3
	fadd [edx].Vec3.y
	fdiv r4_6
	fstp [esi+(3*sizeof real8)].b


	mov edx,p1
	fld [edx].Vec3.z
	fchs
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.z
	fadd
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.z
	mov edx,p4
	fadd [edx].Vec3.z
	fsub
	fdiv r4_6
	fstp [esi]._c

	;b[1] = ( 3*p1.y - 6*p2.y + 3*p3.y )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.z
	fld r4_6
	mov edx,p2
	fmul [edx].Vec3.z
	fsub
	fld r4_3
	mov edx,p3
	fmul [edx].Vec3.z
	fadd
	fdiv r4_6
	fstp [esi+sizeof real8]._c

	;b[2] = (-3*p1.y + 3*p3.y )/6.0;
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.z
	fchs
	mov edx,p1
	fld r4_3
	fmul [edx].Vec3.z
	fadd
	fdiv r4_6
	fstp [esi+(2*sizeof real8)]._c

	;b[3] = ( p1.y + 4*p2.y + p3.y )/6.0;
	mov edx,p1
	fld [edx].Vec3.z
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.z
	fadd
	mov edx,p3
	fadd [edx].Vec3.z
	fdiv r4_6
	fstp [esi+(3*sizeof real8)]._c

MethodEnd

Method Spline.Init_Catmull_Rom_2D,uses esi, p0,p1,p2,p3
	SetObject esi
	;out.x = 0.5f * ( 
    ;( 2.0f * p1.x ) +
	;( -p0.x + p2.x ) * t
	;+ ( 2.0f * p0.x - 5.0f * p1.x + 4 * p2.x - p3.x ) * t2
	;+    ( -p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x ) * t3
	; )
	mov edx,p1
	fld [edx].Vec3.y
	fadd st(0),st(0)
	fstp [esi].a
	
	mov edx,p0
	fld [edx].Vec3.y
	fchs
	mov edx,p2
	fadd [edx].Vec3.y
	fstp [esi+sizeof real8].a
	
	mov edx,p0
	fld [edx].Vec3.y
	fadd st(0),st(0)
	mov edx,p1
	fld r4_5
	fmul [edx].Vec3.y
	fsub
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.y
	fadd
	mov edx,p3
	fsub [edx].Vec3.y
	fstp [esi+(2*sizeof real8)].a
	 
	mov edx,p0
	fld [edx].Vec3.y
	fchs
	fld r4_3
	mov edx,p1
	fmul [edx].Vec3.y
	fadd
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.y
	fsub
	mov edx,p3
	fadd [edx].Vec3.y
	fstp [esi+(3*sizeof real8)].a
	 
	mov edx,p1
	fld [edx].Vec3.y
	fadd st(0),st(0)
	fstp [esi].b
	
	mov edx,p0
	fld [edx].Vec3.y
	fchs
	mov edx,p2
	fadd [edx].Vec3.y
	fstp [esi+sizeof real8].b
	
	mov edx,p0
	fld [edx].Vec3.y
	fadd st(0),st(0)
	mov edx,p1
	fld r4_5
	fmul [edx].Vec3.y
	fsub
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.y
	fadd
	mov edx,p3
	fsub [edx].Vec3.y
	fstp [esi+(2*sizeof real8)].b
	 
	mov edx,p0
	fld [edx].Vec3.y
	fchs
	fld r4_3
	mov edx,p1
	fmul [edx].Vec3.y
	fadd
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.y
	fsub
	mov edx,p3
	fadd [edx].Vec3.y
	fstp [esi+(3*sizeof real8)].b

MethodEnd

Method Spline.Init_Catmull_Rom_3D,uses esi, p0,p1,p2,p3
	SetObject esi
	;out.x = 0.5f * ( 
    ;( 2.0f * p1.x ) +
	;( -p0.x + p2.x ) * t
	;+ ( 2.0f * p0.x - 5.0f * p1.x + 4 * p2.x - p3.x ) * t2
	;+    ( -p0.x + 3.0f * p1.x - 3.0f * p2.x + p3.x ) * t3
	; )
	mov edx,p1
	fld [edx].Vec3.y
	fadd st(0),st(0)
	fstp [esi].a
	
	mov edx,p0
	fld [edx].Vec3.y
	fchs
	mov edx,p2
	fadd [edx].Vec3.y
	fstp [esi+sizeof real8].a
	
	mov edx,p0
	fld [edx].Vec3.y
	fadd st(0),st(0)
	mov edx,p1
	fld r4_5
	fmul [edx].Vec3.y
	fsub
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.y
	fadd
	mov edx,p3
	fsub [edx].Vec3.y
	fstp [esi+(2*sizeof real8)].a
	 
	mov edx,p0
	fld [edx].Vec3.y
	fchs
	fld r4_3
	mov edx,p1
	fmul [edx].Vec3.y
	fadd
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.y
	fsub
	mov edx,p3
	fadd [edx].Vec3.y
	fstp [esi+(3*sizeof real8)].a
	 
	mov edx,p1
	fld [edx].Vec3.y
	fadd st(0),st(0)
	fstp [esi].b
	
	mov edx,p0
	fld [edx].Vec3.y
	fchs
	mov edx,p2
	fadd [edx].Vec3.y
	fstp [esi+sizeof real8].b
	
	mov edx,p0
	fld [edx].Vec3.y
	fadd st(0),st(0)
	mov edx,p1
	fld r4_5
	fmul [edx].Vec3.y
	fsub
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.y
	fadd
	mov edx,p3
	fsub [edx].Vec3.y
	fstp [esi+(2*sizeof real8)].b
	 
	mov edx,p0
	fld [edx].Vec3.y
	fchs
	fld r4_3
	mov edx,p1
	fmul [edx].Vec3.y
	fadd
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.y
	fsub
	mov edx,p3
	fadd [edx].Vec3.y
	fstp [esi+(3*sizeof real8)].b


	mov edx,p1
	fld [edx].Vec3.z
	fadd st(0),st(0)
	fstp [esi]._c
	
	mov edx,p0
	fld [edx].Vec3.z
	fchs
	mov edx,p2
	fadd [edx].Vec3.z
	fstp [esi+sizeof real8]._c
	
	mov edx,p0
	fld [edx].Vec3.z
	fadd st(0),st(0)
	mov edx,p1
	fld r4_5
	fmul [edx].Vec3.z
	fsub
	fld r4_4
	mov edx,p2
	fmul [edx].Vec3.z
	fadd
	mov edx,p3
	fsub [edx].Vec3.z
	fstp [esi+(2*sizeof real8)]._c
	 
	mov edx,p0
	fld [edx].Vec3.z
	fchs
	fld r4_3
	mov edx,p1
	fmul [edx].Vec3.z
	fadd
	fld r4_3
	mov edx,p2
	fmul [edx].Vec3.z
	fsub
	mov edx,p3
	fadd [edx].Vec3.z
	fstp [esi+(3*sizeof real8)]._c

MethodEnd
 
;Returns Vec2 on FPU stack
Method Spline.BSpline_Lerp_2D,uses esi, t:real8
	SetObject esi
	
	;spline_out_x[i] = a[3] + t* (a[2] + t* (a[1] + t*a[0]) );
	fld [esi].a
	fmul t
	fadd [esi+(1*sizeof real8)].a
	fmul t
	fadd [esi+(2*sizeof real8)].a
	fmul t
	fadd [esi+(3*sizeof real8)].a

	;spline_out_y[i] = b[3] + t*(b[2] + t*(b[1] + t*b[0]));
	fld [esi].b
	fmul t
	fadd [esi+(1*sizeof real8)].b
	fmul t
	fadd [esi+(2*sizeof real8)].b
	fmul t
	fadd [esi+(3*sizeof real8)].b
MethodEnd

;Returns Vec3 on FPU stack
Method Spline.BSpline_Lerp_3D,uses esi, t:real8
	SetObject esi
	
	;spline_out_x[i] = a[3] + t* (a[2] + t* (a[1] + t*a[0]) );
	fld [esi].a
	fmul t
	fadd [esi+(1*sizeof real8)].a
	fmul t
	fadd [esi+(2*sizeof real8)].a
	fmul t
	fadd [esi+(3*sizeof real8)].a

	;spline_out_y[i] = b[3] + t*(b[2] + t*(b[1] + t*b[0]));
	fld [esi].b
	fmul t
	fadd [esi+(1*sizeof real8)].b
	fmul t
	fadd [esi+(2*sizeof real8)].b
	fmul t
	fadd [esi+(3*sizeof real8)].b

	fld [esi]._c
	fmul t
	fadd [esi+(1*sizeof real8)]._c
	fmul t
	fadd [esi+(2*sizeof real8)]._c
	fmul t
	fadd [esi+(3*sizeof real8)]._c
MethodEnd

;Returns Vec2 on FPU stack
Method Spline.Catmull_Lerp_2D,uses esi, t:real8
LOCAL t2:real8, t3:real8
	SetObject esi
; t2 = t * t
	fld t
	fmul st(0),st(0)
	fst t2
; t3 = t2 * t
	fmul t
	fstp t3
	 	
 	fld [esi].a
 	fld [esi+sizeof real8].a
 	fmul t
 	fld [esi+(2*sizeof real8)].a
 	fmul t2
 	fld [esi+(3*sizeof real8)].a
 	fmul t3
 	fadd
 	fadd
 	fadd
 	fmul r4_half
 	
 	fld [esi].b
 	fld [esi+sizeof real8].b
 	fmul t
 	fld [esi+(2*sizeof real8)].b
 	fmul t2
 	fld [esi+(3*sizeof real8)].b
 	fmul t3
 	fadd
 	fadd
 	fadd
 	fmul r4_half
 	
MethodEnd

;Returns Vec3 on FPU stack
Method Spline.Catmull_Lerp_3D,uses esi, t:real8
LOCAL t2:real8, t3:real8
	SetObject esi
; t2 = t * t
	fld t
	fmul st(0),st(0)
	fst t2
; t3 = t2 * t
	fmul t
	fstp t3
	 	
 	fld [esi].a
 	fld [esi+sizeof real8].a
 	fmul t
 	fld [esi+(2*sizeof real8)].a
 	fmul t2
 	fld [esi+(3*sizeof real8)].a
 	fmul t3
 	fadd
 	fadd
 	fadd
 	fmul r4_half
 	
 	fld [esi].b
 	fld [esi+sizeof real8].b
 	fmul t
 	fld [esi+(2*sizeof real8)].b
 	fmul t2
 	fld [esi+(3*sizeof real8)].b
 	fmul t3
 	fadd
 	fadd
 	fadd
 	fmul r4_half
 
  	fld [esi]._c
 	fld [esi+sizeof real8]._c
 	fmul t
 	fld [esi+(2*sizeof real8)]._c
 	fmul t2
 	fld [esi+(3*sizeof real8)]._c
 	fmul t3
 	fadd
 	fadd
 	fadd
 	fmul r4_half
MethodEnd

Method Spline.WalkPoints_First_2D,uses esi

	SetObject esi
	mov [esi].Position,0
	.if [esi].dCount >= 4*2	;need 4 elements
		mov eax,[esi].pItems
		mov [esi].p1,eax
		add eax,2*sizeof real4
		mov [esi].p2,ecx
		add eax,2*sizeof real4
		mov [esi].p3,edx
		add eax,2*sizeof real4
		mov [esi].p4,edx
	.endif	 
MethodEnd

Method Spline.WalkPoints_First_3D,uses esi

	SetObject esi
	mov [esi].Position,0
	.if [esi].dCount >= 4*2	;need 4 elements
		mov eax,[esi].pItems
		mov [esi].p1,eax
		add eax,3*sizeof real4
		mov [esi].p2,ecx
		add eax,3*sizeof real4
		mov [esi].p3,edx
		add eax,3*sizeof real4
		mov [esi].p4,edx
	.endif	 
MethodEnd

;Returns TRUE (Vec2 returned on FPU)
;     or FALSE , EDX = 1 : We Ran out of Points
;				 EDX = 0 : We incremented to the next point (reset your t to 0.0)
Method Spline.BSpline_WalkPoints_Next_2D,uses esi, t:real8

	SetObject esi
	fld1
	fsub t
	fstpReg eax
	.ifBitSet eax,BIT31
		inc [esi].Position
		
		m2m [esi].p1,[esi].p2,edx
		m2m [esi].p2,[esi].p3,edx
		m2m [esi].p3,[esi].p4,edx

		mov edx,[esi].Position
		.if edx>=[esi].dCount
			mov [esi].Position,0
			m2m [esi].p3, [esi].pItems, edx
			mov edx,1
		.else
			mov eax,[esi].p2
			add eax,2*sizeof real4
			mov [esi].p3,eax
			xor edx,edx
		.endif
		return FALSE
	.endif
	OCall esi.BSpline_Lerp_2D,t
	mov eax,TRUE
MethodEnd


;Returns TRUE (Vec3 returned on FPU)
;     or FALSE , EDX = 1 : We Ran out of Points
;				 EDX = 0 : We incremented to the next point (reset your t to 0.0)
Method Spline.BSpline_WalkPoints_Next_3D,uses esi, t:real8

	SetObject esi
	fld1
	fsub t
	fstpReg eax
	.ifBitSet eax,BIT31
		inc [esi].Position
		
		m2m [esi].p1,[esi].p2,edx
		m2m [esi].p2,[esi].p3,edx
		m2m [esi].p3,[esi].p4,edx

		mov edx,[esi].Position
		.if edx>=[esi].dCount
			mov [esi].Position,0
			m2m [esi].p3, [esi].pItems, edx
			mov edx,1
		.else
			mov eax,[esi].p2
			add eax,3*sizeof real4
			mov [esi].p3,eax
			xor edx,edx
		.endif
		return FALSE
	.endif
	OCall esi.BSpline_Lerp_3D,t
	mov eax,TRUE
MethodEnd
