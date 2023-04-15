;==========================================================

WSAOVERLAPPED struct
    Internal DWORD ?
    InternalHigh DWORD ?
    an_Offset DWORD ?
    OffsetHigh DWORD ?
    hEvent WSAEVENT ?
WSAOVERLAPPED ends

;==========================================================

WSABUF struct
    len DWORD ?
    buf PBYTE ?
WSABUF ends

;==========================================================

ClientID equ 93846
Object Client,ClientID,Primer
	;Override the inherited Primer.Init
	RedefineMethod Init,Pointer,dword	 ;->Owner, FirstOperation
	;Override the inherited Primer.Done
	RedefineMethod Done
	;Data Members are effectively an Extended Overlapped struct
	;but we must give address of OVL rather than simply pClient
	;due to ObjAsm32 consuming first 12 bytes of each Object instance.
	DefineVariable OVL, WSAOVERLAPPED, {}
	DefineVariable hSocket,dword,NULL
    DefineVariable operation,dword,0
    DefineVariable pbuffer,dword,0
    DefineVariable bytes,dword,0
    DefineVariable wsabuf,WSABUF,{}
    DefineVariable wsabuf_count,dword,0	
ObjectEnd
;============================================
;CONSTRUCTOR METHOD
;Returns Success=TRUE or FALSE.
;============================================
Method Client.Init, uses esi, pOwner, FirstOperation
LOCAL sock:dword
LOCAL pbuf:dword
LOCAL opt:dword
	;Create a Socket
    invoke WSASocket, AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED
    IfCase eax==NULL
    	mov eax,FALSE
    	ExitMethod
    EndCase
    mov sock, eax
    
	;Set socket options
    mov opt, 0
    lea edx, opt
    invoke setsockopt, sock, IPPROTO_TCP, TCP_NODELAY, addr opt, 1
    .if eax!=0
    	mov eax,FALSE
    	ExitMethod
    .endif
    
    ;Allocate bufferspace
    mov pbuf, $MemAlloc(4096,HEAP_ZERO_MEMORY)
    
	;Store socket and pbuffer in Client.hSocket
    SetObject esi
    m2m [esi].hSocket, sock
    m2m [esi].pbuffer, pbuf
    ;Initialize the first Operation (typically OPERATION_ACCEPTED)
    m2m [esi].operation, FirstOperation
    
    ;AncestorCall to Primer.Init (set Client's Owner)
    ACall Init, pOwner
    
    mov eax,TRUE
MethodEnd

;============================================
;DESTRUCTOR METHOD
;Returns NOTHING
;============================================ 
Method Client.Done,uses esi
local mylinger:linger 
	SetObject esi
	.if [esi].hSocket != INVALID_SOCKET
		;Perform an "abortive close" of the socket, just in case..
		mov mylinger.l_onoff , 1
		mov mylinger.l_linger , 0
		invoke setsockopt,[esi].hSocket,SOL_SOCKET, SO_LINGER, addr mylinger,sizeof linger
		.if eax==SOCKET_ERROR
		invoke WSAGetLastError
		DbgDec eax,"ERROR on SetSockOpt"
		.endif
		SetObject esi
		invoke closesocket, [esi].hSocket
	.endif
	;Deallocate memory
	SetObject esi
	.if [esi].pbuffer!=0
		MemFree [esi].pbuffer
	.endif
   ;AncestorCall to Primer.Done
   ACall Done
MethodEnd

