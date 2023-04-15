
Object Evaluator, 543453, Interpreter
	RedefineMethod Init, Pointer,Pointer
	StaticMethod Assign,	Pointer
	StaticMethod Add_Exp,	Pointer
	StaticMethod Mult_Exp,	Pointer
	StaticMethod Negate_Exp,Pointer
	StaticMethod Value,		Pointer
ObjectEnd

Register macro thename, thehandler
	OCall pParser::Parser.GetSymbolByName,&thename
	.if eax!=0
		OCall esi.InsertAt, NULL, edx
		OCall esi.InsertAt, 1   , $MethodAddr(Evaluator.&thehandler)
	.else
		DbgWarning "Evaluator.Init failed to register '&thename' handler"
	.endif
endm

Method Evaluator.Init,uses esi,pOwner,pParser
	SetObject esi
	ACall Init,pOwner,pParser
	
	;Get Symbol
	.data
	szStatements db "S",0,"t",0,"a",0,"t",0,"e",0,"m",0,"e",0,"n",0,"t",0,"s",0,0,0	
	szStatement  db "S",0,"t",0,"a",0,"t",0,"e",0,"m",0,"e",0,"n",0,"t",0,0,0
	szAssign 	 db "A",0,"s",0,"s",0,"i",0,"g",0,"n",0,0,0
	szExpression db "E",0,"x",0,"p",0,"r",0,"e",0,"s",0,"s",0,"i",0,"o",0,"n",0,0,0
	szAdd_Exp    db "A",0,"d",0,"d",0," ",0,"E",0,"x",0,"p",0,0,0
	szMult_Exp   db "M",0,"u",0,"l",0,"t",0," ",0,"E",0,"x",0,"p",0,0,0
	szNegate_Exp db "N",0,"e",0,"g",0,"a",0,"t",0,"e",0," ",0,"E",0,"x",0,"p",0,0,0
	szValue		 db "V",0,"a",0,"l",0,"u",0,"e",0,0,0
	.code
	
	
	;Register <Statements> handler (default)	
	Register <addr szStatements>, DefaultHandler
	Register <addr szStatement>,  DefaultHandler
	Register <addr szAssign>,     Assign
	Register <addr szExpression>, DefaultHandler
	Register <addr szAdd_Exp>, 	  Add_Exp
	Register <addr szMult_Exp>,   Mult_Exp
	Register <addr szNegate_Exp>, Negate_Exp
	Register <addr szValue>, 	  Value

	DbgLine
MethodEnd



;The <Value> Rule has 5 forms:
;<Value> ::= <SimpleValue>
;<Value> ::= '(' <Expression> ')'
;<Value> ::= Id '++'
;<Value> ::= Id '--'
;<Value> ::= Id
Method Evaluator.Value,uses esi,pReductionNode
	SetObject esi	
	mov eax,pReductionNode
	.if [eax].Reduction.TokenCount==1
		;Given the Forms in the method comments, we can see that
		;when the tokencount is 1, we must be looking at an Id...
		;Let's return the Id token, as it is Terminal.
		DbgWarning "Collapsing Value node, returning Id"
		Collapse	;<-- return a cloned copy of the (only) token
	.else
		DbgWarning "Unhandled case: resolve a Value Exp"
		int 3
		;Resolve the components of the statement
		OCall esi.DefaultHandler, pReductionNode
	.endif
MethodEnd

;The <Negate Exp> Rule has 3 forms:
;<Negate Exp> ::= '-' <Value>
;<Negate Exp> ::= <Value>
Method Evaluator.Negate_Exp,uses esi,pReductionNode
	SetObject esi
	;Resolve the components of the statement
	OCall esi.DefaultHandler, pReductionNode
	
	mov edx,pReductionNode
	.if [edx].Reduction.TokenCount==1
		ExitMethod

	.elseif [edx].Reduction.TokenCount==2
		;We need to negate the sign of the Value
		DbgWarning "Need to Negate"
		int 3
	.else
		DbgWarning "Evaluator.Negate_Exp: Unexpected token count"
		int 3
	.endif

MethodEnd

;The <Mult Exp> Rule has 3 forms:
;<Mult Exp> ::= <Mult Exp> '*' <Negate Exp>
;<Mult Exp> ::= <Mult Exp> '/' <Negate Exp>
;<Mult Exp> ::= <Negate Exp>
Method Evaluator.Mult_Exp,uses esi,pReductionNode
	SetObject esi
	;Resolve the components of the statement
	OCall esi.DefaultHandler, pReductionNode
	
	mov edx,pReductionNode
	.if [edx].Reduction.TokenCount==1
		ExitMethod
	.else
		DbgWarning "Evaluator.Mult_Exp: Unexpected token count"
		int 3
	.endif
MethodEnd

;The <Add Exp> Rule has 5 forms:
;<Add Exp> ::= <Add Exp> '+' <Mult Exp>
;<Add Exp> ::= <Add Exp> '-' <Mult Exp>
;<Add Exp> ::= <Add Exp> '&' <Mult Exp>
;<Add Exp> ::= <Add Exp> '|' <Mult Exp>
;<Add Exp> ::= <Mult Exp>
Method Evaluator.Add_Exp,uses esi,pReductionNode
	SetObject esi
	;Resolve the components of the statement
	OCall esi.DefaultHandler, pReductionNode
	
	mov edx,pReductionNode
	.if [edx].Reduction.TokenCount==1
		ExitMethod
	.else
		DbgWarning "Evaluator.Add_Exp: Unexpected token count"
		int 3
	.endif
MethodEnd

;The <Assign> Rule has 3 forms:
;<Assign> ::= <Assign> '=' <Expression>
;<Assign> ::= '=' <Expression>
;<Assign> ::= <Assign Sub> 
Method Evaluator.Assign,uses esi,pReductionNode
	SetObject esi
	;Resolve the components of the Assign statement
	OCall esi.DefaultHandler, pReductionNode
	
	mov edx,pReductionNode
	movzx edx,[edx].Reduction.TokenCount
	.if edx==1
		DbgWarning "Assign Sub isnt implemented"
		int 3
	.elseif edx==2
		DbgWarning "Assign = Expression isnt implemented"
		int 3
	.elseif eax==3
		DbgWarning "Assign = Assign = Expression isnt implemented"
		int 3
	.endif
MethodEnd

