
;This object creates a list of these elements:
;-labeled code chunks
;-branch instructions
;-unlabelled code following a branch instruction

SECTION_ALIGNMENT equ 512	;PageSize by which segment size can grow


;Segments hold a list of two kinds of elements:
CODE_ELEMENT	equ 0
BRANCH_ELEMENT 	equ 1

;Contains binary data for one or more completely resolved opcodes.
CodeElement struct
	dType   dword   ?
	pLabel	Pointer ?	;Optional pointer to a Label for the start of this code chunk
	pBuffer Pointer ?	;Buffer for the binary data
	dSize   dword   ?	;#bytes in buffer
CodeElement ends

;Represents a logical branch instruction
BranchElement struct
	dType	dword   ?
	pLabel	Pointer ?
	dBranchInstruction dd ?
	pTargetLabel Pointer  ?	
	bIsResolved BOOL ?			;Can only be resolved when no unresolved Branches exist between this one and its target
BranchElement ends

;Labeled elements can appear anywhere in the list
_Label struct
	pName	 LPWSTR  ?
	pElement Pointer ?
_Label ends




Object _Segment, 4354353, DataCollection
	RedefineMethod	Init,	Pointer, Pointer
	RedefineMethod	Done
	RedefineMethod  Insert, Pointer				;append an input element to list
	StaticMethod	Emit,	Pointer, dword		;spit out some binary code to final output segment
	StaticMethod	FindLabel, LPWSTR			;search for label by name - local to this segment
	StaticMethod	MergeCode,Pointer,Pointer	;join two code elements
	DefineVariable pName,   Pointer, NULL			;Name of this segment (SysAllocString)
	DefineVariable dFlags,  DWORD,   NULL			;SEGMENT_XXX flags (see XASM class)
	DefineVariable pBuffer, Pointer, NULL			;Buffer for final output binary data
	DefineVariable dSizeAllocated					;Allocated size of above
	DefineVariable dSizeUsed						;Consumed size of above
	Embed Labels, DataCollection					;List of labels, local to this segment
ObjectEnd

Method _Segment.Init, uses esi, pOwner, pName:LPWSTR
	SetObject esi
	DbgUStr pName, "Creating Segment"
	mov [esi].pBuffer,$MemAlloc(SECTION_ALIGNMENT,MEM_INIT_ZERO)
	mov [esi].dSizeAllocated,SECTION_ALIGNMENT
	OCall [esi].Labels::DataCollection.Init, esi, 16,256,-1
	mov [esi].pName,$invoke(SysAllocString,pName)
MethodEnd

Method _Segment.Done, uses esi ebx
	SetObject esi
	invoke SysFreeString,[esi].pName
	MemFree [esi].pBuffer
	xor ebx,ebx
	.while [esi].Labels.dCount!=0
		OCall [esi].Labels::DataCollection.DeleteAt,0
		push eax
		invoke SysFreeString,[eax]._Label.pName
		pop eax
		MemFree eax
	.endw
	ACall Done
MethodEnd

Method _Segment.Emit, uses esi, pData, dLen
	SetObject esi
	;check if we have enough memory
@@:	mov eax,[esi].dSizeAllocated
	sub eax,[esi].dSizeUsed
	.if eax>dLen
		;no? grow by filesection alignment
		mov eax,[esi].dSizeAllocated
		add eax,SECTION_ALIGNMENT
		push eax
		MemReAlloc [esi].pBuffer,eax,MEM_INIT_ZERO
		pop edx
		.if eax!=0
			mov [esi].pBuffer,eax
			mov [esi].dSizeAllocated,edx
		.endif
		jmp @B	;check if we NOW have enough space, usually we will
	.endif
	;copy the data
	mov eax,[esi].pBuffer
	add eax,[esi].dSizeUsed
	invoke RtlMoveMemory,eax,pData,dLen
	;increment #used counter
	mov eax,dLen
	add [esi].dSizeUsed,eax
MethodEnd

;Remarks:	If last element is code, and input is unlabelled code, we merge them
Method _Segment.Insert,uses esi, pElement
	SetObject esi
	mov edx,pElement
	.if [edx].CodeElement.dType==BRANCH_ELEMENT		
		ACall Insert,pElement
	.elseif [edx].CodeElement.pLabel==0		;if this opcode carries no label
		;If the last element is a code element
		mov eax,[esi].dCount
		dec eax
		OCall esi.ItemAt,eax
		.if [eax].CodeElement.dType==CODE_ELEMENT
			;Store the input code element's data in the last code element
			OCall esi.MergeCode,eax,pElement
		.else
			;Append the unlabeled code element
			ACall Insert,pElement
		.endif
	.else
		ACall Insert,pElement
	.endif
MethodEnd

;Method:	Segment.MergeCode
;Purpose:	Append an input code element (pSource) to the buffer of existing code element (pTarget)
Method _Segment.MergeCode, uses esi edi, pTarget, pSource
	SetObject esi
	mov edi, pTarget
	mov edx,[edi].CodeElement.dSize
	mov eax, pSource
	add edx,[eax].CodeElement.dSize
	MemReAlloc [edi].CodeElement.pBuffer,eax
	.if eax!=0
		mov [edi].CodeElement.pBuffer,eax
		mov edx,[edi].CodeElement.dSize
		push edx
		mov eax, pSource
		add edx,[eax].CodeElement.dSize
		mov [edi].CodeElement.dSize,edx
		pop edx
		add edx,[edi].CodeElement.pBuffer
		invoke RtlMoveMemory,edx,[eax].CodeElement.pBuffer,[eax].CodeElement.dSize
		mov eax,pSource
		MemFree [eax].CodeElement.pBuffer
		MemFree pSource
		mov eax,pTarget
	.else
		DbgWarning "Out of memory"
		int 3
	.endif
MethodEnd

;Search for Label with matching name
Method _Segment.FindLabel, uses esi ebx, pName:LPWSTR
	SetObject esi
	xor ebx,ebx
	.while ebx<[esi].Labels.dCount
		OCall [esi].Labels::DataCollection.ItemAt,ebx
		push eax
		invoke WideCompare,[eax]._Label.pName,pName
		pop edx
		.if eax==0
			return edx
		.endif
		
		inc ebx
	.endw
	xor eax,eax
MethodEnd

;Create a new Label...
;Set the Label of a Code or Branch Element by Name
Method _Segment.SetLabel, uses esi, pElement, pName
	SetObject esi
	OCall esi.FindLabel,pName
	.if eax!=0
		DbgWarning "Error: Attempt to redefine Label detected"
		int 3
	.endif
	invoke SysAllocString, pName
	push eax
	MemAlloc sizeof _Label
	pop [eax]._Label.pName
	m2m [eax]._Label.pElement, pElement, edx		;Label points to Element
	mov [edx].CodeElement.pLabel, eax			;Element points to Label
	OCall [esi].Labels::DataCollection.Insert,eax
MethodEnd


;Determine best branch encodings, finalize segment
;Method Segment.Optimize,uses esi
;
;MethodEnd
