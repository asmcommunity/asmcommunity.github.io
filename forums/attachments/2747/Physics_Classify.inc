;Some code and data for classifying point/plane and face/plane

.data
MINIMUMRELATION  real4 0.8f		;fine-tuning of the dual heuristic in 'ChooseBestDividingPlane'
MINRELATIONSCALE real4 0.8f		;how much better is 'better', anyway? Angry Factor
r4_1             real4 1.0f

NONE equ 666

;Compares Point and Plane
;returns one of these values:
FRONT		equ 1
COPLANAR	equ 0
BACK		equ -1
SPLIT 		equ 666

;Compare Face and Plane - return values
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

;The 'plane thickness' tolerance for our math
ifndef fEpsilon
    fEpsilon equ fEpsilon_Penetration
endif

.code
;This procedure classifies a Point and a Plane
;Returns EAX = FRONT, COPLANAR or BACK
ClassifyPointPlane proc pPoint, pPlane
LOCAL fTemp


	;shortest distance from point to plane = (v1 . v2) - fPlaneD
	mov eax,pPoint
	mov edx,pPlane
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

;This function classifies a Face and a Plane given in the same Space.
;It can tell us about the intersection of a Face and Plane,
;in terms of what side of Plane each vertex is located :)
;Returns EAX = triple such as FRONTFRONTBACK or COPLANARBACKFRONT
;which classifies triangle points A, B and C respectively.
ClassifyFacePlane proc uses ebx pFace,pPlane
LOCAL results[3]

	mov ebx,pFace
	.if [ebx].Face.pV0==0 || [ebx].Face.pV1==0 || [ebx].Face.pV2==0
		DbgWarning "Bad data in ClassifyFacePlane"
		DbgWarning "NULL VERTEX POINTER"
		DbgHex pFace
		DbgHex [ebx].Face.pV0
		DbgHex [ebx].Face.pV1
		DbgHex [ebx].Face.pV2
		DbgHex [ebx].Face.pPlane
		invoke ExitProcess,0
	.elseif [ebx].Face.pPlane==0
		DbgWarning "Bad data in ClassifyFacePlane"
		DbgWarning "NULL PLANE POINTER"
		DbgHex pFace
		DbgHex [ebx].Face.pV0
		DbgHex [ebx].Face.pV1
		DbgHex [ebx].Face.pV2
		DbgHex [ebx].Face.pPlane
		invoke ExitProcess,0
		invoke ExitProcess,0
	.endif

	;Classify the three input vertices against the input plane
	invoke ClassifyPointPlane,[ebx].Face.pV0,pPlane
	mov results[0],eax
	invoke ClassifyPointPlane,[ebx].Face.pV1,pPlane
	mov results[4],eax
	invoke ClassifyPointPlane,[ebx].Face.pV2,pPlane
	mov results[8],eax
	
		
	;Analyze the three results
	.if results[0]==FRONT
		.if results[4]==FRONT 
			.if results[8]==FRONT
				mov edx,0
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
ClassifyFacePlane endp

;This version supports 'triangle' struct
ClassifyTrianglePlane proc pTriangle,pPlane
LOCAL results[3]

	.if pTriangle==0
		DbgWarning "Bad data in ClassifyFacePlane"
		DbgWarning "NULL VERTEX POINTER"
		invoke ExitProcess,0
	.elseif pPlane==0
		DbgWarning "Bad data in ClassifyFacePlane"
		DbgWarning "NULL PLANE POINTER"
		invoke ExitProcess,0
	.endif

	;Classify the three input vertices against the input plane
	mov ebx,pTriangle
	mov results[0],$invoke (ClassifyPointPlane,addr [ebx].triangle.v0,pPlane)
	mov results[4],$invoke (ClassifyPointPlane,addr [ebx].triangle.v1,pPlane)
	mov results[8],$invoke (ClassifyPointPlane,addr [ebx].triangle.v2,pPlane)
		
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

;Measure distance from given Point to closest point on given Edge
;returns value on FPU
DistancePointLine proc pvPoint, pvA,pvB
LOCAL vV:Vec3
LOCAL vW:Vec3
LOCAL c1,c2,b
LOCAL temp:Vec3
     ; v = P1 - P0
     mov edx,pvB
     mov eax,pvA
     Vec3_Sub [edx].Vec3,[eax].Vec3
     Vec3_Stow vV
     ; w = P - P0
     mov edx,pvPoint
     Vec3_Sub [edx].Vec3,[eax].Vec3
     Vec3_Stow vW
     
     Vec3_Dot vW,vV
     fstp c1
      .if $IsPositive(c1)==FALSE
            mov edx,pvPoint
            mov eax,pvA
            Vec3_Distance [edx].Vec3, [eax].Vec3 
            ret
      .endif
      
      Vec3_Dot vV,vV
      fstp c2
      
      .if $IsLessOrEqual(c2,c1) == TRUE
            mov edx,pvPoint
            mov eax,pvB
            Vec3_Distance [edx].Vec3, [eax].Vec3 
            ret
      .endif
      
      fld c1
      fdiv c2
      fstp b
      
      Vec3_Scale vV,b
      mov edx,pvA
      Vec3_AddFrom [edx].Vec3 
      Vec3_Stow temp ;Pb = P0 + b*v
      
      mov edx,pvPoint
      mov eax,pvB
      
      Vec3_Distance [edx].Vec3, temp
      ret
DistancePointLine endp

;returns closes point along Edge on FPU
ClosestPointOnLine proc pvA,pvB,pvPoint
LOCAL vC:Vec3
LOCAL vV:Vec3
LOCAL fd:real8
LOCAL ft
	; Determine t (the length of the vector from ‘a’ to ‘p’)
	mov edx,pvPoint
	mov eax,pvA
	Vec3_Sub [edx].Vec3,[eax].Vec3
	Vec3_Stow vC ; c = p-a
	mov edx,pvB
	Vec3_Sub [edx].Vec3,[eax].Vec3
	Vec3_Stow vV ; V = b-a
	
	Vec3_Length vV
	fstp fd
      
	Vec3_Normalize vV
	  
	Vec3_Dot vV,vC
	fstp ft

    .if $IsNegative(ft)==TRUE
        mov edx,pvA
        Vec3_Load [edx].Vec3
        ret
    .endif
    
    .if $IsGreater(ft,fd)==TRUE
        mov edx,pvB
        Vec3_Load [edx].Vec3
        ret
    .endif
  
    Vec3_Scale vV,ft
    mov edx,pvA
    Vec3_AddFrom [edx].Vec3
    ret
ClosestPointOnLine endp

;Returns the closest point on the boundary of a triangle on FPU
ClosestPointOnTriangle proc pFace,pvPoint
LOCAL Rab:Vec3
LOCAL Rbc:Vec3
LOCAL Rca:Vec3
LOCAL dAB,dBC,dCA
LOCAL vOut:Vec3
LOCAL temp:Vec3
LOCAL min
    mov edx,pFace
    invoke ClosestPointOnLine,[edx].Face.pV0, [edx].Face.pV1, pvPoint
    Vec3_Stow Rab
    mov edx,pFace
    invoke ClosestPointOnLine,[edx].Face.pV1, [edx].Face.pV2, pvPoint
    Vec3_Stow Rbc
    mov edx,pFace
    invoke ClosestPointOnLine,[edx].Face.pV2, [edx].Face.pV0, pvPoint
    Vec3_Stow Rca 
      
      
    ;dAB = lengthOfVector(p-Rab)
    mov edx,pvPoint
    Vec3_Sub [edx].Vec3,Rab  
    Vec3_Stow temp
    Vec3_Length temp
    fstp dAB
    
    ; dBC = lengthOfVector(p-Rbc)
    mov edx,pvPoint
    Vec3_Sub [edx].Vec3,Rbc  
    Vec3_Stow temp
    Vec3_Length temp
    fstp dBC
    
        
    ;  double dCA = lengthOfVector(p-Rca)
    mov edx,pvPoint
    Vec3_Sub [edx].Vec3,Rca 
    Vec3_Stow temp
    Vec3_Length temp
    fstp dCA
    
    
    ; double min = dAB
    m2m min,dAB
    
    Vec3_Copy vOut, Rab
    .if $IsLess(dBC , min) == TRUE
        m2m min , dBC
        Vec3_Copy vOut , Rbc
    .endif
    .if $IsLess(dCA , min) == TRUE
       Vec3_Copy vOut , Rca
    .endif
    
    Vec3_Load vOut
    ret
ClosestPointOnTriangle endp

;Compares a Point and a Triangle, using Barycentric technique
;to determine whether the given Point is located within the given 3D Triangle.
;Inputs are assumed to be defined in the same Space.
;ptrianglepoints       = triangle to be tested
;ptriangleplane        = plane of triangle
;pvTestPoint           = Point to be tested
;
;Returns TRUE/FALSE
;
;NOTES : The Barycentric coordinates (u,v) can be
;thought of as 'weights' for linear interpolation along
;the triangle edges AB and AC.
;It is therefore possible to convert these coordinates
;into TEXTURE COORDINATES via linear interpolation
;of the texcoords at the three points of the triangle 
PointInTriangle proc ptrianglepoints, ptriangleplane,pvTestPoint
LOCAL d:real8
LOCAL v0:Vec3
LOCAL v1:Vec3
LOCAL v2:Vec3
LOCAL dot00:real8
LOCAL dot01:real8
LOCAL dot02:real8
LOCAL dot11:real8
LOCAL dot12:real8
LOCAL invDenom:real8
LOCAL u:real8
LOCAL v:real8


    mov eax,pvTestPoint
    mov edx,ptriangleplane
    Vec3_Distance_Plane [eax].Vec3,[edx].Vec4
    fstpReg eax
    .ifBitSet eax,BIT31
        
        ; Compute edge vectors        
        ;v0 = C - A
        mov eax,ptrianglepoints
        Vec3_Sub [eax].triangle.v2,[eax].triangle.v0  ;C-A
        Vec3_Stow v0
        
        ;v1 = B - A
        Vec3_Sub [eax].triangle.v1,[eax].triangle.v0  ;B-A
        Vec3_Stow v1

        ;v2 = P - A
        mov edx,pvTestPoint     
        Vec3_Sub [edx].Vec3,[eax].triangle.v0  ;P-A
        Vec3_Stow v2
    
        ; Compute dot products
        ;dot00 = dot(v0, v0)
        ;dot01 = dot(v0, v1)
        ;dot02 = dot(v0, v2)
        ;dot11 = dot(v1, v1)
        ;dot12 = dot(v1, v2)
        Vec3_Dot v0,v0
        fstp dot00
        Vec3_Dot v0,v1
        fstp dot01
        Vec3_Dot v0,v2
        fstp dot02
        Vec3_Dot v1,v1
        fstp dot11
        Vec3_Dot v1,v2
        fstp dot12
    
        ; Compute barycentric coordinates:
        ;invDenom = 1 / (dot00 * dot11 - dot01 * dot01)
        ;u = (dot11 * dot02 - dot01 * dot12) * invDenom
        ;v = (dot00 * dot12 - dot01 * dot02) * invDenom
        fld1
        fld  dot00
        fmul dot11
        fld  dot01
        fmul st(0),st(0)
        fsub
        fdiv
        fst invDenom
        
        fld  dot11
        fmul dot02
        fld  dot01
        fmul dot12
        fsub
        fmul ;invDenom
        fstp u
        
        fld  dot00
        fmul dot12
        fld  dot01
        fmul dot02
        fsub
        fmul invDenom
        fstp v
    
        ; Check if point is in triangle
        ;return (u > 0) && (v > 0) && (u + v < 1)
        .if $IsPositive(u)==TRUE && $IsPositive(v)==TRUE
            fld u
            fadd v
            fld1
            fjge @F
            return TRUE
        .endif
    .endif
    @@:
    mov eax, FALSE
    ret
PointInTriangle endp

;Detect intersection of Edge and Plane
;Returns Clear, or Colliding (fpu contains point of intersection)
Edge_intersect_Plane proc pv0,pv1,pPlane,pPointOnPlane
LOCAL u:Vec3,w:Vec3
LOCAL D,N,tmp
    mov edx,pv0
    mov eax,pv1
    Vec3_Sub [eax].Vec3,[edx].Vec3
    Vec3_Stow u
    mov eax,pPointOnPlane
    Vec3_Sub [edx].Vec3,[eax].Vec3
    Vec3_Stow w

    mov edx,pPlane
    Vec3_Dot [edx].Vec3,w
    fchs
    fstp N
    
    Vec3_Dot [edx].Vec3,u
    fst D
    fabs
    fstp tmp
    .if $IsLess(tmp , fEpsilon)==TRUE ; segment is parallel to plane
        .if $IsLess(N, fEpsilon)==TRUE ; segment lies in plane
            DbgWarning "Edge in Plane"
            int 3
        .else
            return Clear
        .endif
    .endif
    ; they are not parallel
    
    ; compute intersect param
    fld N
    fdiv D
    fstp tmp
    .if $IsNegative(tmp)==TRUE || $IsGreater(tmp,r4_1)==TRUE
        return Clear
    .endif

    ; compute segment intersect point
    Vec3_Scale u,tmp
    mov edx,pv0
    Vec3_AddFrom [edx].Vec3
    
    return Colliding
Edge_intersect_Plane endp

;I'll use this to test edges of coplanar triangles
;Find intersection of input edges P1-P2 and P3-P4
;Returns FALSE (no intersection) or TRUE (FPU contains Vec3 point of intersection)
;
;Remarks:
;This is actually finding the length of the shortest line between two lines,
;if its less than Tolerance, we call it a Hit
;Thus we are detecting 'near misses' within tolerance as hits.
Edge_intersect_Edge proc p1,p2,p3,p4
LOCAL tmp:Vec3
LOCAL vA:Vec3
LOCAL vB:Vec3
LOCAL p13:Vec3,p43:Vec3,p21:Vec3
LOCAL d1343:real8,d4321:real8,d1321:real8,d4343:real8,d2121:real8
LOCAL numer:real8,denom:real8,mua:real8,mub:real8

    mov eax,p1
    mov edx,p3
    Vec3_Sub [eax].Vec3,[edx].Vec3
    Vec3_Stow p13
    mov eax,p4
    Vec3_Sub [eax].Vec3,[edx].Vec3
    Vec3_Stow p43
    
   fld p43.x
   fabs
   fstp tmp.x
   fld p43.y
   fabs
   fstp tmp.y
   fld p43.z
   fabs
   fstp tmp.z
   .if $IsLess(tmp.x,fEpsilon)==TRUE && $IsLess(tmp.y, fEpsilon)==TRUE && $IsLess(tmp.z, fEpsilon)==TRUE
      return FALSE
   .endif
   
   mov eax,p2
   mov edx,p1
   Vec3_Sub [eax].Vec3,[edx].Vec3
   Vec3_Stow p21
   fld p21.x
   fabs
   fstp tmp.x
   fld p21.y
   fabs
   fstp tmp.y
   fld p21.z
   fabs
   fstp tmp.z
   .if $IsLess(tmp.x,fEpsilon)==TRUE && $IsLess(tmp.y, fEpsilon)==TRUE && $IsLess(tmp.z, fEpsilon)==TRUE
      return FALSE
   .endif

   Vec3_Dot p13,p43
   fstp d1343
   Vec3_Dot p43,p21
   fstp d4321
   Vec3_Dot p13,p21
   fstp d1321  
   Vec3_Dot p43,p43
   fstp d4343 
   Vec3_Dot p21,p21
   fstp d2121 
   
   fld d2121
   fmul d4343
   fld d4321
   fmul d4321
   fsub
   fst denom
   fabs
   fstp tmp.x
   .if $IsLess(tmp.x,fEpsilon)==TRUE
      return FALSE
   .endif
   
   fld d1343
   fmul d4321
   fld d1321
   fmul d4343
   fsub
   fst numer
   fdiv denom
   fst mua
   fmul d4321
   fadd d1343
   fdiv d4343
   fstp mub
   
   Vec3_Scale p21, mua
   mov edx,p1
   Vec3_AddFrom [edx].Vec3
   Vec3_Stow vA

   Vec3_Scale p43, mub
   mov edx,p3
   Vec3_AddFrom [edx].Vec3
   Vec3_Stow vB
   
   Vec3_Distance vA,vB
   fabs
   fstp  tmp.x
   .if $IsLess(tmp.x, fEpsilon)==TRUE
       Vec3_Load vA
       ret
   .endif
   
   return FALSE
Edge_intersect_Edge endp

