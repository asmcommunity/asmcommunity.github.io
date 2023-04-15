Vec4 struct
	x	real4 ?
	y	real4 ?
	z	real4 ?
	w	real4 ?
Vec4 ends

;=====[[ makeShadowSSE >>===\
makeShadowSSE proc ; edi = outVec, esi=inVec, ebx = Plane
	assume esi:ptr Vec4
	assume edi:ptr Vec4
	assume ebx:ptr Vec4
	;----[ make xmm3 = Plane.normal ]-----\
	movups xmm3,[ebx]
	movaps xmm0,xmm3
	mulps  xmm0,xmm0
	movaps xmm1,xmm0
	movhlps xmm2,xmm0
	shufps xmm1,xmm1,1
	addss xmm0,xmm2
	addss xmm0,xmm1 
	rsqrtss xmm0,xmm0
	; now xmm0.0 = 1/sqrt(px^2+py^2+pz^2)
	shufps xmm0,xmm0,0
	mulps xmm3,xmm0 ; now xmm3 = P.normal
	;-------------------------------------/
	;---[ make xmm4 = dist ]---------\
	movups xmm0,[esi]
	movaps xmm4,xmm0
	mulps  xmm4,xmm3
	movaps xmm1,xmm4
	movhlps xmm2,xmm4
	shufps xmm1,xmm1,1
	addss xmm4,xmm2
	addss xmm4,xmm1
	addss xmm4,[ebx].w
	shufps xmm4,xmm4,0
	;--------------------------------/
	;----[ make outvec ]------\
	mulps xmm3,xmm4
	addps xmm3,xmm0
	movups [edi],xmm3
	;-------------------------/	
	
	
	assume esi:nothing
	assume edi:nothing
	assume ebx:nothing
	ret
makeShadowSSE endp
;=======/

makeShadowFPUSSE proc
	local norm:Vec4
	
	assume esi:ptr Vec4
	assume edi:ptr Vec4
	assume ebx:ptr Vec4
	
	;------[ computer norm ]---------\
	fld [ebx].z
	fld [ebx].y
	fld [ebx].x
	fld ST(0) ;x
	fmul ST,ST
	fld ST(2) ; y
	fmul ST,ST
	fadd
	fld ST(3)
	fmul ST,ST
	fadd
	fstp norm.w
	movss xmm0,norm.w
	rsqrtss xmm0,xmm0
	movss norm.w,xmm0
	fld norm.w
	fmul ST(2),ST
	fmul ST(3),ST
	fmul
	fstp norm.x
	fstp norm.y
	fstp norm.z
	;----------------------------------/
	
	;--[ compute dist ]------\
	fld [esi].x
	fmul norm.x
	fld [esi].y
	fmul norm.y
	fld [esi].z
	fmul norm.z
	fadd
	fadd
	fadd [ebx].w
	; ST(0) = dist
	;------------------------/
	
	
	fld ST
	fmul norm.x
	fadd [esi].x
	
	fld ST(1)
	fmul norm.y
	fadd [esi].y
	
	fld ST(2)
	fmul norm.z
	fadd [esi].z
	
	fstp [edi].z
	fstp [edi].y
	fstp [edi].x
	fstp norm.w	
	
	assume esi:nothing
	assume edi:nothing
	assume ebx:nothing
	ret
makeShadowFPUSSE endp
