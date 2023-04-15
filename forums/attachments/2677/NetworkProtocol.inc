
;NetEventSink decribes the 'event sinking' methods of NetworkProtocol,
;it's been defined separately in case you wish to implement
;an eventsink in the APPLICATION, that your derived protocol handler
;can forward event notifications to.. which can be a handy thing.
NAES equ 346766
Object NetEventSink,NAES,Primer
  DynamicAbstract	OnReadCompleted,			Pointer,Pointer,dword,dword	 ;pClient, pXOVL, dwBytes, dErrorCode
  DynamicAbstract   OnWriteCompleted,           Pointer,Pointer,dword        ;pClient, pXOVL, dwBytes
  DynamicAbstract	OnConnectCompleted,			Pointer,dword	            ;-> Client, dErrorCode
  DynamicAbstract   OnDisconnected,             Pointer,dword                ;-> Client, dErrorCode
  DynamicAbstract   OnAcceptCompleted,          Pointer,dword	            ;-> Client, dErrorCode
  DynamicAbstract   OnUserIOCompleted,          Pointer,Pointer,dword        ;pClient, pJob, dError
  DynamicAbstract	OnClientQuit,				Pointer			            ;-> Client
ObjectEnd

;NetworkProtocol is a Base Class from which you can derive
;your own protocol and application specific classes.
;You can imagine that this class sits inbetween the main Server class
;and your application - it is an 'event sink' - its methods are called 
;directly by the Server class.
;Its main job is to enforce the rules of a network protocol,
;but it can also be used to alert your application about some
;important events, for example the completion of a Connect attempt.
;You should create one of these objects for each network protocol 
;that you wish to support, and hand such objects to the Server whenever 
;you call Server.ConnectTo or Server.Listen...
;The Server class will thereafter marshal completed events
;to the NetworkProtocol derived class you provided on a per Client basis.
;This implies that each network session can use a different protocol ;)

;Furthermore, the 'On XXX' events can be marshalled to the APPLICATION,
;by setting the lpAppEventSink to point to a NetEventSink object in the APP.
;If you override ANY of these methods, make sure it contains an ACALL
;so that the Application Event Sink (lpAppEventSink::NetEventSink)
;will automatically receive event notifications.

NetworkProtocolID equ 23543
Object NetworkProtocol,NetworkProtocolID,NetEventSink
  RedefineMethod    Init,                       Pointer, Pointer

  DynamicMethod		FindDelimiter,	            Pointer,Pointer 		 ;pClient,pxovl
  DynamicMethod		ProcessReceivedData,		Pointer     			 ;pClient


  RedefineMethod    OnReadCompleted,			Pointer,Pointer,dword,dword	 ;pClient, pXOVL, dwBytes, dErrorCode
  RedefineMethod    OnWriteCompleted,           Pointer,Pointer,dword        ;pClient, pXOVL, dwBytes
  RedefineMethod	OnConnectCompleted,			Pointer,dword	     ;-> Client, dErrorCode
  RedefineMethod    OnDisconnected,             Pointer,dword        ;-> Client, dErrorCode
  RedefineMethod    OnAcceptCompleted,          Pointer,dword	     ;-> Client, dErrorCode
  RedefineMethod    OnUserIOCompleted,          Pointer,Pointer,dword   ;pClient, pJob, dError
  RedefineMethod	OnClientQuit,				Pointer			     ;-> Client

  DefineVariable    lpAppEventSink,             Pointer, NULL
ObjectEnd

if IMPLEMENT
; ——————————————————————————————————————————————————————————————————————————————————————————————————

;Constructor Method
;pOwner is usually a ptr to NetEngine object.
;lpAppEventSink, if not NULL, is a ptr to Application's NetEventSink derivation.
Method NetworkProtocol.Init,uses esi,pOwner,lpAppEventSink
    SetObject esi
    ACall esi.Init,pOwner
    m2m [esi].lpAppEventSink,lpAppEventSink
MethodEnd

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
	SetObject esi
	.if [esi].lpAppEventSink!=0
	    OCall [esi].lpAppEventSink::NetEventSink.OnClientQuit,pClient
	.endif
	mov eax,TRUE
MethodEnd

;An outbound connection attempt has completed.
;When you override this method in a derived class,
;you should check the value of dError parameter
;to determine whether the connection succeeded,
;and possibly queue the first Send operation.
;dError will contain NULL for success,  
;or it will contain a socket error code.
Method NetworkProtocol.OnConnectCompleted,uses esi ,pClient,dError
	DbgWarning "NetworkProtocol: Outbound Connection has completed"
	SetObject esi
	.if [esi].lpAppEventSink!=0
	    OCall [esi].lpAppEventSink::NetEventSink.OnConnectCompleted,pClient,dError
	.endif
MethodEnd

;Returns:
;TRUE  - Let this Client die
;FALSE - Attempt to reconnect
Method NetworkProtocol.OnDisconnected,uses esi  ,pClient,dError
	DbgWarning "NetworkProtocol: Connection has been lost"
	SetObject esi
	.if [esi].lpAppEventSink!=0
	    OCall [esi].lpAppEventSink::NetEventSink.OnDisconnected,pClient,dError
	.else
	    mov eax,TRUE
	.endif
MethodEnd

;An inbound connection 'accept' has completed.
;When you override this method in a derived class,
;you should check the value of dError parameter
;to determine whether the connection succeeded,
;and possibly queue the first Send operation.
;dError will contain NULL for success,  
;or it will contain a socket error code.
Method NetworkProtocol.OnAcceptCompleted,uses esi ,pClient,dError
	DbgWarning "NetworkProtocol: Inbound Connection has completed"
	SetObject esi
	.if [esi].lpAppEventSink!=0
	    OCall [esi].lpAppEventSink::NetEventSink.OnAcceptCompleted,pClient,dError
	.endif
MethodEnd

;A recv job has completed - it may be a PARTIAL receive
;so this method is really just a courtesy notification
Method NetworkProtocol.OnReadCompleted,uses esi, pClient,pXOVL, bytes, dError
	DbgWarning "NetworkProtocol: Data was received"
	SetObject esi
	.if [esi].lpAppEventSink!=0
	    OCall [esi].lpAppEventSink::NetEventSink.OnReadCompleted,pClient,pXOVL,bytes,dError
	.endif
MethodEnd

;A send job has completed
;This method is really just a courtesy notification
Method NetworkProtocol.OnWriteCompleted,uses esi, pClient,pXOVL, bytes
	DbgWarning "NetworkProtocol: Data was sent"
	SetObject esi
	.if [esi].lpAppEventSink!=0
	    OCall [esi].lpAppEventSink::NetEventSink.OnWriteCompleted,pClient,pXOVL,bytes
	.endif
MethodEnd

Method NetworkProtocol.OnUserIOCompleted,uses esi, pClient, pJob, dError
    DbgWarning "NetworkProtocol: User-defined IO completed"
	SetObject esi
	.if [esi].lpAppEventSink!=0
	    OCall [esi].lpAppEventSink::NetEventSink.OnUserIOCompleted,pClient,pJob,dError
	.endif
MethodEnd
endif

