
; ——————————————————————————————————————————————————————————————————————————————————————————————————
; The Assembler class is the highest-order class in this project.
; ——————————————————————————————————————————————————————————————————————————————————————————————————


;Assembler <- Evaluator <- Interpreter <- Parser
; |   |
; |	 OpCodeBank <- OpCodes <- Interpreter <- Parser
;Segments      <-    Collection (of Segment) 

SEGMENT_EXECUTABLE 		equ 0	;Default is "executable, read-only"
SEGMENT_NOT_EXECUTABLE  equ 1	;If present, segment is not executable
SEGMENT_WRITEABLE 		equ 2	;If present, segment is writeable
SEGMENT_BSS				equ 4	;If present, segment is NOT INITIALIZED (data?)


; ——————————————————————————————————————————————————————————————————————————————————————————————————
Object Assembler, 534534, Evaluator
	RedefineMethod Init,Pointer,LPSTR,LPSTR,LPSTR
	RedefineMethod Done
	RedefineMethod	Normal_Stm,		Pointer

	StaticMethod	SelectSegment,	LPWSTR,dword
	Private
		StaticMethod	FindSegment,	LPWSTR
		StaticMethod	CreateSegment,	LPWSTR,dword
	PrivateEnd
	
	DefineVariable	pCurrentSegment,	Pointer,	NULL
	DefineVariable  pSymbol_UserSegment,Pointer,NULL
	
	Embed Segments,		Collection
	Embed OpCodeBank, 	OpCodes
ObjectEnd
; ——————————————————————————————————————————————————————————————————————————————————————————————————

;Names of some Symbols that we're interested in, either as reductions or just as tokens.
.data
szDataSegment  db ".",0,"d",0,"a",0,"t",0,"a",0,0,0
szCodeSegment  db ".",0,"c",0,"o",0,"d",0,"e",0,0,0
szBSSSegment   db ".",0,"d",0,"a",0,"t",0,"a",0,"?",0,0,0
szUserSegment  db ".",0,"S",0,"e",0,"g",0,"m",0,"e",0,"n",0,"t",0,0,0
szExecutable   db "E",0,"x",0,"e",0,"c",0,"u",0,"t",0,"a",0,"b",0,"l",0,"e",0,0,0
szWriteable	   db "W",0,"r",0,"i",0,"t",0,"e",0,"a",0,"b",0,"l",0,"e",0,0,0
szBSS		   db "B",0,"S",0,"S",0,0,0
.code

;Method:	Assembler.Init
;Purpose:	Initialize components, load grammars, register handlers, etc
;Args:		pOwner -> owner object
;			pszOpcodeGrammar -> (Ascii String) pathname of opcode-encodings grammar file
;			pszOpCodeText	 -> (Ascii String) pathname of opcode-encodings source file
;			pszMainGrammar	 -> (Ascii String) pathname of Main assembler grammar gile
Method Assembler.Init,uses esi, pOwner,pszOpcodeGrammar:LPSTR, pszOpCodeText:LPSTR, pszMainGrammar:LPSTR
	SetObject esi	
	;Initialize ancestors (->Evaluator->Interpreter->Parser)
	ACall Init, pOwner	
	
	;Initialize cousins - especially OpCodes manager
	OCall [esi].OpCodeBank::OpCodes.Init, pOwner, pszOpcodeGrammar, pszOpCodeText
	OCall [esi].Segments::Collection.Init,esi,16,256,-1

	;Load the main grammar tables for the assembler syntax
	OCall esi.LoadGrammarFile,pszMainGrammar

	;Note any particularly useful symbols
	mov [esi].pSymbol_UserSegment, $OCall (esi.GetSymbolByName,addr szUserSegment)

	;Register callback handlers for main grammar
	;This is where we implement code to expand reductions and recognize stuff
	OCall esi.RegisterHandler, addr szNormal_Stm, $MethodAddr(Assembler.Normal_Stm)
MethodEnd

;Method:	Assembler.Done
;Purpose:	Tear everything down, you guys suck, taking my ball and going home
Method Assembler.Done,uses esi
	SetObject esi
	OCall [esi].OpCodeBank::OpCodes.Done
	OCall [esi].Segments::Collection.Done
	ACall Done
MethodEnd

;Method:	Assembler.FindSegment
;Purpose:	Search for Segment by Name
;Args:		pSegmentName -> (Wide String) Name of Segment
;Returns:	EAX -> Segment,
;		or  EAX = NULL (Not Found)
Method Assembler.FindSegment, uses esi ebx, pSegmentName:LPWSTR
	SetObject esi
	xor ebx,ebx
	.while ebx<[esi].Segments.dCount
		OCall [esi].Segments::Collection.ItemAt, ebx
		.if [eax]._Segment.pName==0
			DbgWarning "Error - null segment name"
			int 3
		.endif
		push eax
		invoke WideCompare,[eax]._Segment.pName,pSegmentName
		pop edx
		.if eax==0
			return edx
		.endif
		inc ebx
	.endw
	xor eax,eax
MethodEnd

;Select an output Segment by name.
;If the segment does not exist, it will be created,
;and if its a 'known' segment, with known flags,
;and otherwise, with the given flags.
Method Assembler.SelectSegment, uses esi ebx, pSegmentName:LPWSTR, dFlags
	SetObject esi
	DbgUStr pSegmentName,"Selecting Segment"
	OCall esi.FindSegment, pSegmentName
	.if eax==0
		invoke WideCompare, pSegmentName, addr szDataSegment
		.if eax==0
			mov eax, SEGMENT_NOT_EXECUTABLE + SEGMENT_WRITEABLE
		.else
			invoke WideCompare, pSegmentName, addr szCodeSegment
			.if eax==0
				mov eax,SEGMENT_EXECUTABLE
			.else
				invoke WideCompare, pSegmentName, addr szBSSSegment
				.if eax==0
					mov eax,SEGMENT_NOT_EXECUTABLE + SEGMENT_WRITEABLE + SEGMENT_BSS
				.else
					mov eax,dFlags
				.endif
			.endif
		.endif
		OCall esi.CreateSegment, pSegmentName, eax
	.endif
	mov [esi].pCurrentSegment,eax
MethodEnd

;Method:	Assembler.CreateSegment
;Purpose:	
;		dFlag = combination of SEGMENT_XXX flags (NULL = executable, read-only)
Method Assembler.CreateSegment,uses esi, pSegmentName:LPWSTR, dFlags:dword
	SetObject esi
	OCall [esi].Segments::Collection.Insert,$New(_Segment, Init, esi, pSegmentName)
	m2m [eax]._Segment.dFlags, dFlags, edx
MethodEnd


;<Normal Stm>  ::= display <Expression>
;               |  display <Expression> read ID
;		 		|  <Assignment>						! Support for =, +=, -=, *=, /=, and repeat of these
;		 		|  '.code'
;		 		|  '.data'
;		 		|  '.Segment' ID <FlagList>
Method Assembler.Normal_Stm, uses esi edi ecx, pReduction
LOCAL IsExecutable:BOOL, IsWriteable:BOOL, IsBSS:BOOL
LOCAL pSegNameToken,pToken
	SetObject esi
	DbgWarning "Normal Statement"

	OCall esi.DefaultHandler, pReduction
	invoke DbgReduction,pReduction

	mov edx,pReduction
	.if [edx].Reduction.TokenCount==1
		;its .code or .data
		mov eax,[edx].Reduction.Tokens
		OCall esi.SelectSegment, [eax].Token.TokenData,NULL
	.else
		mov eax,[edx].Reduction.Tokens
		mov edx,[eax].Token.ParentSymbol
		.if edx==[esi].pSymbol_UserSegment
			add eax,sizeof Token
			mov pSegNameToken,eax
			mov edx,pReduction
			.if [edx].Reduction.TokenCount>2
				mov edi,edx
				add eax,sizeof Token
				mov cx,[edx].Reduction.TokenCount
				sub cx,2
				mov IsExecutable,FALSE
				mov IsWriteable,FALSE
				mov IsBSS,FALSE
				.while cx!=0
					mov pToken,$Get_Element (Token, [edi].Reduction.Tokens, cx)
					invoke WideCompare,[eax].Token.TokenData, addr szExecutable
					.if eax==0
						mov IsExecutable,TRUE
					.else
						mov eax,pToken
						invoke WideCompare,[eax].Token.TokenData, addr szWriteable
						.if eax==0
							mov IsWriteable,TRUE
						.else
							mov eax,pToken
							invoke WideCompare,[eax].Token.TokenData, addr szBSS
							.if eax==0
								mov IsBSS,TRUE
							.else
								DbgWarning "Syntax Error: Unexpected Symbol in SegDef"
							.endif
						.endif
					.endif
					dec cx
				.endw
				mov eax,pSegNameToken
				xor edx,edx
				.if IsExecutable==FALSE
					mov edx,1
				.endif
				.if IsWriteable==TRUE
					or edx,SEGMENT_WRITEABLE				
				.endif
				.if IsBSS==TRUE
					or edx,SEGMENT_BSS
				.endif
				OCall esi.SelectSegment, [eax].Token.TokenData, edx
			.else
				OCall esi.SelectSegment, [eax].Token.TokenData, SEGMENT_EXECUTABLE
			.endif
		.else
			DbgWarning "Error : Unhandled Case in Assembler.Normal_Stm"
			int 3
		.endif
	.endif
	
	OCall esi.ReleaseReduction, pReduction
MethodEnd

