



.data
r4_half         real4 +0.5f
fEpsilon        real4 +1.0e-3
fMinusEpsilon   real4 -1.0e-3

.code
;Compares Point and Plane
;returns one of these values:
FRONT		equ 1
COPLANAR	equ 0
BACK		equ -1
SPLIT 		equ 666
NONE        equ 666

;Compare Triangle and Plane - return values
;These values describe exactly the outcome of cutting a triangle with a plane.
;They tell us what 'kind' of split, ie, what side of Plane each Vertex lays
FRONTFRONTFRONT 			equ FRONT
FRONTFRONTBACK				equ 10
FRONTFRONTCOPLANAR			equ 11
FRONTBACKFRONT				equ 12
FRONTBACKBACK				equ 13
FRONTBACKCOPLANAR			equ 14
FRONTCOPLANARFRONT			equ 15
FRONTCOPLANARBACK			equ 16
FRONTCOPLANARCOPLANAR		equ 17
;
BACKFRONTFRONT 				equ 18
BACKFRONTBACK				equ 19
BACKFRONTCOPLANAR			equ 20
BACKBACKFRONT				equ 21
BACKBACKBACK				equ BACK
BACKBACKCOPLANAR			equ 22
BACKCOPLANARFRONT			equ 23
BACKCOPLANARBACK			equ 24
BACKCOPLANARCOPLANAR		equ 25
;
COPLANARFRONTFRONT 			equ 26
COPLANARFRONTBACK			equ 27
COPLANARFRONTCOPLANAR		equ 28
COPLANARBACKFRONT			equ 29
COPLANARBACKBACK			equ 30
COPLANARBACKCOPLANAR		equ 31
COPLANARCOPLANARFRONT		equ 32
COPLANARCOPLANARBACK		equ 33
COPLANARCOPLANARCOPLANAR	equ COPLANAR

;Compare Quad and Plane - return values
FRONTFRONTFRONTFRONT        equ FRONT
FRONTFRONTFRONTBACK         equ 10
FRONTFRONTFRONTCOPLANAR     equ 11
FRONTFRONTBACKFRONT         equ 12
FRONTFRONTBACKBACK          equ 13
FRONTFRONTBACKCOPLANAR      equ 14
FRONTFRONTCOPLANARFRONT     equ 15
FRONTFRONTCOPLANARBACK      equ 16
FRONTFRONTCOPLANARCOPLANAR  equ 17
FRONTBACKFRONTFRONT         equ 18
FRONTBACKFRONTBACK          equ 19
FRONTBACKFRONTCOPLANAR      equ 20
FRONTBACKBACKFRONT          equ 21
FRONTBACKBACKBACK           equ 22
FRONTBACKBACKCOPLANAR       equ 23
FRONTBACKCOPLANARFRONT      equ 24
FRONTBACKCOPLANARBACK       equ 25
FRONTBACKCOPLANARCOPLANAR   equ 26
FRONTCOPLANARFRONTFRONT     equ 27
FRONTCOPLANARFRONTBACK      equ 28
FRONTCOPLANARFRONTCOPLANAR  equ 29
FRONTCOPLANARBACKFRONT      equ 30
FRONTCOPLANARBACKBACK       equ 31
FRONTCOPLANARBACKCOPLANAR   equ 32
FRONTCOPLANARCOPLANARFRONT  equ 33
FRONTCOPLANARCOPLANARBACK   equ 34
FRONTCOPLANARCOPLANARCOPLANAR equ 35

BACKFRONTFRONTFRONT         equ 36
BACKFRONTFRONTBACK          equ 37
BACKFRONTFRONTCOPLANAR      equ 38
BACKFRONTBACKFRONT          equ 39
BACKFRONTBACKBACK           equ 40
BACKFRONTBACKCOPLANAR       equ 41
BACKFRONTCOPLANARFRONT      equ 42
BACKFRONTCOPLANARBACK       equ 43
BACKFRONTCOPLANARCOPLANAR   equ 44
BACKBACKFRONTFRONT          equ 45
BACKBACKFRONTBACK           equ 46
BACKBACKFRONTCOPLANAR       equ 47
BACKBACKBACKFRONT           equ 48
BACKBACKBACKBACK            equ BACK
BACKBACKBACKCOPLANAR        equ 49
BACKBACKCOPLANARFRONT       equ 50
BACKBACKCOPLANARBACK        equ 51
BACKBACKCOPLANARCOPLANAR    equ 52
BACKCOPLANARFRONTFRONT      equ 53
BACKCOPLANARFRONTBACK       equ 54
BACKCOPLANARFRONTCOPLANAR   equ 55
BACKCOPLANARBACKFRONT       equ 56
BACKCOPLANARBACKBACK        equ 57
BACKCOPLANARBACKCOPLANAR    equ 58
BACKCOPLANARCOPLANARFRONT   equ 59
BACKCOPLANARCOPLANARBACK    equ 60
BACKCOPLANARCOPLANARCOPLANAR equ 61

COPLANARFRONTFRONTFRONT     equ 62
COPLANARFRONTFRONTBACK      equ 63
COPLANARFRONTFRONTCOPLANAR  equ 64
COPLANARFRONTBACKFRONT      equ 65
COPLANARFRONTBACKBACK       equ 66
COPLANARFRONTBACKCOPLANAR   equ 67
COPLANARFRONTCOPLANARFRONT  equ 68
COPLANARFRONTCOPLANARBACK   equ 69
COPLANARFRONTCOPLANARCOPLANAR equ 70
COPLANARBACKFRONTFRONT      equ 71
COPLANARBACKFRONTBACK       equ 72
COPLANARBACKFRONTCOPLANAR   equ 73
COPLANARBACKBACKFRONT       equ 74
COPLANARBACKBACKBACK        equ 75
COPLANARBACKBACKCOPLANAR    equ 76
COPLANARBACKCOPLANARFRONT   equ 77
COPLANARBACKCOPLANARBACK    equ 78
COPLANARBACKCOPLANARCOPLANAR equ 79
COPLANARCOPLANARFRONTFRONT  equ 80
COPLANARCOPLANARFRONTBACK   equ 81
COPLANARCOPLANARFRONTCOPLANAR equ 82
COPLANARCOPLANARBACKFRONT   equ 83
COPLANARCOPLANARBACKBACK    equ 84
COPLANARCOPLANARBACKCOPLANAR equ 85
COPLANARCOPLANARCOPLANARFRONT equ 86
COPLANARCOPLANARCOPLANARBACK equ 87
COPLANARCOPLANARCOPLANARCOPLANAR equ COPLANAR

;Test for intersection of 3D Ray and Sphere
;Returns EAX = FALSE (no intersection)
;or EAX = TRUE, FPU = distance along Ray from Origin to Intersection Point
Sphere_Intersect_Ray proc pvRayOrigin,pvRayDir,fRadius:real4
LOCAL _a:real4,_b:real4,_c:real4
LOCAL ftemp:real4
LOCAL q:real4,t0:real4,t1:real4
    ;Compute A, B and C coefficients
    mov edx,pvRayDir
    Vec3Dot [edx].Vec3, [edx].Vec3
    fstp _a                         ;_a = dir.dir
    
    mov eax,pvRayOrigin
    Vec3Dot [edx].Vec3, [eax].Vec3
    fadd st(0),st(0)
    fstp _b                         ;_b=2*(dir.origin)
    
    Vec3Dot [eax].Vec3, [eax].Vec3
    fld fRadius
    fmul st(0),st(0)
    fsub
    fstp _c                         ;_c=(origin.origin) - (r^2)

    ;Find discriminant = b^2 - 4ac
    fld _b
    fmul st(0),st(0)
    fld _a
    fmul _c
    fadd st(0),st(0)
    fadd st(0),st(0)
    fsub
    fst ftemp
    
    ; if discriminant is negative there are no real roots..
    ;return false as ray misses sphere
    .if $IsNegative(ftemp)==TRUE
        fUnload
        return FALSE
    .endif

    ; compute q
    fsqrt
    fstp ftemp
    
    .if $IsNegative(_b)
        ;q = (-b - distSqrt) / 2
        fld _b
        fchs
        fsub ftemp         
    .else
        ;q = (-b + distSqrt) / 2
        fld ftemp
        fsub _b
    .endif
    fmul r4_half
    fst q  
        
    ; compute t0 and t1
    fdiv _a
    fstp t0             ;t0 = q / a;
    fld _c
    fdiv q
    fstp t1             ;t1 = c / q;
    

    ; make sure t0 is smaller than t1
    .if $IsGreater(t1 , t0)==TRUE
        push t0
        m2m t0,t1,edx
        pop t1
    .endif

    ; if t1 is less than zero, the object is in the ray's negative direction
    ; and consequently the ray misses the sphere
    .if $IsNegative(t1)==TRUE
        return FALSE
    .endif

    ; if t0 is less than zero, the intersection point is at t1
    .if $IsNegative(t0)==TRUE
        fld t1
    ; else the intersection point is at t0
    .else
        fld t0
    .endif
    return TRUE
Sphere_Intersect_Ray endp


;This procedure classifies a Point and a Plane
ClassifyPointPlane proc pPoint, pPlane
LOCAL fTemp
	;shortest distance from point to plane = (v1 . v2) - fPlaneD
	invoke D3DXPlaneDotCoord,pPlane,pPoint
	fstp fTemp

	;Clamp result to zero when its quite close to zero
	fAbsMax fTemp,fEpsilon	;if unsigned value is smaller than epsilon,
	fstpReg edx
	.if edx==fEpsilon
		mov fTemp,0				;Set value to zero
	.endif

	;check result..
	mov edx,fTemp
	mov eax,edx
	and edx,7FFFFFFFh	;mask bit 31 - checking whether Value is +/- zero
	.if edx==0
		mov eax,COPLANAR
	;	DbgText "COPLANAR"
	.else
		and eax,80000000h	;mask all but bit 31 - just checking Sign
		.if eax==0
			;its positive
			mov eax,FRONT
		.else
			;its negative
			mov eax,BACK
		.endif	
	.endif
	ret
ClassifyPointPlane endp

;This function classifies a Triangle and a Plane...
;It can tell us about the intersection of a Face and Plane,
;in terms of what side of Plane each vertex is located :)
ClassifyTrianglePlane proc uses ebx pV0,pV1,pV2,pPlane
LOCAL results[3]

	;Classify the three input vertices against the input plane
	invoke ClassifyPointPlane,pV0,pPlane
	mov results[0],eax
	invoke ClassifyPointPlane,pV1,pPlane
	mov results[4],eax
	invoke ClassifyPointPlane,pV2,pPlane
	mov results[8],eax
	
		
	;Analyze the three results
	.if results[0]==FRONT
		.if results[4]==FRONT 
			.if results[8]==FRONT
				return FRONTFRONTFRONT
			.elseif results[8]==BACK
				return FRONTFRONTBACK
			.else	;results[8]==COPLANAR
				return FRONTFRONTCOPLANAR
			.endif		
			
		.elseif results[4]==BACK 
			.if results[8]==FRONT
				return FRONTBACKFRONT
			.elseif results[8]==BACK
				return FRONTBACKBACK		
			.else	;results[8]==COPLANAR
				return FRONTBACKCOPLANAR
			.endif
		
		.else ;results[4]==COPLANAR
			.if results[8]==FRONT
				return FRONTCOPLANARFRONT
			.elseif results[8]==BACK
				return FRONTCOPLANARBACK
			.else	;results[8]==COPLANAR
				return FRONTCOPLANARCOPLANAR
			.endif
		.endif			
			
	;---		
	.elseif results[0]==BACK
		.if results[4]==FRONT 
			.if results[8]==FRONT
				return BACKFRONTFRONT
			.elseif results[8]==BACK
				return BACKFRONTBACK
			.else	;results[8]==COPLANAR
				return BACKFRONTCOPLANAR
			.endif		
			
		.elseif results[4]==BACK 
			.if results[8]==FRONT
				return BACKBACKFRONT		
			.elseif results[8]==BACK
				return BACKBACKBACK
			.else	;results[8]==COPLANAR
				return BACKBACKCOPLANAR
			.endif
		
		.else ;results[4]==COPLANAR
			.if results[8]==FRONT
				return BACKCOPLANARFRONT
			.elseif results[8]==BACK
				return BACKCOPLANARBACK
			.else	;results[8]==COPLANAR
				return BACKCOPLANARCOPLANAR
			.endif
		.endif
	;---	
		
	.else	; results[0]==COPLANAR
		.if results[4]==FRONT 
			.if results[8]==FRONT
				return COPLANARFRONTFRONT
			.elseif results[8]==BACK
				return COPLANARFRONTBACK
			.else	;results[8]==COPLANAR
				return COPLANARFRONTCOPLANAR
			.endif		
			
		.elseif results[4]==BACK 
			.if results[8]==FRONT
				return COPLANARBACKFRONT		
			.elseif results[8]==BACK
				return COPLANARBACKBACK			
			.else	;results[8]==COPLANAR
				return COPLANARBACKCOPLANAR
			.endif
		
		.else ;results[4]==COPLANAR
			.if results[8]==FRONT
				return COPLANARCOPLANARFRONT
			.elseif results[8]==BACK	
				return COPLANARCOPLANARBACK
			.else	;results[8]==COPLANAR
				return COPLANARCOPLANARCOPLANAR
			.endif
		.endif
	.endif	
	ret
ClassifyTrianglePlane endp


;This function classifies a Quad and a Plane...
;It can tell us about the intersection of a Quad and Plane,
;in terms of what side of Plane each vertex is located :)
ClassifyQuadPlane proc uses ebx pV0,pV1,pV2,pV3,pPlane
LOCAL results[4]

	;Classify the three input vertices against the input plane
	invoke ClassifyPointPlane,pV0,pPlane
	mov results[0],eax
	invoke ClassifyPointPlane,pV1,pPlane
	mov results[4],eax
	invoke ClassifyPointPlane,pV2,pPlane
	mov results[8],eax
	invoke ClassifyPointPlane,pV3,pPlane
	mov results[12],eax
		
	;Analyze the four results
	.if results[0]==FRONT
		.if results[4]==FRONT 
			.if results[8]==FRONT
			    .if results[12]==FRONT
				    return FRONTFRONTFRONTFRONT
				.elseif results[12]==BACK
				    return FRONTFRONTFRONTBACK
				.else
				    return FRONTFRONTFRONTCOPLANAR
				.endif
			.elseif results[8]==BACK
			    .if results[12]==FRONT
				    return FRONTFRONTBACKFRONT
				.elseif results[12]==BACK
				    return FRONTFRONTBACKBACK
				.else
				    return FRONTFRONTBACKCOPLANAR
				.endif
			.else
			    .if results[12]==FRONT
				    return FRONTFRONTCOPLANARFRONT
				.elseif results[12]==BACK
				    return FRONTFRONTCOPLANARBACK
				.else
				    return FRONTFRONTCOPLANARCOPLANAR
				.endif
			.endif		
			
		.elseif results[4]==BACK 
			.if results[8]==FRONT
			    .if results[12]==FRONT
				    return FRONTBACKFRONTFRONT
				.elseif results[12]==BACK
				    return FRONTBACKFRONTBACK
				.else
				    return FRONTBACKFRONTCOPLANAR
				.endif
			.elseif results[8]==BACK
			    .if results[12]==FRONT
				    return FRONTBACKBACKFRONT
				.elseif results[12]==BACK
				    return FRONTBACKBACKBACK
				.else
				    return FRONTBACKBACKCOPLANAR
				.endif
			.else
			    .if results[12]==FRONT
				    return FRONTBACKCOPLANARFRONT
				.elseif results[12]==BACK
				    return FRONTBACKCOPLANARBACK
				.else
				    return FRONTBACKCOPLANARCOPLANAR
				.endif
			.endif
		
		.else
			.if results[8]==FRONT
			    .if results[12]==FRONT
				    return FRONTCOPLANARFRONTFRONT
				.elseif results[12]==BACK
				    return FRONTCOPLANARFRONTBACK
				.else
				    return FRONTCOPLANARFRONTCOPLANAR
				.endif
			.elseif results[8]==BACK
			    .if results[12]==FRONT
				    return FRONTCOPLANARBACKFRONT
				.elseif results[12]==BACK
				    return FRONTCOPLANARBACKBACK
				.else
				    return FRONTCOPLANARBACKCOPLANAR
				.endif
			.else
			    .if results[12]==FRONT
				    return FRONTCOPLANARCOPLANARFRONT
				.elseif results[12]==BACK
				    return FRONTCOPLANARCOPLANARBACK
				.else
				    return FRONTCOPLANARCOPLANARCOPLANAR
				.endif
			.endif
		.endif			
			
	;---		
	.elseif results[0]==BACK
		.if results[4]==FRONT 
			.if results[8]==FRONT
			    .if results[12]==FRONT
				    return BACKFRONTFRONTFRONT
				.elseif results[12]==BACK 
				    return BACKFRONTFRONTBACK
				.else
				    return BACKFRONTFRONTCOPLANAR
				.endif   
			.elseif results[8]==BACK
			    .if results[12]==FRONT
				    return BACKFRONTBACKFRONT
				.elseif results[12]==BACK
				    return BACKFRONTBACKBACK
				.else
				    return BACKFRONTBACKCOPLANAR
				.endif
			.else
			    .if results[12]==FRONT
				    return BACKFRONTCOPLANARFRONT
				.elseif results[12]==BACK
				    return BACKFRONTCOPLANARBACK
				.else
				    return BACKFRONTCOPLANARCOPLANAR
				.endif
			.endif		
			
		.elseif results[4]==BACK 
			.if results[8]==FRONT
			    .if results[12]==FRONT
				    return BACKBACKFRONTFRONT
				.elseif results[12]==BACK
				    return BACKBACKFRONTBACK
				.else
				    return BACKBACKFRONTCOPLANAR
				.endif
			.elseif results[8]==BACK
			    .if results[12]==FRONT
				    return BACKBACKBACKFRONT
				.elseif results[12]==BACK
				    return BACKBACKBACKBACK
				.else
				    return BACKBACKBACKCOPLANAR
				.endif
			.else
			     .if results[12]==FRONT
				    return BACKBACKCOPLANARFRONT
				 .elseif results[12]==BACK
				    return BACKBACKCOPLANARBACK
				 .else
				    return BACKBACKCOPLANARCOPLANAR
				 .endif
			.endif
		
		.else 
			.if results[8]==FRONT
			    .if results[12]==FRONT
				    return BACKCOPLANARFRONTFRONT
				.elseif results[12]==BACK
				    return BACKCOPLANARFRONTBACK
				.else
				    return BACKCOPLANARFRONTCOPLANAR
				.endif
			.elseif results[8]==BACK
			    .if results[12]==FRONT
				    return BACKCOPLANARBACKFRONT
				.elseif results[12]==BACK
				    return BACKCOPLANARBACKBACK
				.else
				    return BACKCOPLANARBACKCOPLANAR
				.endif
			.else
			    .if results[12]==FRONT
				    return BACKCOPLANARCOPLANARFRONT
				.elseif results[12]==BACK
				    return BACKCOPLANARCOPLANARBACK
				.else
				    return BACKCOPLANARCOPLANARCOPLANAR
				.endif
			.endif
		.endif
	;---	
		
	.else
		.if results[4]==FRONT 
			.if results[8]==FRONT
			    .if results[12]==FRONT
				    return COPLANARFRONTFRONTFRONT
				.elseif results[12]==BACK
				    return COPLANARFRONTFRONTBACK
				.else
				    return COPLANARFRONTFRONTCOPLANAR
				.endif
			.elseif results[8]==BACK
			    .if results[12]==FRONT
				    return COPLANARFRONTBACKFRONT
				.elseif results[12]==BACK    
				    return COPLANARFRONTBACKBACK
				.else
				    return COPLANARFRONTBACKCOPLANAR
				.endif
			.else
			    .if results[12]==FRONT
				    return COPLANARFRONTCOPLANARFRONT
				.elseif results[12]==BACK
				    return COPLANARFRONTCOPLANARBACK
				.else
				    return COPLANARFRONTCOPLANARCOPLANAR
				.endif
			.endif		
			
		.elseif results[4]==BACK 
			.if results[8]==FRONT
			    .if results[12]==FRONT
				    return COPLANARBACKFRONTFRONT
				.elseif results[12]==BACK
				    return COPLANARBACKFRONTBACK
				.else
				    return COPLANARBACKFRONTCOPLANAR
				.endif
			.elseif results[8]==BACK
			    .if results[12]==FRONT
				    return COPLANARBACKBACKFRONT
				.elseif results[12]==BACK
				    return COPLANARBACKBACKBACK
				.else
				    return COPLANARBACKBACKCOPLANAR
				.endif
			.else
			    .if results[12]==FRONT
				    return COPLANARBACKCOPLANARFRONT
				.elseif results[12]==BACK
				    return COPLANARBACKCOPLANARBACK
				.else
				    return COPLANARBACKCOPLANARCOPLANAR
				.endif
			.endif
		
		.else
			.if results[8]==FRONT
			    .if results[12]==FRONT
				    return COPLANARCOPLANARFRONTFRONT
				.elseif results[12]==BACK
				    return COPLANARCOPLANARFRONTBACK
				.else
				    return COPLANARCOPLANARFRONTCOPLANAR
				.endif
			.elseif results[8]==BACK	
			    .if results[12]==FRONT
				    return COPLANARCOPLANARBACKFRONT
				.elseif results[12]==BACK
				    return COPLANARCOPLANARBACKBACK
				.else
				    return COPLANARCOPLANARBACKCOPLANAR
				.endif
			.else
			    .if results[12]==FRONT
				    return COPLANARCOPLANARCOPLANARFRONT
				.elseif results[12]==BACK
				    return COPLANARCOPLANARCOPLANARBACK
				.else
				    return COPLANARCOPLANARCOPLANARCOPLANAR
				.endif
			.endif
		.endif
	.endif	
	ret
ClassifyQuadPlane endp


