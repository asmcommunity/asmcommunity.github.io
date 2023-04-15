;=====================================================
;This class describes a GENERIC SERVER.
;Code is provided to set up the Listen socket and
;its associated EventObject, but NOTE THIS WELL:
;NO CODE IS PROVIDED TO MONITOR OR HANDLE EVENTS HERE !!!
;That stuff is handled by GenericServer-->SpecificServer ;)
;=====================================================
GenericServerID equ 61875
;=====================================================
Object GenericServer,GenericServerID,Primer
StaticMethod   MakeServerSocket, Pointer,dword
DefineVariable pListenSocket,Pointer
DefineVariable hListenEvent, dword
ObjectEnd
;=====================================================

;=====================================================
;This method creates the Listening socket.
;Just tell it what Port to Listen on..
Method GenericServer.MakeServerSocket,uses esi,pOwner,dwPort
LOCAL pSocket
LOCAL hEvent

	SetObject esi
	mov pSocket, $New (Socket, Init, pOwner)
	OCall pSocket::Socket.Create,PF_INET,IPPROTO_TCP,SOCK_STREAM
	;Create an EventObject (to be triggered by Socket Events)
	;Associate the EventObject / SocketObject pair
	;and declare which Socket event we are interested in
	invoke WSACreateEvent
	mov hEvent,eax
	mov eax,pSocket
	invoke WSAEventSelect,[eax].Socket.hSocket,hEvent,FD_ACCEPT
	.if eax!=0
	  invoke WSAGetLastError
      DbgDec eax,"ERROR on WSAEventSelect"
    .else
     	SetObject esi
     	m2m [esi].pListenSocket, pSocket
     	m2m [esi].hListenEvent, hEvent
	.endif	
	DbgHex pSocket, "LISTEN Socket Starting.."	
	OCall pSocket::Socket.SetHost,INADDR_ANY,dwPort
	OCall pSocket::Socket.Bind
	OCall pSocket::Socket.Listen,dwPort	
	mov eax,TRUE
MethodEnd

;=====================================================

