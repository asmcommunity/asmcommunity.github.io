;Bugs fixed in OrthonormalizeOrientation and Normalize

LPMAT33 typedef ptr Mat33
LPVEC3 typedef ptr Vec3

.data
r4_1em10 real4 1.0e-10
fEpsilon real4 0.0001f
r4_2 real4 2.0f
r8_0 real8 0.0f
.code

ifndef Quaternion
	Quaternion typedef D3DXQUATERNION
endif

Mat33 struct
	union
		m00 real4 ?
		M0	real4 ?
	ends
	union
		m01 real4 ?
		M1  real4 ?
	ends
	union
		m02 real4 ?
		M2  real4 ?
	ends
	union
		m10 real4 ?
		M3  real4 ?
	ends
	union
		m11 real4 ?
		M4  real4 ?
	ends
	union
		m12 real4 ?
		M5  real4 ?
	ends
	union
		m20 real4 ?
		M6 real4 ?
	ends
	union
		m21 real4 ?
		M7  real4 ?
	ends
	union
		m22 real4 ?
		M8  real4 ?
	ends
Mat33 ends

.data
fEpsilon_QuatNormality real4 0.0001f
r4_Half		real4 0.50f
r4_Quarter	real4 0.25f
r4_0pt2		real4 0.2f
r4_9 		real4 9.0f
r4_100		real4 100.0f
r4_1em20 real4 1.0e-20
.code
;       |   m[0]        m[1]        m[2]    |
;       |   m[3]        m[4]        m[5]    |           1D array
;       |   m[6]        m[7]        m[8]    |

;       |   n[0][0]     n[0][1]     n[0][2] |
;       |   n[1][0]     n[1][1]     n[1][2] |           2D array
;       |   n[2][0]     n[2][1]     n[2][2] |

;Using this macro, we can address matrix elements in 2D
;For example:		 fld real4 ptr [eax+$Mat33_Indexed(2,1)]
;is equivalent to:	 fld [eax].Mat33.M7
;or if you prefer:	 fld [eax].Mat33.M21
$Mat33_index macro y,x
	push edx
	mov eax,sizeof Vec3
	mul y
	mov edx,x
	shl edx,2
	add eax,edx
	pop edx
	exitm <(eax)>
endm


__LoadFloat3 macro vec
	fld vec.x
	fld vec.y
	fld vec.z
endm

__StowFloat3 macro vec
	fstp vec.z
	fstp vec.y
	fstp vec.x
endm

__MoveFloat3 macro vecT, vecS
	m2m vecT.x, vecS.x
	m2m vecT.y, vecS.y
	m2m vecT.z, vecS.z
endm

;** leaves Vec3 on fpu
;Scales input Vec3 by input scalar (real4 or real8)
__ScaleFloat3 macro vec, fscalar
	fld vec.x
	fld vec.y
	fld vec.z
	fld fscalar
	fmul st(3),st(0)
	fmul st(2),st(0)
	fmul
endm

;** leaves Vec3 on fpu
;Scales input Vec3 by MINUS input scalar (real4 or real8)
__NegScaleFloat3 macro vec, fscalar
	fld vec.x
	fld vec.y
	fld vec.z
	fld fscalar
	fchs
	fmul st(3),st(0)
	fmul st(2),st(0)
	fmul
endm

;** leaves Vec3 on fpu
;fpu = v1 - v2
__SubFloat3 macro v1, v2
	fld v1.x
	fsub v2.x
	fld v1.y
	fsub v2.y
	fld v1.z
	fsub v2.z
endm

;** removes Vec3 from fpu
; Subtract the fpu Vec3 from v2 and store the result in v1
;v1 = v2 - fpu
__SubFromFloat3 macro v1, v2
	fld v2.z
	fsubr
	fstp v1.z
	fld v2.y
	fsubr
	fstp v1.y
	fld v2.x
	fsubr
	fstp v1.x	
endm

;** removes Vec3 from fpu
; Subtract v2 from the fpu Vec3 and store the result in v1
;v1 = fpu - v2
__SubStowFloat3 macro v1, v2
	fsub v2.z
	fstp v1.z
	fsub v2.y
	fstp v1.y
	fsub v2.x
	fstp v1.x
endm
	
;** leaves Vec3 on fpu
;fpu = v1 + v2
__AddFloat3  macro v1, v2
	fld v1.x
	fadd v2.x
	fld v1.y
	fadd v2.y
	fld v1.z
	fadd v2.z
endm


;** unloads Vec3 from fpu
;Adds input Vec3 to Vec3 on FPU
;Stores result in output Vec3
__AddToFloat3 macro vecOut, vecIn
	fadd vecIn.z
	fstp vecOut.z
	fadd vecIn.y
	fstp vecOut.y
	fadd vecIn.x
	fstp vecOut.x
endm

GenerateReasonableRandomReal proc fMinima, fMaxima
	invoke GetTickCount
	and eax,65535
	fildReg eax
	mov eax,65535
	fildReg eax
	fdiv
	fmul fMaxima
	fadd fMinima
	fabs
	ret
GenerateReasonableRandomReal endp

;Leaves result on fpustack
DotProduct macro A, B
	fld  A.x
	fmul B.x
	fld  A.y
	fmul B.y
	fld  A.z
	fmul B.z
	fadd
	fadd
endm

Vec3_Mag2 macro A, B
	fld  A.x
	fsub B.x
	fmul st(0),st(0)
	fld  A.y
	fsub B.y
	fmul st(0),st(0)
	fld  A.z
	fsub B.z
	fmul st(0),st(0)	
	fadd
	fadd
endm

;Leaves Vec3 on fpustack (in reverse order)
CrossProduct macro A, B
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

;;We need this macro to multiply a 3d vector by a 3x3 matrix.
;;vOut = Mat * V
;Vec3_mult_Mat33 macro Vout, Mat, V
;
;	;x = A.x * M.m[0] + A.y * M.m[1] + A.z * M.m[2]
;	;y = A.x * M.m[3] + A.y * M.m[4] + A.z * M.m[5]
;	;z = A.x * M.m[6] + A.y * M.m[7] + A.z * M.m[8]
;	fld  V.x
;	fmul  Mat.M0
;	fld  V.y
;	fmul  Mat.M1
;	fld  V.z
;	fmul  Mat.M2
;	fadd
;	fadd
;	fstp Vout.x
;	
;	fld  V.x
;	fmul  Mat.M3
;	fld  V.y
;	fmul  Mat.M4
;	fld  V.z
;	fmul  Mat.M5
;	fadd
;	fadd
;	fstp Vout.y
;
;	fld  V.x
;	fmul  Mat.M6
;	fld  V.y
;	fmul  Mat.M7
;	fld  V.z
;	fmul  Mat.M8
;	fadd
;	fadd
;	fstp Vout.z
;
;endm
;


;We need this macro to multiply a 3d vector by a 3x3 matrix.
;vOut = Mat * V
;Leaves result on fpu !!
Vec3_mult_Mat33 macro  Mat, V
	;x = A.x * M.m[0] + A.y * M.m[1] + A.z * M.m[2]
	;y = A.x * M.m[3] + A.y * M.m[4] + A.z * M.m[5]
	;z = A.x * M.m[6] + A.y * M.m[7] + A.z * M.m[8]
	lea edx,Mat.M0
	DotProduct V, [edx].Vec3
	lea edx,Mat.M3
	DotProduct V, [edx].Vec3
	lea edx,Mat.M6
	DotProduct V, [edx].Vec3	
endm


;This macro seems not to be very good, or is broken...
Vec3_normalize macro A
    ;modA2 = A.x * A.x + A.y * A.y + A.z * A.z;
    DotProduct A,A
    fstReg eax
    fcomp r4_1em10
    fjge @F
    	
    fld A.x
    fld A.y
    fld A.z
    fld1
    fldReg eax
    fsqrt
    fdiv
    fmul st(3),st(0)
    fmul st(2),st(0)
    fmul
    fstp A.z
    fstp A.y
    fstp A.x
    
@@:

endm



;Set matrix as the * or ~ operator on the vector :
;THIS IS ALSO KNOWNAS THE SKEW SYMMETRIC
;          [  0   -z    y ]
; Target = [  z    0   -x ]
;          [ -y    x    0 ]
Mat33_star macro Mat, Vec
	
	fldz
	fst  Mat.M0
	fst  Mat.M4
	fstp Mat.M8

	fld  Vec.Vec3.z
	fst Mat.M3
	fchs
	fstp Mat.M1
	
	fld  Vec.Vec3.y
	fst Mat.M2
	fchs
	fstp Mat.M6
	
	fld  Vec.Vec3.x
	fst Mat.M7
	fchs
	fstp Mat.M5
endm

;Target = Target * fScalar
Mat33_mult_Scalar macro pMat, fScalar
	push edx
	push ecx
	mov edx,pMat
	xor ecx,ecx
	.while ecx<9
		push ecx
		fld  REAL4 ptr[edx+ecx*4]
		fmul fScalar
		fstp REAL4 ptr[edx+ecx*4]
		pop ecx
		inc ecx
	.endw
	pop ecx
	pop edx
endm

Mat33_Add macro pMatOut,pMat1, pMat2
	pushad
	mov edx,pMat2
	mov eax,pMat1
	mov ebx,pMatOut
	xor ecx,ecx
	.while ecx<9
		fld  REAL4 ptr[edx+ecx*4]
		fadd REAL4 ptr[eax+ecx*4]
		fstp REAL4 ptr[ebx+ecx*4]
		inc ecx
	.endw
	popad
endm

;Out = Mat1 - Mat2
Mat33_Sub macro pMatOut,pMat1, pMat2
	pushad
	mov edx,pMat2
	mov eax,pMat1
	mov ebx,pMatOut
	xor ecx,ecx
	.while ecx<9
		fld  REAL4 ptr[eax+ecx*4]
		fsub REAL4 ptr[edx+ecx*4]
		fstp REAL4 ptr[ebx+ecx*4]
		inc ecx
	.endw
	popad
endm

Mat33_multiply proc uses ebx edx pTarget,pSource1:LPMAT33,pSource2:LPMAT33
	mov eax,pTarget
	mov ebx,pSource1
	mov edx,pSource2

;	m[0] = A.m[0] * B.m[0] + A.m[1] * B.m[3] + A.m[2] * B.m[6];
	fld  [ebx].Mat33.M0
	fmul [edx].Mat33.M0
	fld  [ebx].Mat33.M1
	fmul [edx].Mat33.M3
	fld  [ebx].Mat33.M2
	fmul [edx].Mat33.M6	
	fadd
	fadd
	fstp [eax].Mat33.M0

;	m[1] = A.m[0] * B.m[1] + A.m[1] * B.m[4] + A.m[2] * B.m[7];
	fld  [ebx].Mat33.M0
	fmul [edx].Mat33.M1
	fld  [ebx].Mat33.M1
	fmul [edx].Mat33.M4
	fld  [ebx].Mat33.M2
	fmul [edx].Mat33.M7	
	fadd
	fadd
	fstp [eax].Mat33.M1
;	m[2] = A.m[0] * B.m[2] + A.m[1] * B.m[5] + A.m[2] * B.m[8];
	fld  [ebx].Mat33.M0
	fmul [edx].Mat33.M2
	fld  [ebx].Mat33.M1
	fmul [edx].Mat33.M5
	fld  [ebx].Mat33.M2
	fmul [edx].Mat33.M8	
	fadd
	fadd
	fstp [eax].Mat33.M2
	
;	m[3] = A.m[3] * B.m[0] + A.m[4] * B.m[3] + A.m[5] * B.m[6];
	fld  [ebx].Mat33.M3
	fmul [edx].Mat33.M0
	fld  [ebx].Mat33.M4
	fmul [edx].Mat33.M3
	fld  [ebx].Mat33.M5
	fmul [edx].Mat33.M6	
	fadd
	fadd
	fstp [eax].Mat33.M3

;	m[4] = A.m[3] * B.m[1] + A.m[4] * B.m[4] + A.m[5] * B.m[7];
	fld  [ebx].Mat33.M3
	fmul [edx].Mat33.M1
	fld  [ebx].Mat33.M4
	fmul [edx].Mat33.M4
	fld  [ebx].Mat33.M5
	fmul [edx].Mat33.M7	
	fadd
	fadd
	fstp [eax].Mat33.M4

;	m[5] = A.m[3] * B.m[2] + A.m[4] * B.m[5] + A.m[5] * B.m[8];
	fld  [ebx].Mat33.M3
	fmul [edx].Mat33.M2
	fld  [ebx].Mat33.M4
	fmul [edx].Mat33.M5
	fld  [ebx].Mat33.M5
	fmul [edx].Mat33.M8	
	fadd
	fadd
	fstp [eax].Mat33.M5
	
;	m[6] = A.m[6] * B.m[0] + A.m[7] * B.m[3] + A.m[8] * B.m[6];
	fld  [ebx].Mat33.M6
	fmul [edx].Mat33.M0
	fld  [ebx].Mat33.M7
	fmul [edx].Mat33.M3
	fld  [ebx].Mat33.M8
	fmul [edx].Mat33.M6	
	fadd
	fadd
	fstp [eax].Mat33.M6
	
;	m[7] = A.m[6] * B.m[1] + A.m[7] * B.m[4] + A.m[8] * B.m[7];
	fld  [ebx].Mat33.M6
	fmul [edx].Mat33.M1
	fld  [ebx].Mat33.M7
	fmul [edx].Mat33.M4
	fld  [ebx].Mat33.M8
	fmul [edx].Mat33.M7	
	fadd
	fadd
	fstp [eax].Mat33.M7

;	m[8] = A.m[6] * B.m[2] + A.m[7] * B.m[5] + A.m[8] * B.m[8];
	fld  [ebx].Mat33.M6
	fmul [edx].Mat33.M2
	fld  [ebx].Mat33.M7
	fmul [edx].Mat33.M5
	fld  [ebx].Mat33.M8
	fmul [edx].Mat33.M8	
	fadd
	fadd
	fstp [eax].Mat33.M8
	ret
Mat33_multiply endp

;Multiply Mat33 with the Mat33 portion of a Mat44 (ie, rotational part)
Mat33_mult_Mat44 macro mat33Out, mat33In, mat44In
	fld  mat33In.M0
	fmul mat44In.m00		;orientation
	fld  mat33In.M1
	fmul mat44In.m10
	fld  mat33In.M2
	fmul mat44In.m20
	fadd
	fadd
	fstp mat33Out.M0

	fld  mat33In.M0
	fmul mat44In.m01
	fld  mat33In.M1
	fmul mat44In.m11
	fld  mat33In.M2
	fmul mat44In.m21
	fadd
	fadd
	fstp mat33Out.M1
	
	fld  mat33In.M0
	fmul mat44In.m02
	fld  mat33In.M1
	fmul mat44In.m12
	fld  mat33In.M2
	fmul mat44In.m22
	fadd
	fadd
	fstp mat33Out.M2
	
	fld  mat33In.M3
	fmul mat44In.m00
	fld  mat33In.M4
	fmul mat44In.m10
	fld  mat33In.M5
	fmul mat44In.m20
	fadd
	fadd
	fstp mat33Out.M3

	fld  mat33In.M3
	fmul mat44In.m01
	fld  mat33In.M4
	fmul mat44In.m11
	fld  mat33In.M5
	fmul mat44In.m21
	fadd
	fadd
	fstp mat33Out.M4
	
	fld  mat33In.M3
	fmul mat44In.m02
	fld  mat33In.M4
	fmul mat44In.m12
	fld  mat33In.M5
	fmul mat44In.m22
	fadd
	fadd
	fstp mat33Out.M5
	
	fld  mat33In.M6
	fmul mat44In.m00
	fld  mat33In.M7
	fmul mat44In.m10
	fld  mat33In.M8
	fmul mat44In.m20
	fadd
	fadd
	fstp mat33Out.M6

	fld  mat33In.M6
	fmul mat44In.m01
	fld  mat33In.M7
	fmul mat44In.m11
	fld  mat33In.M8
	fmul mat44In.m21
	fadd
	fadd
	fstp mat33Out.M7
	
	fld  mat33In.M6
	fmul mat44In.m02
	fld  mat33In.M7
	fmul mat44In.m12
	fld  mat33In.M8
	fmul mat44In.m22
	fadd
	fadd
	fstp mat33Out.M8
endm

;Add two 3x3 matrices
Mat33_addto macro pMatOut, pMatIn
	push eax
	push edx
	push ecx
	mov eax,pMatOut
	mov edx,pMatIn
	xor ecx,ecx
	.while ecx<9
		push ecx
		fld  REAL4 ptr[edx+ecx*4]
		fadd REAL4 ptr[eax+ecx*4]
		fstp REAL4 ptr[eax+ecx*4]
		pop ecx
		inc ecx
	.endw
	pop ecx
	pop edx
	pop eax
endm

Mat33_transpose macro Mat
	push  Mat.M1
	m2m  Mat.M1, Mat.M3
	pop Mat.M3
	push  Mat.M2
	m2m  Mat.M2, Mat.M6
	pop Mat.M6
	push Mat.M5
	m2m Mat.M5, Mat.M7
	pop Mat.M7
endm

Mat33_Transpose macro MatOut,MatIn
	push  MatIn.M1
	m2m  MatOut.M1, MatIn.M3
	pop MatOut.M3
	push  MatIn.M2
	m2m  MatOut.M2, MatIn.M6
	pop MatOut.M6
	push MatIn.M5
	m2m MatOut.M5, MatIn.M7
	pop MatOut.M7
endm

Mat33_neg macro Mat
	push ecx
	xor ecx,ecx
	.while ecx<9
		push ecx
		xor Mat.m00[ecx*4],80000000h
		pop ecx
		inc ecx
	.endw
	pop ecx
endm

;This is the EXPENSIVE way of orthogonalising a Mat33
;It's preferable to convert it to a Quaternion,
;use Quat-based math where possible, 
;normalize the Quat, and convert back to Mat33 !!!
OrthonormalizeOrientation proc pOrientation:LPMAT33
LOCAL vX:Vec3
LOCAL vY:Vec3
LOCAL vZ:Vec3

     ;X(Orientation(0,0),Orientation(1,0),Orientation(2,0))
     ;Y(Orientation(0,1),Orientation(1,1),Orientation(2,1))
	mov edx,pOrientation

	m2m vX.x,[edx].Mat33.M0 
	m2m vX.y,[edx].Mat33.M3 
	m2m vX.z,[edx].Mat33.M6
	;
	m2m vY.x,[edx].Mat33.M1 
	m2m vY.y,[edx].Mat33.M4 
	m2m vY.z,[edx].Mat33.M7  

    ;X.Normalize()
;    Vec3_normalize vX
    invoke D3DXVec3Normalize, addr vX,addr vX
    ;Z = CrossProduct(X,Y).Normalize()
    CrossProduct vX,vY
    fstp vZ.z
    fstp vZ.y
    fstp vZ.x
;    Vec3_normalize vZ
    invoke D3DXVec3Normalize,addr vZ,addr vZ
    ;Y = CrossProduct(Z,X).Normalize()
    CrossProduct vZ,vX
    fstp vY.z
    fstp vY.y
    fstp vY.x   
;    Vec3_normalize vY
    invoke D3DXVec3Normalize,addr vY,addr vY
    
    ;Orientation(0,0) = X(0); Orientation(0,1) = Y(0); Orientation(0,2) = Z(0);
    mov edx,pOrientation
    m2m [edx].Mat33.M0, vX.x
    m2m [edx].Mat33.M1, vY.x
    m2m [edx].Mat33.M2, vZ.x
    
    ;Orientation(1,0) = X(1); Orientation(1,1) = Y(1); Orientation(1,2) = Z(1);
    m2m [edx].Mat33.M3, vX.y
    m2m [edx].Mat33.M4, vY.y
    m2m [edx].Mat33.M5, vZ.y
    
    ;Orientation(2,0) = X(2); Orientation(2,1) = Y(2); Orientation(2,2) = Z(2);
    m2m [edx].Mat33.M6, vX.z
    m2m [edx].Mat33.M7, vY.z
    m2m [edx].Mat33.M8, vZ.z
	ret
OrthonormalizeOrientation endp

Mat33_identity macro Mat
	fld1
	fst  Mat.M0
	fst  Mat.M4
	fstp Mat.M8
	fldz
	fst  Mat.M1
	fst  Mat.M2
	fst  Mat.M3
	fst  Mat.M5
	fst  Mat.M6
	fstp Mat.M7
endm

;Leaves value on FPU Stack !!!
Mat33_det macro Mat

;	return m[0] * (m[4] * m[8] - m[5] * m[7])
;		- m[1] * (m[3] * m[8] - m[5] * m[6])
;		+ m[2] * (m[3] * m[7] - m[4] * m[6]);

	fld  Mat.M4
	fmul Mat.M8
	fld  Mat.M5
	fmul Mat.M7	
	fsub
	fmul Mat.M0
	
	fld  Mat.M3
	fmul Mat.M8
	fld  Mat.M5
	fmul Mat.M6	
	fsub
	fmul Mat.M1
	
	fsub
	
	fld  Mat.M3
	fmul Mat.M7
	fld  Mat.M4
	fmul Mat.M6	
	fsub
	fmul Mat.M2
	
	fadd

endm

;Returns TRUE/FALSE in eax (failure means the matrix is not orthogonal, so cannot be inverted)
Mat33_inv proc pMatOut,pMat
LOCAL detM:real8
LOCAL oneOverDetM:real8
LOCAL temp:real4

	mov eax,pMat
	Mat33_det [eax].Mat33
	fst detM

;	if (fabs(detM) < 1e-20)                     // cant invert this
	fabs
	fstp temp
	fMin temp, r4_1em20
	fstpReg eax
	.if eax==temp
		return FALSE
	.endif

; oneOverDetM = 1.0 / detM;     // one over det M
	fld1
	fdiv detM
	fstp oneOverDetM

	mov edx,pMatOut
	mov ebx,pMat
	
;	m[0] = oneOverDetM * (M.m[4] * M.m[8] - M.m[5] * M.m[7]);
	fld  [ebx].Mat33.M4
	fmul [ebx].Mat33.M8
	fld  [ebx].Mat33.M5
	fmul [ebx].Mat33.M7
	fsub
	fmul oneOverDetM
	fstp [edx].Mat33.M0

;	m[1] = -oneOverDetM * (M.m[1] * M.m[8] - M.m[2] * M.m[7]);
	fld  [ebx].Mat33.M1
	fmul [ebx].Mat33.M8
	fld  [ebx].Mat33.M2
	fmul [ebx].Mat33.M7
	fsub
	fmul oneOverDetM
	fchs
	fstp [edx].Mat33.M1

;	m[2] = oneOverDetM * (M.m[1] * M.m[5] - M.m[2] * M.m[4]);
	fld  [ebx].Mat33.M1
	fmul [ebx].Mat33.M5
	fld  [ebx].Mat33.M2
	fmul [ebx].Mat33.M4
	fsub
	fmul oneOverDetM
	fstp [edx].Mat33.M2

;	m[3] = -oneOverDetM * (M.m[3] * M.m[8] - M.m[5] * M.m[6]);
	fld  [ebx].Mat33.M3
	fmul [ebx].Mat33.M8
	fld  [ebx].Mat33.M5
	fmul [ebx].Mat33.M6
	fsub
	fmul oneOverDetM
	fchs
	fstp [edx].Mat33.M3

;	m[4] = oneOverDetM * (M.m[0] * M.m[8] - M.m[2] * M.m[6]);
	fld  [ebx].Mat33.M0
	fmul [ebx].Mat33.M8
	fld  [ebx].Mat33.M2
	fmul [ebx].Mat33.M6
	fsub
	fmul oneOverDetM
	fstp [edx].Mat33.M4

;	m[5] = -oneOverDetM * (M.m[0] * M.m[5] - M.m[2] * M.m[3]);
	fld  [ebx].Mat33.M0
	fmul [ebx].Mat33.M5
	fld  [ebx].Mat33.M2
	fmul [ebx].Mat33.M3
	fsub
	fmul oneOverDetM
	fchs
	fstp [edx].Mat33.M5

;	m[6] = oneOverDetM * (M.m[3] * M.m[7] - M.m[4] * M.m[6]);
	fld  [ebx].Mat33.M3
	fmul [ebx].Mat33.M7
	fld  [ebx].Mat33.M4
	fmul [ebx].Mat33.M6
	fsub
	fmul oneOverDetM
	fstp [edx].Mat33.M6
	
;	m[7] = -oneOverDetM * (M.m[0] * M.m[7] - M.m[1] * M.m[6]);
	fld  [ebx].Mat33.M0
	fmul [ebx].Mat33.M7
	fld  [ebx].Mat33.M1
	fmul [ebx].Mat33.M6
	fsub
	fmul oneOverDetM
	fchs
	fstp [edx].Mat33.M7

;	m[8] = oneOverDetM * (M.m[0] * M.m[4] - M.m[1] * M.m[3]);
	fld  [ebx].Mat33.M0
	fmul [ebx].Mat33.M4
	fld  [ebx].Mat33.M1
	fmul [ebx].Mat33.M3
	fsub
	fmul oneOverDetM
	fstp [edx].Mat33.M8
	return TRUE
Mat33_inv endp




;This checks that the input is a pure rotation matrix.
;The condition for this is:
;R' * R = I
;and
;det(R) =1

;00 01 02	0 1 2
;10 11 12   3 4 5
;20 21 22   6 7 8

isRotationMatrix proc pM
LOCAL ftemp:real4
	mov eax,pM
	;if (Math.abs(m[0][0]*m[0][1] + m[0][1]*m[1][1] + m[0][2]*m[1][2]) > epsilon) return false;
	fld  [eax].Mat33.M0
	fmul [eax].Mat33.M3	;should this be m3 ? (10) i thinkso - not 01
	fld  [eax].Mat33.M1
	fmul [eax].Mat33.M4
	fld  [eax].Mat33.M2
	fmul [eax].Mat33.M5
	fadd
	fadd
	fabs
	fstp ftemp
	fMax ftemp,fEpsilon
	fstpReg edx
	.if edx==ftemp
		return FALSE
	.endif
	
	;if (Math.abs(m[0][0]*m[2][0] + m[0][1]*m[2][1] + m[0][2]*m[2][2]) > epsilon) return false;
	fld  [eax].Mat33.M0
	fmul [eax].Mat33.M6
	fld  [eax].Mat33.M1
	fmul [eax].Mat33.M7
	fld  [eax].Mat33.M2
	fmul [eax].Mat33.M8
	fadd
	fadd
	fabs
	fstp ftemp
	fMax ftemp,fEpsilon
	fstpReg edx
	.if edx==ftemp
		return FALSE
	.endif

	;if (Math.abs(m[1][0]*m[2][0] + m[1][1]*m[2][1] + m[1][2]*m[2][2]) > epsilon) return false;
	fld  [eax].Mat33.M3
	fmul [eax].Mat33.M6
	fld  [eax].Mat33.M4
	fmul [eax].Mat33.M7
	fld  [eax].Mat33.M5
	fmul [eax].Mat33.M8
	fadd
	fadd
	fabs
	fstp ftemp
	fMax ftemp,fEpsilon
	fstpReg edx
	.if edx==ftemp
		return FALSE
	.endif


	;if (Math.abs(m[0][0]*m[0][0] + m[0][1]*m[0][1] + m[0][2]*m[0][2] - 1) > epsilon) return false;
	fld  [eax].Mat33.M0
	fmul [eax].Mat33.M0
	fld  [eax].Mat33.M1
	fmul [eax].Mat33.M1
	fld  [eax].Mat33.M2
	fmul [eax].Mat33.M2
	fadd
	fadd
	fabs
	fstp ftemp
	fMax ftemp,fEpsilon
	fstpReg edx
	.if edx==ftemp
		return FALSE
	.endif

	;if (Math.abs(m[1][0]*m[1][0] + m[1][1]*m[1][1] + m[1][2]*m[1][2] - 1) > epsilon) return false;
;00 01 02	0 1 2
;10 11 12   3 4 5
;20 21 22   6 7 8
	fld  [eax].Mat33.M3
	fmul [eax].Mat33.M3
	fld  [eax].Mat33.M4
	fmul [eax].Mat33.M4
	fld  [eax].Mat33.M5
	fmul [eax].Mat33.M5
	fadd
	fadd
	fabs
	fstp ftemp
	fMax ftemp,fEpsilon
	fstpReg edx
	.if edx==ftemp
		return FALSE
	.endif

	;if (Math.abs(m[2][0]*m[2][0] + m[2][1]*m[2][1] + m[2][2]*m[2][2] - 1) > epsilon) return false;
	fld  [eax].Mat33.M6
	fmul [eax].Mat33.M6
	fld  [eax].Mat33.M7
	fmul [eax].Mat33.M7
	fld  [eax].Mat33.M8
	fmul [eax].Mat33.M8
	fadd
	fadd
	fabs
	fstp ftemp
	fMax ftemp,fEpsilon
	fstpReg edx
	.if edx==ftemp
		return FALSE
	.endif
		
	;return (Math.abs(det(m)-1) < epsilon);
	fld1
	Mat33_det [eax].Mat33
	fsub
	fabs
	fstp ftemp
	fMax ftemp,fEpsilon
	fstpReg edx
	.if edx==ftemp
		mov eax,FALSE
	.else
		mov eax,TRUE
	.endif
	ret
isRotationMatrix endp



;This is an inline macro, its part of Mat33_eigenvectors (below)
EIGENROTATE macro a,i,j,k,l
      ;g=a.n[i][j]
      fld real4 ptr a[$Mat33_index(i,j)]
      fstp g
      ;h=a.n[k][l]
      fld real4 ptr a[$Mat33_index(k,l)]
      fstp h
      ;a.n[i][j]=g-s*(h+g*tau)
      fld g
      fld g
      fmul tau
      fadd h
      fmul s
      fsub
      fstp real4 ptr a[$Mat33_index(i,j)]
      ;a.n[k][l]=h+s*(g-h*tau);
      fld g
	  fld h
	  fmul tau
	  fsub
	  fmul s
	  fadd h
	  fstp  real4 ptr a[$Mat33_index(k,l)]
endm


;***********************************************************************
;eigenvectors
;   Find the eigen vectors and values for a matrix using the Jacobi rotation method.
;   Assumes the matrix is symmetric, i.e. the bottom half is ignored.
;
;params :             
;	pMatOut                  -   unit length eigen vectors returned here (in columns)
;	pVecOut                  -   eigen values returned here
;	pMatIn					 -   matrix to find eigen vectors for (symmetric)
;
;returns :		             -   0 OK, -1 error
;
;***********************************************************************
Mat33_eigenvectors proc pMatOut, pVecOut, pMatIn
local j, iq, ip, i, _end										;integers
local tresh, theta, tau, t, sm, s, h, g, cc	, ftemp, fabstemp, fbiggy			;floats
local TempMat33:Mat33
LOCAL b[3]:real4
LOCAL z[3]:real4
	invoke RtlMoveMemory,addr TempMat33,pMatIn,sizeof Mat33		; make a copy of MatIn (this gets trashed)

	mov edx,pMatOut
    Mat33_identity [edx].Mat33
    Mat33_identity TempMat33   
    
  	xor ecx,ecx
  	mov edx,pVecOut
  	.while ecx<3
		;b[ip]=VecOut.m[ip]=TempMat33.n[ip][ip]
		;z[ip]=0.0
		mov eax,$Mat33_index(ecx,ecx)
		fld real4 ptr TempMat33[eax]
		fst real4 ptr[edx+ecx*4]
		fstp b[ecx*4]
		mov  z[ecx*4],0
		inc ecx
    .endw

	xor ecx,ecx
	.while ecx<50
		mov i,ecx
		
		mov sm,0
		mov ip,0
		.while ip<2
			mov ecx,ip
			inc ecx
			.while ecx<3
				fld  real4 ptr TempMat33[$Mat33_index(ip,ecx)]
				fadd sm
				fstp sm
	            inc ecx
	        .endw	        
	        inc ip
        .endw
        .if sm == 0 || sm==80000000h	;zero (or negative zero, heh)
		    xor eax,eax					;=SUCCESS
		    jmp @F
        .endif

		.if i < 3
		    ;tresh=0.2f*float(fabs(sm)/(3*3))
		    fld sm
		    fdiv r4_9
		    fabs
		    fmul r4_0pt2
		    fstp tresh 
		.else
	        and tresh,0
	    .endif
	    
	    mov ip,0
		.while ip<2
			mov ecx,ip
			inc ecx
			mov iq,ecx
			.while iq<3
			
				m2m ftemp, real4 ptr TempMat33[$Mat33_index(ip,iq)]	
				
				fMax fabstemp, tresh
				fstp fbiggy		
			
		        ;g=100.0f*float(fabs(TempMat33.n[ip][iq]));
		        fld ftemp
		        fabs
		        fst fabstemp
		        fmul r4_100
		        fstp g

                ;if (i > 4  && (float)(fabs(VecOut.m[ip])+g) == (float)fabs(VecOut.m[ip])
                ; 			&& (float)(fabs(VecOut.m[iq])+g) == (float)fabs(VecOut.m[iq]))
                mov edx,pVecOut
                mov eax,ip
                fld real4 ptr [edx+eax*4]
                fabs
                fstReg ebx
                fadd g
                fstpReg eax
                .if eax==ebx
                	mov eax,iq
                	fld real4 ptr [edx+eax*4]
                	fabs
                	fstReg ebx
                	fadd g
                	fstpReg eax
                	mov edx,fabstemp
                	.if eax==ebx
	                	fldz           
	                    fstp real4 ptr TempMat33[$Mat33_index(ip,iq)]               
            		.elseif fbiggy==edx ;fabs(TempMat33.n[ip][iq]) > tresh) {
	                	;h=VecOut.m[iq]-VecOut.m[ip];
	                	mov edx,pVecOut
	                	mov eax,ip
	                	mov ebx,iq
	                	fld real4 ptr[edx+ebx*4]
	                	fsub real4 ptr[edx+eax*4]
	                	fst h
                                        
	                    ;if (fabs(h)+g) == fabs(h))
	                    fabs
	                    fstReg ebx
	                    fadd g
	                    fstpReg eax
	                    .if eax==ebx
	                    	;t=(TempMat33.n[ip][iq])/h;
	                    	fld ftemp
	                    	fdiv h
	                    	fstp t
	                    .else 
	                    	;theta=0.5f*h/(TempMat33.n[ip][iq]);
	                    	fld r4_Half
	                    	mul h
	                    	fdiv ftemp
	                    	fstp theta
	                    	
	                        ;t=1.0f/(float(fabs(theta))+float(sqrt(1.0f+theta*theta)));
	                        fld1                        
	                        fld theta
	                        fmul st(0),st(0)
	                        fld1
	                        fadd
	                        fsqrt
	                        fld theta
	                        fabs
	                        fadd
	                        fdiv
	                        fstp t
	                        
	                    	;if (theta < 0.0) 
	                    	mov eax,theta
	                    	and eax,80000000h
	                    	.if eax!=0
	                    		fld t
	                    		fchs
	                    		fstp t
	                    	.endif
	                    .endif
                    
		                ;cc=1.0f/float(sqrt(1.0f+t*t));
		                fld1
		                fld t
		                fmul st(0),st(0)
		                fld1
		                fadd
		                fsqrt
		                fdiv
		                fst cc
	
	                	;s=t*c
	                	fmul t
	                	fst s
	                	
		                ;tau=s/(1.0f+c);
		                fld1
		                fadd cc
		                fdiv
		                fstp tau
		                	                
	                    ;h=t*TempMat33.n[ip][iq];
	                    fld t
	                    fmul real4 ptr TempMat33[$Mat33_index(ip,iq)]
	                    fstp h
	
	                	;z[ip] -= h;
	                	;z[iq] += h;
	                	mov eax,ip
	                	mov ebx,iq
	                	fld  z[eax*4]
	                	fsub h
	                	fstp z[eax*4]                	
	                	fld  z[ebx*4]
	                	fadd h
	                	fstp z[ebx*4]
	                	
	                	;VecOut.m[ip] -= h
	                    ;VecOut.m[iq] += h
	                    mov edx,pVecOut
	                    fld real4 ptr[edx+eax*4]
	                    fsub h
	                    fstp real4 ptr[edx+eax*4]
	                    fld real4 ptr[edx+ebx*4]
	                    fadd h
	                    fstp real4 ptr[edx+ebx*4]
	                      
	                    fldz                  
	                	fstp real4 ptr TempMat33[$Mat33_index(ip,iq)]
	                	
		                xor ecx,ecx
		                .while ecx<ip
		                	push ecx
	                    	EIGENROTATE TempMat33,ecx,ip,ecx,iq
	                    	pop ecx
	                    	inc ecx
	                    .endw
	
						inc ecx	;ecx now = ip+1
						.while ecx<iq
							push ecx
	                    	EIGENROTATE TempMat33,ip,ecx,ecx,iq 
	                    	pop ecx
	                    	inc ecx
	                    .endw
	                    
	                    inc ecx ;ecx now = iq+1
	                	.while ecx<3
	                		push ecx
	                    	EIGENROTATE TempMat33,ip,ecx,iq,ecx
	                    	pop ecx
	                    	inc ecx
	                    .endw
	                    
	                    
	                    xor ecx,ecx
	                    .while ecx<3
	                   		push ecx
	                   		mov edx,pMatOut
	                    	EIGENROTATE [edx].Mat33,ecx,ip,ecx,iq
	                    	pop ecx
	                    	inc ecx
	                    .endw
               		.endif
			    .endif
			    
			    inc iq
            .endw
            inc ip
        .endw
        
    	xor ecx,ecx
    	mov edx,pVecOut
    	.while ecx<3
        	;b[ip] += z[ip]
        	fld  z[ecx*4]
        	fadd b[ecx*4]
        	fstp b[ecx*4]
	        ;VecOut.m[ip] = b[ip]
	        m2m [edx+ecx*4],b[ecx*4]
	        mov z[ecx*4],0
	        inc ecx
        .endw
        
        inc i
        mov ecx,i
        
    .endw
    mov eax,-1
@@:
ret
Mat33_eigenvectors endp

;Creates an Orthogonal set of Normal Vectors (axes) from a given Vector, as Columns of a Mat33
;DO NOT USE EDX
Mat33_OrthoNormalBasis macro Mat,V

	;The X column vector is taken directly from our collision Normal
    m2m Mat.m00, V.x
    m2m Mat.m10, V.y
    m2m Mat.m20, V.z
    
    ;Pick an axis which is orthogonal to the Major axis
    fAbsMax V.x, V.y
    fstpReg edx
    .if edx==V.x    
        fld Mat.m00
        fchs        
        fld Mat.m20
        
         ;s = 1 /sqrt(m20*m20 + m00*m00)
        ; The new X-axis is at right angles to the world Y-axis
     ;   m01 = m20*s
     ;   m11 = 0
     ;   m21 = m00*s

        ; The new Y-axis is at right angles to the new X- and Z- axes
      ;  m02 = m10*m01
      ;  m12 = m20*m01 - m00*m21
      ;  m22 = -m10*m01
      
         fld1
         fld Mat.m20
         fmul st(0),st(0)
         fld Mat.m00
         fmul st(0),st(0)
         fadd
         fsqrt
         fdiv
         
         fmul st(2),st(0)
         fmul
        mov  Mat.m11,0
        fstp Mat.m01       
        fstp Mat.m21
        fld  Mat.m10
        fmul Mat.m01
        fstp Mat.m02
        fld  Mat.m20
        fmul Mat.m01
        fld  Mat.m00
        fmul Mat.m21
        fsub        
        fstp Mat.m12
        fld  Mat.m10        
        fmul Mat.m01
        fchs
        fstp Mat.m22

    .else       
        fld Mat.m10
        fld Mat.m20
        fchs
        
        ;s = 1/sqrt(m20*m20 + m10*m10)
        ; The new X-axis is at right angles to the world X-axis
       ; m01 = 0
       ; m11 = -m20*s
       ; m21 = m10*s
        ; The new Y-axis is at right angles to the new X- and Z- axes
        ;m02 = m10*m21 - m20*m11
        ;m12 = -m00*m21
        ;m22 = m00*m11
        
         fld1
         fld Mat.m20
         fmul st(0),st(0)
         fld Mat.m10
         fmul st(0),st(0)
         fadd
         fsqrt
         fdiv
		 ;
		 fmul st(2),st(0)
		 fmul
		 ;
        mov  Mat.m01,0
        fstp Mat.m11
        fstp Mat.m21
        fld  Mat.m10
        fmul Mat.m21
        fld  Mat.m20
        fmul Mat.m11
        fsub
        fstp Mat.m02
        fld  Mat.m00        
        fmul Mat.m21
        fchs        
        fstp Mat.m12
        fld  Mat.m00
        fmul Mat.m11
        fstp Mat.m22

    .endif
     
endm

;Assumes that l2 and ftemp are declared locals in the calling proc
Quaternion_normalize macro _Quat, _length, _fun

;fun = (l2 + 1.0) / (2.0 * l2);
    fld1
    fadd _length
    fld _length
    fadd st(0),st(0)
    fdiv
    fstp _fun   
    
    ;quat *= fun
    fld _Quat.x
    fld _Quat.y
    fld _Quat.z
	fld _Quat.w
	fld _fun
	fmul st(4),st(0)
	fmul st(3),st(0)
	fmul st(2),st(0)
	fmul;st(1),st(0)...fUnload
	fstp _Quat.w
	fstp _Quat.z
	fstp _Quat.y
	fstp _Quat.x
	
	;_length *= fun * fun
	fld _fun
	fmul st(0),st(0)
	fmul _length
	fstp _length

endm

;returns value on fpu stack
Quaternion_length macro quat
	;l2 = qRotPos.x * qRotPos.x + qRotPos.y * qRotPos.y + qRotPos.z * qRotPos.z + qRotPos.w * qRotPos.w;
 	fld quat.x
 	fmul st(0),st(0)
 	fld quat.y
 	fmul st(0),st(0)	
 	fld quat.z
 	fmul st(0),st(0)	
 	fld quat.w
 	fmul st(0),st(0)
	fadd 
	fadd 	
	fadd 
endm



;***********************************************************************
;  Matrix3x3::fromQuatL2
;
;   params :
;		pm					- 	ptr to output matrix
;       pq                  -   ptr to quaternion to make into matrix
;       float l2            -   length^2 of quaternion
;
;   Create rotation matrix from the quaternion [x y z w].
;   w = cos(theta / 2)
;   [x y z] = axis * sin(theta / 2)
;
;   Will always be orthogonal even if quaternion not normalised because the
;   matrix is automatically scaled by l2.
;**********************************************************************/

Mat33_fromQuatL2 proc pm, pq, l2:real4
LOCAL x2,y2,z2
LOCAL xy, wz, xz, wy, yz, wx
LOCAL scale

    ;x2 = q.x * q.x;
    ;y2 = q.y * q.y;
    ;z2 = q.z * q.z;
    mov edx,pq
    fld  [edx].Quaternion.x
    fmul st(0),st(0)
    fstp x2
    fld  [edx].Quaternion.y
    fmul st(0),st(0)
    fstp y2   
    fld  [edx].Quaternion.z
    fmul st(0),st(0)
    fstp z2
    
    ;xy = q.x * q.y;
    fld  [edx].Quaternion.x
    fmul [edx].Quaternion.y
    fstp xy
    
    ;wz = q.w * q.z;
    fld  [edx].Quaternion.w
    fmul [edx].Quaternion.z
    fstp wz
    
    ;xz = q.x * q.z;
    fld  [edx].Quaternion.x
    fmul [edx].Quaternion.z
    fstp xz
    
    ;wy = q.w * q.y;
    fld  [edx].Quaternion.w
    fmul [edx].Quaternion.y
    fstp wy

    ;yz = q.y * q.z;
    fld  [edx].Quaternion.y
    fmul [edx].Quaternion.z
    fstp yz
    
    ;wx = q.w * q.x;
    fld  [edx].Quaternion.w
    fmul [edx].Quaternion.x
    fstp wx

    ;float scale = 2.0f / l2;                         // scale to make it orthogonal
    fld r4_2
    fdiv l2
    fstp scale
    DbgFloat scale
    
    ;m[0] = 1.0f - scale * (y2 + z2)
    mov edx,pm
    fld1
    fld y2
    fadd z2
    fmul scale
    fsub
    fstp [edx].Mat33.M0
    
   ; m[1] = scale * (xy - wz)
   fld xy
   fsub wz
   fmul scale
   fstp [edx].Mat33.M1
   
    ;m[2] = scale * (xz + wy)
   fld xz
   fadd wy
   fmul scale
   fstp [edx].Mat33.M2
    
    ;m[3] = scale * (xy + wz)
   fld xy
   fadd wz
   fmul scale
   fstp [edx].Mat33.M3
   
    ;m[4] = 1.0f - scale * (z2 + x2)
    fld1
   fld z2
   fadd x2
   fmul scale
   fsub
   fstp [edx].Mat33.M4

    ;m[5] = scale * (yz - wx)
   fld yz
   fsub wx
   fmul scale
   fstp [edx].Mat33.M5
   
    ;m[6] = scale * (xz - wy)
   fld xz
   fsub wy
   fmul scale
   fstp [edx].Mat33.M6
   
   ; m[7] = scale * (yz + wx)
   fld yz
   fadd wx
   fmul scale
   fstp [edx].Mat33.M7
   
   ; m[8] = 1.0f - scale * (x2 + y2)
   fld1
   fld x2
   fadd y2
   fmul scale
   fsub
   fstp [edx].Mat33.M8
	ret
Mat33_fromQuatL2 endp

;This macro calculates the Trace of a Mat33 (sum of Diagonals)
;Value returned on fpu stack
Mat33_trace macro Mat
	fld Mat.M0
	fadd Mat.M4
	fadd Mat.M8
endm

Mat44_trace macro Mat
	fld Mat.m00
	fadd Mat.m11
	fadd Mat.m22
	fadd Mat.m33
	DbgFloat Mat.m33
endm


;Returns value on fpu
Quaternion_mod2 macro Quat
	fld Quat.x
	fmul Quat.x
	fld Quat.y
	fmul Quat.y	
	fld Quat.z
	fmul Quat.z
	fld Quat.w
	fmul Quat.w
	fadd
	fadd 
	fadd
endm

Quaternion_frommMatrix proc pQuatOut, pMatIn
LOCAL S,T
	;Calculate the Trace of the matrix
	mov edx,pMatIn
	Mat33_trace [edx].Mat33
	fstp T
	;If the trace of the matrix is greater than zero, perform an "instant" calculation.
	mov eax,T
	and eax,80000000h
	.if eax==0
      ;S = 0.5 / sqrt(T)
      fld r4_Half
      fld T
      fsqrt
      fdiv
      fstp S
      mov eax,pQuatOut
      ;W = 0.25 / S
      fld r4_Quarter
      fdiv S
      fstp [eax].Quaternion.w
 	  ;X = ( mat[7] - mat[5] ) * S  
 	  fld  [edx].Mat33.M7
 	  fsub [edx].Mat33.M5
 	  fmul S
 	  fstp [eax].Quaternion.x
      ;Y = ( mat[2] - mat[6] ) * S
 	  fld  [edx].Mat33.M2
 	  fsub [edx].Mat33.M6
 	  fmul S
 	  fstp [eax].Quaternion.y
      ;Z = ( mat[3] - mat[1] ) * S
 	  fld  [edx].Mat33.M3
 	  fsub [edx].Mat33.M1
 	  fmul S
 	  fstp [eax].Quaternion.z
 	.else
 		DbgWarning "Trace less than zero - error"
 		invoke ExitProcess,0
;	    If the trace of the matrix is less than or equal to zero
;	    then identify which major diagonal element has the greatest
;	    value.
;	
;	    Depending on this value, calculate the following:
;	
;	      Column 0:
;	        S  = sqrt( 1.0 + mr[0] - mr[5] - mr[10] ) * 2;
;	
;	        Qx = 0.5 / S;
;	        Qy = (mr[1] + mr[4] ) / S;
;	        Qz = (mr[2] + mr[8] ) / S;
;	        Qw = (mr[6] + mr[9] ) / S;
;	
;	      Column 1:
;	        S  = sqrt( 1.0 + mr[5] - mr[0] - mr[10] ) * 2;
;	
;	        Qx = (mr[1] + mr[4] ) / S;
;	        Qy = 0.5 / S;
;	        Qz = (mr[6] + mr[9] ) / S;
;	        Qw = (mr[2] + mr[8] ) / S;
;	
;	      Column 2:
;	        S  = sqrt( 1.0 + mr[10] - mr[0] - mr[5] ) * 2;
;	
;	        Qx = (mr[2] + mr[8] ) / S;
;	        Qy = (mr[6] + mr[9] ) / S;
;	        Qz = 0.5 / S;
;	        Qw = (mr[1] + mr[4] ) / S; 		
 	.endif
	ret

Quaternion_frommMatrix endp

Quaternion_frommMat44 proc pQuatOut, pMatIn
LOCAL S,T
	;Calculate the Trace of the matrix
	mov edx,pMatIn
	Mat44_trace [edx].D3DXMATRIX
	fstp T
	;If the trace of the matrix is greater than zero, perform an "instant" calculation.
	mov eax,T
	and eax,80000000h
	.if eax==0
      ;S = 0.5 / sqrt(T)
      fld r4_Half
      fld T
      fsqrt
      fdiv
      fstp S
      mov eax,pQuatOut
      ;W = 0.25 / S
      fld r4_Quarter
      fdiv S
      fstp [eax].Quaternion.w
 	  ;X = ( mat[7] - mat[5] ) * S  
 	  fld  [edx].D3DXMATRIX.m21
 	  fsub [edx].Mat33.m12
 	  fmul S
 	  fstp [eax].Quaternion.x
      ;Y = ( mat[2] - mat[6] ) * S
 	  fld  [edx].Mat33.m02
 	  fsub [edx].Mat33.m20
 	  fmul S
 	  fstp [eax].Quaternion.y
      ;Z = ( mat[3] - mat[1] ) * S
 	  fld  [edx].Mat33.m10
 	  fsub [edx].Mat33.m01
 	  fmul S
 	  fstp [eax].Quaternion.z
 	.else
 		DbgWarning "Trace less than zero - error"
 		invoke ExitProcess,0
;	    If the trace of the matrix is less than or equal to zero
;	    then identify which major diagonal element has the greatest
;	    value.
;	
;	    Depending on this value, calculate the following:
;	
;	      Column 0:
;	        S  = sqrt( 1.0 + mr[0] - mr[5] - mr[10] ) * 2;
;	
;	        Qx = 0.5 / S;
;	        Qy = (mr[1] + mr[4] ) / S;
;	        Qz = (mr[2] + mr[8] ) / S;
;	        Qw = (mr[6] + mr[9] ) / S;
;	
;	      Column 1:
;	        S  = sqrt( 1.0 + mr[5] - mr[0] - mr[10] ) * 2;
;	
;	        Qx = (mr[1] + mr[4] ) / S;
;	        Qy = 0.5 / S;
;	        Qz = (mr[6] + mr[9] ) / S;
;	        Qw = (mr[2] + mr[8] ) / S;
;	
;	      Column 2:
;	        S  = sqrt( 1.0 + mr[10] - mr[0] - mr[5] ) * 2;
;	
;	        Qx = (mr[2] + mr[8] ) / S;
;	        Qy = (mr[6] + mr[9] ) / S;
;	        Qz = 0.5 / S;
;	        Qw = (mr[1] + mr[4] ) / S; 		
 	.endif
	ret
Quaternion_frommMat44 endp
	
Quaternion_fromMat33 proc pQuat,pMat
LOCAL qw2,qx2,qy2,qz2,tmp,i

	;quaternion components squared
    ;qw2 = 0.25f * (M.m[0] + M.m[4] + M.m[8] + 1.0f)
    mov edx,pMat
    fld  [edx].Mat33.M0
    fadd [edx].Mat33.M4
    fadd [edx].Mat33.M8
    fld1
    fadd
    fmul r4_Quarter
    fst qw2
    
    ;qx2 = qw2 - 0.5f * (M.m[4] + M.m[8])
    fld  [edx].Mat33.M4
    fadd [edx].Mat33.M8
    fmul r4_Half
    fsub
    fstp qx2
    
    ;qy2 = qw2 - 0.5f * (M.m[8] + M.m[0])
    fld qw2
    fld  [edx].Mat33.M8
    fadd [edx].Mat33.M0
    fmul r4_Half
    fsub
    fstp qy2

    ;qz2 = qw2 - 0.5f * (M.m[0] + M.m[4])
    fld qw2
    fld  [edx].Mat33.M0
    fadd [edx].Mat33.M4
    fmul r4_Half
    fsub
    fstp qz2	

	;maximum magnitude component
	
;    int i = (qw2 > qx2 ) ? ((qw2 > qy2) ? ((qw2 > qz2) ? 0 : 3) : ((qy2 > qz2) ? 2 : 3)) :
;        ((qx2 > qy2) ? ((qx2 > qz2) ? 1 : 3) : ((qy2 > qz2) ? 2 : 3));
	fMax qw2,qx2
	fstp tmp
	fMax tmp,qy2
	fstp tmp
	fMax tmp,qz2
	fstpReg eax
	mov i,0
	.if eax==qx2
		mov i,1
	.elseif eax==qy2
		mov i,2
	.elseif eax==qz2
		mov i,3
	.endif

; compute signed quaternion components using numerically stable method
   mov edx,pQuat
   .switch i 
   .case 0
   		mov eax,pMat
   		fld qw2
   		fsqrt
   		fstp [edx].Quaternion.w
   		fld r4_Quarter
   		fdiv [edx].Quaternion.w
   		fstp tmp
   		fld  [eax].Mat33.M7
   		fsub [eax].Mat33.M5
   		fmul tmp
   		fstp [edx].Quaternion.x
   		fld  [eax].Mat33.M2
   		fsub [eax].Mat33.M6
   		fmul tmp
   		fstp [edx].Quaternion.y
   		fld  [eax].Mat33.M3
   		fsub [eax].Mat33.M1
   		fmul tmp
   		fstp [edx].Quaternion.z

   .case 1
   		mov eax,pMat
   		fld qx2
   		fsqrt
   		fstp [edx].Quaternion.x
   		fld r4_Quarter
   		fdiv [edx].Quaternion.x
   		fstp tmp
   		fld  [eax].Mat33.M7
   		fsub [eax].Mat33.M5
   		fmul tmp
   		fstp [edx].Quaternion.w
   		fld  [eax].Mat33.M1
   		fadd [eax].Mat33.M3
   		fmul tmp
   		fstp [edx].Quaternion.y
   		fld  [eax].Mat33.M6
   		fadd [eax].Mat33.M2
   		fmul tmp
   		fstp [edx].Quaternion.z

   .case 2

   		mov eax,pMat
   		fld qy2
   		fsqrt
   		fstp [edx].Quaternion.y
   		fld r4_Quarter
   		fdiv [edx].Quaternion.y
   		fstp tmp
   		fld  [eax].Mat33.M2
   		fsub [eax].Mat33.M6
   		fmul tmp
   		fstp [edx].Quaternion.w
   		fld  [eax].Mat33.M1
   		fadd [eax].Mat33.M3
   		fmul tmp
   		fstp [edx].Quaternion.x
   		fld  [eax].Mat33.M5
   		fadd [eax].Mat33.M7
   		fmul tmp
   		fstp [edx].Quaternion.z

   .case 3
   		mov eax,pMat
   		fld qz2
   		fsqrt
   		fstp [edx].Quaternion.z
   		fld r4_Quarter
   		fdiv [edx].Quaternion.z
   		fstp tmp
   		fld  [eax].Mat33.M3
   		fsub [eax].Mat33.M1
   		fmul tmp
   		fstp [edx].Quaternion.w
   		fld  [eax].Mat33.M2
   		fadd [eax].Mat33.M6
   		fmul tmp
   		fstp [edx].Quaternion.x
   		fld  [eax].Mat33.M5
   		fadd [eax].Mat33.M7
   		fmul tmp
   		fstp [edx].Quaternion.y

    .endsw
    
    mov edx,pQuat
    mov eax,[edx].Quaternion.w
    and eax,80000000h
    .if i!=0 && eax!=0  ; make w always +ve
        fld [edx].Quaternion.w
        fld [edx].Quaternion.x
        fld [edx].Quaternion.y
        fld [edx].Quaternion.z
        fchs
        fstp [edx].Quaternion.z
        fchs
        fstp [edx].Quaternion.y
        fchs
        fstp [edx].Quaternion.x
        fchs
        fstp [edx].Quaternion.w
    .endif



	;normalise it to be safe
    fld1
    fld [edx].Quaternion.w
    fmul st(0),st(0)
    fld [edx].Quaternion.x
    fmul st(0),st(0)
    fld [edx].Quaternion.y
    fmul st(0),st(0)
    fld [edx].Quaternion.z
    fmul st(0),st(0)
    fadd
    fadd
    fadd
    fsqrt
    fdiv
    fst tmp
    fmul [edx].Quaternion.z
    fstp [edx].Quaternion.z
    fld tmp
    fmul [edx].Quaternion.y
    fstp [edx].Quaternion.y
    fld tmp
    fmul [edx].Quaternion.x
    fstp [edx].Quaternion.x
    fld tmp
    fmul [edx].Quaternion.w
	fstp [edx].Quaternion.w            
      
	ret
Quaternion_fromMat33 endp

Quaternion_fromMat44 proc pQuat,pMat
LOCAL qw2,qx2,qy2,qz2,lengt,fun,tmp,i

	;quaternion components squared
    ;qw2 = 0.25f * (M.m[0] + M.m[4] + M.m[8] + 1.0f)
    mov edx,pMat
    fld r4_Half
    fld  [edx].D3DXMATRIX.m00
    fadd [edx].D3DXMATRIX.m11
    fadd [edx].D3DXMATRIX.m22
    fsqrt
    fdiv
;    fmul r4_Quarter
    fst qw2
    
    ;qx2 = qw2 - 0.5f * (M.m[4] + M.m[8])
    fld  [edx].D3DXMATRIX.m11
    fadd [edx].D3DXMATRIX.m22
    fmul r4_Half
    fsub
    fstp qx2
    
    ;qy2 = qw2 - 0.5f * (M.m[8] + M.m[0])
    fld qw2
    fld  [edx].D3DXMATRIX.m22
    fadd [edx].D3DXMATRIX.m00
    fmul r4_Half
    fsub
    fstp qy2

    ;qz2 = qw2 - 0.5f * (M.m[0] + M.m[4])
    fld qw2
    fld  [edx].D3DXMATRIX.m00
    fadd [edx].D3DXMATRIX.m11
    fmul r4_Half
    fsub
    fstp qz2	

	;maximum magnitude component
	
;    int i = (qw2 > qx2 ) ? ((qw2 > qy2) ? ((qw2 > qz2) ? 0 : 3) : ((qy2 > qz2) ? 2 : 3)) :
;        ((qx2 > qy2) ? ((qx2 > qz2) ? 1 : 3) : ((qy2 > qz2) ? 2 : 3));
	fMax qw2,qx2
	fstp tmp
	fMax tmp,qy2
	fstp tmp
	fMax tmp,qz2
	fstpReg eax
	mov i,0
	.if eax==qx2
		mov i,1
	.elseif eax==qy2
		mov i,2
	.elseif eax==qz2
		mov i,3
	.endif

; compute signed quaternion components using numerically stable method
   mov edx,pQuat
   DbgDec i
   .switch i 
   .case 0
   		mov eax,pMat
   		fld qw2
   		fsqrt
   		fstp [edx].Quaternion.w
   		fld r4_Quarter
   		fdiv [edx].Quaternion.w
   		fstp tmp
   		fld  [eax].D3DXMATRIX.m21
   		fsub [eax].D3DXMATRIX.m12
   		fmul tmp
   		fstp [edx].Quaternion.x
   		fld  [eax].Mat33.m02
   		fsub [eax].Mat33.m20
   		fmul tmp
   		fstp [edx].Quaternion.y
   		fld  [eax].Mat33.m10
   		fsub [eax].Mat33.m01
   		fmul tmp
   		fstp [edx].Quaternion.z

   .case 1
   		mov eax,pMat
   		fld qx2
   		fsqrt
   		fstp [edx].Quaternion.x
   		fld r4_Quarter
   		fdiv [edx].Quaternion.x
   		fstp tmp
   		fld  [eax].Mat33.m21
   		fsub [eax].Mat33.m12
   		fmul tmp
   		fstp [edx].Quaternion.w
   		fld  [eax].Mat33.m01
   		fadd [eax].Mat33.m10
   		fmul tmp
   		fstp [edx].Quaternion.y
   		fld  [eax].Mat33.m20
   		fadd [eax].Mat33.m02
   		fmul tmp
   		fstp [edx].Quaternion.z

   .case 2

   		mov eax,pMat
   		fld qy2
   		fsqrt
   		fstp [edx].Quaternion.y
   		fld r4_Quarter
   		fdiv [edx].Quaternion.y
   		fstp tmp
   		fld  [eax].Mat33.m02
   		fsub [eax].Mat33.m20
   		fmul tmp
   		fstp [edx].Quaternion.w
   		fld  [eax].Mat33.m01
   		fadd [eax].Mat33.m10
   		fmul tmp
   		fstp [edx].Quaternion.x
   		fld  [eax].Mat33.m12
   		fadd [eax].Mat33.m21
   		fmul tmp
   		fstp [edx].Quaternion.z

   .case 3
   		mov eax,pMat
   		fld qz2
   		fsqrt
   		fstp [edx].Quaternion.z
   		fld r4_Quarter
   		fdiv [edx].Quaternion.z
   		fstp tmp
   		fld  [eax].Mat33.m10
   		fsub [eax].Mat33.m01
   		fmul tmp
   		fstp [edx].Quaternion.w
   		fld  [eax].Mat33.m02
   		fadd [eax].Mat33.m20
   		fmul tmp
   		fstp [edx].Quaternion.x
   		fld  [eax].Mat33.m12
   		fadd [eax].Mat33.m21
   		fmul tmp
   		fstp [edx].Quaternion.y

    .endsw
    
    mov edx,pQuat
    mov eax,[edx].Quaternion.w
    and eax,80000000h
    .if i!=0 && eax!=0  ; make w always +ve
        fld [edx].Quaternion.w
        fld [edx].Quaternion.x
        fld [edx].Quaternion.y
        fld [edx].Quaternion.z
        fchs
        fstp [edx].Quaternion.z
        fchs
        fstp [edx].Quaternion.y
        fchs
        fstp [edx].Quaternion.x
        fchs
        fstp [edx].Quaternion.w
    .endif



	;normalise it to be safe
	Quaternion_length [edx].Quaternion
	fstp lengt
	Quaternion_normalize [edx].Quaternion,lengt, fun
    
      
	ret
Quaternion_fromMat44 endp

Vec3_add macro vOut, v1, v2
	__LoadFloat3 v1
	fadd v2.z
	fstp v2.z
	fadd v2.y
	fstp v2.y	
	fadd v2.x
	fstp v2.x	
endm

Vec3_Length macro v1,v2
	fld v2.x
	fsub v1.x
	fmul st(0),st(0)
	fld v2.y
	fsub v2.y
	fmul st(0),st(0)
	fadd
	fld v2.z
	fsub v2.z
	fmul st(0),st(0)
	fadd
	fsqrt
endm


;Pre-Multiply the column vector A by the Transpose of matrix M :
;       out = transpose(M) * A
;LEAVES VEC3 ON FPU STACK !!!
Vec3_transMult macro Mat, VecIn


	;x = A.x * M.m[0] + A.y * M.m[3] + A.z * M.m[6];
	fld  VecIn.x
	fmul Mat.M0
	fld  VecIn.y
	fmul Mat.M3
	fld  VecIn.z
	fmul Mat.M6
	fadd
	fadd
	
	;y = A.x * M.m[1] + A.y * M.m[4] + A.z * M.m[7];
	fld  VecIn.x
	fmul Mat.M1
	fld  VecIn.y
	fmul Mat.M4
	fld  VecIn.z
	fmul Mat.M7
	fadd
	fadd
	
	;z = A.x * M.m[2] + A.y * M.m[5] + A.z * M.m[8];
	fld  VecIn.x
	fmul Mat.M2
	fld  VecIn.y
	fmul Mat.M5
	fld  VecIn.z
	fmul Mat.M8
	fadd
	fadd
endm

;Transform a point (or vector) from one coordinate system to another
;LEAVES VEC3 ON FPU STACK !!!
Vec3_transform macro Point, MatrotPos, VeclinPos
	;x = r.x * rotPos.m[0] + r.y * rotPos.m[1] + r.z * rotPos.m[2] + linPos.x;
	fld Point.x
	fmul MatrotPos.M0
	fld Point.y
	fmul MatrotPos.M1
	fadd
	fld Point.z
	fmul MatrotPos.M2
	fadd
	fadd VeclinPos.x
	
	;y = r.x * rotPos.m[3] + r.y * rotPos.m[4] + r.z * rotPos.m[5] + linPos.y;
	fld Point.x
	fmul MatrotPos.M3
	fld Point.y
	fmul MatrotPos.M4
	fadd
	fld Point.z
	fmul MatrotPos.M5
	fadd
	fadd VeclinPos.y

	;z = r.x * rotPos.m[6] + r.y * rotPos.m[7] + r.z * rotPos.m[8] + linPos.z;
	fld Point.x
	fmul MatrotPos.M6
	fld Point.y
	fmul MatrotPos.M7
	fadd
	fld Point.z
	fmul MatrotPos.M8
	fadd
	fadd VeclinPos.z
	
endm


;Vec3_invTransform - Inverse of Vec3_transform
; ... this = trans(rotPos) * (r - linPos)
;pr                     -   point in coord system 1
;protPos                -   rotation from system 1 to 2
;plinPos                -   translation from system 1 to 2
;LEAVES VEC3 ON FPU STACK !!!
Vec3_invTransform proc  pr:LPVEC3, protPos:LPMAT33, plinPos:LPVEC3
local sx, sy, sz

	mov edx,pr
	mov eax,plinPos	
	__SubFloat3 [edx].Vec3, [eax].Vec3
	fstp sz
	fstp sy
	fstp sx

	mov edx,protPos
	fld sx
	fmul [edx].Mat33.M0
	fld sy
	fmul [edx].Mat33.M3
	fld sz
	fmul [edx].Mat33.M6
	fadd
	fadd
	;
	fld sx
	fmul [edx].Mat33.M1
	fld sy
	fmul [edx].Mat33.M4
	fld sz
	fmul [edx].Mat33.M7
	fadd
	fadd
	;
	fld sx
	fmul [edx].Mat33.M2
	fld sy
	fmul [edx].Mat33.M5
	fld sz
	fmul [edx].Mat33.M8
	fadd
	fadd
	ret
Vec3_invTransform endp




;=================================
;=================================
;=================================



;Measure distance from Point (or Sphere) to Plane.
;Radius param is Optional, and is used for Sphere test.
DistancePointPlane macro Point:req,Plane:req
	DotProduct Point,Plane.vNormal
	fadd Plane.fPlaneD
endm

DistanceSpherePlane macro Point, radius, Plane
	DistancePointPlane Point, Plane
	fsub radius
endm