ifndef __Friction
    __Friction equ 1
    
.data

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

;Conditions under which to simulate Friction
CONDITIONS_UNDEFINED equ -1
CONDITIONS_DRY       equ 0
CONDITIONS_STICKY    equ 1
CONDITIONS_SLIPPERY  equ 2

;Defaults
MATERIALID_DEFAULT   equ MATERIALID_WOOD
CONDITIONS_DEFAULT   equ CONDITIONS_DRY





;Struct used to describe cells in our table of friction coefficients
MyTableElement struct
	_Static  real4 ?
	_Dynamic real4 ?
MyTableElement ends

;Struct used to describe rows of cells in our table 
MyTableRow struct
	Wood   MyTableElement <>
	Stone  MyTableElement <>
	Ice    MyTableElement <>
	Glass  MyTableElement <>
	Metal  MyTableElement <>
	Rubber MyTableElement <>
	Velcro MyTableElement <>
	Meat   MyTableElement <>
MyTableRow ends

;--------------------------------------------------------------------------------------------------------------------------------------
;				WOOD			STONE			ICE			GLASS			METAL			RUBBER			VELCRO			MEAT
;--------------------------------------------------------------------------------------------------------------------------------------
;Material Density is to be looked up by the geometry-class init methods
DensityTable real4 0.75f,		0.0f,			0.8f,		2.0f,			8.0f,			1.2f,			0.9f,			1.26f

FrictionTable LABEL Pointer
;-------------------
;NORMAL CONDITIONS
;-------------------
;WOOD
MyTableRow	{<0.6f,0.5f>,	<0.5f,0.4f>,	<0.2f,0.1f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;STONE
MyTableRow	{<0.5f,0.4f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<1.0f,0.8f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;ICE
MyTableRow	{<0.2f,0.1f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.1f,0.03f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;GLASS
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.1f,0.03f>,	<0.95f,0.4f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;METAL
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.6f,0.4f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;RUBBER
MyTableRow	{<0.0f,0.0f>,	<1.0f, 0.8f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;VELCRO
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<6.0f,4.0f>,	<0.0f,0.0f>}
;MEAT
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}

FrictionTableSticky LABEL Pointer
;-------------------
;STICKY CONDITIONS
;-------------------
;WOOD
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;STONE
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<1.5f,1.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;ICE
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;GLASS
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;METAL
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;RUBBER
MyTableRow	{<0.0f,0.0f>,	<1.5f,1.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;VELCRO
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;MEAT
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}

FrictionTableSlippery LABEL Pointer
;---------------------
;SLIPPERY CONDITIONS
;---------------------
;WOOD
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;STONE
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.7f,0.5f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;ICE
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;GLASS
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;METAL
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.1f,0.05f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;RUBBER
MyTableRow	{<0.0f,0.0f>,	<0.7f,0.5f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;VELCRO
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
;MEAT
MyTableRow	{<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>,	<0.0f,0.0f>}
.code


__Get_Friction macro CONDITION, MaterialID_A, MaterialID_B

    mov eax,sizeof MyTableElement
    %mul &MaterialID_A
    
    .if MaterialID_B!=MATERIALID_DEFAULT
        push edx
        push eax
        mov eax,sizeof MyTableRow
        %mul &MaterialID_B
        pop edx
        add eax,edx
        pop edx
    .endif
    
    .switch CONDITION
    .case CONDITIONS_DEFAULT
        lea eax,FrictionTable[eax]  
    .case CONDITIONS_STICKY
        lea eax,FrictionTableSticky[eax]
    .case CONDITIONS_SLIPPERY
        lea eax,FrictionTableSlippery[eax]
    .default
        DbgWarning "Error - unknown Condition in __GetFriction"
        int 3
    .endsw
endm 
endif