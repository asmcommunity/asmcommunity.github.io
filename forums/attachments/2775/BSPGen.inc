;Struct used to return information from 'ChooseBestDividingPlane'
BestResult struct
 pbestPlane	  dd ?	;Pointer to Best Dividing Plane
 bestPositive dd ?	;#Faces estimated on Front Side
 bestNegative dd ?	;#Faces estimated on Back Side
 bestSpanning dd ?	;#Faces estimated to be Split
 bestCoinciding dd ?
BestResult ends

;Struct represents a 3D Triangle
Face struct
	hV0     dword  ?	    ;Handle of 1st Vertex
	hV1     dword  ?		;Handle of 2nd Vertex
	hV2     dword  ?		;Handle of 3rd Vertex
	pPlane  Pointer ?		;Pointer to D3DXPLANE
	dAttrId dword   ?		;Each face is Tagged by attribute-group, which tell us the Material used.
Face ends					;Faces will be sorted by Material after the BSP Tree has been generated.

;Portal geometry is not stored in a D3D_VertexBuffer.
;Each portal fragment contains its own copy of each vertex.
;This allows us to unload a fragment + its vertices
;without affecting geometry that might share those vertices.

PortalTriangle struct
    v0 Vec3 <>               ;      A
    v1 Vec3 <>
    v2 Vec3 <>               ;    C   B
PortalTriangle ends

PortalQuad struct
    v0 Vec3 <>               ;  A       B
    v1 Vec3 <>
    v2 Vec3 <>
    v3 Vec3 <>               ;  D       C
PortalQuad ends

;Struct represents a Node in BSP Tree
BSPNode struct
	pFaces	Pointer ?		;Pointer to DataCollection
	pSplitterPlane Pointer ?;Pointer to D3DXPLANE
	pFront	Pointer ?		;Pointer to "front" child
	pBack	Pointer ?		;Pointer to "back" child

	vOrigin Vec3 <>
	fRadius real4 ?
	vMin	Vec3 <>
	vMax	Vec3 <>
	
	;Only valid in Leaf nodes
	pPortalTriangles Pointer ?
	pPortalQuads     Pointer ?
	pPortals         Pointer ?
	
BSPNode ends

;Struct represents a Portal connecting two Leaf Nodes in the BSP Tree.
;Initially, pFrags contains a large planar rectangle made from two triangles.
;Eventually, it will contain a list of frags that have the same attributes
;(ie, which connect the same two Nodes).
Portal struct
    pPlane      Pointer ?    ;Construction plane
    pTriangles  Pointer ?
    pQuads      Pointer ?
    pNodeA      Pointer ?    ;LeafNode connected by this Portal
    pNodeB      Pointer ?    ;LeafNode connected by this Portal
Portal ends

; ——————————————————————————————————————————————————————————————————————————————————————————————————
; Most of these INLINE macros will only make sense in the context of
; the function(s) which reference them, so ignore them for now,
; Come back and read them when you are ready.

	    .data
	    bugger real4 ?
	    .code
;Calculate the intersection of Edge and Plane
OneEdgeIntersect macro intersectout,PointA,PointB

    ;Find intersections of edge AB with Plane
    push edx
    invoke D3DXPlaneIntersectLine, intersectout,pPlane,PointA,PointB
    pop edx
    .if eax==NULL
        DbgWarning "Error - No Intersection: Plane and Edge appear parallel"
        DbgVec4 pPlane
        DbgHex PointA
        DbgVec3 PointA
        DbgHex PointB
        DbgVec3 PointB
        int 3

    .endif	    
    
    ;Debug - verify that calculated point is on the plane
    invoke ClassifyPointPlane, intersectout,pPlane
    .if eax!=COPLANAR
        DbgWarning "Error - the calculated point of intersection failed coplanarity test"
        DbgHex pPlane
        DbgVec4 pPlane
        DbgVec3 PointA
        DbgVec3 PointB
        DbgHex intersectout
        DbgVec3 intersectout
	    invoke D3DXPlaneDotCoord,pPlane, intersectout
	    fstp bugger
	    DbgFloat bugger,"calculated error magnitude"
        int 3
    .endif
    
    ifdef fScaling
        CalcTexCoords intersectout,PointA,PointB
    endif
endm

;Calculate two Edge/Plane intersections
TwoEdgesIntersect macro PointA,PointB,PointC,PointD
    DbgHex pNewVertex1
    DbgHex pNewVertex2
    OneEdgeIntersect pNewVertex1,PointA,PointB      ;Find intersections of edge AB with Plane
    OneEdgeIntersect pNewVertex2,PointC,PointD		;Find intersections of edge CD with Plane  
    
endm

;Allocate one new Face Vertex
NewFaceVertex1 macro      
    mov eax,[esi].Vertices.dCapacity
    push eax
    inc eax
    OCall [esi].Vertices::D3D_VertexBuffer.Resize,eax
    pop eax
    push eax
    OCall [esi].Vertices::D3D_VertexBuffer.GetPtr,eax
    mov pNewVertex1,eax
    pop eax
    OCall [esi].Vertices::D3D_VertexBuffer.GetHandle,eax
    mov hNewVertex1,eax
    ;refresh vertexpointers
    mov eax,pFace
    mov edx,[eax].Face.hV0
    add edx,[esi].Vertices.pBuffer
    mov pvA,edx
    mov edx,[eax].Face.hV1
    add edx,[esi].Vertices.pBuffer
    mov pvB,edx
    mov edx,[eax].Face.hV2
    add edx,[esi].Vertices.pBuffer
    mov pvC,edx
endm

;Allocate two new Face Vertices
NewFaceVertex2 macro
    mov eax,[esi].Vertices.dCapacity
    push eax
    add eax,2
    OCall [esi].Vertices::D3D_VertexBuffer.Resize,eax
    pop eax
    push eax
    OCall [esi].Vertices::D3D_VertexBuffer.GetPtr,eax
    mov pNewVertex1,eax    
    add eax,[esi].Vertices.dVertexSize
    mov pNewVertex2,eax
    pop eax
    OCall [esi].Vertices::D3D_VertexBuffer.GetHandle,eax
    mov hNewVertex1,eax
    add eax,[esi].Vertices.dVertexSize
    mov hNewVertex2,eax
    ;refresh vertexpointers
    mov eax,pFace
    mov edx,[eax].Face.hV0
    add edx,[esi].Vertices.pBuffer
    mov pvA,edx
    mov edx,[eax].Face.hV1
    add edx,[esi].Vertices.pBuffer
    mov pvB,edx
    mov edx,[eax].Face.hV2
    add edx,[esi].Vertices.pBuffer
    mov pvC,edx
endm

;Calculate Vertex Normal and UV Coordinates (for new vertex at Edge/Plane Intersection)
CalcTexCoords macro pNewVertex, pvA, pvB

    pushad
    push edi
    push esi
	;Measure the lengths of the edges A-I and I-B
	invoke D3DXPlaneDotCoord,pPlane,pvA
	fabs
	fstp fLength1		
	invoke D3DXPlaneDotCoord,pPlane,pvB
	fabs
	fstp fLength2
	
	;DbgFloat fLength1
	;DbgFloat fLength2
	
	;Is the length less than epsilon? if so, the triangle would be 'abberant'
	.if $IsLess(fLength1, fEpsilon)
		DbgWarning "Error - would generate aberrant edge 1"
		int 3
		return NONE
	.endif

	.if $IsLess(fLength2, fEpsilon)
		DbgWarning "Error - would generate aberrant edge 2"
		int 3
		return NONE
	.endif
	
	;Find a Scaling factor for UV interpolation 
	;fScale = distance from edgestart to intersection / total length of edge
	fld fLength1
	fld fLength1
	fadd fLength2	
	fdiv
	fstp fScaling

	;Calculate TextureCoords for each set of texcoords in the vertex format
	xor ecx,ecx
	.while ecx<[esi].dNumUVSets
		push ecx
		
		;Calculate UVSet for IntersectionVertex
		mov eax,[esi+ecx*4].aUVOffsets
		mov ebx,eax
		mov ecx,eax
		mov edi,eax		
		add edi,pNewVertex
		add eax,pvA
		add ebx,pvB
		add ecx,pvC
		invoke D3DXVec2Lerp,edi,ebx,eax,fScaling	


		pop ecx
		inc ecx
	.endw

    ;Calculate Normal for IntersectionVertex
	mov eax,[esi].dVertexNormalOffset
	mov ebx,eax
	mov ecx,eax
	mov edi,eax		
	add edi,pNewVertex
	add eax,pvA
	add ebx,pvB
	add ecx,pvC
	invoke D3DXVec3Lerp,edi,ebx,eax,fScaling
	invoke D3DXVec3Normalize,edi,edi
	pop esi
	pop edi
	popad
	
endm

;This macro compares a plane and a face,
;and tells us which Side of plane the Face points more towards
ChooseSide macro pFace,pPlane
LOCAL @front
	;Perform a quick check for same plane pointers
	mov eax,pFace
	mov eax,[eax].Face.pPlane
	mov edx,pPlane
	.if edx==eax				;if the planes are exactly the same,
		jmp @front				;we know the face goes to the front side
	.endif
	;Compare the two Normals using 1-DotProduct=cosTheta
	;(similar normals should yield a value close to zero)
	fld1
	Vec3Dot [eax].Vec3, [edx].Vec3
	fabs
	fsub 				
	fstp ftemp
	.if $IsLessOrEqual(ftemp,fEpsilon)
		;The normals are the same, or damn close to being the same - send to Front
	@front:		mov eax,FRONT
	.else
		;The normals are not similar - not even close - send to Back
			 mov eax,BACK
	.endif
	exitm <eax>
endm
; ——————————————————————————————————————————————————————————————————————————————————————————————————


Object BSPGen,234234,Primer
    RedefineMethod Init, Pointer
    RedefineMethod Done
    VirtualMethod New_Face,                  dword,dword,dword,Pointer,dword
    VirtualMethod New_PortalTriangle,        Pointer,Pointer,Pointer
    VirtualMethod New_PortalQuad,            Pointer,Pointer,Pointer,Pointer
    VirtualMethod PrepareFaces,              Pointer
    VirtualMethod ProbeBounds,               Pointer
    VirtualMethod GenerateTree,              Pointer
    VirtualMethod GenerateNode,              Pointer
    VirtualMethod ChooseBestDividingPlane,   Pointer,Pointer
    VirtualMethod DivideFacesWithPlane,      Pointer,Pointer,Pointer,Pointer
    VirtualMethod CutFaceWithPlane,          Pointer,Pointer,Pointer,Pointer
    VirtualMethod CutPortalTriangleWithPlane,Pointer,Pointer,Pointer,Pointer
    VirtualMethod CutPortalQuadWithPlane,    Pointer,Pointer,Pointer,Pointer,Pointer,Pointer
    VirtualMethod GeneratePortals,           Pointer
    VirtualMethod CreatePortal,              Pointer
    VirtualMethod ShatterPortal,             Pointer
    VirtualMethod DividePortalWithPlane,     Pointer,Pointer
    VirtualMethod Walk_PortalTriangle,       Pointer,Pointer
    VirtualMethod Walk_PortalQuad,           Pointer,Pointer
    VirtualMethod PruneFakePortals
    VirtualMethod IsFakePortalTriangle,      Pointer,Pointer
    VirtualMethod IsFakePortalQuad,          Pointer,Pointer
    VirtualMethod ClipPortalToPlaneFront,    Pointer,Pointer
    VirtualMethod ClipPortalToLeaf,          Pointer,Pointer
    VirtualMethod ClipPortalsToLeaves
    VirtualMethod FoundRealPortalTriangle,   Pointer,Pointer,Pointer
    VirtualMethod FoundRealPortalQuad,       Pointer,Pointer,Pointer
 ;   VirtualMethod RenderPortals,            Pointer
 ;   VirtualMethod Render
    VirtualMethod Release
    VirtualMethod Restore
    DefineVariable pDirect3D,Pointer,NULL
    DefineVariable pRootNode,Pointer,NULL
    DefineVariable dNumLeaves,dword,NULL
    DefineVariable dNumUVSets,dword,NULL            ;#texcoords in each vertex struct
    DefineVariable aUVOffsets,dword, 8 dup (<?>)    ;offset of each texcoord in vertex struct
    DefineVariable dVertexNormalOffset,dword,NULL
    Embed Vertices,      D3D_VertexBuffer
    Embed PortalVertices,DataCollection
    Embed Planes,        Vec4Collection
    Embed UsedPlanes,    DwordCollection
    Embed Portals,       DataCollection
    Embed Leaves,        DwordCollection
ObjectEnd

if IMPLEMENT

;A few standard OA32/D3D methods
Method BSPGen.Release,uses esi
    SetObject esi
    OCall [esi].Vertices::D3D_VertexBuffer.Release
MethodEnd

Method BSPGen.Restore,uses esi
    SetObject esi
    OCall [esi].Vertices::D3D_VertexBuffer.Restore,NULL
MethodEnd

Method BSPGen.Init,uses esi,pDirect3D
    SetObject esi
    m2m [esi].pDirect3D,pDirect3D,edx
    OCall [esi].Planes        ::Vec4Collection.Init,esi,16,256,-1
    OCall [esi].UsedPlanes    ::DwordCollection.Init,esi,16,256,-1
    mov [esi].UsedPlanes.dDuplicates,FALSE
    OCall [esi].PortalVertices::DataCollection.Init,esi,512,4096,-1
    OCall [esi].Portals       ::DataCollection.Init,esi,512,4096,-1
    OCall [esi].Leaves        ::DwordCollection.Init,esi,512,4096,-1
MethodEnd

Method BSPGen.Done,uses esi
    SetObject esi
    OCall [esi].Leaves        ::DwordCollection.Done
    OCall [esi].Vertices      ::D3D_VertexBuffer.Done
    OCall [esi].Planes        ::Vec4Collection.Done
    OCall [esi].PortalVertices::DataCollection.Done
    OCall [esi].Portals       ::DataCollection.Done
    OCall [esi].UsedPlanes    ::DwordCollection.Done
    ACall Done
MethodEnd

;Import all necessary data from the given D3D_Mesh object.
;Convert the Mesh's vertex and index data into more friendly structures.
;Unique Planes are stored in a separate collection, duplicates are filtered.
Method BSPGen.PrepareFaces,uses esi ebx,pRefMesh
LOCAL cnt,endofgroup
LOCAL numculled     ;#triangles NOT imported, because they were 'abberant' (not legal triangles).
LOCAL pRange		;current 'range of faces' (D3DXATTRIBUTERANGE)
LOCAL pFace			;newly created Face struct
LOCAL hA,hB,hC,pA,pB,pC		;vertex handles and pointers
LOCAL plane:Vec4, plane2:Vec4
LOCAL vtemp:Vec3
local dNumAttributes, iAttr
LOCAL iB,iC
LOCAL Faces

	SetObject esi
	DbgText "Preparing Faces"
	mov numculled,0
	mov Faces,$New(DataCollection,Init,esi,16,256,-1)
	
	mov ebx,pRefMesh
    OCall [esi].Vertices::D3D_VertexBuffer.Create,esi, [esi].pDirect3D, [ebx].D3D_Mesh.VertexBuffer.dCapacity,\
    [ebx].D3D_Mesh.VertexBuffer.dVertexSize, [ebx].D3D_Mesh.VertexBuffer.dUsage,\
    [ebx].D3D_Mesh.VertexBuffer.dPool
    .if eax!=0
        DbgWarning "Error - failed to initialize vb"
        int 3
    .endif
    m2m dNumAttributes, [ebx].D3D_Mesh.dNumAttributes, edx
    
    ;Clone the vertex data
    mov eax,[ebx].D3D_Mesh.VertexBuffer.dVertexSize
    mul [ebx].D3D_Mesh.VertexBuffer.dCapacity
    invoke RtlMoveMemory,[esi].Vertices.pBuffer,[ebx].D3D_Mesh.VertexBuffer.pBuffer,eax
	
	
	;Examine the vertex declaration to find out about texture coords
    xor ecx,ecx
    .while ecx<7
        push ecx
        OCall pRefMesh::D3D_Mesh.GetVertexComponent, NULL, D3DDECLUSAGE_TEXCOORD, ecx
        pop ecx
        .break .if eax == 0
        .if [eax].D3DVERTEXELEMENT9._Type!=D3DDECLTYPE_FLOAT2   ;I expect Vec2 texcoords
            DbgWarning "error - unexpected uv size"
            int 3
        .endif
        movzx eax,[eax].D3DVERTEXELEMENT9._Offset
        mov [esi+ecx*4].aUVOffsets,eax
        
        inc ecx
    .endw
    mov [esi].dNumUVSets,ecx

    ;Examine the vertex declaration to find out about Vertex Normal
    ;(I expect there to be only one Normal component)
    OCall pRefMesh::D3D_Mesh.GetVertexComponent, NULL, D3DDECLUSAGE_NORMAL, 0
    .if eax!=0
        movzx eax,[eax].D3DVERTEXELEMENT9._Offset
        mov [esi].dVertexNormalOffset,eax
    .endif

	;Figure out how many triangles
	mov ebx,pRefMesh
	mov eax,[ebx].D3D_Mesh.IndexBuffer.dCapacity
	mov ebx,3
	xor edx,edx
	div ebx
	mov cnt,eax		;#faces
	
	
	;Spew some info to debug
	mov ebx,pRefMesh
	DbgDec [ebx].D3D_Mesh.VertexBuffer.dVertexSize
	DbgDec cnt, "#Faces"	
	DbgDec [ebx].D3D_Mesh.dNumAttributes,     "#FaceGroups"	
	DbgDec [ebx].D3D_Mesh.MaterialColl.dCount,"#Materials"
	
	

	;For each Attribute Group (ie, for all Faces sharing one Material)
	xor ecx,ecx
	.while ecx<dNumAttributes	
		push ecx
		mov iAttr,ecx	
		
		;Access the current Attribute Group 
		mov ebx,pRefMesh
		mov eax,sizeof D3DXATTRIBUTERANGE
		mul ecx
		add eax,[ebx].D3D_Mesh.pAttributes
		mov pRange,eax
		
		;Calculate end of group
		mov ecx, [eax].D3DXATTRIBUTERANGE.FaceStart
		mov edx, [eax].D3DXATTRIBUTERANGE.FaceCount
		add edx, ecx
		mov endofgroup, edx
		
		DbgDec ecx,"start of group"
		DbgDec endofgroup,"umm"

		;For each triangle in the current Group
		.while ecx<endofgroup
			push ecx
           ;DbgDec ecx,"current faceid"
            
			;Obtain three vertex-indices from indexbuffer (one triangle)
			mov ebx,pRefMesh
			mov eax,ecx
			mul [ebx].D3D_Mesh.IndexBuffer.dIndexSize	
			mov edx,3
			mul edx
			add eax,[ebx].D3D_Mesh.IndexBuffer.pBuffer
			.if [ebx].D3D_Mesh.IndexBuffer.dIndexSize==2
				movzx ebx,word ptr[eax+2]
				movzx ecx,word ptr[eax+4]
				movzx eax,word ptr[eax]
			.else
				mov ebx,dword ptr[eax+4]
				mov ecx,dword ptr[eax+8]
				mov eax,dword ptr[eax]
			.endif
			
			mov iB,ebx
			mov iC,ecx
			
			DbgDec eax
			DbgDec iB
			DbgDec iC

			;Convert indices into vertex handles (safe)
			mov ebx,pRefMesh
			mov hA,$OCall ([ebx].D3D_Mesh.VertexBuffer::D3D_VertexBuffer.GetHandle,eax)
			DbgDec hA
			mov hB,$OCall ([ebx].D3D_Mesh.VertexBuffer::D3D_VertexBuffer.GetHandle,iB)
			DbgDec hB
			mov hC,$OCall ([ebx].D3D_Mesh.VertexBuffer::D3D_VertexBuffer.GetHandle,iC)
			DbgDec hC

			;Convert handles into vertex pointers (unsafe)
			mov pA,$OCall ([ebx].D3D_Mesh.VertexBuffer::D3D_VertexBuffer.PtrFromHandle,hA)
			mov pB,$OCall ([ebx].D3D_Mesh.VertexBuffer::D3D_VertexBuffer.PtrFromHandle,hB)
			mov pC,$OCall ([ebx].D3D_Mesh.VertexBuffer::D3D_VertexBuffer.PtrFromHandle,hC)
			
			DbgVec3 pA
			DbgVec3 pB
			DbgVec3 pC
			DbgLine
			
			;Calculate the Plane for this Face
			invoke D3DXPlaneFromPoints,addr plane,pA,pB,pC
			;Check for illegal (NaN) values
			.if plane.x==0FFC00000h || plane.y==0FFC00000h || plane.z==0FFC00000h		
				MemFree pFace
				inc numculled
				DbgWarning "PreCulled Face"
			.else
                ;Normalize the plane normal
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
				

				DbgVec4 plane
				;Add the plane to a collection of unique planes
				OCall [esi].Planes::Vec4Collection.Insert,addr plane
				
    			;Create new Face
    			mov pFace, $OCall(esi.New_Face,hA,hB,hC,eax,iAttr)
				
			

				;Add Face struct to global collection
				OCall Faces::DataCollection.Insert,pFace
                
			.endif
			
			;next Face
			pop ecx
			inc ecx
		.endw	

		;next Material group
		pop ecx
		inc ecx
	.endw


	DbgDec numculled,"culled early due to aberrance"
	DbgDec [esi].Planes.dCount,"#unique planes"

    mov eax,Faces
	DbgDec [eax].DataCollection.dCount,"#Faces in total"
	DbgDec dNumAttributes,"#groups of faces (#materials)"
	

MethodEnd

;Entrypoint for BSP Tree Generator, passes the RootNode to the GenerateNode method.
Method BSPGen.GenerateTree,uses esi,pInputFaces
	SetObject esi
	MemAlloc sizeof BSPNode,MEM_INIT_ZERO
	m2m [eax].BSPNode.pFaces,pInputFaces,edx
	mov [esi].pRootNode,eax
	OCall esi.GenerateNode,eax
	DbgDec [esi].dNumLeaves
	DbgDec [esi].Leaves.dCount
	DbgLine
	OCall esi.GeneratePortals,[esi].pRootNode
	DbgLine
	OCall esi.PruneFakePortals
	DbgLine
	OCall esi.ClipPortalsToLeaves
	DbgLine
	DbgWarning "Thats all so far"
MethodEnd

;Generate a BSPTree from a given BSPNode.
;This recursive function takes a BSPNode containing an input set of faces
;finds the face whose plane best splits the faceset,
;then splits the faceset into child BSPNodes,
;and then recurses each child BSPNode. 
Method BSPGen.GenerateNode,uses esi ebx, pBSPNode
LOCAL Result:BestResult
LOCAL poutFront,poutBack
	SetObject esi
	
	DbgLine
	DbgWarning "Generating BSP Node"
	;Measure bounds of input face set
	OCall esi.ProbeBounds,pBSPNode

	;Handle Cases of (#Faces = 0) and (#Faces = 1)
	mov ebx,pBSPNode
	mov edx,[ebx].BSPNode.pFaces
	.if [edx].DataCollection.dCount==0
		DbgWarning "GenerateBSPNode : NO FACES TO PROCESS"
		int 3
	.elseif [edx].DataCollection.dCount==1
		;We have discovered a Leaf Node (just a single Face)
		;Collect the Leaf node into linear list
		DbgWarning "Single-Face Leaf"
		inc [esi].dNumLeaves
		OCall [esi].Leaves::DwordCollection.Insert,pBSPNode
		ExitMethod
	.endif	

	;Handle Cases of (#Faces > 1) ... 
	;Choose one Face whose Plane best divides the remaining Faces
	invoke RtlZeroMemory,addr Result,sizeof BestResult
	OCall esi.ChooseBestDividingPlane,addr Result,[ebx].BSPNode.pFaces
	.if eax==NONE
		;We failed to find a suitable splitting plane - why?
		.if edx==1
			;We have found a concave or convex set..
			;This is just another kind of Leaf Node.
			DbgWarning "GenerateBSPNode : DISCOVERED LEAF NODE (CONVEX SUBSET)"
			inc [esi].dNumLeaves
			OCall [esi].Leaves::DwordCollection.Insert,pBSPNode
			ExitMethod
		.else
			DbgWarning "WARNING GenerateBSPNode - UNSPLITTABLE"
			inc [esi].dNumLeaves
			OCall [esi].Leaves::DwordCollection.Insert,pBSPNode
			ExitMethod
		.endif
	.else
	
	    ;Add Plane of selected face to 'Used Planes' collection,
	    ;preventing it from being selected again in future.
		push eax
		OCall [esi].UsedPlanes::DwordCollection.Insert,[eax].Face.pPlane
		pop eax
		
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
				.if [eax].DataCollection.dCount==0
					DbgWarning "Warning - Why was I handed an empty leaf?"
					int 3
				.endif
				inc [esi].dNumLeaves
				OCall [esi].Leaves::DwordCollection.Insert,pBSPNode
				ret
			.endif
		.endif
		
		;Mark the Node with the 'best splitting plane' we discovered
		mov ebx,pBSPNode
		m2m [ebx].BSPNode.pSplitterPlane, [eax].Face.pPlane

    	;Create lists to receive output faces
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
    	
    	mov edx,[ebx].BSPNode.pFaces
    	DbgDec [edx].DataCollection.dCount,"#faces to be sorted"
	
		;split the faces into 'front and back child' nodes
		OCall DivideFacesWithPlane,[ebx].BSPNode.pFaces, [ebx].BSPNode.pSplitterPlane,poutFront,poutBack
        mov [ebx].BSPNode.pFaces,NULL
        
    	;Shove non-empty output lists into new child nodes
    	mov eax,poutFront
    	.if [eax].DataCollection.dCount==0
    		Destroy poutFront
    	.else
    	    push eax
    	    DbgDec [eax].DataCollection.dCount,"#faces went to Front"
    		mov [ebx].BSPNode.pFront,$MemAlloc (sizeof BSPNode,MEM_INIT_ZERO)
    		pop [eax].BSPNode.pFaces
    	.endif
    
    	mov eax,poutBack
    	.if [eax].DataCollection.dCount==0
    		Destroy poutBack
        .else
    	    push eax
    	    DbgDec [eax].DataCollection.dCount,"#faces went to Back"
    		mov [ebx].BSPNode.pBack,$MemAlloc (sizeof BSPNode,MEM_INIT_ZERO)
    		pop [eax].BSPNode.pFaces
    	.endif	




	.endif
	
	;Recurse the Front child, if any
	.if [ebx].BSPNode.pFront!=0
	    DbgWarning "Recursing Front"
		OCall GenerateNode,[ebx].BSPNode.pFront
	.endif
	;Recurse the Back child, if any
	.if [ebx].BSPNode.pBack!=0
	    DbgWarning "Recursing Back"
		OCall GenerateNode,[ebx].BSPNode.pBack
	.endif	
MethodEnd

;Walk the BSP Tree looking for NON LEAF nodes.
;Generate a large flat rectangle on the splittingplane of each nonleaf node.
;Cut the rectangle with all the other splitting planes.
;Send each resulting fragment through the tree until they hit a Leaf node.
;When this method returns, the Leaf nodes will contain all the Fragments.
Method BSPGen.GeneratePortals,uses esi edi,pBSPNode
LOCAL pPortal
    SetObject esi
    mov edx,pBSPNode
    .if [edx].BSPNode.pFront==0 && [edx].BSPNode.pBack==0
        ;DbgText "DiscoverPortals found a Leaf"
        
    .else
        ;Generate a large flat rectangle on the splittingplane 
        mov pPortal,$OCall (esi.CreatePortal,pBSPNode)
        ;Cut the rectangle with all the other splitting planes.
        OCall esi.ShatterPortal,eax
        
        ;Send all fragments through the tree
        mov edi,pPortal
        mov edi,[edi].Portal.pTriangles
        .while [edi].DataCollection.dCount!=0
            OCall edi::DataCollection.DeleteAt,0
            OCall esi.Walk_PortalTriangle,[esi].pRootNode,eax
        .endw
        Destroy edi     ;Trash the (empty) input list
            
        mov edi,pPortal
        mov edi,[edi].Portal.pQuads
        .while [edi].DataCollection.dCount!=0
            OCall edi::DataCollection.DeleteAt,0
            OCall esi.Walk_PortalQuad,[esi].pRootNode,eax
        .endw
        Destroy edi
        
        ;Trash the container
        MemFree pPortal
        
        ;Recurse child nodes
        mov edi,pBSPNode
        .if [edi].BSPNode.pFront!=0
            OCall esi.GeneratePortals,[edi].BSPNode.pFront
        .endif
        .if [edi].BSPNode.pBack!=0
            OCall esi.GeneratePortals,[edi].BSPNode.pBack
        .endif
    .endif
MethodEnd

;Cut a new portal rectangle against all the splittingplanes in the bsp tree
Method BSPGen.ShatterPortal,uses esi ebx,pPortal
    SetObject esi
    DbgText "Splitting Large Portal"    
    ;Cut the portal fragments against all Splitting Planes
    ;(except the Construction plane)
    xor ebx,ebx
    .while ebx<[esi].Planes.dCount
        OCall [esi].Planes::Vec4Collection.ItemAt,ebx
        mov edx,pPortal
        .if [edx].Portal.pPlane!=eax
            OCall esi.DividePortalWithPlane,pPortal,eax
            mov eax,pPortal
            mov eax,[eax].Portal.pTriangles
            DbgDec [eax].DataCollection.dCount,"#triangles in portal"
            mov eax,pPortal
            mov eax,[eax].Portal.pQuads
            DbgDec [eax].DataCollection.dCount,"#quads in portal"
        .endif
        inc ebx
    .endw
MethodEnd

;Recursively 'walk' a Fragment through the BSP Tree until it lands in a Leaf.
;Since we already 'pre-split' the portal against all splitting planes,
;we should never need to Split fragments within this method.
;This fact can be used to detect errors in the triangle splitting code.
Method BSPGen.Walk_PortalTriangle,uses esi ebx,pBSPNode,pPortalTriangle
    SetObject esi

    mov ebx,pBSPNode
    .if [ebx].BSPNode.pFront==0 && [ebx].BSPNode.pBack==0
        DbgWarning "Walk_PortalTriangle found a Leaf"
        .if [ebx].BSPNode.pPortalTriangles==0
            mov [ebx].BSPNode.pPortalTriangles,$New(DwordCollection,Init,esi,0,256,-1)            
        .endif
        OCall [ebx].BSPNode.pPortalTriangles::DwordCollection.Insert,pPortalTriangle
    .else
        ;Classify the fragment against the plane..
        ;Remember, we expect the result to show the fragment
        ;clearly on one side of the plane or the other ... not 'split' by it,
        ;since we already 'pre-split' the fragments as much as possible.
        mov edx,pPortalTriangle
        .switch ($invoke (ClassifyTrianglePlane,addr [edx].PortalTriangle.v0,addr [edx].PortalTriangle.v1,addr [edx].PortalTriangle.v2,[ebx].BSPNode.pSplitterPlane))
        .case BACKBACKBACK,BACKCOPLANARCOPLANAR,BACKCOPLANARBACK,BACKBACKCOPLANAR,COPLANARCOPLANARBACK,COPLANARBACKBACK,COPLANARBACKCOPLANAR
            ;Send portal fragment reference down the Back child
            OCall esi.Walk_PortalTriangle,[ebx].BSPNode.pBack,pPortalTriangle
        .case FRONTFRONTFRONT,COPLANARCOPLANARFRONT,COPLANARFRONTCOPLANAR,FRONTCOPLANARCOPLANAR,COPLANARFRONTFRONT,FRONTCOPLANARFRONT,FRONTFRONTCOPLANAR
            ;Send portal fragment reference down the Front child
            OCall esi.Walk_PortalTriangle,[ebx].BSPNode.pFront,pPortalTriangle
        .case COPLANAR
            ;Special case - send portal fragment reference down BOTH childs...
            ;We will seek the duplicated pointers later.
            OCall esi.Walk_PortalTriangle,[ebx].BSPNode.pFront,pPortalTriangle
            OCall esi.Walk_PortalTriangle,[ebx].BSPNode.pBack,pPortalTriangle
        .default
            DbgWarning "Error - No splits should occur during WalkTriangle,"
            DbgWarning "however there MAY be unhandled cases in this Method."
            DbgDec eax,"Reported Split Type"
           ; mov ebx,pFragment
           ; DbgVec3 [ebx].Face.pV0
           ; DbgVec3 [ebx].Face.pV1
           ; DbgVec3 [ebx].Face.pV2
           ; mov edx,pBSPNode
           ; DbgVec4 [edx].BSPNode.pSplitterPlane
            int 3
        .endsw
    .endif
MethodEnd

;Recursively 'walk' a Fragment through the BSP Tree until it lands in a Leaf.
;Since we already 'pre-split' the portal against all splitting planes,
;we should never need to Split fragments within this method.
;This fact can be used to detect errors in the triangle splitting code.
Method BSPGen.Walk_PortalQuad,uses esi ebx,pBSPNode,pPortalQuad
    SetObject esi

    mov ebx,pBSPNode
    .if [ebx].BSPNode.pFront==0 && [ebx].BSPNode.pBack==0
        DbgWarning "Walk_PortalQuad found a Leaf"
        .if [ebx].BSPNode.pPortalQuads==0
            mov [ebx].BSPNode.pPortalQuads,$New(DwordCollection,Init,esi,0,256,-1)            
        .endif
        OCall [ebx].BSPNode.pPortalQuads::DwordCollection.Insert,pPortalQuad
    .else
        ;Classify the fragment against the plane..
        ;Remember, we expect the result to show the fragment
        ;clearly on one side of the plane or the other ... not 'split' by it,
        ;since we already 'pre-split' the fragments as much as possible.
        mov edx,pPortalQuad
        .switch ($invoke (ClassifyQuadPlane,addr [edx].PortalQuad.v0,addr [edx].PortalQuad.v1,addr [edx].PortalQuad.v2,addr [edx].PortalQuad.v3,[ebx].BSPNode.pSplitterPlane))
	    .case  FRONTFRONTFRONTFRONT, FRONTFRONTFRONTCOPLANAR, FRONTFRONTCOPLANARFRONT,FRONTCOPLANARFRONTFRONT,COPLANARFRONTFRONTFRONT
            ;Send portal fragment reference down the Front child
            OCall esi.Walk_PortalQuad,[ebx].BSPNode.pFront,pPortalQuad		
        .case  COPLANARCOPLANARFRONTFRONT,FRONTCOPLANARCOPLANARFRONT,FRONTFRONTCOPLANARCOPLANAR,COPLANARFRONTFRONTCOPLANAR
            ;Send portal fragment reference down the Front child
            OCall esi.Walk_PortalQuad,[ebx].BSPNode.pFront,pPortalQuad		
	    .case BACKBACKBACKBACK, BACKBACKBACKCOPLANAR, BACKBACKCOPLANARBACK, BACKCOPLANARBACKBACK,COPLANARBACKBACKBACK	      
            ;Send portal fragment reference down the Back child
            OCall esi.Walk_PortalQuad,[ebx].BSPNode.pBack,pPortalQuad
        .case  COPLANARCOPLANARBACKBACK,BACKCOPLANARCOPLANARBACK,BACKBACKCOPLANARCOPLANAR,COPLANARBACKBACKCOPLANAR
            ;Send portal fragment reference down the Back child
            OCall esi.Walk_PortalQuad,[ebx].BSPNode.pBack,pPortalQuad
        .case FRONTFRONTFRONT,COPLANARCOPLANARFRONT,COPLANARFRONTCOPLANAR,FRONTCOPLANARCOPLANAR,COPLANARFRONTFRONT,FRONTCOPLANARFRONT,FRONTFRONTCOPLANAR
            ;Send portal fragment reference down the Front child
            OCall esi.Walk_PortalQuad,[ebx].BSPNode.pFront,pPortalQuad
        .case COPLANAR,NONE
            ;Special case - send portal fragment reference down BOTH childs...
            ;We will seek the duplicated pointers later.
            OCall esi.Walk_PortalQuad,[ebx].BSPNode.pFront,pPortalQuad
            OCall esi.Walk_PortalQuad,[ebx].BSPNode.pBack,pPortalQuad
        .default
            DbgWarning "Error - No splits should occur during WalkQuad,"
            DbgWarning "however there MAY be unhandled cases in this Method."
            DbgDec eax,"Reported Split Type"
            int 3
        .endsw
    .endif
MethodEnd




;================================================================
;ChooseBestDividingPlane
;Given an input set of Faces (DataCollection),
;select the Face whose Plane "best" divides the FaceSet.
;The "best" dividing plane can be described as that plane 
;which divides the faces most equally, while also splitting
;the fewest faces, which really are mutually exclusive goals.
;Returns a pointer to the "best" Face, or NONE if no optimal
;splitter could be determined (due to 100% concave or convex set).
;Return Values :
;FAILED  - EAX = NONE
;SUCCESS - EAX = ptr BestFace, EBX = ptr to Results struct
;================================================================
Method BSPGen.ChooseBestDividingPlane,uses esi ebx, pResultOut:Pointer, pFacesIn
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

SetObject esi
mov eax,[esi].UsedPlanes.dCount
.if eax==[esi].Planes.dCount
    DbgWarning "doh - ran out of unique planes, bsptree generation terminating"
    mov eax,NONE
    mov edx,1
    ret
.endif


;Obtain #Faces in FaceSet
mov ebx,pFacesIn
m2m NumFacesBeingProcessed,[ebx].DataCollection.dCount

mov fBestRelation,NONE
mov pBestFace, NONE


.data
MINIMUMRELATION  FLOAT 0.8f		;fine-tuning of the dual heuristic in 'ChooseBestDividingPlane'
MINRELATIONSCALE FLOAT 0.5f;.8		;how much better is 'better', anyway? Angry Factor
.code
m2m fMinRelation , MINIMUMRELATION
mov LeastSplits ,  INFINITE

;Loop to find the face whose plane best divides the set.
.while pBestFace == NONE

    xor ebx,ebx                             ;Reset the "Current SplitterFace" counter
    mov SplitterFaceIndex,ebx
    .while ebx < NumFacesBeingProcessed     ;for each face pSplitterFace in FaceSet
  	
    	;Obtain pointer to current Face
		mov pSplitterFace, $OCall (pFacesIn::DataCollection.ItemAt, ebx)

		;Obtain pointer to SplitterPlane
		m2m pSplitterPlane, [eax].Face.pPlane
		
		;Has this Plane been used as a SplitterPlane previously?
		OCall [esi].UsedPlanes::DwordCollection.IndexOf,[eax].Face.pPlane
		.if eax==-1
		    DbgVec4 pSplitterPlane
    		;Evaluate the candidate SplitterFace to find out
    		;how nicely it would divide the input faces
    		xor ebx,ebx
    		mov NumPositive , ebx ;#faces in front of Plane
    		mov NumNegative , ebx ;#faces behind Plane
    		mov NumSpanning , ebx ;#faces Spanning the Plane
    		mov iTestFace	 ,ebx ;index of 'each face other than pSplitterFace'  
    		mov NumCoinciding,	1 ;#faces APON the Plane
           
    		
    		.while (ebx < NumFacesBeingProcessed)   ; for each face in FaceSet (except pSplitterFace)
    			.if ebx!=SplitterFaceIndex			
    				push ebx
    				
    				;Obtain pointer to current input Face
    				mov pTestFace, $OCall (pFacesIn::DataCollection.ItemAt, ebx)
    
    				;Evaluate the current input Face against the current candidate SplitterFace
    				mov edx,[eax].Face.hV0
    				add edx,[esi].Vertices.pBuffer
    				mov ebx,[eax].Face.hV1
    				add ebx,[esi].Vertices.pBuffer
    				mov eax,[eax].Face.hV2
    				add eax,[esi].Vertices.pBuffer
    				;Evaluate the current input Face against the current candidate SplitterFace
    				invoke ClassifyTrianglePlane,edx,ebx,eax,pSplitterPlane
    				.if 	eax == FRONTFRONTFRONT \
    				 	 || eax == FRONTFRONTCOPLANAR	|| eax == FRONTCOPLANARFRONT || eax==FRONTCOPLANARCOPLANAR\
    				 	 || eax == COPLANARFRONTFRONT	|| eax == COPLANARFRONTCOPLANAR || eax==COPLANARCOPLANARFRONT
    					inc NumPositive 
    				;	DbgText "front"
    				.elseif eax == BACKBACKBACK\
    				 	 || eax == BACKBACKCOPLANAR		|| eax == BACKCOPLANARBACK || eax==BACKCOPLANARCOPLANAR\
    					 || eax == COPLANARBACKBACK		|| eax == COPLANARBACKCOPLANAR || eax==COPLANARCOPLANARBACK
    					inc NumNegative 
    				;	DbgText "back"
    				.elseif eax == FRONTFRONTBACK || eax==FRONTBACKFRONT || eax == BACKBACKFRONT	\
    					 || eax == FRONTBACKBACK || eax == BACKFRONTFRONT  || eax == BACKFRONTBACK	\
    					 || eax == FRONTBACKCOPLANAR || eax== FRONTCOPLANARBACK\
    					 || eax == BACKFRONTCOPLANAR		|| eax== BACKCOPLANARFRONT \
    					 || eax == COPLANARFRONTBACK	|| eax == COPLANARBACKFRONT
    					inc NumSpanning 
    				;	DbgText "spanning"
    				.elseif eax==COPLANARCOPLANARCOPLANAR
    					inc NumCoinciding
    				;	DbgText "coplanar"
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
    			.else
    				;Hacky fix for special cases such as just two coplanar faces
    				jmp Acceptable
    			.endif        	
    			invoke ExitProcess,0
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
                 
                    
    		;If the current Splitter has less faces spanning it than our Best Splitter,
    		;OR, if it creates no MORE splits AND is better balanced,
    		;then we WILL accept this Splitter as the best candidate.
            mov eax,NumSpanning                 
            mov ebx,fBestRelation
            .if (eax < LeastSplits)
                m2m LeastSplits , NumSpanning,edx
                jmp Acceptable
            .elseif eax == LeastSplits && $IsGreater(fRelation,fBestRelation)
                .if $IsGreater(fRelation,fBestRelation)
    Acceptable:
    			m2m pBestFace,pSplitterFace,edx                
                m2m fBestRelation , fRelation,edx
                .if pResultOut==NULL
    	           	DbgWarning "ERROR - RETURNPTR PARAM IS NULL"
                   	invoke ExitProcess,0
                .endif
                mov eax,pResultOut
                m2m [eax].BestResult.pbestPlane, pSplitterPlane                    
                m2m [eax].BestResult.bestPositive,NumPositive
                m2m [eax].BestResult.bestNegative,NumNegative
                m2m [eax].BestResult.bestSpanning,NumSpanning 
                m2m [eax].BestResult.bestCoinciding,NumCoinciding 
                DbgHex pSplitterFace
                DbgDec NumPositive
                DbgDec NumNegative
                DbgDec NumSpanning
                DbgDec NumCoinciding
                DbgFloat fRelation
                 .endif
            .endif
       .endif
@@:
       inc SplitterFaceIndex
       mov ebx,SplitterFaceIndex  ; <--- update ebx for loop control
    .endw
         
    ;Decrease the number least acceptable relation by dividing it with a predefined constant.
    fld fMinRelation 
    fmul MINRELATIONSCALE
    fstp fMinRelation 
    
    .if $IsLess(fMinRelation, fEpsilon)
        DbgWarning "doh - minrelation got too small"
        jmp @F
    .endif
.endw

;DbgWarning "Best SplitterFace was decided"
;DbgHex pBestFace
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
.elseif [eax].BestResult.bestPositive!=0 && [eax].BestResult.bestNegative==0 && [eax].BestResult.bestSpanning==0
	DbgWarning "All Convex on Front side"
.else
	DbgWarning "Check the results, not sure WHY, maybe two parallel and concave faces?"
	mov eax,pResultOut
    DbgDec [eax].BestResult.bestPositive
    DbgDec [eax].BestResult.bestNegative
    DbgDec [eax].BestResult.bestSpanning 
    DbgDec [eax].BestResult.bestCoinciding 
    mov eax,pFacesIn
    DbgDec [eax].DataCollection.dCount
    xor ebx,ebx
    .while ebx<[eax].DataCollection.dCount
        OCall pFacesIn::DataCollection.ItemAt,ebx
        DbgVec4 [eax].Face.pPlane
        inc ebx
        mov eax,pFacesIn
    .endw
    mov edx,1
.endif
mov eax, NONE
MethodEnd

;Measure the Bounds of the set of faces that was input to the given Node
Method BSPGen.ProbeBounds,uses esi ebx edi,pBSPNode
LOCAL halflength:Vec3
	SetObject esi
	mov ebx,pBSPNode
    mov edi,$OCall ([ebx].BSPNode.pFaces::DataCollection.ItemAt,NULL)
	OCall [esi].Vertices::D3D_VertexBuffer.PtrFromHandle,[edi].Face.hV0 
	fld  [eax].Vec3.x
	fst  [ebx].BSPNode.vMin.x
	fstp [ebx].BSPNode.vMax.x
	fld  [eax].Vec3.y
	fst  [ebx].BSPNode.vMin.y
	fstp [ebx].BSPNode.vMax.y
	fld  [eax].Vec3.z
	fst  [ebx].BSPNode.vMin.z
	fstp [ebx].BSPNode.vMax.z
	
	;Calculate BoundingBox
	mov edi,[ebx].BSPNode.pFaces
	DbgDec [edi].DataCollection.dCount,"#triangles input to Node"
	mov ecx,1
	.while ecx<[edi].DataCollection.dCount
		push ecx
		
		shl ecx,2
		add ecx,[edi].DataCollection.pItems
		mov ecx,[ecx]
		
		push ecx
		OCall [esi].Vertices::D3D_VertexBuffer.PtrFromHandle,[ecx].Face.hV0
		fMin [eax].Vec3.x, [ebx].BSPNode.vMin.x
		fstp [ebx].BSPNode.vMin.x
		fMin [eax].Vec3.y, [ebx].BSPNode.vMin.y
		fstp [ebx].BSPNode.vMin.y
		fMin [eax].Vec3.z, [ebx].BSPNode.vMin.z
		fstp [ebx].BSPNode.vMin.z	
		fMax [eax].Vec3.x, [ebx].BSPNode.vMax.x
		fstp [ebx].BSPNode.vMax.x
		fMax [eax].Vec3.y, [ebx].BSPNode.vMax.y
		fstp [ebx].BSPNode.vMax.y
		fMax [eax].Vec3.z, [ebx].BSPNode.vMax.z
		fstp [ebx].BSPNode.vMax.z
		
        pop ecx
        push ecx
		OCall [esi].Vertices::D3D_VertexBuffer.PtrFromHandle,[ecx].Face.hV1
		fMin [eax].Vec3.x, [ebx].BSPNode.vMin.x
		fstp [ebx].BSPNode.vMin.x
		fMin [eax].Vec3.y, [ebx].BSPNode.vMin.y
		fstp [ebx].BSPNode.vMin.y
		fMin [eax].Vec3.z, [ebx].BSPNode.vMin.z
		fstp [ebx].BSPNode.vMin.z	
		fMax [eax].Vec3.x, [ebx].BSPNode.vMax.x
		fstp [ebx].BSPNode.vMax.x
		fMax [eax].Vec3.y, [ebx].BSPNode.vMax.y
		fstp [ebx].BSPNode.vMax.y
		fMax [eax].Vec3.z, [ebx].BSPNode.vMax.z
		fstp [ebx].BSPNode.vMax.z
		
		pop ecx
		OCall [esi].Vertices::D3D_VertexBuffer.PtrFromHandle,[ecx].Face.hV2
		fMin [eax].Vec3.x, [ebx].BSPNode.vMin.x
		fstp [ebx].BSPNode.vMin.x
		fMin [eax].Vec3.y, [ebx].BSPNode.vMin.y
		fstp [ebx].BSPNode.vMin.y
		fMin [eax].Vec3.z, [ebx].BSPNode.vMin.z
		fstp [ebx].BSPNode.vMin.z	
		fMax [eax].Vec3.x, [ebx].BSPNode.vMax.x
		fstp [ebx].BSPNode.vMax.x
		fMax [eax].Vec3.y, [ebx].BSPNode.vMax.y
		fstp [ebx].BSPNode.vMax.y
		fMax [eax].Vec3.z, [ebx].BSPNode.vMax.z
		fstp [ebx].BSPNode.vMax.z
		

		
		pop ecx
		inc ecx
	.endw
	
	DbgVec3 [ebx].BSPNode.vMin,"BoundingBox"
	DbgVec3 [ebx].BSPNode.vMax
	

	;calculate origin
	fld [ebx].BSPNode.vMax.x
	fsub [ebx].BSPNode.vMin.x
	fmul r4_half
	fst halflength.x
	fadd [ebx].BSPNode.vMin.x
	fstp [ebx].BSPNode.vOrigin.x
	
	fld [ebx].BSPNode.vMax.y
	fsub [ebx].BSPNode.vMin.y
	fmul r4_half
	fst halflength.y
	fadd [ebx].BSPNode.vMin.y
	fstp [ebx].BSPNode.vOrigin.y

	fld [ebx].BSPNode.vMax.z
	fsub [ebx].BSPNode.vMin.z
	fmul r4_half
	fst halflength.z
	fadd [ebx].BSPNode.vMin.z
	fstp [ebx].BSPNode.vOrigin.z

    ;estimate radius
	fld halflength.x
	fmul st(0),st(0)
	fld halflength.y
	fmul st(0),st(0)
	fadd
	fld halflength.z
	fmul st(0),st(0)
	fadd
	fsqrt
	fstp [ebx].BSPNode.fRadius

MethodEnd

;Allocate a new Face struct and fill with the given data
;Returns eax = pFace
Method BSPGen.New_Face,uses esi,hv0,hv1,hv2,pPlane,dAttrId
    SetObject esi
	MemAlloc sizeof Face,MEM_INIT_ZERO
	m2m [eax].Face.hV0 ,hv0                             ,edx
	m2m [eax].Face.hV1 ,hv1                             ,edx
	m2m [eax].Face.hV2 ,hv2                             ,edx
	m2m [eax].Face.pPlane,  pPlane                      ,edx
	m2m [eax].Face.dAttrId, dAttrId                     ,edx
MethodEnd

;Allocate a new PortalTriangle struct and fill with the given data
;Returns eax = pPortalTriangle
Method BSPGen.New_PortalTriangle,uses esi,pv0,pv1,pv2
    SetObject esi
	MemAlloc sizeof PortalTriangle,MEM_INIT_ZERO
	mov edx,pv0
	Vec3_Copy [eax].PortalTriangle.v0,[edx].Vec3
	mov edx,pv1
	Vec3_Copy [eax].PortalTriangle.v1,[edx].Vec3
	mov edx,pv2
	Vec3_Copy [eax].PortalTriangle.v2,[edx].Vec3
MethodEnd

;Allocate a new PortalQuad struct and fill with the given data
;Returns eax = pPortalQuad
Method BSPGen.New_PortalQuad,uses esi,pv0,pv1,pv2,pv3
    SetObject esi
	MemAlloc sizeof PortalQuad,MEM_INIT_ZERO
	mov edx,pv0
	Vec3_Copy [eax].PortalQuad.v0,[edx].Vec3
	mov edx,pv1
	Vec3_Copy [eax].PortalQuad.v1,[edx].Vec3
	mov edx,pv2
	Vec3_Copy [eax].PortalQuad.v2,[edx].Vec3
	mov edx,pv3
	Vec3_Copy [eax].PortalQuad.v3,[edx].Vec3
MethodEnd

;Generate a big flat rectangle (2 triangles)
;apon the SplittingPlane of the given BSP tree node,
;and large enough in size to separate the node's (child) geometry.
;Returns ptr to Portal struct
Method BSPGen.CreatePortal,uses esi edi ebx,pBSPNode
LOCAL v0:Vec3
LOCAL v1:Vec3
LOCAL v2:Vec3
LOCAL v3:Vec3
LOCAL fx,fy,fz

    ;Generate vertices for Large Planar Rectangle
    ;by examining the Plane for its Major Axes
    SetObject esi
    
    DbgWarning "Creating large portal"

    mov edi,pBSPNode
    mov edx,[edi].BSPNode.pSplitterPlane
    .if edx==0
        DbgWarning "Error - BSPNode has null Plane in CreateLargePortal"
    .endif
    Vec3_Load [edx].Vec3
    fabs
    fstp fz
    fabs
    fstp fy
    fabs
    fstp fx

    .if $IsGreaterOrEqual(fx,fy)
        .if $IsGreaterOrEqual(fx,fz)
            DbgText "Plane points mostly into X" 	    
            m2m v0.y , [edi].BSPNode.vMin.y, edx
            m2m v0.z , [edi].BSPNode.vMax.z, edx
            m2m v1.y , [edi].BSPNode.vMin.y, edx
            m2m v1.z , [edi].BSPNode.vMin.z, edx
            m2m v2.y , [edi].BSPNode.vMax.y, edx
            m2m v2.z , [edi].BSPNode.vMin.z, edx
            m2m v3.y , [edi].BSPNode.vMax.y, edx
            m2m v3.z , [edi].BSPNode.vMax.z, edx

            mov edx,[edi].BSPNode.pSplitterPlane
            fld [edx].Vec4.y 
            fmul [edi].BSPNode.vMin.y
            fld [edx].Vec4.z
            fmul [edi].BSPNode.vMax.z
            fadd
            fadd [edx].Vec4.w
            fdiv [edx].Vec4.x
            fchs
            fstp v0.x
            
            fld [edx].Vec4.y 
            fmul [edi].BSPNode.vMin.y
            fld [edx].Vec4.z
            fmul [edi].BSPNode.vMin.z
            fadd
            fadd [edx].Vec4.w
            fdiv [edx].Vec4.x
            fchs
            fstp v1.x
            
            fld [edx].Vec4.y 
            fmul [edi].BSPNode.vMax.y
            fld [edx].Vec4.z
            fmul [edi].BSPNode.vMin.z
            fadd
            fadd [edx].Vec4.w
            fdiv [edx].Vec4.x
            fchs
            fstp v2.x
            
            fld [edx].Vec4.y 
            fmul [edi].BSPNode.vMax.y
            fld [edx].Vec4.z
            fmul [edi].BSPNode.vMax.z
            fadd
            fadd [edx].Vec4.w
            fdiv [edx].Vec4.x
            fchs
            fstp v3.x
            jmp @F
        .endif
    .endif



    .if ($IsGreaterOrEqual(fy,fz))

        DbgText "Plane points mostly into Y"
        
        DbgVec3 [edi].BSPNode.vMin
        DbgVec3 [edi].BSPNode.vMax
        
        m2m v0.x , [edi].BSPNode.vMin.x, edx
        m2m v0.z , [edi].BSPNode.vMin.z, edx
        m2m v1.x , [edi].BSPNode.vMax.x, edx
        m2m v1.z , [edi].BSPNode.vMin.z, edx
        m2m v2.x , [edi].BSPNode.vMax.x, edx
        m2m v2.z , [edi].BSPNode.vMax.z, edx
        m2m v3.x , [edi].BSPNode.vMin.x, edx
        m2m v3.z , [edi].BSPNode.vMax.z, edx
    
        mov edx,[edi].BSPNode.pSplitterPlane
        fld [edx].Vec4.x
        fmul [edi].BSPNode.vMin.x
        fld [edx].Vec4.z
        fmul [edi].BSPNode.vMax.z
        fadd
        fadd [edx].Vec4.w
        fdiv [edx].Vec4.y
        fchs
        fstp v0.y
        
        fld [edx].Vec4.x
        fmul [edi].BSPNode.vMax.x
        fld [edx].Vec4.z
        fmul [edi].BSPNode.vMax.z
        fadd
        fadd [edx].Vec4.w
        fdiv [edx].Vec4.y
        fchs
        fstp v1.y
                
        fld [edx].Vec4.x
        fmul [edi].BSPNode.vMax.x
        fld [edx].Vec4.z
        fmul [edi].BSPNode.vMin.z
        fadd
        fadd [edx].Vec4.w
        fdiv [edx].Vec4.y
        fchs
        fstp v2.y
    
        fld [edx].Vec4.x
        fmul [edi].BSPNode.vMin.x
        fld [edx].Vec4.z
        fmul [edi].BSPNode.vMin.z
        fadd
        fadd [edx].Vec4.w
        fdiv [edx].Vec4.y
        fchs
        fstp v3.y
        jmp @F

    .endif


    DbgText "Plane points mostly into Z"
    m2m v0.y , [edi].BSPNode.vMin.x, edx
    m2m v0.z , [edi].BSPNode.vMin.y, edx
    m2m v1.y , [edi].BSPNode.vMax.x, edx
    m2m v1.z , [edi].BSPNode.vMin.y, edx
    m2m v2.y , [edi].BSPNode.vMax.x, edx
    m2m v2.z , [edi].BSPNode.vMax.y, edx
    m2m v3.y , [edi].BSPNode.vMin.x, edx
    m2m v3.z , [edi].BSPNode.vMax.x, edx
    
    mov edx,[edi].BSPNode.pSplitterPlane
    fld [edx].Vec4.x
    fmul [edi].BSPNode.vMin.x
    fld [edx].Vec4.y
    fmul [edi].BSPNode.vMin.y
    fadd
    fadd [edx].Vec4.w
    fdiv [edx].Vec4.z
    fchs
    fstp v0.z
    
    fld [edx].Vec4.x
    fmul [edi].BSPNode.vMax.x
    fld [edx].Vec4.y
    fmul [edi].BSPNode.vMin.y
    fadd
    fadd [edx].Vec4.w
    fdiv [edx].Vec4.z
    fchs
    fstp v1.z  

    fld [edx].Vec4.x
    fmul [edi].BSPNode.vMax.x
    fld [edx].Vec4.y
    fmul [edi].BSPNode.vMax.y
    fadd
    fadd [edx].Vec4.w
    fdiv [edx].Vec4.z
    fchs
    fstp v2.z
            
    fld [edx].Vec4.x
    fmul [edi].BSPNode.vMin.x
    fld [edx].Vec4.y
    fmul [edi].BSPNode.vMax.y
    fadd
    fadd [edx].Vec4.w
    fdiv [edx].Vec4.z
    fchs
    fstp v3.z
 
@@:
    ;Create a container struct
    mov edi,$MemAlloc(sizeof Portal,MEM_INIT_ZERO)
    ;Create lists for portal fragments
    mov [edi].Portal.pTriangles,$New(DataCollection,Init,esi,16,256,-1)
    mov [edi].Portal.pQuads,    $New(DataCollection,Init,esi,16,256,-1)
    
    ;Construct our Large Rectangle (two frags)
    OCall [edi].Portal.pQuads::DataCollection.Insert, $OCall (esi.New_PortalQuad,addr v0,addr v1,addr v2,addr v3)
    
    DbgVec3 v0
    DbgVec3 v1
    DbgVec3 v2
    DbgVec3 v3

    
    mov eax,edi
MethodEnd

;Divide the given node's faces into child lists
;Inputs are:
;pFaces = DataCollection of input Faces
;pPlane = plane used to classfy/cut faces
;poutFront = DataCollection of output Faces on the Front side of Plane
;poutBack  = DataCollection of output Faces on the Back side of Plane
Method BSPGen.DivideFacesWithPlane ,uses esi ebx, pFaces,pPlane,poutFront,poutBack
	SetObject esi
	mov ebx,pFaces
	;Divide the input Faces into Front, Back and Coplanar collections
	.while [ebx].DataCollection.dCount!=0
		;Fetch Nth Face from the input set
		OCall pFaces::DataCollection.DeleteAt,0	
        OCall esi.CutFaceWithPlane,eax,pPlane,poutFront,poutBack
	.endw
	;Trash the input face list, since we now have two potential output lists
	Destroy pFaces
MethodEnd

;Split the given Portal's geometry against the given Plane.
;Inputs are:
;pPortal = input Portal
;pPlane = cutting plane
Method BSPGen.DividePortalWithPlane ,uses esi ebx, pPortal,pPlane
LOCAL pouts,pouts2
	SetObject esi
	
	DbgText "DividePortalWithPlane"
	mov pouts,$New(DataCollection,Init,esi,16,256,-1)
	mov pouts2,$New(DataCollection,Init,esi,16,256,-1)

	mov ebx,pPortal
	.if [ebx].Portal.pTriangles==0
	    DbgWarning "Portal contains No TriangleList"
	.else
    	mov ebx,[ebx].Portal.pTriangles
    	DbgDec [ebx].DataCollection.dCount,"#tri frags"
    	.while [ebx].DataCollection.dCount!=0
    		OCall ebx::DataCollection.DeleteAt,0
    		DbgHex eax	
            OCall esi.CutPortalTriangleWithPlane,eax,pPlane,pouts,pouts
            .if eax==FALSE
                DbgWarning "Need coplanar tri cut"
                int 3
            .endif
    	.endw
    	Destroy ebx
	.endif
	
	mov ebx,pPortal
	.if [ebx].Portal.pQuads==0
	    DbgWarning "Portal contains No QuadList"
	.else
    	mov ebx,[ebx].Portal.pQuads
    	DbgDec [ebx].DataCollection.dCount,"#quad frags"
    	.while [ebx].DataCollection.dCount!=0
    		OCall ebx::DataCollection.DeleteAt,0	
            OCall esi.CutPortalQuadWithPlane,eax,pPlane,pouts,pouts,pouts2,pouts2
            .if eax==FALSE
                DbgWarning "Need coplanar quad cut"
                int 3
            .endif
    	.endw
    	Destroy ebx
	.endif
	
	mov ebx,pPortal
	m2m [ebx].Portal.pQuads,pouts2,edx
	m2m [ebx].Portal.pTriangles,pouts,edx
	
MethodEnd

;Clip the given Portal's geometry against the given Plane,
;rejecting all geometry found to be on the "back" side of Plane.
;Inputs are:
;pPortal = input Portal
;pPlane = cutting plane
Method BSPGen.ClipPortalToPlaneFront ,uses esi ebx, pPortal,pPlane
LOCAL pouts,pouts2
	SetObject esi
	
	DbgText "ClipPortalToPlaneFront"
	mov pouts,$New(DataCollection,Init,esi,16,256,-1)
	mov pouts2,$New(DataCollection,Init,esi,16,256,-1)

	mov ebx,pPortal
	.if [ebx].Portal.pTriangles==0
	    DbgWarning "Portal contains No TriangleList"
	.else
    	mov ebx,[ebx].Portal.pTriangles
    	.while [ebx].DataCollection.dCount!=0
    		OCall ebx::DataCollection.DeleteAt,0
    		DbgHex eax	
            OCall esi.CutPortalTriangleWithPlane,eax,pPlane,pouts,NULL
            .if eax==FALSE
                DbgWarning "Need coplanar tri cut in ClipPortalToPlaneFront"
                int 3
            .endif
    	.endw
    	Destroy ebx
	.endif
	
	mov ebx,pPortal
	.if [ebx].Portal.pQuads==0
	    DbgWarning "Portal contains No QuadList"
	.else
    	mov ebx,[ebx].Portal.pQuads
    	.while [ebx].DataCollection.dCount!=0
    		OCall ebx::DataCollection.DeleteAt,0	
            OCall esi.CutPortalQuadWithPlane,eax,pPlane,pouts,NULL,pouts2,NULL
            .if eax==FALSE
                DbgWarning "Need coplanar quad cut in ClipPortalToPlaneFront"
                int 3
            .endif
    	.endw
    	Destroy ebx
	.endif
	
	mov ebx,pPortal
	m2m [ebx].Portal.pQuads,pouts2,edx
	m2m [ebx].Portal.pTriangles,pouts,edx
	
MethodEnd

;Slice the input Face with the given 'cutting plane'.
;Returns the resulting Face fragments in the Front and Back output lists.
Method BSPGen.CutFaceWithPlane ,uses esi ebx, pFace,pPlane,poutFront,poutBack
LOCAL ftemp
LOCAL dStyle
LOCAL pvA,pvB,pvC
LOCAL pNewVertex1,pNewVertex2
LOCAL hNewVertex1,hNewVertex2
LOCAL fScaling:real4,fLength1:real4,fLength2:real4


    SetObject esi
    
    ;Examine the Triangle / Plane to predict the outcome of triangle slicing
    mov eax,pFace
    mov edx,[eax].Face.hV0
    add edx,[esi].Vertices.pBuffer
    mov pvA,edx
    mov ebx,[eax].Face.hV1
    add ebx,[esi].Vertices.pBuffer
    mov pvB,edx
    mov eax,[eax].Face.hV2
    add eax,[esi].Vertices.pBuffer
    mov pvC,edx
	mov dStyle,$invoke (ClassifyTrianglePlane,edx,ebx,eax,pPlane)
	DbgDec dStyle
	mov ebx,pFace
	.if eax==COPLANAR || eax==NONE
	    ;Triangles which are coplanar with the cutting plane
	    ;will be sent to whichever childnode they face towards
	    ;so that we tend to build clusters of inward-pointing faces
		.if ChooseSide (pFace, pPlane)==FRONT
			OCall poutFront::DataCollection.Insert,pFace
		.else
		    OCall poutBack::DataCollection.Insert,pFace
		.endif

	.elseif eax == FRONTFRONTFRONT \
		 || eax == FRONTFRONTCOPLANAR	|| eax == FRONTCOPLANARFRONT || eax==FRONTCOPLANARCOPLANAR\
		 || eax == COPLANARFRONTFRONT	|| eax == COPLANARFRONTCOPLANAR || eax==COPLANARCOPLANARFRONT
		;Simple case - triangle is totally in front of cutting plane
		OCall poutFront::DataCollection.Insert, pFace	
		
	.elseif eax == BACKBACKBACK\
		 || eax == BACKBACKCOPLANAR		|| eax == BACKCOPLANARBACK || eax==BACKCOPLANARCOPLANAR\
		 || eax == COPLANARBACKBACK		|| eax == COPLANARBACKCOPLANAR || eax==COPLANARCOPLANARBACK
		;Simple case - triangle is totally behind the cutting plane
		OCall poutBack::DataCollection.Insert,pFace

	.else
	    ;Cases where the cutting plane passes through the triangle,
	    ;which require that we Split the triangle into 2 or 3 pieces
	    
;	b       c                    Pretty ascii art showing each Split case...
;                 FRONT          Plane's FRONT is always "UP"
;=====1===2================             
;	              BACK
;	    a
		.if eax==BACKFRONTFRONT
            ;Allocate two new vertices
            NewFaceVertex2
            ;Calculate two points of intersection (our two new vertices)
			TwoEdgesIntersect pvA,pvB,pvA,pvC 
            ;Build the three new fragments and store in appropriate collection
            OCall esi.New_Face,[ebx].Face.hV0,hNewVertex1,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV2,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax                           	
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV1,[ebx].Face.hV2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax   

;  c        a
        
;   2     1
        
;      b
		.elseif eax==FRONTBACKFRONT
		    NewFaceVertex2
			TwoEdgesIntersect pvB,pvA,pvB,pvC 
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV1,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax
            OCall esi.New_Face,[ebx].Face.hV0,hNewVertex1,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax                
            OCall esi.New_Face,[ebx].Face.hV0,hNewVertex2,[ebx].Face.hV2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax   

;a        b

;  1    2

;    c
		.elseif eax==FRONTFRONTBACK
		    NewFaceVertex2
			TwoEdgesIntersect pvC,pvA,pvC,pvB 
            OCall esi.New_Face,[ebx].Face.hV0,hNewVertex2,hNewVertex1,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax
            OCall esi.New_Face,[ebx].Face.hV0,[ebx].Face.hV1,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax                
            OCall esi.New_Face,hNewVertex2,[ebx].Face.hV2,hNewVertex1,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax      

;    a

;  2   1

;c       b
		.elseif eax==FRONTBACKBACK
		    NewFaceVertex2
			TwoEdgesIntersect pvA,pvB,pvA,pvC
            OCall esi.New_Face,[ebx].Face.hV0,hNewVertex1,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV1,[ebx].Face.hV2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax                
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV2,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax   

;      b

;    1   2

;  a       c
		.elseif eax==BACKFRONTBACK
		    NewFaceVertex2
			TwoEdgesIntersect pvA,pvB,pvC,pvB
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV1,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax
            OCall esi.New_Face,[ebx].Face.hV0,hNewVertex1,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax                
            OCall esi.New_Face,[ebx].Face.hV0,hNewVertex2,[ebx].Face.hV2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax 

;     c

;   2  1

;  b     a		    
		.elseif eax==BACKBACKFRONT
		    NewFaceVertex2
			TwoEdgesIntersect pvA,pvC,pvB,pvC
            OCall esi.New_Face,hNewVertex1,hNewVertex2,[ebx].Face.hV2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax
            OCall esi.New_Face,[ebx].Face.hV0,hNewVertex2,hNewVertex1,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax                
            OCall esi.New_Face,[ebx].Face.hV0,[ebx].Face.hV1,hNewVertex2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax 

;       c

; b     1

;       a
		.elseif eax==BACKCOPLANARFRONT
		    NewFaceVertex1
			OneEdgeIntersect pNewVertex1,pvA,pvC		    
            OCall esi.New_Face,[ebx].Face.hV0,[ebx].Face.hV1,hNewVertex1,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV1,[ebx].Face.hV2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax 

;        b

; a      1

;        c
		.elseif eax==COPLANARFRONTBACK
		    NewFaceVertex1
			OneEdgeIntersect pNewVertex1,pvC,pvB
            OCall esi.New_Face,[ebx].Face.hV0,[ebx].Face.hV1,hNewVertex1,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax 
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV2,[ebx].Face.hV0,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax

;        a 

;  c     1
  
;        b		    
		.elseif eax==FRONTBACKCOPLANAR
		    NewFaceVertex1
			OneEdgeIntersect pNewVertex1,pvB,pvA
            OCall esi.New_Face,[ebx].Face.hV2,[ebx].Face.hV0,hNewVertex1,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax 
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV1,[ebx].Face.hV2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax

; b

; 1    c

; a		    
		.elseif eax==BACKFRONTCOPLANAR
		    NewFaceVertex1
			OneEdgeIntersect pNewVertex1,pvA,pvB
            OCall esi.New_Face,[ebx].Face.hV2,[ebx].Face.hV0,hNewVertex1,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV1,[ebx].Face.hV2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax 

; a

; 1   b

; c
		.elseif eax==FRONTCOPLANARBACK
		    NewFaceVertex1
			OneEdgeIntersect pNewVertex1,pvC,pvA
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV1,[ebx].Face.hV2,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV0,[ebx].Face.hV1,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax 

;  c

;  1    a

;  b
		.elseif eax==COPLANARBACKFRONT
		    NewFaceVertex1
			OneEdgeIntersect pNewVertex1,pvB,pvC
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV0,[ebx].Face.hV1,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutBack::DataCollection.Insert,eax
            OCall esi.New_Face,hNewVertex1,[ebx].Face.hV2,[ebx].Face.hV0,[ebx].Face.pPlane,[ebx].Face.dAttrId
            OCall poutFront::DataCollection.Insert,eax 

		.else
		    DbgText "UNHANDLED SPLIT TYPE"
		    DbgDec eax
		    int 3
		.endif
		
		;Since the input face was Split, we can trash it.
		MemFree pFace		

	.endif
MethodEnd

;Slice the input PortalTriangle with a 'cutting plane'.
;Returns the resulting PortalTriangle fragments in the Front and Back output lists.
;Supports Front-Clip operations (NULL back list).
;Returns TRUE (success) or FALSE (failed due to special case of coplanarity)
Method BSPGen.CutPortalTriangleWithPlane ,uses esi ebx, pPortalTriangle,pPlane,poutFront,poutBack
LOCAL dStyle
LOCAL pvA,pvB,pvC
LOCAL intersect1:Vec3,intersect2:Vec3
    SetObject esi
        
    mov eax,pPortalTriangle
    lea edx,[eax].PortalTriangle.v0
    mov pvA,edx
    lea edx,[eax].PortalTriangle.v1
    mov pvB,edx
    lea edx,[eax].PortalTriangle.v2
    mov pvC,edx
	mov dStyle,$invoke (ClassifyTrianglePlane,pvA,pvB,pvC,pPlane)
	DbgDec dStyle
	mov ebx,pPortalTriangle
	.if eax==COPLANAR || eax==NONE
	    .if poutBack!=0
		    OCall poutFront::DataCollection.Insert, pPortalTriangle
        .else
		    ;Fragment and CuttingFace are coplanar.
		    ;We can't use this Plane to cut the fragment.
            return FALSE

        .endif
        
	.elseif eax == FRONTFRONTFRONT \
		 || eax == FRONTFRONTCOPLANAR	|| eax == FRONTCOPLANARFRONT || eax==FRONTCOPLANARCOPLANAR\
		 || eax == COPLANARFRONTFRONT	|| eax == COPLANARFRONTCOPLANAR || eax==COPLANARCOPLANARFRONT
		OCall poutFront::DataCollection.Insert, pPortalTriangle	
		
	.elseif eax == BACKBACKBACK\
		 || eax == BACKBACKCOPLANAR		|| eax == BACKCOPLANARBACK || eax==BACKCOPLANARCOPLANAR\
		 || eax == COPLANARBACKBACK		|| eax == COPLANARBACKCOPLANAR || eax==COPLANARCOPLANARBACK
		 .if poutBack!=0
		    OCall poutBack::DataCollection.Insert,pPortalTriangle
        .else
            MemFree pPortalTriangle
        .endif
	.else

		.if eax==BACKFRONTFRONT
		    lea edx,intersect1
            OneEdgeIntersect edx,pvA,pvB
            lea edx,intersect2
			OneEdgeIntersect edx,pvA,pvC
			.if poutBack!=0
                OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect1,addr intersect2
                OCall poutBack::DataCollection.Insert,eax
            .endif
            OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v2,addr intersect2
            OCall poutFront::DataCollection.Insert,eax                           	
            OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v1,addr [ebx].PortalTriangle.v2
            OCall poutFront::DataCollection.Insert,eax   

		.elseif eax==FRONTBACKFRONT 
		    lea edx,intersect1
			OneEdgeIntersect edx,pvB,pvA
			lea edx,intersect2
			OneEdgeIntersect edx,pvB,pvC 
			.if poutBack!=0
                OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v1,addr intersect2
                OCall poutBack::DataCollection.Insert,eax
            .endif
            OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect1,addr intersect2
            OCall poutFront::DataCollection.Insert,eax                
            OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect2,addr [ebx].PortalTriangle.v2
            OCall poutFront::DataCollection.Insert,eax   

		.elseif eax==FRONTFRONTBACK
		    lea edx,intersect1
		    OneEdgeIntersect edx,pvC,pvA
		    lea edx,intersect2
		    OneEdgeIntersect edx,pvC,pvB
            OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect2,addr intersect1
            OCall poutFront::DataCollection.Insert,eax
            OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr [ebx].PortalTriangle.v1,addr intersect2
            OCall poutFront::DataCollection.Insert,eax
            .if poutBack!=0                
                OCall esi.New_PortalTriangle,addr intersect2,addr [ebx].PortalTriangle.v2,addr intersect1
                OCall poutBack::DataCollection.Insert,eax      
            .endif
		.elseif eax==FRONTBACKBACK
		    lea edx,intersect1
		    OneEdgeIntersect edx,pvA,pvB
		    lea edx,intersect2
		    OneEdgeIntersect edx,pvA,pvC
            OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect1,addr intersect2
            OCall poutFront::DataCollection.Insert,eax
            .if poutBack!=0
                OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v1,addr [ebx].PortalTriangle.v2
                OCall poutBack::DataCollection.Insert,eax                
                OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v2,addr intersect2
                OCall poutBack::DataCollection.Insert,eax   
            .endif
		.elseif eax==BACKFRONTBACK
		    lea edx,intersect1
			OneEdgeIntersect edx,pvA,pvB
			lea edx,intersect2
			OneEdgeIntersect edx,pvC,pvB
            OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v1,addr intersect2
            OCall poutFront::DataCollection.Insert,eax
            .if poutBack!=0
                OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect1,addr intersect2
                OCall poutBack::DataCollection.Insert,eax                
                OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect2,addr [ebx].PortalTriangle.v2
                OCall poutBack::DataCollection.Insert,eax 
	        .endif
		.elseif eax==BACKBACKFRONT
		    lea edx,intersect1
			OneEdgeIntersect edx,pvA,pvC
			lea edx,intersect2
			OneEdgeIntersect edx,pvB,pvC
            OCall esi.New_PortalTriangle,addr intersect1,addr intersect2,addr [ebx].PortalTriangle.v2
            OCall poutFront::DataCollection.Insert,eax
            .if poutBack!=0
                OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect2,addr intersect1
                OCall poutBack::DataCollection.Insert,eax                
                OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr [ebx].PortalTriangle.v1,addr intersect2
                OCall poutBack::DataCollection.Insert,eax 
            .endif
		.elseif eax==BACKCOPLANARFRONT
		    lea edx,intersect1
			OneEdgeIntersect edx,pvA,pvC
			.if poutBack!=0
                OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr [ebx].PortalTriangle.v1,addr intersect1
                OCall poutBack::DataCollection.Insert,eax
            .endif
            OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v1,addr [ebx].PortalTriangle.v2
            OCall poutFront::DataCollection.Insert,eax 

		.elseif eax==COPLANARFRONTBACK
		    lea edx,intersect1
			OneEdgeIntersect edx,pvC,pvB
            OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr [ebx].PortalTriangle.v1,addr intersect1
            OCall poutFront::DataCollection.Insert,eax 
            .if poutBack!=0
                OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v2,addr [ebx].PortalTriangle.v0
                OCall poutBack::DataCollection.Insert,eax
		    .endif
		    
		.elseif eax==FRONTBACKCOPLANAR
		    lea edx,intersect1
			OneEdgeIntersect edx,pvB,pvA
            OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v2,addr [ebx].PortalTriangle.v0,addr intersect1
            OCall poutFront::DataCollection.Insert,eax 
            .if poutBack!=0
                OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v1,addr [ebx].PortalTriangle.v2
                OCall poutBack::DataCollection.Insert,eax
            .endif
            
		.elseif eax==BACKFRONTCOPLANAR
		    lea edx,intersect1
			OneEdgeIntersect edx,pvA,pvB
			.if poutBack!=0
                OCall esi.New_PortalTriangle,addr [ebx].PortalTriangle.v2,addr [ebx].PortalTriangle.v0,addr intersect1
                OCall poutBack::DataCollection.Insert,eax
            .endif
            OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v1,addr [ebx].PortalTriangle.v2
            OCall poutFront::DataCollection.Insert,eax 

		.elseif eax==FRONTCOPLANARBACK
		    lea edx,intersect1
			OneEdgeIntersect edx,pvC,pvA
			.if poutBack!=0
                OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v1,addr [ebx].PortalTriangle.v2
                OCall poutBack::DataCollection.Insert,eax
            .endif
            OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v0,addr [ebx].PortalTriangle.v1
            OCall poutFront::DataCollection.Insert,eax 

		.elseif eax==COPLANARBACKFRONT
		    lea edx,intersect1
			OneEdgeIntersect edx,pvB,pvC
			.if poutBack!=0
                OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v0,addr [ebx].PortalTriangle.v1
                OCall poutBack::DataCollection.Insert,eax
            .endif
            OCall esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalTriangle.v2,addr [ebx].PortalTriangle.v0
            OCall poutFront::DataCollection.Insert,eax 

		.else
		    DbgText "UNHANDLED SPLIT TYPE"
		    DbgDec eax
		    int 3
		.endif
		
		;Since the input face was Split, we can trash it.
		MemFree pPortalTriangle		

	.endif
	mov eax,TRUE
MethodEnd

;Slice the input PortalQuad with a 'cutting plane'.
;Returns the resulting PortalQuad fragments in the Front and Back output lists.
;Supports Front-Clip operations (NULL back list).
;Returns TRUE (success) or FALSE (failed due to special case of coplanarity)
Method BSPGen.CutPortalQuadWithPlane ,uses esi ebx, pPortalQuad,pPlane,pFrontTris,pBackTris,pFrontQuads,pBackQuads
LOCAL dStyle
LOCAL pvA,pvB,pvC,pvD
LOCAL intersect1:Vec3,intersect2:Vec3
    SetObject esi
    
    DbgText "CutPortalQuadWithPlane"
    
    mov eax,pPortalQuad
    lea edx,[eax].PortalQuad.v0
    mov pvA,edx
    lea edx,[eax].PortalQuad.v1
    mov pvB,edx
    lea edx,[eax].PortalQuad.v2
    mov pvC,edx
    lea edx,[eax].PortalQuad.v3
    mov pvD,edx
    
	mov dStyle,$invoke (ClassifyQuadPlane,pvA,pvB,pvC,pvD,pPlane)
	DbgDec dStyle
	mov ebx,pPortalQuad
	.switch eax
	.case COPLANAR,NONE
	    .if pBackQuads!=0
	        DbgWarning "Coplanar quad = shoving to front"
		    OCall pFrontQuads::DataCollection.Insert, pPortalQuad
        .else
		    ;Fragment and CuttingFace are coplanar.
		    ;We can't use this Plane to cut the fragment.
            return FALSE
        .endif
	.case  FRONTFRONTFRONTFRONT, FRONTFRONTFRONTCOPLANAR, FRONTFRONTCOPLANARFRONT,FRONTCOPLANARFRONTFRONT,COPLANARFRONTFRONTFRONT
	    DbgText "Front quad"
		OCall pFrontQuads::DataCollection.Insert, pPortalQuad	
		
    .case  COPLANARCOPLANARFRONTFRONT,FRONTCOPLANARCOPLANARFRONT,FRONTFRONTCOPLANARCOPLANAR,COPLANARFRONTFRONTCOPLANAR
        DbgText "Front quad"
		OCall pFrontQuads::DataCollection.Insert, pPortalQuad	
		
	.case BACKBACKBACKBACK, BACKBACKBACKCOPLANAR, BACKBACKCOPLANARBACK, BACKCOPLANARBACKBACK,COPLANARBACKBACKBACK	      
		.if pBackQuads!=0
		    OCall pBackQuads::DataCollection.Insert,pPortalQuad
		    DbgText "Back quad"
        .else
            MemFree pPortalQuad
		.endif
		
    .case  COPLANARCOPLANARBACKBACK,BACKCOPLANARCOPLANARBACK,BACKBACKCOPLANARCOPLANAR,COPLANARBACKBACKCOPLANAR
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,pPortalQuad
            DbgText "Back quad"
        .else
            MemFree pPortalQuad
        .endif
;A /    B
; /
;/
;
;D      C
	.case BACKFRONTFRONTFRONT
	DbgText "Front quad, Front tri, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvA,pvB
        lea edx,intersect2
	    OneEdgeIntersect edx,pvA,pvD
	    .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect1,addr intersect2)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr [ebx].PortalTriangle.v1,addr [ebx].PortalTriangle.v2,addr intersect2)
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect2,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        MemFree pPortalQuad
	.case FRONTBACKBACKBACK
	DbgText "Front tri, Back tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvB,pvA
        lea edx,intersect2
	    OneEdgeIntersect edx,pvD,pvA
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalTriangle.v0,addr intersect1,addr intersect2)
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect2,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr [ebx].PortalTriangle.v1,addr [ebx].PortalTriangle.v2,addr intersect2)
        .endif
        MemFree pPortalQuad
        
;A   /  B
;   /
;  /
; /
;D      C
	.case BACKFRONTFRONTCOPLANAR 
	    DbgText "Front quad, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvA,pvB
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v0,addr intersect1,addr [ebx].PortalQuad.v3)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        MemFree pPortalQuad
        
    .case FRONTBACKBACKCOPLANAR
    DbgText "Front tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvB,pvA
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v0,addr intersect1,addr [ebx].PortalQuad.v3)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        MemFree pPortalQuad
        
        
;A    B
;    /
;  /
;/
; 
;D    C

	.case BACKCOPLANARFRONTFRONT
	    DbgText "Front quad,Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvA,pvD
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect1)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        MemFree pPortalQuad
    .case FRONTCOPLANARBACKBACK
    DbgText "Front tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvD,pvA
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect1)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        MemFree pPortalQuad

;A    B
;    /
;   /
;  /
; / 
;D    C
    .case BACKCOPLANARFRONTCOPLANAR
    DbgText "Front tri, Back tri"
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v3)
        .endif
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        MemFree pPortalQuad
    .case FRONTCOPLANARBACKCOPLANAR
    DbgText "Front tri, Back tri"
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v3)
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        MemFree pPortalQuad
        
;A     B
;     /    
;    /
;   / 
;D /   C
    .case BACKCOPLANARFRONTBACK
    DbgText "Front tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvD,pvC
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect1)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect1,addr [ebx].PortalQuad.v3)
        .endif
        MemFree pPortalQuad

    .case FRONTCOPLANARBACKFRONT
    DbgText "Front quad, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvC,pvD
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect1)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect1,addr [ebx].PortalQuad.v3)
        MemFree pPortalQuad
    
;A     B
;
;      /
;     /    
;   /
; / 
;D     C
    .case BACKBACKFRONTCOPLANAR
    DbgText "Front tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvB,pvC
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect1,addr [ebx].PortalQuad.v3)
        .endif
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        MemFree pPortalQuad
    .case FRONTFRONTBACKCOPLANAR
    DbgText "Front quad, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvC,pvB
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect1,addr [ebx].PortalQuad.v3)
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        MemFree pPortalQuad

    
;A     B
;      
;      /    
;     /
;    /
;D  /  C
    .case BACKBACKBACKFRONT
    DbgText "Front tri, Back tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvB,pvC
		lea edx,intersect2
        OneEdgeIntersect edx,pvD,pvC
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr intersect2,addr [ebx].PortalQuad.v3)
        .endif
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect1)
        .endif
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalQuad.v2,addr intersect2)
        MemFree pPortalQuad
    .case FRONTFRONTFRONTBACK
    DbgText "Front quad, Front tri, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvC,pvB
		lea edx,intersect2
        OneEdgeIntersect edx,pvC,pvD
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr intersect2,addr [ebx].PortalQuad.v3)
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect1)
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalQuad.v2,addr intersect2)
        .endif
        MemFree pPortalQuad

;A \ B
;   \
;    \
;     
;D   C
    .case FRONTBACKFRONTFRONT
    DbgText "Front quad, Front tri, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvB,pvA
		lea edx,intersect2
        OneEdgeIntersect edx,pvB,pvC
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr intersect2,addr [ebx].PortalQuad.v3)
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect2, addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalQuad.v1,addr intersect2)
        .endif
        MemFree pPortalQuad
    .case BACKFRONTBACKBACK
    DbgText "Front tri, Back tri, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvA,pvB
		lea edx,intersect2
        OneEdgeIntersect edx,pvC,pvB
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr intersect2,addr [ebx].PortalQuad.v3)
        .endif
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect2, addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1,addr [ebx].PortalQuad.v1,addr intersect2)
        MemFree pPortalQuad

;A \  B
;   \
;    \     
;D    C
    .case FRONTBACKCOPLANARFRONT
    DbgText "Front quad,  Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvB,pvA
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1, addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        MemFree pPortalQuad
    .case BACKFRONTCOPLANARBACK
    DbgText "Front tri,  Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvA,pvB
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1, addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        MemFree pPortalQuad

;A      B
;
;\   
;  \  
;    \
;      \
;D      C
    .case BACKBACKCOPLANARFRONT
    DbgText "Front tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvA,pvD
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1, addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect1)
        .endif
        MemFree pPortalQuad
    .case FRONTFRONTCOPLANARBACK
    DbgText "Front quad, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvD,pvA
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1, addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect1)
        MemFree pPortalQuad


;A      B
; \
;   \
;     \ 
;       \
;
;D      C 
    .case COPLANARBACKFRONTFRONT
    DbgText "Front quad,Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvB,pvC
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1, addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        MemFree pPortalQuad
    .case COPLANARFRONTBACKBACK
    DbgText "Front tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvC,pvB
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1, addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        MemFree pPortalQuad

;A      B
; \
;  \
;   \  
;D   \  C
    .case COPLANARBACKBACKFRONT
    DbgText "Front tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvC,pvD
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1, addr [ebx].PortalQuad.v3,addr [ebx].PortalQuad.v0)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect1)
        .endif
        MemFree pPortalQuad
    .case COPLANARFRONTFRONTBACK
    DbgText "Front quad, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvD,pvC
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,addr intersect1, addr [ebx].PortalQuad.v3,addr [ebx].PortalQuad.v0)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect1)
        MemFree pPortalQuad
    
;A   B
; \
;  \
;   \  
;D   C
    .case COPLANARBACKCOPLANARFRONT
    DbgText "Front tri,  Back tri"
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,  addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2)
        .endif
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle, addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        MemFree pPortalQuad
    .case COPLANARFRONTCOPLANARBACK
    DbgText "Front tri,  Back tri"
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,  addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2)
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle, addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        MemFree pPortalQuad

;A     B
;
;\
; \
;  \  
;D  \  C
    .case BACKBACKBACKFRONT
    DbgText "Front tri,  Back tri, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvA,pvD
		lea edx,intersect2
        OneEdgeIntersect edx,pvC,pvD
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,  addr intersect1,addr intersect2,addr [ebx].PortalQuad.v3)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect2)
        .endif
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,  addr [ebx].PortalQuad.v0,addr intersect2,addr intersect1)
        .endif
        MemFree pPortalQuad
    .case FRONTFRONTFRONTBACK
     DbgText "Front quad,  Front tri, Back tri"
		lea edx,intersect1
        OneEdgeIntersect edx,pvD,pvA
		lea edx,intersect2
        OneEdgeIntersect edx,pvD,pvC
        .if pBackTris!=0
            OCall pBackTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,  addr intersect1,addr intersect2,addr [ebx].PortalQuad.v3)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect2)
        OCall pFrontTris::DataCollection.Insert,$OCall (esi.New_PortalTriangle,  addr [ebx].PortalQuad.v0,addr intersect2,addr intersect1)
        MemFree pPortalQuad

;A      B
;
;--------
;
;D      C
    .case BACKBACKFRONTFRONT
     DbgText "Front quad, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvA,pvD
		lea edx,intersect2
        OneEdgeIntersect edx,pvB,pvC
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr intersect2,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect2,addr intersect1)
        .endif
        MemFree pPortalQuad
    .case FRONTFRONTBACKBACK
    DbgText "Front quad, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvD,pvA
		lea edx,intersect2
        OneEdgeIntersect edx,pvC,pvB
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr intersect2,addr [ebx].PortalQuad.v2,addr [ebx].PortalQuad.v3)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr [ebx].PortalQuad.v1,addr intersect2,addr intersect1)
        MemFree pPortalQuad        
;A  |   B
;   |
;   |
;D  |   C
    .case BACKFRONTFRONTBACK
    DbgText "Front quad, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvA,pvB
		lea edx,intersect2
        OneEdgeIntersect edx,pvD,pvC
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr intersect2,addr [ebx].PortalQuad.v3)
        .endif
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect2)
        MemFree pPortalQuad
    .case FRONTBACKBACKFRONT
    DbgText "Front quad, Back quad"
		lea edx,intersect1
        OneEdgeIntersect edx,pvB,pvA
		lea edx,intersect2
        OneEdgeIntersect edx,pvC,pvD
        OCall pFrontQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr [ebx].PortalQuad.v0,addr intersect1,addr intersect2,addr [ebx].PortalQuad.v3)
        .if pBackQuads!=0
            OCall pBackQuads::DataCollection.Insert,$OCall (esi.New_PortalQuad,addr intersect1,addr [ebx].PortalQuad.v1,addr [ebx].PortalQuad.v2,addr intersect2)
        .endif
        MemFree pPortalQuad

	.default
		    DbgText "UNHANDLED SPLIT TYPE"
		    DbgDec eax
		    int 3
	.endsw
		
    mov eax,TRUE
MethodEnd

;For each leaf, identify and eliminate all the obvious 'fake' portal fragments.
Method BSPGen.PruneFakePortals,uses ebx esi edi
LOCAL pfake,pBSPNode
    SetObject esi
    DbgWarning "Pruning Fake portals"
    xor ebx,ebx
    .while ebx<[esi].Leaves.dCount
        push ebx
        
        mov pBSPNode,$OCall ([esi].Leaves::DataCollection.ItemAt,ebx)
        
        .if [eax].BSPNode.pPortalTriangles!=0
            mov edi, [eax].BSPNode.pPortalTriangles
            xor ebx,ebx
            .while ebx<[edi].DwordCollection.dCount
                mov pfake,$OCall (edi::DwordCollection.ItemAt,ebx)
                DbgHex eax,"frag"
                
                .if ($OCall(esi.IsFakePortalTriangle,eax,pBSPNode))==TRUE
                    DbgWarning "FAKE portal triangle"
                    ;Remove fragment from leaf
                    OCall edi::DwordCollection.Delete,pfake
                    MemFree pfake
    
                .else
                    DbgWarning "REAL portal triangle"              
                    inc ebx
                .endif
            .endw
        .endif
        
        mov eax,pBSPNode
        .if [eax].BSPNode.pPortalQuads!=0
            mov edi, [eax].BSPNode.pPortalQuads
            xor ebx,ebx
            .while ebx<[edi].DwordCollection.dCount
                mov pfake,$OCall (edi::DwordCollection.ItemAt,ebx)
                DbgHex eax,"frag"
                
                .if ($OCall(esi.IsFakePortalQuad,eax,pBSPNode))==TRUE
                    DbgWarning "FAKE portal quad"
                    ;Remove fragment from leaf
                    OCall edi::DwordCollection.Delete,pfake
                    MemFree pfake
    
                .else
                    DbgWarning "REAL portal quad"              
                    inc ebx
                .endif
            .endw
        .endif
        pop ebx
        inc ebx
    .endw
MethodEnd

;Determine if the given PortalTriangle is 'real' or 'fake'.
;Real PortalTriangles exist in two Leaf nodes,
;whereas the fake fragments only exist in one leaf.
;pPortalTriangle -> input PortalTriangle
;pOrigNode -> Leaf known to contain the input PortalTriangle
Method BSPGen.IsFakePortalTriangle,uses ebx esi edi,pPortalTriangle,pOrigNode
LOCAL pBSPNode
    SetObject esi
    xor ebx,ebx
    .while ebx<[esi].Leaves.dCount
        mov pBSPNode,$OCall ([esi].Leaves::DataCollection.ItemAt,ebx)
        .if eax!=pOrigNode && [eax].BSPNode.pPortalTriangles!=0
            OCall [eax].BSPNode.pPortalTriangles::DwordCollection.IndexOf,pPortalTriangle
            .if eax!=-1
                ;Record this fragment in output Portal
                OCall esi.FoundRealPortalTriangle,pOrigNode,pBSPNode,pPortalTriangle
                return FALSE
            .endif
        .endif
        inc ebx
    .endw
    mov eax,TRUE
MethodEnd

;Determine if the given PortalQuad is 'real' or 'fake'.
;Real PortalQuads exist in two Leaf nodes,
;whereas the fake fragments only exist in one leaf.
;pPortalQuad -> input PortalQuad
;pOrigNode -> Leaf known to contain the input PortalQuad
Method BSPGen.IsFakePortalQuad,uses ebx esi edi,pPortalQuad,pOrigNode
LOCAL pBSPNode
    SetObject esi
    xor ebx,ebx
    .while ebx<[esi].Leaves.dCount
        mov pBSPNode,$OCall ([esi].Leaves::DataCollection.ItemAt,ebx)
        .if eax!=pOrigNode && [eax].BSPNode.pPortalQuads!=0
            OCall [eax].BSPNode.pPortalQuads::DwordCollection.IndexOf,pPortalQuad
            .if eax!=-1
                OCall esi.FoundRealPortalQuad,pOrigNode,pBSPNode,pPortalQuad
                return FALSE
            .endif
        .endif
        inc ebx
    .endw
    mov eax,TRUE 
MethodEnd

;Add a PortalTriangle to an output Portal.
;This method is used to group the 'real PortalTriangles'
;according to the pair of leaves connected by them.
;The PortalTriangles are moved into an output Portal struct,
;and references to Portals are collected in the Leaves.
Method BSPGen.FoundRealPortalTriangle,uses esi ebx edi,pBSPNodeA,pBSPNodeB,pPortalTriangle
    SetObject esi
    ;Search for a Portal which connects the two given leafnodes
    xor ebx,ebx
    .while ebx < [esi].Portals.dCount
        OCall [esi].Portals::DataCollection.ItemAt,ebx
        mov edx,pBSPNodeA
        mov edi,pBSPNodeB
        .if [eax].Portal.pNodeA==edx && [eax].Portal.pNodeB==edi
            .break
        .elseif [eax].Portal.pNodeA==edi && [eax].Portal.pNodeB==edx
            .break
        .endif
        inc ebx
    .endw
    
    .if ebx==[esi].Portals.dCount
        DbgWarning "Adding fragment to NEW Portal"
        
        push $New (DataCollection,Init,esi,0,256,-1)
        OCall [esi].Portals::DataCollection.Insert,$MemAlloc(sizeof Portal,MEM_INIT_ZERO)
        mov ebx,pPortalTriangle
        m2m [eax].Portal.pPlane,[ebx].Face.pPlane,edx
        m2m [eax].Portal.pNodeA,pBSPNodeA,edx
        m2m [eax].Portal.pNodeB,pBSPNodeB,edx
        pop [eax].Portal.pTriangles
        
        ;Mark Leaf A with Portal reference
        push eax
        mov ebx,pBSPNodeA
        .if [ebx].BSPNode.pPortals==0
            mov [ebx].BSPNode.pPortals,$New(DwordCollection,Init,0,16,256,-1)
        .endif
        pop eax
        push eax
        OCall [ebx].BSPNode.pPortals::DwordCollection.Insert,eax
        push eax
        
        ;Mark Leaf B with Portal reference
        mov ebx,pBSPNodeB
        .if [ebx].BSPNode.pPortals==0
            mov [ebx].BSPNode.pPortals,$New(DwordCollection,Init,0,16,256,-1)
        .endif
        pop eax
        push eax
        OCall [ebx].BSPNode.pPortals::DwordCollection.Insert,eax
        
        pop eax

    .else
        DbgWarning "Adding fragment to Existing Portal"
    .endif
    
    OCall [eax].Portal.pTriangles::DataCollection.Insert,pPortalTriangle
MethodEnd

;Add a PortalQuad to an output Portal.
;This method is used to group the 'real PortalQuads'
;according to the pair of leaves connected by them.
;The PortalQuads are moved into an output Portal struct,
;and references to Portals are collected in the Leaves.
Method BSPGen.FoundRealPortalQuad,uses esi ebx edi,pBSPNodeA,pBSPNodeB,pPortalQuad
    SetObject esi
    ;Search for a Portal which connects the two given leafnodes
    xor ebx,ebx
    .while ebx < [esi].Portals.dCount
        OCall [esi].Portals::DataCollection.ItemAt,ebx
        mov edx,pBSPNodeA
        mov edi,pBSPNodeB
        .if [eax].Portal.pNodeA==edx && [eax].Portal.pNodeB==edi
            .break
        .elseif [eax].Portal.pNodeA==edi && [eax].Portal.pNodeB==edx
            .break
        .endif
        inc ebx
    .endw
    
    .if ebx==[esi].Portals.dCount
        DbgWarning "Adding fragment to NEW Portal"
        
        push $New (DataCollection,Init,esi,0,256,-1)
        OCall [esi].Portals::DataCollection.Insert,$MemAlloc(sizeof Portal,MEM_INIT_ZERO)
        mov ebx,pPortalQuad
        m2m [eax].Portal.pPlane,[ebx].Face.pPlane,edx
        m2m [eax].Portal.pNodeA,pBSPNodeA,edx
        m2m [eax].Portal.pNodeB,pBSPNodeB,edx
        pop [eax].Portal.pQuads
        
        ;Mark Leaf A with Portal reference
        push eax
        mov ebx,pBSPNodeA
        .if [ebx].BSPNode.pPortals==0
            mov [ebx].BSPNode.pPortals,$New(DwordCollection,Init,0,16,256,-1)
        .endif
        pop eax
        push eax
        OCall [ebx].BSPNode.pPortals::DwordCollection.Insert,eax
        push eax
        
        ;Mark Leaf B with Portal reference
        mov ebx,pBSPNodeB
        .if [ebx].BSPNode.pPortals==0
            mov [ebx].BSPNode.pPortals,$New(DwordCollection,Init,0,16,256,-1)
        .endif
        pop eax
        push eax
        OCall [ebx].BSPNode.pPortals::DwordCollection.Insert,eax
        
        pop eax

    .else
        DbgWarning "Adding fragment to Existing Portal"
    .endif
    
    OCall [eax].Portal.pQuads::DataCollection.Insert,pPortalQuad
MethodEnd

;Front-Clip the geometry of each Portal to the Face geometry 
;which is stored in the two Leaves connected by it.
Method BSPGen.ClipPortalsToLeaves,uses esi edi ebx
    SetObject esi
    DbgWarning "Clipping portals"
    xor ebx,ebx
    .while ebx<[esi].Portals.dCount
        mov edi,$OCall ([esi].Portals::DataCollection.ItemAt,ebx)
        ;Clip the Portal fragments against the Faces in both Leaves
        OCall esi.ClipPortalToLeaf,[edi].Portal.pNodeA,edi      
        OCall esi.ClipPortalToLeaf,[edi].Portal.pNodeB,edi
        DbgDec [eax].DataCollection.dCount
        inc ebx
    .endw
    DbgWarning "Portal clipped"
MethodEnd

;Front-Clip the input fragments against all faces in given node.
;(only keep the stuff in front of faceplanes)
;Special case - fragments are clipped against edgeplanes of coplanar triangles
Method BSPGen.ClipPortalToLeaf,uses esi edi ebx,pBSPNode,pPortal
	SetObject esi
	DbgText "clip clip clip"
	mov ebx,pBSPNode
	mov edi,[ebx].BSPNode.pFaces
	DbgDec [edi].DataCollection.dCount,"#faces to clip against"
	xor ebx,ebx
	.while ebx<[edi].DataCollection.dCount
        OCall edi::DataCollection.ItemAt,ebx
        DbgVec4 [eax].Face.pPlane
     	OCall esi.ClipPortalToPlaneFront,pPortal,[eax].Face.pPlane
        inc ebx
    .endw
    mov eax,pPortal
    mov eax,[eax].Portal.pTriangles
    DbgDec [eax].DataCollection.dCount,"#triangle frags survived"
    mov eax,pPortal
    mov eax,[eax].Portal.pQuads
    DbgDec [eax].DataCollection.dCount,"#quad frags survived"
MethodEnd

endif
