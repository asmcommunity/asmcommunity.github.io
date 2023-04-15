
DOING_CONNECT equ 1
DOING_ACCEPT  equ 2
DOING_READ    equ 4
DOING_WRITE   equ 8

;Some observations:
;The Client class is a wrapper for a Socket and a SocketAddress,
;and implements general socket-related functionality.
;Do note that Client.pOwner = handle of iocp port

SAFE_FREE macro crapola
	.if crapola!=0
		MemFree crapola
		mov crapola,NULL
	.endif
endm

USERID struct
	namelength db ?
	namestring db 47 dup (?)
	pass_hash  db 16 dup (?)
USERID ends

ClientID equ 93846
Object Client,ClientID,DwordCollection
	RedefineMethod Init,Pointer,Pointer,Pointer,Pointer,BOOL
	RedefineMethod Done	
	VirtualMethod  Goodbye
	VirtualMethod  SetLocalHost,dword,dword
	VirtualMethod  SetRemoteHost,dword,dword

	VirtualMethod CreateSocket,BOOL
	VirtualMethod AllocateJob,dword  			;dwOperation
	VirtualMethod Notify,dword                  ;dwOperation

	;Methods to place IO jobs on the IOCP queue etc
	VirtualMethod QueueRead
	VirtualMethod QueueWrite,Pointer,dword		;pSendBuf,dwSize
	VirtualMethod QueueConnect,dword,dword		;dwHostIP, dwPort
	VirtualMethod QueueAccept,HANDLE		    ;hListenSocket
   	VirtualMethod FreeJob,Pointer				;-> IOJob

    ;Methods to marshal completion notifications
    VirtualMethod OnAcceptCompleted,dword,Pointer,dword		;hiocp, pJob, dError
   	VirtualMethod OnConnectCompleted,Pointer,dword                 ;pJob, dError
	VirtualMethod OnReadCompleted,Pointer,dword,dword       ;pJob,bytes,dError
	VirtualMethod OnWriteCompleted,Pointer,dword,dword              ;bytes, dError
	VirtualMethod OnUserIOCompleted,Pointer,dword
	
	;Methods to (re)issue send/recv jobs
    VirtualMethod DoRead, Pointer				;-> XOVL	
	VirtualMethod DoWrite, Pointer				;-> XOVL    
	
	VirtualMethod SetUserID,Pointer,Pointer	
	VirtualMethod Set_StateBlock,Pointer,dword
	
	VirtualMethod Pack_StateChanges
	VirtualMethod Unpack_StateChanges,Pointer,dword
	
	;Data Members
	DefineVariable pJobAllocator,Pointer,NULL

	DefineVariable PendingReadCounter,dword,NULL
	DefineVariable PendingWriteCounter,dword,NULL	
	DefineVariable UserID, USERID,{<>}
	DefineVariable pCurrentChannel,Pointer,NULL		;ptr to current chatroom/communication channel
	DefineVariable pPreviousChannel,Pointer,NULL	;ptr to previous chatroom/communication channel
	DefineVariable Mute,BYTE,FALSE					;Heh
	
	;For inbound connections, this field is used to determine
	;how long the client has been accepted without sending anything.
	;The client has several seconds to send one full protocol packet
	;before being summarily dismissed by the AntiZombie thread in NetEngine.
	DefineVariable dTimeIdleSinceAccept,dword,NULL
	
	;The Client STATE 
	DefineVariable pUser_StateBlock,Pointer,NULL	
	DefineVariable dwUser_StateBlock_Size,dword,NULL	
	;The Client PREVIOUS STATE
	;This is used to implement DELTA COMPRESSION
	;Whenever we wish to send changes in the Client State,
	;we'll only send the Bytes Which Changed,
	;accompanied by a BITMAP used to Map the Changed Bytes...
	;where each Bit represents one Byte of the StateBlock data
	;EXAMPLE : If the User's ClientState struct is 40 bytes,
	;and only 3 bytes have changed in value,
	;we'll need to send those 3 bytes, plus 5 bytes of BITMAP data (40/8=5)
	;So we'll need to send 8 bytes instead of 40.	
	DefineVariable pUser_StateBlock_PREV,Pointer,NULL	

	;Networking stuff
	DefineVariable pListener,Pointer,NULL
	DefineVariable pProtocol,Pointer,NULL
	DefineVariable hSocket,dword,NULL
	DefineVariable Sin_RemoteHost,sockaddr_in,{<>}
	DefineVariable Sin_LocalHost,sockaddr_in,{<>}
	DefineVariable bSocketIsUDP,BOOL,FALSE
	
ObjectEnd

if IMPLEMENT
;============================================
;CONSTRUCTOR METHOD
;Returns Success=TRUE or FALSE.
;============================================
Method Client.Init, uses esi, hiocp, pJobAllocator, pProtocol, pListener, bWantUDP:BOOL
	SetObject esi
    ACall esi.Init, hiocp,16,256,-1
    DbgHex esi, "Initializing Client"
    m2m [esi].pJobAllocator,pJobAllocator,edx
    m2m [esi].pListener,pListener,edx
    m2m [esi].pProtocol,pProtocol,edx
    m2m [esi].bSocketIsUDP,bWantUDP,edx
    

	;Create a Socket
	OCall CreateSocket,bWantUDP
    .if eax==NULL
    	mov eax,FALSE
    .else         
        mov eax,TRUE
    .endif
MethodEnd

;============================================
;DESTRUCTOR METHOD
;Returns NOTHING
;============================================ 
Method Client.Done,uses esi
	SetObject esi
	.if [esi].hSocket!=INVALID_SOCKET
    	OCall esi.Goodbye
    .endif
    ACall esi.Done
MethodEnd

;=========================================
;Title   - Client.CreateSocket
;Purpose - (Re)Create Socket
;Returns - eax=hSocket, or NULL=FAILURE
Method Client.CreateSocket,uses esi ecx,bWant_UDP:BOOL
LOCAL opt
LOCAL tries
	SetObject esi

    DbgWarning "CreateSocket"
;    invoke EnterCriticalSection,addr [esi]._CS
    mov tries,0
    
@@:
	;Create a Socket
	.if bWant_UDP==TRUE
	    DbgText "UDP datagram socket"
	    mov eax,SOCK_DGRAM
	    mov edx,IPPROTO_UDP
	.else
	    DbgText "TCP stream socket"
	    mov eax,SOCK_STREAM
	    mov edx,IPPROTO_TCP
	.endif
    mov [esi].hSocket, $invoke (WSASocket, AF_INET, eax, edx, NULL, 0, WSA_FLAG_OVERLAPPED)
    .if eax==INVALID_SOCKET
        DbgWarning "Failed to create socket"
        invoke WSAGetLastError
        DbgDec eax,"Client.CreateSocket failed on WSASocket"
    ;    invoke LeaveCriticalSection,addr [esi]._CS
        xor eax,eax
    	ExitMethod
    .else
        DbgHex eax,"Socket Handle"
    .endif

	;Set socket options for TCP
	.if [esi].bSocketIsUDP==FALSE
        mov opt, 0
        invoke setsockopt, [esi].hSocket, IPPROTO_TCP, TCP_NODELAY , addr opt, sizeof opt
        ;I have found through experimenting that this call can fail
        ;with error 10038 - not socket ... even though the handle is valid.
        ;It seems to always be the same socket handle at fault !!! (0D4h on my system)
        ;However if we close the socket handle and recreate it (we get SAME handle)
        ;then try setting sock option again, it will work! Weird, right?
        ;Its NECESSARY to recreate the socket - just retrying with same open handle
        ;and without recreating the socket is not sufficient.
        .if eax==INVALID_SOCKET
            invoke WSAGetLastError
            push eax
            DbgDec eax,"Client.CreateSocket failed on setsockopt"
            DbgHex [esi].hSocket,"Handle which failed"         
            invoke closesocket,[esi].hSocket
            invoke CloseHandle,[esi].hSocket
            pop eax
            .if eax==10038 && tries<5
                inc tries
                jmp @B   
            .endif

        	xor eax,eax
        	ExitMethod
        .else
            DbgText "Socket Option TCP_NODELAY was Set"
        .endif
    .endif
  ;  invoke LeaveCriticalSection,addr [esi]._CS
    mov eax,[esi].hSocket
MethodEnd
;=========================================

;This method marshals a QUIT event notification to the Client's Protocol handler,
;and looks after closing of the Socket handle and other Client resources.
Method Client.Goodbye,uses esi
local mylinger:linger
	SetObject esi
	
    DbgWarning "Goodbye"

	.if [esi].pProtocol!=0
    	OCall [esi].pProtocol::NetworkProtocol.OnClientQuit,esi
    .endif
 
	.if [esi].hSocket != INVALID_SOCKET
	    .if [esi].bSocketIsUDP==FALSE
    		;Perform an "abortive close" of the socket, just in case there is unsent data queued..    		
    		;I am currently using a 'linger' time of NULL, which means 'close it hard'
    		;so any pending data is totally lost...
    		;If we set the timeout to a nonzero value, and data is pending,
    		;then closesocket will fail with EWOULDBLOCK, and socket is still alive!!
    		;We will need to call closesocket AGAIN, and need a mechanism to do that.
    		;For this reason, its not recommended to use a nonzero timeout on nonblocking sockets
    		;but its VERY doable if you understand exactly what the implications are...
    		;Read that twice!!
    		mov mylinger.l_onoff , 1
    		mov mylinger.l_linger , 0
    		invoke setsockopt,[esi].hSocket,SOL_SOCKET, SO_LINGER, addr mylinger,sizeof linger
    		.if eax==SOCKET_ERROR
    		    invoke WSAGetLastError
    		    DbgDec eax,"ERROR on SetSockOpt"
    		.endif
    	.endif
		invoke closesocket, [esi].hSocket
   		.if eax==SOCKET_ERROR   ;this error check only valid for nonzero linger time
   		    invoke WSAGetLastError
   		    DbgDec eax,"ERROR on closesocket"
   		.endif
		invoke CloseHandle, [esi].hSocket
    .endif
    
    mov [esi].hSocket,INVALID_SOCKET

	SAFE_FREE [esi].pUser_StateBlock
	SAFE_FREE [esi].pUser_StateBlock_PREV
	
	;Release the IOJob Stack (if any)
	.while [esi].dCount!=0
	    OCall esi.DeleteAt,0
	    OCall esi.FreeJob,eax
	.endw
	

		    
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
;Title   - Client.AllocateJob
;Purpose - Assign a free IOJob to a Client
;Returns - pIOJob
; ——————————————————————————————————————————————————————————————————————————————————————————————————
Method Client.AllocateJob,uses ebx esi,dwOperation
	SetObject esi	
	DbgWarning "Client is allocating a Job"
	OCall [esi].pJobAllocator::IOJobPool.Allocate,esi
	m2m [eax].IOJob.xovl.operation,dwOperation 
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
;Title   - Client.Notify
;Purpose - Posts a custom completion notification message
;Returns - nothing
; ——————————————————————————————————————————————————————————————————————————————————————————————————
Method Client.Notify,uses esi,dwOperation
    SetObject esi
    OCall esi.AllocateJob,dwOperation
    mov edx,eax
    invoke PostQueuedCompletionStatus,[esi].pOwner,0,edx,addr [edx].IOJob.xovl
MethodEnd

;This method creates an ACCEPT io job, which typically
;will be sent to the IOCP queue, pending completion.
;Success : eax=pJob
;Failure : eax=NULL
Method Client.QueueAccept,uses esi edi ebx,listensock:dword  
    SetObject esi 
    DbgWarning "QueueAccept"    
    mov [esi].dTimeIdleSinceAccept,0
          
    mov edx,$OCall (esi.AllocateJob,OPERATION_DOACCEPT) 
    push edx   
    invoke AcceptEx, listensock, [esi].hSocket , [edx].IOJob.xovl.wsabuf.buf,
            NULL, sizeof sockaddr_in + 16, sizeof sockaddr_in + 16,
            addr [edx].IOJob.xovl.bytes, addr [edx].IOJob.xovl
    pop edx
    .if eax==TRUE
    	;The accept completed synchronously - call the completion routine manually??
    	DbgWarning "Accept completed synchronously"
    .endif   

        
MethodEnd

Method Client.SetRemoteHost,uses esi,dwRemoteHost,dwPort
    SetObject esi
    DbgWarning "SetRemoteHost"   

	mov [esi].Sin_RemoteHost.sin_family,AF_INET
	m2m [esi].Sin_RemoteHost.sin_addr.S_un,dwRemoteHost,edx
	invoke htons,dwPort
	mov [esi].Sin_RemoteHost.sin_port,ax
MethodEnd

Method Client.SetLocalHost,uses esi,dwLocalHost,dwPort
    SetObject esi
    DbgWarning "SetLocalHost" 
	mov [esi].Sin_LocalHost.sin_family,AF_INET
	m2m [esi].Sin_LocalHost.sin_addr.S_un,dwLocalHost,edx
	invoke htons,dwPort
	mov [esi].Sin_LocalHost.sin_port,ax
MethodEnd

;This method creates a CONNECT io job, which typically
;will be sent to the IOCP queue, pending completion.
;Success : eax=pClient
;Failure : eax=NULL
Method Client.QueueConnect,uses esi,dwRemoteHost,dwPort
LOCAL pJob
LOCAL opt
LOCAL pxovl,pConnectEx
LOCAL HostName[256]:BYTE

	SetObject esi
    DbgWarning "QueueConnect" 
    OCall esi.SetRemoteHost,dwRemoteHost,dwPort

	mov [esi].Sin_LocalHost.sin_family,AF_INET
	mov [esi].Sin_LocalHost.sin_port,0		;any local port will do
	

   invoke   gethostname, ADDR HostName, 256
   invoke   gethostbyname, ADDR HostName
   mov      eax, [eax].hostent.h_list
   mov      ebx, [eax]
   ; --- ebx = pointer to the first entry in 0-term. ip list ---
  ; .WHILE   TRUE   ;scan trough list for all IP's
      mov      eax, [ebx]
  ;    .BREAK .IF eax==0 ;stop if null-terminator reached
  ;    DbgHex eax
  ;    add   ebx, 4 ;go to next entry
  ; .ENDW
	mov [esi].Sin_LocalHost.sin_addr.S_un,eax 
	
	mov opt,1
    invoke setsockopt, [esi].hSocket, SOL_SOCKET, SO_REUSEADDR , addr opt, sizeof opt
	invoke bind,[esi].hSocket,addr [esi].Sin_LocalHost,sizeof sockaddr_in
	.if eax==-1
		DbgDec eax,"BIND result"	
		invoke WSAGetLastError
		DbgDec eax,"SOCKET ERRORCODE"
		xor eax,eax
	.else
	
	    ;Allocate an IOJob of appropriate type ("OPERATION_CONNECT")
		mov pJob,$OCall ([esi].pJobAllocator::IOJobPool.Allocate,esi)
		mov [eax].IOJob.xovl.operation,OPERATION_CONNECT
		DbgHex eax,"IOJob"
		lea edi,[eax].IOJob.xovl
		mov pxovl,edi

		;Try to obtain ptr to the ConnectEx function
		mov pConnectEx,0
		invoke WSAIoctl,[esi].hSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,
		addr ConnectExGuid,sizeof GUID,
		addr pConnectEx,sizeof pConnectEx,
		addr [edi].XOVL.bytes,
		pxovl,0
		.if  eax==-1	
			DbgWarning "FAILED TO GET PTR TO CONNECTEX"
			invoke WSAGetLastError
			DbgDec eax,"Socket Error Code"
			xor eax,eax
			
		.else
  
    	  ;  mov eax,[edi].XOVL.bufsizeorig   
    	  ;  sub eax,(2*sizeof sockaddr_in)+32	
    	  
    	    ;Make a call to ConnectEx
    	    mov [edi].XOVL.wsabuf.len,0
    	    push pxovl
    	    push NULL
    	    push 0
    	    push NULL
    	    push sizeof sockaddr_in
    	    lea eax,[esi].Sin_RemoteHost
    		push eax
    		push [esi].hSocket
    		call pConnectEx
    		.if eax==FALSE
    			invoke WSAGetLastError
    			.if eax!=ERROR_IO_PENDING	  
    			    DbgWarning "Problem"  
    				OCall esi.OnConnectCompleted,pJob,eax
    				ExitMethod
    			.endif
    		.else
    		    ;In the case of Synchronous completion, I believe that
    		    ;we will NOT receive completion notification via IOCP Worker.
    		    ;Therefore, we should marshall the notification directly,
    		    ;or alternatively, post a completion notification ourselves.
    		    DbgWarning "ConnectEx completed Synchronously"
    		    ;Here I've opted to skip the whole iocp mechanism
    		    ;and marshall the notification directly (its faster).
    		    OCall esi.OnConnectCompleted,pJob,NULL
    		.endif
    		mov eax,esi
    	.endif
	.endif

MethodEnd

;This method queues as many WRITE Jobs
;as are required to completely send the user data
Method Client.QueueWrite,uses ebx esi,pData,dwLen
LOCAL pJob,dUsefulBufSpace

	SetObject esi
    DbgWarning "QueueWrite" 
 ;   invoke EnterCriticalSection,addr [esi]._CS	 
    DbgWarning "entered"
	xor ecx,ecx
	.while dwLen!=0
    	mov pJob,$OCall (esi.AllocateJob,OPERATION_DOWRITE)

    	;Calculate maximum usable buffer space
    	;We must reserve some space, which will
    	;be filled with sockaddr info on completion (I think).
    	mov edx,[eax].IOJob.xovl.bufsizeorig
    	sub edx,2*sizeof (sockaddr_in)+32
    	mov dUsefulBufSpace,edx
    	
    	mov edx,dwLen
    	.if edx <=dUsefulBufSpace
    	    mov [eax].IOJob.xovl.wsabuf.len, edx
	        invoke RtlMoveMemory,[eax].IOJob.xovl.wsabuf.buf,pData,dwLen	
	        mov eax,pJob
	        mov dwLen,0
	    .else
    	    m2m [eax].IOJob.xovl.wsabuf.len, dUsefulBufSpace
	        invoke RtlMoveMemory,[eax].IOJob.xovl.wsabuf.buf,pData,dUsefulBufSpace
	        mov eax,pJob
	        mov edx,dUsefulBufSpace
	        sub dwLen,edx
	        add pData,edx
	    .endif
    	OCall DoWrite,addr [eax].IOJob.xovl
    	inc [esi].PendingWriteCounter
	.endw	
	;invoke LeaveCriticalSection,addr [esi]._CS
	mov eax,pJob
MethodEnd

;This method allocates a new or recycled Job,
;initializes it as a READ job, and realizes the io operation.
;Returns pJob (success) or NULL (failed)
Method Client.QueueRead,uses ebx esi
LOCAL pJob
	SetObject esi	
    DbgWarning "QueueRead" 
	mov pJob,$OCall ([esi].pJobAllocator::IOJobPool.Allocate,esi)
	mov [eax].IOJob.xovl.operation,OPERATION_READ
	OCall DoRead,addr [eax].IOJob.xovl
	.if eax==TRUE
    	inc [esi].PendingReadCounter
	    mov eax,pJob
    .else
        DbgWarning "Failed to queue a READ operation"
        OCall esi.FreeJob,pJob
        xor eax,eax    
    .endif
MethodEnd




;This method performs a WSARECV (or WSARecvFrom, if UDP) call on an initialized IO job
;Returns TRUE (success) or FALSE (failed)
Method Client.DoRead,uses esi ebx ecx,pXOVL
LOCAL flags
LOCAL sinsize

    SetObject esi
    DbgWarning "DoRead" 
;    invoke EnterCriticalSection,addr [ebx]._CS
    DbgWarning "entered"
	mov ebx,pXOVL
    mov flags, 0	
    mov [ebx].XOVL.operation, OPERATION_READ
    lea ecx, [ebx].XOVL.wsabuf    	
    mov eax,[ebx].XOVL.bufsizeorig  
    sub eax,(2*sizeof sockaddr_in)+32
    mov [ecx].WSABUF.len, eax

    .if [esi].bSocketIsUDP==FALSE
        invoke WSARecv,    [esi].hSocket,ecx,1, addr [ebx].XOVL.bytes,addr flags,    ebx,NULL
    .else
       ; DbgText "Issuing UDP receive"
       mov sinsize,sizeof sockaddr_in
        invoke WSARecvFrom,[esi].hSocket,ecx,1, addr [ebx].XOVL.bytes,addr flags,addr [esi].Sin_RemoteHost,addr sinsize,ebx,NULL
    .endif
    
    .if eax==NULL
	     DbgText "READ IO Completed Synchronously.","Worker"
    	 mov eax,TRUE
    .else
	    invoke WSAGetLastError
	    .if eax!=ERROR_IO_PENDING
        	mov ecx,[esi].hSocket
    		.if eax==10045

 				DbgHex ecx,"Socket (Error 10045 - Operation Not Supported.)","Client.DoRead Error"
        
    			mov eax,FALSE 		
			.else
 
				DbgDec eax,"Socket Error","Client.DoRead Error"							
				DbgHex esi,"Client Failure","Client.DoRead Error"
				DbgHex ecx,"Failed Socket","Client.DoRead Error"

    			mov eax,FALSE
	    	.endif
    	.else
    	    DbgText "recv pending"
    		mov eax,TRUE
    	.endif
    .endif
    
  ;  push eax
 ;   invoke LeaveCriticalSection,addr [ebx]._CS
;    pop eax
MethodEnd

;This method performs a WSASEND (TCP) or WSASendTo (UDP) call on an initialized IO job
;For UDP, it is assumed that SetRemoteHost was already called.
Method Client.DoWrite,uses esi ebx ecx,pXOVL
LOCAL flags


	SetObject esi
    DbgWarning "DoWrite"     
    DbgLine
	mov flags, 0	
	mov ebx,pXOVL
    mov [ebx].XOVL.operation, OPERATION_DOWRITE       
    DbgHex [ebx].XOVL.wsabuf.buf
    DbgDec [ebx].XOVL.wsabuf.len
    DbgDec [ebx].XOVL.bytesused
    DbgDec [ebx].XOVL.bufsizeorig
    DbgHex [ebx].XOVL.pbuforig
    DbgHex [esi].hSocket
    DbgHex pXOVL
	.if [esi].bSocketIsUDP==FALSE
	    DbgText "Sending TCP packet"
        invoke WSASend, [esi].hSocket, addr [ebx].XOVL.wsabuf , 1, addr [ebx].XOVL.bytes, flags, ebx, NULL
    .else
        DbgText "Sending UDP packet"
        invoke WSASendTo,[esi].hSocket,addr [ebx].XOVL.wsabuf ,1, addr [ebx].XOVL.bytes,flags,addr [esi].Sin_RemoteHost,sizeof sockaddr_in,ebx,NULL
    .endif
    .if eax==NULL
	     DbgText "WRITE IO Completed Synchronously.","Worker"
    	 mov eax,TRUE
    .else
	    invoke WSAGetLastError
	    .if eax!=ERROR_IO_PENDING
	        DbgDec eax,"Error in Client.DoWrite"
   			mov eax,FALSE
    	.else
    		DbgText "WRITE IO PENDING","Worker"
    		mov eax,TRUE
    	.endif
    .endif
MethodEnd

;This method associates a newly-accepted socket with the IOCP,
;converts the completed ACCEPT as a pending READ,
;and then marshals the completion notification to the Protocol handler.
Method Client.OnAcceptCompleted,uses ebx esi,hIOCP,pJob,dError
    ;Bind the Client Socket to the IOCP
    SetObject esi
    DbgWarning "OnAcceptCompleted" 
    invoke setsockopt, [esi].hSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,  NULL,   0 
    invoke CreateIoCompletionPort, [esi].hSocket, hIOCP, esi, 0  

    ;ReQueue the ACCEPT job as a READ job
	mov eax,pJob
	mov [eax].IOJob.xovl.operation,OPERATION_READ
	OCall DoRead,addr [eax].IOJob.xovl
	inc [esi].PendingReadCounter

    ;Notify the Client's Protocol handler
    .if [esi].pProtocol!=0
        OCall [esi].pProtocol::NetworkProtocol.OnAcceptCompleted,esi,dError    
    .endif
    
    ;Set the Zombie Timer
    mov [esi].dTimeIdleSinceAccept,$invoke (GetTickCount)

MethodEnd

;This method converts the completed CONNECT into a pending READ,
;and then marshals the completion notification to the Protocol handler.
Method Client.OnConnectCompleted,uses esi,pJob,dError
	SetObject esi
   DbgWarning "OnConnectCompleted" 
    ;Marshal the completion notification to the Protocol handler
    .if [esi].pProtocol!=0
    	OCall [esi].pProtocol::NetworkProtocol.OnConnectCompleted,esi,dError
    .endif

    .switch dError
    .case 0
    	invoke setsockopt, [esi].hSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT,  NULL,   0     
        ;ReQueue the CONNECT job as a READ job
    	mov eax,pJob
    	mov [eax].IOJob.xovl.operation,OPERATION_READ
    	OCall esi.DoRead,addr [eax].IOJob.xovl
    	inc [esi].PendingReadCounter
    .case 10061;Here's a nice place to enumerate the various Connect errors
        DbgWarning "Connect failed because it was refused - perhaps the Server is not Listening"
    .default
        DbgWarning "Connect refused for some unknown reason"
        DbgDec dError,"Reason for failure"
    .endsw
    


MethodEnd

Method Client.OnWriteCompleted,uses esi,pJob,bytes,dError
    SetObject esi
       DbgWarning "OnWriteCompleted"
 ;    invoke EnterCriticalSection,addr [esi]._CS
    DbgWarning "entered"
    ;Check whether the read job completed successfully
    .if dError!=0
        DbgDec dError,"Socket Error on send"
 ;       invoke LeaveCriticalSection,addr [esi]._CS
        return ERROR_USERQUIT
    .endif
    ;Just a "Courtesy Notification"
    .if [esi].pProtocol!=0
        OCall [esi].pProtocol::NetworkProtocol.OnWriteCompleted,bytes
    .endif
 ;   invoke LeaveCriticalSection,addr [esi]._CS
    OCall esi.FreeJob,pJob
    xor eax,eax
MethodEnd

;This method collects 'full' READ jobs,
;until a suitable delimiter is found in the stream.
;Then the collection is passed to the protocol handler for processing,
;and finally, the collection is emptied of all consumed data.
Method Client.OnReadCompleted,uses esi,pJob,bytes,dError
LOCAL consumed
    SetObject esi
    DbgWarning "OnReadCompleted"    

    ;Check whether the read job completed successfully
    .if dError!=0
        DbgDec dError,"Socket Error on receive"
        return ERROR_USERQUIT
    .endif

	;IMPORTANT!
	;Copy this value across as it is more accurate.
	mov eax,bytes
	mov [edx].IOJob.xovl.bytes,eax
	add [edx].IOJob.xovl.bytesused,eax
	add [edx].IOJob.xovl.wsabuf.buf,eax
	
    ;Just a "Courtesy Notification"
    .if [esi].pProtocol!=0
        mov eax,pJob
        OCall [esi].pProtocol::NetworkProtocol.OnReadCompleted,esi,addr [eax].IOJob.xovl,bytes,dError
    .endif

;--
    mov edx,pJob
	OCall [esi].pProtocol::NetworkProtocol.FindDelimiter,esi,addr [edx].IOJob.xovl
	DbgDec eax,"result of FindDelimiter"
	.if eax==ERROR_BADPROTOCOL||eax==ERROR_USERQUIT
		;Just let these return to the Worker thread
		DbgWarning "bad protocol, or quit"
		ExitMethod
		
	.elseif eax!=0
	    ;We have received a fully delimited packet that passed protocol checks
	    ;so lets stop AntiZombie from dismissing this connection..
    	mov [esi].dTimeIdleSinceAccept,0
	    ;Add the final completed Job to Client's job stack
    	OCall esi.Insert,pJob
    	;We pass the job stack, which is synonymous with Client
    	DbgWarning "PROCESSING RECEIVED DATA"
		OCall [esi].pProtocol::NetworkProtocol.ProcessReceivedData, esi
		.if eax==ERROR_BADPROTOCOL || eax==ERROR_USERQUIT	
			ExitMethod
		.else
			DbgDec eax,"#bytes were Processed"
			mov consumed,eax
			;eax = #bytes Consumed , which should be Removed
			;Note this value could be larger than one Job buffer,
			;meaning that it represents several Jobs
			;Also note that the total #bytes buffered
			;could be larger than the #bytes consumed
			;meaning the last iojob will contain
			;some data that is not part of the complete receive
			;and must remain buffered for subsequent appending!!
			.while [esi].dCount!=0
			    OCall esi.DeleteAt,0
			    .if [esi].dCount==0
			        ;this is the last iojob - it may or may not be full
			       OCall eax::IOJob.Cut,consumed ;returns TRUE (buffer empty) or FALSE (buffer not empty)
			    .else
    			    ;This is not the last IOJob, so it must be totally full
    			    ;Lets just throw it away
    			    mov edx,[eax].IOJob.xovl.bytesused		        
    			    sub consumed,edx
    			    OCall esi.FreeJob,eax
    			    mov eax,FALSE
    			.endif
			.endw
			
			

		.endif
	.else
		;Apparently, we don't have enough Data to process yet
		;We'll want to receive more data, and we wanna Append to buffer
		mov eax,FALSE
	.endif



;--
    ; eax now contains one of the following:
    ;        : ERROR_BADPROTOCOL or  ERROR_USERQUIT = client kick required
    ;		 : TRUE = buffer reset required 
    ;		 : FALSE = want more data
    .if eax==FALSE
        ;Check if the buffer is full.. if it is full,
        ;we will throw the job into PartialReads stack,
        ;issue another read job, and let the Server know
        ;that we still want more data :)
        mov edx,pJob
        mov eax,[edx].IOJob.xovl.bytesused
        .if eax==[edx].IOJob.xovl.bufsizeorig
            OCall esi.Insert,pJob
            OCall esi.QueueRead
        .endif
        mov eax,FALSE
    .endif

MethodEnd
; ——————————————————————————————————————————————————————————————————————————————————————————————————

Method Client.OnUserIOCompleted,uses esi,pJob,dError
    SetObject esi
    .if [esi].pProtocol!=0
        OCall [esi].pProtocol::NetworkProtocol.OnUserIOCompleted,esi,pJob,dError
    .endif
MethodEnd

;This method sends a defunct Job to the recycling pool
Method Client.FreeJob,uses esi ecx,pJob
	SetObject esi
    DbgWarning "FreeJob"
;	invoke EnterCriticalSection,addr [esi]._CS
    DbgWarning "entered"
	mov ecx,pJob
	.if [ecx].IOJob.xovl.operation==OPERATION_READ
		dec [esi].PendingReadCounter
	.elseif [ecx].IOJob.xovl.operation==OPERATION_DOWRITE
		dec [esi].PendingWriteCounter
	.endif
	OCall [esi].pJobAllocator::IOJobPool.Free,pJob
;	invoke LeaveCriticalSection,addr [esi]._CS
MethodEnd

;See USERID struct for the format of these strings
Method Client.SetUserID,uses esi,pName,pPass
	SetObject esi
    DbgWarning "SetUserID"

	mov edx,pName
	movzx eax,byte ptr[edx]
	inc eax
	invoke lstrcpyn,addr [esi].UserID.namelength,edx,eax
	lea edx, [esi].UserID.namestring
	DbgStr edx,"Client Name was set by Server"
	mov edx,pPass
	.if edx!=0
		invoke RtlMoveMemory,addr [esi].UserID.pass_hash,edx,16
		DbgStr edx,"Client Password Hash was set by Server"
	.endif

MethodEnd

;This method attempts to DeltaCompress the changes in the User StateBlock
;Returns Packed Data representing CHANGED BYTES or NULL=FAILED (no changes?)
;Data presented as:
;-(#UserBytes/8) bytes worth of BITKEY representing CHANGES
;-#UserBytes or less bytes worth of CHANGED BYTES
Method Client.Pack_StateChanges,uses esi edi ebx
LOCAL pbitkey,keysize
LOCAL changes
	
	SetObject esi
    DbgWarning "Pack_StateChanges"
	mov changes,0
	mov eax,[esi].dwUser_StateBlock_Size
	.if eax!=0
		shr eax,3
		mov keysize,eax
		add eax,[esi].dwUser_StateBlock_Size	;max.size of all data 
		mov pbitkey,$MemAlloc(eax)
		
		xor ecx,ecx
		mov edi,[esi].pUser_StateBlock
		mov edx,[esi].pUser_StateBlock_PREV
		.while ecx<[esi].dwUser_StateBlock_Size
			mov al, byte ptr[edi+ecx]
			.if al==byte ptr[edx+ecx]
				mov eax,pbitkey
				btc pbitkey,ecx
			.else
				inc changes
				mov ebx,pbitkey
				mov byte ptr[ebx+keysize],al
				bts pbitkey,ecx
			.endif
			inc ecx
		.endw
		.if changes==0
			MemFree pbitkey
			mov pbitkey,0
		.endif
	.endif
	mov eax,pbitkey
MethodEnd

;Unpacks a DeltaCompressed package of changes to the User StateBlock
Method Client.Unpack_StateChanges,uses esi,pPackedChanges,dPackedSize
LOCAL numkeybits
	SetObject esi
    DbgWarning "Unpack_StateChanges"
	mov eax,[esi].dwUser_StateBlock_Size
	shr eax,3	
	sub dPackedSize,eax	;=#bytes of Changes
	mov numkeybits,eax
	.if eax!=0
		xor ecx,ecx
		.while ecx<numkeybits
			bt pPackedChanges,ecx
			jc @F
			mov edx,pPackedChanges
			mov al,byte ptr [edx+ecx]
			mov edx,[esi].pUser_StateBlock
			mov byte ptr[edx+ecx],al
		@@: inc ecx
		.endw
		mov eax,TRUE
	.else
		mov eax,FALSE
	.endif	
MethodEnd

;Initializes the User StateBlock (current and prev)
Method Client.Set_StateBlock,uses esi, pUserData, dwUserDataSize
	SetObject esi
    DbgWarning "Set_StateBlock"
	m2m [esi].dwUser_StateBlock_Size,dwUserDataSize
	SAFE_FREE [esi].pUser_StateBlock
	SAFE_FREE [esi].pUser_StateBlock_PREV
	mov [esi].pUser_StateBlock_PREV,$MemAlloc(dwUserDataSize)
	invoke RtlMoveMemory,eax,pUserData,dwUserDataSize
	mov [esi].pUser_StateBlock,$MemAlloc(dwUserDataSize)
	invoke RtlMoveMemory,eax,pUserData,dwUserDataSize
MethodEnd

endif
