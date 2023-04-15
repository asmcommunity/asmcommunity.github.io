;Struct used to build an intermediate BSP Tree from Model geometry
BSPNode struct
	pFaces	Pointer ?		;Pointer to DataCollection
	pSplitterPlane Pointer ?;Pointer to Vec4
	pFront	Pointer ?		;Pointer to "front" child
	pBack	Pointer ?		;Pointer to "back" child
	vOrigin Vec3 <>         ;BoundingSphere Position
	fRadius real4 ?         ;BoundingSphere Radius
BSPNode ends

;Struct used to describe a Triangle from the input Model
Face struct
	pV0 Pointer ?			;Pointer to 1st Vertex
	pV1 Pointer ?			;Pointer to 2nd Vertex
	pV2 Pointer ?			;Pointer to 3rd Vertex
	pPlane Pointer ?		;Pointer to plane
Face ends

;Struct used to return information from 'ChooseBestDividingPlane'
BestResult struct
 pbestPlane	  dd ?	;Pointer to Best Dividing Plane
 bestPositive dd ?	;#Faces estimated on Front Side
 bestNegative dd ?	;#Faces estimated on Back Side
 bestSpanning dd ?	;#Faces estimated to be Split
 bestCoinciding dd ?
BestResult ends



.data
MINIMUMRELATION  real4 0.8f		;fine-tuning of the dual heuristic in 'ChooseBestDividingPlane'
MINRELATIONSCALE real4 0.8f		;how much better is 'better', anyway? Angry Factor
.code

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

ifndef fEpsilon
    fEpsilon equ fEpsilon_Penetration
endif

;This procedure classifies a Point and a Plane
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



;This function classifies a Face and a Plane...
;It can tell us about the intersection of a Face and Plane,
;in terms of what side of Plane each vertex is located :)
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


; ==================================================================================================
; Title:   D3D_CollisionMesh.inc
; Authors: G. Friedrich, Homer
; Version: 1.0.3
; HEAVILY Based on D3D_Mesh
; This object describes a 3D solid of arbitrary geometry
; which can be loaded from a .X or other file.
; Although essentially this is a reference object
; which describes the basic attributes for N 'instances',
; this object supports a SphereTree (defined in BodySpace)
; which can be used for narrow-phase collision detection,
; and for simultaneous multiple contact point generation.
; Just realize that tree is a 'bodyspace quanta' 
; and you'll understand why it belongs in the Reference object...
; It's not stateful, it describes a constant for all instances!!
; ==================================================================================================

D3DXF_FILEFORMAT_BINARY     equ 0
D3DXF_FILEFORMAT_TEXT       equ 1
D3DXF_FILEFORMAT_COMPRESSED equ 2

MESH_LOAD_DATA struc
  pOwner          Pointer   ?
  pDirect3D       Pointer   ?
  pTextureManager Pointer   ?
MESH_LOAD_DATA ends

;D3D_CollisionMesh is designed to be a reference object.
;Please see the above notes to understand this concept.
Object D3D_CollisionMesh, 4564536, CollisionBody
  RedefineMethod    Done
  RedefineMethod    Init,                   Pointer, dword, Pointer, Pointer
  RedefineMethod    Load,                   Pointer, Pointer        ;-> Stream, -> Parameter(s), dShapeID
  RedefineMethod    Store,                  Pointer                 ;-> Stream

  VirtualMethod     GetVertexComponent,     dword, dword, dword  ;Stream, Component, ComponentIndex
  VirtualMethod     GetVertexNumStreams
  VirtualMethod     GetNumVertexComponent,  dword,dword        ;Stream, Component
  VirtualMethod     LoadFromXFile,          LPSTR, Pointer          ;-> Filename, -> ScaleXYZ Vec3, dShapeID
  VirtualMethod     LoadFromXMLFile,        Pointer
  DynamicMethod     Render
  VirtualMethod     SaveToXFile,             LPSTR                   ;-> Filename
  VirtualMethod     Release
  VirtualMethod     RestoreFromXFile

  Private
      DynamicMethod     RenderByAttr,           dword
      VirtualMethod     PrepareFaces
      ;Methods relating to 'collision detection tree'
      VirtualMethod     GenerateTree
      VirtualMethod     GenerateNode,           Pointer
      VirtualMethod     ProbeBounds,            Pointer
      VirtualMethod     ChooseBestDividingPlane,Pointer,Pointer
      VirtualMethod     DivideFacesWithPlane,   Pointer,Pointer
      VirtualMethod     DivideConvexLeaf,       Pointer
      VirtualMethod     GrabClosestFace,        Pointer,Pointer
      VirtualMethod     DestroyTree,            Pointer
  PrivateEnd

  DefineVariable    pDirect3D,              Pointer,      NULL      ;-> Direct3D
  DefineVariable    pName,                  Pointer,      NULL
  DefineVariable    pTextureManager,        Pointer,      NULL      ;-> D3D_TextureManager
  DefineVariable    pIMesh,                 Pointer,      NULL      ;-> ID3DXMesh
  DefineVariable    pAttributes,            Pointer,      NULL
  DefineVariable    dNumAttributes,         dword,        NULL

  ;World transform matrices
  DefineVariable    mRotation,              D3DXMATRIX,   {1.0,0,0,0, 0,1.0,0,0, 0,0,0,1.0, 0,0,0,1.0}
  DefineVariable    mTranslation,           D3DXMATRIX,   {1.0,0,0,0, 0,1.0,0,0, 0,0,0,1.0, 0,0,0,1.0}

  ;Mesh bounding sphere based on VertexBuffer data. Coordinates are in model space. Radius always +.
  DefineVariable    BoundingSphere,         D3DXVECTOR4,  {0.0, 0.0, 0.0, 0.0}   ;x,y,z, radius

  DefineVariable    pIMaterialBuffer,       Pointer,  NULL

  DefineVariable    pVertexDeclaration,     Pointer,  NULL  ;array
  DefineVariable    pIDecl,                 Pointer,  NULL  ;-> interface
  DefineVariable    dVertexDeclarationCount,dword,    0
  DefineVariable    dVertexDeclarationSize, dword,    0

  ;Objects used for D3D Mesh
  Embed VertexBuffer, D3D_VertexBuffer
  Embed IndexBuffer,  D3D_IndexBuffer
  Embed MaterialColl, DataCollection
  Embed TextureColl,  DwordCollection
  
  ;Pointer to Root Node of Tree
  DefineVariable pRootNode,Pointer,NULL

ObjectEnd

; ==================================================================================================
;    D3D_CollisionMesh implementation
; ==================================================================================================

if IMPLEMENT

externdef IID_ISAXXMLReader:GUID
externdef CLSID_SAXXMLReader40:GUID

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Destructor: D3D_CollisionMesh.Done
; Purpose:   Finalizes the object.
; Arguments: None.
; Return:    Nothing.

TextureRelease proc pTextureData:Pointer, pTextureManager:Pointer
    OCall pTextureManager::D3D_TextureManager.ReleaseTexture, pTextureData
    ret
TextureRelease endp

Method D3D_CollisionMesh.Done, uses esi
    SetObject esi
    SAFE_RELEASE_AND_NULLIFY [esi].pIMesh
    SAFE_RELEASE_AND_NULLIFY [esi].pIMaterialBuffer

    OCall [esi].VertexBuffer::D3D_VertexBuffer.Done
    OCall [esi].IndexBuffer::D3D_IndexBuffer.Done
    OCall [esi].MaterialColl::DataCollection.Done

    .if [esi].pTextureManager != NULL
      OCall [esi].TextureColl::DwordCollection.ForEach, offset TextureRelease, [esi].pTextureManager
    .endif
    OCall [esi].TextureColl::DwordCollection.Done

    .if [esi].pName != NULL
      invoke StrDispose, [esi].pName
      mov [esi].pName, NULL
    .endif

    .if [esi].pAttributes != NULL
      MemFree [esi].pAttributes
      mov [esi].pAttributes, NULL
    .endif

    .if [esi].pVertexDeclaration != NULL
      MemFree [esi].pVertexDeclaration
      mov [esi].pVertexDeclaration, NULL
    .endif

    SAFE_RELEASE_AND_NULLIFY [esi].pIDecl
    
    Destroy [esi].pPlanes
    
    OCall esi.DestroyTree,[esi].pRootNode

    ACall esi.Done
MethodEnd NONE

Method D3D_CollisionMesh.DestroyTree,uses esi ebx,pNode
    SetObject esi
    mov ebx,pNode
    DbgHex pNode
    .if [ebx].BSPNode.pFront!=0
        DbgText "front"
        OCall esi.DestroyTree,[ebx].BSPNode.pFront
    .endif
    .if [ebx].BSPNode.pBack!=0
        DbgText "back"
        OCall esi.DestroyTree,[ebx].BSPNode.pBack
    .endif
    .if [ebx].BSPNode.pFaces!=0
        DbgText "faces"
        Destroy [ebx].BSPNode.pFaces
    .endif
    DbgText "node"
    MemFree pNode
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.GetVertexNumStreams
; Purpose:   Returns a count of the number of Vertex Streams provided by the Mesh at load-time
;            (should usually be 1).
; Arguments: None.
; Return:    Count.

Method D3D_CollisionMesh.GetVertexNumStreams, uses esi
    SetObject esi
    xor ecx, ecx
    xor edx, edx
    mov eax, [esi].pVertexDeclaration
    .while ecx < [esi].dVertexDeclarationCount
      .if [eax].D3DVERTEXELEMENT9.Stream > dx
        mov dx, [eax].D3DVERTEXELEMENT9.Stream
      .endif
      add eax, sizeof D3DVERTEXELEMENT9
      inc ecx
    .endw
    inc edx
    mov eax, edx
MethodEnd eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.GetVertexComponent
; Purpose:   Returns the vertex component.
; Arguments: Arg1: data stream.
;            Arg2: Component ID
;            Arg3: Index.
; Return:    eax -> D3DVERTEXELEMENT9, otherwise NULL.

;  SEE D3D9Types.inc for a list of constants including:
;
;    D3DDECLUSAGE_POSITION = 0,
;    D3DDECLUSAGE_BLENDWEIGHT = 1,
;    D3DDECLUSAGE_BLENDINDICES = 2,
;    D3DDECLUSAGE_NORMAL = 3,
;    D3DDECLUSAGE_PSIZE = 4,
;    D3DDECLUSAGE_TEXCOORD = 5,
;    D3DDECLUSAGE_TANGENT = 6,
;    D3DDECLUSAGE_BINORMAL = 7,
;    D3DDECLUSAGE_TESSFACTOR = 8,
;    D3DDECLUSAGE_POSITIONT = 9,
;    D3DDECLUSAGE_COLOR = 10,
;    D3DDECLUSAGE_FOG = 11,
;    D3DDECLUSAGE_DEPTH = 12,
;    D3DDECLUSAGE_SAMPLE = 13,
;
;EXAMPLE CODE - obtain the number of, datatypes of, and offsets to, each UV Set in the Vertex Format
;Note that this example does not bother to check which STREAM each component lives in, which is based
;on the assumption that only one Stream of data exists.
;Each Stream is associated with a given VB at a given offset in the VB, so one VB COULD use different
;Strides, but each Stream has a FIXED Stride, and thats how this interleaving of components works.
;
;  xor ecx,ecx
;  .while ecx<7
;    push eax
;    OCall GetVertexComponent, NULL, D3DDECLUSAGE_TEXCOORD, ecx
;    .break .if eax == -1
;    movzx edx,[eax].D3DVERTEXELEMENT9._Type
;    mov [esi+ecx*4].Vertex_UV_Types,edx    ;eg D3DDECLTYPE_FLOAT2 = 8 bit U,V,
;                                           ;D3DDECLTYPE_FLOAT16_2 = 16 bit U,V
;    movzx edx,[eax].D3DVERTEXELEMENT9._Offset
;    mov [esi+ecx*4].Vertex_UV_Offsets, edx
;    inc ecx
;  .endw
;  mov [esi].Vertex_Number_of_UVs, ecx
;
;Search the Vertex Declaration Array for a particular vertex component, such as position, normal,
;color, etc dStream is the index of the Stream which we are querying
;dComponentID is a value from the D3DDECLUSAGE constants, such as D3DDECLUSAGE_POSITION.
;dIndex is the index of this component, should more than one be present (eg multiple texture coords)
;Returns null (component does not exist), or a pointer to a D3DVERTEXELEMENT9 structure which contains
;detailed information regarding the specific.

Method D3D_CollisionMesh.GetVertexComponent, uses esi ebx, dStream:dword, dComponentID:dword, dIndex:dword
    SetObject esi
    xor ecx, ecx
    mov eax, [esi].pVertexDeclaration
    mov ebx, dComponentID
    mov edx, dIndex
    .while ecx < [esi].dVertexDeclarationCount
      .if [eax].D3DVERTEXELEMENT9.Usage == bl && [eax].D3DVERTEXELEMENT9.UsageIndex == dl
        push edx
        mov edx, dStream
        .if [eax].D3DVERTEXELEMENT9.Stream == dx
          add esp, 4
          ExitMethod                                    ;Found eax -> D3DVERTEXELEMENT9
        .endif
        pop edx
      .endif
      add eax, sizeof D3DVERTEXELEMENT9
      inc ecx
    .endw
    xor eax, eax
MethodEnd eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.GetNumVertexComponent
; Purpose:   Returns a count of the number of components of given type that are associated with a
;            given stream. For example, "how many sets of UV data in the vertices of stream 0".
; Arguments: Arg1: data stream.
;            Arg2: Component ID
; Return:    Count.

Method D3D_CollisionMesh.GetNumVertexComponent, uses esi ebx, dStream, dComponentID
    SetObject esi
    mov eax, [esi].pVertexDeclaration
    mov ebx, dComponentID
    xor ecx, ecx
    xor edx, edx
    .while ecx < [esi].dVertexDeclarationCount
      .if [eax].D3DVERTEXELEMENT9.Usage == bl && [eax].D3DVERTEXELEMENT9.UsageIndex >= dl
        push edx
        mov edx, dStream
        .if [eax].D3DVERTEXELEMENT9.Stream == dx
          pop edx
          inc edx
        .else
          pop edx
        .endif
      .endif
      add eax, sizeof D3DVERTEXELEMENT9
      inc ecx
    .endw
    mov eax, edx
MethodEnd eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Constructor: D3D_CollisionMesh.Init
; Purpose:     Initializes the object.
; Arguments:   Arg1: -> Owner object.
;              Arg2: -> Direct3D object.
;              Arg3: -> Texture manager object.
; Return:      Nothing.

Method D3D_CollisionMesh.Init, uses esi, pOwner:Pointer, dShapeID:dword, pDirect3D:Pointer, pTextureManager:Pointer
    SetObject esi
    m2m [esi].pDirect3D, pDirect3D, eax
    m2m [esi].pTextureManager, pTextureManager, eax
    ACall Init,pOwner,dShapeID
    OCall [esi].MaterialColl::DataCollection.Init, esi, 10, 10, COLLECTIONMAXSIZE
    OCall [esi].TextureColl::DwordCollection.Init, esi, 10, 10, COLLECTIONMAXSIZE
MethodEnd NONE

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Constructor: D3D_CollisionMesh.Load
; Purpose:     Loads the object from a stream.
; Arguments:   Arg1: -> Stream object.
;              Arg2: -> BUFFER_LOAD_DATA.
; Return:      Nothing.

Method D3D_CollisionMesh.Load, uses ebx esi, pStream:Pointer, pLoadData:Pointer
    local dMaxSize:dword, dDelta:dword, dLimit:dword, dCount:dword      ;Reverse order!
    local BufferLoadData:BUFFER_LOAD_DATA
    local dVertexCount:dword, dFaceCount:dword, pAttrBuffer:Pointer

    SetObject esi
    mov eax, pLoadData
    m2m [esi].pDirect3D, [eax].MESH_LOAD_DATA.pDirect3D, ecx
    m2m [esi].pTextureManager, [eax].MESH_LOAD_DATA.pTextureManager, ecx
    ACall esi.Init, [eax].MESH_LOAD_DATA.pOwner,[esi].dShapeID
    OCall pStream::Stream.BinRead, addr dFaceCount, 4
    OCall pStream::Stream.BinRead, addr dVertexCount, 4
    MemFree [esi].pVertexDeclaration, MEM_SAFE_FREE
    OCall pStream::Stream.BinRead, addr [esi].dVertexDeclarationSize, 4
    mov [esi].pVertexDeclaration, $MemAlloc(MAX_FVF_DECL_SIZE)
    OCall pStream::Stream.BinRead, eax, [esi].dVertexDeclarationSize

    ;Restore bounding sphere information
    OCall pStream::Stream.BinRead, addr [esi].BoundingSphere, sizeof D3DXVECTOR4

    mov ecx, pLoadData
    lea ebx, BufferLoadData
    mov [ebx].BUFFER_LOAD_DATA.pOwner, esi
    m2m [ebx].BUFFER_LOAD_DATA.pDirect3D, [ecx].MESH_LOAD_DATA.pDirect3D, eax
    mov edx, [ecx].MESH_LOAD_DATA.pDirect3D
    invoke D3DXCreateMesh, dFaceCount, dVertexCount, D3DXMESH_WRITEONLY, \
                              [esi].pVertexDeclaration, [edx].Direct3D.pID3DDevice, addr [esi].pIMesh

    ICall [esi].pIMesh::ID3DXMesh.LockAttributeBuffer, D3DLOCK_READONLY, addr pAttrBuffer
    mov eax, dFaceCount
    shl eax, 2
    OCall pStream::Stream.BinRead, pAttrBuffer, eax
    ICall [esi].pIMesh::ID3DXMesh.UnlockAttributeBuffer
    ICall [esi].pIMesh::ID3DXMesh.GetVertexBuffer, addr [ebx].BUFFER_LOAD_DATA.pIBuffer
    OCall [esi].VertexBuffer::D3D_VertexBuffer.Load, pStream, ebx
    ICall [esi].pIMesh::ID3DXMesh.GetIndexBuffer, addr [ebx].BUFFER_LOAD_DATA.pIBuffer
    OCall [esi].IndexBuffer::D3D_IndexBuffer.Load, pStream, ebx
    OCall [esi].MaterialColl::DataCollection.Load, pStream, esi

    OCall pStream::Stream.BinRead, addr dCount, 4 * 4     ;Count, ..., MaxSize
    OCall [esi].TextureColl::DwordCollection.Init, esi, dLimit, dDelta, dMaxSize
    .while dCount > 0
      OCall pStream::Stream.StrRead
      push eax
      .if [esi].pTextureManager != NULL
        OCall [esi].pTextureManager::D3D_TextureManager.GetTexture, eax
      .else
        xor eax, eax
      .endif
      OCall [esi].TextureColl::DwordCollection.Insert, eax
      call StrDispose
      dec dCount
    .endw

    ;Set buffer markers
    mov [esi].VertexBuffer.dFirstPos, 0
    m2m [esi].VertexBuffer.dLastPos, [esi].VertexBuffer.dCapacity, eax
    mov [esi].IndexBuffer.dFirstPos, 0
    m2m [esi].IndexBuffer.dLastPos, [esi].IndexBuffer.dCapacity, eax

    ACall esi.Load, pStream, [esi].pOwner
MethodEnd NONE

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.LoadFromXFile
; Purpose:   Loads a mesh from a .x file.
; Arguments: Arg1: -> FileName.
;            Arg2: -> scale D3DXVECTOR3.
; Return:    Zero if succeeded, otherwise error code.

Method D3D_CollisionMesh.LoadFromXFile, uses ebx esi, pFileName:Pointer, pvScale:Pointer
    local pIAdjacency:Pointer, pMaterials:Pointer, dMaterialCount:dword, pMaterial:Pointer
    local pIBuffer:Pointer, pBuffer:Pointer
    LOCAL nStreams, iStream
    local bName[1024]:byte, pNamePos:Pointer


    SetObject esi

    ;Create a full file name string
    mov pNamePos, NULL
    lea ebx, bName
    invoke GetFullPathName, pFileName, sizeof bName - 1, ebx, addr pNamePos

    .if pNamePos == NULL
      mov pNamePos, ebx                                 ;Save for later use
    .endif

    .if [esi].pName != NULL
      invoke StrDispose, [esi].pName
    .endif
    mov [esi].pName, $invoke(StrNew, ebx)
    

    ;Release all previous resources
    SAFE_RELEASE_AND_NULLIFY [esi].pIMesh
    OCall [esi].VertexBuffer::D3D_VertexBuffer.Done
    OCall [esi].IndexBuffer::D3D_IndexBuffer.Done
    OCall [esi].TextureColl::DwordCollection.ForEach, offset TextureRelease, [esi].pTextureManager
    OCall [esi].TextureColl::DwordCollection.DeleteAll
    OCall [esi].MaterialColl::DataCollection.DisposeAll

    DbgWarning "Attempt to load STATIC MESH"
    DbgHex esi,"ptr to D3D_CollisionMesh"
    DbgStr [esi].pName
    mov ecx, [esi].pDirect3D
    invoke D3DXLoadMeshFromX, [esi].pName, 0, [ecx].Direct3D.pID3DDevice, NULL, \
              addr [esi].pIMaterialBuffer, addr pIAdjacency, addr dMaterialCount, addr [esi].pIMesh
    .if SUCCEEDED(eax)
      ICall [esi].pIMesh::ID3DXMesh.OptimizeInplace, D3DXMESHOPT_ATTRSORT, pIAdjacency, NULL, NULL, NULL      

      mov [esi].pVertexDeclaration, $MemAlloc(MAX_FVF_DECL_SIZE)
      ICall [esi].pIMesh::ID3DXMesh.GetDeclaration, eax
      mov [esi].dVertexDeclarationCount, $invoke (D3DXGetDeclLength, [esi].pVertexDeclaration)
      mov ebx, sizeof D3DVERTEXELEMENT9
      mul ebx
      mov [esi].dVertexDeclarationSize, eax
      mov ebx, [esi].pDirect3D
      ICall [ebx].Direct3D.pID3DDevice::IDirect3DDevice9.CreateVertexDeclaration, \
              [esi].pVertexDeclaration, addr [esi].pIDecl
              
      
      DbgLine
      DbgWarning "Enumerating the components of the Vertex Declaration"
      
      mov nStreams,$OCall (GetVertexNumStreams)
      xor ecx,ecx
      .while ecx<nStreams
          push ecx
          mov iStream,ecx
          DbgLine
          DbgDec iStream, "Vertex Stream#"
                    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_POSITION
          .if eax != 0
            DbgDec eax,"#Position elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_POSITION, ebx
                .if eax!=0
                    movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                    DbgDec eax, "Offset of Position in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no Positions)"
          .endif
          
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_BLENDWEIGHT
          .if eax != 0
            DbgDec eax,"#BlendWeight elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_BLENDWEIGHT, ebx
                .if eax!=0           
                    movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                    DbgDec eax, "Offset of BlendWeight in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no BlendWeights)"
          .endif
          
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_BLENDINDICES
          .if eax != 0
            DbgDec eax,"#BlendIndex elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_BLENDINDICES, ebx
                .if eax!=0
                    movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                    DbgDec eax, "Offset of BlendIndex in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no BlendIndices)"
          .endif
    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_NORMAL
          .if eax != 0
            DbgDec eax,"#Normal elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_NORMAL, ebx
                .if eax!=0
                    movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                    DbgDec eax, "Offset of Normal in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no Normals)"
          .endif
    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_PSIZE
          .if eax != 0
            DbgDec eax,"#PSize elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_PSIZE, ebx
                .if eax!=0            
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of PSize in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no PSizes)"
          .endif
    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_TEXCOORD
          .if eax != 0
            DbgDec eax,"#TexCoord elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_TEXCOORD, ebx
                .if eax!=0 
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of TexCoord in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no TexCoords)"
          .endif
    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_TANGENT
          .if eax != 0
            DbgDec eax,"#Tangent elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_TANGENT, ebx
                .if eax!=0 
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of Tangent in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no Tangents)"
          .endif
          
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_BINORMAL
          .if eax != 0
            DbgDec eax,"#BiNormal elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_BINORMAL, ebx
                .if eax!=0 
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of BiNormal in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no BiNormals)"
          .endif
    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_TESSFACTOR
          .if eax != 0
            DbgDec eax,"#TessFactor elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_TESSFACTOR, ebx
                .if eax!=0 
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of TessFactor in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no TessFactors)"
          .endif
    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_POSITIONT
          .if eax != 0
            DbgDec eax,"#PositionT elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_POSITIONT, ebx
                .if eax!=0 
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of PositionT in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no PositionTs)"
          .endif
          
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_COLOR
          .if eax != 0
            DbgDec eax,"#Color elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_COLOR, ebx
                .if eax!=0
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of Color in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no Colors)"
          .endif      
    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_FOG
          .if eax != 0
            DbgDec eax,"#Fog elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_FOG, ebx
                .if eax!=0
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of Fog in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no Fogs)"
          .endif
    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_DEPTH
          .if eax != 0
            DbgDec eax,"#Depth elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_DEPTH, ebx
                .if eax!=0
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of Depth in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no Depths)"
          .endif
    
          OCall GetNumVertexComponent, iStream, D3DDECLUSAGE_SAMPLE
          .if eax != 0
            DbgDec eax,"#Sample elements in Vertex"
            xor ebx,ebx
            .while ebx<eax
                push eax
                OCall GetVertexComponent, iStream, D3DDECLUSAGE_SAMPLE, ebx
                .if eax!=0
                  movzx eax, [eax].D3DVERTEXELEMENT9._Offset
                  DbgDec eax, "Offset of Sample in Vertex"
                .else
                    DbgWarning "(Element not found)"
                .endif
                pop eax
                inc ebx
            .endw
          .else
            DbgText "(Vertex contains no Samples)"
          .endif  
          
          pop ecx
          inc ecx
      .endw

      DbgLine
      DbgWarning "Enumerating the Materials"
      DbgDec dMaterialCount,"Total #Materials in mesh"
      ;Fill the material DataCollection
      mov pMaterials, $ICall([esi].pIMaterialBuffer::ID3DXBuffer.GetBufferPointer)
      xor ebx, ebx
      .while ebx < dMaterialCount
        ;Allocate a new Material
        .if ($MemAlloc (sizeof D3DXMATERIAL)) != NULL
          mov pMaterial, eax
          mov eax, pMaterials
          
          DbgDec ebx,"Material#"
          
          ;Set Ambient = Diffuse
          m2m [eax].D3DMATERIAL9.Ambient.a,[eax].D3DMATERIAL9.Diffuse.a,edx
          DbgFloat edx,"Alpha"
          m2m [eax].D3DMATERIAL9.Ambient.r,[eax].D3DMATERIAL9.Diffuse.r,edx
          DbgFloat edx,"Red"
          m2m [eax].D3DMATERIAL9.Ambient.g,[eax].D3DMATERIAL9.Diffuse.g,edx
          DbgFloat edx,"Green"
          m2m [eax].D3DMATERIAL9.Ambient.b,[eax].D3DMATERIAL9.Diffuse.b,edx
          DbgFloat edx,"Blue"
          

          mov eax, [eax].D3DXMATERIAL.pTextureFilename
          .if eax != NULL
            push eax
            .if word ptr [eax + 1] == "\:"
              lea edx, bName
              push edx
              ;its a qualified path - Overwrite the filepath
            .else
              push pNamePos
              ;its a relative path - assume relative to the x-file
            .endif
            call StrCopy
            DbgWarning "STATIC Mesh loading a Texture"
            DbgStr bName
            OCall [esi].pTextureManager::D3D_TextureManager.GetTexture, addr bName
            .if eax == 0
              DbgWarning "Failed to obtain texturedata"
              int 3
            .endif
            OCall [esi].TextureColl::DwordCollection.Insert, eax
          .endif
          push eax
          invoke MemClone, pMaterial, pMaterials, sizeof D3DMATERIAL9
          pop eax
          ;Mark the Material with the Texture it is using (or NULL)
          mov edx, pMaterial
          mov [edx].D3DXMATERIAL.pTextureFilename, eax

          ;Set the ambient color for the material (D3DX does not do this)
          mov eax, pMaterial
          ;fmov [eax].D3DMATERIAL9.Diffuse.a, 1.0
          m2m [eax].D3DMATERIAL9.Ambient.r, [eax].D3DMATERIAL9.Diffuse.r, ecx
          m2m [eax].D3DMATERIAL9.Ambient.g, [eax].D3DMATERIAL9.Diffuse.g, ecx
          m2m [eax].D3DMATERIAL9.Ambient.b, [eax].D3DMATERIAL9.Diffuse.b, ecx
          m2m [eax].D3DMATERIAL9.Ambient.a, [eax].D3DMATERIAL9.Diffuse.a, ecx
          ;Add the new Material to the Materials collection
          OCall [esi].MaterialColl::DataCollection.Insert, eax

          DbgWarning "Texture and Material prepared"
          add pMaterials, sizeof D3DXMATERIAL
          inc ebx
        .else
          OCall esi.ErrorReport, NULL, D3D_OUT_OF_MEMORY
          .break
        .endif
      .endw

      ;Update Vertex Buffer
      ICall [esi].pIMesh::ID3DXMesh.GetVertexBuffer, addr pIBuffer

      ;Initialize the vertex buffer, handing it the vertex size
      OCall [esi].VertexBuffer::D3D_VertexBuffer.Init, esi, [esi].pDirect3D, pIBuffer, $ICall ([esi].pIMesh::ID3DXBaseMesh.GetNumBytesPerVertex)
      .if eax==0
          ICall pIBuffer::IDirect3DVertexBuffer9._Lock, 0, 0, addr pBuffer, D3DLOCK_READONLY     
          .if eax==0
              ;Apply Scale
              mov edx, pvScale
              .if edx != NULL
                DbgWarning "(Scaling Vertices)"
                xor ecx, ecx
                mov ebx, pBuffer
                .while ecx<[esi].VertexBuffer.dCapacity
                  Vec3_Mul  [ebx].Vec3,[edx].Vec3
                  Vec3_Stow [ebx].Vec3
                  DbgVec3 ebx
                  add ebx, [esi].VertexBuffer.dVertexSize
                  inc ecx
                .endw
              .else
                  DbgWarning "(Not Scaling Mesh)"
              .endif  
              xor edx, edx
              mov eax, [esi].VertexBuffer.dVertexSize
              mov ecx, [esi].VertexBuffer.dCapacity
              mul ecx
              invoke MemClone, [esi].VertexBuffer.pBuffer, pBuffer, eax
              ICall pIBuffer::IDirect3DVertexBuffer9.Unlock
          .else
              DbgWarning "Failed to Lock VB"
              int 3
          .endif
      .else
          DbgWarning "Failed to initialize VB"
          DbgDec eax,"grr"
          int 3
      .endif
      
      ;Update Index Buffer
      DbgWarning "Cloning Indices"
      ICall [esi].pIMesh::ID3DXMesh.GetIndexBuffer, addr pIBuffer
      OCall [esi].IndexBuffer::D3D_IndexBuffer.Init, esi, [esi].pDirect3D, pIBuffer
      ICall pIBuffer::IDirect3DIndexBuffer9._Lock, 0, 0, addr pBuffer, D3DLOCK_READONLY
      xor edx, edx
      mov eax, [esi].IndexBuffer.dIndexSize
      mov ecx, [esi].IndexBuffer.dCapacity
      mul ecx
      invoke MemClone, [esi].IndexBuffer.pBuffer, pBuffer, eax
      ICall pIBuffer::IDirect3DIndexBuffer9.Unlock

      ;Compute the bounding sphere
      DbgDec [esi].VertexBuffer.dCapacity
      invoke D3DXComputeBoundingSphere, [esi].VertexBuffer.pBuffer, [esi].VertexBuffer.dCapacity, \
             [esi].VertexBuffer.dVertexSize, addr [esi].BoundingSphere, addr [esi].BoundingSphere.w
      BitClr [esi].BoundingSphere.w, BIT31        ;Make sure we have always a positive radius
      
      ;Copy the boundingsphere radius
      m2m [esi].fRadius,[esi].BoundingSphere.w,edx

      ;Get #AttributeGroups
      ICall [esi].pIMesh::ID3DXMesh.GetAttributeTable,NULL,addr [esi].dNumAttributes
      ;Allocate memory for the AttributeGroups table
      mov eax,sizeof D3DXATTRIBUTERANGE
      mul [esi].dNumAttributes
      mov [esi].pAttributes, $MemAlloc(eax)
      .if eax==NULL
        DbgWarning "Failed to allocate AttributeTable"
      .else
        ;Fill the AttributeGroup Table
        lea eax, [esi].dNumAttributes
        ICall [esi].pIMesh::ID3DXMesh.GetAttributeTable, [esi].pAttributes, eax
      .endif
      DbgWarning "Successfully loaded Mesh"
      
      OCall esi.GenerateTree
      
      xor eax, eax
    .elseif eax==D3DERR_INVALIDCALL
      DbgWarning "Error - INVALID CALL"
    .elseif eax==E_OUTOFMEMORY
      DbgWarning "Error - OUT OF MEMORY"
    .else
      DbgComError eax, "in D3D_CollisionMesh.LoadFromXFile"
      OCall esi.ErrorReport, NULL, D3D_FILE_LOAD_FAILED
    .endif
MethodEnd eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.LoadFromXMLFile
; Purpose:   Loads a mesh from a .3dxml file.
; Arguments: Arg1: -> Unicode FileName.
; Return:    Zero if succeeded, otherwise error code.

AddCount proc pDwordCollection:Pointer, pSum:Pointer
    mov eax, pDwordCollection
    mov ecx, [eax].DwordCollection.dCount
    mov eax, pSum
    add [eax], ecx
    ret
AddCount endp

Method D3D_CollisionMesh.LoadFromXMLFile, uses ebx edi esi, pFileName:Pointer
    local ContentHandler:XML3DContentHandler
    local PositionStorageColl:Collection, NormalStorageColl:Collection, IndexStorageColl:Collection
    local AttrColl:DwordCollection
    local pXMLReader:Pointer, dSum:dword, dFaceCount:dword, dVertexCount:dword
    local pAttrData:Pointer, pBuffer:Pointer, dCount:dword, wIndexOffset:word, dCurrAttr:dword

    SetObject esi
    invoke CoCreateInstance, offset CLSID_SAXXMLReader40, NULL, \   ;Create an instance of a
              CLSCTX_INPROC_SERVER, offset IID_ISAXXMLReader, \     ;SAXXMLReader40 object
              addr pXMLReader
    .if SUCCEEDED(eax)                                              ;If creation was successful
      SAFE_RELEASE [esi].pIMesh

      ;Initialize the recieving containers
      LNew PositionStorageColl, Collection, Init, esi, 10, 10, COLLECTIONMAXSIZE
      LNew NormalStorageColl, Collection, Init, esi, 10, 10, COLLECTIONMAXSIZE
      LNew IndexStorageColl, Collection, Init, esi, 10, 10, COLLECTIONMAXSIZE
      LNew AttrColl, DwordCollection, Init, esi, 100, 100, COLLECTIONMAXSIZE
      OCall [esi].MaterialColl::DataCollection.DisposeAll
      LNew ContentHandler, XML3DContentHandler, Init, esi, [esi].pDirect3D, \
                                  addr PositionStorageColl, addr NormalStorageColl, \
                                  addr IndexStorageColl, addr [esi].MaterialColl

      mov AttrColl.DwordCollection.dDuplicates, TRUE
      ;Invoke the XML reader
      ICall pXMLReader::ISAXXMLReader.putContentHandler, addr ContentHandler
      ICall pXMLReader::ISAXXMLReader.parseURL, pFileName
      push eax
      ICall pXMLReader::ISAXXMLReader.Release
      pop eax
      .if SUCCEEDED(eax)
        ;Process the recieved data
        mov dSum, 0
        OCall IndexStorageColl::Collection.ForEach, addr AddCount, addr dSum
        mov eax, dSum
        xor edx, edx
        mov ecx, 3
        div ecx
        mov dFaceCount, eax

        mov dSum, 0
        OCall PositionStorageColl::Collection.ForEach, addr AddCount, addr dSum
        mov eax, dSum
        xor edx, edx
        mov ecx, 3
        div ecx
        mov dVertexCount, eax

        mov ecx, [esi].pDirect3D
        invoke D3DXCreateMesh, dFaceCount, dVertexCount, \
                                  D3DXMESH_WRITEONLY, [esi].pVertexDeclaration, \
                                  [ecx].Direct3D.pID3DDevice, addr [esi].pIMesh
        .if SUCCEEDED(eax)
          ICall [esi].pIMesh::ID3DXMesh.GetVertexBuffer, addr pBuffer
          ICall [esi].pIMesh::ID3DXMesh.GetNumBytesPerVertex
          OCall [esi].VertexBuffer::D3D_VertexBuffer.Init, esi, [esi].pDirect3D, pBuffer, eax

          ;Load the VertexBuffer with the obtained data
          mov edi, [esi].VertexBuffer.pBuffer
          xor ebx, ebx
          push esi
          .while ebx < PositionStorageColl.Collection.dCount
            push ebx
            OCall PositionStorageColl::Collection.ItemAt, ebx
            mov esi, [eax].DwordCollection.pItems
            mov eax, [eax].DwordCollection.dCount
            xor edx, edx
            mov ecx, 3
            div ecx
            mov dCount, eax
            OCall NormalStorageColl::Collection.ItemAt, ebx
            mov edx, [eax].DwordCollection.pItems

            ;Shuffle position and normal vectors
            xor ebx, ebx
            .while ebx < dCount
              mov ecx, 3
              rep movsd
              xchg edx, esi
              mov ecx, 3
              rep movsd
              xchg edx, esi
              inc ebx
            .endw
            pop ebx
            inc ebx
          .endw
          pop esi
          mov [esi].VertexBuffer.dFirstPos, 0
          m2m [esi].VertexBuffer.dLastPos, [esi].VertexBuffer.dCapacity, eax

          ICall [esi].pIMesh::ID3DXMesh.GetIndexBuffer, addr pBuffer
          OCall [esi].IndexBuffer::D3D_IndexBuffer.Init, esi, [esi].pDirect3D, pBuffer

          ;Load the IndexBuffer with the obtained data
          mov edi, [esi].IndexBuffer.pBuffer
          xor ebx, ebx
          mov wIndexOffset, bx
          push esi
          .while ebx < IndexStorageColl.Collection.dCount
            mov dCurrAttr, ebx
            push ebx
            OCall IndexStorageColl::Collection.ItemAt, ebx
            m2m dCount, [eax].DwordCollection.dCount, ecx
            mov esi, [eax].DwordCollection.pItems

            ;D3DFMT_INDEX16 => Scale down from dword to word
            xor ebx, ebx
            xor eax, eax
            .while ebx < dCount
              mov edx, dword ptr [esi]
              add dx, wIndexOffset
              mov [edi], dx
              add esi, 4
              add edi, 2
              inc ebx
              .if eax == 2
                OCall AttrColl::DwordCollection.Insert, dCurrAttr
                xor eax, eax
              .else
                inc eax
              .endif
            .endw
            pop ebx

            OCall PositionStorageColl::Collection.ItemAt, ebx
            mov eax, [eax].DwordCollection.dCount
            xor edx, edx
            mov ecx, 3
            div ecx
            add wIndexOffset, ax

            inc ebx
          .endw
          pop esi
          mov [esi].IndexBuffer.dFirstPos, 0
          m2m [esi].IndexBuffer.dLastPos, [esi].IndexBuffer.dCapacity, eax

          ;Update Attribute buffer
          ICall [esi].pIMesh::ID3DXMesh.LockAttributeBuffer, D3DLOCK_DISCARD, addr pAttrData
          mov eax, AttrColl.DwordCollection.dCount
          shl eax, 2
          invoke MemClone, pAttrData, AttrColl.DwordCollection.pItems, eax
          ICall [esi].pIMesh::ID3DXMesh.UnlockAttributeBuffer

          ;Load Texture Collection with NULLs
          xor ebx, ebx
          .while ebx < [esi].MaterialColl.dCount
            OCall [esi].TextureColl::DwordCollection.Insert, NULL
            inc ebx
          .endw

          ;Compute the bounding sphere
          invoke D3DXComputeBoundingSphere, [esi].VertexBuffer.pBuffer, \
                                            [esi].VertexBuffer.dCapacity, \
                                            [esi].VertexBuffer.dVertexSize, \
                                            addr [esi].BoundingSphere, \
                                            addr [esi].BoundingSphere.w
          BitClr [esi].BoundingSphere.w, BIT31        ;Make sure we have always a positive radius

        .else
          OCall esi.ErrorReport, NULL, D3D_MESH_CREATE_FAILED
        .endif
      .else
        OCall esi.ErrorReport, NULL, D3D_INVALID_3DXML_FILE
      .endif
      ;Housekeeping
      OCall PositionStorageColl::Collection.Done
      OCall NormalStorageColl::Collection.Done
      OCall IndexStorageColl::Collection.Done
      xor eax, eax

    .else
      OCall esi.ErrorReport, NULL, D3D_FILE_LOAD_FAILED
    .endif
MethodEnd eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.Render
; Purpose:   Renders the mesh.
; Arguments: Nothing.
; Return:    Zero if succeeded, otherwise error code.

Method D3D_CollisionMesh.Render, uses ebx esi
    SetObject esi
    .if [esi].pIMesh != NULL && [esi].dErrorCode == OBJ_OK
      OCall [esi].VertexBuffer::D3D_VertexBuffer.Update
      OCall [esi].IndexBuffer::D3D_IndexBuffer.Update
      mov ebx, [esi].pDirect3D
      ICall [ebx].Direct3D.pID3DDevice::IDirect3DDevice9.SetVertexDeclaration, [esi].pIDecl
      xor ebx, ebx
      .while ebx < [esi].MaterialColl.dCount
        OCall esi.RenderByAttr, ebx
        inc ebx
      .endw
      mov ebx, [esi].pDirect3D
      ICall [ebx].Direct3D.pID3DDevice::IDirect3DDevice9.SetVertexDeclaration, NULL
    .endif
    xor eax, eax                ;Return Zero
MethodEnd eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.RenderByAttr
; Purpose:   (Private) Renders a mesh surface given an attribute ID.
; Arguments: Arg1: Attribute ID.
; Return:    Nothing.

Method D3D_CollisionMesh.RenderByAttr, uses ebx edi esi, dAttrID:dword
    SetObject esi
    OCall [esi].MaterialColl::DataCollection.ItemAt, dAttrID
    .if eax != NULL
      ;Set the Material
      mov ebx, [esi].pDirect3D
      mov edi, [eax].D3DXMATERIAL.pTextureFilename      ;This is a -> TEXTURE_DATA
      ICall [ebx].Direct3D.pID3DDevice::IDirect3DDevice9.SetMaterial, eax

      ;If theres a Texture, set that too
      .if edi != NULL
        sub esp, 4                                      ;Make room for the texture pointer
        ICall [ebx].Direct3D.pID3DDevice::IDirect3DDevice9.GetTexture, 0, esp  ;Save old texture
        ICall [ebx].Direct3D.pID3DDevice::IDirect3DDevice9.SetTexture, 0, [edi].TEXTURE_DATA.pInterface
        push $OCall(ebx::Direct3D.SetTSS, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1)
        push $OCall(ebx::Direct3D.SetTSS, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE)
        OCall ebx::Direct3D.Realize
        ICall [esi].pIMesh::ID3DXMesh.DrawSubset, dAttrID
        OCall ebx::Direct3D.SetTSS, 0, D3DTSS_COLORARG1, /dword
        OCall ebx::Direct3D.SetTSS, 0, D3DTSS_COLOROP, /dword
        ICall [ebx].Direct3D.pID3DDevice::IDirect3DDevice9.SetTexture, 0, /dword
      .else
        ICall [esi].pIMesh::ID3DXMesh.DrawSubset, dAttrID
      .endif
    .else
        DbgWarning "Material Index out of range in D3D_CollisionMesh.RenderByAttr"
    .endif
MethodEnd NONE

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.SaveToXFile
; Purpose:   Saves a mesh to a .x file.
; Arguments: Arg1: -> FileName.
; Return:    Zero if succeeded, otherwise error code.

Method D3D_CollisionMesh.SaveToXFile, uses esi, pFileName
    SetObject esi
    ICall [esi].pIMaterialBuffer::ID3DXBuffer.GetBufferPointer
    invoke D3DXSaveMeshToX,pFileName, [esi].pIMesh, NULL, eax, NULL, [esi].MaterialColl.dCount, \
                                      D3DXF_FILEFORMAT_COMPRESSED
    .if eax != S_OK
      DbgWarning "Error saving XFile"
      DbgDec eax, "Error Code"
    .endif
MethodEnd eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.Store
; Purpose:   Saves the object in a stream.
; Arguments: Arg1: -> Stream
; Return:    Nothing.

StoreTexture proc pTextureData:Pointer, pStream:Pointer
    mov eax, pTextureData
    OCall pStream::Stream.StrWrite, [eax].TEXTURE_DATA.pSourcePath
    ret
StoreTexture endp

Method D3D_CollisionMesh.Store, uses esi, pStream:Pointer
    local dFaceCount:dword, pAttrBuffer:Pointer

    SetObject esi
    ;Store the required parameters to create the mesh
    xor edx, edx
    mov eax, [esi].IndexBuffer.dCapacity
    mov ecx, 3
    div ecx
    mov dFaceCount, eax
    OCall pStream::Stream.BinWrite, addr dFaceCount, 4
    OCall pStream::Stream.BinWrite, addr [esi].VertexBuffer.dCapacity, 4
    OCall pStream::Stream.BinWrite, addr [esi].dVertexDeclarationSize, 4
    OCall pStream::Stream.BinWrite, [esi].pVertexDeclaration, [esi].dVertexDeclarationSize

    ;Store bounding sphere information
    OCall pStream::Stream.BinWrite, addr [esi].BoundingSphere, sizeof D3DXVECTOR4

    ;Store mesh buffers and related information
    ICall [esi].pIMesh::ID3DXMesh.LockAttributeBuffer, D3DLOCK_READONLY, addr pAttrBuffer
    mov eax, dFaceCount
    shl eax, 2
    OCall pStream::Stream.BinWrite, pAttrBuffer, eax
    ICall [esi].pIMesh::ID3DXMesh.UnlockAttributeBuffer
    OCall [esi].VertexBuffer::D3D_VertexBuffer.Store, pStream
    OCall [esi].IndexBuffer::D3D_IndexBuffer.Store, pStream
    OCall [esi].MaterialColl::DataCollection.Store, pStream
    OCall pStream::Stream.BinWrite, addr [esi].TextureColl.dCount, 4 * 4    ;Count, ..., MaxSize
    OCall [esi].TextureColl::DwordCollection.ForEach, offset StoreTexture, pStream
    ACall esi.Store, pStream
MethodEnd NONE

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.Release
; Purpose:   Releases all resources.
; Arguments: None.
; Return:    Zero if succeeded, otherwise error code.

Method D3D_CollisionMesh.Release, NOFRAME
    push esi
    SetObject esi,, [esp + 8]
    OCall [esi].IndexBuffer::D3D_IndexBuffer.Release
    OCall [esi].VertexBuffer::D3D_VertexBuffer.Release
    SAFE_RELEASE_AND_NULLIFY [esi].pIMesh
    pop esi
    xor eax, eax
MethodEnd eax

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Method:    D3D_CollisionMesh.RestoreFromXFile
; Purpose:   Restores the mesh and all associated resources.
; Arguments: None.
; Return:    Zero if succeeded, otherwise error code.

Method D3D_CollisionMesh.RestoreFromXFile, uses ebx esi
    local dMaterialCount:dword, pIBuffer:Pointer, pVBuffer:Pointer
    local pMaterials:Pointer, pIMaterialBuffer:Pointer
    local bName[1024]:byte

    SetObject esi

    ;Get a new IMesh interface
    mov ecx, [esi].pDirect3D
    invoke D3DXLoadMeshFromX, [esi].pName, 0, [ecx].Direct3D.pID3DDevice, NULL, \
                              addr pIMaterialBuffer, 0, addr dMaterialCount, addr [esi].pIMesh
    .if SUCCEEDED(eax)
      ;Restore VB and IB from user buffers
      ICall [esi].pIMesh::ID3DXMesh.GetVertexBuffer, addr pVBuffer
      OCall [esi].VertexBuffer::D3D_VertexBuffer.Restore, pVBuffer

      ICall [esi].pIMesh::ID3DXMesh.GetIndexBuffer, addr pIBuffer
      OCall [esi].IndexBuffer::D3D_IndexBuffer.Restore, pIBuffer

      ;Restore material textures
      .if [esi].pTextureManager != NULL

        mov pMaterials, $ICall(pIMaterialBuffer::ID3DXBuffer.GetBufferPointer)
        xor ebx, ebx
        .while ebx < dMaterialCount
          mov eax, pMaterials

          mov eax, [eax].D3DXMATERIAL.pTextureFilename
          .if eax != NULL
            push eax
            .if word ptr [eax + 1] != "\:"
              ;its a relative path - assume relative to the x-file
              invoke StrRScan, [esi].pName, "\"
              .if eax != NULL
                sub eax, [esi].pName
                inc eax
                invoke StrLECopy, addr bName, [esi].pName, eax
                push eax
                call StrCopy
                lea eax, bName
                push eax
              .endif
            .endif
            OCall [esi].pTextureManager::D3D_TextureManager.GetTexture, /eax
            OCall [esi].TextureColl::DwordCollection.Insert, eax
          .endif
          inc ebx
          add pMaterials, sizeof D3DXMATERIAL
        .endw
        ICall pIMaterialBuffer::ID3DXBuffer.Release
      .endif
      xor eax, eax
    .endif
MethodEnd eax


; ——————————————————————————————————————————————————————————————————————————————————————————————————
;All the following methods relate to building of a SphereTree
;using an extended BSP tree algorithm



;Convert the Mesh's vertex and index data into more friendly Face structures..
;Unique Planes are stored in a separate collection, duplicates are filtered.
;We set up the vertex pointers in the face structs with actual vertices,
;but the rest of the code doesn't know or care about anything except that
;the position xyz data is at offset zero for each vertex.
;This means we can use anything starting with a Vec3 as a vertex,
;we have decoupled the vertex structure from the bsp-genning and portal-finding code :D
Method D3D_CollisionMesh.PrepareFaces,uses esi
LOCAL cnt,numculled
LOCAL p0,p1,p2		;vertex pointers
LOCAL plane:D3DXVECTOR4
LOCAL faces,pFace

	SetObject esi
	mov numculled,0
	mov faces,$New(DataCollection,Init,0,16,256,-1)
		
	;Find out how many triangles are in the model
	mov cnt,$ICall([esi].pIMesh::ID3DXMesh.GetNumFaces)		;#faces

    DbgDec cnt,"#Faces in Model"

	xor ecx,ecx
	.while ecx<cnt
		push ecx

			;Obtain three vertex-indices from indexbuffer (one triangle)
			mov eax,[esi].IndexBuffer.dIndexSize
			mul ecx
			mov ebx,3
			mul ebx
			add eax,[esi].IndexBuffer.pBuffer
			.if [esi].IndexBuffer.dIndexSize==2
				movzx ebx,word ptr[eax+2]
				movzx ecx,word ptr[eax+4]
				movzx eax,word ptr[eax]
			.else
				mov ebx,dword ptr[eax+4]
				mov ecx,dword ptr[eax+8]
				mov eax,dword ptr[eax]
			.endif

			;Convert indices into vertex-pointers, and set any '-0' values to '0'
			mul [esi].VertexBuffer.dVertexSize
			add eax,[esi].VertexBuffer.pBuffer
			mov p0,eax
			.if [eax].Vec3.x==80000000h
				mov [eax].Vec3.x,0
			.endif
			.if [eax].Vec3.y==80000000h
				mov [eax].Vec3.y,0
			.endif
			.if [eax].Vec3.z==80000000h
				mov [eax].Vec3.z,0
			.endif

			mov eax,ebx
			mul [esi].VertexBuffer.dVertexSize
			add eax,[esi].VertexBuffer.pBuffer
			mov p1,eax
			.if [eax].Vec3.x==80000000h
				mov [eax].Vec3.x,0
			.endif
			.if [eax].Vec3.y==80000000h
				mov [eax].Vec3.y,0
			.endif
			.if [eax].Vec3.z==80000000h
				mov [eax].Vec3.z,0
			.endif
	
			mov eax,ecx
			mul [esi].VertexBuffer.dVertexSize
			add eax,[esi].VertexBuffer.pBuffer
			mov p2,eax		
			.if [eax].Vec3.x==80000000h
				mov [eax].Vec3.x,0
			.endif
			.if [eax].Vec3.y==80000000h
				mov [eax].Vec3.y,0
			.endif
			.if [eax].Vec3.z==80000000h
				mov [eax].Vec3.z,0
			.endif
				
			;Allocate a new Face struct, and write vertexpointers
			mov pFace, $OCall(faces::DataCollection.Insert, $MemAlloc(sizeof Face,MEM_INIT_ZERO))
			.if eax==0
				DbgWarning "Out of memory allocating Face"
				int 3
			.endif
			m2m [eax].Face.pV0, p0, edx
			m2m [eax].Face.pV1, p1, edx
			m2m [eax].Face.pV2, p2, edx	

			;Calculate the Plane for this Face
			invoke D3DXPlaneFromPoints,addr plane,p0,p1,p2
			;Check for illegal (NaN) values
			.if plane.x==0FFC00000h || plane.y==0FFC00000h || plane.z==0FFC00000h		
				MemFree pFace
				inc numculled
				DbgWarning "PreCulled Face"
			.else
			;	lea eax,plane
			;	DbgVec4 eax
				invoke D3DXVec3Normalize,addr plane,addr plane
				;Correct any -0 values to 0
				.if plane.x==80000000h
					mov plane.x,0
				.endif
				.if plane.y==80000000h
					mov plane.y,0
				.endif
				.if plane.z==80000000h
					mov plane.z,0
				.endif
				.if plane.w==80000000h
					mov plane.w,0
				.endif
				
				;Add the plane to a collection of unique planes
				OCall [esi].pPlanes::Vec4Collection.Append,addr plane

				;write ptr to unique plane into Face struct
				mov edx,pFace
				mov [edx].Face.pPlane,eax
				
				
			.endif
			
			;next Face
			pop ecx
			inc ecx

	.endw

	DbgDec numculled,"culled early due to aberrance"
	
	mov eax,faces
	DbgDec [eax].DataCollection.dCount,"#faces collected"

	
MethodEnd

;Entrypoint for BSP Tree Generator, passes the RootNode to the GenerateNode method.
Method D3D_CollisionMesh.GenerateTree,uses esi

	SetObject esi
	
	;Prepare the input collection of Faces
	mov edi,$OCall (esi.PrepareFaces)
	
	;Prepare the Root Node
	MemAlloc sizeof BSPNode,MEM_INIT_ZERO
	mov [eax].BSPNode.pFaces,edi
	mov [esi].pRootNode,eax
	
	;Recursively generate the Tree
	OCall esi.GenerateNode,eax
MethodEnd

;Recursive method to generate BSP Tree
Method D3D_CollisionMesh.GenerateNode,uses esi edi, pBSPNode
LOCAL Result:BestResult
	SetObject esi
	
	;Measure bounds of input face set
	OCall esi.ProbeBounds,pBSPNode

	;Handle Cases of (#Faces = 0) and (#Faces = 1)
	mov edi,pBSPNode
	mov ebx,[edi].BSPNode.pFaces
	.if [ebx].DataCollection.dCount==0
		DbgWarning "GenerateBSPNode : NO FACES TO PROCESS"
		int 3
	.elseif [ebx].DataCollection.dCount==1
		;We have discovered a Leaf Node (just a single Face)
		DbgWarning "Single-Face Leaf"
		ExitMethod
    .else
        DbgDec [ebx].DataCollection.dCount,"#Faces input to Node"
	.endif	

	;Handle Cases of (#Faces > 1) ... 
	;Choose one Face whose Plane best divides the remaining Faces
	invoke RtlZeroMemory,addr Result,sizeof BestResult
	OCall esi.ChooseBestDividingPlane,addr Result,[edi].BSPNode.pFaces
	.if eax==0
		;We failed to find a suitable splitting plane - why?
		.if ebx==1
			;We have found a concave or convex set..
			;This is just another kind of Leaf Node.
			int 3
			DbgWarning "GenerateBSPNode : DISCOVERED LEAF NODE (CONVEX SUBSET)"
			;inc NumLeaves
			ExitMethod
		.else
			DbgWarning "WARNING GenerateBSPNode - UNSPLITTABLE"
			;inc NumLeaves		
			int 3
			ExitMethod
		.endif
	.else
	    ;DbgHex eax,"Found best dividing Face"
		;DbgHex Result.pbestPlane,"The nicest splitting plane"
		;DbgDec Result.bestPositive,"has this many faces in front of it,"
		;DbgDec Result.bestNegative,"and this many faces behind it,"
		;DbgDec Result.bestSpanning,"it cuts through this many faces,"
		;DbgDec Result.bestCoinciding,"and this many faces are coplanar with it"
		
		;If our results are all zero, it means that the set of faces is convex (or concave)
		;ie, all faces are one the same side of the planes of all faces,
		;ie, all pointing in, or all pointing outwards
		.if Result.bestSpanning==0
			.if Result.bestPositive==0 || Result.bestNegative==0
				mov eax,pBSPNode
				mov eax,[eax].BSPNode.pFaces
				DbgDec [eax].DataCollection.dCount,"#Faces in Convex Leaf"
				mov edx,[eax].DataCollection.dCount
				.if edx==0
					DbgWarning "Warning - Why was I handed an empty leaf?"
					int 3
				.elseif edx>1
				    DbgWarning "Non-Trivial Convex Leaf"
                    OCall esi.DivideConvexLeaf,pBSPNode
				.endif
                ExitMethod
			.endif
		.endif
		
		;Mark the Node with the 'best splitting plane' we discovered
		DbgHex pBSPNode
		mov ebx,pBSPNode
		m2m [ebx].BSPNode.pSplitterPlane, [eax].Face.pPlane
		
		;Divide the faces into 'front and back child' nodes
		OCall esi.DivideFacesWithPlane,pBSPNode,addr Result
	.endif
	

	
	mov ebx,pBSPNode
	mov ebx,[ebx].BSPNode.pFaces	
	.if ebx!=0 && [ebx].DataCollection.dCount!=0
	    DbgDec [ebx].DataCollection.dCount
		DbgWarning "Danger, Will Robinson.. I expected this node's faces to be Exhausted"
		int 3
	.endif	
	
	;Recurse the Front, if any
	mov ebx,pBSPNode
	.if [ebx].BSPNode.pFront!=0
		DbgWarning "GenerateBSPNode - Recursing Front"
		mov ebx,pBSPNode
		OCall GenerateNode,[ebx].BSPNode.pFront
	.endif
	
	;Recurse the Back, if any
	mov ebx,pBSPNode
	.if [ebx].BSPNode.pBack!=0
		DbgWarning "GenerateBSPNode - Recursing Back"	
		mov ebx,pBSPNode
		OCall GenerateNode,[ebx].BSPNode.pBack
	.endif	

MethodEnd


;Given an input Node containing a list of triangles,
;find the BoundingSphere and Centroid (origin)
;by first finding the Bounding Box (axial limits)
Method D3D_CollisionMesh.ProbeBounds,uses esi ebx edi,pBSPNode
LOCAL halflength:Vec3
LOCAL vMin:Vec3
LOCAL vMax:Vec3
LOCAL cnt
	SetObject esi
	
	mov ebx,pBSPNode
	mov edi,[ebx].BSPNode.pFaces
	xor ebx,ebx
	mov vMin.x,ebx
	mov vMin.y,ebx
	mov vMin.z,ebx
	mov vMax.x,ebx
	mov vMax.y,ebx
	mov vMax.z,ebx
	m2m cnt,[edi].DataCollection.dCount,edx
	mov edi,[edi].DataCollection.pItems
	;Find the Axial Limits of the Bounding Box containing the input set of vertices
	.while ebx<cnt

		mov ecx,[edi+ebx*4]

		mov edx,[ecx].Face.pV0
		fMin [edx].Vec3.x, vMin.x
		fstp vMin.x
		fMax [edx].Vec3.x, vMax.x
		fstp vMax.x
		fMin [edx].Vec3.y, vMin.y
		fstp vMin.y
		fMax [edx].Vec3.y, vMax.y
		fstp vMax.y
		fMin [edx].Vec3.z, vMin.z
		fstp vMin.z
		fMax [edx].Vec3.z, vMax.z
		fstp vMax.z
		mov edx,[ecx].Face.pV1
		fMin [edx].Vec3.x, vMin.x
		fstp vMin.x
		fMax [edx].Vec3.x, vMax.x
		fstp vMax.x
		fMin [edx].Vec3.y, vMin.y
		fstp vMin.y
		fMax [edx].Vec3.y, vMax.y
		fstp vMax.y
		fMin [edx].Vec3.z, vMin.z
		fstp vMin.z
		fMax [edx].Vec3.z, vMax.z
		fstp vMax.z
		mov edx,[ecx].Face.pV2
		fMin [edx].Vec3.x, vMin.x
		fstp vMin.x
		fMax [edx].Vec3.x, vMax.x
		fstp vMax.x
		fMin [edx].Vec3.y, vMin.y
		fstp vMin.y
		fMax [edx].Vec3.y, vMax.y
		fstp vMax.y
		fMin [edx].Vec3.z, vMin.z
		fstp vMin.z
		fMax [edx].Vec3.z, vMax.z
		fstp vMax.z

		inc ebx
	.endw
	
	mov ebx,pBSPNode
	
    ;The Origin of the Node is taken to be the centroid of the Bounding Box
	fld vMax.x
	fsub vMin.x
	fmul r4_Half
	fst halflength.x
	fadd vMin.x
	fstp [ebx].BSPNode.vOrigin.x
	
	fld vMax.y
	fsub vMin.y
	fmul r4_Half
	fst halflength.y
	fadd vMin.y
	fstp [ebx].BSPNode.vOrigin.y

	fld vMax.z
	fsub vMin.z
	fmul r4_Half
	fst halflength.z
	fadd vMin.z
	fstp [ebx].BSPNode.vOrigin.z


	;The BoundingSphere Radius is calculated from the 3D Hypotenuse of the Bounding Box
	;using Pythagoras Theorem, this is likely a little larger than the real bounding radius
	;but we'd need to measure the distance from every vertex to our recently calculated origin
	;to find the precise radius, we might want to improve this later ??
	fld halflength.x
	fmul st(0),st(0)
	fld halflength.y
	fmul st(0),st(0)
	fld halflength.z
	fmul st(0),st(0)
	fadd
	fadd
	fsqrt
	fstp [ebx].BSPNode.fRadius
	

MethodEnd

;================================================================
;ChooseBestDividingPlane
;Given an input set of Faces (aka triangles),
;Select the face whose Plane "best" divides the input set.
;The "best" dividing plane can be described as that plane 
;which divides the faces most equally, while also 'cutting'
;the fewest faces.. really these are mutually exclusive goals.
;We use a 'Dual heuristic' to make intelligent decisions.
;Returns a pointer to the "best" Face, or NONE if no optimal
;'cutting plane' could be determined (due to 100% concave or convex set).
;Return Values :
;FAILED  - EAX = NONE
;SUCCESS - EAX = ptr BestFace
;================================================================
Method D3D_CollisionMesh.ChooseBestDividingPlane,uses esi, pResultOut:Pointer, pFacesIn;:ptr DataCollection
local pBestFace
local pSplitterFace
LOCAL pSplitterPlane
local pTestFace
local LeastSplits 
local NumPositive 
local NumNegative 
local NumSpanning 
LOCAL NumCoinciding
local fMinRelation 
local fRelation 
local fBestRelation 
local NumFacesBeingProcessed
local SplitterFaceIndex
LOCAL iTestFace
LOCAL string[256]:BYTE

;Obtain #Faces in FaceSet
mov ebx,pFacesIn
m2m NumFacesBeingProcessed,[ebx].DataCollection.dCount

mov fBestRelation,NONE
mov pBestFace, NONE

m2m fMinRelation , MINIMUMRELATION
mov LeastSplits ,  INFINITE

;Loop to find the face whose plane best divides the set.
.while pBestFace == NONE

	;Reset the "Current SplitterFace" counter
    xor ebx,ebx
    mov SplitterFaceIndex,ebx
	;for each face pSplitterFace in FaceSet
    .while ebx < NumFacesBeingProcessed
 ;   	DbgDec SplitterFaceIndex,"current test candidate face"    	
    	
    	;Obtain pointer to current Face
		mov pSplitterFace, $OCall (pFacesIn::DataCollection.ItemAt, ebx)
;		DbgHex pSplitterFace

        ;if pSplitterFace has not been used as divider previously during the creation of the tree,
		;Obtain pointer to SplitterPlane
		m2m pSplitterPlane, [eax].Face.pPlane
		
		;Evaluate the current candidate SplitterFace to find out
		;how nicely it would divide the input faces
		xor ebx,ebx
		mov NumPositive , ebx ;#faces in front of Plane
		mov NumNegative , ebx ;#faces behind Plane
		mov NumSpanning , ebx ;#faces Spanning the Plane
		mov iTestFace	 ,ebx ;index of 'each face other than pSplitterFace'  
		mov NumCoinciding,	1 ;#faces APON the Plane
            
		; for each face in FaceSet (except pSplitterFace)
		.while (ebx < NumFacesBeingProcessed)
			.if ebx!=SplitterFaceIndex			
				push ebx
				
				;Obtain pointer to current input Face
				mov pTestFace, $OCall (pFacesIn::DataCollection.ItemAt, ebx)

				;Evaluate the current input Face against the current candidate SplitterFace
				invoke ClassifyFacePlane,pTestFace,pSplitterPlane
				.if 	eax == FRONTFRONTFRONT \
				 	 || eax == FRONTFRONTCOPLANAR	|| eax == FRONTCOPLANARFRONT || eax==FRONTCOPLANARCOPLANAR\
				 	 || eax == COPLANARFRONTFRONT	|| eax == COPLANARFRONTCOPLANAR || eax==COPLANARCOPLANARFRONT
					inc NumPositive 
			;		DbgText "front"
				.elseif eax == BACKBACKBACK\
				 	 || eax == BACKBACKCOPLANAR		|| eax == BACKCOPLANARBACK || eax==BACKCOPLANARCOPLANAR\
					 || eax == COPLANARBACKBACK		|| eax == COPLANARBACKCOPLANAR || eax==COPLANARCOPLANARBACK
					inc NumNegative 
			;		DbgText "back"
				.elseif eax == FRONTFRONTBACK || eax==FRONTBACKFRONT || eax == BACKBACKFRONT	\
					 || eax == FRONTBACKBACK || eax == BACKFRONTFRONT  || eax == BACKFRONTBACK	\
					 || eax == FRONTBACKCOPLANAR || eax== FRONTCOPLANARBACK\
					 || eax == BACKFRONTCOPLANAR		|| eax== BACKCOPLANARFRONT \
					 || eax == COPLANARFRONTBACK	|| eax == COPLANARBACKFRONT
					inc NumSpanning 
			;		DbgText "spanning"
				.elseif eax==COPLANARCOPLANARCOPLANAR
					inc NumCoinciding
			;		DbgText "coplanar"
				.else
					DbgWarning "Unhandled return value in ChooseSplitter"
					DbgDec eax,"Unhandled"
					invoke ExitProcess,0
					.break
				.endif
				pop ebx
			.endif
			inc ebx
			mov iTestFace,ebx						
		.endw
			
		.if NumPositive==0 && NumNegative==0
			.if NumCoinciding==1 && NumSpanning==0
				DbgWarning "ERROR - ClassifyPlaneFace Failed"
				int 3
			.else
				;Hacky fix for special cases such as just two coplanar faces
				jmp Acceptable
			.endif        	
		.endif
      
		;Compare the #faces on either side of the Plane
		;We will create a ratio ("Relationship factor")
		;by looking at the two values as a fraction,
		;whose value will be greater than 0.0 and as large as 1.0
		;ie, dividing the smaller value by the larger value
		;so we end up with a value of 1.0 = perfect balance
		;and worst balance indicated by the smallest value.
		mov eax,NumPositive 
		.if eax < NumNegative
			 fild NumPositive 
			 fiadd NumCoinciding
			 fild NumNegative	   ; = NumPositive / NumNegative
			 fiadd NumCoinciding
			 fdiv
		.else
			fild NumNegative 
			fiadd NumCoinciding
			fild NumPositive ; = NumNegative / NumPositive
			fiadd NumCoinciding
			fdiv
		.endif
		fst fRelation              
		;If the current Relation is less than or equal to the Minimum Relation
		;then we will NOT accept this Splitter as the best candidate
		fcomp fMinRelation
		fjle @F             
             
       	;OK, so which is larger - the current Relation, or the Best Relation So Far?            	
        fMax fRelation, fBestRelation
        fstp fBestRelation
                
		;If the current Splitter has less faces spanning it than our Best Splitter,
		;OR, if it creates no MORE splits AND is better balanced,
		;then we WILL accept this Splitter as the best candidate.
        mov eax,NumSpanning                 
        mov ebx,fBestRelation
        .if (eax < LeastSplits) || (eax == LeastSplits && ebx == fRelation)
Acceptable:
			m2m pBestFace,pSplitterFace
            m2m LeastSplits , NumSpanning
            m2m fBestRelation , fRelation
            .if pResultOut==NULL
	           	DbgWarning "ERROR - RETURNPTR PARAM IS NULL"
               	int 3
            .endif
            mov eax,pResultOut
            m2m [eax].BestResult.pbestPlane, pSplitterPlane                    
            m2m [eax].BestResult.bestPositive,NumPositive
            m2m [eax].BestResult.bestNegative,NumNegative
            m2m [eax].BestResult.bestSpanning,NumSpanning 
            m2m [eax].BestResult.bestCoinciding,NumCoinciding 
;            DbgHex pSplitterFace
        .endif
@@:
       inc SplitterFaceIndex
       mov ebx,SplitterFaceIndex  ; <--- update ebx for loop control
    .endw
         
    ;Decrease the number least acceptable relation by dividing it with a predefined constant.
    fld fMinRelation 
    fmul MINRELATIONSCALE
    fst fMinRelation 
    
    fcomp fEpsilon
    fjl @F
    
.endw
return pBestFace

@@: 
;Theres very few situations where selecting a suitable Splitter
;is simply impossible - one such example is where we have
;just two Faces, whose Planes are parallel but not coinciding.
;I dont think it should EVER happen, but it does?
DbgWarning "FAILED TO FIND ACCEPTABLE SPLITTER"
mov eax,pResultOut
.if [eax].BestResult.bestPositive==0 && [eax].BestResult.bestNegative!=0 && [eax].BestResult.bestSpanning==0
	DbgWarning "All Convex on Back Side"
	int 3
.elseif [eax].BestResult.bestPositive!=0 && [eax].BestResult.bestNegative==0 && [eax].BestResult.bestSpanning==0
	DbgWarning "All Convex on Front side"
	int 3
.else
	DbgWarning "Check the results, not sure WHY, maybe two parallel and concave faces?"
	int 3
.endif
mov eax, NONE
MethodEnd

;If a triangle would be 'cut' by the dividing plane,
;we use this macro to shove the entire triangle
;to whichever side of the plane it 'faces' (front or back child node)
;by comparing the Surface Normal of the triangle with that of the dividing plane.
ChooseSide macro pFace,pPlane
LOCAL @front
	;Perform a quick check for same plane pointers
	mov eax,pFace
	mov eax,[eax].Face.pPlane
	mov edx,pPlane
	.if eax==0 
		DbgWarning "NULL FacePlane in ChooseSide"
		int 3
	.elseif edx==0
		DbgWarning "NULL NodePlane in ChooseSide"
		int 3
	.elseif edx==eax				;if the planes are exactly the same,
		jmp @front				;we know the face goes to the front side
	.endif
	;Compare the two Normals using 1-DotProduct=cosTheta
	;(similar normals should yield a value close to zero)
	fld1
	Vec3Dot [eax].Vec3, [edx].Vec3
	fabs
	fsub 				
	fstp ftemp
	fMax ftemp,fEpsilon
	fstpReg eax
	.if eax==fEpsilon
		;The normals are the same, or damn close to being the same - send to Front
	@front:		mov eax,FRONT
	.else
		;The normals are not similar - not even close - send to Back
			 mov eax,BACK
	.endif
	exitm <eax>
endm

;Divide the given node's faces into 'front and back' child nodes
Method D3D_CollisionMesh.DivideFacesWithPlane ,uses esi ebx, pBSPNode,pBestResult
LOCAL pFaces,pFace,ftemp
LOCAL pPlane
LOCAL poutFront,poutBack
LOCAL dStyle
	SetObject esi
	
	mov ebx,pBSPNode
	m2m pPlane,[ebx].BSPNode.pSplitterPlane
	mov ebx,[ebx].BSPNode.pFaces
	.if ebx==0
		DbgWarning "Error : NULL input collection in DivideFacesWithPlane"
		ret
	.elseif [ebx].DataCollection.dCount==0
		DbgWarning "Error : empty input collection in DivideFacesWithPlane"
		ret
	.endif
	mov pFaces,ebx

	;Allocate collections to receive output faces
	;using the "best results" from ChooseBestDividingPlane
	;as a guide to the expected size of each collection
	mov poutFront,	 $New(DataCollection,Init,NULL,128,4096,-1)	
	.if eax==0
		DbgWarning "Out of memory creating front coll"
		int 3
	.endif
	mov poutBack, 	 $New(DataCollection,Init,NULL,128,4096,-1)	
	.if eax==0
		DbgWarning "Out of memory creating back coll"
		int 3
	.endif

	;Divide the input Faces into Front & Back collections
	mov ebx,pFaces
	.while [ebx].DataCollection.dCount!=0
		
		;Fetch next Face from the input set
		OCall ebx::DataCollection.DeleteAt,0
		mov pFace,eax
				
		mov dStyle,$invoke (ClassifyFacePlane, pFace, pPlane)
		.if eax==COPLANAR	|| eax==NONE			
			.if ChooseSide (pFace, pPlane)==FRONT
				OCall poutFront::DataCollection.Insert,pFace
			.else
				OCall poutBack::DataCollection.Insert,pFace
			.endif

		.elseif eax == FRONTFRONTFRONT \
			 || eax == FRONTFRONTCOPLANAR	|| eax == FRONTCOPLANARFRONT || eax==FRONTCOPLANARCOPLANAR\
			 || eax == COPLANARFRONTFRONT	|| eax == COPLANARFRONTCOPLANAR || eax==COPLANARCOPLANARFRONT
			 ;This face is in FRONT of the Splitting plane, we don't need to split this face
			;so we'll just move it into the Front Faces list
			OCall poutFront::DataCollection.Insert, pFace	
					

		.elseif eax == BACKBACKBACK\
			 || eax == BACKBACKCOPLANAR		|| eax == BACKCOPLANARBACK || eax==BACKCOPLANARCOPLANAR\
			 || eax == COPLANARBACKBACK		|| eax == COPLANARBACKCOPLANAR || eax==COPLANARCOPLANARBACK
			 ;This face is BEHIND the Splitting Plane, we don't need to split this face
			 ;so we'll just move it into the Back Faces list
			OCall poutBack::DataCollection.Insert, pFace
								
		.else
		    ;Face would be Cut by the Plane...
		    ;We'll take this opportunity to BALANCE OUR TREE
		    ;by sending this face to whichever child has fewer faces.
		    ;This will usually force that child's sphere to expand slightly.
		    mov eax,pBestResult
		    mov edx,[eax].BestResult.bestPositive
		    .if edx < [eax].BestResult.bestNegative
		        inc [eax].BestResult.bestPositive
                OCall poutFront::DataCollection.Insert,pFace
            .else
                inc [eax].BestResult.bestNegative
                OCall poutBack::DataCollection.Insert,pFace
            .endif
		.endif
		
		mov ebx,pFaces
	.endw
	
	;Trash the input face list, since we now have two potential output lists
	mov eax,pBSPNode
	mov edx,[eax].BSPNode.pFaces
	DbgDec [edx].DataCollection.dCount,"#faces remaining"
;	OCall [eax].BSPNode.pFaces::DataCollection.DeleteAll
;	mov eax,pBSPNode
;	Destroy [eax].BSPNode.pFaces
;	mov eax,pBSPNode
;	mov [eax].BSPNode.pFaces,0

	;Shove non-empty output lists into new child nodes
	mov eax,poutFront
	.if [eax].DataCollection.dCount==0
		Destroy poutFront
		mov poutFront,0
	.else   
	    DbgDec [eax].DataCollection.dCount,"front"
		;Create a child to hold Front faces
		mov ebx,pBSPNode
		mov [ebx].BSPNode.pFront,$MemAlloc (sizeof BSPNode,MEM_INIT_ZERO)
		m2m [eax].BSPNode.pFaces, poutFront
	.endif

	mov eax,poutBack
	.if [eax].DataCollection.dCount==0
		Destroy poutBack
		mov poutBack,0
	.else
	    DbgDec [eax].DataCollection.dCount,"back"
		;Create a child to hold Back faces
		mov ebx,pBSPNode
		mov [ebx].BSPNode.pBack,$MemAlloc (sizeof BSPNode,MEM_INIT_ZERO)
		m2m [eax].BSPNode.pFaces, poutBack
	.endif	

MethodEnd

;When the BSP generator discovers a convex set of faces,
;this method is used to CONTINUE generating the tree
;by splitting the convex set into two subsets
;based on the proximity of the faces.
Method D3D_CollisionMesh.DivideConvexLeaf,uses esi, pBSPNode
LOCAL cnt,cnt2
LOCAL listA,listB
LOCAL iMax
LOCAL ftemp
LOCAL iA,iB
LOCAL imax1,imax2
LOCAL v1:Vec3
LOCAL v2:Vec3

    SetObject esi

    mov edi,pBSPNode
    mov edi,[edi].BSPNode.pFaces
    push edi
    m2m cnt,[edi].DataCollection.dCount,edx
    dec edx
    mov cnt2,edx
    mov edi,[edi].DataCollection.pItems
    
    DbgDec cnt,"Dividing a Convex Set"
    
    ;Find the most distant pair of faces
    xor ebx,ebx
    mov iMax,ebx
    .while ebx<cnt2
        push ebx
        
        mov iA,ebx
        mov eax,[edi+ebx*4]
        mov edx,[eax].Face.pV0
        Vec3_Load [edx].Vec3
        mov edx,[eax].Face.pV0
        Vec3_AddFrom [edx].Vec3
        mov edx,[eax].Face.pV0
        Vec3_AddFrom [edx].Vec3
        Vec3_Stow v1
        
        inc ebx
        .while ebx<cnt
        
            mov iB,ebx
            mov edx,[edi+ebx*4]
            .if eax!=edx
                push eax
                
                mov eax,edx
                mov edx,[eax].Face.pV0
                Vec3_Load [edx].Vec3
                mov edx,[eax].Face.pV0
                Vec3_AddFrom [edx].Vec3
                mov edx,[eax].Face.pV0
                Vec3_AddFrom [edx].Vec3
                Vec3_Stow v2
                
                Vec3_Distance v1,v2
                fst ftemp
                fsub iMax
                fstpReg eax
                .ifBitClr eax,BIT31
                    DbgFloat ftemp
                    m2m imax1,iA,edx
                    m2m imax2,iB,edx
                    m2m iMax,ftemp,edx
                .endif
                pop eax
            .endif
            inc ebx
        .endw
 
        pop ebx
        inc ebx
    .endw     
    pop edi
    DbgDec imax1
    DbgDec imax2
    
    ;Create two output face lists
    ;Put each face in its own collection
    mov listB,$New(DataCollection,Init,0,16,256,-1)
    OCall edi::DataCollection.DeleteAt,imax2
    OCall listB::DataCollection.Insert,eax
    
    mov listA,$New(DataCollection,Init,0,16,256,-1)
    OCall edi::DataCollection.DeleteAt,imax1
    OCall listA::DataCollection.Insert,eax
    sub cnt,2
    
    ;Create two Child Nodes and place the new lists inside them
    mov edi,pBSPNode
    mov [edi].BSPNode.pFront,$MemAlloc(sizeof BSPNode,MEM_INIT_ZERO)
    m2m [eax].BSPNode.pFaces,listA,edx
    mov [edi].BSPNode.pBack,$MemAlloc(sizeof BSPNode,MEM_INIT_ZERO)
    m2m [eax].BSPNode.pFaces,listB,edx
    
    ;Process all remaining faces in a loop..
    .while cnt>0
        
        ;Find closest face to Origin A, add to list A
        OCall esi.GrabClosestFace,edi,[edi].BSPNode.pFront
        
        dec cnt
        .if cnt>0
            
            ;Find closest face to Origin B, add to list B
            OCall esi.GrabClosestFace,edi,[edi].BSPNode.pBack
            
            dec cnt
        .endif   
   
    .endw
 
    ;Release the (empty) input face collection
    mov edi,pBSPNode
    Destroy [edi].BSPNode.pFaces
    mov [edi].BSPNode.pFaces,0
    
    ;Recurse the non-trivial child nodes
    mov eax,[edi].BSPNode.pFront
    mov eax,[eax].BSPNode.pFaces
    .if eax!=0
        mov eax,[eax].DataCollection.dCount
        .if eax>1
            OCall esi.DivideConvexLeaf,[edi].BSPNode.pFront
        .elseif eax==1
            ;Calculate boundingsphere for single triangle
            OCall esi.ProbeBounds,[edi].BSPNode.pFront
        .endif
    .else
        DbgWarning "Whoops - empty node created"
        int 3
    .endif
    ;
    mov eax,[edi].BSPNode.pBack
    mov eax,[eax].BSPNode.pFaces
    .if eax!=0
        mov eax,[eax].DataCollection.dCount
        .if eax>1
            OCall esi.DivideConvexLeaf,[edi].BSPNode.pBack
        .elseif eax==1
            ;Calculate boundingsphere for single triangle
            OCall esi.ProbeBounds,[edi].BSPNode.pBack
        .endif
    .else
        DbgWarning "Whoops - empty node created"
        int 3
    .endif
    
MethodEnd


;Find the closest triangle in Parent Node's list to origin of Child Node
;Move this triangle from parent node to child node
Method D3D_CollisionMesh.GrabClosestFace, uses esi edi ebx, pParentNode, pChildNode
LOCAL cnt
LOCAL vtmp:Vec3
LOCAL tmp,min,ibest
    SetObject esi
    DbgWarning "GrabClosestFace"
    
    OCall esi.ProbeBounds,pChildNode
    DbgText "Bounds probed"
    
    mov edi,pParentNode
    mov edx,[edi].BSPNode.pFaces
    m2m cnt,[edx].DataCollection.dCount
    mov edi,[edx].DataCollection.pItems
    xor ebx,ebx
    fmov min,200000.0
    mov ibest,INFINITE
    .while ebx<cnt
        DbgDec ebx
        
        ;Obtain ptr to next face
        mov eax,[edi+ebx*4]
        push eax
        ;Calculate centroid of face
        mov edx,[eax].Face.pV0
        Vec3_Load [edx].Vec3
        mov edx,[eax].Face.pV1
        Vec3_AddFrom [edx].Vec3
        mov edx,[eax].Face.pV2
        Vec3_AddFrom [edx].Vec3
        mov eax,3
        fld1
        fildReg eax
        fdiv
        fmul st(3),st(0)
        fmul st(2),st(0)
        fmul
        Vec3_Stow vtmp
        
        ;Calculate distance from centroid to origin of child node
        mov edx,pChildNode
        Vec3_Distance vtmp,[edx].BSPNode.vOrigin
        fst tmp
        fsub min
        fstpReg edx
        pop eax
        .ifBitSet edx,BIT31 ;tmp < min
            m2m min,tmp
            mov ibest,ebx
        .endif
        
        inc ebx
    .endw


    mov edx,pParentNode
    OCall [edx].BSPNode.pFaces::DataCollection.DeleteAt, ibest
    mov edx,pChildNode
    OCall [edx].BSPNode.pFaces::DataCollection.Insert,eax
MethodEnd

endif