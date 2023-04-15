SocketID equ 31337

Object Socket, SocketID,Primer
	;"primer" methods
    RedefineMethod Done
    StaticMethod StartUp
    StaticMethod ShutDown
    RedefineMethod Init,Pointer
    
    ;"utility" methods
    StaticMethod GetHandle
    StaticMethod ResolveHost, dword
    StaticMethod SetHost, dword, dword
        
    ;"standard" socket methods
    StaticMethod Create, dword, dword, dword
    StaticMethod Bind
    StaticMethod Connect
    StaticMethod Listen, dword
    StaticMethod Accept, Pointer
    StaticMethod AbortiveClose
    StaticMethod Close
    StaticMethod Send,Pointer,dword
    StaticMethod ReadAvailable
    StaticMethod Recv,Pointer,dword
    
    ;"WSA" socket methods
    StaticMethod CreateWSA, dword, dword, dword, dword
    StaticMethod ConnectWSA, dword, dword

    ;"Special" socket methods
    StaticMethod AcceptEx, dword, dword, dword, dword, dword
    
    DefineVariable hSocket, dword, INVALID_SOCKET
	DefineVariable hZombieTimer,dword,0    
    DefineVariable sin, sockaddr_in,{<>}
    DefineVariable State,dword,0

ObjectEnd Lib

;================================================
Method Socket.Done,uses esi
    SetObject esi
    DbgHex esi,"SOCKET BEING DESTROYED"
   .if [esi].hSocket != INVALID_SOCKET
      OCall AbortiveClose      
   .endif
   .if [esi].hZombieTimer!=0
	   invoke KillTimer, hWnd,[esi].hZombieTimer
   .endif
MethodEnd

Method Socket.StartUp
	DbgText "OA32 Startup - Socket"
MethodEnd

Method Socket.ShutDown 
	DbgText "OA32 ShutDown - Socket"
MethodEnd

Method Socket.Init, uses esi, pOwner
	DbgHex esi,"SOCKET NOW OWNED BY"
	DbgHex pOwner,"SOCKET CONTAINER"
	ACall Init, pOwner
MethodEnd

Method Socket.GetHandle, uses ecx
    SetObject ecx
    mov eax, [ecx].hSocket
MethodEnd

Method Socket.Connect,uses ecx
   SetObject ecx
   invoke connect, [ecx].hSocket, addr [ecx].sin,sizeof sockaddr_in
   .if eax==SOCKET_ERROR
      invoke WSAGetLastError
      .if eax==10035
      	  DbgText "Connect Pending..."
      .else
	      DbgDec eax,"ERROR on Connect"
	  .endif
   .endif
MethodEnd

Method Socket.Accept,uses esi,psin
   SetObject esi
   invoke accept, [esi].hSocket, psin,0
   .if eax==SOCKET_ERROR
      invoke WSAGetLastError
      DbgDec eax,"ERROR on Accept"
      xor eax,eax  	     	  
   .endif
MethodEnd

Method Socket.AcceptEx,uses ecx, pOtherSocket, pOutBuf, dwUsableBufSize, pRecvdLength, pOvl
   SetObject ecx
   invoke AcceptEx, [ecx].hSocket, pOtherSocket, pOutBuf, dwUsableBufSize, 64,64,pRecvdLength, pOvl
   .if eax==SOCKET_ERROR
      invoke WSAGetLastError
      DbgDec eax,"ERROR on Acceptex"
   .endif
MethodEnd

Method Socket.Listen,uses ecx, dwbacklog
   SetObject ecx
   invoke listen,[ecx].hSocket,dwbacklog
   .if eax==SOCKET_ERROR
      invoke WSAGetLastError
      DbgDec eax,"ERROR on Listen"
   .endif
MethodEnd

Method Socket.Close,uses ecx
   SetObject ecx
   invoke closesocket, [ecx].hSocket
   .if eax!=0   
      invoke WSAGetLastError
      DbgDec eax,"ERROR on Close"
   .endif
MethodEnd

linger struct
  l_onoff word ?
  l_linger word ?
linger ends

Method Socket.AbortiveClose,uses ecx
local mylinger:linger 
   SetObject ecx
   mov mylinger.l_onoff , 1
   mov mylinger.l_linger , 0
   invoke setsockopt,[ecx].hSocket,SOL_SOCKET, SO_LINGER, addr mylinger,sizeof linger
   .if eax==SOCKET_ERROR
      invoke WSAGetLastError
      DbgDec eax,"ERROR on SetSockOpt"
   .endif
   OCall Close
MethodEnd

Method Socket.ResolveHost,,pHostName
   invoke inet_addr,pHostName  ;assume its an ip string
   .if eax==INADDR_NONE
      invoke gethostbyname,pHostName;ok so its a domain string
      .if eax==NULL
        DbgDec eax,"ERROR on ResolveHost"
        xor eax,eax
      .else
        mov eax,[eax+12]
        mov eax,[eax]
        mov eax,[eax]
        push eax
        DbgText "Resolved Host IP Address"
    	pop eax
    
      .endif
   .endif   
MethodEnd

Method Socket.SetHost, uses ecx, dwIP, dwPort
   invoke htons, dwPort
   push eax
   SetObject ecx
   pop eax
   mov [ecx].sin.sin_port , ax 
   m2m [ecx].sin.sin_addr ,dwIP   
MethodEnd   

Method Socket.Bind, uses ecx
   SetObject ecx
   invoke bind, [ecx].hSocket, addr [ecx].sin, sizeof sockaddr_in
   .if eax==SOCKET_ERROR
      invoke WSAGetLastError
      DbgDec eax,"ERROR on Bind"
   .endif
MethodEnd   

Method Socket.Create, uses ecx, dwfamily:dword,dwproto:dword,dwtype:dword
    SetObject ecx
    push ecx
    invoke socket, dwfamily,dwtype,dwproto
    pop ecx
    .if eax==INVALID_SOCKET  
        invoke WSAGetLastError
        DbgDec eax,"ERROR on CreateSocket"
    .else
        mov [ecx].hSocket, eax
        m2m [ecx].sin.sin_family, dwfamily
    .endif
MethodEnd

Method Socket.CreateWSA, uses ecx, dwfamily,dwproto,dwtype,dwflags
    SetObject ecx
    push ecx
    invoke WSASocket, dwfamily, dwtype, dwproto,0,0,dwflags
    pop ecx
    mov [ecx].hSocket, eax
    m2m [ecx].sin.sin_family, dwfamily
    .if eax==INVALID_SOCKET          
        invoke WSAGetLastError
        DbgDec eax,"ERROR on CreateSocketWSA"
    .endif
MethodEnd

Method Socket.ConnectWSA, uses ecx, pDataOut, pDataIn
    SetObject ecx
    invoke WSAConnect, [ecx].hSocket, addr [ecx].sin, sizeof sockaddr_in, pDataOut, pDataIn, 0,0
   .if eax==SOCKET_ERROR
        invoke WSAGetLastError
        DbgDec eax,"ERROR on ConnectWSA"
   .endif
MethodEnd

Method Socket.Send,uses ecx,pDataOut,dwSize
	DbgStr pDataOut
	SetObject ecx
	invoke send,[ecx].hSocket,pDataOut,dwSize,0
MethodEnd

Method Socket.ReadAvailable,uses esi
LOCAL dwbytes
	SetObject esi
	invoke ioctlsocket,[esi].hSocket,FIONREAD,addr dwbytes
	mov eax,dwbytes	
MethodEnd
	
Method Socket.Recv,uses esi,pDataIn,bufSize
	SetObject esi	
	invoke recv,[esi].hSocket,pDataIn,bufSize,0
	.if eax==SOCKET_ERROR
		invoke WSAGetLastError
		DbgDec eax,"ERROR on Recv"
		mov edx,eax
		xor eax,eax
	.endif
MethodEnd