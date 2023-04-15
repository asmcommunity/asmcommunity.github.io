


;Homer's implementation of ID3DXAllocateHierarchy

;This interface is a user callback interface for 
;the D3DXLoadMeshHierarchyFromX api function.
;THE USER IS NOT MEANT TO CALL THESE METHODS DIRECTLY !!
;It contains four methods for creating and destroying
;the FRAME and MESHCONTAINER structs.
;These structs are custom (extended) variants of D3DXFRAME 
;and D3DXMESHCONTAINER which are defined in D3DX9Anim.inc


;Our extended D3DXFRAME struct
FRAME struct
;D3DXFRAME members
	Base D3DXFRAME <>
;Extra stuff
	matCombined D3DXMATRIX <>;16A
;Optional : RagDoll physics data
	pRagDollBone dd ?
FRAME ends

;Our extended D3DXMESHCONTAINER struct
MESHCONTAINER struct
;D3DXMESHCONTAINER members
	Base D3DXMESHCONTAINER <>
;Extra stuff	
	;Mesh variables
    ppTextures dd ?
	pMaterials9 dd ?
	
	;Skinned mesh variables
	pSkinMesh  		dd ?;LPD3DXMESH           
	ppBoneOffsetMatrices 	dd ?;array of ptrs to 'BoneOffset Matrices'
	ppFrameMatrices dd ?;LPD3DXMATRIX	array of ptrs to Frame matrices
	ppEffects 		dd ?

	; Attribute table stuff
	pAttributeTable    dd ?;LPD3DXATTRIBUTERANGE The attribute table
    NumAttributeGroups dd ?;The number of attribute groups

MESHCONTAINER ends



;Here we define the interface structure for the ICall macro... 
;If we dont use ICall, we don't need it!

;BEGIN_INTERFACE ID3DXAllocateHierarchy, IUnknown, <00000000-0000-0000-0000-000000000000>  ;????
;  STDMETHOD CreateFrame,          Pointer, Pointer
;  STDMETHOD CreateMeshContainer,  Pointer, Pointer, Pointer, Pointer, dword, Pointer, Pointer, Pointer
;  STDMETHOD DestroyFrame,         Pointer
;  STDMETHOD DestroyMeshContainer, Pointer
;END_INTERFACE

ID3DXAllocateHierarchyID equ 66676
Object ID3DXAllocateHierarchy,ID3DXAllocateHierarchyID, Primer
  InterfaceMethod CreateFrame,          Pointer, Pointer
  InterfaceMethod CreateMeshContainer,  Pointer, Pointer, Pointer, Pointer, dword, Pointer, Pointer, Pointer
  InterfaceMethod DestroyFrame,         Pointer
  InterfaceMethod DestroyMeshContainer, Pointer
  ;This extra data variable is special... 
  DefineVariable pModelBeingLoaded,Pointer
ObjectEnd

if IMPLEMENT

Method ID3DXAllocateHierarchy.CreateFrame,uses ebx ecx esi edi, pName, ppNewFrame
LOCAL pFrame
LOCAL nNameSize

	; Initialize the output frame
	mov eax,ppNewFrame
	mov dword ptr[eax] , NULL
	
	; Create a frame (using my derived struct here)
	mov pFrame , $MemAlloc(sizeof FRAME,MEM_INIT_ZERO)
	.if eax==NULL
		DbgWarning "Error : out of memory","ID3DXAllocateHierarchy.CreateFrame"
		return E_FAIL
	.endif	

	; Copy the name of the frame (if any)
	mov eax,pName
	.if eax!=0 && byte ptr[eax]!=0
	    DbgStr pName,"FRAME NAMES"
		invoke lstrlen,pName 
		inc eax
		mov nNameSize,eax
		mov ebx,pFrame
		mov [ebx].FRAME.Base.pName , $MemAlloc(nNameSize,MEM_INIT_ZERO)
		invoke lstrcpy,eax, pName
	.else
		mov ebx,pFrame
		mov [ebx].FRAME.Base.pName ,NULL
	.endif
	
	; Initialize the frame's transform and combined matrices
	mov ebx,pFrame
	push ebx
	invoke D3DXMatrixIdentity,addr [ebx].FRAME.matCombined
	pop ebx
	invoke D3DXMatrixIdentity,addr [ebx].FRAME.Base.TransformationMatrix

	; Set the output frame to the one that we made
	mov eax,ppNewFrame
	m2m [eax],pFrame

	; Return 'Success' value
    mov eax, S_OK
MethodEnd


Method ID3DXAllocateHierarchy.CreateMeshContainer,uses ebx ecx esi edi,pName,pMeshData,pMaterials, pEffectInstances, NumMaterials, pAdjacency, pSkinInfo,  ppNewMeshContainer 
LOCAL pMeshContainer,nNameSize, uBones,dwFaces,matl

	DbgWarning "CreateMeshContainer"
	; Initialize passed in Container
	mov eax,ppNewMeshContainer
	mov dword ptr [eax],NULL

    ; Allocate a meshcontainer struct (using my derived struct here)
	mov pMeshContainer , $MemAlloc(sizeof MESHCONTAINER,MEM_INIT_ZERO)
	.if eax==NULL
		DbgWarning "Error : out of memory","ID3DXAllocateHierarchy.CreateMeshContainer"
		return E_FAIL
	.endif
	
	;Copy the D3DXMESHDATA from input param
	;Among other things, the original pMesh is in here..
	invoke RtlMoveMemory,addr [eax].MESHCONTAINER.Base.MeshData,pMeshData,sizeof D3DXMESHDATA
	
	;Deal with the Mesh Name (if any)
	mov eax,pName
	.if eax!=0 && byte ptr[eax]!=0
;		DbgStr eax,"Name of Mesh"
		invoke lstrlen,pName
		inc eax
		mov nNameSize,eax
		mov ebx,pMeshContainer
		mov [ebx].MESHCONTAINER.Base.pName ,$MemAlloc(nNameSize,MEM_INIT_ZERO)
		invoke lstrcpy,eax,pName
	.else
;		DbgText "Nameless Mesh","Name of Mesh"
		mov ebx,pMeshContainer
		mov [ebx].MESHCONTAINER.Base.pName ,NULL
	.endif
	
	;Set our Mesh Type
	mov ebx,pMeshContainer
	mov [ebx].MESHCONTAINER.Base.MeshData.dType , D3DXMESHTYPE_MESH
	
	; Get the number of Faces
	mov ebx,pMeshData
	mov dwFaces ,$ICall ([ebx].D3DXMESHDATA.pMesh::ID3DXBaseMesh.GetNumFaces)
	DbgDec dwFaces

	; Get the number of Materials
	mov ebx,pMeshContainer
	m2m [ebx].MESHCONTAINER.Base.NumMaterials , NumMaterials
	DbgDec NumMaterials
	;Allocate array of Materials
	mov eax,sizeof D3DMATERIAL9
	mul NumMaterials
	mov [ebx].MESHCONTAINER.pMaterials9,$MemAlloc(eax)
	.if eax==NULL
		DbgWarning "Error : Out of memory in CreateMeshContainer"
		invoke ExitProcess,0
	.endif

	;Allocate array of ptrs to Textures
	mov eax,NumMaterials
	shl eax,2
	mov ebx,pMeshContainer
	mov [ebx].MESHCONTAINER.ppTextures,$MemAlloc(eax,MEM_INIT_ZERO)
	.if eax==NULL
		DbgWarning "Error : Out of memory in CreateMeshContainer"
		invoke ExitProcess,0
	.endif
	
	;Allocate and Copy the face adjacency data
	mov eax,3	; Multiply by 3 because there are three adjacent triangles
	mul dwFaces
	shl eax,2 	; Multiply by sizeof DWORD
	push eax
	mov [ebx].MESHCONTAINER.Base.pAdjacency,$MemAlloc(eax)
	.if eax==NULL
		DbgWarning "Error : Out of memory in CreateMeshContainer"
		invoke ExitProcess,0
	.endif
	pop ecx
	invoke RtlMoveMemory,eax,pAdjacency,ecx

	; Get the device to use
	; I believe this is redundant since I have access
	; to a GLOBAL pD3DDevice, so I've commented it out.
;	LPDIRECT3DDEVICE9 pd3dDevice = NULL;// Direct3D Rendering device
;	pMeshData->pMesh->GetDevice(&pd3dDevice);

	; Clone the Mesh exactly (output fvf = input fvf)
	mov ebx,pMeshData
	ICall [ebx].D3DXMESHDATA.pMesh::ID3DXBaseMesh.GetFVF
	push eax
	mov ebx,pMeshData
	pop ecx
	mov edx,pMeshContainer
	ICall [ebx].D3DXMESHDATA.pMesh::ID3DXBaseMesh.CloneMeshFVF,D3DXMESH_MANAGED,ecx,pD3DDevice, addr [edx].MESHCONTAINER.Base.MeshData.pMesh
	DbgText "Cloned the mesh in Bind Pose"
	
	;Process the Materials array, loading Textures on demand
	xor ecx,ecx
	mov matl,ecx
	.while ecx < NumMaterials	
	
		;Preset nth texturepointer to NULL
		mov ebx,pMeshContainer
		mov eax,ecx
		shl eax,2
		add eax,[ebx].MESHCONTAINER.ppTextures
		mov dword ptr[eax], NULL

		;Process the nth Material...		
		mov eax,sizeof D3DXMATERIAL
		mul matl
		add eax,pMaterials		
		push eax
		
		mov eax,sizeof D3DMATERIAL9
		mul ecx
		add eax,[ebx].MESHCONTAINER.pMaterials9
		
		;Copy Material
		pop ebx
		push ebx
		invoke RtlMoveMemory,eax,ebx,sizeof D3DMATERIAL9
				
		pop eax
		
		;Does it have a TextureName?	
		mov ebx,[eax].D3DXMATERIAL.pTextureFilename
		.if ebx!=0 && byte ptr[ebx]!=0
			;Calculate address of nth TexturePointer (again)
			;since this is where to store the loaded Texture
			mov ecx,pMeshContainer
			mov ecx,[ecx].MESHCONTAINER.ppTextures
			mov eax,matl
			shl eax,2
			add ecx,eax
			;Try to load the Texture
			push ebx
			invoke D3DXCreateTextureFromFile,pD3DDevice, ebx, ecx
			pop ebx
			.if FAILED(eax)
				DbgWarning "Failed to load Texture"
				DbgStr ebx,"Failure sucks, eh?"
				DbgText "(setting texture to NULL)",0
				mov ecx,pMeshContainer
				mov ecx,[ecx].MESHCONTAINER.ppTextures
				mov eax,matl
				shl eax,2
				add ecx,eax
				mov dword ptr[ecx],0
			.endif
			
		.endif
		inc matl
		mov ecx,matl
	.endw

	;Release the device
	;This is redundant, see notes above.
;	SAFE_RELEASE pd3dDevice

	;If we have Skin Information..
	.if pSkinInfo!=0
		; first save off the SkinInfo and original mesh data
		mov ebx,pMeshContainer
	    m2m [ebx].MESHCONTAINER.Base.pSkinInfo , pSkinInfo
	    
	     ;AddRef will cause DX to NOT free the SkinInfo
	     ;along with other resources when Loading completes
	    ICall pSkinInfo::ID3DXSkinInfo.AddRef

		; Get the number of Bones
		mov uBones ,$ICall (pSkinInfo::ID3DXSkinInfo.GetNumBones)

		; Allocate array of ptrs to each bone's FrameMatrix
		mov eax,uBones
		shl eax,2
		mov [ebx].MESHCONTAINER.ppFrameMatrices, $MemAlloc(eax)

	    ; Allocate array of ptrs to INVERSE BoneMatrices 
	    ; These matrices are for transforming the mesh vertices
	    ; "from figure space to bone space"
	    ; ie, they "move bodyparts to the figure origin".
		mov eax,uBones
		shl eax,2
	    mov ebx,pMeshContainer
	    mov [ebx].MESHCONTAINER.ppBoneOffsetMatrices , $MemAlloc(eax)	
	    .if eax==NULL
	    	DbgWarning "Error - out of memory (ppBoneOffsetMatrices)"
	    	invoke ExitProcess,0 
	    .endif    
	    ; Fill the array we just allocated
		mov edi,pMeshContainer
		mov edi,[edi].MESHCONTAINER.ppBoneOffsetMatrices
		xor ecx,ecx
		.while ecx<uBones			
			push ecx
			mov ebx,pMeshContainer
			ICall [ebx].MESHCONTAINER.Base.pSkinInfo::ID3DXSkinInfo.GetBoneOffsetMatrix,ecx
			pop ecx
			push ecx
			shl ecx,2
			mov ebx,pMeshContainer
			add ecx,[ebx].MESHCONTAINER.ppBoneOffsetMatrices
			mov dword ptr[ecx],eax
			pop ecx
			inc ecx
	    .endw

	
	.else
		mov ebx,pMeshContainer
		mov [ebx].MESHCONTAINER.Base.pSkinInfo, NULL
		mov [ebx].MESHCONTAINER.ppBoneOffsetMatrices ,  NULL
		mov [ebx].MESHCONTAINER.pSkinMesh , 	NULL
		mov [ebx].MESHCONTAINER.ppFrameMatrices,NULL
	.endif

	mov ebx,pMeshContainer
	mov [ebx].MESHCONTAINER.Base.pMaterials , NULL
	mov [ebx].MESHCONTAINER.ppEffects  		, NULL

	;pMeshContainer->MeshData.pMesh->OptimizeInplace(
	;	D3DXMESHOPT_VERTEXCACHE|D3DXMESHOPT_COMPACT|D3DXMESHOPT_ATTRSORT,
	;	pMeshContainer->pAdjacency,NULL,NULL,NULL);

	; Set the output mesh container to the temp one
	mov eax,ppNewMeshContainer
	m2m dword ptr[eax],pMeshContainer

    mov eax, S_OK
MethodEnd 

Method ID3DXAllocateHierarchy.DestroyFrame,uses ebx, pFrame
	mov ebx,pFrame
	SafeFree [ebx].FRAME.Base.pName
	SafeFree [ebx].FRAME.pRagDollBone
    MemFree pFrame
    return S_OK
MethodEnd  

Method ID3DXAllocateHierarchy.DestroyMeshContainer,uses ebx ecx,pMeshContainer

	mov ebx,pMeshContainer
	SafeFree [ebx].MESHCONTAINER.Base.pName
	SafeFree [ebx].MESHCONTAINER.pMaterials9

	;Release the textures
	.if [ebx].MESHCONTAINER.ppTextures!=0
		xor ecx,ecx
		.while ecx<[ebx].MESHCONTAINER.Base.NumMaterials
			push ecx
			push ebx
			shl ecx,2
			add ecx,[ebx].MESHCONTAINER.ppTextures
			SafeRelease dword ptr[ecx]
			pop ebx
			pop ecx
			inc ecx
		.endw
		SafeFree [ebx].MESHCONTAINER.ppTextures
	.endif
	
	SafeFree [ebx].MESHCONTAINER.Base.pAdjacency
	SafeFree [ebx].MESHCONTAINER.ppBoneOffsetMatrices
	SafeFree [ebx].MESHCONTAINER.ppFrameMatrices
	SafeFree [ebx].MESHCONTAINER.pAttributeTable
	SafeRelease [ebx].MESHCONTAINER.pSkinMesh
	SafeRelease [ebx].MESHCONTAINER.Base.MeshData.pMesh
	SafeRelease [ebx].MESHCONTAINER.Base.pSkinInfo
	MemFree pMeshContainer
	
    mov eax, S_OK
MethodEnd

endif


;When the interface pointer is required, simply pass the instance pointer of the ID3DXAllocateHierarchy
; instance you get with $New(ID3DXAllocateHierarchy)

