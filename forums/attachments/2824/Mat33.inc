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
		M6  real4 ?
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

;Supporting DOUBLE PRECISION !!! :D
Mat33_Double struct
	union
		m00 real8 ?
		M0	real8 ?
	ends
	union
		m01 real8 ?
		M1  real8 ?
	ends
	union
		m02 real8 ?
		M2  real8 ?
	ends	
	
	union
		m10 real8 ?
		M3  real8 ?
	ends
	union
		m11 real8 ?
		M4  real8 ?
	ends
	union
		m12 real8 ?
		M5  real8 ?
	ends
	
	union
		m20 real8 ?
		M6  real8 ?
	ends
	union
		m21 real8 ?
		M7  real8 ?
	ends
	union
		m22 real8 ?
		M8  real8 ?
	ends
Mat33_Double ends

.data
r4_0pt2 real4 0.2f
r4_9 real4 9.0f
r4_100 real4 100.0f

;----------------------------
;Data for Matrix Diagonalizer
.data?
 j dd ?
 iq dd ?
 ip dd ?
 i  dd ?
 _end dd ?

 r4_tresh real4 ?
 r4_theta real4 ?
 r4_tau real4 ?
 r4_t real4 ?
 r4_sm real4 ?
 r4_s real4 ?
 r4_h real4 ?
 r4_g real4 ?
 r4_cc real4 ?
 r4_ftemp real4 ?
 r4_fabstemp real4 ?
 r4_fbiggy real4 ?
 TempM33S Mat33 <>
 _b_ Vec3 <>
 _z_ Vec3 <>


 r8_tresh      real8 ?
 r8_theta      real8 ?
 r8_tau        real8 ?
 r8_t          real8 ?
 r8_sm         real8 ?
 r8_s          real8 ?
 r8_h          real8 ?
 r8_g          real8 ?
 r8_cc         real8 ?
 r8_temp    real8 ?
 r8_fabstemp   real8 ?
 r8_fbiggy     real8 ?
 TempM33D Mat33_Double <>
 __b        Vec3_Double  <>
 __z        Vec3_Double  <>
;----------------------------



.code




DbgMat33 macro Mat, Text, Window
    lea edx,Mat
    DbgVec3 edx,Text,Window
    add edx,sizeof Vec3
    DbgVec3 edx,    ,Window
    add edx,sizeof Vec3
    DbgVec3 edx,    ,Window
endm

DbgMat33_Double macro Mat, Text, Window
    lea edx,Mat
    DbgVec3_Double edx,Text,Window
    add edx,sizeof Vec3_Double
    DbgVec3_Double edx,    ,Window
    add edx,sizeof Vec3_Double
    DbgVec3_Double edx,    ,Window
endm

$Is_Mat33_Identity macro Mat
    mov eax,Mat.m01
    or  eax,Mat.m02
    or  eax,Mat.m10
    or  eax,Mat.m12
    or  eax,Mat.m20
    or  eax,Mat.m21
    .if eax==0
        mov eax,Mat.m00
        or  eax,Mat.m11
        or  eax,Mat.m22
        fld1
        fstpReg edx
        .if eax==edx
            mov eax,TRUE
        .else
            xor eax,eax
        .endif
    .else
        xor eax,eax
    .endif
    exitm <eax>
endm


.data 
r4_1em20 real4 1.0e-20

.code

;OrthoNormalize a Mat33 (or Mat33_Double)
;m = (in/out) Mat33
;vXYZ = temp Vec3 (or Vec3_Double)
Mat33_OrthoNormalize macro m,vX,vY,vZ
	fld m.M0 ;0,0
	fld m.M3 ;1,0
	fld m.M6 ;2,0
	Vec3_Stow vX
	
	fld m.M1 ;0,1
	fld m.M4 ;1,1
	fld m.M7 ;2,1  
	Vec3_Stow vY   

    Vec3_Normalize vX    
    Vec3_Cross vX,vY
    Vec3_Stow  vZ
    Vec3_Normalize vZ
    Vec3_Cross vZ,vX
    Vec3_Stow vY 
    Vec3_Normalize vY

    Vec3_Load vX
    fstp m.M6
    fstp m.M3
    fstp m.M0
    
    Vec3_Load vY
    fstp m.M7
    fstp m.M4
    fstp m.M1
    
    Vec3_Load vZ
    fstp m.M8
    fstp m.M5
    fstp m.M2
    
endm

;Target = Mat1 + Mat2
Mat33_Add macro MatOut,Mat1, Mat2
    fld  Mat1.M0
    fadd Mat2.M0
    fstp MatOut.M0
    fld  Mat1.M1
    fadd Mat2.M1
    fstp MatOut.M1
    fld  Mat1.M2
    fadd Mat2.M2
    fstp MatOut.M2
    fld  Mat1.M3
    fadd Mat2.M3
    fstp MatOut.M3
    fld  Mat1.M4
    fadd Mat2.M4
    fstp MatOut.M4
    fld  Mat1.M5
    fadd Mat2.M5
    fstp MatOut.M5
    fld  Mat1.M6
    fadd Mat2.M6
    fstp MatOut.M6
    fld  Mat1.M7
    fadd Mat2.M7
    fstp MatOut.M7
    fld  Mat1.M8
    fadd Mat2.M8
    fstp MatOut.M8
endm

;Leaves value on FPU Stack !!!
Mat33_Determinant macro Mat

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

Mat33_Identity macro Mat
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

Mat33_Inverse macro MatOut, MatIn

	Mat33_Determinant MatIn
	fst r8_temp
	fabs
	fsub r4_1em20
	fstpReg edx
	.ifBitSet edx,BIT31
		DbgWarning "Error - Mat33_Inverse failed, not orthogonal input matrix"
	    int 3
	.endif

; oneOverDetM = 1.0 / det
	fld1
	fdiv r8_temp
	fstp r8_temp
	
;	m[0] = oneOverDetM * (M.m[4] * M.m[8] - M.m[5] * M.m[7]);
	fld  MatIn.M4
	fmul MatIn.M8
	fld  MatIn.M5
	fmul MatIn.M7
	fsub
	fmul r8_temp
	fstp MatOut.M0

;	m[1] = -oneOverDetM * (M.m[1] * M.m[8] - M.m[2] * M.m[7]);
	fld  MatIn.M1
	fmul MatIn.M8
	fld  MatIn.M2
	fmul MatIn.M7
	fsub
	fmul r8_temp
	fchs
	fstp MatOut.M1

;	m[2] = oneOverDetM * (M.m[1] * M.m[5] - M.m[2] * M.m[4]);
	fld  MatIn.M1
	fmul MatIn.M5
	fld  MatIn.M2
	fmul MatIn.M4
	fsub
	fmul r8_temp
	fstp MatOut.M2

;	m[3] = -oneOverDetM * (M.m[3] * M.m[8] - M.m[5] * M.m[6]);
	fld  MatIn.M3
	fmul MatIn.M8
	fld  MatIn.M5
	fmul MatIn.M6
	fsub
	fmul r8_temp
	fchs
	fstp MatOut.M3

;	m[4] = oneOverDetM * (M.m[0] * M.m[8] - M.m[2] * M.m[6]);
	fld  MatIn.M0
	fmul MatIn.M8
	fld  MatIn.M2
	fmul MatIn.M6
	fsub
	fmul r8_temp
	fstp MatOut.M4

;	m[5] = -oneOverDetM * (M.m[0] * M.m[5] - M.m[2] * M.m[3]);
	fld  MatIn.M0
	fmul MatIn.M5
	fld  MatIn.M2
	fmul MatIn.M3
	fsub
	fmul r8_temp
	fchs
	fstp MatOut.M5

;	m[6] = oneOverDetM * (M.m[3] * M.m[7] - M.m[4] * M.m[6]);
	fld  MatIn.M3
	fmul MatIn.M7
	fld  MatIn.M4
	fmul MatIn.M6
	fsub
	fmul r8_temp
	fstp MatOut.M6
	
;	m[7] = -oneOverDetM * (M.m[0] * M.m[7] - M.m[1] * M.m[6]);
	fld  MatIn.M0
	fmul MatIn.M7
	fld  MatIn.M1
	fmul MatIn.M6
	fsub
	fmul r8_temp
	fchs
	fstp MatOut.M7

;	m[8] = oneOverDetM * (M.m[0] * M.m[4] - M.m[1] * M.m[3]);
	fld  MatIn.M0
	fmul MatIn.M4
	fld  MatIn.M1
	fmul MatIn.M3
	fsub
	fmul r8_temp
	fstp MatOut.M8
endm

;Multiply a 3d vector by a 3x3 matrix.
;vOut = Mat * V
;Leaves vector on FPU Stack !!!
Mat33_Mul_Vec3 macro  Mat, V

	fld  V.x
	fmul  Mat.M0
	fld  V.y
	fmul  Mat.M1
	fld  V.z
	fmul  Mat.M2
	fadd
	fadd
	
	fld  V.x
	fmul  Mat.M3
	fld  V.y
	fmul  Mat.M4
	fld  V.z
	fmul  Mat.M5
	fadd
	fadd

	fld  V.x
	fmul  Mat.M6
	fld  V.y
	fmul  Mat.M7
	fld  V.z
	fmul  Mat.M8
	fadd
	fadd

endm

;Multiply a Mat33 by another Mat33
;Result in TARGET
Mat33_Mul macro mTarget, mSource1, mSource2
;	m[0] = A.m[0] * B.m[0] + A.m[1] * B.m[3] + A.m[2] * B.m[6];
	fld  mSource1.M0
	fmul mSource2.M0
	fld  mSource1.M1
	fmul mSource2.M3
	fld  mSource1.M2
	fmul mSource2.M6	
	fadd
	fadd
	fstp mTarget.M0

;	m[1] = A.m[0] * B.m[1] + A.m[1] * B.m[4] + A.m[2] * B.m[7];
	fld  mSource1.M0
	fmul mSource2.M1
	fld  mSource1.M1
	fmul mSource2.M4
	fld  mSource1.M2
	fmul mSource2.M7	
	fadd
	fadd
	fstp mTarget.M1
;	m[2] = A.m[0] * B.m[2] + A.m[1] * B.m[5] + A.m[2] * B.m[8];
	fld  mSource1.M0
	fmul mSource2.M2
	fld  mSource1.M1
	fmul mSource2.M5
	fld  mSource1.M2
	fmul mSource2.M8	
	fadd
	fadd
	fstp mTarget.M2
	
;	m[3] = A.m[3] * B.m[0] + A.m[4] * B.m[3] + A.m[5] * B.m[6];
	fld  mSource1.M3
	fmul mSource2.M0
	fld  mSource1.M4
	fmul mSource2.M3
	fld  mSource1.M5
	fmul mSource2.M6	
	fadd
	fadd
	fstp mTarget.M3

;	m[4] = A.m[3] * B.m[1] + A.m[4] * B.m[4] + A.m[5] * B.m[7];
	fld  mSource1.M3
	fmul mSource2.M1
	fld  mSource1.M4
	fmul mSource2.M4
	fld  mSource1.M5
	fmul mSource2.M7	
	fadd
	fadd
	fstp mTarget.M4

;	m[5] = A.m[3] * B.m[2] + A.m[4] * B.m[5] + A.m[5] * B.m[8];
	fld  mSource1.M3
	fmul mSource2.M2
	fld  mSource1.M4
	fmul mSource2.M5
	fld  mSource1.M5
	fmul mSource2.M8	
	fadd
	fadd
	fstp mTarget.M5
	
;	m[6] = A.m[6] * B.m[0] + A.m[7] * B.m[3] + A.m[8] * B.m[6];
	fld  mSource1.M6
	fmul mSource2.M0
	fld  mSource1.M7
	fmul mSource2.M3
	fld  mSource1.M8
	fmul mSource2.M6	
	fadd
	fadd
	fstp mTarget.M6
	
;	m[7] = A.m[6] * B.m[1] + A.m[7] * B.m[4] + A.m[8] * B.m[7];
	fld  mSource1.M6
	fmul mSource2.M1
	fld  mSource1.M7
	fmul mSource2.M4
	fld  mSource1.M8
	fmul mSource2.M7	
	fadd
	fadd
	fstp mTarget.M7

;	m[8] = A.m[6] * B.m[2] + A.m[7] * B.m[5] + A.m[8] * B.m[8];
	fld  mSource1.M6
	fmul mSource2.M2
	fld  mSource1.M7
	fmul mSource2.M5
	fld  mSource1.M8
	fmul mSource2.M8	
	fadd
	fadd
	fstp mTarget.M8
endm

;Pre-Multiply the vector A by the Transpose of matrix M :
;       out = transpose(M) * A
;LEAVES VEC3 ON FPU STACK !!!
Mat33_TransMult_Vec3 macro Mat, VecIn


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

;Target = Source1 * Transpose(Source2)
Mat33_TransMult_Mat33 macro target,source1,source2

;	m[0] = A.m[0] * B.m[0] + A.m[1] * B.m[3] + A.m[2] * B.m[6];
	fld  source1.M0
	fmul source2.M0
	fld  source1.M1
	fmul source2.M1
	fld  source1.M2
	fmul source2.M2	
	fadd
	fadd
	fstp target.M0

;	m[1] = A.m[0] * B.m[1] + A.m[1] * B.m[4] + A.m[2] * B.m[7];
	fld  source1.M0
	fmul source2.M3
	fld  source1.M1
	fmul source2.M4
	fld  source1.M2
	fmul source2.M5	
	fadd
	fadd
	fstp target.M1
;	m[2] = A.m[0] * B.m[2] + A.m[1] * B.m[5] + A.m[2] * B.m[8];
	fld  source1.M0
	fmul source2.M6
	fld  source1.M1
	fmul source2.M7
	fld  source1.M2
	fmul source2.M8	
	fadd
	fadd
	fstp target.M2

	
;	m[3] = A.m[3] * B.m[0] + A.m[4] * B.m[3] + A.m[5] * B.m[6];
	fld  source1.M3
	fmul source2.M0
	fld  source1.M4
	fmul source2.M1
	fld  source1.M5
	fmul source2.M2	
	fadd
	fadd
	fstp target.M3

;	m[4] = A.m[3] * B.m[1] + A.m[4] * B.m[4] + A.m[5] * B.m[7];
	fld  source1.M1
	fmul source2.M1
	fld  source1.M4
	fmul source2.M4
	fld  source1.M5
	fmul source2.M5	
	fadd
	fadd
	fstp target.M4

;	m[5] = A.m[3] * B.m[2] + A.m[4] * B.m[5] + A.m[5] * B.m[8];
	fld  source1.M3
	fmul source2.M6
	fld  source1.M4
	fmul source2.M7
	fld  source1.M5
	fmul source2.M8	
	fadd
	fadd
	fstp target.M5
	
;	m[6] = A.m[6] * B.m[0] + A.m[7] * B.m[3] + A.m[8] * B.m[6];
	fld  source1.M6
	fmul source2.M0
	fld  source1.M7
	fmul source2.M1
	fld  source1.M8
	fmul source2.M2	
	fadd
	fadd
	fstp target.M6
	
;	m[7] = A.m[6] * B.m[1] + A.m[7] * B.m[4] + A.m[8] * B.m[7];
	fld  source1.M6
	fmul source2.M3
	fld  source1.M7
	fmul source2.M4
	fld  source1.M8
	fmul source2.M5	
	fadd
	fadd
	fstp target.M7

;	m[8] = A.m[6] * B.m[2] + A.m[7] * B.m[5] + A.m[8] * B.m[8];
	fld  source1.M6
	fmul source2.M6
	fld  source1.M7
	fmul source2.M7
	fld  source1.M8
	fmul source2.M8	
	fadd
	fadd
	fstp target.M8
endm

;Perform a Similarity Transform
;matOut = (matA * matB * transpose(matA))
Mat33_SimilarityTransform macro matOut, temporal, matBasisA, matBasisB
    Mat33_TransMult_Mat33   temporal, matBasisA,  matBasisB
	Mat33_Mul               matOut,   temporal,    matBasisA
endm

;Transform a given Vec3 Point from BodySpace to WorldSpace
Mat33_Transform_BodyToWorld macro vPointOut,vPointIn,Mat33_BodyToWorld,vTranslation
	Mat33_Mul_Vec3 Mat33_BodyToWorld,vPointIn
	Vec3_AddToTarget vPointOut, vTranslation
endm

;Transform a given Vec3 Point from WorldSpace to this BodySpace
Mat33_Transform_WorldToBody macro vPointOut,vPointIn, Mat33_BodyToWorld, vTranslation
    Mat33_TransMult_Vec3 Mat33_BodyToWorld,vPointIn  ;unrotate the Point
    Vec3_Subtract vTranslation                       ;untranslate the Point
    Vec3_Stow vPointOut
endm


;Flip sign of all elements in Mat33
Mat33_Neg macro Mat
	fld Mat.M0
	fld Mat.M1
	fld Mat.M2
	fchs
	fstp Mat.M2
	fchs
	fstp Mat.M1
	fchs
	fstp Mat.M0
	
	fld Mat.M3
	fld Mat.M4
	fld Mat.M5
	fchs
	fstp Mat.M5
	fchs
	fstp Mat.M4
	fchs
	fstp Mat.M3
	
	fld Mat.M6
	fld Mat.M7
	fld Mat.M8
	fchs
	fstp Mat.M8
	fchs
	fstp Mat.M7
	fchs
	fstp Mat.M6
	
endm

;Creates an Orthogonal set of Normal Vectors (axes) from a given Vector, as Columns of a Mat33
Mat33_OrthoNormalBasis macro Mat,V

	;The X column vector is taken directly from our collision Normal
	Vec3_Load V
	fstp Mat.m20
	fstp Mat.m10
	fstp Mat.m00
	
    ;Pick an axis which is orthogonal to the Major axis
    .if $IsAbsGreater (V.x, V.y)  
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
        fldz
        fstp Mat.m11
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
        fldz
        fstp Mat.m01
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

Mat33_Scale macro Mat, fScalar
    fld fScalar
    fmul Mat.M0
    fstp Mat.M0
    fld fScalar
    fmul Mat.M1
    fstp Mat.M1
    fld fScalar
    fmul Mat.M2
    fstp Mat.M2
    fld fScalar
    fmul Mat.M3
    fstp Mat.M3
    fld fScalar
    fmul Mat.M4
    fstp Mat.M4
    fld fScalar
    fmul Mat.M5
    fstp Mat.M5
    fld fScalar
    fmul Mat.M6
    fstp Mat.M6
    fld fScalar
    fmul Mat.M7
    fstp Mat.M7
    fld fScalar
    fmul Mat.M8
    fstp Mat.M8
endm

;Set matrix as the * or ~ operator on the vector :
;THIS IS ALSO KNOWN AS A SKEW SYMMETRIC MATRIX
;          [  0   -z    y ]
; Target = [  z    0   -x ]
;          [ -y    x    0 ]
Mat33_Star macro Mat, Vec
	
	fldz
	fst  Mat.M0
	fst  Mat.M4
	fstp Mat.M8

	fld  Vec.z
	fst  Mat.M3
	fchs
	fstp Mat.M1
	
	fld  Vec.y
	fst  Mat.M2
	fchs
	fstp Mat.M6
	
	fld  Vec.x
	fst  Mat.M7
	fchs
	fstp Mat.M5
endm

;MatOut = Transpose (MatIn)
;CAN TRANSPOSE MATRIX IN PLACE
Mat33_Transpose macro MatOut, MatIn	
    fld  MatIn.M0
    fld  MatIn.M4
    fld  MatIn.M8
    fstp MatOut.M8
    fstp MatOut.M4
    fstp MatOut.M0

    fld  MatIn.M1
    fld  MatIn.M3    
	fstp MatOut.M1
	fstp MatOut.M3
	
    fld  MatIn.M2
    fld  MatIn.M6    
	fstp MatOut.M2
	fstp MatOut.M6

    fld  MatIn.M5
    fld  MatIn.M7    
	fstp MatOut.M5
	fstp MatOut.M7
endm

;Can copy a Mat33 (single or double precision)
;Can Convert between Mat33/Mat44 (rotation part of matrix only)
Matrix_From_Matrix macro MatOut, MatIn

    fld MatIn.m00
    fld MatIn.m01
    fld MatIn.m02
    fstp MatOut.m02
    fstp MatOut.m01
    fstp MatOut.m00
    
    fld MatIn.m10
    fld MatIn.m11
    fld MatIn.m12
    fstp MatOut.m12
    fstp MatOut.m11
    fstp MatOut.m10
    
    fld MatIn.m20
    fld MatIn.m21
    fld MatIn.m22
    fstp MatOut.m22
    fstp MatOut.m21
    fstp MatOut.m20
endm


;Using this macro, we can address matrix elements in 2D
;For example:		 fld real4 ptr [eax+$Mat33_Index(2,1)]
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

;Using this macro, we can address matrix elements in 2D
;For example:		 fld real4 ptr [eax+$Mat33_Indexed(2,1)]
;is equivalent to:	 fld [eax].Mat33.M7
;or if you prefer:	 fld [eax].Mat33.M21
$Mat33_Double_index macro y,x
	push edx
	mov eax,sizeof Vec3_Double
	mul y
	mov edx,x
	shl edx,2
	add eax,edx
	pop edx
	exitm <(eax)>
endm

;This is an inline macro, its part of Mat33_eigenvectors (below)
EIGENROTATE macro a,i,j,k,l
      ;g=a.n[i][j]
      fld real4 ptr a[$Mat33_index(i,j)]
      fstp r4_g
      ;h=a.n[k][l]
      fld real4 ptr a[$Mat33_index(k,l)]
      fstp r4_h
      ;a.n[i][j]=g-s*(h+g*tau)
      fld r4_g
      fld r4_g
      fmul r4_tau
      fadd r4_h
      fmul r4_s
      fsub
      fstp real4 ptr a[$Mat33_index(i,j)]
      ;a.n[k][l]=h+s*(g-h*tau);
      fld r4_g
	  fld r4_h
	  fmul r4_tau
	  fsub
	  fmul r4_s
	  fadd r4_h
	  fstp  real4 ptr a[$Mat33_index(k,l)]
endm

;This is an inline macro, its part of Mat33_eigenvectors (below)
EIGENROTATE_DOUBLE macro a,i,j,k,l
      ;g=a.n[i][j]
      fld real8 ptr a[$Mat33_Double_index(i,j)]
      fstp r8_g
      ;h=a.n[k][l]
      fld real8 ptr a[$Mat33_Double_index(k,l)]
      fstp r8_h
      ;a.n[i][j]=g-s*(h+g*tau)
      fld r8_g
      fld r8_g
      fmul r8_tau
      fadd r8_h
      fmul r8_s
      fsub
      fstp real8 ptr a[$Mat33_Double_index(i,j)]
      ;a.n[k][l]=h+s*(g-h*tau);
      fld r8_g
	  fld r8_h
	  fmul r8_tau
	  fsub
	  fmul r8_s
	  fadd r8_h
	  fstp  real8 ptr a[$Mat33_Double_index(k,l)]
endm

;***********************************************************************
;eigenvectors
;   Find the eigen vectors and values for a matrix using the Jacobi rotation method.
;   Assumes the matrix is symmetric, i.e. the bottom half is ignored.
;
;params :             
;	MatOut                  -   unit length eigen vectors returned here (in columns)
;	VecOut                  -   eigen values returned here
;	MatIn					 -   matrix to find eigen vectors for (symmetric)
;
;returns :		             -   0 OK, -1 error
;
;***********************************************************************
Mat33_eigenvectors macro MatOut, VecOut, MatIn										;integers
    
    Mat33_Identity MatOut
    Mat33_Identity TempM33S      
    fld1
    fst VecOut.z
    fst VecOut.y
    fst VecOut.x
    fst _b_.z
    fst _b_.y
    fstp _b_.x
    fldz
    fst _z_.z
    fst _z_.y
    fstp _z_.x

	xor ecx,ecx
	.while ecx<50
		mov i,ecx
		
        fld  TempM33S.m01
        fadd TempM33S.m02
        fadd TempM33S.m11
        fadd TempM33S.m12
        fstp r4_sm
        .if $IsZero (r4_sm) 
		    xor eax,eax	;=SUCCESS
		    jmp @F
        .endif

		.if i < 3
		    ;tresh=0.2f*float(fabs(sm)/(3*3))
		    fld r4_sm
		    fdiv r4_9
		    fabs
		    fmul r4_0pt2
		    fstp r4_tresh 
		.else
	        and r4_tresh,0
	    .endif
	    
	    mov ip,0
		.while ip<2
			mov ecx,ip
			inc ecx
			mov iq,ecx
			.while iq<3
			
				m2m r4_ftemp, real4 ptr TempM33S[$Mat33_index(ip,iq)]	
				
				fMax r4_fabstemp, r4_tresh
				fstp r4_fbiggy		
			
		        ;g=100.0f*float(fabs(TempMat33.n[ip][iq]));
		        fld r4_ftemp
		        fabs
		        fst r4_fabstemp
		        fmul r4_100
		        fstp r4_g

                ;if (i > 4  && (float)(fabs(VecOut.m[ip])+g) == (float)fabs(VecOut.m[ip])
                ; 			&& (float)(fabs(VecOut.m[iq])+g) == (float)fabs(VecOut.m[iq]))
                mov eax,ip
                fld real4 ptr VecOut[eax*4]
                fabs
                fstReg ebx
                fadd r4_g
                fstpReg eax
                .if eax==ebx
                	mov eax,iq
                	fld real4 ptr VecOut[eax*4]
                	fabs
                	fstReg ebx
                	fadd r4_g
                	fstpReg eax
                	mov edx,r4_fabstemp
                	.if eax==ebx
	                	fldz           
	                    fstp real4 ptr TempM33S[$Mat33_index(ip,iq)]               
            		.elseif r4_fbiggy==edx ;fabs(TempMat33.n[ip][iq]) > tresh) {
	                	;h=VecOut.m[iq]-VecOut.m[ip];
	                	mov eax,ip
	                	mov ebx,iq
	                	fld  real4 ptr VecOut[ebx*4]
	                	fsub real4 ptr VecOut[eax*4]
	                	fst r4_h
                                        
	                    ;if (fabs(h)+g) == fabs(h))
	                    fabs
	                    fstReg ebx
	                    fadd r4_g
	                    fstpReg eax
	                    .if eax==ebx
	                    	;t=(TempMat33.n[ip][iq])/h;
	                    	fld r4_ftemp
	                    	fdiv r4_h
	                    	fstp r4_t
	                    .else 
	                    	;theta=0.5f*h/(TempMat33.n[ip][iq]);
	                    	fld r4_Half
	                    	fmul r4_h
	                    	fdiv r4_ftemp
	                    	fstp r4_theta
	                    	
	                        ;t=1.0f/(float(fabs(theta))+float(sqrt(1.0f+theta*theta)));
	                        fld1                        
	                        fld r4_theta
	                        fmul st(0),st(0)
	                        fld1
	                        fadd
	                        fsqrt
	                        fld r4_theta
	                        fabs
	                        fadd
	                        fdiv
	                        fstp r4_t
	                        
	                    	;if (theta < 0.0) 
	                    	mov eax,r4_theta
	                    	and eax,80000000h
	                    	.if eax!=0
	                    		fld r4_t
	                    		fchs
	                    		fstp r4_t
	                    	.endif
	                    .endif
                    
		                ;cc=1.0f/float(sqrt(1.0f+t*t));
		                fld1
		                fld r4_t
		                fmul st(0),st(0)
		                fld1
		                fadd
		                fsqrt
		                fdiv
		                fst r4_cc
	
	                	;s=t*c
	                	fmul r4_t
	                	fst r4_s
	                	
		                ;tau=s/(1.0f+c);
		                fld1
		                fadd r4_cc
		                fdiv
		                fstp r4_tau
		                	                
	                    ;h=t*TempMat33.n[ip][iq];
	                    fld r4_t
	                    fmul real4 ptr TempM33S[$Mat33_index(ip,iq)]
	                    fstp r4_h
	
	                	;z[ip] -= h;
	                	;z[iq] += h;
	                	mov eax,ip
	                	mov ebx,iq
	                	fld  real4 ptr _z_[eax*4]
	                	fsub r4_h
	                	fstp real4 ptr _z_[eax*4]                	
	                	fld  real4 ptr _z_[ebx*4]
	                	fadd r4_h
	                	fstp real4 ptr _z_[ebx*4]
	                	
	                	;VecOut.m[ip] -= h
	                    ;VecOut.m[iq] += h
	                    fld  real4 ptr VecOut[eax*4]
	                    fsub r4_h
	                    fstp real4 ptr VecOut[eax*4]
	                    fld  real4 ptr VecOut[ebx*4]
	                    fadd r4_h
	                    fstp real4 ptr VecOut[ebx*4]
	                      
	                    fldz                  
	                	fstp real4 ptr TempM33S[$Mat33_index(ip,iq)]
	                	
		                xor ecx,ecx
		                .while ecx<ip
		                	push ecx
	                    	EIGENROTATE TempM33S,ecx,ip,ecx,iq
	                    	pop ecx
	                    	inc ecx
	                    .endw
	
						inc ecx	;ecx now = ip+1
						.while ecx<iq
							push ecx
	                    	EIGENROTATE TempM33S,ip,ecx,ecx,iq 
	                    	pop ecx
	                    	inc ecx
	                    .endw
	                    
	                    inc ecx ;ecx now = iq+1
	                	.while ecx<3
	                		push ecx
	                    	EIGENROTATE TempM33S,ip,ecx,iq,ecx
	                    	pop ecx
	                    	inc ecx
	                    .endw
	                    
	                    
	                    xor ecx,ecx
	                    .while ecx<3
	                   		push ecx
	                    	EIGENROTATE MatOut,ecx,ip,ecx,iq
	                    	pop ecx
	                    	inc ecx
	                    .endw
               		.endif
			    .endif
			    
			    inc iq
            .endw
            inc ip
        .endw
        
        Vec3_Add  _z_,_b_
        Vec3_Stow _b_
        Vec3_Copy VecOut, _b_
        
        
        inc i
        mov ecx,i
        
    .endw
    mov eax,-1
@@:
endm


Mat33_Double_eigenvectors macro MatOut, VecOut, MatIn


    Mat33_Identity MatOut
    Mat33_Identity TempM33D      
    fld1
    fst VecOut.z
    fst VecOut.y
    fst VecOut.x
    fst __b.z
    fst __b.y
    fstp __b.x
    fldz
    fst __z.z
    fst __z.y
    fstp __z.x

	xor ecx,ecx
	.while ecx<50           ;Max. #iterations
		mov i,ecx
		
        
        fld  TempM33D.m01
        fadd TempM33D.m02
        fadd TempM33D.m11
        fadd TempM33D.m12
        fstp r8_sm
        .if $IsZero (r8_sm) 
		    xor eax,eax	;=SUCCESS
		    jmp @F
        .endif

		.if i < 3
		    ;tresh=0.2f*fabs(sm)/(3*3)
		    fld r8_sm
		    fdiv r4_9
		    fabs
		    fmul r4_0pt2
		    fstp r8_tresh 
		.else
		    fldz
	        fstp r8_tresh
	    .endif
	    
	    mov ip,0
		.while ip<2
			mov ecx,ip
			inc ecx
			mov iq,ecx
			.while iq<3
			    fld  real8 ptr TempM33D[$Mat33_Double_index(ip,iq)]	
				fstp r8_ftemp
				
				fMax r8_fabstemp, r8_tresh
				fstp r8_fbiggy		
			
		        ;g=100.0f*float(fabs(TempM33D.n[ip][iq]));
		        fld r8_ftemp
		        fabs
		        fst r8_fabstemp
		        fmul r4_100
		        fstp r8_g

                ;if (i > 4  && fabs(VecOut.m[ip])+g) == fabs(VecOut.m[ip]
                ; 			&& fabs(VecOut.m[iq])+g) == fabs(VecOut.m[iq]
                lea edx,VecOut
                mov eax,ip
                fld real8 ptr [edx+eax*8]
                fabs
                fstReg ebx
                fadd r8_g
                fstpReg eax
                .if eax==ebx
                	mov eax,iq
                	fld real8 ptr [edx+eax*8]
                	fabs
                	fstReg ebx
                	fadd r8_g
                	fstpReg eax
                	mov edx,r8_fabstemp
                	.if eax==ebx
	                	fldz           
	                    fstp real8 ptr TempM33D[$Mat33_Double_index(ip,iq)]               
            		.elseif r8_fbiggy==edx ;fabs(TempM33D.n[ip][iq]) > tresh) {
	                	;h=VecOut.m[iq]-VecOut.m[ip];
	                	lea edx,VecOut
	                	mov eax,ip
	                	mov ebx,iq
	                	fld real8 ptr[edx+ebx*8]
	                	fsub real8 ptr[edx+eax*8]
	                	fst r8_h
                                        
	                    ;if (fabs(h)+g) == fabs(h))
	                    fabs
	                    fstReg ebx
	                    fadd r8_g
	                    fstpReg eax
	                    .if eax==ebx
	                    	;t=(TempM33D.n[ip][iq])/h;
	                    	fld r8_ftemp
	                    	fdiv r8_h
	                    	fstp r8_t
	                    .else 
	                    	;theta=0.5f*h/(TempM33D.n[ip][iq]);
	                    	fld r4_Half
	                    	mul r8_h
	                    	fdiv r8_ftemp
	                    	fstp r8_theta
	                    	
	                        ;t=1.0f/(float(fabs(theta))+float(sqrt(1.0f+theta*theta)));
	                        fld1                        
	                        fld r8_theta
	                        fmul st(0),st(0)
	                        fld1
	                        fadd
	                        fsqrt
	                        fld r8_theta
	                        fabs
	                        fadd
	                        fdiv
	                        fstp r8_t
	                        
	                    	;if (theta < 0.0) 
	                    	mov eax,r8_theta
	                    	and eax,80000000h
	                    	.if eax!=0
	                    		fld r8_t
	                    		fchs
	                    		fstp r8_t
	                    	.endif
	                    .endif
                    
		                ;cc=1.0f/float(sqrt(1.0f+t*t));
		                fld1
		                fld r8_t
		                fmul st(0),st(0)
		                fld1
		                fadd
		                fsqrt
		                fdiv
		                fst r8_cc
	
	                	;s=t*c
	                	fmul r8_t
	                	fst r8_s
	                	
		                ;tau=s/(1.0f+c);
		                fld1
		                fadd r8_cc
		                fdiv
		                fstp r8_tau
		                	                
	                    ;h=t*TempM33D.n[ip][iq];
	                    fld r8_t
	                    fmul real8 ptr TempM33D[$Mat33_Double_index(ip,iq)]
	                    fstp r8_h
	
	                	;z[ip] -= h;
	                	;z[iq] += h;
	                	mov eax,ip
	                	mov ebx,iq
	                	fld  __z[eax*8]
	                	fsub r8_h
	                	fstp __z[eax*8]                	
	                	fld  __z[ebx*8]
	                	fadd r8_h
	                	fstp __z[ebx*8]
	                	
	                	;VecOut.m[ip] -= h
	                    ;VecOut.m[iq] += h
	                    lea edx,VecOut
	                    fld real8 ptr[edx+eax*8]
	                    fsub r8_h
	                    fstp real8 ptr[edx+eax*8]
	                    fld real8 ptr[edx+ebx*8]
	                    fadd r8_h
	                    fstp real8 ptr[edx+ebx*8]
	                      
	                    fldz                  
	                	fstp real8 ptr TempM33D[$Mat33_Double_index(ip,iq)]
	                	
		                xor ecx,ecx
		                .while ecx<ip
		                	push ecx
	                    	EIGENROTATE_DOUBLE TempM33D,ecx,ip,ecx,iq
	                    	pop ecx
	                    	inc ecx
	                    .endw
	
						inc ecx	;ecx now = ip+1
						.while ecx<iq
							push ecx
	                    	EIGENROTATE_DOUBLE TempM33D,ip,ecx,ecx,iq 
	                    	pop ecx
	                    	inc ecx
	                    .endw
	                    
	                    inc ecx ;ecx now = iq+1
	                	.while ecx<3
	                		push ecx
	                    	EIGENROTATE_DOUBLE TempM33D,ip,ecx,iq,ecx
	                    	pop ecx
	                    	inc ecx
	                    .endw
	                    
	                    
	                    xor ecx,ecx
	                    .while ecx<3
	                   		push ecx
	                    	EIGENROTATE_DOUBLE MatOut,ecx,ip,ecx,iq
	                    	pop ecx
	                    	inc ecx
	                    .endw
               		.endif
			    .endif
			    
			    inc iq
            .endw
            inc ip
        .endw
        
        Vec3_Add  __z,__b
        Vec3_Stow __b
        Vec3_Copy VecOut, __b
        
        inc i
        mov ecx,i
        
    .endw
    mov eax,-1
@@:
endm
