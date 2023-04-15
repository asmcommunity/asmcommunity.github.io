
.data
;If penetration depth is less than this,
;we consider the penetration as 'trivial'
fEpsilon_Penetration real4 0.01f
.code

;Enumeration of 3D Geometric Shapes
SHAPEID_UNDEFINED equ -1
SHAPEID_PLANE     equ 0     ;derived from CollisionShape
SHAPEID_TRIANGLE  equ 1     ;derived from CollisionPlane
SHAPEID_SPHERE    equ 2     ;derived from CollisionBody

;Types of Physical Material supported by the physics engine:
MATERIALID_UNDEFINED equ -1
MATERIALID_WOOD	    equ 0
MATERIALID_STONE	equ 1
MATERIALID_ICE		equ 2
MATERIALID_GLASS	equ 3
MATERIALID_METAL	equ 4
MATERIALID_RUBBER	equ 5
MATERIALID_VELCRO	equ 6
MATERIALID_MEAT	    equ 7

;Enumeration of Collision result
Penetrating equ -1
Clear       equ 0
Colliding   equ 1

;This struct describes the minimal physical State for collision checking.
;Note that this describes a physical body 'without mass',
;although 'CM' refers to the Center of Mass of the object, which is ASSUMED to be its Origin.
state struct
    CMPosition        Vec3  <>          ;Position in world space
    CMVelocity        Vec3  <>          ;Rate of change of Position
    CMForce           Vec3  <>          ;Linear force at center of mass
    Orientation       Mat33 <>          ;rotation matrix, bodyspace to worldspace
    AngularVelocity   Vec3  <>          ;Rate of change of orientation
    AngularMomentum   Vec3  <>          ;
    CMTorque          Vec3  <>          ;Angular force at center of mass
    dCollisionState   dword Clear       ;Clear, Colliding, or Penetrating
state ends


; ——————————————————————————————————————————————————————————————————————————————————————————————————


;The CollisionShape class is our baseclass for collidable entities.
Object CollisionShape, 345343, Primer
    RedefineMethod Init, Pointer,  dword
    DefineVariable dShapeID,       dword,  SHAPEID_UNDEFINED       ;describes the shape of the 3D geometry
    DefineVariable dMaterialID,    dword,  MATERIALID_UNDEFINED    ;describes the physical material properties    
ObjectEnd

Method CollisionShape.Init,uses esi, pOwner, dShapeID
    SetObject esi
    ACall Init, pOwner
    m2m [esi].dShapeID, dShapeID, edx
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————


;The CollisionPlane class is our most primitive CollisionShape.
Object CollisionPlane, 234234, CollisionShape
    RedefineMethod Init, Pointer, dword, Pointer
    DefineVariable plane, Vec4, {<>}
ObjectEnd

Method CollisionPlane.Init,uses esi,pOwner,dShapeID,pplane
    SetObject esi
    ACall Init, pOwner, dShapeID    
    mov edx,pplane
    m2m [esi].plane.x,[edx].Vec4.x,eax
    m2m [esi].plane.y,[edx].Vec4.y,eax
    m2m [esi].plane.z,[edx].Vec4.z,eax
    m2m [esi].plane.w,[edx].Vec4.w,eax
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————


;The CollisionTriangle class is derived from CollisionPlane.
Object CollisionTriangle,234232,CollisionPlane
    RedefineMethod Init,Pointer,dword,Pointer,Pointer,Pointer
    RedefineMethod Done
    VirtualMethod versus_Point, Pointer
    DefineVariable pPoints,Pointer,NULL
ObjectEnd

Method CollisionTriangle.Init,uses esi,pOwner,dShapeID,pvA,pvB,pvC
LOCAL Plane:Vec4
LOCAL EdgeAB:Vec3
LOCAL EdgeAC:Vec3
    SetObject esi

    ;Keep a local record of the triangle vertices
    mov [esi].pPoints,$MemAlloc(3*sizeof Vec3) 
    mov edx,eax    
    mov eax,pvA
    mov ebx,pvB
    mov ecx,pvC
    Vec3_Load [eax].Vec3
    Vec3_Stow [edx].Vec3
    Vec3_Load [ebx].Vec3
    Vec3_Stow [edx+1*sizeof Vec3].Vec3
    Vec3_Load [ecx].Vec3
    Vec3_Stow [edx+2*sizeof Vec3].Vec3

    ;Calculate the Plane of the triangle    
    Vec3_Sub [ebx].Vec3, [eax].Vec3     ;Calculating Normal
    Vec3_Stow EdgeAB                    ;its the crossproduct
    Vec3_Sub [ecx].Vec3, [eax].Vec3     ;of any two Edges
    Vec3_Stow EdgeAC
    Vec3_Cross EdgeAB,EdgeAC    
    Vec3_Stow Plane
    Vec3_Normalize Plane                ;Normalize the normal
    Vec3_Dot Plane,Plane                ;Calculating the PlaneD
    fchs                                ;= -dot (normal, normal)
    fstp Plane.w
    DbgVec4 Plane
    
    ;Initialize the ancestor CollisionPlane
    ACall Init,pOwner,dShapeID,addr Plane  
      
MethodEnd

Method CollisionTriangle.Done,uses esi
    SetObject esi
    MemFree [esi].pPoints
    ACall Done
MethodEnd


;This procedure uses the Barycentric technique
;to determine whether the given Point
;is located within the given 3D Triangle
;Returns TRUE/FALSE
;pvPoint = Point to be tested
;
;NOTES : The Barycentric coordinates (u,v) can be
;thought of as 'weights' for linear interpolation along
;the triangle edges AB and AC.
;It is therefore possible to convert these coordinates
;into TEXTURE COORDINATES via linear interpolation
;of the texcoords at the three points of the triangle 
Method CollisionTriangle.versus_Point, uses esi, pvPoint
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

    SetObject esi
    ; Compute vectors        
    ;v0 = C - A
    ;v1 = B - A
    ;v2 = P - A
    mov eax,[esi].pPoints
    mov ebx,eax
    mov ecx,eax
    add ebx,1*sizeof Vec3
    add ecx,2*sizeof Vec3
    mov edx,pvPoint
    Vec3_Sub [ecx].Vec3, [eax].Vec3
    Vec3_Stow v0
    Vec3_Sub [ebx].Vec3, [eax].Vec3
    Vec3_Stow v1
    Vec3_Sub [edx].Vec3, [eax].Vec3
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
    fld u
    fldz
    fcompp
    fjle @F
    fld v
    fldz
    fcompp
    fjle @F
    fld  u
    fadd v
    fld1
    fjge @F
    
    return TRUE
    
    @@:
    mov eax, FALSE




MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————


;The CollisionBody class describes a collidable object.
;It has linear and angular velocity (3D speed and spin), but has NO Mass-related properties, 
;and cannot RESOLVE collisions, only DETECT them.
Object CollisionBody, 82124, CollisionShape

    ;Constructor/Destructor methods
    RedefineMethod Init, Pointer, dword

    ;Get/Set methods
    VirtualMethod  SetMass, real4, real8
    VirtualMethod  SetVelocity, Pointer
    VirtualMethod  Get_Closing_And_Relative_Velocities, Pointer
    VirtualMethod  Get_Local_Velocity,Pointer,Pointer   ;pm33_ContactToWorld, pvrelativeContactPosition
    VirtualMethod  Get_Linear_Momentum_Due_To_Rotation,Pointer,Pointer

    ;Collision Detection / Resolution methods
    VirtualMethod  Body_Sweep_Body,    Pointer
    VirtualMethod  Body_Sweep_Plane,   Pointer, Pointer ;pContactOut
    VirtualMethod  Sphere_Sweep_Plane, Pointer, Pointer ;pContactOut, pPlaneIn
    VirtualMethod  Apply_Correction,   Pointer,Pointer
    
    ;Physics Integration
    VirtualMethod  Integrate, real8

    ;Constants
    DefineVariable CoefficientOfRestitution,real4,0.8 ;'bouncyness'
    DefineVariable Density,     real4, 1.0     ;Density of Material - in case we need to calculate Mass from Volume
    DefineVariable Mass,        real4, 1.0     ;Mass of Object - should be either calculated or explicitly set
    DefineVariable OneOverMass, real4, 1.0     ;Inverse of Mass, used to avoid some divisions, speeds up some maths
    DefineVariable fRadius, real4, 0.0         ;BoundingSphere Radius
    DefineVariable InverseBodyInertiaTensor,  Mat33, {<>}

    ;Physical States
    DefineVariable OldState, state, {<>} ;state at the start of the timestep
    DefineVariable NewState, state, {<>} ;state at the  end  of the timestep

    ;Variables
    ;This is the velocity-delta which was caused only by acceleration in this frame
    DefineVariable VelocityChange,   Vec3, {<>}
    ;This is the accumulator of velocity-deltas resulting from collision impulses     
    DefineVariable Velocity_Delta,    Vec3,{<>}
    DefineVariable AngMomentum_Delta, Vec3,{<>}
    
    ;This is the number N of Impulses which have been applied to this Body
    ;due to collisions with N other bodies (usually ONE)    
    DefineVariable dNumContacts,dword,NULL
    
    ;DefineVariable AngMomentumChange,Vec3, {<>} ;angular momentum delta for this frame

    DefineVariable InverseWorldInertiaTensor, Mat33, {<>}
    DefineVariable m44_BodyToWorld, Mat44, {<>}		  ;rotation & translation (no scaling or shear)


    ;Switches
    DefineVariable IsAwake, BOOL,TRUE			        ;Sleeping objects cannot 'cause' a collision
    DefineVariable IsHollow,BOOL,FALSE                  ;TRUE if object is a hollow shell
ObjectEnd

Method CollisionBody.Init,uses esi, pOwner, dShapeID
    SetObject esi
    ACall Init, pOwner, dShapeID
    invoke RtlZeroMemory,addr [esi].OldState,sizeof state*2
    
    Mat33_Identity [esi].OldState.Orientation
MethodEnd

Method CollisionBody.SetOrientation, uses esi, pMat44
    SetObject esi
    mov eax,pMat44
    m2m [esi].OldState.Orientation.m00, [eax].Mat44.m00,edx
    m2m [esi].OldState.Orientation.m01, [eax].Mat44.m01,edx
    m2m [esi].OldState.Orientation.m02, [eax].Mat44.m02,edx
    m2m [esi].OldState.Orientation.m10, [eax].Mat44.m10,edx
    m2m [esi].OldState.Orientation.m11, [eax].Mat44.m11,edx
    m2m [esi].OldState.Orientation.m12, [eax].Mat44.m12,edx
    m2m [esi].OldState.Orientation.m20, [eax].Mat44.m20,edx
    m2m [esi].OldState.Orientation.m21, [eax].Mat44.m21,edx
    m2m [esi].OldState.Orientation.m22, [eax].Mat44.m22,edx
MethodEnd

Method CollisionBody.SetVelocity, uses esi, pVec3
    SetObject esi
    mov eax,pVec3
    m2m [esi].OldState.CMVelocity.x, [eax].Vec3.x,edx
    m2m [esi].OldState.CMVelocity.y, [eax].Vec3.y,edx
    m2m [esi].OldState.CMVelocity.z, [eax].Vec3.z,edx
MethodEnd

Method CollisionBody.SetPosition, uses esi, pVec3
    SetObject esi
    mov eax,pVec3
    m2m [esi].OldState.CMPosition.x, [eax].Vec3.x,edx
    m2m [esi].OldState.CMPosition.y, [eax].Vec3.y,edx
    m2m [esi].OldState.CMPosition.z, [eax].Vec3.z,edx
MethodEnd

Method CollisionBody.SetSpin, uses esi, pVec3
    SetObject esi
    mov eax,pVec3
    m2m [esi].OldState.AngularVelocity.x, [eax].Vec3.x,edx
    m2m [esi].OldState.AngularVelocity.y, [eax].Vec3.y,edx
    m2m [esi].OldState.AngularVelocity.z, [eax].Vec3.z,edx
MethodEnd

Method CollisionBody.SetMass, uses esi, fDensity:real4, fVolume:real8
    SetObject esi
    fld1
    fld fDensity
    fmul fVolume
    fst [esi].Mass
    fdiv
    fstp [esi].OneOverMass
MethodEnd

QuadraticFormula macro

; q = b*b - 4*a*c
	fld _b
	fmul st(0),st(0)
	fld _a
	fmul _c
	fadd st(0),st(0)
	fadd st(0),st(0)
	fsub
	fstReg eax	
    ;if q >= 0 
	.ifBitSet eax, BIT31
	   fUnload
        mov eax, FALSE	;complex root
    .else
        ; sq = sqrt(q)
        fsqrt
        fstp sq   
        ;d = 1 / (2*a)
        fld1
        fld _a
        fadd st(0),st(0)
        fdiv
        fstp _d
        ;u0 = ( -b + sq ) * d
        fld _b
  ;      fchs
        fadd sq
        fmul _d
        fstp u0        
        ;u1 = ( -b - sq ) * d
        fld _b
   ;     fchs
        fsub sq
        fmul _d
        fstp u1
	 ;load fpu with T = smaller of two impact times			
        fMin u0, u1		
        mov eax,TRUE
	.endif
endm

;Sweep for collision of moving sphere and fixed plane
;Possible return values are:
;eax=Penetrating - Body was penetrating in previous frame (pSource)
;eax=Colliding   - Body will collide with Plane at point pCi, the Time scalar is on the fpu stack
;eax=Clear		 - Body does not collide with Plane during this frame
Method CollisionBody.Sphere_Sweep_Plane,uses esi,pContact,pPlane
local u:real8		;normalized time of collision
LOCAL d0, d1
LOCAL vab
LOCAL ftemp:real8
	SetObject esi

    mov eax,pPlane
	;Calculate distance from starting position to plane
    Vec3_PlaneDotCoord [esi].OldState.CMPosition, [eax].CollisionPlane.plane
    fst d0	
    ;Calculate distance from end position to plane
    Vec3_PlaneDotCoord [esi].NewState.CMPosition, [eax].CollisionPlane.plane
    fst d1
    
    ;If d0=d1 , the sphere is not moving
    ;If d0<d1 , the sphere is moving away from the plane
    fsub
    fstpReg edx
 ;   .if edx==0 || edx==80000000h
 ;      DbgWarning "Sphere is not moving"
 ;      return Clear
 ;   .else
 ;       .ifBitSet edx,BIT31
 ;       	DbgWarning "Sphere moving away from Plane"
 ;   	    return Clear
 ;       .endif
 ;  .endif
    
    DbgFloat d0
    DbgFloat [esi].fRadius
	;check if sphere was touching on previous frame	
	fld d0
    fabs
	fsub [esi].fRadius
	fchs
	fst ftemp
	fstpReg eax    
	.if eax==0 || eax==80000000h	
	    ;The Body is 'just touching' the Plane
	    DbgWarning "Detected a 'perfect' collision state"
	    fldz
	    return Colliding
	.endif
	
	.ifBitSet eax,BIT31

		;check if the sphere penetrated during this frame		
		fMin d1, [esi].fRadius
		fstpReg eax
		.if eax==d1
		    ;Calculate the Time where contact occurred
		    ;The result will be in 'Normalized time'
		    ;IE, will be a scalar representing a fraction of the input delta
		    ;u = (d0-r)/(d0-d1)
		    fld  d0
		    fsub [esi].fRadius
		    fld  d0
		    fsub d1
		    fdiv
		    fst u	;<--leave on fpu stack
		    DbgFloat u,"Normalized collision time"
		    
		    .if pContact!=0			    
		        ;Calculate point of intersection on plane
		        ;(the place where 'first contact' occurred)		        
			    ;Contact = (1-u)*C0 + u*C1
			    Vec3_Load [esi].OldState.CMPosition
			    fld1
			    fsub u
			    fmul st(3),st(0)
			    fmul st(2),st(0)
			    fmul
			    mov edx,pContact
			    Vec3_Stow        [edx].Vec3
			    Vec3_Scale       [esi].NewState.CMPosition, u
			    Vec3_AddToTarget [edx].Vec3, [edx].Vec3	
			.endif	    
		    mov eax, Colliding
		.else
			mov eax,Clear
		.endif

	.else
    	;if abs(d0) <= r then we're already penetrating	
		;Sphere was ALREADY penetrating the Plane
		;We should never have let this happen.. We're screwed?
		fMin ftemp, fEpsilon_Penetration
		fstpReg eax
		.if eax==fEpsilon_Penetration
		    ;The penetration is significantly large..
		    ;Tell the Simulator to attempt 'Penetration Correction'.
		    fld ftemp
		    mov eax,Penetrating
		.else
		    fldz
		    mov eax,Colliding
		.endif	
	.endif

MethodEnd



;Perform Broad-Phase collision testing of a pair of Bodies
;Convention is that 'this body' is the potential 'aggressor', colliding with 'other body'.
;Returns EAX = TRUE (success) or FALSE (failed)
;        If eax=TRUE, EDX = Clear, Penetrating or Colliding
;        If edx = Colliding, FPU contains delta time until impact will occur
;
Method CollisionBody.Body_Sweep_Body,uses esi, pOtherBody
LOCAL psrcA,ptgtA	;Source and Target configs for BodyA (this)
LOCAL psrcB,ptgtB	;Source and Target configs for BodyB (other)
LOCAL vA:Vec3,vB:Vec3,vtemp:Vec3
LOCAL AB:Vec3	
LOCAL rab2:real8	;squared sum of radii
LOCAL _a, _b, _c, _d
LOCAL u0,u1			;normalized time of first and second collisions
LOCAL sq
LOCAL ClosingVelocity:real4
LOCAL RelativeVelocity:Vec3

    SetObject esi
   .if [esi].IsAwake==TRUE
        ;Only AWAKE Bodies are potential Aggressors in my collision tests..
        ;Sleeping bodies are not moving, so they can't collide with anything.

        .switch [esi].dShapeID
        .case SHAPEID_UNDEFINED
            DbgWarning "Error in CollisionBody.BroadSweep : Body A - undefined Type"
            
        .case SHAPEID_SPHERE
            mov eax,pOtherBody      
            .switch [eax].CollisionBody.dShapeID
            .case SHAPEID_UNDEFINED
                 DbgWarning "Error in CollisionBody.BroadSweep : Body A - undefined Type"
            .case SHAPEID_SPHERE
                ;perform sphere/sphere sweep test...	
                ;Get the Closing velocity of the Spheres, and also the Relative velocity vector
                OCall esi.Get_Closing_And_Relative_Velocities, pOtherBody
                fstp ClosingVelocity
                Vec3_Stow RelativeVelocity

                ;Make sure that the Spheres are actually approaching one another..
                ;If we find that they are actually moving apart, then collision isnt possible and we can quit early	
                .ifBitSet ClosingVelocity,BIT31
                    DbgWarning "Spheres are Approaching"
                .else
                    DbgWarning "Spheres are Separating"
                    mov eax,TRUE
                    mov edx,Clear
                    ExitMethod
	            .endif
	
                ;Calculate vector between starting points
                ; AB = previous pos B  - previous pos A
                mov eax,pOtherBody
                Vec3_Sub [eax].CollisionBody.OldState.CMPosition, [esi].OldState.CMPosition
                Vec3_Stow AB

                ;Calculate sum of sphere radii, squared
                ; rab = ra + rb
                fld  [esi].fRadius
                fadd [eax].CollisionBody.fRadius
                fmul st(0),st(0)
                fstp rab2			; = sum of sphere radii, squared	

                Vec3_Dot RelativeVelocity, RelativeVelocity
                fstp _a				; = Relative Velocity, squared
                Vec3_Dot RelativeVelocity,AB
                fadd st(0),st(0)
                fstp _b				; = Relative Velocity * Starting Distance * 2
                Vec3_Dot AB, AB
                fsub rab2
                fstp _c				; = Starting Separation/Penetration Depth, squared

                ;check if spheres are (already) overlapping (at their starting positions)
                .ifBitSet _c,BIT31
                    ;The spheres are ALREADY intersecting in their START POSITIONS
                    ;We're going to have to check the Bodies for Penetration.
                    DbgWarning "Spheres were already intersecting in the Previous Frame"
                    mov eax, TRUE
                    mov edx,Penetrating
                .else
                    ;check if spheres will collide anywhere along their travel paths
                    QuadraticFormula
                    .if eax==TRUE 			;FPU contains T = smaller of two Collision Times
                        mov edx, Colliding  ;Note: T is Normalized (its a time scalar)
                    .else
                        ;They can't possibly collide ..
                        mov eax,TRUE
                        mov edx,Clear
                    .endif
                .endif

            ;return eax=TRUE, edx=RESULT
            .default
                DbgWarning "Error in CollisionBody.BroadSweep : Body A - unhandled Type B"
                int 3
            .endsw
        .default
            DbgWarning "Error in CollisionBody.BroadSweep : Body A - unhandled Type A"
            int 3
        .endsw
    .endif
    mov eax,FALSE
MethodEnd


;Possible return values are:
;eax=Penetrating - Body was penetrating in previous frame (pSource)
;eax=Colliding   - Body will collide with Plane at point pIntersectionOut
;                  the Time scalar is on the fpu stack
;eax=Clear		 - Body does not collide with Plane during this frame
Method CollisionBody.Body_Sweep_Plane,uses esi, pIntersectionOut,pPlane

local u:real8		;normalized time of collision
LOCAL d0, d1
LOCAL adjusted_plane:Vec4

	SetObject esi
    .switch [esi].dShapeID
    .case SHAPEID_UNDEFINED
        DbgWarning "Undefined BodyShape in Body_Sweep_Plane"
        int 3
    .case SHAPEID_SPHERE
        OCall esi.Sphere_Sweep_Plane,  pIntersectionOut ,pPlane   
    .default
        DbgWarning "Unhandled BodyShape in Body_Sweep_Plane"
        int 3
    .endsw



MethodEnd


;** Returns values on FPU
;Calculate the Closing Velocity of two moving Bodies
;This is a floating point value whose Sign indicates whether the Bodies
;are moving toward each other (negative), or away from each other (positive)
;and whose scale indicates the magnitude of the relative velocity.
;Returns FOUR values on the FPU
;st(0) contains the Closing Velocity
;st(1-3) contains the DIRECTION of the Relative Velocity
Method CollisionBody.Get_Closing_And_Relative_Velocities,uses esi edi,pOtherBody
LOCAL pOtherConfig
LOCAL vCollisionNormal_AB:Vec3
LOCAL vCollisionNormal_BA:Vec3
LOCAL dpa:Vec3, dpb:Vec3

	SetObject esi


	; Calculate 'change in position' of 'this' body
	Vec3_Sub  [esi].NewState.CMPosition, [esi].OldState.CMPosition
	Vec3_Stow dpa
	
	; Calculate 'change in position' of 'other' body
	mov edx,pOtherBody
	Vec3_Sub  [edx].CollisionBody.NewState.CMPosition, [edx].CollisionBody.OldState.CMPosition
	Vec3_Stow dpb
	
	; Calculate RELATIVE VELOCITY vector, leave it on the fpu
	Vec3_Sub dpb, dpa

	; Calculate Normal from A to B
	Vec3_Sub [eax].CollisionBody.NewState.CMPosition, [esi].NewState.CMPosition
	Vec3_Stow vCollisionNormal_AB
	Vec3_Normalize vCollisionNormal_AB	

    ; Calculate Normal from B to A
	Vec3_Neg vCollisionNormal_BA, vCollisionNormal_AB

	; Closing Velocity of two Bodies = dpa . Normal (pB-pA) + dpb . Normal (pA-pB)
	Vec3_Dot dpa, vCollisionNormal_AB
	Vec3_Dot dpb, vCollisionNormal_BA
	fadd
MethodEnd

;Calculate the Velocity at the CollisionPoint, with respect to 
;the Center of Mass of this Body... result is in Contact coordinates
;Returns Vec3 on fpu
Method CollisionBody.Get_Local_Velocity,uses esi,pmatContactToWorld, pvrelativeContactPosition
LOCAL velocity:Vec3
LOCAL accVelocity:Vec3
LOCAL vout:Vec3
	SetObject esi
	
	DbgWarning "CollisionBody.Get_Local_Velocity"
    ; Work out the velocity of the contact point.
    mov eax,pvrelativeContactPosition
    Vec3_Cross                 [esi].OldState.AngularVelocity ,[eax].Vec3  ;velocity due to Angular motion
    Vec3_AddToTarget velocity, [esi].OldState.CMVelocity				   ;+velocity due to Linear motion
    lea edx,velocity
    DbgVec3 edx,"in world coords"    

    ; Turn the velocity into contact-coordinates.
    mov eax,pmatContactToWorld
    Mat33_TransMult_Vec3 [eax].Mat33, velocity
    Vec3_Stow vout
    lea edx,vout
    DbgVec3 edx,"velocity in contact coords"
    
    ; Convert "the change in velocity that was due to linear acceleration In This Frame" into contact-coordinates.
    mov eax,pmatContactToWorld    
    Mat33_TransMult_Vec3 [eax].Mat33, [esi].VelocityChange
    Vec3_Stow accVelocity
    lea edx,accVelocity
    DbgVec3 edx,"deltavelocity in contact coords"
    
    ; We ignore any component of acceleration in the contact normal direction, 
    ; we are only interested in 'planar' acceleration
    mov accVelocity.x,0

    ; Add the planar velocities - if there's enough friction they will be removed during velocity resolution
	Vec3_Add vout, accVelocity

MethodEnd

;**Returns float on FPU
;Given a Collision Normal and a Contact Point, 
;this method calculates the Linear Momentum
;that is due ONLY to rotation, and which is in
;the direction of the Collision Normal.
Method CollisionBody.Get_Linear_Momentum_Due_To_Rotation,uses esi,pvContactNormal, pvrelativeContactPosition
LOCAL angularInertiaWorld:Vec3
	SetObject esi
	;Calculate Torque per unit impulse at contact point
	mov edx,pvContactNormal
	mov eax,pvrelativeContactPosition
	Vec3_Cross [eax].Vec3, [edx].Vec3			;torque = R cross N = change in angular momentum over time
	Vec3_Stow angularInertiaWorld
	;Convert Torque into Angular Momentum
	Mat33_Mul_Vec3 [esi].CollisionBody.InverseWorldInertiaTensor, angularInertiaWorld		;rotation = !I * torque = change in angular velocity
	Vec3_Stow angularInertiaWorld
	;Find the Linear part of that Angular motion
	Vec3_Cross angularInertiaWorld , [eax].Vec3
	Vec3_Stow angularInertiaWorld
	;Find the Linear part that is along the Normal
	Vec3_Dot angularInertiaWorld   , [edx].Vec3		
MethodEnd

Method CollisionBody.Apply_Correction,uses esi, pvLinearChange, pvAngularChange
LOCAL matTemp:Mat33
LOCAL matTemp2:Mat33
LOCAL num:real8,ftemp:real8
LOCAL deltaPos
	SetObject esi
	
	;Scale the LINEAR Velocity as follows:
	;CMVelocity and CMVelocityChange from CHANGE IN POSITION
	;S						 = linearchange / old deltaPosition
	;target.CMVelocity		 = target.CMVelocity * S
	;target.CMVelocityChange = target.CMVelocityChange * S	
	
	
	.data
	Threshold_Small_PositionDelta real4 0.1f
	.code
	mov eax,pvLinearChange
	Vec3_Length [eax].Vec3
	fstp num
	fMin num, Threshold_Small_PositionDelta
	fstpReg eax
	.if eax==Threshold_Small_PositionDelta
		;If there was a reasonably large correction in Position,
		;then we will bother to correct the Linear Velocity
		Vec3_Distance  [esi].NewState.CMPosition, [esi].OldState.CMPosition
		fst deltaPos
		fdiv num 		
		fstp ftemp
		DbgFloat ftemp,"Scalar"
		
		Vec3_Scale [esi].OldState.CMVelocity, ftemp
		Vec3_Stow  [esi].NewState.CMVelocity
		
		Vec3_Scale [esi].VelocityChange,    ftemp
		Vec3_Stow  [esi].VelocityChange

		
	.else
	    DbgWarning "Linear position change is small - not correcting linvel"
	.endif
	
	; Apply the change in position     
	mov eax,pvLinearChange
	Vec3_Sub   [esi].NewState.CMPosition,[eax].Vec3
	Vec3_Stow  [esi].NewState.CMPosition    
    	
	; And the change in orientation 
	mov eax,pvAngularChange
	Vec3_Sub 	[esi].NewState.AngularMomentum,[eax].Vec3
	Vec3_Stow 	[esi].NewState.AngularMomentum
	
	;We've changed the POSITION and ANGULAR MOMENTUM
	;We need to fix up the Linear and Angular Velocities

	;AngVelocity from AngMomentum as seen in INTEGRATE...
	;Make a new Orientation matrix based on the Angular Change
	mov eax,pvAngularChange
	Mat33_Star matTemp, [eax].Vec3	
	Mat33_TransMult_Mat33 matTemp2, matTemp, [esi].OldState.Orientation
	Mat33_Add  [esi].NewState.Orientation, matTemp2, [esi].OldState.Orientation  
	Mat33_OrthoNormalize [esi].NewState.Orientation	
	
	;Fix up the Inertia Tensor for the new Orientation
	Mat33_SimilarityTransform [esi].InverseWorldInertiaTensor, matTemp, [esi].InverseBodyInertiaTensor, [esi].NewState.Orientation

	; Extract new Angular Velocity from the new Tensor and Momentum
	; new AngularVelocity = InverseWorldInertiaTensor * new AngularMomentum			
	Mat33_Mul_Vec3 [esi].InverseWorldInertiaTensor, [esi].NewState.AngularMomentum
	Vec3_Stow 	   [esi].NewState.AngularVelocity
	
;	mov edx,pTarget
	;Ensure that our worldspace vertex array is up to date
;	OCall esi.TransformVertices,addr [edx].configuration.CMPosition, addr [edx].configuration.Orientation


	
	
MethodEnd

Method CollisionBody.Integrate,uses esi, deltaTime:real8
LOCAL matTemp:Mat33
LOCAL matTemp2:Mat33
    SetObject esi

    
        DbgVec3 [eax].CollisionBody.OldState.CMPosition,"CURRENT POSITION"
        DbgVec3 [eax].CollisionBody.OldState.CMVelocity,"CURRENT VELOCITY"
        ;/// BEGIN INTEGRATION
        ; Position is integrated from Velocity, applied for some Time:
        ;new position = old position + (old velocity * deltaTime)
        Vec3_Scale [esi].OldState.CMVelocity, deltaTime
        Vec3_AddToTarget [esi].NewState.CMPosition, [esi].OldState.CMPosition
        DbgVec3 [esi].NewState.CMPosition,"PROPOSED NEW POSITION"
        
        ; Linear Velocity is integrated from Linear Force, applied for some Time:
        ;New Velocity = Old Velocity + (OneOverMass * Old Force * DeltaTime)
    	Vec3_Scale2 [esi].OldState.CMForce, [esi].OneOverMass, deltaTime
    	Vec3_Stow   [esi].VelocityChange	
    	DbgVec3 [esi].VelocityChange
    	Vec3_Load   [esi].VelocityChange
    	Vec3_AddToTarget [esi].NewState.CMVelocity,[esi].OldState.CMVelocity	
        DbgVec3 [esi].NewState.CMVelocity,"PROPOSED NEW VELOCITY"

        ;Angular Momentum is integrated from Torque, applied for some Time:
        ;New AngularMomentum = Old AngularMomentum + (Old Torque + DeltaTime)
    	Vec3_Scale [esi].OldState.CMTorque, deltaTime
        Vec3_AddToTarget [esi].NewState.AngularMomentum, [esi].OldState.AngularMomentum

        ;NEW Orientation is found from Ang. Velocity using a Skew Symmetric matrix...
        ;Prepare the SkewSymmetric matrix
        Mat33_Star  matTemp, [esi].OldState.AngularVelocity
        ;Scale the SS matrix with Time
        Mat33_Scale matTemp, deltaTime
        ;Apply the scaled SS matrix to the Orientation to find 'Offset Rotation Matrix'
        Mat33_Mul   matTemp2,   [esi].OldState.Orientation, matTemp
        ;new orientation = old orientation + offset rotation
        Mat33_Add   [eax].CollisionBody.NewState.Orientation, matTemp2, [eax].CollisionBody.OldState.Orientation
    	;Orthogonalizing the result to prevent Skew	(deforming of matrix)
        Mat33_OrthoNormalize [esi].NewState.Orientation

    	; Transform the InvInertiaTensor into WorldSpace via a Similarity transform:
    	; InverseWorldInertiaTensor = New Orientation * InverseBodyInertiaTensor * Transpose(Old Orientation)	
    	Mat33_Transpose matTemp, [esi].OldState.Orientation
        Mat33_Mul matTemp2,[esi].InverseBodyInertiaTensor, matTemp
        Mat33_Mul [esi].InverseWorldInertiaTensor, [esi].NewState.Orientation, matTemp2    

		; Angular Velocity is a PRODUCT of Angular Momentum
		; New AngularVelocity = InverseWorldInertiaTensor * New AngularMomentum			
		Mat33_Mul_Vec3 [esi].InverseWorldInertiaTensor, [esi].NewState.AngularMomentum
		Vec3_Stow      [esi].NewState.AngularVelocity

        ;/// END INTEGRATION    
MethodEnd