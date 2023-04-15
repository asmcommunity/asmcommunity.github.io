
;Statements are initially parsed into collections of Tokens representing their Terms..
;These values are used to 'loosely Type' the Terms in a Statement
TYPE_UNKNOWN	equ 0
TYPE_BINARY		equ 1
TYPE_HEX		equ 2
TYPE_INTEGER	equ 3
TYPE_FLOAT		equ 4
TYPE_OPERATOR	equ 5

;And here is the container structure for each Token 
Token struct
	dType	dd ?		;Type of Token - see below
	union
		dValue   dd ?	;Value of Numeric Literal
		pstrTerm dd ?	;ptr to Term string, if Type not identified
	ends
Token ends

;And here is the container structure for a Statement worth of Tokens
Statement struct
	pszStatement	dd ?	;PlainText Statement
	__LINE__		dd ?	;Line at which Statement began
	__FILE__		dd ?	;File from which Statement was sourced
	pTokens			dd ?	;DataCollection of Term Tokens
Statement ends

.data
;The following Characters are 'special operators',
;or are used to create them (such as != or ::)
szTermDelims db "`~!@#$%^&*()_-+={[]}\|;:'",'"<>,./?',0
.code

;This class parses Statements from an input SourceCode file, and stores them
;as a DataCollection of 'Statement Container objects'... each container object
;stored a copy of the plaintext Statement, a collection of Tokens representing
;the TERMS in that Statement, and tags identifying the source File and LineNumber
;from which the Statement was obtained.
;By default, this class supports the use of a MultiLine Terminator character 
;(which can be altered, or set to Zero),
;and takes the liberty of stripping Comments and Empty Lines,
;while correctly tracking and recording Line Numbers.


GLID equ 38858
Object StatementParser,GLID,Parser
	RedefineMethod	Init,Pointer,LPSTR
	RedefineMethod	Done
	RedefineMethod	onParsedFile
	RedefineMethod	onParseLine						;called when a linefeed has been detected
	RedefineMethod	onParseTerm,LPSTR
	RedefineMethod	onParseChar,LPSTR				;called when a byte is ready to be examined	
	;
	StaticMethod	IdentifyTermType,LPSTR
	StaticMethod	MakeToken,LPSTR
	;
	DefineVariable __LINE__,	dword, 1			;Line counter, incremented during Parsing
	DefineVariable __IGNORING__,BOOL,FALSE			;used to suppress comments
	DefineVariable pStatement,	Pointer,0			;Container for input Statement being Parsed
	DefineVariable pStatements,	Pointer,0			;DataCollection of Statement containers
	DefineVariable pszTerm,Pointer,0
	DefineVariable pUniqueTerms,Pointer,0
	DefineVariable specialEOL,			BYTE,';'	;This char will be used to suppress the end of a Line (eg Comments)
	DefineVariable specialMultiLine,	BYTE,'\'	;This char will be used to Continue parsing on Next Line
ObjectEnd


Method StatementParser.IdentifyTermType,uses esi,pstrTerm
	SetObject esi
	invoke IsTermBinary,pstrTerm
	.if $invoke (IsTermBinary,pstrTerm)==TRUE
		mov eax,TYPE_BINARY
	.elseif $invoke (IsTermHex,pstrTerm)==TRUE
		mov eax,TYPE_HEX
	.elseif $invoke (IsTermInteger,pstrTerm)==TRUE
		mov eax,TYPE_INTEGER
	.elseif $invoke (IsTermFloat,pstrTerm)==TRUE
		mov eax,TYPE_FLOAT
	.else
		lea edx,szTermDelims
		.while byte ptr[edx]!=0
			.break .if byte ptr[edx]==al
			inc edx
		.endw
		.if byte ptr[edx]==0
			mov eax,TYPE_OPERATOR
		.else
			mov eax,TYPE_UNKNOWN
		.endif	
	.endif
MethodEnd

Method StatementParser.MakeToken,uses esi edi,pstrTerm
LOCAL ptoken
	SetObject esi
	mov ptoken,$MemAlloc(sizeof Token)	
	;If we can identify the Type of Term, and its some kind of Numeric Literal,
	;we'll convert its Value to a dword and store it for later..
	;Otherwise, we'll just store the StringPointer as the Value field.
	OCall IdentifyTermType,pstrTerm
	mov edi,ptoken
	mov [edi].Token.dType,eax
	.if eax==TYPE_HEX
		invoke h2dw,pstrTerm
	.elseif eax==TYPE_INTEGER
		invoke a2dw,pstrTerm
;	.elseif eax==TYPE_FLOAT
;		invoke String2Float,pstrTerm
;	.elseif eax==TYPE_BINARY
;		invoke bin2dw,pstrTerm
	.else
		mov eax,pstrTerm
	.endif
	mov [edi].Token.dValue,eax
	mov eax,edi
MethodEnd

Method StatementParser.Init,uses esi edi, pOwner, pName:LPSTR
	SetObject esi
	mov [esi].pStatements,$New (DataCollection,Init,pOwner,16,256,-1)
	mov [esi].pUniqueTerms,$New(DataCollection,Init,pOwner,16,256,-1)
	ACall Init,pOwner,pName	
MethodEnd

Method StatementParser.Done,uses esi
	SetObject esi
	DbgWarning "Destroying StatementParser"
	Destroy [esi].pUniqueTerms
	Destroy [esi].pStatements
MethodEnd

Method StatementParser.onParseTerm,uses esi edi,pszTerm
	SetObject esi

	;Send term to debug
	;ACall onParseTerm,pszTerm

	.if [esi].pStatement!=0
		mov edi,[esi].pStatement	
		.if [edi].Statement.pTokens!=0
			;Search in Unique Terms for the input Term
			xor ecx,ecx
			mov edi,[esi].pUniqueTerms
			.while ecx<[edi].DataCollection.dCount
				push ecx
				shl ecx,2
				add ecx,[edi].DataCollection.pItems
				push ecx
				invoke lstrcmp,pszTerm,dword ptr[ecx]
				pop ecx
				.if eax==0
					;Search Succeeded - Use existing Term
					DbgStr pszTerm,"Known Term"
					mov eax,dword ptr[ecx]
					pop ecx
					jmp @F
				.endif
				pop ecx
				inc ecx			
			.endw
			;Search Failed - Learn new Term
			DbgStr pszTerm,"Learning new Term"
			OCall [esi].pUniqueTerms::DataCollection.Insert,$StrNew(pszTerm)
@@:			mov edi,[esi].pStatement		
			OCall [edi].Statement.pTokens::DataCollection.Insert,$OCall (MakeToken, pszTerm)

		.else
			DbgWarning "No Container TermCollection"
			DbgStr pszTerm
		.endif
	.else
		DbgWarning "No Container Statement for Term"
		DbgStr pszTerm
	.endif
MethodEnd

Method StatementParser.onParsedFile,uses esi
	SetObject esi
	DbgDec [esi].__LINE__
	mov eax,[esi].pStatements
	.if eax!=0
		DbgDec [eax].DataCollection.dCount,"#Statements"
	.endif
MethodEnd

;This callback method presents us with a complete input Statement, in plaintext form.
;The statement contains whitespace and grammar terms, but no 'CR/LF'
;We increment a LineCounter, and thunks to its ancestor.
Method StatementParser.onParseLine,uses esi edi ebx
	SetObject esi
	
	mov edi,[esi].pStatement
	.if edi==0
		DbgWarning "NULL STATEMENT CONTAINER in onParseLine"
		ret
	.elseif [edi].Statement.pszStatement==0
		DbgWarning "NULL STATEMENT STRINGPOINTER in onParseLine"
		ret
	.else
		mov eax,[edi].Statement.pszStatement
		.if byte ptr[eax]==0
			DbgWarning "NULL STATEMENT STRING in onParseLine"
			DbgDec [esi].__LINE__
			ret
		.endif
	.endif

	push dword ptr[ebx]
	mov byte ptr [ebx],0
	push ebx

	;This just outputs the line to debug window	
	ACall onParseLine,[edi].Statement.pszStatement
	
	;Duplicate the statement string into its own memory object
	mov [edi].Statement.pszStatement,$StrNew([edi].Statement.pszStatement)
	
	;Store the Statement container object in DataCollection
	OCall [esi].pStatements::DataCollection.Insert,edi
	mov [esi].pStatement,0													;Tell the parser to begin parsing a new Statement
	pop ebx
	pop dword ptr[ebx]
MethodEnd

;This callback method presents us with Bytes of data.
;We're watching for (and counting) the LineFeeds (Ascii 10).
;We partition the input into entire Statements (delimited by LineFeeds)
;and we hand each complete Statement to 'onParsedLine'.
Method StatementParser.onParseChar,uses esi ebx ecx, pBuf
	SetObject esi
	
	mov ebx,pBuf				;Point ebx to the input character
	mov al,byte ptr[ebx]		;Fetch the input character into AL
	
	;The first thing we Check for is "LineFeed" Delimiter
	.if al==10
		inc [esi].__LINE__					;increment LineCounter
		mov [esi].__IGNORING__,FALSE		;Turn OFF the 'ignore input' flag, so the NEXT Line will be considered

		;If we have a Term cached, purge it
		.if [esi].pszTerm!=0 && ebx!=[esi].pszTerm	
			push dword ptr[ebx]
			mov byte ptr[ebx],0
			OCall onParseTerm,[esi].pszTerm
			pop dword ptr[ebx]
			mov [esi].pszTerm,0
		.endif	

		;Check the LAST character in the LINE for 'special MultiLine Terminator'
		;Remember, right now EBX points to a LINEFEED
		;Scan backwards from EBX to find the last NON WHITESPACE character
		mov edx,ebx
		.while byte ptr[edx-1]==32
			dec edx
		.endw
		mov al,byte ptr[edx-1]
		.if al!=0 && al==[esi].specialMultiLine
			;We found a MultiLine Terminator, so we're NOT ready to emit the current Statement,
			;We will bridge the current and next Lines :)
			mov byte ptr[edx-1],32		;Overwrite the 'special MultiLine Terminator' character
			mov byte ptr[ebx],32		;Overwrite the LineFeed
		.else
			;The Line terminated cleanly, so cleanly terminate it :P
			mov byte ptr[edx],0			
			;We are ready to emit a Statement, if we actually have one to emit..
			;Are we currently recording a Statement?
			mov eax,[esi].pStatement
			.if [esi].pStatement!=0 && ebx!=[eax].Statement.pszStatement	;ignore 'null' statements
				OCall onParseLine			;Handball the input to onParseLine	
			.endif
		.endif
		jmp @Done
	.endif	

	;If we are IGNORING (some comment junk), we only care for LineFeed
	.if [esi].__IGNORING__==TRUE
		ret
	.endif	

	;Check for LineComment character
	.if al== [esi].specialEOL
		mov byte ptr[ebx],0				;Overwrite the LineComment character with ZeroTerminator
		mov [esi].__IGNORING__,TRUE		;Enable the 'Ignore' flag (for the duration of the current Line)
	
	;Check for ShiteSpace
	.elseif al==32 || al==13 || al==9
		mov byte ptr[ebx],32	;Overwrite the WhiteSpace with a Space
		;If we have a Term cached, purge it
		.if [esi].pszTerm!=0 && ebx!=[esi].pszTerm	
			push dword ptr[ebx]
			mov byte ptr[ebx],0
			OCall onParseTerm,[esi].pszTerm
			pop dword ptr[ebx]
			mov [esi].pszTerm,0
		.endif	
		ret	
	
	.else
		;Check for Special input character
		lea edx,szTermDelims
		.while byte ptr[edx]!=0
			.break .if byte ptr[edx]==al
			inc edx
		.endw
		;If we're not collecting a Term yet, accept the first reasonable character
		.if [esi].pszTerm==0 && al!=10
			mov [esi].pszTerm,ebx
			jmp @Char
		.endif
		.if byte ptr[edx]==0
			;If we find a NON SPECIAL, check if we have a SPECIAL cached
			.if [esi].pszTerm!=0 && ebx!=[esi].pszTerm	
				mov eax,[esi].pszTerm
				mov al,byte ptr[eax]
				lea edx,szTermDelims
				.while byte ptr[edx]!=0
					.break .if byte ptr[edx]==al
					inc edx
				.endw
				.if byte ptr[edx]==0
					jmp @Char
				.endif
				;Purge the Special		
				mov ebx,[esi].pszTerm	
				push dword ptr[ebx]	
				mov byte ptr[ebx+1],0
				OCall onParseTerm,ebx
				pop dword ptr[ebx]	
				m2m [esi].pszTerm,pBuf
				;Handle the mundane char
				jmp @Char
			.endif
		.endif
		;If we found a Special, check if we have a NON SPECIAL cached
		.if [esi].pszTerm!=0 && ebx!=[esi].pszTerm	
			mov eax,[esi].pszTerm
			mov al,byte ptr[eax]
			lea edx,szTermDelims
			.while byte ptr[edx]!=0
				.break .if byte ptr[edx]==al
				inc edx
			.endw
			.if byte ptr[edx]==0
				;Purge the Non Special	
				push dword ptr[ebx]
				mov byte ptr[ebx],0
				OCall onParseTerm,[esi].pszTerm
				pop dword ptr[ebx]
				m2m [esi].pszTerm,pBuf
				;Handle the Special char
				;jmp @Char
			.endif
		.endif
	.endif
	

		

;Default handling of (assumedly Printable) Chars
@Char:
	.if al>127
		DbgDec al,"WARNING : NON PRINTABLE CHARACTER CODE"
	.endif
	;If we're not yet recording a Statement, point LineStart
	;to the first legal character that we are shown
	.if [esi].pStatement==0 && [esi].__IGNORING__==FALSE
		;Remember the address of the first valid character in the statement
		mov edi, $MemAlloc(sizeof Statement,MEM_INIT_ZERO)
		.if edi!=0
			m2m [edi].Statement.__LINE__,[esi].__LINE__ ;Note the Line this Statement begins on
			m2m [edi].Statement.__FILE__,[esi].__FILE__	;Note the File this Statement appears in
			m2m [edi].Statement.pszStatement,pBuf		;Set start of new Statement
			;Create a collection to hold the tokenized statement
			mov [edi].Statement.pTokens,$New(DataCollection,Init,[esi].pOwner,16,256,-1)
			mov [esi].pStatement,edi					;set statement container
			m2m [esi].pszTerm,pBuf						;set term pointer
		.else
			DbgWarning "Not enough memory"
		.endif
	.endif	

@Done: 
MethodEnd

