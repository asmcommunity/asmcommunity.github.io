;Homer's IOCP Networking Engine
;This file contains a bunch of necessary data declarations (structs and so forth),
;as well as 'almost' all of the necessary 'includes' required.
;The following files must be included by YOU in your application:
;- ws2_32 and Mswsock (inc and lib)
;- Stream, Collection, and DwordCollection object classes


OPERATION_ENDED 	equ 666
OPERATION_DOACCEPT  equ 08008h
OPERATION_DOWRITE	equ 0BEEFh
OPERATION_WRITTEN   equ 0d00dh
OPERATION_READ		equ 0F00Dh
OPERATION_CONNECT	equ 04321h

ERROR_BADPROTOCOL equ -1
ERROR_BADVERSION  equ -2
ERROR_USERQUIT	  equ -666

CLIENT_DISCONNECTED equ 0FFFFFFFFh

WSA_FLAG_OVERLAPPED equ 1
WSA_INVALID_EVENT   equ 00h
WSA_FLAG_OVERLAPPED equ 01h
MSG_PARTIAL         equ 8000h
SO_UPDATE_ACCEPT_CONTEXT equ 700Bh
WSAEVENT typedef HANDLE

WSAOVERLAPPED struct
    Internal DWORD ?
    InternalHigh DWORD ?
    an_Offset DWORD ?
    OffsetHigh DWORD ?
    hEvent WSAEVENT ?
WSAOVERLAPPED ends
PWSAOVERLAPPED typedef ptr WSAOVERLAPPED

WSABUF struct
    len DWORD ?
    buf PBYTE ?
WSABUF ends
PWSABUF typedef ptr WSABUF

XOVL struct
    ovl WSAOVERLAPPED <>
    wsabuf WSABUF <>
    bytesused dd ? 	   ;#bytes currently in buffer 
    operation dd ?
    bytes 	  dd ? 	   ;#bytes due to this operation
    piojob 	  dd ?     ;ptr to wrapper object
    pbuforig  dd ?     ;original copy of wsabuf.buf pointer
    bufsizeorig dd ?   ;original copy of wsabuf.len value
XOVL ends 

 IOC_WS2    equ 008000000h
 SIO_GET_EXTENSION_FUNCTION_POINTER equ IOC_INOUT or IOC_WS2 or 6
 SO_UPDATE_CONNECT_CONTEXT equ 07010h

.data
ConnectExGuid GUID <25a207b9h,0ddf3h,4660h,{8eh,0e9h,76h,0e5h,8ch,74h,06h,3eh}>
.code

;Networking framework
MakeObjects UPNPNAT             ;Support for Universal Plug n Play based NAT Traversal (Port Forwarding)
;MakeObjects NetApp             ;Abstract class for the Application to talk to NetEngine
MakeObjects Pool				;BaseClass for recycling of arbitrary derived objects
MakeObjects IOJob				;this object represents one package of network data, owned by a Client
MakeObjects IOJobPool			;Derived class for recycling of IOJob objects
MakeObjects NetworkProtocol		;BaseClass for handling of Network Events
MakeObjects Client				;this object represents one TCP/IP network session, or if you like, "a live connection with remote user"
MakeObjects ClientPool			;Derived class for recycling of Client objects
MakeObjects ClientGroup         ;Support for arbitrary and hierarchical grouping of clients (eg chat channel)
;MakeObjects BaseProtocol		;Foundation of our protocol enforcement
;MakeObjects LobbyProtocol		;Actual useful protocol implementation
;MakeObjects GameProtocol		;TODO


;=============================================================

NetEngineID equ 32840
Object NetEngine,NetEngineID,Primer
    RedefineMethod    Init,     dword,dword,dword             
    RedefineMethod    Done
    VirtualMethod     Listen,   Pointer,dword                       ;pProtocol,dListenPort     
    VirtualMethod     ConnectTo,Pointer,dword,dword,LPSTR,LPSTR     ;pProtocol,dwHost,dwPort,pUsernameString,pPasswordString 
    VirtualMethod     UDP_Broadcast,Pointer,dword,dword,Pointer,dword;pProtocol,dwBroadcastAddress,dwPort,pData,dLen
    Private
       VirtualMethod     GoodbyeClient,Pointer,Pointer               ;pClient, pJob
       VirtualMethod     OnIOCompleted,Pointer,dword                 ;-> XOVL, dwBytes 
       VirtualMethod     QueueAcceptorClient,Pointer
       VirtualMethod     Worker
       ;This method counts available number of logical cpus
       VirtualMethod GetLogicalCPUCount
    PrivateEnd

  DefineVariable wsadata,WSADATA,{}
  DefineVariable io_completion_port,Pointer,NULL
  DefineVariable end_event,dword,NULL
  DefineVariable ShuttingDown,BOOL,FALSE
  DefineVariable dMinimum_Pending_Accepts,dword,0
  DefineVariable NumWorkerThreads,dword,NULL            ;#Worker Threads waiting for jobs to complete

  Embed RootClientGroup,ClientGroup                     ;Support for a Tree of Named communication channels
  Embed InClients,      ClientPool                      ;Pool of INBOUND Clients
  Embed OutClients,     ClientPool                      ;Pool of OUTBOUND Clients
  Embed Listeners,      ClientPool                      ;Pool of LISTENING Clients
  Embed IOJobs,         IOJobPool                       ;Pool of IO Jobs
 ; Embed Nat,NATUPNP                                    ;Support for NAT Traversal via UPNP
ObjectEnd

;=============================================================
align 4
if IMPLEMENT



; ——————————————————————————————————————————————————————————————————————————————————————————————————
;This method allocates a new Client and enqueues an Accept io job.
Method NetEngine.QueueAcceptorClient,uses esi,pListener
LOCAL pClient
    SetObject esi

    ;Allocate a Client, telling it to use the Listener's protocol
    mov pClient,$OCall ([esi].InClients::ClientPool.Allocate)
    mov edx,pListener
    push edx
    OCall pClient::Client.Init, [esi].io_completion_port,addr [esi].IOJobs,[edx].Client.pProtocol,pListener, FALSE
    
    ;Set Listener field
    mov eax,pClient
    pop [eax].Client.pListener

    ;Queue the accept job
    mov edx,pListener
    OCall pClient::Client.QueueAccept,[edx].Client.hSocket
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
;Destructor method - trash 'everything' :)
Method NetEngine.Done, uses esi
    SetObject esi

    ;Tell NetEngine Worker to die, and wait for the Death
    mov [esi].ShuttingDown,TRUE
    .while [esi].NumWorkerThreads!=0
        invoke Sleep,500
    .endw
    
    ;Destroy Events
    invoke CloseHandle, [esi].io_completion_port
    invoke CloseHandle, [esi].end_event

    ;Clean up resource pools
    OCall [esi].IOJobs::IOJobPool.Done
    OCall [esi].Listeners::ClientPool.Done
    OCall [esi].InClients::ClientPool.Done
    OCall [esi].OutClients::ClientPool.Done
    OCall [esi].RootClientGroup::ClientGroup.Done
    
    ;Close down WinSock
    invoke WSACleanup
MethodEnd

;Constructor method - initialize internal collections, threads, IOCP etc.
;Returns eax=pThis or NULL=Failed
Method NetEngine.Init,uses esi,MinClients,MaxClients,MaxIOJobs
LOCAL pClient,pJob,tid,NumWorkers

    SetObject esi
    invoke WSAStartup, 0202h, addr [esi].wsadata
    .if eax!= NULL
        DbgWarning "FAILED TO START WINSOCK 2.2 - QUITTING"
        xor eax,eax
        ExitMethod
    .endif    
    m2m [esi].dMinimum_Pending_Accepts, MinClients
    
    ;Initialize some collections
    OCall [esi].InClients::ClientPool.Init,esi, MaxClients
    OCall [esi].OutClients::ClientPool.Init,esi, MaxClients
    OCall [esi].Listeners::ClientPool.Init,esi, MaxClients
    OCall [esi].IOJobs::IOJobPool.Init,esi, MaxIOJobs, 4096
    
    ;The Root ClientGroup should be initialized with pOwner==NULL
    ;because this hierarchical object uses pOwner to point to Parent
    OCall [esi].RootClientGroup::ClientGroup.Init,NULL,$OfsCStr("Root"),$OfsCStr("Mother of all evil")

    ;Create the IOCP
    invoke CreateIoCompletionPort, INVALID_HANDLE_VALUE, 0, 0, 0
    .if eax==NULL
        DbgWarning "FAILED TO CREATE IO COMPLETION PORT - QUITTING"
        jmp @F
    .endif
    mov [esi].io_completion_port, eax   

    ;Create a special event to signal Worker Threads to terminate
    push esi
    invoke CreateEvent, 0, TRUE, FALSE, 0
    pop esi
    mov [esi].end_event, eax    
    .if eax== NULL
        DbgWarning "FAILED TO CREATE KILLNetEngine EVENT OBJECT - QUITTING"        
        jmp @F
    .endif
    
    ;Create one Worker Thread per available logical cpu
    mov NumWorkers, $OCall (esi.GetLogicalCPUCount)
    DbgDec eax,"Number of available logical cpus"
    xor ebx,ebx
    .while ebx<NumWorkers
        lea edx,    tid
        invoke CreateThread, 0, (16*1024), $MethodAddr(NetEngine.Worker), esi ,0, edx
        .if eax!=0
            invoke CloseHandle,eax
            inc [esi].NumWorkerThreads
        .else
            DbgWarning "FAILED TO CREATE WORKER THREAD - QUITTING"
    @@:        ExitMethod
        .endif    
        inc ebx
    .endw
    
    ;Return pThis
    mov eax,esi
MethodEnd

;Create a "Listener" - ie, begin listening on a given Port, and for a given Protocol.
;Success: eax=NULL
;Failed: eax=error code
Method NetEngine.Listen,uses esi,pProtocol,dListenPort
local pListener
LOCAL sockaddrin:sockaddr_in
    SetObject esi

    ;Create a Client to represent our Listening socket
    mov pListener,$OCall ([esi].Listeners::ClientPool.Allocate)
    DbgHex pListener
    OCall pListener::Client.Init, [esi].io_completion_port,addr [esi].IOJobs,pProtocol,NULL,FALSE

    DbgText "Created a Listening socket"
    
    ;Define our Listener's LOCAL Network Address
    invoke htons, dListenPort
    mov edx,pListener
    mov [edx].Client.Sin_LocalHost.sin_port,ax
    mov [edx].Client.Sin_LocalHost.sin_family, AF_INET
    mov [edx].Client.Sin_LocalHost.sin_addr.S_un.S_addr, INADDR_ANY 
    invoke bind, [edx].Client.hSocket, addr [edx].Client.Sin_LocalHost, sizeof sockaddr_in
    .if eax!= NULL
        invoke WSAGetLastError
        DbgWarning "FAILED TO BIND LISTENING SOCKET TO PORT - QUITTING"
        DbgDec eax,"error code"
        jmp @F
    .endif      
    DbgText "SocketAddress has been Bound to Socket"

    ;Start listening
    mov edx,pListener
    invoke listen,[edx].Client.hSocket, 127
    .if eax!= NULL
        DbgWarning "LISTENING SOCKET REFUSES TO LISTEN - QUITTING"
        jmp @F
    .endif
     DbgText "Socket is Listening"
    
    ;Bind the Listening Socket to the IOCP
    mov edx,pListener
    invoke CreateIoCompletionPort, [edx].Client.hSocket, [esi].io_completion_port, 0, 0
    .if eax==NULL
        DbgWarning "FAILED TO BIND LISTENING SOCKET TO IO COMPLETION PORT - QUITTING"
        mov eax,-1
        jmp @F
    .endif 
    DbgText "Socket has been Bound to IOCP"

    ;Queue some Acceptor clients and their Accept jobs
    xor ebx,ebx
    .while ebx<[esi].dMinimum_Pending_Accepts
        OCall esi.QueueAcceptorClient,pListener
        inc ebx    
    .endw
    DbgText "Accepts are pending"
    
     xor eax,eax
     
@@:
MethodEnd

;Creates an OUTBOUND Client, and initiates a Connect attempt.
;Returns TRUE/FALSE (eax=pClient or NULL)
Method NetEngine.ConnectTo,uses esi,pProtocol,dwRemoteHost,dwPort,pUsername,pPass
LOCAL pClient
LOCAL pIOJob
LOCAL userid[64]:byte
    ;Allocate a free Client and (re)initialize it
    ;The FALSE Boolean indicates that we want a TCP socket
    SetObject esi
    mov pClient,$OCall ([esi].OutClients::ClientPool.Allocate)
    OCall pClient::Client.Init, [esi].io_completion_port,addr [esi].IOJobs,pProtocol,NULL,FALSE
    
    ;Register this socket with iocp, because we want socket event notifications
    mov edx,pClient
    invoke CreateIoCompletionPort, [edx].Client.hSocket, [esi].io_completion_port, pClient, 0

    ;Set the Client's (desired) Name and PasswordHash (USERID)
    ;These can be ignored if not required by a given Protocol
    .if pUsername!=0
        invoke lstrlen,pUsername
        inc eax
        mov byte ptr userid[0],al
        lea edx,userid[1]
        invoke lstrcpy,edx,pUsername    
        OCall pClient::Client.SetUserID,addr userid,NULL
    .endif

    ;Initiate the Connection attempt : Returns eax=pClient or NULL
    DbgWarning "NetEngine issues a Connect"
    OCall pClient::Client.QueueConnect, dwRemoteHost, dwPort 
MethodEnd

;This method is for initiating a BROADCAST UDP session.
;These are 'connectionless', so its a little different to usual.
;But as usual, the Protocol Handler is what matters.
;WSASendTo and WSARecvFrom are used in the Client class.
;Returns pClient (success) or NULL (failed)
Method NetEngine.UDP_Broadcast,uses esi, pProtocol,dwBroadcastAddress,dwPort,pData,dLen
LOCAL pClient,bOptLen,bOptVal
    ;Allocate an outbound Client and (re)initialize it as a UDP Socket
    SetObject esi
    mov pClient,$OCall ([esi].OutClients::ClientPool.Allocate)
    OCall pClient::Client.Init, [esi].io_completion_port,addr [esi].IOJobs,pProtocol,NULL,TRUE
    ;Register this socket with iocp, because we want socket event notifications
    mov edx,pClient
    invoke CreateIoCompletionPort, [edx].Client.hSocket, [esi].io_completion_port, edx, 0
    ;Enable the SO_BROADCAST option
	mov bOptVal,TRUE
	mov bOptLen ,sizeof BOOL
	mov edx,pClient
	invoke setsockopt,[edx].Client.hSocket, SOL_SOCKET, SO_BROADCAST, addr bOptVal, bOptLen 
	.if eax==SOCKET_ERROR
		OCall esi.GoodbyeClient,pClient,NULL
	.else
    	;Set the remote broadcast address we'll be sending to
    	;Note this will be overwritten if/when we receive a response!
    	OCall pClient::Client.SetRemoteHost,dwBroadcastAddress,dwPort
    	
        ;Set the local address we'll use to receive data
        ;In this case its "any remote ip, any remote port"
    	OCall pClient::Client.SetLocalHost,INADDR_ANY,NULL
    	
    	;Bind to local host address, else we cannot receive data
    	mov edx,pClient
    	invoke bind,[edx].Client.hSocket,addr [edx].Client.Sin_LocalHost,sizeof sockaddr_in
    	
    	;Queue a READ to capture the response, if any
    	;Note we set up the 'pending recv' BEFORE we send anything..
    	;We're setting a TRAP to capture the response!!
    	OCall pClient::Client.QueueRead
    	.if eax!=0
    	    ;The recv trap has been set up..
            ;Queue a WRITE to send our broadcast packet
            OCall pClient::Client.QueueWrite,pData,dLen
            ;Return ptr to Client
            mov eax,pClient
        .else
            ;failed to queue a READ operation
            OCall esi.GoodbyeClient,pClient,NULL
            xor eax,eax
        .endif
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————

;This method determines what type of IO completed
;and then handles it according to its type.
Method NetEngine.OnIOCompleted,uses esi ebx ecx,pXOVL,bytes
LOCAL pClient,pJob,flags,dError

    invoke IsBadReadPtr,pXOVL,sizeof XOVL
    .if eax==TRUE
        DbgWarning "DANGER - XOVL BAD POINTER DISCARDED","WEIRDNESS"
        DbgHex pXOVL,"WEIRDNESS"
        ExitMethod
    .endif
    mov esi,pXOVL
    mov eax,[esi].XOVL.piojob
    mov pJob,eax
    invoke IsBadReadPtr,pJob,sizeof IOJob
    .if eax==TRUE
        DbgWarning "DANGER - IOJob BAD POINTER DISCARDED","WEIRDNESS"
        DbgHex pJob,"WEIRDNESS"
        ExitMethod
    .endif    
    
    ;Determine whether the Operation completed successfully or not
    mov dError, NULL
    
    mov eax,pJob
    m2m pClient,[eax].IOJob.pOwner,edx
    invoke WSAGetOverlappedResult,[edx].Client.hSocket,pXOVL,addr bytes,FALSE,addr flags
    .if eax==FALSE
          invoke WSAGetLastError
          .if eax==996
              ;if we get this error, it means the operation is still pending...
              ;we can expect another Completion Notification soon,
              ;so dont do anything and certainly do not free this job
              ExitMethod
          .endif
        DbgWarning "A NETWORK IO Operation has FAILED"
          DbgDec eax,"ERROR CODE"
           mov dError,$invoke (WSAGetLastError)
           .if eax==ERROR_NETNAME_DELETED ;Fail silently on this error
               ExitMethod
           .elseif eax==WSAETIMEDOUT && [esi].XOVL.operation==OPERATION_CONNECT
               DbgWarning "An outbound connection attempt has timed out."
           .elseif eax==WSAEADDRNOTAVAIL
               DbgWarning "Remote Host Address not available"
           .endif
      .endif
    
    ;regardless of whether the completed job succeeded or failed,
    ;we will pass the notification to the protocol handler
     
    .switch [esi].XOVL.operation
    .case OPERATION_DOACCEPT            
        SetObject esi
        OCall pClient::Client.OnAcceptCompleted,[esi].io_completion_port,pJob,dError
        ;We just consumed a pending Acceptor client
        mov edx,pClient
        OCall QueueAcceptorClient,[edx].Client.pListener

    .case OPERATION_READ
        SetObject esi
                            
        ;Detect special case of Client remotely Disconnecting..
          mov ebx,pXOVL
        .if bytes==0 && [ebx].XOVL.bytes==0            
            ;A read of ZERO indicates that the session has terminated.
@@:            ;We should release this redundant Client object
            OCall esi.GoodbyeClient,pClient,pJob
            ExitMethod
        .endif    
   
        OCall pClient::Client.OnReadCompleted,pJob, bytes ,dError
        .if eax==ERROR_BADPROTOCOL || eax==ERROR_USERQUIT
            jmp @B
        .elseif eax==TRUE ;The IOJob buffer requires purging..    
            DbgWarning "Resetting Buffer"
            OCall pJob::IOJob.Reset
            OCall pClient::Client.DoRead,pXOVL     ;Requeue this IOJob as a Read    
            ExitMethod
        .endif
        
        ;Now we've processed the data from the Read,
        ;let's decide what to do with the defunct IOJobs..
        mov ebx,pClient
        .if [ebx].Client.PendingReadCounter==1
            ;Without purging the buffer,
            ;Requeue this IOJob as a Read
            OCall pClient::Client.DoRead,pXOVL
        .else
            ;We can afford to Free this IOJob
            OCall pClient::Client.FreeJob,pJob
        .endif

    .case OPERATION_DOWRITE
        ;Toss the completed IOJob back in the IOJobPool
        ;mov ebx,pClient
;        .if [ebx].Client.PendingWriteCounter==1
    ;        DbgDec [ebx].Client.PendingWriteCounter,"Freeing IOJob [SEND]..","Worker"
            OCall pClient::Client.FreeJob,pJob
;        .endif           

    .case OPERATION_CONNECT        
        ;Pass this notification to the Client's Protocol handler
        OCall pClient::Client.OnConnectCompleted,pJob,dError
        .if dError!=0
            OCall GoodbyeClient,pClient,pJob
        .endif
    .default
        DbgWarning "UNKNOWN IO OPERATION IDENTIFIER"
    .endsw
    
MethodEnd


;This method trashes a Client and optionally, an IOJob.
;It contains some safety checking that may be removed at a later date.
;WE SHOULD REMOVE CLIENT FROM ALL CLIENTGROUPS
Method NetEngine.GoodbyeClient,uses esi,pClient,pJob
LOCAL pListener
    SetObject esi    
    DbgWarning "Terminating Client"

    mov eax,pClient
    m2m pListener,[eax].Client.pListener,edx

    ;Notify the dead Client's Protocol handler
    ;in order to free any Protocol-specific client resources
    OCall pClient::Client.Goodbye,pClient
    
    ;Remove the Client from all ClientGroups
    OCall [esi].RootClientGroup::ClientGroup.GoodbyeClient, pClient

    ;Attempt to locate and release the Client..
    ;We keep inbound and outbound clients in separate pools
    ;Which pool does this client belong to?
    ;(Maybe clients should be tagged with this information?)
    OCall [esi].InClients::ClientPool.IsActive,pClient
    .if eax==TRUE
        ;If there was a defunct Job passed in with the defunct Client,
        ;we'd better trash that object as well.
        .if pJob!=0
            OCall [esi].IOJobs::IOJobPool.IsActive,pJob
            .if eax==TRUE
                OCall pClient::Client.FreeJob,pJob
            .else
                DbgText "Attempt to free inactive IOJob suppressed"
            .endif
        .endif
        OCall [esi].InClients::ClientPool.Free,pClient
        DbgWarning "Inbound Client is set free"
    .else
        OCall [esi].OutClients::ClientPool.IsActive,pClient
        .if eax==TRUE
            .if pJob!=0
                OCall [esi].IOJobs::IOJobPool.IsActive,pJob
                .if eax==TRUE
                    OCall pClient::Client.FreeJob,pJob
                .else
                    DbgText "Attempt to free inactive IOJob suppressed"
                .endif
            .endif
            OCall [esi].OutClients::ClientPool.Free,pClient
            DbgWarning "Outbound Client is set free"
        .else
            DbgWarning "Error - NetEngine.GoodbyeClient failed to locate target object"
        .endif
    .endif
    
    ;Ensure we have sufficient pending accepts
    ;associated with the dead client's listener
    mov eax,[esi].dMinimum_Pending_Accepts    
    .if [esi].InClients.Available.dCount<eax
        .if pListener!=0    ;if NULL, socket is outbound, forget it
            OCall esi.QueueAcceptorClient,pListener
        .endif
    .endif
MethodEnd
; ——————————————————————————————————————————————————————————————————————————————————————————————————

;Here is the HEART AND SOUL of the NetEngine.
;This thread is responsible for waiting on IOCP completion notifications
;and passing them to the 'OnIOCompleted' method.
;There can be several of these Worker threads operating asynchronously.
;===================================================
Method NetEngine.Worker, uses esi
    LOCAL compport:HANDLE
    LOCAL pJob:Pointer
    LOCAL pClient:Pointer
    LOCAL bytes:DWORD
    LOCAL accept_socket:SOCKET
    LOCAL wsabuf:PWSABUF
    LOCAL count:DWORD
    LOCAL flags:DWORD
    LOCAL err:DWORD
    LOCAL pOVL:Pointer
    LOCAL pAcceptJob:Pointer
    
    SetObject esi
    m2m compport, [esi].io_completion_port

    .repeat
        .break .if [esi].ShuttingDown==TRUE
        ;The GetQueuedCompletionStatus api call will (if successful)
        ;return two useful pieces of information to us.
        ;One is the "completion key" of the completed io,
        ;the other is a ptr to the XOVL which represents 
        ;the io operation which (we hope) is completed.
        ;We can use either, or both of these.
        ;I am using pClient as a completion key,
        ;as we can obtain pIOJob from the xovl.
        ;In fact we can also obtain pClient from IOJob.pOwner !!!
        invoke GetQueuedCompletionStatus, compport, addr bytes,
               addr pClient, addr pOVL, 500
        .if eax==TRUE
            .if pClient==NULL && pOVL==NULL
                DbgText "GetQueuedCompletionStatus went horribly wrong.","Worker"
                .break
            .endif
            ;Check for special 'time to die' message
            mov edx,pOVL
            .if [edx].XOVL.operation==OPERATION_ENDED
                DbgWarning "NetEngine WORKER DEATH"
                .break
            .endif
            ;Process the completed IOJob
            OCall esi.OnIOCompleted,pOVL,bytes
        .else
            .if pOVL==NULL    ;simply means timeout
                .if [esi].ShuttingDown==TRUE
                    .break    
                .endif

            .else
                ;An IOJob was dequeued, but apparently it FAILED
                ;Lets take a closer look shall we?
                ;But first, check for special 'time to die' message
                mov edx,pOVL
                .if [edx].XOVL.operation==OPERATION_ENDED
                    DbgWarning "NetEngine WORKER DEATH"
                    .break
                .endif
                OCall esi.OnIOCompleted,pOVL,bytes
            .endif
            
        .endif
    .until 0 ;<-- condition 0 = infinite loop unless YOU break out of it,
             ;    either using .break or by jumping.
    DbgWarning "NetEngine Worker Death"
    dec [esi].NumWorkerThreads
    invoke ExitThread,0
MethodEnd        
;===================================================
;====================================    
;=======================
;===========

;This method counts the number of logical cpus available to the Process,
;this is the optimal number of Worker Threads.
Method NetEngine.GetLogicalCPUCount,uses ebx edi esi
LOCAL dProcessAffinity:dword, dSystemAffinity:dword, hCurrentProcessHandle:Handle
LOCAL _si:SYSTEM_INFO
    SetObject esi

    invoke GetSystemInfo,addr _si
    DbgDec _si.dwNumberOfProcessors
    
    mov hCurrentProcessHandle, $invoke(GetCurrentProcess)
    invoke GetProcessAffinityMask, hCurrentProcessHandle, addr dProcessAffinity, addr dSystemAffinity
    .if eax != FALSE
      xor edi, edi
      xor ebx, ebx
      inc edi
@@:
      test edi, dSystemAffinity
      .if !Zero?
        ;Check if this logical processor is available to this process
        .if $invoke(SetProcessAffinityMask, hCurrentProcessHandle, edi)
          invoke Sleep, 0               ;Give OS time to switch CPU
          inc ebx
        .endif
      .endif
      shl edi, 1                        ;Prepare mask for the next logical processor
      jz @F                             ;Exit if the mask is above 32
      cmp edi, dSystemAffinity
      jbe @B
@@:
      ;Set previous processor affinity
      invoke SetProcessAffinityMask, hCurrentProcessHandle, dProcessAffinity

      mov eax, ebx
    .else
        DbgDec $invoke (GetLastError),"GetProcessAffinityMask Failure Reason"
        mov eax,1
    .endif

MethodEnd



endif
