
ifndef __VEC3_MATH
    __VEC3_MATH equ 1    
    
    ;My definition of a 3D and a 4D vector are the same thing.
    ;The reason is so we always have 16-byte alignment.
    ;For a Vec3, we just pretend the w field doesnt exist.
    Vec4 struct
        x real4 ?
        y real4 ?
        z real4 ?
        w real4 ?
    Vec4 ends
    Vec3 typedef Vec4


    ifndef f_OK
        ;We must include OA32's fMath macros (so far only 'fjle' in Normalize requires it)
        ;and they require that D3DXVECTOR3 is defined.
        ifndef D3DXVECTOR3
            D3DXVECTOR3 typedef Vec3
        endif
        % include &MacPath&fMath.inc
    endif

    
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
    Vec3_Sub macro sourceA, sourceB
        fld  sourceA.x
        fsub sourceB.x
        fld  sourceA.y
        fsub sourceB.y
        fld  sourceA.z
        fsub sourceB.z
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
    
    ;Multiply a vector by a scalar, result on fpu
    Vec3_Scale macro vector, scalar
        Vec3_Load vector
        fld scalar
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
    ;fpu = Normal(SourceA,SourceB)
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
endif