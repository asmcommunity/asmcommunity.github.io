
ifndef __VEC3_MATH
    __VEC3_MATH equ 1    
    ifndef f_OK
        ;We must include OA32's fMath macros (so far only 'fjle' in Normalize requires it)
        ;and they require that D3DXVECTOR3 is defined.
        ifndef D3DXVECTOR3
            D3DXVECTOR3 typedef Vec3
        endif
        % include &MacPath&fMath.inc
    endif
    
    ;Supporting DOUBLE PRECISION vectors :D
    Vec3_Double struct
        x real8 ?
        y real8 ?
        z real8 ?
    Vec3_Double ends

    Vec3_Copy macro target, source
        Vec3_Load source
        Vec3_Stow target
    endm
    
    ;Load a Vec3 onto the fpu
    ;fpu = source
    Vec3_Load macro source
        fld source.x
        fld source.y
        fld source.z
    endm 
    
    ;Store a Vec3 from fpu
    ;target = fpu
    Vec3_Stow macro target
        fstp target.z
        fstp target.y
        fstp target.x
    endm
    
    ;FPU = abs(source)
    Vec3_Abs macro source
        fld source.x
        fabs
        fld source.y
        fabs
        fld source.z
        fabs
    endm
    
    ;Perform vector Addition, result on fpu
    ;fpu = sourceA + sourceB
    Vec3_Add macro sourceA, sourceB
        fld  sourceA.x
        fadd sourceB.x
        fld  sourceA.y
        fadd sourceB.y
        fld  sourceA.z
        fadd sourceB.z
    endm
    
    ;Perform vector Addition, result on fpu
    ;fpu = fpu + source
    Vec3_AddFrom macro source
        fld source.x
        faddp st(3),st(0)
        fld source.y
        faddp st(2),st(0)
        fld source.z
        faddp st(1),st(0)
    endm


    ;Perform vector Addition
    ;target = target + fpu
    Vec3_AddToFPU macro source
        Vec3_Load source
        faddp st(3),st(0)
        faddp st(2),st(0)
        faddp st(1),st(0)
    endm
    
    ;Perform vector Addition
    ;target = target + fpu
    Vec3_AddTo macro target
        fadd target.z
        fstp target.z
        fadd target.y
        fstp target.y
        fadd target.x
        fstp target.x
    endm

    ;Perform vector Addition
    ;target = source + fpu
    Vec3_AddToTarget macro target, source
        fadd source.z
        fstp target.z
        fadd source.y
        fstp target.y
        fadd source.x
        fstp target.x
    endm
    
    ;Perform vector Subtraction, result on fpu
    ;fpu = sourceA - sourceB
    Vec3_Sub macro sourceA:req, sourceB:req
        fld  sourceA.x
        fsub sourceB.x
        fld  sourceA.y
        fsub sourceB.y
        fld  sourceA.z
        fsub sourceB.z
    endm
    
    ;Target = Source - FPU
    Vec3_SubFrom macro TargetA,SourceA
        fld SourceA.z
        fsubp st(1),st(0)
        fld SourceA.y
        fsubp st(2),st(0)
        fld SourceA.z
        fsubp st(3),st(0)
        Vec3_Stow TargetA
    endm
    
    ;FPU = FPU - source
    Vec3_Subtract macro source
        fld source.x
        fsubp st(3),st(0)
        fld source.y
        fsubp st(2),st(0)
        fld source.z
        fsubp st(1),st(0)
    endm
    
    ;FPU = source - FPU
    Vec3_Subr macro source
        fld source.x
        fsubrp st(3),st(0)
        fld source.y
        fsubrp st(2),st(0)
        fld source.z
        fsubrp st(1),st(0)
    endm
    
    ;Perform vector Multiply, result on fpu
    ;fpu = sourceA * sourceB
    Vec3_Mul macro sourceA, sourceB
        fld  sourceA.x
        fmul sourceB.x
        fld  sourceA.y
        fmul sourceB.y
        fld  sourceA.z
        fmul sourceB.z
    endm
   
    Vec3_ScaleFPU macro scalar
        fld scalar
        fmul st(3),st(0)
        fmul st(2),st(0)
        fmul
    endm
     
    ;Multiply a vector by a scalar, result on fpu
    Vec3_Scale macro vector, scalar
        Vec3_Load vector
        Vec3_ScaleFPU scalar
    endm
    
    ;Multiply a vector by two scalars, result on fpu
    Vec3_Scale2 macro vector, scalar1, scalar2
        Vec3_Load vector
        fld  scalar1
        fmul scalar2
        fmul st(3),st(0)
        fmul st(2),st(0)
        fmul
    endm


    
    ;Perform vector Divide, result on fpu
    ;fpu = sourceA / sourceB
    Vec3_Div macro sourceA, sourceB
        fld  sourceA.x
        fdiv sourceB.x
        fld  sourceA.y
        fdiv sourceB.y
        fld  sourceA.z
        fdiv sourceB.z
    endm
    
    
    
    
    ;Perform a Dot Product, result on fpu
    ;fpu = DotProduct (a,b) = ax*bx + ay*by + az*bz
    Vec3_Dot macro a,b
        fld a.x
        fmul b.x
        fld a.y
        fmul b.y
        fld a.z
        fmul b.z
        fadd
        fadd
    endm

    ;Calculate distance from Point to Plane along the Plane's Normal
    ;fpu = Dotproduct(Vector, PlaneNormal) + PlaneDistance
    Vec3_PlaneDotCoord macro Point, Plane
        Vec3_Dot Point, Plane
        fadd Plane.w
    endm
    

    ;Calculate distance from Point to Plane along given vector
    Vec3_DistanceRayPlane macro vOrigin, vNormal, Plane
        Vec3_Dot Plane, vOrigin
        fadd Plane.w
        Vec3_Dot Plane, vNormal
        fdiv
        fchs
    endm
    

    ;Perform a Cross Product, result on fpu
    Vec3_Cross macro A, B
    	;x = A.y * B.z - A.z * B.y
    	fld  A.y
    	fmul B.z
    	fld  A.z
    	fmul B.y
    	fsub
    	;y = A.z * B.x - A.x * B.z
    	fld  A.z
    	fmul B.x
    	fld  A.x
    	fmul B.z
    	fsub
    	;z = A.x * B.y - A.y * B.x
    	fld  A.x
    	fmul B.y
    	fld  A.y
    	fmul B.x
    	fsub
    endm
    
    .data
    r4_1em10 real4 1.0e-10
    .code
    
    ;Normalize a vector, result on fpu
    ;fpu = Normal(Source)
    Vec3_Normal macro Source
        Vec3_Load Source          ;Preload the fpu with the src vector
        Vec3_Dot Source,Source    ;Calculate dot product
        fcom r4_1em10             ;if the dot product is very small,
        fjle @F                   ;just return the source vector ... dont bother normalizing very small vectors
        fsqrt                     ;sqrt of something very small is something very smaller !!
        fld1
        fdivr                     ;st(0) = 1/sqrt(dot src,src)
        fmul st(3),st(0)
        fmul st(2),st(0)
        fmul st(1),st(0)
    @@: fstp st(0)                ;pop the junk value (dotproduct, or 1/sqrt(dot src,src))
    endm
    
    ;Normalize a vector, result in vector
    Vec3_Normalize macro vector
        Vec3_Normal vector
        Vec3_Stow vector
    endm
    
    ;Negate the sign of source vector, result in target vector
    Vec3_Neg macro target, source
        Vec3_Load source
        fchs
        fstp target.z
        fchs
        fstp target.y
        fchs
        fstp target.x
    endm

    ;FPU = source * scalar
    Vec3_NegScale macro  source, scalar
        Vec3_Load source
        
        fld scalar
        fchs
        
        fmul st(3),st(0)
        fmul st(2),st(0)
        fmul
        
    endm
    
    ;target = source * scalar
    Vec3_NegScaleTo macro target, source, scalar
        Vec3_NegScale source, scalar
        Vec3_Stow target
    endm
        
    ;fpu = length(v)
    Vec3_Length macro a
        Vec3_Dot a,a
        fsqrt
    endm
    
    ;fpu = distance (a,b)
    Vec3_Distance macro a,b
        fld a.x
        fsub b.x
        fmul st(0),st(0)
        fld a.y
        fsub b.y
        fmul st(0),st(0)
        fld a.z
        fsub b.z
        fmul st(0),st(0)
        fadd
        fadd
        fsqrt
    endm
    
    Vec3_Distance_Plane macro Point,Plane
        Vec3_PlaneDotCoord Point, Plane 
    endm
    
    
    $Vec3_Equal macro v1,v2
        Vec3_Sub v1,v2
        fadd
        fadd
        fstpReg edx
        .if edx==0
            mov eax,TRUE
        .else
            mov eax,FALSE
        .endif
        exitm <eax>
    endm
    
    ;Calculate Vector from A to B
    ;This is a Direction with Magnitude.
    Vec3_Direction macro vOut,vA,vB
        Vec3_Sub vB,vA
        Vec3_Stow vOut
    endm
    
    ;Calculate Normalized Vector from A to B
    ;This is a Direction with unit length.
    Vec3_NormalDir macro vOut,vA,vB
        Vec3_Direction vOut,vA,vB
        Vec3_Normalize vOut
    endm
    
    ;Find the MidPoint of an Edge
    Vec3_MidPoint macro vOut,vA,vB
        Vec3_Sub vB,vA
        Vec3_ScaleFPU r4_half
        Vec3_AddFrom vA
        Vec3_Stow vOut
    endm
    
.data
costheta real8 0.0
sintheta real8 0.0
.code

;Rotate a Point by given angle around arbitrary axis.
;ASSUMES RIGHT HAND COORDINATE SYSTEM
;Positive angles are counterclockwise as seen looking down the axis vector.
;Returns rotated point on FPU.
Vec3_RotateAboutAxis macro Point,Angle,Axis

   Vec3_Normalize Axis
   fld Angle
   fsincos
   fstp costheta
   fstp sintheta

   ;q.x = (costheta + (1 - costheta) * r.x * r.x) * p.x;
   ;q.x += ((1 - costheta) * r.x * r.y - r.z * sintheta) * p.y;
   ;q.x += ((1 - costheta) * r.x * r.z + r.y * sintheta) * p.z;
   fld1
   fsub costheta
   fmul Axis.x
   fmul Axis.x
   fadd costheta
   fmul Point.x
   
   fld1
   fsub costheta
   fmul Axis.x
   fmul Axis.y
   fld Axis.z
   fmul sintheta
   fsub
   fmul Point.y
   fadd
   
   fld1
   fsub costheta
   fmul Axis.x
   fmul Axis.z
   fld Axis.y
   fmul sintheta
   fadd
   fmul Point.z
   fadd

   ;q.y = ((1 - costheta) * r.x * r.y + r.z * sintheta) * p.x;
   ;q.y += (costheta + (1 - costheta) * r.y * r.y) * p.y;
   ;q.y += ((1 - costheta) * r.y * r.z - r.x * sintheta) * p.z;
   fld1
   fsub costheta
   fmul Axis.x
   fmul Axis.y
   fld Axis.z
   fmul sintheta
   fadd
   fmul Point.x
   
   fld1
   fsub costheta
   fmul Axis.y
   fmul Axis.y
   fadd costheta
   fmul Point.y
   fadd
   
   fld1
   fsub costheta
   fmul Axis.y
   fmul Axis.z
   fld Axis.x
   fmul sintheta
   fsub
   fmul Point.z
   fadd
   
   ;q.z = ((1 - costheta) * r.x * r.z - r.y * sintheta) * p.x;
   ;q.z += ((1 - costheta) * r.y * r.z + r.x * sintheta) * p.y;
   ;q.z += (costheta + (1 - costheta) * r.z * r.z) * p.z;
   fld1
   fsub costheta
   fmul Axis.x
   fmul Axis.z
   fld Axis.y
   fmul sintheta
   fsub
   fmul Point.x
   
   fld1
   fsub costheta
   fmul Axis.y
   fmul Axis.z
   fld Axis.x
   fmul sintheta
   fadd
   fmul Point.y
   fadd
   
   fld1
   fsub costheta
   fmul Axis.z
   fmul Axis.z
   fadd costheta
   fmul Point.z
   fadd
endm
   
;Given an Edge and a Normal of a Triangle,
;find a vector orthogonal to both.
Vec3_OrthoNormal macro vOut,vA,vB,vNormal
    ;Given that vNorm = vA x VB
    ;find (vA x vB) x (vA-vB)
    Vec3_Sub vA,vB
    Vec3_Stow vOut
    Vec3_Cross vNormal, vOut
    Vec3_Stow vOut
endm

;Given two vectors, return smallest axial values on FPU
;Warning - dont use eax!
Vec3_Minima macro vA, vB
	fMin vA.x, vB.x
	fMin vA.y, vB.y
	fMin vA.z, vB.z
endm

;Given two vectors, return largest axial values on FPU
;Warning - dont use eax!
Vec3_Maxima macro vA, vB
	fMax vA.x, vB.x
	fMax vA.y, vB.y
	fMax vA.z, vB.z
endm

endif