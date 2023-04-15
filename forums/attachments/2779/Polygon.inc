;This object represents a 3D POLYGON which:
;- is constructed from an arbitrary number of points
;- is constructed apon some 3D Plane (all points are coplanar)
;- is a Convex POLYGON

;Inline Macro to Calculate the intersection of Edge and Plane
OneEdgeIntersect macro intersectout,PointA,PointB

    ;Find intersections of edge AB with Plane
    push edx
    invoke D3DXPlaneIntersectLine, intersectout,pPlane,PointA,PointB
    pop edx
    .if eax==NULL
        DbgWarning "Error - No Intersection: Plane and Edge appear parallel"
        int 3
    .endif	    
    
    ;Debug - verify that calculated point is on the plane
    push edx
    invoke ClassifyPointPlane, intersectout,pPlane
    pop edx
    .if eax!=COPLANAR
        DbgWarning "Error - the calculated point of intersection failed coplanarity test"
        int 3
    .endif
endm

PolID equ 345345
Object POLYGON, PolID, DataCollection
    RedefineMethod Init,Pointer,Pointer
    RedefineMethod Insert,Pointer
    VirtualMethod  IsCorrectlyWound,Pointer,Pointer,Pointer
    VirtualMethod  Split,Pointer,Pointer,Pointer
    DefineVariable pPlane,Pointer,NULL
ObjectEnd

Method POLYGON.Init, uses esi, pOwner, pConstructionPlane
    SetObject esi
    m2m [esi].pPlane,pConstructionPlane,edx
    ACall Init,pOwner,16,256,-1
MethodEnd

;Add a new point to this (convex) polygon.
;Returns TRUE (Success) or FALSE (Failed)
Method POLYGON.Insert,uses esi,pVec3
LOCAL xx,yy,zz
    SetObject esi
    .if [esi].dCount>1
        ;Ensure that the polygon remains Convex
        mov edx,[esi].dCount
        sub edx,2
        mov eax,[esi].pItems
        OCall IsCorrectlyWound,dword ptr[eax+edx*4],dword ptr 4[eax+edx*4],pVec3
        .if eax==FALSE
            DbgWarning "POLYGON ERROR - Failed to Insert new point - Not Convex"
            DbgWarning "Could be a bug in Portal constructor"
            DbgVec3 pVec3
            ret
        .endif
    .endif
    ;Add new point to Polygon
    MemAlloc sizeof Vec3
    mov edx,pVec3
    Vec3_Copy [eax].Vec3,[edx].Vec3
    ACall Insert,eax
    mov eax,TRUE
MethodEnd

;This method determines whether the triangle
;formed by the three input points is wound
;in the same direction as this POLYGON
;(ie both wound in the CW or CCW direction)
;by calculating the direction of the triangle's 
;surfacenormal, and comparing it to the direction
;of the POLYGON's construction plane.
;REMARKS:
;Since this method relies on the POLYGON's
;construction plane being correct, it returns TRUE
;when the Winding Direction is the same as that 
;of the POLYGON, and otherwise returns FALSE.
;Therefore th
Method POLYGON.IsCorrectlyWound,uses esi,pv0,pv1,pv2
LOCAL e1:Vec3
LOCAL e2:Vec3
    SetObject esi
    
    mov eax,pv0
    mov edx,pv1
    Vec3_Sub [edx].Vec3,[eax].Vec3
    Vec3_Stow e1
    mov edx,pv2
    Vec3_Sub  [edx].Vec3,[eax].Vec3
    Vec3_Stow e2
    Vec3_Cross e1,e2
    Vec3_Stow e1
    
    ;Change -0 into +0 (so surfacenormal agrees with direct3d ones)
    .if e1.x==80000000h
        mov e1.x,0
    .endif
    .if e1.y==80000000h
        mov e1.y,0
    .endif
    .if e1.z==80000000h
        mov e1.z,0
    .endif
    
    mov edx,[esi].pPlane
    .ifBitSet e1.x,BIT31
        .ifBitClr [edx].Vec3.x,BIT31
            return FALSE
        .endif
    .else
        .ifBitSet[edx].Vec3.x,BIT31
            return FALSE
        .endif
    .endif
    .ifBitSet e1.y,BIT31
        .ifBitClr [edx].Vec3.y,BIT31
            return FALSE
        .endif
    .else
        .ifBitSet[edx].Vec3.y,BIT31
            return FALSE
        .endif
    .endif
    .ifBitSet e1.z,BIT31
        .ifBitClr [edx].Vec3.z,BIT31
            return FALSE
        .endif
    .else
        .ifBitSet[edx].Vec3.z,BIT31
            return FALSE
        .endif
    .endif
    mov eax,TRUE
MethodEnd

;Attempt to slice this (convex) POLYGON with the given Plane,
;returning either the original POLYGON, or two (convex) POLYGONs.
Method POLYGON.Split,uses esi, pPlane,pFront,pBack
LOCAL ptA, ptB
LOCAL sideA, sideB
LOCAL intersect:Vec3
  SetObject esi

  ;We begin with the LAST point
  mov eax,[esi].dCount
  dec eax
  mov ptA,$OCall (esi.ItemAt,eax)
  
  ;What side of the Plane is it on?
  mov sideA,$invoke (ClassifyPointPlane,ptA,pPlane)
  
  ;Walk through the connected Edges of the POLYGON.
  ;Beginning with the FIRST point
  xor ebx,ebx
  
  .while ebx<[esi].dCount
    ;Grab the NEXT point
    mov ptB,$OCall (esi.ItemAt,ebx)
    
    ;What side of the Plane is it on?
    mov sideB,$invoke (ClassifyPointPlane,ptB,pPlane)
    
    ;If the edge cross the plane from the Back side to the Front
    .if sideB ==FRONT 
      .if sideA ==BACK
        ;Calculate the intersection point (Edge/Plane)
        lea ebx,intersect
        OneEdgeIntersect ebx,ptA,ptB
        ;
        OCall pFront::POLYGON.Insert,  ebx
        OCall pBack::POLYGON.Insert,   ebx
      .endif
      OCall pFront::POLYGON.Insert, ptB

    .elseif sideB==BACK
      .if sideA ==FRONT
        lea ebx,intersect
        OneEdgeIntersect ebx, ptA,ptB
        OCall pFront::POLYGON.Insert,  ebx
        OCall pBack::POLYGON.Insert,   ebx
      .endif
      OCall pBack::POLYGON.Insert, ptB
    
    .else
      OCall pBack::POLYGON.Insert, ptB
      OCall pFront::POLYGON.Insert, ptB
      
    .endif
    m2m ptA, ptB,edx
    m2m sideA , sideB,edx
    inc ebx
  .endw
MethodEnd


