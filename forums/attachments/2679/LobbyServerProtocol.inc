

DBRecord struct
    UserName db 64 dup (?)
    PassWord db 64 dup (?)
    Email    db 64 dup (?)
DBRecord ends

LoadObjects DiskStream

;This structure describes the packet header of this protocol.
;There might be a payload following each header, but maybe not.
LobbyPacket struct
    dProtocolID  dd ?
    dServiceCode dd ?    ;what kind of request
    dPacketSize  dd ?    ;sizeof packet header + packet payload
LobbyPacket ends

REQ_AUTH         equ 0  ;Someone wishes to authenticate 
REQ_LIST_SERVERS equ 1  ;requesting a list of GameServers
REQ_LIST_PLAYERS equ 2  ;query a GameServer for its list of Players
REQ_HOST_SERVER  equ 3  ;requesting to be advertised as a host server

LobbyServerProtocolID equ 25775
Object LobbyServerProtocol,LobbyServerProtocolID,NetworkProtocol
  RedefineMethod    Done
  RedefineMethod    Init,               Pointer, Pointer;pOwner, pAppEventSink
  RedefineMethod    FindDelimiter,      Pointer, Pointer;pClient, pXOVL
  RedefineMethod    ProcessReceivedData,Pointer         ;pClient
  RedefineMethod    OnConnectCompleted, Pointer, dword  ;pClient, dError
  RedefineMethod    OnAcceptCompleted,  Pointer, dword  ;pClient, dError
  VirtualMethod     Authenticate,       LPSTR,LPSTR,LPSTR
  Embed             Database,           DiskStream
  
ObjectEnd

if IMPLEMENT
; ——————————————————————————————————————————————————————————————————————————————————————————————————

Method LobbyServerProtocol.Init,uses esi, pOwner, pAppEventSink
    SetObject esi
    ACall esi.Init, pOwner, pAppEventSink
    ;First thing we'll do is add a ClientGroup to track GameServers
    mov edx,pOwner
    OCall [edx].NetEngine.RootClientGroup::ClientGroup.AddSubGroup,$OfsCStr("GameServers"),$OfsCStr("Servers which are hosting games")
    ;Now we'll fire up our database
    OCall [esi].Database::DiskStream.Init,esi,$OfsCStr("Database.data")
    
MethodEnd

Method LobbyServerProtocol.Done,uses esi
    SetObject esi
    OCall [esi].Database::DiskStream.Done
    ACall Done
MethodEnd

;Scan the Database for user/pass
;Possible return values are:
;0 = username was just registered
;1 = username exists but password is wrong
;2 = username exists and password is correct
Method LobbyServerProtocol.Authenticate,uses esi,pUserName,pPassword,pEmail
LOCAL sighs
LOCAL dbr:DBRecord
LOCAL result
    SetObject esi
    
    DbgWarning "Authenticating"
    
    mov sighs,$OCall ([esi].Database::DiskStream.GetSize)
    mov result,0
    .if sighs!=0
        xor ebx,ebx
        invoke SetFilePointer,[esi].Database.hFile,0,0,FILE_BEGIN
        .while ebx < sighs
            OCall [esi].Database::DiskStream.BinRead,addr dbr,sizeof dbr
            invoke lstrcmpi,pUserName,addr dbr.UserName
            .if eax==0
                inc result
                invoke lstrcmp,pPassword,addr dbr.PassWord
                .if eax==0
                    inc result                    
                .endif
                .break
            .endif
            add ebx,sizeof DBRecord
        .endw
    .endif
    
    .if result==0
        ;The username does not exist - register it now
        ;(we're already at the end of the database file,
        ;so there's no need to set the filepointer)
        invoke RtlZeroMemory,addr dbr,sizeof dbr
        invoke lstrcpy,addr dbr.UserName,pUserName
        invoke lstrcpy,addr dbr.PassWord,pPassword
        invoke lstrcpy,addr dbr.Email,pEmail
        OCall [esi].Database::DiskStream.BinWrite,addr dbr,sizeof dbr
    .endif
    
    mov eax, result
MethodEnd

;ERROR_BADPROTOCOL - The buffer content is not following our Protocol - the Client will get Booted for this offence
;ERROR_USERQUIT	   - The user sent a nice QUIT message using our Protocol - the Client will get Booted for this 
;Positive INT32	   - This many Bytes are part of the Complete Packet
;NULL			   - We need more data
Method LobbyServerProtocol.FindDelimiter,uses esi edi,pClient,pXOVL
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
Method LobbyServerProtocol.ProcessReceivedData,uses esi edi,pClientJobs
LOCAL bytes
LOCAL namelen,passlen,emaillen
LOCAL pname,ppass,pemail
LOCAL cnt,len
    mov bytes,0
    SetObject esi
	mov edi,$OCall (pClientJobs::DwordCollection.ItemAt,0)
	mov eax,[edi].IOJob.xovl.pbuforig 
	
    .switch [eax].LobbyPacket.dServiceCode
    .case REQ_AUTH
        DbgText "LobbyServerProtocol: Client authenticating as Player"

        DbgMem [edi].IOJob.xovl.pbuforig ,[edi].IOJob.xovl.bytesused
        mov eax,[edi].IOJob.xovl.pbuforig 
        mov [eax].LobbyPacket.dPacketSize,sizeof LobbyPacket+1
        add eax,sizeof LobbyPacket
        
        ;Grab the user, pass and email strings from the incoming packet
        mov pname,eax
        invoke lstrlen,eax
        inc eax
        mov namelen,eax        
        add eax,pname
        mov ppass,eax
        invoke lstrlen,eax
        inc eax
        mov passlen,eax
        add eax,ppass
        mov pemail,eax
        invoke lstrlen,eax
        inc eax
        mov emaillen,eax
        
        ;We'll set the serverside client id
        OCall pClientJobs::Client.SetUserID,pname,ppass,pemail
        
        ;Perform authentication
        OCall esi.Authenticate,pname,ppass,pemail
        ;Write the result of auth-check as a single byte
        mov edx,pname
        mov byte ptr[edx],al
        push eax
        ;Inform the remote client of the result
        OCall pClientJobs::Client.QueueWrite,[edi].IOJob.xovl.pbuforig,sizeof LobbyPacket+1
        pop eax
        .if eax==1 ;password was wrong - termiante session
            mov eax,ERROR_USERQUIT
        .else      ;password correct, or account is new - leave session open
            mov eax,[edi].IOJob.xovl.bytesused
        .endif
        
        
        
    .case REQ_LIST_SERVERS
        DbgText "LobbyServerProtocol: Client requesting Server List"
        mov edx,[esi].pOwner
        OCall [edx].NetEngine.RootClientGroup::ClientGroup.FindGroupByName,$OfsCStr("GameServers")
        push edi

        .if eax!=0
            ;We will build a packet whose payload contains
            ;a list of the members of the HOSTS group (as strings)
            ;username,0
            ;ipstring,0

            m2m cnt,[eax].ClientGroup.dCount,edx
            DbgDec cnt,"#Clients in HOSTS group"
            mov edi,eax
            
            mov ebx,$MemAlloc(32768,MEM_INIT_ZERO)
            push ebx
            add ebx,sizeof LobbyPacket

            
            xor ecx,ecx
            mov len,ecx
            .while ecx<cnt
                push ecx
                
                OCall edi::ClientGroup.ItemAt,ecx 
                push eax
                invoke lstrcpy,ebx,addr [eax].Client.UserID.namestring
                pop eax
                DbgStr [eax].Client.UserID.namestring
                
                movzx edx,[eax].Client.UserID.namelength
                inc edx
                add ebx,edx
                add len,edx
                
                mov eax,[eax].Client.Sin_RemoteHost.sin_addr.S_un
                invoke inet_ntoa, eax
                invoke lstrcpy,ebx,eax
                invoke lstrlen,ebx
                inc eax
                add ebx,eax
                add len,eax
                
                pop ecx
                inc ecx
            .endw
            
            pop ebx
            mov [ebx].LobbyPacket.dProtocolID,"YBOL"
            mov [ebx].LobbyPacket.dServiceCode,REQ_LIST_SERVERS
            mov eax,len
            add eax,sizeof LobbyPacket
            mov [ebx].LobbyPacket.dPacketSize,eax
            OCall pClientJobs::Client.QueueWrite,ebx,eax
            MemFree ebx
        
        .endif    
         pop edi
         mov eax,[edi].IOJob.xovl.bytesused

    .case REQ_LIST_PLAYERS
        DbgText "LobbyServerProtocol: Client requesting Player List"
        mov eax,[edi].IOJob.xovl.bytesused

    .case REQ_HOST_SERVER
        DbgText "LobbyServerProtocol: Client wants to advertise as a host"
        mov edx,[esi].pOwner
        OCall [edx].NetEngine.RootClientGroup::ClientGroup.FindGroupByName,$OfsCStr("GameServers")
        .if eax!=0
            ;Ensure the client is not already a member of the group
            push eax
            OCall eax::ClientGroup.FindClient,pClientJobs
            pop edx
            .if eax==0
                ;Add the client to the group
                OCall edx::ClientGroup.Insert,pClientJobs
                DbgWarning "Client has joined the HOSTS group"
            .endif
        .endif
        
        mov eax,[edi].IOJob.xovl.bytesused
    .default
        DbgWarning "Unexpected ServiceCode received"
        DbgDec [edi].LobbyPacket.dServiceCode
        mov eax,ERROR_BADPROTOCOL
    .endsw
	


MethodEnd

Method LobbyServerProtocol.OnConnectCompleted,uses esi,pClient,dError
    .if dError == 0
        DbgText "LobbyServerProtocol says an outbound Client connection succeeded."
    .else
        DbgDec dError, "LobbyServerProtocol says an outbound Client connection failed."
    .endif
MethodEnd

Method LobbyServerProtocol.OnAcceptCompleted,uses esi,pClient,dError
    .if dError == 0
        DbgText "LobbyServerProtocol says an Accept succeeded."
    .else
        DbgDec dError, "LobbyServerProtocol says an Accept failed."
    .endif
MethodEnd




endif

