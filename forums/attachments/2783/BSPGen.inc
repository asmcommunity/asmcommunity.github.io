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
	pPortalPolygons  Pointer ?  ;Unprocessed list of portal polygons
	pPortals         Pointer ?
	
BSPNode ends

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
    push edx
    invoke ClassifyPointPlane, intersectout,pPlane
    pop edx
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
    VirtualMethod New_Face,                         dword,dword,dword,Pointer,dword
    VirtualMethod PrepareFaces,                     Pointer
    VirtualMethod ProbeBounds,                      Pointer
    VirtualMethod GenerateTree,                     Pointer
    VirtualMethod GenerateNode,                     Pointer
    VirtualMethod ChooseBestDividingPlane,          Pointer,Pointer
    VirtualMethod DivideFacesWithPlane,             Pointer,Pointer,Pointer,Pointer
    VirtualMethod CutFaceWithPlane,                 Pointer,Pointer,Pointer,Pointer
    ;VirtualMethod New_PortalTriangle,               Pointer,Pointer,Pointer
    ;VirtualMethod New_PortalQuad,                   Pointer,Pointer,Pointer,Pointer
    ;VirtualMethod DividePortalQuadListWithPlane,    Pointer,Pointer,Pointer,Pointer,Pointer,Pointer
    ;VirtualMethod DividePortalTriangleListWithPlane,Pointer,Pointer,Pointer,Pointer
    ;VirtualMethod CutPortalTriangleWithPlane,       Pointer,Pointer,Pointer,Pointer
    ;VirtualMethod CutPortalQuadWithPlane,           Pointer,Pointer,Pointer,Pointer,Pointer,Pointer
    VirtualMethod GeneratePortals,                  Pointer
    VirtualMethod ShatterPortal,                    Pointer
    VirtualMethod WalkPolygon,                      Pointer,Pointer
    VirtualMethod WalkPortal,                       Pointer
    ;VirtualMethod PruneFakePortals
    ;VirtualMethod IsFakePortalTriangle,             Pointer,Pointer
    ;VirtualMethod IsFakePortalQuad,                 Pointer,Pointer
    ;VirtualMethod ClipPortalToFace,                 Pointer,Pointer
    ;VirtualMethod ClipPortalToLeaf,                 Pointer,Pointer
    ;VirtualMethod ClipPortalsToLeaves
    ;VirtualMethod FoundRealPortalTriangle,          Pointer,Pointer,Pointer
    ;VirtualMethod FoundRealPortalQuad,              Pointer,Pointer,Pointer
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
    OCall [esi].Portals       ::DataCollection.Init,esi,512,4096,-1
    OCall [esi].Leaves        ::DwordCollection.Init,esi,512,4096,-1
MethodEnd

Method BSPGen.Done,uses esi
    SetObject esi
    OCall [esi].Leaves        ::DwordCollection.Done
    OCall [esi].Vertices      ::D3D_VertexBuffer.Done
    OCall [esi].Planes        ::Vec4Collection.Done
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
	;DbgLine
	;OCall esi.PruneFakePortals
	;DbgLine
	;OCall esi.ClipPortalsToLeaves
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
	DbgHex pBSPNode
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
	
	DbgVec3 [ebx].BSPNode.vMin
	DbgVec3 [ebx].BSPNode.vMax

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

;Walk the BSP Tree looking for NON LEAF nodes.
;Generate a large flat rectangle on the splittingplane of each nonleaf node.
;Cut the rectangle with all the other splitting planes.
;Send each resulting fragment through the tree until they hit a Leaf node.
;When this method returns, the Leaf nodes will contain all the Fragments.
Method BSPGen.GeneratePortals,uses esi ebx,pBSPNode
LOCAL pPortal
    SetObject esi
    mov ebx,pBSPNode
    .if [ebx].BSPNode.pFront==0 && [ebx].BSPNode.pBack==0
        ;DbgText "DiscoverPortals found a Leaf"
        
    .else
        ;Generate a large flat rectangle on the splittingplane 
        mov pPortal,$New (Portal,Init,esi,[ebx].BSPNode.pSplitterPlane,addr [ebx].BSPNode.vMin,addr [ebx].BSPNode.vMax)
        
        ;Cut the rectangle with all the other splitting planes.
        OCall esi.ShatterPortal,eax
        
        ;Send all its polygons through the tree to the leaf nodes
        OCall esi.WalkPortal,pPortal

        ;Trash the container, since its polygons are now in the leaves
        Destroy pPortal
        
        ;Recurse child nodes
        .if [ebx].BSPNode.pFront!=0
            OCall esi.GeneratePortals,[ebx].BSPNode.pFront
        .endif
        .if [ebx].BSPNode.pBack!=0
            OCall esi.GeneratePortals,[ebx].BSPNode.pBack
        .endif
    .endif
MethodEnd

;Cut a new portal rectangle against all the splittingplanes in the bsp tree
Method BSPGen.ShatterPortal,uses esi ebx,pPortal
    SetObject esi
    ;Cut the portal fragments against all Splitting Planes
    ;(except the Construction plane)
    xor ebx,ebx
    .while ebx<[esi].Planes.dCount
        OCall [esi].Planes::Vec4Collection.ItemAt,ebx
        mov edx,pPortal
        .if [edx].Portal.pPlane!=eax
            OCall pPortal::Portal.SliceWithPlane,eax
        .endif
        inc ebx
    .endw
MethodEnd

;Given a Portal which has been pre-sliced against all the splitting planes,
;walk its geometry through the Tree into the Leaf nodes.
Method BSPGen.WalkPortal,uses esi ebx,pPortal
    SetObject esi
    mov ebx,pPortal
    .while [ebx].Portal.dCount!=0    
        OCall esi.WalkPolygon,[esi].pRootNode,$OCall (ebx::Portal.DeleteAt,0)
    .endw
MethodEnd

;Recursively 'walk' a Portal polygon through the BSP Tree until it lands in a Leaf.
;Since we already 'pre-split' the portal against all splitting planes,
;we should never need to Split polygons within this method.
Method BSPGen.WalkPolygon,uses esi ebx,pBSPNode,pPolygon
    SetObject esi
    mov ebx,pBSPNode
    .if [ebx].BSPNode.pFront==0 && [ebx].BSPNode.pBack==0
        ;DbgWarning "WalkPolygon found a Leaf"
        .if [ebx].BSPNode.pPortalPolygons==0
            mov [ebx].BSPNode.pPortalPolygons,$New(DwordCollection,Init,esi,16,256,-1)            
        .endif
        OCall [ebx].BSPNode.pPortalPolygons::DwordCollection.Insert,pPolygon
    .else
        ;Classify the fragment against the plane..
        ;Remember, we expect the result to show the fragment
        ;clearly on one side of the plane or the other ... not 'split' by it,
        ;since we already 'pre-split' the fragments as much as possible.
        ;Since we already verified the geometry as we generated it,
        ;I am going to CHEAT here, and just classify the first three points.
        ;THERE BETTER BE AT LEAST THREE POINTS OR WE ARE NOT AMUSED!
        mov edx,pPolygon
        mov eax,[edx].POLYGON.pItems
        .switch ($invoke (ClassifyTrianglePlane, dword ptr[eax], dword ptr[eax+4], dword ptr[eax+8],[ebx].BSPNode.pSplitterPlane))
        .case BACKBACKBACK,BACKCOPLANARCOPLANAR,BACKCOPLANARBACK,BACKBACKCOPLANAR,COPLANARCOPLANARBACK,COPLANARBACKBACK,COPLANARBACKCOPLANAR
            ;Send portal fragment reference down the Back child
            OCall esi.WalkPolygon,[ebx].BSPNode.pBack,pPolygon
        .case FRONTFRONTFRONT,COPLANARCOPLANARFRONT,COPLANARFRONTCOPLANAR,FRONTCOPLANARCOPLANAR,COPLANARFRONTFRONT,FRONTCOPLANARFRONT,FRONTFRONTCOPLANAR
            ;Send portal fragment reference down the Front child
            OCall esi.WalkPolygon,[ebx].BSPNode.pFront,pPolygon
        .case COPLANAR
            ;Special case - send portal fragment reference down BOTH childs...
            ;We will seek the duplicated pointers later.
            OCall esi.WalkPolygon,[ebx].BSPNode.pFront,pPolygon
            OCall esi.WalkPolygon,[ebx].BSPNode.pBack,pPolygon
        .default
            DbgWarning "Error - No splits should occur during WalkPolygon"
            DbgDec eax,"erroneous split style"
            int 3
        .endsw
    .endif
MethodEnd

endif
