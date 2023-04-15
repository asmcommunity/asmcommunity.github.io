;The IOJob object is really a wrapper for XOVL (extended OVERLAPPED) structure,
;which in turn is a container for a WSABUF, a buffer for network io :)
; ——————————————————————————————————————————————————————————————————————————————————————————————————
IOJobID equ 73650
Object IOJob,IOJobID,Primer
	RedefineMethod	Done
	RedefineMethod	Init,Pointer,dword			;pClient,bufsize
	VirtualMethod	Reset
    DefineVariable  xovl, XOVL, {<>}
ObjectEnd

if IMPLEMENT
; ——————————————————————————————————————————————————————————————————————————————————————————————————
Method IOJob.Init,uses esi,pClientOwner,bufsize
	ACall Init,pClientOwner
	SetObject esi	
	.if [esi].xovl.pbuforig==0
		mov [esi].xovl.pbuforig,$MemAlloc(bufsize)	
		m2m [esi].xovl.bufsizeorig,bufsize
	.endif
	OCall esi.Reset
	DbgWarning "IOJob Initialized"
MethodEnd

Method IOJob.Reset,uses esi eax
	SetObject esi
	m2m [esi].xovl.wsabuf.buf,	[esi].xovl.pbuforig
	m2m [esi].xovl.wsabuf.len,	[esi].xovl.bufsizeorig
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


; ——————————————————————————————————————————————————————————————————————————————————————————————————
endif