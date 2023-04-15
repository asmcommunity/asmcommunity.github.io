;Homer's IOCP Networking Engine
;This file contains a bunch of necessary data declarations (structs and so forth),
;as well as 'almost' all of the necessary 'includes' required.
;The following files must be included by YOU in your application:
;- ws2_32 and Mswsock (inc and lib)
;- Stream, Collection, and DwordCollection object classes


;These values define the various Operation Types of IOJobs
;Anything outside this range is a USER DEFINED job type.
OPERATION_ENDED 	equ 0
OPERATION_DOACCEPT  equ 1
OPERATION_DOWRITE	equ 2
OPERATION_READ		equ 3
OPERATION_CONNECT	equ 4

;These values define some standard error codes
ERROR_BADPROTOCOL equ -1    ;Data received does not conform to network protocol
ERROR_BADVERSION  equ -2    ;Packet Header contains incorrect Protocol version Identifier
ERROR_USERQUIT	  equ -666  ;Client has terminated the session

;These values should have been defined in the windows header files
WSA_FLAG_OVERLAPPED equ 1
WSA_INVALID_EVENT   equ 00h
WSA_FLAG_OVERLAPPED equ 01h
MSG_PARTIAL         equ 8000h
SO_UPDATE_ACCEPT_CONTEXT equ 700Bh
WSAEVENT typedef HANDLE
IOC_WS2    equ 008000000h
SIO_GET_EXTENSION_FUNCTION_POINTER equ IOC_INOUT or IOC_WS2 or 6
SO_UPDATE_CONNECT_CONTEXT equ 07010h
.data
ConnectExGuid GUID <25a207b9h,0ddf3h,4660h,{8eh,0e9h,76h,0e5h,8ch,74h,06h,3eh}>
.code

;These structs should have been defined in the windows header files
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

;The Extended Overlapped structure
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

;The User Identifier structure
USERID struct
	namelength  db ?
	namestring  db 63 dup (?)
	passlength  db ?
	password    db 63 dup (?)
	emaillength db ?
	email       db 63 dup (?)
USERID ends


MakeObjects Pool				;BaseClass for recycling of arbitrary derived objects
MakeObjects IOJob				;this object represents one package of network data, owned by a Client
MakeObjects IOJobPool			;Derived class for recycling of IOJob objects
MakeObjects NetworkProtocol		;BaseClass for handling of Network Events

;=============================================================

.data
upnp_tcp_port dd 0
NAT_IP_String db 256 dup (0)

;This string is used for upnp discovery via udp broadcast
SEARCH_REQUEST_STRING db	"M-SEARCH * HTTP/1.1",13,10,
                                    "HOST: 239.255.255.250:1900",13,10,
        							"ST:urn:schemas-upnp-org:device:InternetGatewayDevice:1",13,10,	
        							'Man:"ssdp:discover"',13,10,
            						"MX: 3",13,10,13,10,0
.code

;Code for this object is located in UPNPNATProtocol.inc 
Object UPNPNATProtocol,2345234,NetworkProtocol
    RedefineMethod FindDelimiter,       Pointer,Pointer
    RedefineMethod ProcessReceivedData, Pointer
    RedefineMethod OnConnectCompleted,  Pointer,dword
    RedefineMethod OnWriteCompleted,    Pointer,Pointer,dword
    VirtualMethod  AddPortMapping,      dword,BOOL
    DefineVariable Discovery_Stage,dword,0
    DefineVariable hAddPortMapping,HANDLE,0
    DefineVariable describe_url,byte, 1024 dup <(0)>
    DefineVariable controlUrl,  byte, 1024 dup <(0)>
ObjectEnd

;Code for this object is located in Client.inc
ClientID equ 93846
Object Client,ClientID,DwordCollection
	RedefineMethod Init,Pointer,Pointer,Pointer,Pointer,Pointer,BOOL
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
   	VirtualMethod OnDisconnected,    dword                         ;dError
	VirtualMethod OnReadCompleted,Pointer,dword,dword       ;pJob,bytes,dError
	VirtualMethod OnWriteCompleted,Pointer,dword,dword              ;bytes, dError
	VirtualMethod OnUserIOCompleted,Pointer,dword
	
	;Methods to (re)issue jobs
    VirtualMethod DoRead, Pointer				;-> XOVL	
	VirtualMethod DoWrite, Pointer				;-> XOVL    
	VirtualMethod DoConnect, Pointer            ;-> XOVL   
	
	VirtualMethod SetUserID,LPSTR,LPSTR,LPSTR	
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
	
	DefineVariable bClientPassedZombieTest,BOOL,FALSE
	
	;For inbound connections, this field is used to determine
	;how long the client has been accepted without sending anything.
	;The client has FIVE SECONDS to send one full protocol packet
	;before being summarily dismissed by the AntiZombie thread in NetEngine.
	DefineVariable dTimeIdleSinceAccept,dword,NULL
	;For inbound connections, this field is used to determine
	;how long the client has been idle without sending anything,
	;given that it has previously sent a valid packet and passed
	;the initial 'zombie' test.. The client has TEN MINUTES
	;to send another valid packet, or its going to be booted.
	DefineVariable dTimeIdleSinceWrite,dword,NULL
	
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
	DefineVariable hiocp,HANDLE,NULL
	DefineVariable pListener,Pointer,NULL
	DefineVariable pProtocol,Pointer,NULL
	DefineVariable hSocket,dword,NULL
	DefineVariable Sin_RemoteHost,sockaddr_in,{<>}
	DefineVariable Sin_LocalHost,sockaddr_in,{<>}
	DefineVariable bSocketIsUDP,BOOL,FALSE
	
ObjectEnd

;Code for this object is located in ClientGroup.inc
ClientGroupID equ 72543
Object ClientGroup, ClientGroupID, DwordCollection
	RedefineMethod Init,Pointer,LPSTR,LPSTR
	RedefineMethod Done
	VirtualMethod	AddSubGroup,LPSTR,LPSTR
	VirtualMethod	RemoveSubGroup,Pointer
	VirtualMethod	FindGroupByName,LPSTR
	VirtualMethod   FindUserByName,LPSTR
	VirtualMethod   FindClient,Pointer
	VirtualMethod	GoodbyeClient,Pointer	;pClient
	VirtualMethod   Send,Pointer,Pointer,dword
	VirtualMethod   Broadcast,Pointer,Pointer,dword
	DefineVariable Locked,byte,FALSE
	DefineVariable pszGroupName,LPSTR,0
	DefineVariable pszGroupDesc,LPSTR,0
	Embed SubGroups,Collection
ObjectEnd

;Code for this object is located in ClientPool.inc
ClientPoolID equ 45219
Object ClientPool,ClientPoolID,Pool
   RedefineMethod Done
   RedefineMethod CreatePooledObject
   RedefineMethod Free,Pointer
   ;-------------
ObjectEnd

;This is the main object for this project, code located in this file
NetEngineID equ 32840
Object NetEngine,NetEngineID,Primer
    RedefineMethod    Init,         dword,dword,dword,dword            
    RedefineMethod    Done
    VirtualMethod     Listen,       Pointer,dword                       ;pProtocol,dListenPort     
    VirtualMethod     Connect,      Pointer,dword,dword                 ;pProtocol,dwHost,dwPort
    VirtualMethod     UDP_Broadcast,Pointer,dword,dword,Pointer,dword;pProtocol,dwBroadcastAddress,dwPort,pData,dLen
    VirtualMethod     GoodbyeClient,Pointer,Pointer               ;pClient, pJob

    Private
       VirtualMethod     OnIOCompleted,Pointer,dword                 ;-> XOVL, dwBytes 
       VirtualMethod     QueueAcceptorClient,Pointer
       VirtualMethod     Worker
       VirtualMethod     AntiZombie
       ;This method counts available number of logical cpus
       VirtualMethod GetLogicalCPUCount
    PrivateEnd

  DefineVariable wsadata,WSADATA,{}
  DefineVariable io_completion_port,Pointer,NULL
  DefineVariable end_event,dword,NULL
  DefineVariable ShuttingDown,BOOL,FALSE
  DefineVariable dMinimum_Pending_Accepts,dword,0
  DefineVariable NumWorkerThreads,dword,NULL            ;#Worker Threads waiting for jobs to complete

  DefineVariable bLocalMachineIsBehindNATDevice,BOOL,FALSE
  DefineVariable hWake,BOOL,FALSE

  ;Switch to enable zombie killer thread
  DefineVariable IdleKillerActive,BOOL,FALSE

  DefineVariable _CS, CRITICAL_SECTION,{<>}

  Embed RootClientGroup,ClientGroup                     ;Support for a Tree of Named communication channels
  Embed InClients,      ClientPool                      ;Pool of INBOUND Clients
  Embed OutClients,     ClientPool                      ;Pool of OUTBOUND Clients
  Embed Listeners,      ClientPool                      ;Pool of LISTENING Clients
  Embed IOJobs,         IOJobPool                       ;Pool of IO Jobs
  Embed upnp_protocol,  UPNPNATProtocol                 ;Support for NAT Traversal via UPNP
 
ObjectEnd


;Include the code for these 'friend' objects
MakeObjects Client				;this object represents one TCP/IP network session, or if you like, "a live connection with remote user"
MakeObjects ClientPool			;Derived class for recycling of Client objects
MakeObjects ClientGroup         ;Support for arbitrary and hierarchical grouping of clients (eg chat channel)
MakeObjects UPNPNATProtocol

;=============================================================
align 4
if IMPLEMENT





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
    DbgText "NetEngine calls RootClientGroup::ClientGroup.Done"
    OCall [esi].RootClientGroup::ClientGroup.Done
    DbgText "NetEngine calls IOJobs::IOJobPool.Done"
    OCall [esi].IOJobs::IOJobPool.Done
    DbgText "NetEngine calls Listeners::ClientPool.Done"
    OCall [esi].Listeners::ClientPool.Done
    DbgText "NetEngine calls InClients::ClientGroup.Done"
    OCall [esi].InClients::ClientPool.Done
    DbgText "NetEngine calls OutClients::ClientGroup.Done"
    OCall [esi].OutClients::ClientPool.Done
    DbgWarning "NetEngine says NetEngine Resources are released"

    
    invoke DeleteCriticalSection,addr [esi]._CS
    
    ;Close down WinSock
    invoke WSACleanup
MethodEnd

;Constructor method - initialize internal collections, threads, IOCP etc.
;Returns eax=pThis or NULL=Failed
Method NetEngine.Init,uses esi,MinClients,MaxClients,MaxIOJobs,bWantPortForwarding:BOOL
LOCAL pClient,pJob,tid,NumWorkers
LOCAL buf[256]:byte
    SetObject esi
    invoke WSAStartup, 0202h, addr [esi].wsadata
    .if eax!= NULL
        DbgWarning "FAILED TO START WINSOCK 2.2 - QUITTING"
        xor eax,eax
        ExitMethod
    .endif    
    m2m [esi].dMinimum_Pending_Accepts, MinClients
    
    invoke InitializeCriticalSectionAndSpinCount,addr [esi]._CS,500
    
    ;Initialize some collections
    OCall [esi].InClients::ClientPool.Init,esi, MaxClients
    OCall [esi].OutClients::ClientPool.Init,esi, MaxClients
    OCall [esi].Listeners::ClientPool.Init,esi, MaxClients
    OCall [esi].IOJobs::IOJobPool.Init,esi, MaxIOJobs, 8192
    
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
        

    .if bWantPortForwarding==TRUE

            ;Verify whether the local host is behind a NAT device
            ;This is asynchronous, and will take some moments to complete
            OCall [esi].upnp_protocol::UPNPNATProtocol.Init,esi,NULL 
            invoke inet_addr,$OfsCStr("239.255.255.250") ;udp broadcast address for upnp
        .data

        .code
        
        	mov pClient,$OCall (esi.UDP_Broadcast,addr [esi].upnp_protocol,eax,1900,addr SEARCH_REQUEST_STRING,sizeof SEARCH_REQUEST_STRING)
            


        ;We should not return until the UPNP discovery is completed.
        ;This would be a good time to display a splash screen!!
        
        ;We shall temporarily suspend this Thread,
        ;while we wait for upnp discovery to complete..
        ;I've chosen to wait on an event object
        ;because I found that Sleep(Ex) trashes esi 
        ;while our worker threads are performing 
        ;the socket io, and it really bothered me.
        ;Turned out that WaitForSingleObject did not trash esi.
        mov [esi].hWake,$invoke (CreateEvent,0,FALSE,FALSE,0)
        invoke WaitForSingleObject,eax,20000
        push eax
        invoke CloseHandle,[esi].hWake
        pop eax
        .if eax==WAIT_OBJECT_0
            DbgWarning "UPNP control url was discovered"
            mov eax,esi
        .else
            ;UPNP discovery has failed due to timeout...
            ;Perform a cheap test to prove that the local machine is behind a NAT,
            ;by confirming that it falls within one of the three IANA private blocks
            invoke gethostname,addr buf,sizeof buf
            invoke gethostbyname,addr buf 
            mov eax,[eax+12]
            mov eax,[eax]
            mov eax,[eax]        ;eax = local ip address, in network byte order
            .if al==10 ;check for range 1 : 0000000A - FFFFFF0A = 10.0.0.0 - 10.255.255.255
                DbgWarning "IP address appears to be in range 1"
            .elseif al==172 ; check for range 2 : 000010AC - FFFF1FAC = 172.16.0.0 – 172.31.255.255
                shr eax,8
                .if al>=16 && al<=31
                    DbgWarning "IP address appears to be in range 2"
                    jmp @LAN
                .endif
            .elseif al==192       ;check for range 3 : 0000A8C0 - FFFFA8C0 = 192.168.0.0 - 192.168.255.255
                shr eax,8
                .if al==168
                    DbgWarning "IP address appears to be in range 3"
                    jmp @LAN
                .endif             
            .endif
            ;IP address appears to be an external ip address
            DbgWarning "IP address appears to be an external ip address"
            return esi
                   
@LAN:       DbgWarning "Error - UPNP discovery has timed out"
            DbgWarning "Possible reasons for this are:"
            DbgWarning "-UPNP service is not running"
            DbgWarning "-Your firewall is blocking the UPNP ports (udp 1900, tcp 2869)"
            DbgWarning "-Your router has UPNP disabled"
            DbgWarning "-Your router is not UPNP compliant"
            .data
            szCaption db "WARNING",0
            szWarning db "Your IP address appears to be behind a Router or other NAT device,",13,10
                      db "and UPNP discovey has timed out.",13,10
                      db "This means you cannot accept incoming connections from the Internet,",13,10
                      db "however you can still operate a Server on your local network (LAN).",13,10    
                      db "If this is acceptable, choose the YES option below to continue.",13,10,13,10
                      db "If this is NOT acceptable, and you ARE behind a NAT device,"
                      db "please confirm that the following are TRUE:",13,10,13,10
                      db "-Your firewall is NOT blocking the UPNP ports (udp 1900, tcp 2869)"
                      db "-The UPNP service is running (Universal Plug and Play)",13,10
                      db "If behind a router,",13,10
                      db "-Your router is UPNP compliant",13,10
                      db "-Your router has UPNP enabled",13,10,13,10
                      db "Continue without the ability to accept incoming connects from the internet?",0
                      
            .code
            invoke MessageBox,0,addr szWarning,addr szCaption,MB_YESNO
            .if eax==IDYES
                mov eax,esi
            .else
                ;Ouch
                xor eax,eax 
            .endif
            
        .endif
    .else
        mov eax,esi
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
;This method allocates a new Client and enqueues an Accept io job.
Method NetEngine.QueueAcceptorClient,uses esi,pListener
LOCAL pClient
    SetObject esi

    ;Allocate a Client, telling it to use the Listener's protocol
    mov pClient,$OCall ([esi].InClients::ClientPool.Allocate)
    mov edx,pListener
    push edx
    OCall pClient::Client.Init, esi, [esi].io_completion_port,addr [esi].IOJobs,[edx].Client.pProtocol,pListener, FALSE
    
    ;Set Listener field
    mov eax,pClient
    pop [eax].Client.pListener

    ;Queue the accept job
    mov edx,pListener
    OCall pClient::Client.QueueAccept,[edx].Client.hSocket
MethodEnd

;Create a "Listener" - ie, begin listening on a given Port, and for a given Protocol.
;Success: eax=NULL
;Failed: eax=error code
Method NetEngine.Listen,uses esi,pProtocol,dListenPort
local pListener
LOCAL sockaddrin:sockaddr_in
    SetObject esi
    
    .if [esi].IdleKillerActive==FALSE
        ;Create one AntiZombie thread (protects server from zombie DOS clients)
        lea edx,    pListener ;just reusing this local for thread id
        invoke CreateThread, 0, (16*1024), $MethodAddr(NetEngine.AntiZombie), esi ,0,edx
        mov [esi].IdleKillerActive,TRUE        
    .endif
    
    .if [esi].bLocalMachineIsBehindNATDevice==TRUE
            ;We should create a Port Forwarding
            ;otherwise we won't be able to accept incoming connects
        OCall [esi].upnp_protocol::UPNPNATProtocol.AddPortMapping,dListenPort,FALSE

;        invoke inet_addr,addr NAT_IP_String
;    	OCall esi.ConnectTo,addr [esi].upnp_protocol,eax,upnp_tcp_port,NULL,NULL 
    .else
        DbgWarning "Machine NOT behind NAT device"
    .endif
    
    ;Create a Client to represent our Listening socket
    mov pListener,$OCall ([esi].Listeners::ClientPool.Allocate)
    DbgHex pListener
    OCall pListener::Client.Init, esi,[esi].io_completion_port,addr [esi].IOJobs,pProtocol,NULL,FALSE
    .if eax==TRUE
        DbgText "Created a Listening socket"

        mov edx,pListener
        DbgHex [edx].Client.hSocket
    
        ;Set local address and bind it
        OCall pListener::Client.SetLocalHost,INADDR_ANY,dListenPort
        mov edx,pListener
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
    .else
        DbgWarning "Failed Client.Init"
        mov eax,-1
    .endif

@@: .if eax==-1
        push eax
        OCall [esi].Listeners::ClientPool.Free, pListener
        pop eax
    .endif
MethodEnd

;Creates an OUTBOUND Client, and initiates a Connect attempt.
;Returns TRUE/FALSE (eax=pClient or NULL)
Method NetEngine.Connect,uses esi,pProtocol,dwRemoteHost,dwPort
LOCAL pClient
LOCAL pIOJob

    DbgWarning "NetEngine.ConnectTo is being called"


    ;Allocate a free Client and (re)initialize it
    ;The FALSE Boolean indicates that we want a TCP socket
    SetObject esi
    mov pClient,$OCall ([esi].OutClients::ClientPool.Allocate)
    OCall pClient::Client.Init, esi,[esi].io_completion_port,addr [esi].IOJobs,pProtocol,NULL,FALSE
    .if eax==TRUE
        ;Register this socket with iocp, because we want socket event notifications
        mov edx,pClient
        invoke CreateIoCompletionPort, [edx].Client.hSocket, [esi].io_completion_port, pClient, 0
    
        ;Initiate the Connection attempt : Returns eax=pClient or NULL
        DbgWarning "NetEngine issues a Connect"
        OCall pClient::Client.QueueConnect, dwRemoteHost, dwPort 
        .if eax==NULL
            DbgWarning "Client.QueueConnect failed - releasing Client"
            push eax
            OCall [esi].OutClients::ClientPool.Free,pClient
            pop eax
        .else
            DbgWarning "Connect Queued"
        .endif
    .else
        DbgWarning "Error in NetEngine.ConnectTo - failed Client.Init"
    .endif
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
    OCall pClient::Client.Init, esi,[esi].io_completion_port,addr [esi].IOJobs,pProtocol,NULL,TRUE
    ;Register this socket with iocp, because we want socket event notifications
    mov edx,pClient
    invoke CreateIoCompletionPort, [edx].Client.hSocket, [esi].io_completion_port, edx, 0
    ;Enable the SO_BROADCAST option
	mov bOptVal,TRUE
	mov bOptLen ,sizeof BOOL
	mov edx,pClient
	invoke setsockopt,[edx].Client.hSocket, SOL_SOCKET, SO_BROADCAST, addr bOptVal, bOptLen 
	.if eax==SOCKET_ERROR
	    invoke WSAGetLastError
	    DbgDec eax,"Error - failed to set socket option SO_BROADCAST in NetEngine.UDP_Broadcast"
		xor eax,eax
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
	    .if eax==SOCKET_ERROR
	        invoke WSAGetLastError
	        DbgDec eax,"Error - failed to bind socket to local address in NetEngine.UDP_Broadcast"
            xor eax,eax
    	.else
    	    
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
                DbgText "Error - failed to queue a READ operation in NetEngine.UDP_Broadcast"
		        xor eax,eax
            .endif
        .endif
    .endif
    
    ;If we failed, release the client
    .if eax==0
        DbgWarning "Freeing failed outbound client"
        OCall [esi].OutClients::ClientPool.Free,pClient
        xor eax,eax
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
    
    ;Obtain the Client pointer from the completed Job (its pOwner)
    mov eax,pJob
    m2m pClient,[eax].IOJob.pOwner,edx
    .if edx==0
        DbgWarning "Error - NULL Client - trashing Job"
        SetObject ecx
        OCall [ecx].IOJobs::IOJobPool.Free,pJob
        ExitMethod
    .endif
    
    invoke WSAGetOverlappedResult,[edx].Client.hSocket,pXOVL,addr bytes,FALSE,addr flags
    .if eax==FALSE
        mov dError,$invoke (WSAGetLastError)
        .if eax==996
              ;This is not really an error..
              ;it means the operation is still pending...
              ;we can expect another Completion Notification soon,
              ;so dont do anything and certainly do not free this job
              ExitMethod
        .elseif eax==ERROR_NETNAME_DELETED
                ;The socket is toast - kill the Client
                DbgWarning "Terminating Client due to ERROR_NETNAME_DELETED"
                OCall pClient::Client.OnDisconnected,ERROR_NETNAME_DELETED
                .if eax==TRUE
                    OCall GoodbyeClient,pClient,pJob
                .endif
                ExitMethod
        .elseif eax==ERROR_OPERATION_ABORTED 
                ;The Client is toast - free the job
                SetObject ecx
                OCall [ecx].IOJobs::IOJobPool.Free,pJob
                ExitMethod
           ;The remaining errors will be passed to the client
           ;who may pass them to the protocol handler.
        .elseif eax==WSAETIMEDOUT 
           .if [esi].XOVL.operation==OPERATION_CONNECT                    
               DbgWarning "An outbound connection attempt has timed out."
           .else
               ;I'm not expecting to ever see this
               DbgWarning "An IO operation has timed out."
               int 3
           .endif
        .elseif eax==WSAEADDRNOTAVAIL
               DbgWarning "Remote Host Address not available"
        .else
              DbgWarning "A NETWORK IO Operation has FAILED"
              mov edx,pJob
              DbgDec [edx].IOJob.xovl.operation,"Type of operation"
              .if eax==WSAECONNRESET
                  DbgText "Error = 'WSAECONNRESET'"                  
              .else
                DbgDec eax,"ERROR CODE"                
              .endif
        .endif
    .endif
    
    ;regardless of whether the completed job succeeded or failed,
    ;we will marshal the notification to the protocol handler.
     
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
            DbgWarning "Server has Disconnected the Client"
            OCall pClient::Client.OnDisconnected,ERROR_USERQUIT
            .if eax==TRUE
                OCall esi.GoodbyeClient,pClient,pJob
            .else
                
            .endif
            ExitMethod
        .endif    
   
        OCall pClient::Client.OnReadCompleted,pJob, bytes ,dError
        .if eax==ERROR_BADPROTOCOL || eax==ERROR_USERQUIT
            DbgWarning "NetEngine is Terminating a Client due to ERROR_BADPROTOCOL or ERROR_USERQUIT"
            ;We won't bother calling Client.OnDisconnected because we don't wish to recover from this
            OCall esi.GoodbyeClient,pClient,pJob
            ExitMethod
        .elseif eax==FALSE ;The IOJob buffer is NOT EMPTY 
            DbgWarning "Requeuing incomplete READ"
            OCall pClient::Client.DoRead,pXOVL     ;Requeue this IOJob as a Read    
            ExitMethod
        .else   ;The IOJob buffer is EMPTY
                ;What to do with an empty iojob?
            mov ebx,pClient
            DbgDec [ebx].Client.PendingReadCounter
            .if [ebx].Client.PendingReadCounter==1
                DbgWarning "Requeuing READ since theres none pending"
                OCall pClient::Client.DoRead,pXOVL
            .else
                DbgWarning "Trashing redundant READ"
                OCall pClient::Client.FreeJob,pJob
            .endif
                
        .endif
        


    .case OPERATION_DOWRITE
        ;Marshal the notification to the Client
        DbgHex esi
        OCall pClient::Client.OnWriteCompleted,pJob,bytes,dError  
        .if eax==ERROR_USERQUIT
            DbgWarning "Terminating Client due to failed Write"
            OCall pClient::Client.OnDisconnected,ERROR_USERQUIT
            .if eax==TRUE
                OCall esi.GoodbyeClient,pClient,pJob
            .endif
        .endif  

    .case OPERATION_CONNECT        
        ;Pass this notification to the Client's Protocol handler
        DbgWarning "NetEngine calls Client.OnConnectCompleted"
        OCall pClient::Client.OnConnectCompleted,pJob,dError
        .if dError!=0
            DbgWarning "Terminating Client which failed Connect"
            OCall pClient::Client.OnDisconnected,dError
            .if eax==TRUE
                OCall GoodbyeClient,pClient,pJob
            .endif
        .endif
        
    .default
        ;Operations of unknown Type are passed to this method
        DbgWarning "User-Defined IO operation completed"
        OCall pClient::Client.OnUserIOCompleted,pJob,dError
    .endsw
    
MethodEnd


;This method trashes a Client and optionally, an IOJob.
;It contains some safety checking that may be removed at a later date.
;WE SHOULD REMOVE CLIENT FROM ALL CLIENTGROUPS
Method NetEngine.GoodbyeClient,uses esi,pClient,pJob
LOCAL pListener
    SetObject esi 
    mov eax,pClient
    .if [eax].Client.bSocketIsUDP==TRUE  
        DbgWarning "NetEngine.GoodbyeClient is Terminating UDP Client"
    .else
        DbgWarning "NetEngine.GoodbyeClient is Terminating TCP Client"
    .endif
    
    m2m pListener,[eax].Client.pListener,edx

    ;Notify the dead Client's Protocol handler
    ;in order to free any Protocol-specific client resources
    OCall pClient::Client.Goodbye
    
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

;When an Acceptor client is Queued, a timer in the client is set to zero.
;When an Accept completes, the timer is set to current tick count.
;When a Read completes, the timer is set to zero again.
;Every few seconds, this thread will check through all inbound / accepting clients
;looking for those whose timer is not zero...
;These clients have been Accepted, but have not yet sent any data.
;They have five seconds to do so, before they are booted. 
Method NetEngine.AntiZombie,uses esi
LOCAL time
    SetObject esi
    .repeat
        .break .if [esi].ShuttingDown==TRUE
        .if [esi].Listeners.Used.dCount>0
            invoke EnterCriticalSection,addr [esi]._CS
            mov time,$invoke (GetTickCount)
            xor ecx,ecx
            .while ecx < [esi].InClients.Used.dCount
                .break .if [esi].ShuttingDown==TRUE
                push ecx
                OCall [esi].InClients.Used::Collection.ItemAt,ecx
                mov edx,time
                
                ;Check for clients who are idle on accept
                .if [eax].Client.bClientPassedZombieTest==FALSE
                    .if [eax].Client.dTimeIdleSinceAccept!=0
                        sub edx,[eax].Client.dTimeIdleSinceAccept
                        .if edx>5000
                            DbgWarning "Terminating ZOMBIE Client"
                            OCall esi.GoodbyeClient,eax,NULL
                            pop ecx
                        .else
                            pop ecx ;not time yet - check next client
                            inc ecx
                        .endif
                    .else
                        pop ecx ;not accepted yet - check next client
                        inc ecx
                    .endif
                    
                ;Check for clients who are idle but passed zombie test
                .else
                    sub edx,[eax].Client.dTimeIdleSinceWrite
                    .if edx>10*60*1000  ;ten minutes
                        DbgWarning "Terminating IDLE Client"
                        OCall esi.GoodbyeClient,eax,NULL
                        pop ecx
                    .else
                        pop ecx ;not time yet - check next client
                        inc ecx
                    .endif
                

                .endif
                
            .endw
            invoke LeaveCriticalSection,addr [esi]._CS
            invoke Sleep,5000
        .endif
    .until 0  
    DbgWarning "'AntiZombie' Thread has Terminated"
MethodEnd

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
        .break .if [esi].ShuttingDown==TRUE               
        .if eax==TRUE
            .if pClient==NULL && pOVL==NULL
                DbgText "GetQueuedCompletionStatus went horribly wrong.","Worker"
                .break
            .endif
            ;Check for special 'time to die' message
            mov edx,pOVL
            .if [edx].XOVL.operation==OPERATION_ENDED
                DbgWarning "'Worker' Thread has Terminated"
                .break
            .endif
            ;Process the completed IOJob inside a CriticalSection
            ;This guarantees that no other 
            invoke EnterCriticalSection,addr [esi]._CS
            OCall esi.OnIOCompleted,pOVL,bytes
            invoke LeaveCriticalSection,addr [esi]._CS
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

include c:\masm32\objasm32\projects\nnet\superrandom.inc


endif
