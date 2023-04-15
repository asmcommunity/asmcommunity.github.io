;These values define ServiceCodes used in lobby protocol packets
REQ_AUTH         equ 0  ;authenticate user/pass
REQ_LIST_SERVERS equ 1  ;requesting a list of GameServers
REQ_LIST_PLAYERS equ 2  ;query a GameServer for its list of Players
REQ_HOST_SERVER  equ 3  ;requesting to be advertised as a host server

;These values define event notifications to be handed between App and Protocol
AUTHENTICATION_SUCCESS                equ 100
AUTHENTICATION_FAILED_BAD_PASSWORD    equ 101
AUTHENTICATION_SUCCESS_NEW_USER       equ 102

.data
PROTOCOL_IDENTIFIER db "LOBY"
.code

;This structure describes the packet header of this protocol.
;There might be a payload following each header, but maybe not.
LobbyPacket struct
    dProtocolID  dd ?
    dServiceCode dd ?    ;what kind of request
    dPacketSize  dd ?    ;sizeof packet header + packet payload
LobbyPacket ends

LobbyProtocolID equ 236444
Object LobbyProtocol,LobbyProtocolID,NetworkProtocol
    RedefineMethod  FindDelimiter,              Pointer, Pointer ;pClient, pXOVL
	RedefineMethod	OnConnectCompleted,			Pointer, dword	 ;pClient
	RedefineMethod  FindDelimiter,              Pointer, Pointer  ;pClient, pXOVL
	RedefineMethod  ProcessReceivedData,        Pointer
	VirtualMethod   Connect,                    dword,dword,LPSTR,LPSTR,LPSTR,BOOL
	DefineVariable  bIsPlayer,BOOL,TRUE
ObjectEnd

;Returns NULL (failure - check user, pass and email are valid)
;or eax=pClient (connect is pending)
Method LobbyProtocol.Connect,uses esi,dHost,dPort,pUserName,pPass,pEmail,bIsPlayer:BOOL
    SetObject esi
    m2m [esi].bIsPlayer,bIsPlayer,edx
    
    ;UserName MUST be provided
    mov eax,pUserName
    .if eax==0 || byte ptr[eax]==0
        xor eax,eax
        ExitMethod
    .endif

    ;PassWord MUST be provided
    mov eax,pPass
    .if eax==0 || byte ptr[eax]==0
        xor eax,eax
        ExitMethod
    .endif
    
    ;Email is optional, but must at least LOOK like it might be valid
    mov eax,pEmail
    .if eax==0
        ExitMethod
    .elseif byte ptr[eax]!=0
        invoke lstrlen,pEmail
        .if eax<5
            xor eax,eax
            ExitMethod
        .endif
        invoke StrScan,pEmail,'@'
        .if eax==0
            ExitMethod
        .else
            invoke StrScan,eax,'.'
            .if eax==0
                ExitMethod
            .endif
        .endif
    .endif
    
    OCall [esi].pOwner::NetEngine.Connect,esi,dHost,dPort
    .if eax!=0
        push eax
        OCall eax::Client.SetUserID,pUserName,pPass,pEmail
        pop eax
    .endif
MethodEnd

;An OutBound Client has just connected to a LobbyServer
;Under the rules of my Protocol, the Server remains silent
;and waits for the Client to announce itself ...
;Let's build a packet for the Client to say hi to the Server :)
Method LobbyProtocol.OnConnectCompleted,uses esi edi ,pClient,dError
LOCAL temp[512]:byte
local namelen,passlen,emlen

    .if dError==0
    	OCall pClient::Client.QueueRead		;queue a Recv
    	DbgText "Client sends Authentication to Server"
    	
    	mov edi,pClient 
    	
    	movzx eax,[edi].Client.UserID.namelength
    	inc eax
    	mov namelen,eax
    	DbgDec namelen
    	
    	movzx eax,[edi].Client.UserID.passlength
    	inc eax
    	mov passlen,eax
    	DbgDec passlen 
    	
   	    movzx eax,[edi].Client.UserID.emaillength
    	inc eax
    	mov emlen,eax
    	DbgDec emlen 

    	lea edx,temp[sizeof LobbyPacket]
    	invoke RtlMoveMemory,edx,addr [edi].Client.UserID.namestring,namelen
    	lea edx,temp[sizeof LobbyPacket]
    	add edx,namelen
    	invoke RtlMoveMemory,edx,addr [edi].Client.UserID.password,passlen
    	lea edx,temp[sizeof LobbyPacket]
    	add edx,namelen
    	add edx,passlen
    	invoke RtlMoveMemory,edx,addr [edi].Client.UserID.email,emlen
    	
    	DbgStr [edi].Client.UserID.namestring
    	DbgStr [edi].Client.UserID.password
    	
    	lea edx,temp
    	mov eax,dword ptr[PROTOCOL_IDENTIFIER]
    	mov [edx].LobbyPacket.dProtocolID,eax
    	mov [edx].LobbyPacket.dServiceCode,REQ_AUTH
    	mov eax,namelen
    	add eax,passlen
    	add eax,emlen
    	add eax,sizeof LobbyPacket
    	mov [edx].LobbyPacket.dPacketSize,eax
    	
    	DbgDec [edx].LobbyPacket.dPacketSize
    	OCall pClient::Client.QueueWrite,addr temp,eax
    .else
        DbgDec eax,"Error - Connect attempt failed"
    .endif
    
    ;If we redefine an 'OnXXX' method such as OnConnectCompleted,
    ;and we want the Application to receive notifications,
    ;we should always have an ACall to the method of the same name.
    ACall OnConnectCompleted,pClient,dError
MethodEnd

;ERROR_BADPROTOCOL - The buffer content is not following our Protocol - the Client will get Booted for this offence
;ERROR_USERQUIT	   - The user sent a nice QUIT message using our Protocol - the Client will get Booted for this 
;Positive INT32	   - This many Bytes are part of the Complete Packet
;NULL			   - We need more data
Method LobbyProtocol.FindDelimiter,uses esi edi,pClient,pXOVL
LOCAL packetsize
	mov edi,pXOVL
	
	;Do we have at least 4 bytes?
	.if [edi].XOVL.bytesused<4
	    return NULL
	.endif
	
	;Do the first 4 bytes match our protocol identifier?
	mov eax,[edi].XOVL.pbuforig
	.if dword ptr[eax]!="YBOL"
	    return ERROR_BADPROTOCOL
	.endif
	
	;Do we have at least a full packet header?
	.if [edi].XOVL.bytesused<sizeof LobbyPacket
	    return NULL
	.endif
	
	;Do we have the complete payload?
	mov edx,[eax].LobbyPacket.dPacketSize
	.if [edi].XOVL.bytesused<edx
	    ;We don't yet have the complete payload
	    DbgDec edx,"Expected packetsize"
	    DbgDec [edi].XOVL.bytesused,"Received packetsize"
	    xor eax,eax
	.else
	    ;We have a complete packet
	    DbgText "We have a complete packet"
        mov eax,[edi].XOVL.bytesused	    
	.endif
MethodEnd

;ERROR_BADPROTOCOL - The buffer content isnt following our Protocol - the Client will get Booted for this offence
;ERROR_USERQUIT	   - The user sent a nice QUIT message using our Protocol - the Client will get Booted for this 
;Positive INT32	   - This many Bytes were cleanly processed and should be cut from the IOJob's buffer
Method LobbyProtocol.ProcessReceivedData,uses esi edi,pClientJobs
LOCAL bytes
LOCAL p1,p2
LOCAL buf[256]:BYTE
    mov bytes,0
    SetObject esi
	mov edi,$OCall (pClientJobs::DwordCollection.ItemAt,0)
	mov eax,[edi].IOJob.xovl.pbuforig 
	
    .switch [eax].LobbyPacket.dServiceCode
    .case REQ_AUTH
        mov eax,[edi].IOJob.xovl.pbuforig 
        mov [eax].LobbyPacket.dPacketSize,sizeof LobbyPacket+1
        add eax,sizeof LobbyPacket
        .if byte ptr[eax]==0
            DbgWarning "AUTHENTICATION SUCCESS - ACCOUNT WAS CREATED"
            OCall pClientJobs::Client.Notify,AUTHENTICATION_SUCCESS_NEW_USER
            ;remain connected
            mov eax,[edi].IOJob.xovl.bytesused
        .elseif byte ptr[eax]==1
            DbgWarning "AUTHENTICATION FAILED - PASSWORD INCORRECT"
            OCall pClientJobs::Client.Notify,AUTHENTICATION_FAILED_BAD_PASSWORD
            ;disconnect
            mov eax,ERROR_USERQUIT
        .elseif byte ptr[eax]==2
            DbgWarning "AUTHENTICATION SUCCEEDED"
            OCall pClientJobs::Client.Notify,AUTHENTICATION_SUCCESS
            ;remain connected
            mov eax,[edi].IOJob.xovl.bytesused
        .else

        .endif
    .case REQ_LIST_SERVERS
        DbgWarning "Client receives Hosts List"
        invoke SendDlgItemMessage,hWnd,1008,CB_RESETCONTENT,0,0
        mov ebx,[edi].IOJob.xovl.pbuforig 
        DbgMem ebx,[edi].IOJob.xovl.bytesused
        add ebx,sizeof LobbyPacket
        .repeat
            invoke lstrlen,ebx
            .break .if eax==0
            mov p1,ebx
            DbgStr p1

            inc eax
            add ebx,eax            
            invoke lstrlen,ebx
            .break .if eax==0
            mov p2,ebx
            DbgStr p2
            
            inc eax
            add ebx,eax
            
            invoke wsprintf,addr buf,$OfsCStr("%s (%s)"),p2,p1
            invoke SendDlgItemMessage,hWnd,1008,CB_ADDSTRING,0,addr buf
        .until 0
        mov eax,[edi].IOJob.xovl.pbuforig
        mov eax,[eax].LobbyPacket.dPacketSize
            
            
            
            
    .case REQ_LIST_PLAYERS
        DbgWarning "UNHANDLED - REQ_LIST_PLAYERS"
    .default
        DbgWarning "Unexpected ServiceCode received"
        DbgDec [edi].LobbyPacket.dServiceCode
        mov eax,ERROR_BADPROTOCOL
    .endsw
MethodEnd

