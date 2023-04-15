;The Portal object is a container for Polygons.
;Portals are constructed apon a Plane (initially as a rectangular polygon).
;Thereafter, the geometry of a Portal (its Polygons) may be Split, Clipped, etc.
PortalID equ 34532
Object Portal, PortalID, Collection
    RedefineMethod Init,Pointer,Pointer,Pointer,Pointer
    DefineVariable pPlane, Pointer, NULL
ObjectEnd

;Construct a large flat rectangle on the given Plane,
;and within the given WorldSpace bounds.
;Returns nothing.
Method Portal.Init,uses esi ebx,pOwner,pConstructionPlane,pvMin,pvMax
LOCAL v0:Vec3
LOCAL v1:Vec3
LOCAL v2:Vec3
LOCAL v3:Vec3
LOCAL fx,fy,fz

    SetObject esi
    ACall Init,pOwner,16,256,-1
    
    m2m [esi].pPlane,pConstructionPlane,edx

    ;Generate vertices for Large Planar Rectangle
    ;by examining the Plane for its Major Axes
    Vec3_Load [edx].Vec3
    fabs
    fstp fz
    fabs
    fstp fy
    fabs
    fstp fx


    .if $IsGreaterOrEqual(fx,fy)
        .if $IsGreaterOrEqual(fx,fz)
   
            mov ebx,pvMax
            mov eax,pvMin
            m2m v0.y , [ebx].Vec3.y, edx
            m2m v0.z , [eax].Vec3.z, edx
            m2m v1.y , [ebx].Vec3.y, edx
            m2m v1.z , [ebx].Vec3.z, edx
            m2m v2.y , [eax].Vec3.y, edx
            m2m v2.z , [ebx].Vec3.z, edx
            m2m v3.y , [eax].Vec3.y, edx
            m2m v3.z , [eax].Vec3.z, edx

            mov edx,pConstructionPlane
            fld [edx].Vec4.y 
            fmul [ebx].Vec3.y
            fld [edx].Vec4.z
            fmul [eax].Vec3.z
            fadd
            fadd [edx].Vec4.w
            fdiv [edx].Vec4.x
            fchs
            fstp v0.x
            
            fld [edx].Vec4.y 
            fmul [ebx].Vec3.y
            fld [edx].Vec4.z
            fmul [ebx].Vec3.z
            fadd
            fadd [edx].Vec4.w
            fdiv [edx].Vec4.x
            fchs
            fstp v1.x
            
            fld [edx].Vec4.y 
            fmul [eax].Vec3.y
            fld [edx].Vec4.z
            fmul [ebx].Vec3.z
            fadd
            fadd [edx].Vec4.w
            fdiv [edx].Vec4.x
            fchs
            fstp v2.x
            
            fld [edx].Vec4.y 
            fmul [eax].Vec3.y
            fld [edx].Vec4.z
            fmul [eax].Vec3.z
            fadd
            fadd [edx].Vec4.w
            fdiv [edx].Vec4.x
            fchs
            fstp v3.x
            
            .ifBitSet [edx].Vec3.x,BIT31
                DbgText "Plane points mostly into -X" 
            .else
                DbgText "Plane points mostly into +X" 
                push v0.y
                push v1.y
                push v2.y
                push v3.y
                pop v0.y
                pop v1.y
                pop v2.y
                pop v3.y
                
            .endif	 
            
            DbgVec3 v0
            DbgVec3 v1
            DbgVec3 v2
            DbgVec3 v3

            jmp @F
        .endif
    .endif



    .if ($IsGreaterOrEqual(fy,fz))

        mov ebx,pvMax
        mov eax,pvMin
        m2m v0.x , [eax].Vec3.x, edx
        m2m v0.z , [ebx].Vec3.z, edx
        m2m v1.x , [ebx].Vec3.x, edx
        m2m v1.z , [ebx].Vec3.z, edx
        m2m v2.x , [ebx].Vec3.x, edx
        m2m v2.z , [eax].Vec3.z, edx
        m2m v3.x , [eax].Vec3.x, edx
        m2m v3.z , [eax].Vec3.z, edx
    
        mov edx,pConstructionPlane
        fld [edx].Vec4.x
        fmul [eax].Vec3.x
        fld [edx].Vec4.z
        fmul [ebx].Vec3.z
        fadd
        fadd [edx].Vec4.w
        fdiv [edx].Vec4.y
        fchs
        fstp v0.y
        
        fld [edx].Vec4.x
        fmul [ebx].Vec3.x
        fld [edx].Vec4.z
        fmul [ebx].Vec3.z
        fadd
        fadd [edx].Vec4.w
        fdiv [edx].Vec4.y
        fchs
        fstp v1.y
                
        fld [edx].Vec4.x
        fmul [ebx].Vec3.x
        fld [edx].Vec4.z
        fmul [eax].Vec3.z
        fadd
        fadd [edx].Vec4.w
        fdiv [edx].Vec4.y
        fchs
        fstp v2.y
    
        fld [edx].Vec4.x
        fmul [eax].Vec3.x
        fld [edx].Vec4.z
        fmul [eax].Vec3.z
        fadd
        fadd [edx].Vec4.w
        fdiv [edx].Vec4.y
        fchs
        fstp v3.y
            .ifBitSet [edx].Vec3.y,BIT31
                DbgText "Plane points mostly into -Y" 
                push v0.z
                push v1.z
                push v2.z
                push v3.z
                pop v0.z
                pop v1.z
                pop v2.z
                pop v3.z
            .else
                DbgText "Plane points mostly into +Y" 
            .endif
       DbgVec3 v0
       DbgVec3 v1
       DbgVec3 v2
       DbgVec3 v3
        jmp @F

    .endif


    mov eax,pvMin
    mov ebx,pvMax
    m2m v0.x , [eax].Vec3.x, edx
    m2m v0.y , [ebx].Vec3.y, edx
    m2m v1.x , [ebx].Vec3.x, edx
    m2m v1.y , [ebx].Vec3.y, edx
    m2m v2.x , [ebx].Vec3.x, edx
    m2m v2.y , [eax].Vec3.y, edx
    m2m v3.x , [eax].Vec3.x, edx
    m2m v3.y , [eax].Vec3.y, edx
    
    mov edx,pConstructionPlane
    fld [edx].Vec4.x 
    fmul [eax].Vec3.x       
    fld [edx].Vec4.y
    fmul [ebx].Vec3.y
    fadd
    fadd [edx].Vec4.w
    fdiv [edx].Vec4.z
    fchs
    fstp v0.z
    
    fld [edx].Vec4.x
    fmul [ebx].Vec3.x
    fld [edx].Vec4.y
    fmul [ebx].Vec3.y
    fadd
    fadd [edx].Vec4.w
    fdiv [edx].Vec4.z
    fchs
    fstp v1.z  

    fld [edx].Vec4.x
    fmul [ebx].Vec3.x
    fld [edx].Vec4.y
    fmul [eax].Vec3.y
    fadd
    fadd [edx].Vec4.w
    fdiv [edx].Vec4.z
    fchs
    fstp v2.z
            
    fld [edx].Vec4.x
    fmul [eax].Vec3.x
    fld [edx].Vec4.y
    fmul [eax].Vec3.y
    fadd
    fadd [edx].Vec4.w
    fdiv [edx].Vec4.z
    fchs
    fstp v3.z
            .ifBitSet [edx].Vec3.z,BIT31
                DbgText "Plane points mostly into -Z" 
                push v0.x
                push v1.x
                push v2.x
                push v3.x
                pop v0.x
                pop v1.x
                pop v2.x
                pop v3.x
            .else
                DbgText "Plane points mostly into +Z" 
            .endif
    DbgVec3 v0
    DbgVec3 v1
    DbgVec3 v2
    DbgVec3 v3
    
@@: 
    ;Create a new Polygon and insert into this Portal
    mov ebx,$OCall (esi.Insert, $New(POLYGON,Init,esi,pConstructionPlane))
    ;Add four points to the new polygon
    OCall ebx::POLYGON.Insert,addr v0
    OCall ebx::POLYGON.Insert,addr v1
    OCall ebx::POLYGON.Insert,addr v2
    OCall ebx::POLYGON.Insert,addr v3


MethodEnd

