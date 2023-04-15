;One of the things that ID3DXAllocateHierarchy does when we
;load a skinmesh is build an array of pointers to the 'inverse
;transformation matrices' of each of the Bones.

;Once you have a bone's inverse transformation matrix, you can 
;iterate through each vertex that is attached to the bone. 
;What you're going to do is transform those vertices using 
;the bone's inverse transformation. Using the coordinates of 
;these newly transformed vertices, you can compute the extents
;of your bounding box (which will eventually enclose each vertex 
;and bone-to-bone connection point for the current bone).

;The GetBoneBoundingBoxSize method calculates this boundingbox.
;The function takes a pointer to a FRAME structure (which represents the bone)
;and also a pointer to a MeshContainer structure (which represents the mesh)

.data
MINIMUM_BONE_SIZE REAL4 1.0f
.code

;This struct describes the Physics state
;of a Bone, its BoundingBox, etc
RagDollBoneState struct
	m_vecPosition Vec3 <>				;  Position
	m_quatOrientation D3DXQUATERNION <>	;  Orientation
	m_matOrientation D3DXMATRIX <>		;  Orientation
	m_vecAngularMomentum Vec3 <>		;  Angular momentum
	m_vecLinearVelocity Vec3 <>			;  Linear velocity
	m_vecAngularVelocity Vec3 <>		;  Angular velocity
; Transformed points, including connection-to-parent
; position and parent-to-bone offset
	m_vecPoints Vec3 10 dup (<>)
; Body's inverse world moment of inertia tensor matrix
	m_matInvWorldInertiaTensor D3DXMATRIX <>
RagDollBoneState ends

;Consider this as an Extension of the FRAME struct.
;We'll attach one of these to each BoneFrame :)
;It contains per-bone data for RagDoll physics state.

RagDollBone struct
; Frame that this bone is connected to
	m_Frame dd ?
; Mass and 1/Mass (one-over-mass)
	m_Mass REAL4 ?
; Coefficient of restitution value
; 0 = no bounce
; 1 = 'super' bounce
; >1 = gain power in bounce
	m_Coefficient REAL4 ?
	m_ParentBone dd ?;Pointer to parent bone CRagDollBone
	; Connection-to-parent offset and parent-to-bone offset
	m_vecJointOffset Vec3 <>
	m_vecParentOffset Vec3 <>
	m_vecBBSize Vec3 <>
	
; Linear force and angular momentum
	m_vecForce Vec3 <>
	m_vecTorque Vec3 <>
; Original orientation of bone
	m_quatOrientation D3DXQUATERNION <>
; Rate of resolution (0-1) to resolve slerp interpolation
; This is used to make bones return to their initial
; orientation relative to its parent.
	m_ResolutionRate REAL4 ?
; Body's inverse moment of inertia tensor matrix
	m_matInvInertiaTensor D3DXMATRIX <>
; Points (in body space) that form bounding box
; and connection-to-parent offset position
	m_vecPoints Vec3 9 dup (<>)
; Bone state
	m_State RagDollBoneState <>
RagDollBone ends

CRagDollID equ 39495
Object CRagDoll,CRagDollID,CModel
	StaticMethod ExtendFrame,Pointer
	StaticMethod BoneIndexFromBoneFrame,Pointer
	StaticMethod GetBoneBoundingBoxSize,Pointer
ObjectEnd

;Extend a Bone's FRAME struct for RagDoll Physics
;Noe that ID3DXAllocateHierarchy now cleans up this resource
Method CRagDoll.ExtendFrame,uses esi,pBoneFrame
	mov ebx,pBoneFrame
	mov [ebx].FRAME.pRagDollBone,$MemAlloc(sizeof RagDollBone,MEM_INIT_ZERO)
MethodEnd

;Search for a Bone whose Name matches that of the given Frame
;Returns: index of matching Bone, or -1 for failure
Method CRagDoll.BoneIndexFromBoneFrame,uses esi,pBoneFrame
LOCAL pSkin
LOCAL numBones
LOCAL BoneNum
	SetObject esi
	.if [esi].m_pFirstMesh==0
		DbgWarning "NULL m_pFirstMesh","CRagDoll.BoneIndexFromBoneFrame"
		invoke ExitProcess,0
	.endif
	mov ebx,[esi].m_pFirstMesh
	m2m pSkin, [ebx].MESHCONTAINER.Base.pSkinInfo	
	; Search for a bone with same name as frame
	mov numBones,$ICall (pSkin::ID3DXSkinInfo.GetNumBones)
	mov BoneNum , -1
	xor ecx,ecx
	.while ecx<numBones
		push ecx
		ICall pSkin::ID3DXSkinInfo.GetBoneName,ecx
		mov ebx,pBoneFrame		
		invoke lstrcmp,eax, [ebx].D3DXFRAME.pName
		pop ecx
		.if eax==0
			mov BoneNum ,ecx
			.break
		.endif
		inc ecx
	.endw
	mov eax,BoneNum
MethodEnd

;Calculate a BoundingBox for a BoneFrame
;which encloses all vertices affected by
;the Bone, as well as the ConnectionPoints
;between the Bone and any other Bones.
;The resulting Box info is stored in 
;the Frame's RagDollBone extension struct.
Method CRagDoll.GetBoneBoundingBoxSize,uses esi,pBoneFrame
LOCAL pSkin:ptr ID3DXSkinInfo
LOCAL pMesh:ptr MESHCONTAINER
LOCAL pFrameChild
LOCAL numBones		;#Bones in Model
LOCAL BoneNum		;Index of our Bone
LOCAL NumVertices	;#Vertices attached to our Bone
LOCAL Stride		;#Bytes per Vertex in original Mesh
LOCAL Vertices		;ptr to array of dword-sized Vertex Indices
LOCAL Weights		;ptr to array of real4 sized Skin Weights
LOCAL pVertices 	;ptr to original mesh VB
LOCAL pvecPtr		;
LOCAL vecPos:Vec3
LOCAL matInvFrame:D3DXMATRIX
LOCAL pmatInvBone	;ptr to Bone's Inverse Offset Matrix
LOCAL vecMin:Vec3
LOCAL vecMax:Vec3


; Set default min and max coordinates
and vecMin.x,0
and vecMin.y,0
and vecMin.z,0
and vecMax.x,0
and vecMax.y,0
and vecMax.z,0 

;The first order of business in GetBoundingBoxSize is
;to find the bone that matches the frame's name.
;This bone, or rather the skinned mesh bone interface object 
;(ID3DXSkinInfo), queries which vertices are connected to the bone.

; Only process bone vertices if there is a bone to work with..
; Interesting Fact : BoneFrames are Named, other frames are not.
mov ebx,pBoneFrame
.if [ebx].D3DXFRAME.pName!=0
	; Get a pointer to ID3DXSkinInfo interface for easier handling.
	SetObject esi	
	.if [esi].m_pFirstMesh==0
		DbgWarning "NULL m_pFirstMesh","CRagDoll.GetBoneBoundingBoxSize"
		invoke ExitProcess,0
	.endif	
	mov ebx,[esi].m_pFirstMesh
	mov pMesh,ebx
	m2m pSkin, [ebx].MESHCONTAINER.Base.pSkinInfo
	
	;Search for a Bone whose name matches the name of the Frame
	mov BoneNum,$OCall (BoneIndexFromBoneFrame,pBoneFrame)

	; Only continue if we found our target bone by name
	.if BoneNum != -1
		;After you've found an ID3DXSkinInfo for the bone in question, 
		;you query it for the number of vertices attached and allocate 
		;arrays of DWORD and REAL4 values to hold the vertex indices 
		;and weights.
		
		; Get the number of vertices attached to the bone
		mov NumVertices,$ICall(pSkin::ID3DXSkinInfo.GetNumBoneInfluences,BoneNum)
		.if eax!=0
			; Get stride of vertex data
			mov ebx,pMesh
			ICall [ebx].MESHCONTAINER.Base.MeshData.pMesh::ID3DXMesh.GetFVF
			mov Stride ,$invoke (D3DXGetFVFVertexSize,eax)

			; Get bone's offset inversed transformation matrix
			mov pmatInvBone,$ICall (pSkin::ID3DXSkinInfo.GetBoneOffsetMatrix,BoneNum)
			
			;Allocate temp memory for arrays 
			;of vertex indices and skin weights
			shl eax,2	;*4 for dword sized
			push eax
			mov Vertices ,$MemAlloc(eax)
			pop eax
			mov Weights  ,$MemAlloc(eax)
			; Get the bone influences (vertices and skinweights)
			ICall pSkin::ID3DXSkinInfo.GetBoneInfluence,BoneNum, Vertices, Weights

			;Now that the vertex indices are stored in the Vertices buffer 
			;(which you accomplished by calling GetBoneInfluence), you can 
			;begin iterating through each vertex, transforming the vertices 
			;by the bone's inverse transformation and using the transformed 
			;vertices to calculate the size of the bounding box.
						
			; Lock vertex buffer and go through all of
			; the vertices that are connected to our bone
			mov ebx,pMesh
			ICall [ebx].MESHCONTAINER.Base.MeshData.pMesh::ID3DXMesh.LockVertexBuffer,D3DLOCK_READONLY, addr pVertices

			xor ecx,ecx
			.while ecx<NumVertices
				push ecx
				; Get pointer to vertex coordinates
				;pvecPtr = pVertices+Vertices[i]*Stride
				shl ecx,2
				add ecx,Vertices
				mov eax,dword ptr[ecx]	;eax=Vertices[i]
				mul Stride				;eax=Vertices[i]*Stride
				add eax,pVertices		;eax=pVertices+Vertices[i]*Stride
				mov pvecPtr,eax
				; Transform vertex by bone offset transformation
				;(ie from Modelspace to Bonespace)
				;This makes the vertices relative to the Bone instead of 
				;to the Origin of the Model.
				invoke D3DXVec3TransformCoord,addr vecPos, pvecPtr, pmatInvBone

				;Get min/max values
				fMin vecMin.x,vecPos.x
				fstp vecMin.x
				fMin vecMin.y,vecPos.y
				fstp vecMin.y
				fMin vecMin.z,vecPos.z
				fstp vecMin.z
				;
				fMax vecMax.x,vecPos.x
				fstp vecMax.x
				fMax vecMax.y,vecPos.y
				fstp vecMax.y
				fMax vecMax.z,vecPos.z
				fstp vecMax.z
				
				pop ecx
				inc ecx
			.endw

			;Unlock VB
			mov ebx,pMesh
			ICall [ebx].MESHCONTAINER.Base.MeshData.pMesh::ID3DXMesh.UnlockVertexBuffer

			; Free temp resources
			MemFree Vertices
			MemFree Weights
		.endif

		;At the end of this bit of code, you'll have the extents of 
		;the bounding box stored in the two vectors (vecMin and vecMax) 
		;you instanced at the beginning of the function. 
		;The array of vertex indices is freed (as well as the vertex weights), 
		;and processing continues by accounting for the point where 
		;the bone connects to its parent and child bones.

		;To factor in the connection points, you basically grab 
		;the world-space coordinates of the connected bones and
		;transform them by the bone's inverse transformation. 
		;These coordinates are then compared to the coordinates
		;stored in the vecMin and vecMax vectors.
	
		; Factor in child bone connection points to size
		mov ebx,pBoneFrame
		.if [ebx].D3DXFRAME.pFrameFirstChild!=0
			; Get the bone's inverse transformation to
			; position child connections.
			mov ebx,pBoneFrame
			invoke D3DXMatrixInverse,addr matInvFrame,NULL,addr [ebx].FRAME.matCombined

			; Go through all child frames connected to this frame
			mov ebx,pBoneFrame
			m2m pFrameChild,[ebx].D3DXFRAME.pFrameFirstChild
			.while pFrameChild!=0
				; Get the frame's vertex coordinates and transform it
				mov ebx,pFrameChild
				fld [ebx].FRAME.matCombined.m30
				fld [ebx].FRAME.matCombined.m31
				fld [ebx].FRAME.matCombined.m32
				fstp vecPos.z
				fstp vecPos.y
				fstp vecPos.x
				invoke D3DXVec3TransformCoord,addr vecPos, addr vecPos, addr matInvFrame

				;Get min/max values
				fMin vecMin.x,vecPos.x
				fstp vecMin.x
				fMin vecMin.y,vecPos.y
				fstp vecMin.y
				fMin vecMin.z,vecPos.z
				fstp vecMin.z
				;
				fMax vecMax.x,vecPos.x
				fstp vecMax.x
				fMax vecMax.y,vecPos.y
				fstp vecMax.y
				fMax vecMax.z,vecPos.z
				fstp vecMax.z
				
				; Go to next child bone
				mov ebx,pFrameChild
				m2m pFrameChild , [ebx].D3DXFRAME.pFrameSibling
			.endw
		.endif
		
		;You can now finish the function by storing the size of the box. 
		;If the box is too small, set the size to a minimum amount 
		;(MINIMUM_BONE_SIZE is set to 1.0f).
	
		; Calc the bounding box size
		mov eax,pBoneFrame		
		mov eax,[eax].FRAME.pRagDollBone
		fld  vecMax.x
		fsub vecMin.x
		fabs
		fstp [eax].RagDollBone.m_vecBBSize.x
		;
		fld  vecMax.y
		fsub vecMin.y
		fabs
		fstp [eax].RagDollBone.m_vecBBSize.y
		;
		fld  vecMax.z
		fsub vecMin.z
		fabs
		fstp [eax].RagDollBone.m_vecBBSize.z
		
		; Make sure each bone has a minimal size
		mov eax,pBoneFrame
		mov eax,[eax].FRAME.pRagDollBone
		fMin [eax].RagDollBone.m_vecBBSize.x,MINIMUM_BONE_SIZE
		fstpReg ebx
		.if ebx == [eax].RagDollBone.m_vecBBSize.x
			mov eax,pBoneFrame
			fld MINIMUM_BONE_SIZE
			fst [eax].RagDollBone.m_vecBBSize.x
			fmul fHalf
			fstp vecMax.x
		.endif

		mov eax,pBoneFrame
		mov eax,[eax].FRAME.pRagDollBone
		fMin [eax].RagDollBone.m_vecBBSize.y,MINIMUM_BONE_SIZE
		fstpReg ebx
		.if ebx == [eax].RagDollBone.m_vecBBSize.y
			mov eax,pBoneFrame
			fld MINIMUM_BONE_SIZE
			fst [eax].RagDollBone.m_vecBBSize.y
			fmul fHalf
			fstp vecMax.y
		.endif
		
		mov eax,pBoneFrame
		mov eax,[eax].FRAME.pRagDollBone
		fMin [eax].RagDollBone.m_vecBBSize.z,MINIMUM_BONE_SIZE
		fstpReg ebx
		.if ebx == [eax].RagDollBone.m_vecBBSize.z
			fld MINIMUM_BONE_SIZE
			fst [eax].RagDollBone.m_vecBBSize.z
			fmul fHalf
			fstp vecMax.z
		.endif
	
		; Set the bone's offset to center based on half the size
		; of the bounding box and the max position
		mov eax,pBoneFrame
		mov eax,[eax].FRAME.pRagDollBone
		lea ebx,[eax].RagDollBone.m_vecJointOffset
		lea eax,[eax].RagDollBone.m_vecBBSize
		fld [eax].Vec3.x
		fmul fHalf
		fsub vecMax.x
		fstp [ebx].Vec3.x
		;
		fld [eax].Vec3.y
		fmul fHalf
		fsub vecMax.y
		fstp [ebx].Vec3.y
		;
		fld [eax].Vec3.z
		fmul fHalf
		fsub vecMax.z
		fstp [ebx].Vec3.z	
	.endif
.endif
MethodEnd

;At the very end of the function, you finally encounter the 
;JointOffset vector object that I mentioned when you started 
;creating the GetBoundingBoxSize function. 
;Because a rigid body bone can be any size, and you track the bone 
;by its center coordinates, you need to create an extra point that 
;represents the point in the bounding box where the bone connects 
;to its parent. This is the joint offset vector. You'll read more about
;the joint offset vector when you enforce the bone-to-bone connections.
;Now that you've computed the bounding-box size and set the various 
;bones' data, you can set the various forces and resolve the motion 
;of your bones.

