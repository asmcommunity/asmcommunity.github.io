;Homer's 3x3 Matrix Math helpers
;Dec. 16, 2006

;MACROS...
;Mat33Identity
;Mat33MulFloat
;Mat33MulMat33
;Mat33Transpose

;PROCS...
;Mat33FromQuat
;Mat33FromQuatL2
;Mat33EigenVectors

Mat33Identity macro pM
	mov edx,pM
	fld1
	fst  [edx].Mat33.m00
	fst  [edx].Mat33.m11
	fstp [edx].Mat33.m22
	fldz
	fst  [edx].Mat33.m01
	fst  [edx].Mat33.m02
	fstp [edx].Mat33.m10
	fst  [edx].Mat33.m12
	fst  [edx].Mat33.m20
	fstp [edx].Mat33.m21
endm

;Multiply Mat33 by Scalar
Mat33MulFloat macro pmatOut,pmatIn,float
	mov edx,pmatIn
	mov eax,pmatOut
	fld float
	fmul [edx].Mat33.m00
	fstp [eax].Mat33.m00
	fld float
	fmul [edx].Mat33.m01
	fstp [eax].Mat33.m01
	fld float
	fmul [edx].Mat33.m02
	fstp [eax].Mat33.m02
	fld float
	fmul [edx].Mat33.m10
	fstp [eax].Mat33.m10
	fld float
	fmul [edx].Mat33.m11
	fstp [eax].Mat33.m11
	fld float
	fmul [edx].Mat33.m12
	fstp [eax].Mat33.m12
	fld float
	fmul [edx].Mat33.m20
	fstp [eax].Mat33.m20
	fld float
	fmul [edx].Mat33.m21
	fstp [eax].Mat33.m21
	fld float
	fmul [edx].Mat33.m22
	fstp [eax].Mat33.m22
endm

Mat33MulMat33 macro pmatOut,pmatA,pmatB
	mov eax,pmatA
	mov ebx,pmatB
	mov edx,pmatOut
;	m[0] = A.m[0] * B.m[0] + A.m[1] * B.m[3] + A.m[2] * B.m[6];
	fld  [eax].Mat33.m00
	fmul [ebx].Mat33.m00
	fld  [eax].Mat33.m01
	fmul [ebx].Mat33.m10
	fld  [eax].Mat33.m02
	fmul [ebx].Mat33.m20
	fadd
	fadd
	fstp [edx].Mat33.m00
;	m[1] = A.m[0] * B.m[1] + A.m[1] * B.m[4] + A.m[2] * B.m[7];
	fld  [eax].Mat33.m00
	fmul [ebx].Mat33.m01
	fld  [eax].Mat33.m01
	fmul [ebx].Mat33.m11
	fld  [eax].Mat33.m02
	fmul [ebx].Mat33.m21
	fadd
	fadd
	fstp [edx].Mat33.m01
;	m[2] = A.m[0] * B.m[2] + A.m[1] * B.m[5] + A.m[2] * B.m[8];
	fld  [eax].Mat33.m00
	fmul [ebx].Mat33.m02
	fld  [eax].Mat33.m01
	fmul [ebx].Mat33.m12
	fld  [eax].Mat33.m02
	fmul [ebx].Mat33.m22
	fadd
	fadd
	fstp [edx].Mat33.m02
;	m[3] = A.m[3] * B.m[0] + A.m[4] * B.m[3] + A.m[5] * B.m[6];
	fld  [eax].Mat33.m10
	fmul [ebx].Mat33.m00
	fld  [eax].Mat33.m11
	fmul [ebx].Mat33.m10
	fld  [eax].Mat33.m12
	fmul [ebx].Mat33.m20
	fadd
	fadd
	fstp [edx].Mat33.m10
;	m[4] = A.m[3] * B.m[1] + A.m[4] * B.m[4] + A.m[5] * B.m[7];
	fld  [eax].Mat33.m10
	fmul [ebx].Mat33.m01
	fld  [eax].Mat33.m11
	fmul [ebx].Mat33.m11
	fld  [eax].Mat33.m12
	fmul [ebx].Mat33.m21
	fadd
	fadd
	fstp [edx].Mat33.m11
;	m[5] = A.m[3] * B.m[2] + A.m[4] * B.m[5] + A.m[5] * B.m[8];
	fld  [eax].Mat33.m10
	fmul [ebx].Mat33.m02
	fld  [eax].Mat33.m11
	fmul [ebx].Mat33.m12
	fld  [eax].Mat33.m12
	fmul [ebx].Mat33.m22
	fadd
	fadd
	fstp [edx].Mat33.m12
;	m[6] = A.m[6] * B.m[0] + A.m[7] * B.m[3] + A.m[8] * B.m[6];
	fld  [eax].Mat33.m20
	fmul [ebx].Mat33.m00
	fld  [eax].Mat33.m21
	fmul [ebx].Mat33.m10
	fld  [eax].Mat33.m22
	fmul [ebx].Mat33.m20
	fadd
	fadd
	fstp [edx].Mat33.m20
;	m[7] = A.m[6] * B.m[1] + A.m[7] * B.m[4] + A.m[8] * B.m[7];
	fld  [eax].Mat33.m20
	fmul [ebx].Mat33.m01
	fld  [eax].Mat33.m21
	fmul [ebx].Mat33.m11
	fld  [eax].Mat33.m22
	fmul [ebx].Mat33.m21
	fadd
	fadd
	fstp [edx].Mat33.m21
;	m[8] = A.m[6] * B.m[2] + A.m[7] * B.m[5] + A.m[8] * B.m[8];
	fld  [eax].Mat33.m20
	fmul [ebx].Mat33.m02
	fld  [eax].Mat33.m21
	fmul [ebx].Mat33.m12
	fld  [eax].Mat33.m22
	fmul [ebx].Mat33.m22
	fadd
	fadd
	fstp [edx].Mat33.m22
endm

;Transpose a matrix
Mat33Transpose macro pOut,pA
  mov eax,pA
  mov edx,pOut
  m2m [edx].Mat33.m00,[eax].Mat33.m00
  m2m [edx].Mat33.m01,[eax].Mat33.m10
  m2m [edx].Mat33.m02,[eax].Mat33.m20
  m2m [edx].Mat33.m10,[eax].Mat33.m01   
  m2m [edx].Mat33.m11,[eax].Mat33.m11
  m2m [edx].Mat33.m12,[eax].Mat33.m21
  m2m [edx].Mat33.m20,[eax].Mat33.m02
  m2m [edx].Mat33.m21,[eax].Mat33.m12
  m2m [edx].Mat33.m22,[eax].Mat33.m22
endm

;=================================================================
;Make the matrix a rotation matrix from the quaternion [x y z w].
;	pq                  -   quaternion to make into matrix
;
;   w = cos(theta / 2)
;   [x y z] = axis * sin(theta / 2)
;
; This assumes the quaternion is normalised...
; If not the matrix will not be orthogonal.
; See Mat33FromQuatL2 for a version that automatically Normalizes for you..
;=================================================================
Mat33FromQuat proc uses eax pmatOut,pq
LOCAL sq:Vec3
LOCAL xy,wz,xz,wy,yz,wx
;    float x2, y2, z2;
;    x2 = q.x * q.x;
;    y2 = q.y * q.y;
;    z2 = q.z * q.z;
    fldVec3 pq
    fmulVec3 pq
    lea eax,sq
    fstpVec3 eax
    
;    float xy, wz, xz, wy, yz, wx;
;    xy = q.x * q.y;
;    wz = q.w * q.z;
;    xz = q.x * q.z;
;    wy = q.w * q.y;
;    yz = q.y * q.z;
;    wx = q.w * q.x;
    mov edx,pq
    fld  [edx].QUAT.x
    fmul [edx].QUAT.y
    fstp xy
    fld  [edx].QUAT.w
    fmul [edx].QUAT.z
    fstp wz
    fld  [edx].QUAT.x
    fmul [edx].QUAT.z
    fstp xz
    fld  [edx].QUAT.w
    fmul [edx].QUAT.y
    fstp wy
    fld  [edx].QUAT.y
    fmul [edx].QUAT.z
    fstp yz
    fld  [edx].QUAT.w
    fmul [edx].QUAT.x
    fstp wx

	mov eax,pmatOut
;    m[0] = 1.0 - 2.0 * (y2 + z2);
	fld1
	fld  sq.y
	fadd sq.z
	fadd st(0),st(0)
	fsub
	fstp [eax].Mat33.m00	
;    m[1] = 2.0 * (xy - wz);
	fld  xy
	fsub wz
	fadd st(0),st(0)
	fstp [eax].Mat33.m01
;    m[2] = 2.0 * (xz + wy);
	fld  xz
	fadd wy
	fadd st(0),st(0)
	fstp [eax].Mat33.m02
;    m[3] = 2.0 * (xy + wz);
	fld  xy
	fadd wz
	fadd st(0),st(0)
	fstp [eax].Mat33.m10
;    m[4] = 1.0 - 2.0 * (z2 + x2);
	fld1
	fld  sq.z
	fadd sq.x
	fadd st(0),st(0)
	fsub
	fstp [eax].Mat33.m11
;    m[5] = 2.0 * (yz - wx);
	fld  yz
	fsub wx
	fadd st(0),st(0)
	fstp [eax].Mat33.m12
;    m[6] = 2.0 * (xz - wy);
	fld  xz
	fsub wy
	fadd st(0),st(0)
	fstp [eax].Mat33.m20
;    m[7] = 2.0 * (yz + wx);
	fld  yz
	fadd wx
	fadd st(0),st(0)
	fstp [eax].Mat33.m21
;    m[8] = 1.0 - 2.0 * (x2 + y2);
	fld1
	fld  sq.x
	fadd sq.y
	fadd st(0),st(0)
	fsub
	fstp [eax].Mat33.m22
	;
	ret
Mat33FromQuat endp

;=================================================================
;Make the matrix a rotation matrix from the quaternion [x y z w].
;	pq                  -   quaternion to make into matrix
;	l2                  -   length^2 of quaternion
;
;   w = cos(theta / 2)
;   [x y z] = axis * sin(theta / 2)
;
;Will always be orthogonal even if quaternion not normalised 
;(because the matrix is automatically scaled by l2).
;=================================================================
Mat33FromQuatL2 proc uses eax pmatOut,pq,l2:real4
LOCAL sq:Vec3
LOCAL xy,wz,xz,wy,yz,wx
;    float x2, y2, z2;
;    x2 = q.x * q.x;
;    y2 = q.y * q.y;
;    z2 = q.z * q.z;
    fldVec3 pq
    fmulVec3 pq
    lea eax,sq
    fstpVec3 eax
    
;    float xy, wz, xz, wy, yz, wx;
;    xy = q.x * q.y;
;    wz = q.w * q.z;
;    xz = q.x * q.z;
;    wy = q.w * q.y;
;    yz = q.y * q.z;
;    wx = q.w * q.x;
    mov edx,pq
    fld  [edx].QUAT.x
    fmul [edx].QUAT.y
    fstp xy
    fld  [edx].QUAT.w
    fmul [edx].QUAT.z
    fstp wz
    fld  [edx].QUAT.x
    fmul [edx].QUAT.z
    fstp xz
    fld  [edx].QUAT.w
    fmul [edx].QUAT.y
    fstp wy
    fld  [edx].QUAT.y
    fmul [edx].QUAT.z
    fstp yz
    fld  [edx].QUAT.w
    fmul [edx].QUAT.x
    fstp wx
            
            
    mov eax,pmatOut
    fld1
    ;scale to make it orthogonal
;    float scale = 2.0 / l2
    fld l2
    fmul r4_half
    fst scale
;    m[0] = 1.0 - scale * (y2 + z2);
	fld  sq.y
	fadd sq.z
	fmul
	fsub
	fstp [eax].Mat33.m00
;    m[1] = scale * (xy - wz);
	fld  xy
	fsub wz
	fmul scale
	fstp [eax].Mat33.m01
;    m[2] = scale * (xz + wy);
	fld  xz
	fadd wy
	fmul scale
	fstp [eax].Mat33.m02
;    m[3] = scale * (xy + wz);
	fld  xy
	fadd wz
	fmul scale
	fstp [eax].Mat33.m10
;    m[4] = 1.0 - scale * (z2 + x2);
	fld1
	fld scale
	fld sq.z
	fadd sq.x
	fmul
	fsub
	fstp [eax].Mat33.m11
;    m[5] = scale * (yz - wx);
	fld  yz
	fsub wx
	fmul scale
	fstp [eax].Mat33.m12
;    m[6] = scale * (xz - wy);
	fld  xz
	fsub wy
	fmul scale
	fstp [eax].Mat33.m20
;    m[7] = scale * (yz + wx);
	fld  yz
	fadd wx
	fmul scale
	fstp [eax].Mat33.m21
;    m[8] = 1.0 - scale * (x2 + y2);
	fld1
	fld scale
	fld sq.x
	fadd sq.y
	fmul
	fsub
	fstp [eax]/Mat33.m22
	;
	ret
Mat33FromQuatL2 endp


;====================================================================
;A helper proc (used by Mat33EigenVectors, see below).
;====================================================================
EIGENROTATE proc pM1,i,j,k,l
local gtemp,htemp
	mov edx,pM1

	;g=a.n[i][j]
	mov eax,12
	mul j
	mov edx,i
	shl edx,2
	add edx,eax
	add edx,pM1
	m2m gtemp,real4 ptr[edx]

	;h=a.n[k][l]
	mov eax,12
	mul l
	mov ebx,k
	shl ebx,2
	add eax,ebx
	add eax,pM1
	m2m htemp,real4 ptr[eax]	

    ;  a.n[i][j]=g-s*(h+g*tau)
	fld gtemp
	fld gtemp
	fmul tau
	fadd htemp
	fmul s
	fsub
	fstp real4 ptr[edx]

    ;  a.n[k][l]=h+s*(g-h*tau)
	fld gtemp
	fld htemp
	fmul tau
	fsub
	fmul s
	fadd htemp
	fstp real4 ptr[eax]

	ret
EIGENROTATE endp

;====================================================================
;Find the eigen vectors and values for a matrix using the Jacobi rotation method.
;Assumes the matrix is symmetric, i.e. the bottom half is ignored.
;	pM                   -   matrix to find eigen vectors for (symmetric)
;	pV                   -   unit length eigen vectors returned here (in columns)
;	pd                   -   eigen values returned here
;Returns TRUE or FALSE to indicate success/failure
;====================================================================
Mat33EigenVectors proc pM:ptr Mat33, pV:ptr Mat33, pd:ptr Vec3
local M2:Mat33
local j, iq, ip, i
local tresh, theta, tau, t, sm, s, h, g, c, b[3], z[3]
local pm2nipiq

	invoke RtlMoveMemory,addr M2,pM,sizeof M2	; make a copy of M (this gets trashed)

    Mat33_identity pV

;	for (ip=0;ip<3;ip++) {
;		b[ip]=d.m[ip]=M2.n[ip][ip];
;	    z[ip]=0.0;
;	}	
	xor ecx,ecx
	mov ebx,pd
	.while ecx<3
		mov eax,12
		mul ecx
		mov edx,ecx
		shl edx,2
		add eax,edx
		lea edx,M2
		add eax,edx
		fld real4 ptr[eax]
		fst [ebx+ecx*4]
		fstp b[ecx*4]
		mov z[ecx*4],0
		inc ecx
	.endw

;	for (i=0;i<50;i++) {
	mov i,0
	.while i<50

		;for (ip=0;ip<2;ip++) {
		;    for (iq=ip+1;iq<3;iq++)
	    ;        sm += M2.n[ip][iq];
        ;}
		fld  M2.m10	;x0y1
		fadd M2.m20	;x0y2
		fadd M2.m21 ;x1y2
		fstp sm

        .if sm == 0
		    return TRUE
        .endif

		.if i < 3
		    ;tresh=0.2*fabs(sm)/(3*3);
			fld sm
			fmul r4_0pt2
			fabs
			fdiv r4_9
			fstp tresh			
		.else
	        mov tresh,0
		.endif

		;for (ip=0;ip<2;ip++) {
		mov ip,0
		.while ip<2
			; for (iq=ip+1;iq<3;iq++) {
			mov iq,ip
			inc iq
			.while iq<3	   
		        ;g=100.0*fabs(M2.n[ip][iq]);
				mov eax,12
				mul iq
				mov edx,ip
				shl edx,2
				add eax,edx
				lea edx,M2
				add eax,edx
				mov pm2nipiq,eax
				fld real4 ptr[eax]				
				fabs
				fmul r4_100
				fstp g

               ; if (i > 4 && (float)(fabs(d.m[ip])+g) == (float)fabs(d.m[ip])
                ;    	   && (float)(fabs(d.m[iq])+g) == (float)fabs(d.m[iq]))
                    
				.if i>4
					;Check floating IF cases
					mov eax,ip
					shl eax,2
					add eax,pd
					fld real4 ptr[eax]
					fabs
					fadd g
					fcomp real4 ptr[eax]
					fjne @F
					mov eax,iq
					shl eax,2
					add eax,pd
					fld real4 ptr[eax]
					fabs
					fadd g
					fcomp real4 ptr[eax]
					fjne @F

					;M2.n[ip][iq]=0.0;
					fldz
					mov eax,pm2nipiq
					fstp real4 ptr[eax]

@@:
            	.else
					;if (fabs(M2.n[ip][iq]) > tresh) 
					mov eax,pm2nipiq
					fld real4 ptr[eax]
					fabs
					fcomp tresh
					fjle @skip
	                	;h=d.m[iq]-d.m[ip];
						mov edx,iq
						shl edx,2
						add edx,pd
						fld real4 ptr [edx]
						mov edx,ip
						shl edx,2
						add edx,pd
						fsub real4 ptr [edx]
						fstp h

	                    ;if ((fabs(h)+g) == fabs(h))
						fld h
						fabs
						fadd g
						fcomp h
						fjne @F
							;t=(M2.n[ip][iq])/h;
							fld real4 ptr [eax]
							fdiv h
							fstp t
							jmp @smallskip
	                   @@:
	                    	;theta=0.5*h/(M2.n[ip][iq]);
							fld h
							fmul r4_half
							fdiv real4 ptr [eax]
							fstp theta
							
	                        ;t=1.0/(fabs(theta)+sqrt(1.0+theta*theta));
							fld1
							fld theta
							fabs
							fld1
							fld theta
							fmul theta
							fadd
							fsqrt
							fadd
							fdiv
							fstp t

	                    	;if (theta < 0.0)
							mov eax,theta
							and eax,80000000h
							.if eax!=0
								 ;t = -t;
								fld t
								fchs
								fstp t
							.endif
	                   ; endif
				@smallskip:

		                ;c=1.0/sqrt(1+t*t)
						fld1
						fld1
						fld t
						fmul st(0),st(0)
						fadd
						fsqrt
						fdiv
						fst c
						
	                	;s=t*c
						fmul t
						fst s

		                ;tau=s/(1.0+c)
						fld1
						fadd c
						fdiv
						fstp tau

	                    ;h=t*M2.n[ip][iq]
						mov eax,pm2nipiq
						fld t
						fmul real4 ptr[eax]
						fstp h

	                	;z[ip] -= h
						mov edx,ip	
						shl edx,2
						fld z[edx]
						fsub h
						fstp z[edx]					

	                	;z[iq] += h
						mov edx,iq
						shl edx,2
						fld z[edx]
						fadd h
						fstp z[edx]	

						;d.m[ip] -= h;
						mov edx,ip
						shl edx,2
						add edx,pd
						fld real4 ptr [edx]
						fsub h
						fstp real4 ptr [edx]

						;d.m[iq] += h;
						mov edx,iq
						shl edx,2
						add edx,pd
						fld real4 ptr [edx]
						fadd h
						fstp real4 ptr [edx]

						;M2.n[ip][iq] = 0.0;
						mov eax,pm2nipiq
						mov real4 ptr[eax],0

		                ;for (j=0;j<ip;j++) {
						xor ecx,ecx
						.while ecx<ip
							invoke EIGENROTATE,addr M2,ecx,ip,ecx,iq
							inc ecx
	                    .endw

	                	;for (j=ip+1;j<iq;j++) {
						mov ecx,ip
						inc ecx
						.while ecx<iq						
							invoke EIGENROTATE,addr M2,ip,ecx,ecx,iq
							inc ecx
						.endw
	                    
	                	;for (j=iq+1;j<3;j++) {
						mov ecx,iq
						inc ecx
						.while ecx<3
							invoke EIGENROTATE,addr M2,ip,ecx,iq,ecx
							inc ecx
	                    .endw
	
	                  	;for (j=0;j<3;j++) {
						xor ecx,ecx
						.while ecx<3
							invoke EIGENROTATE,pV,ecx,ip,ecx,iq
							inc ecx
						.endw
	                    
					@skip:
			    .endif
				inc iq
            .endw
			inc ip
        .endw

    	;for (ip=0;ip<3;ip++) {
		xor ecx,ecx
		mov ebx,pd
		.while ecx<3
        	;b[ip] += z[ip];
			fld z[ecx*4]
			fadd b[ecx*4]
			fst b[ecx*4]

	        ;d.m[ip] = b[ip];
			fstp [ebx+ecx*4]
			
	        ;z[ip]=0.0;
			mov z[ecx*4],0
			inc ecx
        .endw

		inc i
    .endw
    return FALSE
Mat33EigenVectors endp

