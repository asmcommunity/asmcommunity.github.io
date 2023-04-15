;In an effort to standardize this demo project,
;I've decided that the pOwner of our Mesh objects
;will be the TextureManager (which will eventually
;be renamed to something like 'ResourceManager').
;That is to say, TextureManager is going to manage
;not just Textures but also all the background
;loading, unloading (and rendering?) of what I'm going
;to call 'Reference Meshes'.
;This may seem awkward at first, but later when 
;we decide to implement a 'deferred rendering scheme'
;it will make more sense to you.
;A Reference object is something that you load once,
;then create N instances of it.
;We're not talking about the management of Instances,
;this is only about management of Reference resources.
;
;SO REMEMBER TO INIT WITH pOwner = pD3DTextureManager

D3DMeshID equ 74483
Object D3DMesh,D3DMeshID,Primer
	RedefineMethod Done
	StaticMethod Load,Pointer
	DefineVariable pMesh,Pointer,NULL
	DefineVariable dNumMaterials,dword,NULL
	DefineVariable pMaterials,Pointer,NULL
ObjectEnd

;Note: This class doesn't require its own Init method,
;but it DOES require its own Done method.
Method D3DMesh.Done,uses esi
	SetObject esi
	;Free mesh materials buffer (if any)
	.if [esi].pMaterials!=0
		MemFree [esi].pMaterials
	.endif
	;Release loaded mesh (if any)
	.if [esi].pMesh!=0
		ICall [esi].pMesh::IUnknown.Release
	.endif
MethodEnd

Method D3DMesh.Load,uses esi,pfilename
LOCAL pD3DXMtrlBuffer	; ptr to ID3DXBuffer interface
LOCAL pd3dxMaterials	; ptr to the actual data 'behind' that interface
	SetObject esi
	
    ; Load the mesh from the specified .x file
    invoke D3DXLoadMeshFromX,pfilename, D3DXMESH_SYSTEMMEM,
        pD3DDevice, NULL, addr pD3DXMtrlBuffer, NULL,
        addr [esi].dNumMaterials,addr [esi].pMesh
    .if FAILED(eax)
        return E_FAIL
    .endif
    ; Yay - we loaded the mesh from the file - thats good :)
	; Let's explain what's going on.
	; We have to extract the material properties
	; and texture names from the pD3DXMtrlBuffer..
	; pD3DXMtrlBuffer is a pointer to an ID3DXBuffer interface.
	; We can call its GetBufferPointer method to obtain a pointer
	; to the actual Materials data which it 'owns', which is stored
	; as an array of D3DXMATERIAL structs.
    mov pd3dxMaterials,$ICall (pD3DXMtrlBuffer::ID3DXBuffer.GetBufferPointer)
    ; Now we have access to the mesh's Materials data,
    ; let's allocate a little array to copy them into..
    mov eax,sizeof D3DXMATERIAL
    mul [esi].dNumMaterials
    mov [esi].pMaterials ,$MemAlloc(eax,MEM_INIT_ZERO)
	; It's time to copy our Materials data,
	; and so we'll do that now using a Loop..
    xor ecx,ecx
    mov ebx,pd3dxMaterials			;source
    mov edi,[esi].pMaterials	;destination
    .while ecx<[esi].dNumMaterials
    	;Protect these volatile registers
    	;(so RtlMoveMemory doesn't trash them)
    	push ecx
    	push ebx
    	;Copy one Material
		invoke RtlMoveMemory,edi,ebx,sizeof D3DXMATERIAL		
        ; Set the ambient color for the material (D3DX does not do this)
        ; Its easy - just copy Ambient = Diffuse
		m2m [edi].D3DXMATERIAL.MatD3D.Ambient.a,[edi].D3DXMATERIAL.MatD3D.Diffuse.a
		m2m [edi].D3DXMATERIAL.MatD3D.Ambient.r,[edi].D3DXMATERIAL.MatD3D.Diffuse.r
		m2m [edi].D3DXMATERIAL.MatD3D.Ambient.g,[edi].D3DXMATERIAL.MatD3D.Diffuse.g
		m2m [edi].D3DXMATERIAL.MatD3D.Ambient.b,[edi].D3DXMATERIAL.MatD3D.Diffuse.b
 
        ;Load texture (if any is required)
        ;READ THE COMMENTS !!!
        .if [edi].D3DXMATERIAL.pTextureFilename!=0
        	;We'll use our TextureManager to load Textures.
        	push edi
	        OCall [esi].pOwner::TextureManager.LoadFromFile,[edi].D3DXMATERIAL.pTextureFilename
	        pop edi
	        ;Store the returned TexturePointer (or NULL if failed)
	        ;IMPORTANT NOTE WELL that we are going to abuse
	        ;the pTextureFilename field of our D3DXMATERIAL structs
	        ;and use it to store the loaded pTexture.
	        ;We do this because it saves us from having to allocate
	        ;a special array for storing texturepointers for this mesh,
	        ;and because our TextureManager keeps the filenames for us already.
	        mov [edi].D3DXMATERIAL.pTextureFilename,eax	        
	    .endif
	    
	    ;Increment counter and Pointers
	    pop ebx
	    pop ecx
	    mov eax,sizeof D3DXMATERIAL
	    add ebx,eax
	    add edi,eax
	    inc ecx	    
    .endw

    ;We're done with the material buffer
    ICall pD3DXMtrlBuffer::IUnknown.Release
    return S_OK
MethodEnd

