;NetworkProtocol is a Base Class from which you can derive
;your own protocol and application specific classes.
;You can imagine that this class sits inbetween the main Server class
;and your application - it is an 'event sink' - its methods are called 
;directly by the Server class.
;Its main job is to enforce the rules of a network protocol,
;but it can also be used to alert your application about some
;important events, for example the completion of a Connect attempt.
;Your application should create one of these objects for each
;network protocol that you wish to support, and hand such objects
;to the Server whenever you call Server.ConnectTo or Server.Listen...
;The Server class will thereafter marshal completed events
;to the NetworkProtocol derived class you provided on a per Client basis.
;This implies that each network session can use a different protocol ;)

NetworkProtocolID equ 23543
Object NetworkProtocol,NetworkProtocolID,Primer
  VirtualMethod		OnReadCompleted,			Pointer,Pointer,dword,dword	 ;pClient, pXOVL, dwBytes, dErrorCode
  DynamicMethod		OnConnectCompleted,			Pointer,dword	     ;-> Client, dErrorCode
  DynamicMethod     OnAcceptCompleted,          Pointer,dword	     ;-> Client, dErrorCode
  DynamicMethod		OnClientQuit,				Pointer			     ;-> Client
  DynamicMethod		FindDelimiter,	            Pointer,Pointer 		 ;pClient,pxovl
  DynamicMethod		ProcessReceivedData,		Pointer     			 ;pClient
ObjectEnd

if IMPLEMENT
; ——————————————————————————————————————————————————————————————————————————————————————————————————

;Client class will call this method to determine if we have received
;at least one complete packet, given the rules of your Protocol.
;This is where your derived class examines the received data
;looking to find a valid Delimiter and doing Protocol enforcement.
;When you override this method in a derived class, you should return
;one of the following possible values:
;ERROR_BADPROTOCOL - The buffer content is not following our Protocol - the Client will get Booted for this offence
;ERROR_USERQUIT	   - The user sent a nice QUIT message using our Protocol - the Client will get Booted for this 
;Positive INT32	   - This many Bytes are part of the Complete Packet
;NULL			   - We need more data
Method NetworkProtocol.FindDelimiter,uses esi,pClient,pXOVL
	;In this 'vanilla' protocol handler,
	;we assume EVERYTHING is a delimited chunk of data
	mov eax,pXOVL
	mov eax,[eax].XOVL.bytes
	DbgDec eax,"bytes in partial rx buffer"
MethodEnd

;A client has finished sending us data,
;now we have to do something with that data.
;Make sure your derived method returns one of the following values:
;ERROR_BADPROTOCOL - The buffer content isnt following our Protocol - the Client will get Booted for this offence
;ERROR_USERQUIT	   - The user sent a nice QUIT message using our Protocol - the Client will get Booted for this 
;Positive INT32	   - This many Bytes were cleanly processed and should be cut from the IOJob's buffer
Method NetworkProtocol.ProcessReceivedData,uses esi edi,pClientJobs
LOCAL bytes
    mov bytes,0
	;In this 'vanilla' protocol handler, we'll accept ANYTHING as being legitimate data
    mov edi,pClientJobs
	xor ebx,ebx
	.while ebx<[edi].DwordCollection.dCount
	    OCall edi::DwordCollection.ItemAt,ebx
     	DbgMem [eax].IOJob.xovl.pbuforig,[eax].IOJob.xovl.bytes
    	DbgHex [eax].IOJob.xovl.pbuforig
    	DbgHex [eax].IOJob.xovl.wsabuf.buf
    	mov eax,[eax].IOJob.xovl.bytes
    	add bytes,eax
        inc ebx
    .endw
    mov eax,bytes
MethodEnd

;A client session has, for whatever reason, been terminated.
;You can override this, perhaps to release extra perclient resources
Method NetworkProtocol.OnClientQuit,uses esi,pClient
	DbgWarning "NetworkProtocol: Elvis has left the building"
	mov eax,TRUE
MethodEnd

;An outbound connection attempt has completed.
;When you override this method in a derived class,
;you should check the value of dError parameter
;to determine whether the connection succeeded,
;and possibly queue the first Send operation.
;dError will contain NULL for success,  
;or it will contain a socket error code.
Method NetworkProtocol.OnConnectCompleted, ,pClient,dError
	DbgWarning "NetworkProtocol: Outbound Connection has completed"
MethodEnd

;An inbound connection 'accept' has completed.
;When you override this method in a derived class,
;you should check the value of dError parameter
;to determine whether the connection succeeded,
;and possibly queue the first Send operation.
;dError will contain NULL for success,  
;or it will contain a socket error code.
Method NetworkProtocol.OnAcceptCompleted, ,pClient,dError
	DbgWarning "NetworkProtocol: Inbound Connection has completed"
MethodEnd

;A recv job has completed - it may be a PARTIAL receive
;so this method is really just a courtesy notification
Method NetworkProtocol.OnReadCompleted,uses esi ebx, pClient,pXOVL, bytes, dError
	DbgWarning "NetworkProtocol: Data was received"

MethodEnd
endif

