

;The interpreter uses ancestor DwordCollection to store
;pairs of dwords where the first dword is the index of
;a solvable Rule's NonTerminal Symbol,
;and the second dword is a Pointer to an 'Action' Handler Method.
;
;The START SYMBOL is automatically registered (see Init method).
;Derived interpreter classes must register any extra handlers.
;See example 'Action_START' method, keep same params etc.
;
;The Interpret method will walk the given 'ParseTree'.
;At each Node, the (nonterminal) child branches will be all be recursed.
;After recursion returns, the child subtree(s) will usually be much simplified.
;At this point, the Interpret method will compare the ID of the current node's RuleSymbol
;to its internal list of registered Actions, and if a match is found,
;a call will be made to the corresponding Handler, passing it the current Node.

;The job of each Handler then? Mainly, its to 'Simplify the expression' (subtree)
;by 'solving the problem posed by the current node', and storing 
;the 'answer' (if any) in a (recycled) Terminal token, and returning its pointer.


Object Interpreter, 564645, DwordCollection
	RedefineMethod Init, Pointer,Pointer
	StaticMethod   Interpret,Pointer
	DynamicMethod  Action_START,Pointer
	StaticMethod   DefaultHandler,Pointer
	Private
	  StaticMethod GetSolver,Pointer
	PrivateEnd
	DefineVariable pParser,Pointer
ObjectEnd

;Method:	Interpreter.Action_START
;Purpose:	Action callback handler for 'Start' rule
;Args:		pReductionToken = ptr to Token for a node in Reduction Tree
;Returns:	pResultToken, pReductionToken, or NULL
;Remarks:	Since no special processing is required by this Reduction,
;			execution is passed to the 'default reduction handler'.
;			You can use the same technique for handling 'unimportant nodes'.
;			IE just register them directly to the Resolve method.
;			Just be sure to let the return value from DefaultHandler fall thru ;)
Method Interpreter.Action_START,uses esi, pReductionNode
	SetObject esi
	OCall esi.DefaultHandler, pReductionNode
MethodEnd

;Method:	Interpreter.Init
;Purpose:	Initialize the interpreter
;Args:		pOwner  = ptr to Owner  object
;			pParser = ptr to Parser object
;			dIndex_Of_IF_Symbol_Or_NULL = index of symbol for IF DIRECTIVE
;			(the 'if' compiler switch, not the .if directive)
Method Interpreter.Init, uses esi ebx, pOwner, pParser
	SetObject esi
	;Initialize ancestor DwordCollection (used to store Action Callback registrations)
	ACall Init, pOwner, 16,256,-1
	mov [esi].dDuplicates,FALSE
	m2m [esi].pParser, pParser, edx
	
	;Register START Symbol's Action Handler
	movzx edx,[edx].Parser.wStartSymbol
	OCall esi.InsertAt, NULL, edx
	OCall esi.InsertAt, 1   , $MethodAddr(Interpreter.Action_START)
	
MethodEnd


;Method:	Interpreter.GetSolver
;Purpose:	Search Action Handlers for match
;Args:		dSymbolID = index of nonterminal symbol (reduction rule)
;Returns:	
Method Interpreter.GetSolver,uses esi ebx, dSymbolID
LOCAL cnt
	SetObject esi
	mov eax,[esi].dCount
	shr eax,1
	mov cnt,eax
	xor ebx,ebx
	.while ebx<cnt
		mov eax,[esi].pItems
		mov edx,[eax+ebx*8]
		DbgDec edx
		DbgDec dSymbolID
		.if edx==dSymbolID
			mov eax,[4+eax+ebx*8]
			ExitMethod
		.endif
		inc ebx
	.endw
	xor eax,eax
MethodEnd


;Method:	Interpreter.Interpret
;Purpose:	Determine which handler matches the input reduction
;			and pass execution (recursively) to that handler.
Method Interpreter.Interpret, uses esi edi ebx, pReductionNode:Pointer
 
	SetObject esi
	mov edi,pReductionNode	
	mov edi,[edi].Reduction.ParentRule  ;edi -> Rule which this Reduction used
	
	;Look up Rule's NonTerminal Symbol by index	
	mov eax,[esi].pParser
	Get_Element Symbol,[eax].Parser.Symbols,[edi].Rule.Nonterminal
	DbgUStr [eax].Symbol.sName,"Interpreter is looking at this Reduction"

	;Convert Symbol into an Index into the Parser.Symbols array (ie, SymbolID)
	mov edx,[esi].pParser
	sub eax,[edx].Parser.Symbols
	mov ebx,sizeof Symbol
	xor edx,edx
	div ebx
		
	;Search for a registered Handler for this SymbolID
	OCall esi.GetSolver, eax
	.if eax!=0
		DbgWarning "Located Handler"
		;Call the Handler, passing it the entire Reduction
		push pReductionNode
		push esi
		call eax
	.else
		DbgWarning "Failed to locate handler for Reduction"
	.endif	
	
MethodEnd

;Macro used to collapse resolved nodes of the reduction tree 
Collapse macro
	xor edx,edx		
	Get_Element Token, [eax].Reduction.Tokens, dx
	push eax 
	;Duplicate the token 
	MemAlloc sizeof Token
	pop edx
	m2m [eax].Token.ParentSymbol,[edx].Token.ParentSymbol
	m2m [eax].Token.TokenData,[edx].Token.TokenData
	m2m [eax].Token.State,[edx].Token.State
	mov [edx].Token.TokenData,NULL	;stop this being released
	ExitMethod     ;return Id token
endm

;Method:	Interpreter.Resolve
;Purpose:	Default Handler for Reductions
;Args:		pReductionNode -> Reduction
;Remarks:	Recurses each NonTerminal child token in the Reduction
Method Interpreter.DefaultHandler, uses esi edi ebx, pReductionNode
	SetObject esi
	mov  edi,pReductionNode
	.if [edi].Reduction.TokenCount==0
		DbgWarning "Reduction has no tokens"
	.else
		xor ebx,ebx
		.while bx<[edi].Reduction.TokenCount
			Get_Element Token, [edi].Reduction.Tokens, bx
			
			mov edx,[eax].Token.ParentSymbol
			DbgUStr [edx].Symbol.sName
			.if [edx].Symbol.Kind==STERMINAL
				DbgUStr [eax].Token.TokenData
			.else
				;In reductions, NonTerminals indicate child reductions..
				;Recurse the Child reduction.
				push eax
				OCall esi.Interpret,[eax].Token.TokenData				
				pop edx				
				.if eax!=edx ;NonTerminal token was resolved!!!! : replace it...					
					;First we will destroy the old subtree (which we resolved)
					push eax
					push edx
					OCall [esi].pParser::Parser.DestroyParseTree,[edx].Token.TokenData
					pop edx
					pop eax
					;Second we will overwrite the resolved NonTerminal token
					m2m [edx].Token.ParentSymbol,[eax].Token.ParentSymbol
					m2m [edx].Token.TokenData,[eax].Token.TokenData
					m2m [edx].Token.State,[eax].Token.State
					;Third we will free the 'transient token' used to convey the data to us
					MemFree eax	;free transient token
					;Now the NonTerminal token has been replaced
				.endif
			.endif
			inc ebx
		.endw
	.endif
	
	;There's only one token, 'collapse' on it (return a clone of it)
	mov eax,pReductionNode
	.if [eax].Reduction.TokenCount==1
		DbgWarning "DefaultHandler is Collapsing node of One Token"
		Collapse
	.endif
MethodEnd

