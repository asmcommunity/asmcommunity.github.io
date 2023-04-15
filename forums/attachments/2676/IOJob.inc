;The IOJob object is really a wrapper for XOVL (extended OVERLAPPED) structure,
;which in turn is a container for a WSABUF, a buffer for network io :)
; ——————————————————————————————————————————————————————————————————————————————————————————————————
IOJobID equ 73650
Object IOJob,IOJobID,Primer
	RedefineMethod	Done
	RedefineMethod	Init,Pointer,dword			;pClient,bufsize
	VirtualMethod	Reset
	VirtualMethod   Cut,dword
    DefineVariable  xovl, XOVL, {<>}
ObjectEnd

if IMPLEMENT
; ——————————————————————————————————————————————————————————————————————————————————————————————————
Method IOJob.Init,uses esi,pClientOwner,bufsize
	ACall Init,pClientOwner
	DbgDec bufsize,"Initializing IOJob"
	SetObject esi	
	.if [esi].xovl.pbuforig==0
		mov [esi].xovl.pbuforig,$MemAlloc(bufsize)	
	.else
	    DbgWarning "IOJob.Init is using recycled buffer"
	    DbgHex [esi].xovl.pbuforig
	.endif
	m2m [esi].xovl.bufsizeorig,bufsize,edx
	OCall esi.Reset
	DbgWarning "IOJob Initialized"
MethodEnd

Method IOJob.Reset,uses esi eax
	SetObject esi
	m2m [esi].xovl.wsabuf.buf,	[esi].xovl.pbuforig,    edx
	m2m [esi].xovl.wsabuf.len,	[esi].xovl.bufsizeorig, edx
	mov [esi].xovl.bytesused,	0
	mov [esi].xovl.bytes,0
	mov [esi].xovl.piojob,esi
	invoke RtlZeroMemory,addr [esi].xovl.ovl, sizeof WSAOVERLAPPED
	DbgWarning "IOJob Reset"
MethodEnd

Method IOJob.Done,uses esi
	SetObject esi
	MemFree [esi].xovl.wsabuf.buf	
MethodEnd

;This method will remove the given #bytes from the START of the buffer.
;Returns FALSE if data remains in the buffer, TRUE if the buffer is now empty
Method IOJob.Cut,uses esi,dBytes
    SetObject esi
    mov eax,dBytes
	.if eax<[esi].xovl.bytesused
		;We will be leaving some data in the buffer,
		;so we'll need to copy unconsumed data to start of buffer
		sub [esi].xovl.wsabuf.buf,eax
		sub [esi].xovl.bytesused,eax
		mov edx,[esi].xovl.pbuforig
		add edx,eax
		invoke RtlMoveMemory,[esi].xovl.pbuforig,edx,[esi].xovl.bytesused
		mov eax,FALSE
	.else;if eax >= [esi].xovl.bytesused
	    OCall esi.Reset
	    mov eax,TRUE
	.endif
MethodEnd
; ——————————————————————————————————————————————————————————————————————————————————————————————————
endif