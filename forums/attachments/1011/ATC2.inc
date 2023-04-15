;=====<< STARTUP_INIT_VARS >>===\
.code

;----[ global settings ]---------------\
ifndef ATC_EnlistProcs
	ATC_EnlistProcs		= 0
endif
ifndef ATC_MakeHardcore
	ATC_MakeHardcore	= 0
endif
ATC_PreserveECX			= 1
DefaultClassName		textequ <>
;--------------------------------------/
	
	
;----[ variables for the current class ]------\
ATC_ClassCodeHere		= 1	; if we must specify destructor+constructor
ATC_CurClassName		textequ <>
ATC_CurParentClass		textequ <>
ATC_CurClassNewArgs		textequ <>
ATC_CurClassParentArgs		textequ <>

ATC_FirstParamMe		= 1
ATC_HasVirtualDestructor	= 1
ATC_HasVirtuals			= 1

ATC_NumVoids			= 0
ATC_NumVirtuals			= 0
ATC_NumConstructorArgs		= 0
ATC_NumStaticVars		= 0
ATC_NumVariables		= 0
;---------------------------------------------/

if ATC_EnlistProcs
	.err <--------------End of procedures listing -------------->
endif
if ATC_MakeHardcore
	.err <--------------Happy Hardcore to you ------------------>
endif
 
;=======/
;=====<< TOOL_MACROS >>===\
StripArguments macro var1:VARARG
	local char1,txt
	txt textequ <>
	forc char1,<var1>
	 ifidn <char1>,< >
	 elseifidn <char1>,<	>
	 else
	 txt CATSTR txt,<char1>
	 endif
	endm
	exitm <txt>
endm
;=======/
;=====<< class >>===\
class MACRO ClassName,ParentClass,ClassMode
	
	;--------[ Check arguments ]------------------------------------\
	ifdef ClassName&_$IsDefined
		.err <The class "&ClassName" is already defined!!>
		exitm
	endif
	ifnb ATC_CurClassName
		.err <Do not create the class "&cName" before you finish defining the other class!!>
		exitm
	endif
	ifnb <ParentClass>
	    ifndef ParentClass&_$IsDefined
		.err <The parent class "&ParentClass" is not defined yet, though needed by &ClassName>
		exitm
	    endif
	endif
	;---------------------------------------------------------------/
	
	;---[ set up default global variables ]---------------------\
	ATC_CurClassName	textequ <ClassName>
	ATC_CurParentClass	textequ <ParentClass>
	ClassName&_$IsDefined	= 1
	ClassName&_$Parent	textequ <ParentClass>
	
	
	
	ATC_CurClassNewArgs		textequ <>
	ATC_CurClassParentArgs		textequ <>
	
	ATC_FirstParamMe		= 1
	ATC_HasVirtualDestructor	= 1
	ATC_HasVirtuals			= 1
	
	ATC_NumVoids			= 0
	ATC_NumVirtuals			= 0
	ATC_NumConstructorArgs		= 0
	ATC_NumStaticVars		= 0
	ATC_NumVariables		= 0
	
	@statics_num			= 0
	;-----------------------------------------------------------/
	
	
	;------[ C++ compatible ]------------------\
	ifidni <ClassMode>,<C++ compatible>
		ATC_FirstParamMe 		= 0
	endif
	;------------------------------------------/
	
	;-----[ Compatibility with COM ]-------------\
	ifidni <ClassMode>,<COM compatible>
		ATC_HasVirtualDestructor	= 0
	endif
	;--------------------------------------------/
	
	;-----[ non-virtual ]------------------------\
	ifidni <ClassMode>,<non-virtual>
		ATC_HasVirtualDestructor	= 0
		ATC_FirstParamMe 		= 0
		ATC_HasVirtuals			= 0
	endif
	;--------------------------------------------/
	
	
	;----[ Register ClassMode ]------------\
	if ATC_FirstParamMe
		ClassName&_$cpp = 0
	else
		ClassName&_$cpp = 1
	endif
	;--------------------------------------/
	
	
	;---[ start up struct ]-------------------------------\
ClassName STRUCT
	
	ifidn <ParentClass>,<>
	    if ATC_HasVirtuals
		long $virtual_table
	    endif
	else
		@enlist_class_variables <ParentClass>
	endif
	;-----------------------------------------------------/
	
	
	;------[ make first virtual func ($delete) ]-------\
	if ATC_HasVirtualDestructor
		virtual __$delete
	endif
	;--------------------------------------------------/
	
		
endm
;=======/
;=====<< virtual >>===\
virtual MACRO NameAndArgs:VARARG
	local ClassName,ParameterCheck,AllArgs
	local right_start,desen,argsNum ; for argument count
	local fName
	local arg,cvfn,cvft,RRS ; dummy variables
	
	
	;-------[ check args and init vars ]-------------------------------\
	ife ATC_HasVirtuals
		.err <virtual: You cannot use virtuals in a class without vTable>
		exitm
	endif
	ifb ATC_CurClassName
		.err <virtual: No class defined! >
		exitm
	endif
	ClassName textequ ATC_CurClassName
	ParameterCheck = 0
	AllArgs textequ StripArguments(<NameAndArgs>)
	ParameterCheck=0
	;------------------------------------------------------------------/
	
	;------[ count parameters for parameter check ]----------------\
	if @InStr(1,%AllArgs,<:>) ;-------------------------\
		ParameterCheck = 1
		right_start = @InStr(1,%AllArgs,<:>)
		if @SizeStr(%AllArgs) eq right_start
			desen textequ <>
		else
			desen SUBSTR AllArgs,right_start+1
		endif
		
		; now count the vars num
		
		argsNum=0
		%for arg,<desen>
		  argsNum = argsNum + 1
		endm
		;.err %argsNum
		;.err desen
		RRS= @InStr(1,%AllArgs,<:>) 
		fName SUBSTR AllArgs,1,RRS-1
		
	else
		fName textequ <AllArgs>
		desen textequ <>
		argsNum = 1000
	endif ;------------------------------------------/	
	;--------------------------------------------------------------/
	
	;---------[ setup texts and constants ]-------------------------------------\
	
	cvfn = ATC_NumVirtuals	; number of virtual funcs, already existing
	@CatStr(%ClassName,<_$fn_>,%cvfn) textequ fName
	@CatStr(%ClassName,<_>,%fName,<$argz>) = argsNum
	@CatStr(%ClassName,<_>,%fName,<$isV>)  = 1 ; It is virtual
	
	
	ATC_NumVirtuals = ATC_NumVirtuals + 1
	;---------------------------------------------------------------------------/
	
	;-------[ Enlist procs for fast fun ]----------------\
	if ATC_EnlistProcs
		ifdif fName,<__$delete>
		    echo 
		    if ATC_FirstParamMe
			ifb <desen>
				%echo ClassName&&_&&fName proc me
			else
				%echo ClassName&&_&&fName proc me,desen
			endif
		    else
			%echo ClassName&&_&&fName proc desen
		    endif
		    %echo	ret
		    %echo ClassName&&_&&fName endp
		endif
	endif
	;----------------------------------------------------/
	
endm
;=======/
;=====<< void >>===\
void MACRO NameAndArgs:VARARG
	local ClassName,ParameterCheck,AllArgs
	local right_start,desen,argsNum ; for argument count
	local fName
	local arg,cvfn,cvft,RRS,baka,baka2 ; dummy variables
	local ConstructorArgs,ParentArgs
	
	;-------[ check args and init vars ]-------------------------------\
	ifb ATC_CurClassName
		.err <void: No class defined! >
		exitm
	endif
	ClassName textequ ATC_CurClassName
	ParameterCheck = 0
	AllArgs textequ StripArguments(<NameAndArgs>)
	ParameterCheck=0
	;------------------------------------------------------------------/
	
	;------[ count parameters for parameter check ]----------------\
	if @InStr(1,%AllArgs,<:>) ;-------------------------\
		ParameterCheck = 1
		right_start = @InStr(1,%AllArgs,<:>)
		if @SizeStr(%AllArgs) eq right_start
			desen textequ <>
		else
			desen SUBSTR AllArgs,right_start+1
		endif
		
		; now count the vars num
		argsNum=0
		%for arg,<desen>
			argsNum = argsNum + 1
		endm
		;.err %argsNum
		;.err desen
		RRS= @InStr(1,%AllArgs,<:>) 
		fName SUBSTR AllArgs,1,RRS-1
		
	else
		fName textequ <AllArgs>
		desen textequ <>
		argsNum = 1000
	endif ;------------------------------------------/	
	;--------------------------------------------------------------/
		
	
	;-------[ Enlist procs for fast fun ]----------------\
	if ATC_EnlistProcs
		ifdif fName,<__$delete>
		    echo 
		    if ATC_FirstParamMe
			ifb <desen>
				%echo ClassName&&_&&fName proc me
			else
				%echo ClassName&&_&&fName proc me,desen
			endif
		    else
			%echo ClassName&&_&&fName proc desen
		    endif
		    %echo	ret
		    %echo ClassName&&_&&fName endp
		endif
	endif
	;----------------------------------------------------/
	
	
	
	;-------------------[ constructor modifier ]--------------------------\
	ifidn ClassName,fName
		cvfn INSTR desen,<:>
		if cvfn ; if we send arguments to the parent
			ConstructorArgs SUBSTR desen,1,cvfn-1
			ParentArgs SUBSTR desen,cvfn+1	
			@CatStr(%ClassName,<_$ParentArgs>) textequ ParentArgs
			@CatStr(%ClassName,<_$NewArgs>) textequ ConstructorArgs
			ATC_CurClassNewArgs		textequ ConstructorArgs
			ATC_CurClassParentArgs		textequ ParentArgs
		else
			ConstructorArgs SUBSTR desen,1
			ParentArgs textequ <>
			@CatStr(%ClassName,<_$ParentArgs>) textequ <>
			@CatStr(%ClassName,<_$NewArgs>) textequ desen
			ATC_CurClassNewArgs		textequ ConstructorArgs
		endif	
		
		exitm
	endif ;---------------------------------------------------------------/
	
	
	;------ now setup the  class_func proto :DWORD,:DWORD ... ----------\
	if ParameterCheck
		cvfn = argsNum
		if ATC_FirstParamMe
			cvfn = cvfn + 1
		endif
		cvft CATSTR ClassName,<_>,fName,< proto >
		while cvfn
			cvfn = cvfn - 1
			if cvfn
				cvft CATSTR cvft,<:DWORD,>
			else
				cvft CATSTR cvft,<:DWORD>
			endif
		endm
		cvft
	endif
	;---------------------------------------------------------------------/
	
	
	
	;---------[ setup texts and constants ]-------------------------------------\
	cvfn = ATC_NumVoids	; number of void funcs, already existing
	@CatStr(%ClassName,<_$an_>,%cvfn) textequ fName
	@CatStr(%ClassName,<_>,%fName,<$argz>) = argsNum
	@CatStr(%ClassName,<_>,%fName,<$isV>)  = 0 ; It is NOT virtual
	
	
	ATC_NumVoids = ATC_NumVoids + 1
	;---------------------------------------------------------------------------/
	
endm
;=======/
;=====<< MORE_TOOLS >>===\
@enlist_class_variables MACRO cName
	local ParentClass,baka,cvfn,cvft,cvft2,tName,cName2
	cName2 textequ <cName>
	ParentClass CATSTR cName2,<_$Parent>
	ifdif ParentClass,<>
		@enlist_class_variables <&ParentClass>
	endif
	;---< enlist variables >------------------------------------\
	cvft CATSTR cName2,<_$vn>
	cvfn equ cvft
	baka=0
    while baka LT cvfn ; for each variable in this class ...
	cvft CATSTR cName2,<_$vn_>
	cvft2 CATSTR cvft,%baka
	cvft2
	baka=baka+1
    endm
	;-----------------------------------------------------------/
endm


@AddVirtualFunction macro cName,cFunc
	local already_exists,bakale,ft,ft2,cFunc2,cFunc3,baka2kun,baka3,gClass,bakame,curCla,baka
	local sami,curClass,baka2
	sami textequ <$r>
	curClass textequ <ATC_CurClassName>
	
	;---< check if virtual function has been already defined >---\
	already_exists = 0
	bakale = 0	
	cFunc2 textequ <cFunc>
    while bakale LT @global_done_virtuals
	cFunc3 CATSTR <@built_f_>,%bakale
	ifidn cFunc2,cFunc3
	  already_exists = bakale+1
	endif
    bakale = bakale + 1
    endm
	
	;------------------------------------------------------------/
	
	
	
	if already_exists eq 0
		@CatStr(<@built_f_>,%@global_done_virtuals) textequ cFunc
		@CatStr(<@built_f2_>,%@global_done_virtuals) CATSTR cName,<_>,cFunc
		@global_done_virtuals = @global_done_virtuals + 1
	else
	 @CatStr(<@built_f2_>,%(already_exists-1)) CATSTR cName,<_>,cFunc
	endif
	
	ifdifi cFunc,<__$delete>
		bakame CATSTR <_>, cFunc, <$r>  ;  CATSTR curClass,<_>,cFunc,<$r>
		baka CATSTR cName,<_>,cFunc
		@CatStr(%curClass,%bakame) textequ baka
	endif
	
endm	

@enlist_class_virtuals MACRO cName
	local ParentClass,baka,cvfn,cvft,tName
	local baka2,already_exists,fn,ft,ft2
	
	ParentClass CATSTR cName,<_$Parent>
	ifdif ParentClass,<>
		@enlist_class_virtuals &ParentClass
	endif
	
	
	
	;-------< enlist virtuals >----------------------------------------\
	cvft CATSTR cName,<_$vfn>
	cvfn equ cvft
	baka=0
    while baka LT cvfn ; for each virtual function in this class ...
	cvft CATSTR cName,<_$fn_>
	cvft CATSTR cvft,%baka
	
	;--< ADD VIRTUAL FUNCTION !>------------\*********************************************************
	; cName - cur class
	; cvft  - func name
	@AddVirtualFunction cName,cvft
	
	;---------------------------------------/*********************************************************
	
	@global_virtuals_num=@global_virtuals_num +1
	baka=baka+1
    endm
	;------------------------------------------------------------------/
	
endm

@enlist_class_voids MACRO cName
	local ParentClass,baka,cvfn,cvft,tName
	local baka2,already_exists,fn,ft,ft2
	ParentClass CATSTR cName,<_$Parent>
	
	ifdif ParentClass,<>
		@enlist_class_voids &ParentClass
	endif
	
	
	
	;-------< enlist virtuals >----------------------------------------\
	cvft CATSTR cName,<_$afn>
	cvfn equ cvft
	baka=0
    while baka LT cvfn ; for each virtual function in this class ...
	cvft CATSTR cName,<_$an_>
	cvft CATSTR cvft,%baka
	
	;--< ADD VIRTUAL FUNCTION !>------------\*********************************************************
	; cName - cur class
	; cvft  - func name
	
	@AddVoidFunction cName,cvft
	
	;---------------------------------------/*********************************************************
	
	@global_actions_num=@global_actions_num +1
	baka=baka+1
    endm
	;------------------------------------------------------------------/
	
endm


@AddVoidFunction macro cName,cFunc
	local already_exists,bakale,ft,ft2,cFunc2,cFunc3,baka2kun,baka3,gClass,bakame,curCla,baka
	local sami,curClass,baka2
	sami textequ <$r>
	curClass textequ <ATC_CurClassName>
	
	;---< check if void function has been already defined >------\
	already_exists = 0
	bakale = 0	
	cFunc2 textequ <cFunc>
    while bakale LT @global_done_virtuals
	cFunc3 CATSTR <@built_fa_>,%bakale
	ifidn cFunc2,cFunc3
	  already_exists = bakale+1
	endif
    bakale = bakale + 1
    endm
	
	;------------------------------------------------------------/
	
	
	if already_exists eq 0
		@CatStr(<@built_fa_>,%@global_done_actions) textequ cFunc
		@CatStr(<@built_fa2_>,%@global_done_actions) CATSTR cName,<_>,cFunc
		@global_done_actions = @global_done_actions + 1
	else
	 @CatStr(<@built_fa2_>,%(already_exists-1)) CATSTR cName,<_>,cFunc
	endif
	
	ifdifi cFunc,<__$delete>
		bakame CATSTR <_>, cFunc, <$r>  ;  CATSTR curClass,<_>,cFunc,<$r>
		baka CATSTR cName,<_>,cFunc
		@CatStr(%curClass,%bakame,< textequ >,baka)
	endif
	
endm	

;=======/
;=====<< DEFINE DATA IN CLASS >>===\
AddClassData MACRO vName
	local cName,cvn,cvt
	
	cName textequ <ATC_CurClassName>
	
	cvn = ATC_NumVariables
	
	cvt CATSTR cName,<_$vn_>
	cvt CATSTR cvt,%cvn
	@CatStr(cvt,< textequ <&vName>>)
	ATC_NumVariables = ATC_NumVariables + 1
	
	vName
	
endm
	float MACRO var1
		AddClassData <var1 real4 ?>
	endm
	double MACRO var1
		AddClassData <var1 real8 ?>
	endm
	long MACRO var1
		AddClassData <var1 dd ?>
	endm
	char MACRO var1
		AddClassData <var1 db ?>
	endm
	short_int MACRO var1
		AddClassData <var1 dw ?>
	endm
	des MACRO var1
		AddClassData <var1 !<!>>
	endm
	
	dea MACRO dataName
		AddClassData <dataName dup!(!<!>!)> 
	endm
	array macro argName:REQ, argType:REQ, argNum:REQ
        	AddClassData <argName argType argNum dup (<?>)>
	endm

	
;=======/


;=====<< endclass >>===\
endclass MACRO
	local ClassName
	local baka,baka2,baka3,baka4,baka5
	local Parent,ParentNew,ParentDelete,Constructor,Destructor,ParentArgs
	;-----[ init variables ]----------------\
	ifb ATC_CurClassName
		.err <No class has been defined>
		exitm
	endif
	ClassName textequ ATC_CurClassName
	
	
	@global_virtuals_num 	= 0
	@global_done_virtuals	= 0
	@global_actions_num 	= 0
	@global_done_actions 	= 0
	
	@CatStr(%ClassName,<_$vfn>) = ATC_NumVirtuals
	@CatStr(%ClassName,<_$afn>) = ATC_NumVoids
	@CatStr(%ClassName,<_$vn>)  = ATC_NumVariables
	;---------------------------------------/
	
	;----[ See what the functions will be ]------------------------------------------------\
	@enlist_class_virtuals ClassName  ; start building structure and register virtual functions
	@enlist_class_voids ClassName 
	;--------------------------------------------------------------------------------------/
	
	
	
	
	
  ClassName ends
	
	
	
	
	
	;------------[ Build the virtual table ]----------------------------------------\
 if ATC_HasVirtuals
	if ATC_ClassCodeHere
		@CatStr(%ClassName,<_$virt_table:>)
	endif
	
	baka2=0
   while baka2 LT @global_done_virtuals
	
	baka 		textequ @CatStr(<@built_f2_>,%baka2)
	ThisFuncName 	textequ @CatStr(<@built_f_>,%baka2)
	ThisFuncName CATSTR ClassName,<_>,ThisFuncName
	
	
	baka4 textequ @CatStr(<@built_f_>,%baka2)	
	baka4 CATSTR ClassName,<_>,baka4
	
	if ATC_ClassCodeHere  ; if normal, then ;------------------\
		 dd offset baka   	; register in table			   |  TABLE REGISTERING
	endif ;----------------------------------------------------/
	;--------------[ Display hardcode data ]--------------\
	;----------------------------------------------\
	baka textequ @CatStr(<@built_f_>,%baka2)	;
	baka CATSTR ClassName,<_>,baka,<$>		;	REGISTER VIRTUAL FOR iCall
	baka = baka2					;
	;----------------------------------------------/
	;----------------------------------------------------/
	baka2=baka2+1
   endm
 endif
	;-------------------------------------------------------------------------------/
	
	
	;------ now setup the  class___class proto :DWORD,:DWORD ... ----------\	
	baka2 = 0
	%for baka5,<ATC_CurClassNewArgs>
	  baka2 = baka2 + 1
	endm
	baka5 CATSTR ClassName,<___>,ClassName,< proto >
	while baka2
		baka2 = baka2 - 1
		if baka2
			baka5 CATSTR baka5,<:DWORD,>
		else
			baka5 CATSTR baka5,<:DWORD>
		endif
	endm
	baka5
	;---------------------------------------------------------------------/
	
	
  ife ATC_ClassCodeHere ;-----[ EXIT ALREADY ]----\
	ATC_CurClassName textequ <>
	exitm ;---------------------------------/
  endif
	
	;--------[ static stuff ]-------\
	if @statics_num ne 0
		.data
		baka2 = 0
		while baka2 LT @statics_num
			baka4 CATSTR <@statics>,%baka2
			baka4 CATSTR ClassName,<_>,baka4
			baka4
			baka2 = baka2 + 1
		endm
		.code
	endif
	;-------------------------------/
	
	;----< create real constructor >-------------------------------------------------\
	
	Parent CATSTR ClassName,<_$Parent>
	ParentNew CATSTR Parent,<___>,Parent
	ParentDelete CATSTR Parent,<___$delete>
	Constructor CATSTR ClassName,<_>,ClassName
	Destructor CATSTR ClassName,<_$>,ClassName
	ParentArgs CATSTR ClassName,<_$ParentArgs>
	
	
	
	;------ now setup the  class_class proto :DWORD,:DWORD ... ----------\
	if ATC_FirstParamMe
		baka2 = 1
		Destructor proto :DWORD
	else
		baka2 = 0
		Destructor proto
	endif
	%for baka,<ATC_CurClassNewArgs>
	  baka2 = baka2 + 1
	endm
	baka CATSTR ClassName,<_>,ClassName,< proto >
	while baka2
		baka2 = baka2 - 1
		if baka2
			baka CATSTR baka,<:DWORD,>
		else
			baka CATSTR baka,<:DWORD>
		endif
	endm
	baka
	;---------------------------------------------------------------------/
	
	
	
	
	
	baka CATSTR ClassName,<___>,ClassName
	
  baka proc PUBLIC ATC_CurClassNewArgs	; Object___Object    +args
	.if !ecx
		mov ecx,malloc(sizeof ClassName)
		mov dword ptr[ecx],offset @CatStr(%ClassName,<_$virt_table>)
	.endif
	ifdif Parent,<>
	 push ecx
		ifnb ParentArgs
		  invoke ParentNew,ParentArgs
		else 
		  invoke ParentNew
		endif
	 pop ecx
	endif
	
	push ecx
	ifdif ATC_CurClassNewArgs,<>
	   if ATC_FirstParamMe
	     invoke Constructor,ecx,ATC_CurClassNewArgs
	   else
	     invoke Constructor,ATC_CurClassNewArgs
	   endif
	else
	   if ATC_FirstParamMe
	     push ecx
	     call Constructor
	   else
	     call Constructor
	   endif
	endif
	pop ecx
      ConstructorExtra
	mov eax,ecx
	ret
  baka endp

	;----------------------------------------------------------------------------/
	
	
	;-------< Create real destructor >-------------------------------------------------------\
  baka CATSTR ClassName,<___$delete>
	
  baka proc exitcode  ; Object___$Object
	push ecx
          DestructorExtra
	    if ATC_FirstParamMe
	        push ecx
	    endif
	    call Destructor
	pop ecx
	
	ifdif Parent,<>
	 push ecx
	 invoke ParentDelete,exitcode
	 pop ecx
	else
	 free ecx
	endif
	
	ret
  baka endp	
	
	
	ATC_CurClassName textequ <>
endm
;=======/


;=====<< new >>===\
new MACRO ClassName:REQ,var1:VARARG
	local origFunc
	origFunc textequ <ClassName&___&ClassName>
	
	;----[ check args ]---------------------\
	for vara,<var1>
		ifidni <vara>,<ecx>
		.err <Please do not use ECX as a parameter in "new">
		exitm
		endif
	endm
	;---------------------------------------/
	
	
	xor ecx,ecx
	Scall origFunc,<var1>
	exitm <eax> ; ECX and EAX have it now
endm


$new MACRO What:REQ,ClassName:REQ,var1:VARARG
	local baka4,vara,origFunc
	origFunc textequ <ClassName&___&ClassName>
	
	push eax
	push ecx
	push edi
	lea edi,What
	xor eax,eax
	mov ecx,sizeof ClassName
	rep stosb
	pop edi
	pop ecx
	pop eax
	
	;----[ check args ]---------------------\
	for vara,<var1>
		ifidni <vara>,<ecx>
		.err <Please do not use ECX as a parameter in "new">
		exitm
		endif
	endm
	;---------------------------------------/
	
	lea ecx,What
	Scall origFunc,<var1>

endm	

;=======/

;=====<< delete >>===\
delete MACRO What
	push eax
	push ecx
	mov ecx,What
	.if ecx
		mov eax,[ecx]
		push 0
		call dword ptr[eax]
	.endif

	pop ecx
	pop eax
endm

DeleteCOM macro What:REQ,ClassName:REQ
	push eax
	push ecx
	ifdifi <ecx>,<What>
	mov ecx,What
	endif
	invoke @CatStr(ClassName,<___$delete>),0
	pop ecx
	pop eax
endm	
;=======/

;=====<< SET.macro >>===\
set macro zWhat:VARARG
	local char1,var1,WhatS,dType,lastWasSpace
	
	;----[ clean-up coding stuff ]----------\
	lastWasSpace = 0
	txt textequ <>
	forc char1,<zWhat>
	 ifidn <char1>,< >
		if lastWasSpace eq 0
			lastWasSpace = 1
			 txt CATSTR txt,<char1>
		endif
	 elseifidn <char1>,<	>
		if lastWasSpace eq 0
			lastWasSpace = 1
			 txt CATSTR txt,<char1>
		endif
	 else
		 txt CATSTR txt,<char1>
		lastWasSpace = 0
	 endif
	endm
	;--------------------------------------/
	lastWasSpace INSTR txt,< as >
	WhatS SUBSTR txt,1,lastWasSpace-1
	dType SUBSTR txt,lastWasSpace+4
	%for var1,<WhatS>
		@CatStr(<@rcd$>,<var1>) CATSTR dType
	endm	
	
endm	
;=======/

;=====<< iCall MACRO >>===\
iCall macro What:REQ,ClassName:REQ,FuncName:REQ,var1:VARARG
	local argsNum,thisArgsNum,origFunc,cvft,errTxt1,txt,arg,FunctionID,IsVirtual,baka,EAX_overwritten,num1
	
	;-----[ Initial adjustments ]--------\
	if ATC_PreserveECX
		push ecx
	endif
	thisArgsNum=0
	FOR arg,<var1>
	 IFNB <arg>
		thisArgsNum = thisArgsNum + 1
	 ENDIF
	ENDM
	;------------------------------------/
	
	;------[ Is This Function Defined? Is it virtual? Check parameters, too ]-------------------------\
   ifdef ClassName&_&FuncName&$r
	origFunc CATSTR <ClassName>,<_>,<FuncName>,<$r>
	IsVirtual = @CatStr(%origFunc,<$isV>)
	argsNum = @CatStr(%origFunc,<$argz>)
	if argsNum ne 1000
		if argsNum ne thisArgsNum		
			errTxt1 CATSTR <Parameters number is not >,%argsNum,<, you've forgotten/added some>
			.err errTxt1
			exitm
		endif
	endif
   else	
	.err <Function ClassName&_&FuncName not defined!!>
	exitm
   endif
	;------------------------------------------------------------------------------------------------/
	
	
	;-------[ Push parameters ]------------\
	ifnb <var1>
	EAX_overwritten = 0
	txt TEXTEQU <>
	% FOR arg, <var1>
	      txt CATSTR <arg>, <!,>, txt
	ENDM
 	txt SUBSTR  txt, 1, @SizeStr( %txt ) - 1
   	txt CATSTR  <!<>, txt, <!>>
	% FOR arg,txt
	 IFNB <arg>
		ifidni <arg>,<eax>
			if EAX_overwritten
				.err <EAX has already been overwritten! Do not use EAX on the left of ADDR>
			endif
		endif
		
		
		num1 SIZESTR <arg>
		if num1 LT 6
			num1 = 0
		else
			num1 = 0
			ifidni @SubStr(<arg>,1,5),<addr >
				num1 = 1
			endif
		endif
		
		if num1
			EAX_overwritten = 1
			lea eax,@SubStr(<arg>,6)
			push eax
		else
			push arg
		endif
	 ENDIF
	ENDM
	endif
	;-------------------------------------/
	
	
	;-------------[ Call the function ]----------\
	ifdifi <ecx>,<What>
  	   mov ecx,What
	endif
	ife @CatStr(ClassName, <_$cpp>) ; if not C++ compatible
	  push ecx
	endif
	
	
	if IsVirtual
		FunctionID = ClassName&_&FuncName&$
		mov eax,[ecx]
		call dword ptr[eax+FunctionID*4]
	else
		call origFunc
	endif
	;-------------------------------------------/
	
	;--[ clean up ]----\
	if ATC_PreserveECX
		pop ecx
	endif
	;------------------/
endm 		
;=======/
;=====<< pCall >>===\
pCall macro FuncNameX:REQ,var1:VARARG
	local FuncName,VariableNa,baka,baka2,thisClassType
	baka2 INSTR <FuncNameX>,<.>
	ife baka2
	    .err <Please use this syntax: pCall Variable.FunctionName[,arguments]>
	    exitm
	endif
	VariableNa SUBSTR <FuncNameX>,1,baka2-1
	FuncName SUBSTR <FuncNameX>,baka2+1
	thisClassType CATSTR <@rcd$>,VariableNa
	baka2 INSTR thisClassType,<@rcd$>
	if baka2 eq 1
	    thisClassType CATSTR <Please tell what class ">,VariableNa,<" is, using "set >,VariableNa,< as !<ClassName!>">
	    .err thisClassType
	    exitm
	endif
	
	
	%iCall VariableNa,thisClassType,FuncName,var1
endm		
;=======/
;=====<< hCall >>===\
hcall macro ObjectName:REQ, ObjectType:REQ, FuncName:REQ, var1:VARARG
   push ecx
   lea ecx, ObjectName
   icall ecx, ObjectType, FuncName, var1
   pop ecx
endm
;=======/
;=====<< nCall, oCall >>===\
	
Scall MACRO name:REQ,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22

    FOR arg,<p22,p21,p20,p19,p18,p17,p16,p15,p14,p13,\
             p12,p11,p10,p9,p8,p7,p6,p5,p4,p3,p2,p1>
      IFNB <arg>    ;; If not blank
        push arg    ;; push parameter
      ENDIF
    ENDM

    call name       ;; call the procedure

ENDM
ccall MACRO name:REQ,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11,p12,p13,p14,p15,p16,p17,p18,p19,p20,p21,p22
	local stas
	stas=0
    FOR arg,<p22,p21,p20,p19,p18,p17,p16,p15,p14,p13,\
             p12,p11,p10,p9,p8,p7,p6,p5,p4,p3,p2,p1>
      IFNB <arg>    ;; If not blank
        push arg    ;; push parameter
	stas = stas +4
      ENDIF
    ENDM

    call name       ;; call the procedure
	if stas ne 0
		add esp,stas
	endif
ENDM


nCall macro What:REQ,FuncName:REQ,var1:VARARG
	ifidn DefaultClassName,<>
	  .err <Please set some default class, using "SetDefClass">
	  exitm
	endif	
	%iCall What,DefaultClassName,FuncName,var1
endm	
	
oCall macro FuncName:REQ,var1:VARARG
	ifidn DefaultClassName,<>
	  .err <Please set some default class, using "SetDefClass">
	  exitm
	endif	
	%iCall ecx,DefaultClassName,FuncName,var1
endm	
	
	
static macro what:VARARG
	@CatStr(<@statics>,%@statics_num) textequ <what>
	;.err @CatStr(<@statics>,%@statics_num)
	@statics_num = @statics_num + 1
endm

	
icall textequ <iCall> ; recommended for beginners
ncall textequ <nCall> ; Default Call (Normal Call)
ocall textequ <oCall> ; save-coder-time call
pcall textequ <pCall>

		
SetDefClass macro What
	DefaultClassName textequ <What>
endm	
	

;=======/

;=====<< ConstDestExtra >>===\
;
;
;	Usage: replace these macros to add debugging and so on features to your classes
;
ConstructorExtra macro
endm
DestructorExtra macro
endm
;=======/

;=====<< DoWeSupportGUIDs >>===\
DoWeSupportGUIDs macro What:REQ,Which:VARARG
local _good,giia
for giia,<Which>
	mov ecx,What
	lea edx,giia
	mov eax,[ecx+0]
	sub eax,[edx+0]
	jnz @F
	mov eax,[ecx+4]
	sub eax,[edx+4]
	jnz @F
	mov eax,[ecx+8]
	sub eax,[edx+8]
	jnz @F
	mov eax,[ecx+12]
	sub eax,[edx+12]
	@@:
	jz _good
endm
	mov eax,E_NOINTERFACE
	jmp @F
  _good:
	mov eax,S_OK
    @@:	
endm	
;=======/


;=====<< Malloc/FreeStuff >>===\

malloc_func proto :DWORD
free_func proto :DWORD
realloc_func proto :DWORD,:DWORD


malloc MACRO How:REQ
	invoke malloc_func,How
	exitm <eax>
ENDM
	
free MACRO What
	invoke free_func,What
ENDM
	
realloc MACRO WHat:REQ,How:REQ
	invoke realloc_func,WHat,How
	exitm <eax>
ENDM
;=======/

;=====<< EnterLeaveHostedClass >>===\
EnterHostedClass macro
	ATC_ClassCodeHere = 0 
endm
LeaveHostedClass macro
	ATC_ClassCodeHere = 1
endm
;=======/
