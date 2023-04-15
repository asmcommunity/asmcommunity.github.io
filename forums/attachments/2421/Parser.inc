;This class represents a NAIIVE parser for plaintext files.
;Large Files are supported (64 bit file length).
;Nothing is Known or Assumed about the Grammar or Structure of the plaintext.
;There are two possible modes of operation.
;PUSH MODE will do all the Driving for you.
;PULL MODE requires you to call ParseNextChar in a loop,
;and to break that loop if it returns NULL (=EOF).
;Either way, onParseChar will be automatically called for each input character.

BUFSIZE equ 8192

ParserID equ 83885
Object Parser,ParserID,Primer
	RedefineMethod	Init,				Pointer,LPSTR
	RedefineMethod	Done
	
	StaticMethod	ParseFile,			LPSTR, BOOL		;see notes
	
	;This is the only 
	StaticMethod	ParseNextChar
	Private
		StaticMethod	OpenParseFile,			LPSTR
		StaticMethod	CloseParseFile
		StaticMethod	onBufferExhausted
	PrivateEnd
	
	;You'll probably want to Redefine these in your Derived class or classes..
	DynamicMethod	onParsedFile						;called when Parsing is complete
	DynamicMethod	onParseLine,			LPSTR		;called when a Line or Statement is ready
	DynamicMethod	onParseTerm,			LPSTR		;called when a Term is ready
	DynamicMethod	onParseChar,			LPSTR		;called when a Byte is ready

	DefineVariable	__FILE__,				LPSTR,	0	;This string should remain valid for the lifetime of a call to 'ParseFile'
	DefineVariable	pfile,					Pointer,0	;DiskStream
	DefineVariable	dAvailable,				dword,	0	;#Bytes waiting in buffer
	DefineVariable	pRawInputBuffer,		Pointer,0	;Buffer for raw input
	DefineVariable  dSizeHigh,				dword,	0	;input filesize remaining
	DefineVariable	dSizeLow,				dword,	0
	DefineVariable	dIndex,					dword,	0	;current buffer offset
ObjectEnd

Method Parser.Done,uses esi
	SetObject esi
	;Free the buffer
	MemFree [esi].pRawInputBuffer
		
MethodEnd

Method Parser.Init,uses esi,pOwner,pName	
	SetObject esi
	ACall Init,pOwner	
	;Try to allocate bufferspace
	mov [esi].pRawInputBuffer,$MemAlloc(BUFSIZE+1,MEM_INIT_ZERO)
	mov byte ptr [eax+BUFSIZE],0
	;Parse the input file	
	OCall ParseFile,pName,TRUE
	OCall onParsedFile
MethodEnd

;The following methods are just 'Dummies' used for Debugging
;You shoud redefine them in your Derived class(es)
Method Parser.onParsedFile,uses esi
	DbgWarning "FILE PARSED"
MethodEnd

Method Parser.onParseLine,uses esi,pszLine
	DbgStr pszLine,"Lines"		;Debug Spew
	invoke lstrlen,pszLine
	inc eax
	DbgMem pszLine,eax
MethodEnd

Method Parser.onParseTerm,uses esi ebx,pszTerm
	SetObject esi
	DbgStr pszTerm,"Terms"		;Debug Spew
MethodEnd

Method Parser.onParseChar,uses esi ebx ecx ,pBuf
	mov eax,pBuf
	push dword ptr [eax]		;preserve termination
	mov byte ptr[eax]+1,0		;ZeroTerminate char
	DbgStr pBuf,"Bytes"			;Debug Spew
	pop dword ptr [eax]			;restore termination
MethodEnd

;Reads up to BUFSIZE #bytes from input filestream into buffer
;Returns #Bytes available
Method Parser.onBufferExhausted,uses esi
	SetObject esi
	mov [esi].dIndex,0
	;Read some data into the buffer 
	mov eax,[esi].dSizeLow		;request fileremaining #bytes
	mov edx,BUFSIZE
	.if eax>edx				;if fileremaining > bufferspace
		mov eax,edx			;request bufferspace #bytes
	.endif
	OCall [esi].pfile::DiskStream.BinRead,[esi].pRawInputBuffer,eax
	DbgDec eax,"#BYTES READ"

	mov edx,[esi].pRawInputBuffer
	mov byte ptr[edx+eax],0		;ZeroTerminate the data we just read in
	mov [esi].dAvailable,eax			
	sub [esi].dSizeLow,eax			;Update the FileCounters
	jnc @F
	dec [esi].dSizeHigh		
	@@:	
MethodEnd

;Opens an input filestream
Method Parser.OpenParseFile,uses esi,pszFile
	SetObject esi
	
	DbgWarning "Opening File"
	
	;Set the FileName
	m2m [esi].__FILE__, pszFile
	
	;Try to open input file
	mov [esi].pfile,$New(DiskStream)
	OCall [esi].pfile::DiskStream.Init,[esi].pOwner,pszFile
	invoke GetLastError
	.if eax!=ERROR_ALREADY_EXISTS
		DbgWarning "Error opening file"
		OCall CloseParseFile
		ret
	.endif

	;Get size of input file
	OCall [esi].pfile::DiskStream.GetSize
	mov [esi].dSizeLow,eax
	mov [esi].dSizeHigh,edx
	mov [esi].dAvailable,0

MethodEnd

;Closes an input filestream
Method Parser.CloseParseFile,uses esi
	SetObject esi
	DbgWarning "Closing File"
	Destroy [esi].pfile
	mov [esi].pfile,0
	mov [esi].__FILE__,0
MethodEnd

;Input Params:
;pszFile = ptr to filepath/name
;bFeedMeSeymour = Mode of Operation
;	TRUE  = "PUSH" or "Driving" mode, method won't return until file is totally parsed
;	FALSE = "PULL" or "Driven" mode, method returns immediately, now use GetNextChar !!!
;NOTES:
;Regardless of which Mode you choose, the file will be automatically Closed when totally parsed.
;If you choose PULL mode, watch the retval from GetNextChar for NULL = EOF
;so you can break your character fetching loop.
Method Parser.ParseFile,uses esi,pszFile, bFeedMeSeymour:BOOL
LOCAL pbuf, pfile

	SetObject esi
	OCall OpenParseFile,pszFile
	
	.if bFeedMeSeymour==TRUE	
		;User has selected 'PUSHING' or "DRIVING" mode..
		;We'll do the driving, baby
		.repeat
			.break .if [esi].dSizeLow==0 && [esi].dSizeHigh==0

			.if [esi].dAvailable==0
			;Try to Fetch some more data
				OCall onBufferExhausted
				.break .if eax==0	;Stop if theres no data buffered	
			.endif	

			;Process the data
			mov ebx,[esi].pRawInputBuffer
			xor ecx,ecx
			.while ecx<[esi].dAvailable
				OCall onParseChar,ebx		
				inc ebx
				inc [esi].dIndex
				mov ecx, [esi].dIndex
			.endw
		.until 0
		OCall CloseParseFile
	.endif
MethodEnd

;Possible Return Values are:
;EAX = NULL (ALL DATA EXHAUSTED)
;EAX = ptr (to next available character)
Method Parser.ParseNextChar,uses esi
	SetObject esi
	.if [esi].dAvailable==0
		;Try to Fetch some more data
		OCall onBufferExhausted
		.if eax==0
			push eax
			OCall CloseParseFile
			pop eax
			ret
		.endif	
	.endif	
	mov eax,[esi].pRawInputBuffer
	add eax,[esi].dIndex
;	.if eax!=0
;		OCall onParseChar,eax
;	.endif
MethodEnd

