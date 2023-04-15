;August 7, 2006
;Homer's Intersection-related stuff 
;Various handy procedures for vector math etc


;Calculate DotProduct of two Vectors
;Leave result on fpu
Vec3Dot macro pv1:req,pv2:req
push eax
push ebx
mov eax,pv1
mov ebx,pv2
fld [eax].Vec3.x
fmul [ebx].Vec3.x
fld [eax].Vec3.y
fmul [ebx].Vec3.y
fld [eax].Vec3.z
fmul [ebx].Vec3.z
fadd
fadd
pop ebx
pop eax
endm

;Calculate CrossProduct of two Vectors
Vec3Cross proc pOut, pv1, pv2
mov eax,pv1
mov ebx,pv2
mov ecx,pOut
assume eax:ptr Vec3
assume ebx:ptr Vec3
assume ecx:ptr Vec3

fld [eax].y
fmul [ebx].z
fld [eax].z
fmul [ebx].y
fsub
fstp [ecx].x

fld [eax].z
fmul [ebx].x
fld [eax].x
fmul [ebx].z
fsub
fstp [ecx].y

fld [eax].x
fmul [ebx].y
fld [eax].y
fmul [ebx].x
fsub
fstp [ecx].z

assume eax:nothing
assume ebx:nothing
assume ecx:nothing
ret
Vec3Cross endp	



;Calculate the shortest distance from the Origin to a Plane
;PlaneD = the DotProduct of the Plane's Normal (N)
;and any Point that is known to lay on the Plane
;PlaneD= N . A
;PlaneD=(Nx * Ax) + (Ny * Ay) + (Nz * Az)
CalculatePlaneD proc pvNormal, pvPointOnPlane
	Vec3Dot pvNormal, pvPointOnPlane
	ret
CalculatePlaneD endp

;Normalize (Unitize) a vector
Normalize proc uses ebx pNormal
local fLen:FLOAT
mov ebx,pNormal                        ;First calculate the Length of the un-normalized Vector
fld [ebx].D3DXVECTOR3.x     
fmul [ebx].D3DXVECTOR3.x
fld [ebx].D3DXVECTOR3.y
fmul [ebx].D3DXVECTOR3.y
fld [ebx].D3DXVECTOR3.z
fmul [ebx].D3DXVECTOR3.z
fadd
fadd
fsqrt   
fstp fLen                     ;fLen= sqrt (x^2 + Y^2 + z^2)
fld [ebx].D3DXVECTOR3.z       ;Then, Divide Vector Components by fLen to Unitize them (without altering the Vector)
fdiv fLen
fstp [ebx].D3DXVECTOR3.z
fld [ebx].D3DXVECTOR3.y
fdiv fLen
fstp [ebx].D3DXVECTOR3.y
fld [ebx].D3DXVECTOR3.x
fdiv fLen
fstp [ebx].D3DXVECTOR3.x
ret
Normalize endp

;Calculate the SurfaceNormal Vector of a Triangle...
;Last Modified Jan. 23, 2006
SurfaceNormal proc pNormalOut, pTriangleIn
LOCAL edge1:Vec3
LOCAL edge2:Vec3

mov eax,pTriangleIn	;eax points to Vertex0
mov ebx,eax
add ebx,sizeof Vec3	;ebx points to Vertex1
mov ecx,ebx
add ecx,sizeof Vec3	;ecx points to Vertex2

;Edge1=Vertex1-Vertex0
fld  [ebx].Vec3.x
fsub [eax].Vec3.x
fstp edge1.x
fld  [ebx].Vec3.y
fsub [eax].Vec3.y
fstp edge1.y
fld  [ebx].Vec3.z
fsub [eax].Vec3.z
fstp edge1.z

;Edge2=Vertex2-Vertex0
fld  [ecx].Vec3.x
fsub [eax].Vec3.x
fstp edge2.x
fld  [ecx].Vec3.y
fsub [eax].Vec3.y
fstp edge2.y
fld  [ecx].Vec3.z
fsub [eax].Vec3.z
fstp edge2.z

invoke Vec3Cross, pNormalOut, addr edge1, addr edge2
invoke Normalize, pNormalOut
ret
SurfaceNormal endp

;Create a Normalized Vector from PointA to PointB
VectorFromPoints proc uses edi ebx pPointA,pPointB,pvOut	
	;First, create a Vector from A to B
	mov eax,pPointA
	mov ebx,pPointB
	mov edi,pvOut
	fld [ebx].Vec3.x
	fld [ebx].Vec3.y
	fld [ebx].Vec3.z
	fsub [eax].Vec3.z
	fstp [edi].Vec3.z
	fsub [eax].Vec3.y
	fstp [edi].Vec3.y
	fsub [eax].Vec3.x
	fstp [edi].Vec3.x

	;Next, obtain the Normal of the Vector from A to B
	invoke Normalize,edi
	ret
VectorFromPoints endp

;Tests a 2D circle and a 2D axis-aligned box
;Returns a signed value on the FPU...
;Positive values indicate a penetration, and
;negative values indicate a separation.
Intersection2D_CircleWithBox proc uses ebx fBoxMinX,fBoxMaxX,fBoxMinZ,fBoxMaxZ,fCircleX,fCircleZ,fRadius

    ;find the distance from the sphere to the box
    ;(We'll abuse Pythagorus Theorem to determine penetration depth)
	fld fRadius
	fmul fRadius

    fMin fCircleX,fBoxMinX
	fstpReg ebx
    .if ebx==fCircleX
		fld fCircleX
		fsub fBoxMinX
    .else
    	fMax fCircleX,fBoxMinX
   		fstpReg ebx
    	.if ebx==fCircleX
			fld fCircleX
			fsub fBoxMaxX
		.endif
	.endif
	fmul st(0),st(0)	;Square the axial distance
    
    fMin fCircleZ,fBoxMinZ
    fstpReg ebx
    .if ebx==fCircleZ
		fld fCircleZ
		fsub fBoxMinZ
	.else
		fMax fCircleZ,fBoxMaxZ
		fstpReg ebx
		.if ebx==fCircleZ
			fld fCircleZ
			fsub fBoxMaxZ
		.endif
	.endif
	fmul st(0),st(0)	;Square the axial distance

	;Big shout out to Pythagorus
	fadd				;Add the squared axial components

    ;Calculation penetration or separation distance
    ;(Here's where we compensate for the Radius)
    fsub
    ret
	
Intersection2D_CircleWithBox endp


;Shortest possible distance from point to plane
DistancePointPlane proc uses ebx ecx pPoint, pPlaneNormal,fPlaneD
	Vec3Dot pPlaneNormal,pPoint
	fsub fPlaneD
	ret
DistancePointPlane endp

;Find the distance from point to plane along specific Ray.
;ALWAYS leaves a result on the fpu.
;If they intersect, the distance to the intersection is returned on the fpu
;and eax will contain S_OK
;If they do not intersect, eax will contain E_FAIL
DistanceRayPlane proc pvRayOrigin,pvnRayVector,pvnPlaneNormal,fplaneD
LOCAL cosAlpha:REAL8
LOCAL deltaD:REAL8

    ;cosAlpha = DotProduct(vnRayVector ,  vnPlaneNormal );
	Vec3Dot pvnRayVector,pvnPlaneNormal
	fst cosAlpha
    ;if parallel to the plane (alpha=90)
    fcomp r4_0
    fjne mmk
    	return E_FAIL
   	mmk:   
    ;deltaD = planeD - DotProduct(vRayOrigin,vnPlaneNormal);  
    fld fplaneD
    Vec3Dot pvRayOrigin, pvnPlaneNormal
    fsub
    
    ;return (deltaD/cosAlpha);
    fdiv cosAlpha
	return S_OK
DistanceRayPlane endp


;Calculate intersection point on Plane
;from the Ray implied by two Points
;returns poutPoint=intersection point
;The length of the ray from A to B is
;divided by the distance to the intersection.
;This value is useful for interpolating UV coords.
IntersectionRayPlane proc pPointA,pPointB,pPlane,poutPoint
LOCAL vecRay:Vec3
LOCAL fDist:REAL8
;First, calculate a Normalized Vector from PointA to PointB
	invoke VectorFromPoints,pPointA,pPointB,addr vecRay
	
;Our next job is to calculate the Distance from the
;origin of the Ray to the intersection point on the Plane.
	mov eax,pPlane
	invoke DistanceRayPlane,pPointA,addr vecRay,pPlane,[eax].Vec4.w
	.if eax==E_FAIL
		DbgWarning "IntersectionRayPlane : failed DistanceRayPlane (ray and plane are parallel)"
		ret
	.else
		fstp fDist
;Multiply fDist by Ray Vector, then add Point1
		mov eax,pPointA
		mov ebx,poutPoint
		fld vecRay.x
		fld vecRay.y
		fld vecRay.z
		fmul fDist
		fadd [eax].Vec3.z
		fstp [ebx].Vec3.z
		fmul fDist
		fadd [eax].Vec3.y
		fstp [ebx].Vec3.y
		fmul fDist
		fadd [eax].Vec3.x
		fstp [ebx].Vec3.x	
;Finally, convert fDist to a magnitude (0.0 to 1.0)
		;Calculate 3d length from A to B
		;This is the squareroot of the dotproduct !!
		Vec3Dot pPointA,pPointB
		fsqrt
		;Divide by fDist		
		fdiv fDist
	.endif
	return S_OK
IntersectionRayPlane endp
