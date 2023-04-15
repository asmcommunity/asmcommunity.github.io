

;These are the predefined IO Operation Types for IO Jobs
;Anything outside this range is a User-Defined job type !!!
OPERATION_ENDED 	equ 0
OPERATION_DOACCEPT  equ 1
OPERATION_DOWRITE	equ 2
OPERATION_READ		equ 3
OPERATION_CONNECT	equ 4

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


if IMPLEMENT
;============================================
;CONSTRUCTOR METHOD
;pOwner is a ptr to the NetEngine object
;Returns Success=TRUE or FALSE.
;============================================
Method Client.Init, uses esi, pOwner,hiocp, pJobAllocator, pProtocol, pListener, bWantUDP:BOOL
	SetObject esi
    ACall esi.Init, pOwner,16,256,-1
    DbgHex esi, "Initializing Client"
    m2m [esi].pJobAllocator,pJobAllocator,edx
    m2m [esi].pListener,pListener,edx
    m2m [esi].pProtocol,pProtocol,edx
    m2m [esi].bSocketIsUDP,bWantUDP,edx
    m2m [esi].hiocp, hiocp
    

	;Create a Socket
	OCall esi.CreateSocket,bWantUDP
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
	DbgText "Client is Done"
	;If NetEngine is shitting down, we won't bother releasing client resources
	;We'll just rely on the OS to clean up after us... not very professional?
	mov eax,[esi].pOwner
	.if [eax].NetEngine.ShuttingDown==FALSE
    	.if [esi].hSocket!=INVALID_SOCKET
        	OCall esi.Goodbye
        .endif
    .endif
    DbgText "Client is Clearing"
    ACall esi.Done
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
	
    DbgWarning "Client.Goodbye"

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
;Title   - Client.Notify
;Purpose - Posts a custom completion notification message
;Returns - nothing
; ——————————————————————————————————————————————————————————————————————————————————————————————————
Method Client.Notify,uses esi,dwOperation
    SetObject esi
    OCall esi.AllocateJob,dwOperation
    mov edx,eax
    invoke PostQueuedCompletionStatus,[esi].hiocp,0,edx,addr [edx].IOJob.xovl
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

;This method creates a CONNECT io job, which typically
;will be sent to the IOCP queue, pending completion.
;Success : eax=pClient
;Failure : eax=NULL
Method Client.QueueConnect,uses esi,dwRemoteHost,dwPort
LOCAL pJob
LOCAL pxovl
LOCAL opt
LOCAL HostName[256]:BYTE
	SetObject esi
    DbgWarning "QueueConnect" 
    
    ;Set the remote host address
    OCall esi.SetRemoteHost,dwRemoteHost,dwPort

    ;Set the local host address
    invoke   gethostname, ADDR HostName, 256
    invoke   gethostbyname, ADDR HostName
    mov      eax, [eax].hostent.h_list
    mov      ebx, [eax]
    mov      eax, [ebx]
	OCall esi.SetLocalHost,eax,NULL
	
	;Enable address reuse
	mov opt,1
    invoke setsockopt, [esi].hSocket, SOL_SOCKET, SO_REUSEADDR , addr opt, sizeof opt
    
    ;Bind the local host address to the socket
	invoke bind,[esi].hSocket,addr [esi].Sin_LocalHost,sizeof sockaddr_in
	.if eax==-1
		DbgDec eax,"BIND result"	
		invoke WSAGetLastError
		DbgDec eax,"SOCKET ERRORCODE"
		xor eax,eax
	.else
	
	    ;Allocate an IOJob of appropriate type ("OPERATION_CONNECT")
		mov pJob,$OCall (esi.AllocateJob,OPERATION_CONNECT)
		lea eax,[eax].IOJob.xovl
		OCall esi.DoConnect,eax

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
    DbgWarning "QueueRead is allocating an IOJob" 
	mov pJob,$OCall (esi.AllocateJob,OPERATION_READ)
	DbgWarning "QueueRead is calling DoRead"
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

;Return values are:
;NULL  = failed
;pThis = success
Method Client.DoConnect,uses esi edi,pXOXL
LOCAL pConnectEx,opt
    SetObject esi
    mov edi,pXOXL
    
    mov [edi].IOJob.xovl.operation,OPERATION_CONNECT 
    
	;Try to obtain ptr to the ConnectEx function
	mov pConnectEx,0
	invoke WSAIoctl,[esi].hSocket,SIO_GET_EXTENSION_FUNCTION_POINTER,
	addr ConnectExGuid,sizeof GUID,
	addr pConnectEx,sizeof pConnectEx,
	addr [edi].XOVL.bytes,
	edi,0
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
	    push pXOXL
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
			    int 3
			    mov edx,pXOXL
				OCall esi.OnConnectCompleted,[edx].XOVL.piojob,eax
				xor eax,eax
				ExitMethod
			.endif
		.endif
		mov eax,esi
    .endif
MethodEnd


;This method performs a WSARECV (or WSARecvFrom, if UDP) call on an initialized IO job
;Returns TRUE (success) or FALSE (failed)
Method Client.DoRead,uses esi ebx ecx,pXOVL
LOCAL flags
LOCAL sinsize

    SetObject esi
    DbgLine
    DbgWarning "DoRead" 
    DbgHex pXOVL
    mov flags, 0	
    mov ebx,pXOVL
    mov [ebx].XOVL.operation, OPERATION_READ
    DbgHex [ebx].XOVL.wsabuf.buf
    DbgDec [ebx].XOVL.wsabuf.len
    DbgDec [ebx].XOVL.bytesused
    DbgDec [ebx].XOVL.bufsizeorig
    DbgHex [ebx].XOVL.pbuforig
    DbgHex [esi].hSocket
    DbgHex pXOVL
    

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
	     DbgText "READ IO Completed Synchronously."
    	 mov eax,TRUE
    .else
	    invoke WSAGetLastError
	    .if eax!=ERROR_IO_PENDING
        	mov ecx,[esi].hSocket
    		.if eax==10045

 				DbgHex ecx,"Socket (Error 10045 - Operation Not Supported.)","Client.DoRead Error"
        
    			mov eax,FALSE 		
    		.elseif eax==10057
    		    DbgHex eax,"10057 - WSAENOTCONN"
    		    mov eax,FALSE 	
			.else
 
				DbgDec eax,"Socket Error during DoRead","Client.DoRead Error"							
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
    mov eax,[ebx].XOVL.wsabuf.len
    DbgMem [ebx].XOVL.wsabuf.buf,eax
	.if [esi].bSocketIsUDP==FALSE
	    DbgText "Sending TCP packet"
        invoke WSASend, [esi].hSocket, addr [ebx].XOVL.wsabuf , 1, addr [ebx].XOVL.bytes, flags, ebx, NULL
    .else
        DbgText "Sending UDP packet"
        invoke WSASendTo,[esi].hSocket,addr [ebx].XOVL.wsabuf ,1, addr [ebx].XOVL.bytes,flags,addr [esi].Sin_RemoteHost,sizeof sockaddr_in,ebx,NULL
    .endif
    .if eax==NULL
	     DbgText "WRITE IO Completed Synchronously."
    	 mov eax,TRUE
    .else
	    invoke WSAGetLastError
	    .if eax!=ERROR_IO_PENDING
	        DbgDec eax,"Socket Error during Client.DoWrite"
   			mov eax,FALSE
    	.else
    		DbgText "WRITE IO PENDING","Worker"
    		mov eax,TRUE
    	.endif
    .endif
MethodEnd

;=========================================
;Title   - Client.OnDisconnected
;Purpose - Called when a connection has been lost,
;          this method marshals an event notification
;          to the Client's protocol handler.
;dError  - Value indicates reason this happened
;Returns - TRUE  : We are happy to let this Client die
;          FALSE : We are attempting to reconnect
Method Client.OnDisconnected,uses esi,dError
    SetObject esi
    DbgWarning "Client.OnDisconnected"
    .if [esi].pProtocol!=0
        OCall [esi].pProtocol::NetworkProtocol.OnDisconnected,esi,dError        
        .if eax==FALSE
            DbgWarning "Attempting Reconnect"
            OCall esi.AllocateJob,OPERATION_CONNECT
            OCall esi.DoConnect,eax
        .endif
    .else
        mov eax,TRUE
    .endif
MethodEnd

;This method associates a newly-accepted socket with the IOCP,
;converts the completed ACCEPT as a pending READ,
;and then marshals the completion notification to the Protocol handler.
Method Client.OnAcceptCompleted,uses ebx esi,hIOCP,pJob,dError
LOCAL sizelocal,sizefar
LOCAL plocal,premote
    ;Bind the Client Socket to the IOCP
    SetObject esi
    DbgWarning "OnAcceptCompleted" 
    invoke setsockopt, [esi].hSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,  NULL,   0 
    invoke CreateIoCompletionPort, [esi].hSocket, hIOCP, esi, 0  

    ;Grab the local and remote addresses
    mov edx,pJob
    invoke GetAcceptExSockaddrs,[edx].IOJob.xovl.wsabuf.buf,0,sizeof sockaddr_in + 16,sizeof sockaddr_in + 16,
    addr plocal,addr sizelocal,addr premote,addr sizefar
    invoke RtlMoveMemory,addr [esi].Sin_LocalHost,plocal,sizeof sockaddr_in
    invoke RtlMoveMemory,addr [esi].Sin_RemoteHost,premote,sizeof sockaddr_in 

    ;ReQueue the ACCEPT job as a READ job
	DbgWarning "OnAcceptCompleted is calling DoRead"
	mov eax,pJob
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

    ;Marshal the completion notification to the Protocol handler
    .if [esi].pProtocol!=0
    	OCall [esi].pProtocol::NetworkProtocol.OnConnectCompleted,esi,dError
    .endif

    .switch dError
    .case 0
    	invoke setsockopt, [esi].hSocket, SOL_SOCKET, SO_UPDATE_CONNECT_CONTEXT,  NULL,   0     
        OCall esi.QueueRead
    	OCall esi.FreeJob,pJob
    .case 10061;Here's a nice place to enumerate the various Connect errors
        DbgWarning "Connect failed because it was refused - perhaps the Server is not Listening"
    .default
        DbgWarning "Connect refused for some unknown reason"
        DbgDec dError,"Reason for failure"
    .endsw
    
    DbgWarning "Returning from Client.OnConnectCompleted"


MethodEnd

Method Client.OnWriteCompleted,uses esi,pJob,bytes,dError
    SetObject esi
       DbgWarning "Client.OnWriteCompleted"
    ;Check whether the read job completed successfully
    .if dError!=0
        .if dError==10038
            DbgWarning "Write failed : 10038 - Socket is not Connected"
            DbgHex [esi].hSocket,"closed socket handle"
        .else
            DbgDec dError,"Socket Error on send"
        .endif
        return ERROR_USERQUIT       
    .endif
    ;Just a "Courtesy Notification"
    .if [esi].pProtocol!=0
        mov eax,pJob
        OCall [esi].pProtocol::NetworkProtocol.OnWriteCompleted,esi,addr [eax].IOJob.xovl,bytes
    .endif
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
	mov edx,pJob
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
		    DbgWarning "Client will close its socket due to ERROR_BADPROTOCOL or ERROR_USERQUIT"		    
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

; ——————————————————————————————————————————————————————————————————————————————————————————————————
;Title   - Client.AllocateJob
;Purpose - Assign a free IOJob to a Client
;Returns - pIOJob
; ——————————————————————————————————————————————————————————————————————————————————————————————————
Method Client.AllocateJob,uses ebx esi,dwOperation
	SetObject esi	
	.if dwOperation==OPERATION_CONNECT
	    DbgWarning "Client is allocating a CONNECT Job"
	.elseif dwOperation==OPERATION_READ
	    DbgWarning "Client is allocating a READ Job"
	.elseif dwOperation==OPERATION_DOWRITE
	    DbgWarning "Client is allocating a WRITE Job"
	.elseif dwOperation==OPERATION_DOACCEPT
	    DbgWarning "Client is allocating ACCEPT Job"
	.else
	    DbgWarning "Client is allocating USER DEFINED Job"
	.endif
	OCall [esi].pJobAllocator::IOJobPool.Allocate,esi
	m2m [eax].IOJob.xovl.operation,dwOperation 
MethodEnd

;This method sends a defunct Job to the recycling pool
Method Client.FreeJob,uses esi ecx,pJob
	SetObject esi
    DbgWarning "FreeJob"
	mov ecx,pJob
	.switch [ecx].IOJob.xovl.operation
	.case OPERATION_READ
	    DbgText "Client is Freeing a READ Job"
		dec [esi].PendingReadCounter
	.case OPERATION_DOWRITE
	    DbgText "Client is Freeing a WRITE Job"
		dec [esi].PendingWriteCounter
    .case OPERATION_DOACCEPT
        DbgText "Client is Freeing an ACCEPT Job"
    .case OPERATION_CONNECT
        DbgText "Client is Freeing a CONNECT Job"
    .default
        DbgText "Client is Freeing a USER DEFINED Job"
	.endsw	
	OCall [esi].pJobAllocator::IOJobPool.Free,pJob
MethodEnd



;See USERID struct for the format of these strings
Method Client.SetUserID,uses esi,pName,pPass,pEmail
	SetObject esi

    DbgWarning "Client.SetUserID"

    DbgStr pName
    .if pName!=0
        invoke lstrlen,pName
    	mov [esi].UserID.namelength,al
	    .if eax!=0
	        invoke lstrcpy,addr [esi].UserID.namestring,pName
    	    DbgStr pName, "Client Name was set"
    	.else
    	    DbgWarning "Client Name was not provided"
    	.endif
  	.else
   	    DbgWarning "Client Name was not provided"
	.endif

    DbgStr pPass
	.if pPass!=0
    	invoke lstrlen,pPass
		mov [esi].UserID.passlength,al
		.if eax!=0
    		invoke lstrcpy,addr [esi].UserID.password,pPass
	    	DbgStr pPass,"Client Password was set"
	    .else
	        DbgText "Client Password was not provided"
	    .endif
	.else
		DbgText "Client Password was not provided"
	.endif

    DbgStr pEmail
	.if pPass!=0
    	invoke lstrlen,pEmail
		mov [esi].UserID.emaillength,al
		.if eax!=0
    		invoke lstrcpy,addr [esi].UserID.email,pEmail
	    	DbgStr pPass,"Client Email Address was set"
	    .else
	        DbgText "Client Email Address was not provided"
	    .endif
	.else
		DbgText "Client Email Address was not provided"
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
