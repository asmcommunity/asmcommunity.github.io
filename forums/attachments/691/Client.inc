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
PluginID equ 23543
Object Plugin,PluginID
  StaticAbstract  Done                            ;Should be 1st static method of "primitive" classes
  StaticAbstract  Startup                         ;Should be 2nd static method!
  StaticAbstract  Shutdown    					;Should be 3th static method!
  StaticAbstract  Create, Pointer ;-> Server
  StaticAbstract  Proof
ObjectEnd
;==========================================================

ClientID equ 93846
Object Client,ClientID,Primer
	;Override the inherited Primer.Init
	RedefineMethod Init,Pointer	 			;-> Owner
	;Override the inherited Primer.Done
	RedefineMethod Done	
	StaticMethod Accept,dword,dword			;hListenSocket, FirstOperation
	StaticMethod CreateSocket
	;Data Members
	DefineVariable OVL,XOVERLAPPED,{}
ObjectEnd

;============================================
;CONSTRUCTOR METHOD
;Returns Success=TRUE or FALSE.
;============================================
Method Client.Init, uses esi, pOwner
LOCAL pbuf:dword
	;Create a Socket
	OCall CreateSocket
    IfCase eax==NULL
    	mov eax,FALSE
    	ExitMethod
    EndCase    
    
    ;Allocate bufferspace
    mov pbuf, $MemAlloc(4096,HEAP_ZERO_MEMORY)
    
	;Store pbuffer in Overlapped
    SetObject esi
    m2m [esi].OVL.pbuffer, pbuf
    ;Set the way-cool "pointer to myself"
    ;(We use this to find &Client from &Client.OVL)
    mov [esi].OVL.pClient, esi
    
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
	.if [esi].OVL.hSocket != INVALID_SOCKET
		;Perform an "abortive close" of the socket, just in case..
		mov mylinger.l_onoff , 1
		mov mylinger.l_linger , 0
		invoke setsockopt,[esi].OVL.hSocket,SOL_SOCKET, SO_LINGER, addr mylinger,sizeof linger
		.if eax==SOCKET_ERROR
		invoke WSAGetLastError
		DbgDec eax,"ERROR on SetSockOpt"
		.endif
		SetObject esi
		invoke closesocket, [esi].OVL.hSocket
	.endif
	;Deallocate memory
	SetObject esi
	.if [esi].OVL.pbuffer!=0
		MemFree [esi].OVL.pbuffer
	.endif
   ;AncestorCall to Primer.Done
   ACall Done
MethodEnd

;=============================================
;Title   - Client.Accept
;Purpose - Queue an Accept IO job on the IOCP queue
;Returns - FAIL=NULL, SUCCESS=-1
;=============================================
Method Client.Accept,uses esi,listen_socket,FirstOperation
    SetObject esi 
    push esi
    invoke RtlZeroMemory, [esi].OVL.pbuffer, 2048
    pop esi
    ;Initialize the first Operation (typically OPERATION_ACCEPTED)
    m2m [esi].OVL.operation, FirstOperation    
    invoke AcceptEx, listen_socket, [esi].OVL.hSocket , [esi].OVL.pbuffer,
            NULL, sizeof sockaddr_in + 16, sizeof sockaddr_in + 16,
            addr [esi].OVL.bytes, addr [esi].OVL
    SucceedOn NULL

    invoke WSAGetLastError
    FailOn ERROR_IO_PENDING
    
    SocketError eax,"Client"
    xor eax, eax
    ExitMethod

@@: 
    mov eax,-1
MethodEnd

;=========================================
;Title   - Client.CreateSocket
;Purpose - (Re)Create Socket
;Returns - eax=hSocket, or NULL=FAILURE
Method Client.CreateSocket,uses esi ecx
LOCAL opt
LOCAL sock
	SetObject esi
	;Create a Socket
    invoke WSASocket, AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED
    IfCase eax==NULL
    	ExitMethod
    EndCase
    mov sock,eax
    ;Store hSocket in Client.hSocket
    SetObject esi
    mov [esi].OVL.hSocket, eax 		
	;Set socket options
    mov opt, 0
    invoke setsockopt, sock, IPPROTO_TCP, TCP_NODELAY, addr opt, 1
    .if eax!=0
    	xor eax,eax
    	ExitMethod
    .endif
    mov eax,sock
MethodEnd
;=========================================