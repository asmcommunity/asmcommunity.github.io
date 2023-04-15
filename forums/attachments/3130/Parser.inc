.data
dbgbuf db 256 dup (?)
.code
DbgPrint macro format, window, args:VARARG
	pushad
	@invoke wsprintf,addr dbgbuf,format, args
	popad
	DbgStr dbgbuf,,window
endm

DbgToken macro token, window
;	pushad
;	mov eax,token
;	mov edx,[eax].Token.ParentSymbol
;	.if [edx].Symbol.Kind==STERMINAL
;		pushad
;		invoke GoAscii,addr buf,[edx].Symbol.sName		
;		popad
;		pushad
;		invoke GoAscii,addr buf2,[eax].Token.TokenData
;		popad		
;		mov edx,eax		
;		DbgPrint "Token %lX: Type=%s Terminal Data=%s",window ,edx,addr buf, addr buf2
;	.elseif [eax].Token.TokenData!=0
;		pushad
;		invoke GoAscii,addr buf,[edx].Symbol.sName		
;		popad
;		mov edx,eax
;		DbgPrint "Token %lX: Type=%s NonTerminal Reduction[%lX]",window ,edx,addr buf, [eax].Token.TokenData
;	.else
;		pushad
;		invoke GoAscii,addr buf,[edx].Symbol.sName		
;		popad
;		DbgPrint "Token %lX: Type=%s NonTerminal",window ,edx,addr buf
;	.endif
;	popad
endm
			.data 
			fmt db "<%s> ",0
			buf4 db 256 dup (0)
			.code
			

;Encoding of input plaintext file
TEXT_ANSI equ 0
TEXT_UTF16_LE equ 1
TEXT_UTF16_BE equ 2

;Return values for Parse method
MessageTokenRead equ 1
MessageReduction equ 2
MessageAccept equ 3
MessageNotLoaded equ 4
MessageLexicalError equ 5
MessageSyntaxError equ 6
MessageCommentError equ 7
MessageInternalError equ 8
MessageShift equ 9

;Symbol Types:
;Note that only the first two appear in the LALR stage...
;The rest are filtered / removed in the DFA stage.
SNONTERMINAL  equ 0  ;SymbolTypeNonterminal Normal Nonterminal 
STERMINAL 	  equ 1  ;SymbolTypeTerminal Normal Terminal 
SWHITESPACE	  equ 2  ;SymbolTypeWhitespace Whitespace Terminal 
SENDOFFILE    equ 3  ;SymbolTypeEnd End Character - End of File. This symbol is used to represent the end of the file or the end of the source input. 
SCOMMENTSTART equ 4  ;SymbolTypeCommentStart Start of a block quote 
SCOMMENTEND   equ 5  ;SymbolTypeCommentEnd  End of a block quote 
SCOMMENTLINE  equ 6  ;SymbolTypeCommentLine Line Comment Terminal 
SERROR        equ 7  ;SymbolTypeError Error Terminal. If the parser encounters an error reading a token, this kind of symbol can used to differentiate it from other terminal types. 


;;STRUCTURES:::::::::
Symbol struct
    Kind  WORD ?		;value from above enumeration
    sName LPWSTR ?
Symbol ends

Rule struct 
    Nonterminal 	WORD ?	 ; Each rule derives a single nonterminal symbol. This field contains the index of the symbol in the Symbol Table. 
    SymbolIndices 	Pointer ?; Address of an array which holds the rules indices.
    SymbolIndexCount WORD ? ; total number of indices hold by the above array. NOT IN SPECIFICATION OF GOLD.
Rule ends

; The DFA stage is where 'characters' are input to the parser.
; It is a state engine, the current state is described by the struct below.
; We load an array of these structs from the CGT file.
; Each DFAState contains an array of Edges, and may or may not accept a terminal symbol.
DFAState struct
    Edges 		 Pointer ?  ; Flat array of Edge structs
    wEdgeCount   WORD ? 	; Total number of elements in the above array.
    wAcceptIndex WORD ? 	; If the state accepts a terminal symbol, this field will contain the symbol's index in the Symbol Table. Otherwise, the value in this field should be ignored. 
    bAcceptState BYTE ? 	; Boolean: Each DFA state can accept one of the grammar's terminal symbols. If the state accepts a terminal symbol, the value will be set to True and the Accept Index parameter will contain the symbol's index. 
DFAState ends

; An Edge is a struct which associates a CharacterSet with a DFAState, by indices.
Edge struct
    wCharSetIndex Word ?	; Index of character set
    wTargetIndex  Word ?	; Next DFA State
Edge ends

; The LALR stage is where 'tokens' are input to the parser.
; It is a state engine, the current state is described by the struct below.
; We load an array of these structs from the CGT file.
; Each LALR state contains an array of Actions.
LALR struct
    Actions 	 Pointer ?  ; Flat array of Action structs
    wActionCount WORD ? 	; Total number of elements in the above array.
LALR ends

;An Action tells the LALR stage what to do, given a specific Symbol.
;It could perform a SHIFT, GOTO, REDUCE or ACCEPT.
Action struct
    wSymbolIndex Word ? ; Index in the Symbol Table. 
    wActionType  Word ? ; Represents the action for LALR stage to take based on the symbol.
    wTarget      Word ? ; Index of next LALR State, or ReductionRule Symbol.
Action ends

;A token is an 'instance of a Symbol'.
;Terminals store their string in the TokenData field.
;NonTerminals normally store NULL in this field,
;the exception being those tokens created to represent a Reduction..
;these NonTerminals will store a ptr to a Reduction in this field.
Token struct
    ParentSymbol dd ?      ; -> Symbol
    TokenData    dd ?      ; LPWSTR if Symbol is terminal, otherwise -> Reduction
    State dd ? ; is a LALR state.
Token ends

;A Reduction is a collection of tokens taken from the input stream
;and replaced with a single nonterminal representing the replacement.
Reduction Struct ;;14 bytes.
     ParentRule Pointer ? ; is a Rule address
     Tokens Pointer ? ; Pointer to the beinning of reductions' tokens in TokenStack.
     TokenCount dw ? ; integer
Reduction endS

;LALR Actions:
ACTIONSHIFT  equ 1 
; This action indicates the symbol is to be shifted. 
; The Target field will contain the index of the LALR State that the parsing engine will advance to. 
ACTIONREDUCE equ 2 
; This action denotes that the parser can reduce a rule.
; The Target field will contain the index of the rule in the Rule Table. 
ACTIONGOTO   equ 3 
; This action is used when a rule is reduced and the parser jumps to the state that represents the shifted nonterminal. 
; The Target field will contain the index of the LALR State that the parsing engine will jump to after a reduction is completed. 
ACTIONACCEPT equ 4 
; When the parser encounters the Accept action for a given symbol, the source text is accepted as correct and complete.
; In this case, the Target field is not needed and should be ignored. 

;parsetoken consts:
PARSEACCEPT 		equ 1
PARSESHIFT 			equ 2
PARSEREDUCENORMAL 	equ 3
PARSEREDUCETRIMMED 	equ 4
PARSESYNTAXERROR 	equ -1
PARSEINTERNALERROR 	equ -2

;Get address of Array Element by (byte or word sized) index
;ElementName = datatype / structname (anything we can apply sizeof)
;ArrayBase = ptr to base of array memory 
;wIndex = 8 or 16-bit index (variable or register)
Get_Element macro ElementName, ArrayBase,wIndex
movzx edx, wIndex
push ArrayBase
mov eax, sizeof ElementName
mul edx
pop edx
add eax, edx
endm

$Get_Element macro ElementName, ArrayBase,wIndex
Get_Element ElementName, ArrayBase,wIndex
exitm <eax>
endm

;===================================
;Helpers to read data from CGT file
;===================================

;GET SHORT
getvsh macro pb,ps
	inc pb			;skip entry id 
	mov eax,pb
    mov ax,word ptr[eax]
    mov ps,ax
    add pb,2
endm

;GET BYTE
getvb macro pb, pv
	inc pb			;skip entry id 
	mov eax,pb
	mov al,byte ptr[eax]
	mov byte ptr pv,al
	inc pb
endm
;===================================

;Helper to allocate flat arrays
;what = datatype (can be a struct name)
;many = #elements
$AllocArray macro what, many
	mov eax,sizeof what
	mul many
	MemAlloc eax,MEM_INIT_ZERO
	exitm <eax>
endm

;Convert string: ascii to wide
ToWide proc uses ecx stringin
LOCAL stringout
	invoke lstrlen,stringin
	inc eax
	shl eax,1
	mov stringout,$MemAlloc(eax)
	mov edx,stringin
	xor ecx,ecx
	.while byte ptr[edx]!=0
		mov cl,byte ptr[edx]
		mov word ptr [eax],cx
		inc edx
		add eax,2
	.endw
	mov word ptr [eax],0
	mov eax,stringout
	ret
ToWide endp

;Convert string: wide to ascii (not recommended)
GoAscii proc uses ecx stringout, stringin
	
	mov eax,stringin
	mov edx,stringout
	.while byte ptr[eax]!=0
		mov cx,word ptr[eax]
		mov byte ptr [edx],cl
		inc eax
		inc eax
		inc edx
	.endw
	mov byte ptr [edx],0
	ret

GoAscii endp

;Compare widestrings
;Returns EAX = 0 (match) or eax==-1
WideCompare proc uses ecx p1, p2
	mov eax,p1
	mov edx,p2
	mov cx,-1
	.while word ptr[eax]!=0 && word ptr[edx]!=0
		mov cx,word ptr[eax]
		.if cx!=word ptr[edx]
			.break
		.endif
		add eax,2
		add edx,2
	.endw
	.if word ptr[eax]==0 && word ptr[edx]==0
		xor eax,eax
	.else
		mov eax,-1
	.endif
	ret
WideCompare endp

;Length of WideString
;Returns EAX = Length of WideString in bytes, including terminator
WideLen proc p1
	push edx
	mov edx,p1
	xor eax,eax
	.while word ptr[edx+eax*2]!=0
		inc eax
	.endw
	inc eax
	shl eax,1
	pop edx
	ret
WideLen endp

;Search the given widestring for given Char  (UTF16-LE CodePoint)
LookChar2 Proc Character:Word, AddrUStr:Dword ;;if found return value is non zero, else zero.

	movzx eax, Character
	mov edx, AddrUStr

@@:
    mov cx, [edx]
    add edx, 2

    cmp ax, cx
    je EndWithSuccess

    or cx, cx ; is zero?
    jnz @B
    xor eax, eax ; indicate failure.

EndWithSuccess:
	ret
LookChar2 endp

;Duplicate a WideString
;Returns EAX = HANDLE of new string
;		 EBX = pNextData (pointer to just PAST the input string)
;		 EDX = #Bytes in string
CloneWideString macro addrStart
LOCAL dLen
	push edi
	mov ebx,addrStart
	mov edi, $invoke (WideLen,ebx)
	;OCall [esi].Strings::StringManager.Alloc, eax
	;push eax
	;OCall [esi].Strings::StringManager.GetChars,eax
	push $MemAlloc(eax,MEM_INIT_ZERO)	
	invoke RtlMoveMemory,eax,ebx,edi
	pop eax
	mov edx,edi
	add ebx,edx
	pop edi
endm

$CloneWideString macro addrStart
	CloneWideString addrStart
	exitm <eax>
endm

DbgReduction proc reduction
LOCAL buf[2048]:byte
LOCAL buf2[2048]:byte
LOCAL buf3[8192]:byte
	pushad
	mov eax,reduction
	xor ebx,ebx
	mov buf3[0],0
	.while bx<[eax].Reduction.TokenCount
		mov buf[0],0
		mov buf2[0],0
		push eax
		Get_Element Token, [eax].Reduction.Tokens,bx
		mov edx,[eax].Token.ParentSymbol
		.if [edx].Symbol.Kind==STERMINAL
			pushad
			invoke GoAscii,addr buf,[edx].Symbol.sName
			popad
			pushad
			invoke GoAscii,addr buf2,[eax].Token.TokenData
			popad
			@invoke wsprintf,addr buf4, "#%s# %s " ,addr buf,addr buf2
			invoke lstrcat,addr buf3,addr buf4
		.elseif [eax].Token.TokenData!=0
			pushad
			invoke GoAscii,addr buf,[edx].Symbol.sName		
			popad
			invoke wsprintf,addr buf2, addr fmt,addr buf, [eax].Token.TokenData
			invoke lstrcat,addr buf3,addr buf2
		.else
			DbgWarning "ouch"
			int 3
		.endif
		pop eax
		inc ebx
	.endw
	DbgStr buf3
	popad
	ret
DbgReduction endp

;=============================================================================================
Var equ DefineVariable
Object Parser,2353453,Primer
	RedefineMethod Init,			Pointer
	RedefineMethod Done
	StaticMethod DestroyParseTree,	Pointer
	StaticMethod LoadGrammarFile,	LPSTR
	StaticMethod LoadProgramFile,	LPSTR
	StaticMethod Parse
	StaticMethod GetSymbolByName,	LPWSTR
	StaticMethod ReleaseReduction,  Pointer
	StaticMethod ReleaseToken,	    Pointer
	
	Private
	  StaticMethod Initialize
	  StaticMethod DiscardRestOfLine
	  StaticMethod GetTopToken,		dword
	  StaticMethod ParseToken,		Pointer
	  StaticMethod PushToken,		dword,Pointer
	  StaticMethod PopToken,		dword
	  StaticMethod RetrieveToken,	Pointer
	  StaticMethod Reset
	  StaticMethod ResetStacks	  
	PrivateEnd
		
	;Variables used for DFA Scanner / tokenizer phase
	Var TextEncodingType,dword,0
	Var	StreamCursor,dword,0
	Var StreamStart,dword,0
	Var	StreamEnd,dword,0
	Var CurrentLine,dword,0
	Var CurrentCol,dword,0
	Var SysLine,dword,0
	Var SysCol,dword,0
	Var CommentLevel
	Var CurrentDFAState,			Pointer
	Var CurrentLALRState,			Pointer
	Var LastSuccessfulTokenString
	
	;Major Arrays
	Var CharSets,Pointer
	Var Symbols,					Pointer
	Var Rules,						Pointer
	Var DFAStates,					Pointer
	Var LALRs,						Pointer
	;Major Array Counts
	Var CharacterSetTableCount, 	WORD
	Var SymbolTableCount,			WORD
	Var RuleTableCount,				WORD	
	Var DFATableCount,				WORD
	Var LALRTableCount,				WORD
	
	TOKENSTACKELEMENTCOUNT equ 128
	TOKENSTACKSIZE equ TOKENSTACKELEMENTCOUNT * sizeof Token 

	Var ErrExpStrList, LPSTR, 128 dup (<?>)
	Var CurrentReduction, 	Reduction, {<>}
		
	Var ErrExpectedCount
	Var ReduceRule
	
	;Informational strings from CGT file
	Var GrammarName,LPWSTR
	Var GrammarVersion,LPWSTR
	Var GrammarAuthor,LPWSTR
	Var GrammarAbout,LPWSTR

	;Initial state
	Var wStartSymbol,				WORD
	Var InitDFA,	 				WORD
	Var InitLALR,	 				WORD	
	
	;Booleans
	Var bTablesLoaded, 				BYTE, FALSE	;byte-length booleans
	Var bCaseSensitive,				BYTE, FALSE
	Var bStreamIsFile,				BYTE, FALSE
	
	;Output
	Var ParseTree,Pointer, NULL		;-> root Reduction

	Embed LALR_TokenStack,DataCollection
	Embed Input_TokenStack,DataCollection
	Embed Reductions,DataCollection
	;Embed Strings, StringManager
ObjectEnd

;=============================================================================================
Method Parser.Init,uses esi,pOwner
	SetObject esi
	ACall esi.Init,pOwner
	OCall [esi].LALR_TokenStack::DataCollection.Init,esi,16,256,-1
	OCall [esi].Input_TokenStack::DataCollection.Init,esi,16,256,-1
	OCall [esi].Reductions::DataCollection.Init,esi,16,256,-1
	;OCall [esi].Strings::StringManager.Init,esi	
	DbgWarning "Parser Initialized"
MethodEnd

Method Parser.ReleaseToken,uses esi,pToken
	SetObject esi
	mov eax,pToken
	mov edx,[eax].Token.ParentSymbol
	.if [edx].Symbol.Kind==STERMINAL
		push eax
		invoke SysFreeString,[eax].Token.TokenData
		pop eax
	.endif
MethodEnd

; Release the tokens of a reduction, but not the object itself ***
Method Parser.ReleaseReduction,uses esi edi ebx,pReduction
	SetObject esi
	mov edi,pReduction
	xor ebx,ebx
	.while bx<[edi].Reduction.TokenCount		
		OCall esi.ReleaseToken,$Get_Element (Token,[edi].Reduction.Tokens, bx)
		inc bx
	.endw
	mov [edi].Reduction.TokenCount,0
MethodEnd




FORINPUT equ 0
FORSTACK equ 1


;=========================================================================================

;Note: called at end of 'LoadGrammarFile' - do not call!
Method Parser.Initialize,uses esi
local tkn:Token	
	;Create a new token:
	;Set the State property to the Initial-LALR-State.
	;Set the Parent-Symbol property to the Start-Symbol.
	;Push onto the Token-Stack.
	SetObject esi
	mov tkn.TokenData,0	
	OCall esi.Reset
	mov tkn.ParentSymbol,$Get_Element (Symbol,[esi].Symbols, [esi].wStartSymbol)
	mov [esi].CurrentLALRState, $Get_Element(LALR,[esi].LALRs, [esi].InitLALR)
	mov tkn.State, eax
	OCall esi.PushToken, FORSTACK, addr tkn
	;Part 2: Set initial values.
	;Other code related to setting line counters, etc... can also be added to this procedure.
	;Set the Comment-Level to 0.
	mov [esi].CommentLevel, 0
MethodEnd

;Reset the three Major Stacks
Method Parser.ResetStacks,uses esi edi ebx
	SetObject esi
	DbgDec [esi].LALR_TokenStack.dCount,"to be released"
	.while [esi].LALR_TokenStack.dCount!=0
		OCall[esi].LALR_TokenStack::DataCollection.DeleteAt,0
		push eax
    	mov edx,[eax].Token.ParentSymbol
    	.if [edx].Symbol.Kind==STERMINAL && [eax].Token.TokenData!=0
	    	invoke SysFreeString, [eax].Token.TokenData
    	.endif
		pop eax
		MemFree eax
	.endw
	DbgText "LALR_TokenStack freed"

	DbgDec [esi].Input_TokenStack.dCount,"to be released"
    .while[esi].Input_TokenStack.dCount!=0
    	OCall [esi].Input_TokenStack::DataCollection.DeleteAt,0
    	push eax
    	mov edx,[eax].Token.ParentSymbol
    	.if [edx].Symbol.Kind==STERMINAL && [eax].Token.TokenData!=0
	    	invoke SysFreeString, [eax].Token.TokenData
    	.endif
    	pop eax
		MemFree eax
    .endw
	DbgText "Input_TokenStack freed"

	DbgDec [esi].Reductions.dCount,"to be released"
    .while[esi].Reductions.dCount!=0
    	OCall [esi].Reductions::DataCollection.DeleteAt,0
    	DbgHex eax
    	push eax
    	mov edi,eax
    	DbgDec [edi].Reduction.TokenCount,"to be released"
    	xor ebx,ebx								;For each token in reduction
    	.while bx<[edi].Reduction.TokenCount
			Get_Element Token, [edi].Reduction.Tokens, bx
			mov edx,[eax].Token.ParentSymbol
			.if [edx].Symbol.Kind==STERMINAL && [eax].Token.TokenData!=0
				push eax
				DbgUStr [eax].Token.TokenData,"releasing terminal string"
				invoke SysFreeString, [eax].Token.TokenData	   ;Free terminal token string
				pop eax
			.endif
			inc ebx
		.endw
		pop eax		
		.if [eax].Reduction.Tokens!=0
			push eax
			MemFree [eax].Reduction.Tokens			;Free reduction tokens array
			pop eax	
		.endif			
		MemFree eax
	.endw
	DbgText "Reductions freed"


MethodEnd

;Reset the Parser
Method Parser.Reset,uses esi
	;DbgText "reset"
	SetObject esi
	mov [esi].CurrentReduction.ParentRule, 0
	mov [esi].CurrentReduction.TokenCount, 0
	;Other fields aren't used.

	mov [esi].ErrExpectedCount, 0
	m2m [esi].StreamCursor,  [esi].StreamStart, edx

	mov [esi].CurrentLine, 0
	mov [esi].CurrentCol, 0
	mov [esi].SysLine, 0
	mov [esi].SysCol, 0
	
	OCall esi.ResetStacks
	.if [esi].bStreamIsFile==TRUE && [esi].StreamStart!=0
    	MemFree [esi].StreamStart
    	mov [esi].StreamStart,0
    .endif

	.if [esi].ParseTree!=0
		mov [esi].ParseTree,0
	.endif
	
MethodEnd

;Load grammar tables from .CGT File
;Returns TRUE/FALSE
Method Parser.LoadGrammarFile, uses esi edi ebx, GrammarFile:LPSTR
Local FileMem:dword
Local FileLen:Dword
Local tempFileMem:DWord
Local MemEnd:Dword
LOCAL File:Pointer
LOCAL nEntries:word
LOCAL nIndex:word
LOCAL RecType:byte
LOCAL byt:byte

	SetObject esi

	.IF [esi].bTablesLoaded!=FALSE
		;No support for INCLUDE yet
    	DbgWarning "Can't load another grammar file within the session, sorry."
		return FALSE
	.ENDIF	  

	;***********load the file*************
	invoke GetFileAttributes,GrammarFile
	.if eax==-1
		DbgWarning "Grammar File does not exist"
		return FALSE
	.else
		mov File,$New(DiskStream,Init,esi,GrammarFile)
		mov FileLen,$OCall(File::DiskStream.GetSize)
		mov FileMem,$MemAlloc(eax,MEM_INIT_ZERO)
		OCall File::DiskStream.BinRead, FileMem, FileLen
		.if eax==0
		    DbgWarning "Failed to read grammar file."
		    ExitMethod
		.endif
	.endif
	
	DbgWarning "Loading Grammar"

	;***********check header*******************
	push $invoke (ToWide, $OfsCStr("GOLD Parser Tables/v1.0")) ;returns a HeapAllocated string
	invoke WideCompare, eax, FileMem	
	pop edx
	push eax
	MemFree edx								;<--- release string
	pop eax
	.if eax!=0
	    DbgWarning "Wrong grammar file header."	    
    	return FALSE
	.endif

	;********************************************

	mov eax, FileMem
	add eax, FileLen
	mov MemEnd, eax

	mov eax, FileMem
	add eax, 48 ; 48th reads 'M'
	mov tempFileMem, eax

	.WHILE 1
	    mov eax, tempFileMem
	    .if eax>=MemEnd
			.IF eax>MemEnd
			    DbgWarning "Stupid parsing error occured while loading the grammar file. Sorry."
			    xor eax, eax
			.ELSE
			    mov eax, TRUE
			.ENDIF
			.break 
		
		.elseif byte ptr[eax]!="M"
			;Each Record begins with this identifier (='MultiType')
			DbgWarning "Unexpected File Format"
			DbgMem tempFileMem,256
			mov eax, FALSE
			.break
		.endif
	    
		; read number of entries in record
		getvsh tempFileMem, nEntries
	    getvb tempFileMem,RecType
		.IF al=='P' ;Parameter information.
			mov ebx, tempFileMem
			
			;Fetch informational strings
			inc ebx; skip 'S' entry identifier (=String)
			mov [esi].GrammarName, $CloneWideString(ebx)		    
			inc ebx
			mov [esi].GrammarVersion,  $CloneWideString(ebx)		    
			inc ebx
			mov [esi].GrammarAuthor,  $CloneWideString(ebx)	    
			inc ebx
			mov [esi].GrammarAbout, $CloneWideString(ebx)
			mov tempFileMem, ebx
  
		    getvb  tempFileMem,[esi].bCaseSensitive
		    getvsh tempFileMem,[esi].wStartSymbol
					  			
		.ELSEIF al=='T' ;TableCount entry		

			;Allocate 'Major' Arrays:
			
			;- Symbols
			getvsh tempFileMem, [esi].SymbolTableCount
		    mov [esi].Symbols,  $AllocArray (Symbol, [esi].SymbolTableCount)

			;- CharSets
			getvsh tempFileMem, [esi].CharacterSetTableCount
		    mov [esi].CharSets,  $AllocArray (LPWSTR, [esi].CharacterSetTableCount)
	
			;-Rules
			getvsh tempFileMem, [esi].RuleTableCount
		    mov [esi].Rules,  $AllocArray (Rule, [esi].RuleTableCount)
	    
			;-DFA States
			getvsh tempFileMem, [esi].DFATableCount
		    mov [esi].DFAStates,  $AllocArray (DFAState, [esi].DFATableCount)

			;-LALR States
			getvsh tempFileMem, [esi].LALRTableCount
		    mov [esi].LALRs,  $AllocArray (LALR, [esi].LALRTableCount)

    	.ELSEIF al=='I' ; Initial State indices
			getvsh tempFileMem, [esi].InitDFA
			getvsh tempFileMem, [esi].InitLALR

    	.ELSEIF al=='C' ; CharSet entry
			getvsh tempFileMem, nIndex
			inc tempFileMem
			mov ebx,tempFileMem
			push $CloneWideString (tempFileMem)
			mov tempFileMem,ebx
			;push eax
			Get_Element LPWSTR,[esi].CharSets,nIndex
			pop [eax]
			;DbgUStr [eax]

     	.ELSEIF al=='S' ; Symbol entry
			getvsh tempFileMem, nIndex
			inc tempFileMem
			mov ebx,tempFileMem
			CloneWideString tempFileMem			;duplicate symbol namestring
			mov tempFileMem, ebx						;fixup for CloneWideString
			push eax
			Get_Element Symbol,[esi].Symbols,nIndex		;look up array element (Symbol)
			pop [eax].Symbol.sName						;store duplicated namestring
			DbgDec nIndex
			DbgUStr [eax].Symbol.sName
			mov edx,eax
			getvsh tempFileMem, [edx].Symbol.Kind		;store kind of Symbol
	
    	.ELSEIF al=='R' ;Rule entry
    		;Rules may contain 0 or more 'Symbol Indices'
			getvsh tempFileMem, nIndex
			Get_Element Rule, [esi].Rules, nIndex
			mov edi,eax
			assume edi:ptr Rule			
			getvsh tempFileMem,  [edi].Nonterminal
			mov ax,nEntries
			sub ax,4	;#symbol indices in this Rule
			mov [edi].SymbolIndexCount,ax
			mov [edi].SymbolIndices,$AllocArray(WORD,[edi].SymbolIndexCount)
			xor ebx,ebx
			inc tempFileMem	;skip 'E' record identifier
			.while bx<[edi].SymbolIndexCount
				Get_Element WORD,[edi].SymbolIndices,bx
				mov edx,eax
				getvsh tempFileMem,[edx]				
				inc bx
			.endw
			Assume edi: Nothing

    	.ELSEIF al=='D' ; DFA State entry
			getvsh tempFileMem, nIndex
			Get_Element DFAState,[esi].DFAStates,nIndex
			mov edi,eax
			getvb  tempFileMem, [edi].DFAState.bAcceptState
			getvsh tempFileMem, [edi].DFAState.wAcceptIndex		
			inc tempFileMem;  reserved
			
			;Calculate #Edges = (Entries-5)/3
			movzx eax,nEntries
			sub eax,5
			mov ebx,3
			xor edx,edx
			div ebx
			mov [edi].DFAState.wEdgeCount,ax	
			.if eax>0
				;Allocate Edges array
				mov [edi].DFAState.Edges,$AllocArray(Edge,[edi].DFAState.wEdgeCount)
				;Read in the Edges
				xor ebx,ebx
				.while bx<[edi].DFAState.wEdgeCount
					Get_Element Edge,[edi].DFAState.Edges, bx
					mov edx,eax
					getvsh tempFileMem, [edx].Edge.wCharSetIndex
					getvsh tempFileMem, [edx].Edge.wTargetIndex
					inc tempFileMem ;reserved
					inc bx
				.endw
			.else
				mov [edi].DFAState.Edges,NULL 
			.endif
		
		.ELSEIF al=='L' ; LALR State entry
			getvsh tempFileMem, nIndex
			inc tempFileMem; reserved
			
			Get_Element LALR,[esi].LALRs,nIndex
			mov edi,eax
			
			;Calculate #Actions = (nEntries-3)/4
			movzx edx,nEntries
			sub edx,3
			shr edx,2
			mov [edi].LALR.wActionCount,dx
			;Allocate array of Actions for this LALR_State
			mov [edi].LALR.Actions,$AllocArray(Action, [edi].LALR.wActionCount)

			xor ebx,ebx
			.while bx<[edi].LALR.wActionCount
				Get_Element Action,[edi].LALR.Actions,bx
				mov edx,eax
				getvsh tempFileMem, [edx].Action.wSymbolIndex
				getvsh tempFileMem, [edx].Action.wActionType
				getvsh tempFileMem, [edx].Action.wTarget				
				inc tempFileMem ;reserved
				inc ebx
			.endw

    	.ELSE
			DbgWarning "Unidentified multitype byte is encountered. Probably the grammar file is corrupt."
			sub eax,4
			DbgMem eax,64
			mov eax, FALSE
			.break
    	.ENDIF
	.ENDW

	Finito:
	push eax			;preserve return value
	MemFree FileMem
	Destroy File
	
	
	DbgHex [esi].Symbols,"After Loading Grammar"
	
	OCall esi.Initialize
	pop eax ;return value
	
	mov [esi].bTablesLoaded, al
MethodEnd

;Load the Program File into memory (all of it, this could be bufferstreamed ???)
;Returns TRUE/FALSE
Method Parser.LoadProgramFile,uses esi, ProgramFile:LPSTR
LOCAL File
	SetObject esi
	invoke GetFileAttributes,ProgramFile
	.if eax==-1
		DbgWarning "Program File does not exist"
		return FALSE
	.endif
	
	mov File,$New(DiskStream,Init,esi,ProgramFile)
	mov [esi].StreamEnd,$OCall(File::DiskStream.GetSize)
	mov [esi].StreamStart,$MemAlloc(eax,MEM_INIT_ZERO)
	OCall File::DiskStream.BinRead, eax,  [esi].StreamEnd	
	.IF eax==0
    	DbgWarning "Failed to open program file."
    	mov [esi].StreamStart, eax
    	mov [esi].StreamEnd, eax
    	Destroy File
		xor eax,eax
    	ExitMethod
	.ENDIF
	Destroy File
	mov eax, [esi].StreamStart
	add [esi].StreamEnd, eax
	mov [esi].StreamCursor, eax
	mov [esi].bStreamIsFile, TRUE
	
	;Try to determine the type of plaintext encoding
	;Check for BOM (Byte Order Mark)
	;If present, check for UTF8, UTF16 and UTF32 variants
	mov edx,[esi].StreamCursor
	.if dword ptr[edx]== 0000FEFFh
		DbgWarning "UTF32 LE is unsupported"
		xor eax,eax
    	ExitMethod

	.elseif dword ptr[edx]== 0FFFE0000h
		DbgWarning "UTF32 BE is unsupported"
		xor eax,eax
    	ExitMethod

	.elseif word ptr[edx]==0FEFFh
		mov [esi].TextEncodingType,TEXT_UTF16_LE
		add [esi].StreamCursor,2

	.elseif word ptr[edx]==0FFFEh
		mov [esi].TextEncodingType,TEXT_UTF16_BE	
		add [esi].StreamCursor,2

	.elseif byte ptr[edx]==	0EFh && word ptr [eax+1]== 0BFBBh
		DbgWarning "UTF8 is unsupported"
		xor eax,eax
    	ExitMethod
    .else
    	;Assume its plain ascii
    	mov [esi].TextEncodingType,TEXT_ANSI
	.endif

	mov eax,TRUE

MethodEnd

Method Parser.Done, uses esi
	SetObject esi
	OCall esi.Reset	;free token stacks and input filestream
	OCall [esi].LALR_TokenStack::DataCollection.Done
	OCall [esi].Input_TokenStack::DataCollection.Done
	OCall [esi].Reductions::DataCollection.Done
	;OCall [esi].Strings::StringManager.Done
	DbgWarning "DONE"
	;Var CharSets,Pointer
	;Var Symbols,					Pointer
	;Var Rules,						Pointer
	;Var DFAStates,					Pointer
	;Var LALRs,						Pointer
	
MethodEnd

; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

;This method scans characters from the input datastream
;and returns them as 'Tokens'
Method Parser.RetrieveToken,uses ebx esi edi, pToken
Local Length_:Dword
Local AcceptLength:Dword
Local AcceptState:Dword ;Symbol
LOCAL buffer[512]:byte
	;DbgText "RetrieveToken"
	SetObject esi
	Mov AcceptState, 0

	;if Source has no more characters,
	;Create a new Token.
	;Set the Type property of Token = End-Of-File.
	;**Set the parentsymbol property of token to the EOF symbol, which has the table index of 0.

	mov ecx, [esi].StreamCursor
	.if ecx>=[esi].StreamEnd
		DbgWarning "DFA reached EOF"
		mov ax,NULL
    	push $Get_Element (Symbol,[esi].Symbols,ax)	;get ptr to 'EOF' symbol
    	mov edx,pToken
    	pop [edx].Token.ParentSymbol
    	mov [edx].Token.TokenData,0
    	mov [edx].Token.State,666
	    ExitMethod
	.endif

	Get_Element DFAState,[esi].DFAStates,[esi].InitDFA
	mov [esi].CurrentDFAState, eax
	mov Length_, 0


;*************
GoForTheToken:
;*************

    ;Part 1:
    ;If the current state accepts a terminal, keep track of the cursor position and the state index. 
    ;This will be used later.

    ;if the State accepts a termimal
    ;Set Accept-State = State.
    ;Set Accept-Length = Length.
    mov eax, [esi].CurrentDFAState
    .IF  [eax].DFAState.bAcceptState==TRUE
		mov AcceptState, $Get_Element (Symbol, [esi].Symbols, [eax].DFAState.wAcceptIndex)
		m2m AcceptLength, Length_, edx
    .ENDIF

    ;Part 2:
    ;Look through the edges of the current state and advance if an edge is found that 
    ;contains the character at Lookahead(Position). 
    ;When no edge is found, a token is created and the loop is exited.

    ;If no Accept State was set then an error token is created. 
    ;A single character is read from the Source. 
    ;This allows error recovery by discarding the character that caused the error.

    ;If an Edge exists  (in current DFA state) that contains the character Lookahead(Length) in Source
    mov eax,   [esi].CurrentDFAState
    mov edi,   [eax].DFAState.Edges
    movzx ecx, [eax].DFAState.wEdgeCount
    
    xor ebx, ebx
    .WHILE ebx<ecx  ;Look in all the edges.
	    push ecx ;save it
	    
	    ;Look up charset by index
	    Get_Element LPWSTR,[esi].CharSets,[edi].Edge.wCharSetIndex
	    mov eax, [eax]	;eax = ptr to charset (from array)
	    
	    ;Grab byte from input stream
	    mov edx, [esi].StreamCursor
	    add edx, Length_
	    .if [esi].TextEncodingType==TEXT_ANSI
		    movzx dx,  byte ptr[edx]			;grabbing a char
		.elseif [esi].TextEncodingType==TEXT_UTF16_LE
			mov dx,  word ptr[edx]			;grabbing a wchar
	    .else
			mov dx,  word ptr[edx]			;grabbing a wchar
			rol dx,8						;swap byte order
		.endif
			    
	    ;Does char exist in charset?		 
	    invoke LookChar2, dx, eax
	    .IF eax!=0
	    	;Switch to target DFA state
			Get_Element DFAState, [esi].DFAStates, [edi].Edge.wTargetIndex
			mov [esi].CurrentDFAState, eax
			;Get more input
			.if [esi].TextEncodingType==TEXT_ANSI
				inc Length_  
			.else
				add Length_,2
			.endif
			pop ecx ; just balance stack. 
			jmp GoForTheToken  
	    .endif
	    
		add edi, sizeof Edge
		pop ecx
		inc ebx
    .ENDW

    ;if the Accept-State was set
    ;Create a new Token.
    ;Set the Parent-Symbol property of Token = symbol in Accept-State.
    ;Set the Data property of Token = read **AcceptLength** characters from Source.
    
    mov eax, AcceptState
    .IF eax!=0
    	;DbgText "DFA accepted input terminal"
    	;DbgDec AcceptLength
	    mov eax, AcceptLength
	    inc eax

	   ; DbgDec eax,"allocating token string memory"
	    .if [esi].TextEncodingType==TEXT_ANSI
	    	invoke RtlZeroMemory,addr buffer,sizeof buffer
	    	invoke MultiByteToWideChar,CP_ACP,MB_COMPOSITE,[esi].StreamCursor,AcceptLength,addr buffer,512
	    	push $invoke (SysAllocString,addr buffer)
	    	
	    .elseif [esi].TextEncodingType==TEXT_UTF16_BE
	    	
	    
	    .else
@@:
		    push $invoke(SysAllocStringLen, [esi].StreamCursor,eax)
		    .if eax==0
		    	DbgWarning "OUT OF MEMORY ERROR"
		    	invoke ExitProcess,0
		    .endif
		    
		.endif
	    mov edx,pToken
	    pop [edx].Token.TokenData
	    m2m [edx].Token.ParentSymbol, AcceptState, eax

		;Copy the accepted input string
	    mov eax, [esi].StreamCursor
	    mov ecx, AcceptLength
	    add [esi].StreamCursor, ecx
	    
;	    
;	    .while ecx!=0
;	    	.if [esi].TextEncodingType==TEXT_ANSI
;	    		movzx dx,byte ptr[eax]
;	    		mov word ptr[ebx],dx
;	    		inc eax
;	    		dec ecx
;	    		
;	    	.else
;	    		mov dx,word ptr[eax]
;	    		.if [esi].TextEncodingType==TEXT_UTF16_BE
;	    			rol dx,8
;	    		.endif
;	    		mov word ptr[ebx],dx
;	    		add eax,2
;	    		sub ecx,2
;	    	.endif
;	    	add ebx,2
;	    .endw
;	    mov word ptr[ebx],0
	    
	    ;mov edx,pToken
	    ;DbgUStr [edx].Token.TokenData

	    ;implement line/col counters.
	    mov eax, [esi].SysLine
	    mov ecx, [esi].SysCol
	    mov [esi].CurrentLine, eax
	    mov [esi].CurrentCol, ecx
	    mov ebx,pToken
	    mov ebx,[ebx].Token.TokenData
	    LCCounter:
		mov al, [ebx]
		.IF al==13
		    inc [esi].SysLine
		    ;DbgDec [esi].SysLine
		    mov [esi].SysCol, 0
		    inc ebx ; for 10 as well
		.ELSEIF al==0
		    jmp ExitDFA
		.ELSE
		    inc [esi].SysCol ; unfortunately, it always runs until the next
		    ;DbgDec [esi].SysCol
		    ;token which results in wrong number. Maybe one day... :)
		.ENDIF
	     inc ebx   
	     jmp LCCounter					     
	    
    .ELSE
    	DbgWarning "UNEXPECTED SYMBOL"
    	int 3
    	int 3
    	int 3
		;note: we don't use that information. it's for future use.
	    ;Create a new Token.
	    ;Set the Data property of Token = read one character from Source.
	    ;**Set the Parent Symbol property of token to the Error symbol which has index=1.
	    MemAlloc 2
	    mov word ptr[eax],0
	    mov ebx, eax ; ebx has text
	    mov ax,1
	    mov edi, $Get_Element (Symbol, [esi].Symbols, ax) ; edi has errror symbol		    
	    mov edx,pToken
	    mov [edx].Token.ParentSymbol, edi		    
	    mov ecx, [esi].StreamCursor
	    inc ecx
	    mov cl, [ecx]
	    mov [ebx], cl		  
	    ExitMethod
    .ENDIF

ExitDFA:
MethodEnd


Method Parser.Parse,uses esi
LOCAL tkn:Token
	SetObject esi
	.if [esi].bTablesLoaded==FALSE
    	mov eax, MessageNotLoaded
    	ExitMethod
	.endif

	.repeat
		;DbgLine
		;DbgWarning "Parsing"
		mov tkn.TokenData,0
		mov tkn.ParentSymbol,0
		mov tkn.State,0
		;DbgDec [esi].SysLine
	    .if [esi].Input_TokenStack.dCount==0				
		    OCall esi.RetrieveToken,addr tkn
			OCall esi.PushToken, FORINPUT, addr tkn
			.if [esi].CommentLevel==0
				mov edx,[eax].Token.ParentSymbol
		    	.if [edx].Symbol.Kind==STERMINAL
					;DbgWarning "Token was retrieved (TERMINAL)"
					;DbgUStr [edx].Symbol.sName
					;DbgUStr [eax].Token.TokenData
					mov eax, MessageTokenRead
					ExitMethod
				.else
					;DbgWarning "Token was retrieved (NONTERMINAL)"
					;DbgUStr [edx].Symbol.sName 
					mov tkn.TokenData,0
					mov eax, MessageTokenRead
					ExitMethod
			    .endif
			.endif	  
			
	    .ELSEIF [esi].CommentLevel>0
	    	;DbgWarning "Handling tokens while COMMENTED"
		    OCall esi.PopToken, FORINPUT
		    mov edx,[eax].Token.ParentSymbol
		    mov dx, [edx].Symbol.Kind  
		    .IF dx==SCOMMENTSTART
			 	inc [esi].CommentLevel
		    .ELSEIF dx==SCOMMENTEND
			 	dec [esi].CommentLevel
		    .ELSEIF dx==SENDOFFILE
			 	mov eax, MessageCommentError
			 	ExitMethod
		    .ENDIF			     
	    .endif
	    
		;System in normal mode.
		OCall esi.GetTopToken, FORINPUT
		mov edx,[eax].Token.ParentSymbol
		;DbgUStr [edx].Symbol.sName,"PROCESSING"
		mov dx, [edx].Symbol.Kind 
		.IF dx==SWHITESPACE
			;DbgWarning "Popping WhiteSpace token"
		    OCall esi.PopToken, FORINPUT
		    .continue		 
		    
		.ELSEIF dx==SCOMMENTSTART
			;DbgWarning "Popping commented token"
		    mov [esi].CommentLevel, 1
		    OCall esi.PopToken, FORINPUT
		    ExitMethod
		    
		.ELSEIF dx==SCOMMENTLINE
			;DbgWarning "Popping commented token, discarding commented line"
		    OCall esi.DiscardRestOfLine
		    OCall esi.PopToken, FORINPUT
		    ExitMethod
		    
		.ELSEIF dx==SERROR
		    mov eax, MessageLexicalError
		    ExitMethod		    
		.endif
		
		; we can finally parse the token :)
		OCall esi.ParseToken, eax   
	    .IF eax==PARSESHIFT
			OCall esi.PopToken, FORINPUT
			mov eax, MessageShift
			ExitMethod
					
	    .ELSEIF eax==PARSEREDUCENORMAL
			mov eax, MessageReduction
			ExitMethod
	       
	    .ELSEIF eax==PARSEACCEPT
	    	;Grab last Reduction = root of parsetree
	    	DbgDec [esi].LALR_TokenStack.dCount
	    	mov eax,[esi].Reductions.dCount
	    	dec eax
	    	mov [esi].ParseTree,$OCall([esi].Reductions::DataCollection.DeleteAt,eax)
			;Throw away the rest of the Reductions collection, because
			;we can use the Tree to find Reductions when we want to.
			OCall [esi].Reductions::DataCollection.DeleteAll
			mov eax, MessageAccept
			ExitMethod
	
	    .ELSEIF eax==PARSESYNTAXERROR		 
			mov eax, MessageSyntaxError
			ExitMethod
	
	    .ELSEIF eax==PARSEINTERNALERROR
			mov eax, MessageInternalError
			ExitMethod 
	
	    .ELSE
			;What is this? Is it possible to end up here?
			DbgWarning "Unexpected return value in parsetoken"
			int 3
	    .ENDIF
    .until 0

	;can't end up here but just in case...
	mov eax, MessageInternalError
MethodEnd

Method Parser.ParseToken, uses esi edi ebx, Tkn
LOCAL ErrExpCount
LOCAL reductiontokens,reduction,ReductionSymbol
LOCAL tkn:Token
LOCAL buf [256]:BYTE
LOCAL Acnt:WORD
	;DbgText "ParseToken"
	SetObject esi
	mov ErrExpCount,0
	
	;mov eax,Tkn
	;DbgHex [eax].Token.ParentSymbol
	;mov eax,[eax].Token.ParentSymbol
	;DbgUStr [eax].Symbol.sName
	
	;if an Action for Token exists in the Current-LALR-State
	mov edx, [esi].CurrentLALRState
	mov ax,[edx].LALR.wActionCount
	mov Acnt, ax
	.if ax!=0
	    mov edi, [edx].LALR.Actions ; edi is an action
    	xor ebx, ebx
    	.while bx<Acnt
	    	Get_Element Symbol,[esi].Symbols, [edi].Action.wSymbolIndex
	    	mov edx,Tkn
	    	.if [edx].Token.ParentSymbol==eax
	    		;DbgWarning "LALR Action Found"
				jmp ActionFound  
	    	.endif
	    	add edi,sizeof Action
	    	inc bx
 	   .endw	   
	.endif

	;********************************
	; COMING HERE IS A SYNTAX ERROR.
	; Lets show which Symbols we expected to see:
	DbgWarning "ERROR: EXPECTED ONE OF THE FOLLOWING SYMBOLS"
	mov eax, [esi].CurrentLALRState
	movzx edx, [eax].LALR.wActionCount
	mov ebx, [eax].LALR.Actions
	xor edi, edi
	mov ErrExpCount,0
	.WHILE edi<edx
	    push edx ;save action count
	    mov ax, [ebx].Action.wActionType
	    ;.IF ax==ACTIONSHIFT		  
		Get_Element Symbol,[esi].Symbols, [ebx].Action.wSymbolIndex
		.IF [eax].Symbol.Kind==STERMINAL
		    mov eax, [eax].Symbol.sName
		    lea edx,[esi].ErrExpStrList
		    mov [edx+edi*4], eax
		    DbgUStr eax
		    inc ErrExpCount
		.else
			push eax
			mov edx, $OfsCStr("<%s>")
			pop eax
			push edx
		    mov eax, [eax].Symbol.sName
		    lea edx,[esi].ErrExpStrList
		    mov [edx+edi*4], eax
		    invoke GoAscii,addr buf,eax
		    pop edx		    
		    DbgPrint edx,,addr buf
		    inc ErrExpCount
		.ENDIF
	    ;.ENDIF
	    inc edi
	    add ebx, sizeof Action
	    pop edx ; restore action count
	.ENDW	 

	m2m [esi].ErrExpectedCount,ErrExpCount,edx
	mov eax, PARSESYNTAXERROR
	ExitMethod
;********************************

ActionFound:

	mov cx, [edi].Action.wActionType
	.IF cx==ACTIONSHIFT
		;DbgWarning "SHIFTING LALR STATE"
;Set Current-LALR-State = target state of Action.
		mov [esi].CurrentLALRState, $Get_Element(LALR,[esi].LALRs, [edi].Action.wTarget)
;Later we will wish to remember which state we were Shifting to
;at the moment that a given Token was moved onto the LALR stack.
;Set the State property of Token to the Current-LALR-State (tag it)
		mov edx,Tkn
		mov [edx].Token.State,eax			
;Push Token onto the LALR-Token-Stack.
		OCall esi.PushToken, FORSTACK, Tkn
;Set Result = Shift
		mov eax, PARSESHIFT
		ExitMethod
		
	.ELSEIF cx==ACTIONREDUCE
		;DbgWarning "REDUCING"		
		mov [esi].ReduceRule, $Get_Element(Rule,[esi].Rules, [edi].Action.wTarget)
		movzx edx,[eax].Rule.SymbolIndexCount
		mov edi,eax
		
		;PART 1: REDUCE THE RULE
		mov [esi].CurrentReduction.ParentRule, eax
		mov [esi].CurrentReduction.TokenCount, dx

		;look up reduction rule's nonterminal symbol
		mov edx, [esi].ReduceRule
		mov di,[edx].Rule.Nonterminal
		mov ReductionSymbol, $Get_Element(Symbol,[esi].Symbols, di) ; Nonterminal Symbol on the stack. (real cpu stack :)

		;Print rule name and #symbols
		invoke GoAscii,addr buf,[eax].Symbol.sName
		movzx edx,[esi].CurrentReduction.TokenCount
		;DbgPrint "Reduction Rule %s has %lu symbols", ,addr buf,edx
		
		;If the Rule's right-hand side contains any symbols,
		;we will create a Reduction container, and move that many tokens
		;from the LALR stack into the new container, and collect the container.
		;I will be moving elements from a DataCollection into a flat array.


		;Allocate a new Reduction		
		;DbgLine "Reduction List"
		mov reduction,$OCall ([esi].Reductions::DataCollection.Insert,$MemAlloc(sizeof Reduction,MEM_INIT_ZERO))
		movzx edx,[esi].CurrentReduction.TokenCount
		;DbgPrint "Reduction %lX : %s = %lu tokens","Reduction List",reduction,addr buf,edx
		
		mov dx,[esi].CurrentReduction.TokenCount
		mov [eax].Reduction.TokenCount,dx
		m2m [eax].Reduction.ParentRule,[esi].CurrentReduction.ParentRule,edx
		;If the Reduction has some Tokens:
	;	.if [edx].Rule.SymbolIndexCount!=0
		
			;Allocate array for Tokens in reduction
			push eax
			mov eax,sizeof Token
			mul [esi].CurrentReduction.TokenCount
			mov reductiontokens,$MemAlloc (eax,MEM_INIT_ZERO)
			pop edx
			mov [edx].Reduction.Tokens,eax
			
			;Move the top N tokens from LALR stack into the Reduction's token array
			xor ebx,ebx
			.while bx<[esi].CurrentReduction.TokenCount
			
				;Steal a token from the top of the LALR token stack
				mov eax,[esi].LALR_TokenStack.dCount
				dec eax
				OCall [esi].LALR_TokenStack::DataCollection.DeleteAt,eax
				
				;Debug its data
				DbgToken eax,"Reduction List"
				
				;Copy its data into the nth reduction-array element
				push eax
				mov ax,[esi].CurrentReduction.TokenCount
				dec ax			;token is at [tokencount-1-bx], ie, top n tokens
				sub ax,bx
				mov edx,$Get_Element(Token, reductiontokens, ax)			;edx -> target token
				pop eax
				m2m [edx].Token.ParentSymbol, 	[eax].Token.ParentSymbol
				m2m [edx].Token.TokenData, 		[eax].Token.TokenData
				m2m [edx].Token.State, 			[eax].Token.State
				;Deallocate the token object, but not its data
				MemFree eax
				inc bx
			.endw
			
		;.endif

		;At this point, the token on top of the LALR stack
		;contains the LALR state which was active when this token
		;was moved across from the input tokenstack to the lalr stack.
		;We should 'revert' the current LALR state to this state.
		MOV [esi].CurrentLALRState,$OCall (esi.GetTopToken, FORSTACK)
		
		;Search LALR State for Action matching our Rule's NonTerminal Symbol
		mov eax, [eax].Token.State			;historical tag from when it was Shifted
		movzx ecx, [eax].LALR.wActionCount
		mov   eax, [eax].LALR.Actions ; eax holds an action
		xor ebx, ebx
		.WHILE ebx<ecx
			mov dx, [eax].Action.wSymbolIndex
			.IF di==dx
				;Current LALR state = Action's target LALR state
				mov edx,eax
				push $Get_Element (LALR,[esi].LALRs, [edx].Action.wTarget)
				mov [esi].CurrentLALRState, eax
				;Create a new Token to represent the Reduction,
				;ie to replace the tokens we stole from the LALR stack	
				pop tkn.State
				m2m tkn.ParentSymbol,ReductionSymbol,edx
				m2m tkn.TokenData, reduction, edx	;**IMPORTANT** tag Reduction tokens with their reduction

				;Push the new token onto the LALR stack, replacement is complete
				OCall esi.PushToken, FORSTACK, addr tkn
				
				;Exit with message
				mov eax, PARSEREDUCENORMAL
				ExitMethod
			.ENDIF
			add eax,sizeof Action
			inc ebx
		.ENDW 


		;;Coming here indicates a serious error.                 
		mov eax, PARSEINTERNALERROR
		ret		
	.ELSEIF cx==ACTIONACCEPT
		mov eax, PARSEACCEPT
		ret
	.ENDIF

	;can't possibly end up in here but just in case:
	mov eax, PARSEINTERNALERROR
MethodEnd


; ллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллллл

Method Parser.PopToken,uses esi, ForWhat:Dword
	SetObject esi
	
	.IF ForWhat==FORINPUT
		mov eax,[esi].Input_TokenStack.dCount
		dec eax
    	OCall [esi].Input_TokenStack::DataCollection.DeleteAt,eax
    	push eax
    	mov edx,[eax].Token.ParentSymbol
    	.if [edx].Symbol.Kind==STERMINAL
	    	.if [eax].Token.TokenData!=0
	    		invoke SysFreeString, [eax].Token.TokenData
	    	.endif
	    .endif
	    pop eax
	    MemFree eax
		;DbgDec  [esi].Input_TokenStack.dCount,"TOKEN POPPED on INPUT stack"
	.ELSE
		mov eax,[esi].LALR_TokenStack.dCount
		dec eax
    	OCall [esi].LALR_TokenStack::DataCollection.DeleteAt,eax
    	.if [edx].Symbol.Kind==STERMINAL
	    	.if [eax].Token.TokenData!=0
	    		push eax
	    		invoke SysFreeString, [eax].Token.TokenData
	    		pop eax
	    	.endif 	
	    .endif
		MemFree eax
		;DbgDec  [esi].LALR_TokenStack.dCount,"TOKEN POPPED on LALR stack"
	.ENDIF

MethodEnd

;Method:	Parser.PushToken
;Purpose:	Clone input data into new Token and push it onto the appropriate stack
;Args:		ForWhat = FORINPUT or FORSTACK (input tokenstack or lalr tokenstack)
;			Tkn		= ptr to input Token structure to be cloned
;Returns:	EAX -> Token
Method Parser.PushToken,uses esi, ForWhat:Dword, Tkn:Pointer
	SetObject esi
	
	.if [esi].Input_TokenStack.pItems==0 || [esi].LALR_TokenStack.pItems==0
		DbgWarning "WTF"
		DbgHex esi
		int 3
	.endif
	
	MemAlloc sizeof Token,MEM_INIT_ZERO
	.if eax==0
		DbgWarning "OUT OF MEMORY"
		invoke ExitProcess,0
	.endif
	.if ForWhat==FORINPUT
	    OCall [esi].Input_TokenStack::DataCollection.Insert,eax 
		;DbgDec  [esi].Input_TokenStack.dCount,"TOKENS PUSHED on INPUT stack"
	.else
	    OCall [esi].LALR_TokenStack::DataCollection.Insert,eax
		;DbgDec  [esi].LALR_TokenStack.dCount,"TOKENS PUSHED on LALR-Token stack"
	.endif	 

    m2m [eax].Token.State, 		  [esi].CurrentLALRState,edx
    mov edx,Tkn
    m2m [eax].Token.ParentSymbol, [edx].Token.ParentSymbol,ecx
    
    ;For tokens whose Symbol is Terminal, the TokenData field = LPSTR
    ;Duplicate string via MemAlloc
    .if ecx!=0 && [ecx].Symbol.Kind==STERMINAL
	    .if [edx].Token.TokenData!=0
	    	push eax
	    	;DbgUStr [edx].Token.TokenData
	    ;	int 3 ;<--- XXXXXXXXXXX these are wide strings now, fix this code
		    CloneWideString [edx].Token.TokenData
		    ;DbgUStr eax
		    pop edx
		    mov [edx].Token.TokenData,eax
		    mov eax,edx
		.endif
	;For tokens whose Symbol is NonTerminal,
	;TokenData = NULL (normal nonterminal token), or
	;TokenData -> Reduction (reduction nonterminal token)
	;Either way, just copy it across
	.else
		push [edx].Token.TokenData
		pop [eax].Token.TokenData
	.endif
	
MethodEnd


Method Parser.GetTopToken,uses esi, ForWhat:Dword
	SetObject esi
	.IF ForWhat==FORINPUT
		;DbgText "getting top INPUT token"
		mov eax,[esi].Input_TokenStack.dCount
		dec eax
		OCall [esi].Input_TokenStack::DataCollection.ItemAt,eax
	.ELSE
		;DbgText "getting top LALR token"
		mov eax,[esi].LALR_TokenStack.dCount
		dec eax
		OCall [esi].LALR_TokenStack::DataCollection.ItemAt,eax
	.ENDIF
MethodEnd

Method Parser.DiscardRestOfLine,uses esi
	SetObject esi
	mov eax, [esi].StreamCursor

LookForCR:
	cmp eax, [esi].StreamEnd
	jge DoneLooking
	
	mov dl, [eax]
	inc eax	
	cmp dl, 13
	jne LookForCR
	
	dec eax ;very important. We should leave the CR in the stream
	;untouched so that it is treated as it is. 
	
	DoneLooking:
	mov [esi].StreamCursor, eax
MethodEnd

;Search for Symbol by Name
;Returns EAX -> Symbol, EDX = SymbolID
;	OR	 EAX = NULL,    EDX = -1
Method Parser.GetSymbolByName,uses esi ebx, pszSymbolName:LPWSTR
	SetObject esi
	xor ebx,ebx
	.while bx<[esi].SymbolTableCount
		Get_Element Symbol, [esi].Symbols, bx
		push eax
		invoke WideCompare,[eax].Symbol.sName,pszSymbolName
		pop edx
		.if eax==0
			mov eax,edx
			mov edx,ebx
			ExitMethod
		.endif
		inc ebx
	.endw
	xor eax,eax
	mov edx,-1
MethodEnd

Method Parser.DestroyParseTree,uses esi edi ebx, pReductionNode
	SetObject esi
	xor ebx,ebx
	mov edi,pReductionNode
	.while bx<[edi].Reduction.TokenCount
		Get_Element Token, [edi].Reduction.Tokens,bx
		mov edx,[eax].Token.ParentSymbol
		.if edx==0
			DbgWarning "Token has no Paretn Symbol?"		
		.elseif [edx].Symbol.Kind==STERMINAL
			.if [eax].Token.TokenData!=0
				invoke SysFreeString, [eax].Token.TokenData
			.endif
		.elseif [eax].Token.TokenData!=0
			OCall esi.DestroyParseTree,[eax].Token.TokenData
		.endif
		inc bx
	.endw
	MemFree [edi].Reduction.Tokens
	MemFree edi
MethodEnd