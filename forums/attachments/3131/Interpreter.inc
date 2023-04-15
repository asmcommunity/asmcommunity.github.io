;Class:		Interpreter
;Purpose:	BaseClass for parsetree interpreter
;Remarks:	See Parser, Evaluator

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


Object Interpreter, 564645, Parser
	RedefineMethod Init, Pointer
	RedefineMethod Done
	StaticMethod   Interpret,Pointer
	DynamicMethod  Action_START,Pointer
	StaticMethod   DefaultHandler,Pointer
	StaticMethod   RegisterHandler,Pointer,Pointer
	;Private
	  StaticMethod GetSolver,Pointer
	  StaticMethod   Stitch,Pointer,Pointer,dword
	;PrivateEnd
	Embed Registry, DwordCollection		;Stores registrations for Reduction Handlers
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

Method Interpreter.Done,uses esi
	SetObject esi
	OCall [esi].Registry::DwordCollection.Done
	ACall Done	
MethodEnd

;Method:	Interpreter.Init
;Purpose:	Initialize the interpreter
;Args:		pOwner  = ptr to Owner  object
;			pParser = ptr to Parser object
;			dIndex_Of_IF_Symbol_Or_NULL = index of symbol for IF DIRECTIVE
;			(the 'if' compiler switch, not the .if directive)
Method Interpreter.Init, uses esi, pOwner
	SetObject esi
	ACall Init, pOwner
	;Initialize ancestor DwordCollection (used to store Action Callback registrations)
	OCall [esi].Registry::DwordCollection.Init,esi,16,256,-1
	mov [esi].Registry.dDuplicates,FALSE
	
	;Register START Symbol's Action Handler
	movzx edx,[esi].wStartSymbol
	OCall [esi].Registry::DwordCollection.InsertAt, NULL, edx
	OCall [esi].Registry::DwordCollection.InsertAt, 1   , $MethodAddr(Interpreter.Action_START)
	DbgWarning "Interpreter initialized"
MethodEnd


;Method:	Interpreter.GetSolver
;Purpose:	Search Action Handlers for match
;Args:		dSymbolID = index of nonterminal symbol (reduction rule)
;Returns:	EAX -> CallBack Handler
;	or		EAX = FALSE (NULL)
Method Interpreter.GetSolver,uses esi ebx, dSymbolID
LOCAL cnt
	SetObject esi
	mov eax,[esi].Registry.dCount
	shr eax,1
	mov cnt,eax
	xor ebx,ebx
	.while ebx<cnt
		mov eax,[esi].Registry.pItems
		mov edx,[eax+ebx*8]
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
;			and execute that handler, then return to caller.
Method Interpreter.Interpret, uses esi edi ebx, pReductionNode:Pointer 
	SetObject esi
	mov edi,pReductionNode	
	mov edi,[edi].Reduction.ParentRule  ;edi -> Rule which this Reduction used
	.if edi==0
		DbgWarning "Returned to Root Reduction"
		ExitMethod
	.endif
	;Look up Rule's NonTerminal Symbol by index	
	Get_Element Symbol,[esi].Symbols,[edi].Rule.Nonterminal
	DbgUStr [eax].Symbol.sName,"Interpreting Reduction:"
	mov edx,pReductionNode
	.if [edx].Reduction.TokenCount==0
		ExitMethod
	.endif
	invoke DbgReduction, pReductionNode

	;Convert Symbol into an Index into the Parser.Symbols array (ie, SymbolID)
	sub eax,[esi].Symbols
	mov ebx,sizeof Symbol
	xor edx,edx
	div ebx
		
	;Search for a registered Handler for this SymbolID
	OCall esi.GetSolver, eax
	.if eax!=0
		;DbgHex esp,"in 2"
		;Call the Handler, passing it the entire Reduction
		push pReductionNode
		push esi
		call eax
		;DbgHex esp,"out 2"
	.else
		;Failed to find a registered handler for this reduction..
		;Use default processing (expand parsetree)
		;DbgHex esp,"in"
		OCall esi.DefaultHandler,pReductionNode
		;DbgHex esp,"out"
	.endif	
	
;	DbgWarning "EXPANDED"
;	invoke DbgReduction, pReductionNode
	
MethodEnd


;Method:	Interpreter.Resolve
;Purpose:	Default Handler for Reductions
;Args:		pReductionNode -> Reduction
;Remarks:	Recurses each NonTerminal child token in the Reduction
Method Interpreter.DefaultHandler, uses esi edi ebx, pReductionNode
LOCAL pToken, pReduction
	SetObject esi

	mov  edi,pReductionNode
	.if [edi].Reduction.TokenCount==0
		DbgWarning "Reduction has no tokens"
		int 3
	.else
		movzx ebx,[edi].Reduction.TokenCount
		dec ebx
		xor ebx,ebx
		.while bx<[edi].Reduction.TokenCount
			DbgDec ebx,"index"
			mov pToken,$Get_Element (Token, [edi].Reduction.Tokens, bx)
			
			mov edx,[eax].Token.ParentSymbol
			.if [edx].Symbol.Kind!=STERMINAL
				;In reductions, NonTerminals indicate child reductions..
				push [eax].Token.TokenData	
				pop pReduction
				
				;If this child reduction has content, 'interpret' it
				mov edx,pReduction
				.if [edx].Reduction.TokenCount!=0
					OCall esi.Interpret,pReduction		
					.if eax==-1
						DbgWarning "Exiting with Error"
						ExitMethod
					.endif
				.endif
				
				;Insert the expanded tokens of pReduction into pReductionNode
				;at the current index (EBX) .. this will also delete pReduction
				OCall esi.Stitch, pReductionNode, pReduction, ebx
				.if eax==-1
					;The parent reduction just got SMALLER
					;This can only happen if the child was totally solved.
					;We need to 'correct' the loop counter, because
					;we just deleted (from pReductionNode) the token
					;which we are currently processing - the count changed.
					dec ebx			;Fixup the counter, cuz #tokens got smaller
					xor eax,eax
				.else
					;dec ebx
					add ebx,eax		;We can skip the N new tokens we pasted in our sequence
				.endif
				
			.endif
			inc ebx
		.endw
	.endif
	
MethodEnd


;Method:	Interpreter.Stitch
;Purpose:	Replace a NonTerminal child with its 'expanded' content tokens
;			IE "stitch the result of Child into the parent's token sequence"
;Returns:	EAX = FALSE (ok) or TRUE (need to fix token counter in Interpret method
;Remarks:	This is a cut-and-paste function: TAR[source]GET
;Args:		pTargetReduction -> Reduction to be 'extended'
;			pSourceReduction -> Reduction to be 'inserted'
;			dwTargetIndex = index in Target.Tokens to insert at
Method Interpreter.Stitch,uses esi ebx, pTargetReduction, pSourceReduction, dwTargetIndex
LOCAL pTokens,nTokens
LOCAL dLeadBlock
LOCAL SizeDiff

	SetObject esi
	mov dLeadBlock,0
	
	;Allocate a new array to contain (source+target-1) tokens
	mov eax,pTargetReduction
	movzx edx,[eax].Reduction.TokenCount
	mov SizeDiff,edx
	mov eax,pSourceReduction
	movzx eax,[eax].Reduction.TokenCount
	add edx,eax
	dec edx							;edx = new #tokens
	mov nTokens, edx				; ** this CAN be zero, and its OK **
	mov eax,sizeof Token
	mul edx
	mov pTokens,$MemAlloc (eax)
	
	;If we're not inserting at location index 0
	.if dwTargetIndex>0
		;We need to retain some leading tokens from the Target sequence
		mov eax,sizeof Token
		mul dwTargetIndex
		mov dLeadBlock,eax
		mov edx,pTargetReduction
		invoke RtlMoveMemory,pTokens,[edx].Reduction.Tokens,eax
	.endif

	;Insert copy of main body of tokens 
	;Calculate size of token sequence we are 'importing'
	mov edx,pSourceReduction
	mov eax,sizeof Token
	mul [edx].Reduction.TokenCount
	mov ebx,dLeadBlock
	add ebx,pTokens
	add dLeadBlock,eax
	mov edx,pSourceReduction
	invoke RtlMoveMemory,ebx,[edx].Reduction.Tokens,eax
	
	;If theres any remaining tokens from the original Target sequence
	;then we better stick those on the end
	mov eax,pTargetReduction
	movzx eax,[eax].Reduction.TokenCount
	inc dwTargetIndex		;<--- skip redundant old nonterminal token
	.if eax>dwTargetIndex
		sub eax,dwTargetIndex
		mov edx,eax
		mov eax,sizeof Token
		mul edx
		push eax
		
		mov eax,sizeof Token
		mul dwTargetIndex
		mov edx,pTargetReduction
		add eax,[edx].Reduction.Tokens		
		pop edx
		mov ebx,pTokens
		add ebx,dLeadBlock
		invoke RtlMoveMemory, ebx, eax, edx
	.endif
	
	;Trash the Source Reduction
	mov edx,pSourceReduction
	MemFree [edx].Reduction.Tokens
	MemFree pSourceReduction
	
	;trash the old Target Reduction.Tokens array
	mov edx,pTargetReduction
	MemFree [edx].Reduction.Tokens
	
	;replace the Reduction.Tokens array
	mov edx,pTargetReduction
	m2m [edx].Reduction.Tokens, pTokens, eax
	mov eax,nTokens
	mov [edx].Reduction.TokenCount,ax
	
	mov eax,nTokens
	sub eax,SizeDiff

MethodEnd

Method Interpreter.RegisterHandler,uses esi,pSymbolName,pHandler
	SetObject esi
	OCall esi.GetSymbolByName,pSymbolName
	.if eax!=0
		OCall [esi].Registry::DwordCollection.InsertAt, NULL, edx
		OCall [esi].Registry::DwordCollection.InsertAt, 1   , pHandler
	.else
		DbgWarning "Interpreter failed to register handler: Symbol Not Found"
		DbgUStr pSymbolName
	.endif
MethodEnd
