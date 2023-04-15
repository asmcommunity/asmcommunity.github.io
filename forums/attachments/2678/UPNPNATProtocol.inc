
;Please note that the Object Definition for this class
;has been inserted into the NetEngine file,
;making these two classes 'friends'.
;NetEngine knows what UPNPNAtProtocol's interface looks like.
;UPNPNAtProtocol knows what NetEngine's interface looks like.
;This allows each class to call methods of the other class.
;This is a special situation, since UPNPNAtProtocol
;is embedded as a COMPONENT of NetEngine.

;Further Notes regarding this class:
;Usually, we call Init to set the pOwner field to be some window handle.
;However in this case, pOwner of this object will be the NetEngine object.
;This allows the protocol handler to call NetEngine methods directly.

;Notes regarding UPNP nat traversal 'stages':
;Firstly, we send a special udp broadcast packet, while listening for a reply.
;The reply, if any, contains the tcp ip and port of the upnp controller.

;0 .. connect completed .. write queued .. +1
;1 .. write completed .. read queued .. +1
;2 .. read completed .. ok!!


.data
;UDP broadcast request for upnp discovery
httpreqtemplate db "GET %s HTTP/1.1",13,10,
                   "Accept: text/xml, application/xml",13,10,
                   "User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows NT/5.1)",13,10,
                   "Host: %s:%lu",13,10,"Connection: Keep-Alive",13,10,"Cache-Control: no-cache",13,10,
                   "Pragma: no-cache",13,10,13,10,0	                   

;strings we need to find in the reply of upnp discovery 
szfirst  db "<deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType>",0
szsecond db "<serviceType>urn:schemas-upnp-org:service:WANIPConnection:1</serviceType>",0

;;header of request for the external ip address
;szreqextip db "POST %s HTTP/1.1",13,10,
;              "HOST: %s:%lu",13,10,
;              "CONTENT-LENGTH: %lu",13,10,
;              'Connection: Keep-Alive',13,10,
;              'CONTENT-TYPE: text/xml; charset="utf-8"',13,10,
;              'SOAPACTION: "urn:schemas-upnp-org:service:WANIPConnection:1#GetExternalIPAddress"',13,10,13,10,0
;
;;body of request for the external ip address
;szreqextipsoap db "<s:Envelope",13,10,'    xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"',13,10,
;              '    s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">',13,10,"  <s:Body>",13,10,'    <u:GetExternalIPAddress xmlns:u="urn:schemas-upnp-org:service:WANIPConnection:1">',13,10,
;              '    </u:GetExternalIPAddress>',13,10,'  </s:Body>',13,10,'</s:Envelope>',13,10,13,10,0

;string we need to find in the reply of request for the external ip address
;szextip db '<NewExternalIPAddress xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:',0

;header of request to add a port mapping
szAPMHead db "POST %s HTTP/1.1",13,10,
             'Content-Type: text/xml; charset="utf-8"',13,10,
             'SOAPAction: "urn:schemas-upnp-org:service:WANIPConnection:1#AddPortMapping"',13,10,
             'User-Agent: Mozilla/4.0 (compatible; UPnP/1.0; Windows 9x)',13,10,
             'Host: %s:%lu',13,10,
             'Content-Length: %lu',13,10,
             'Connection: Keep-Alive',13,10,
             'Cache-Control: no-cache',13,10,
             'Pragma: no-cache',13,10,13,10,0

;body of request to add a port mapping
szAPMBodyA db '<?xml version="1.0"?>',13,10,'<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">',
              '<SOAP-ENV:Body><m:AddPortMapping xmlns:m="urn:schemas-upnp-org:service:WANIPConnection:1">',
              '<NewRemoteHost xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:dt="string"></NewRemoteHost>',0
             
szAPMBodyB db '<NewExternalPort xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:dt="ui2">%lu</NewExternalPort>',
              '<NewProtocol xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:dt="string">%s</NewProtocol>',
              '<NewInternalPort xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:dt="ui2">%lu</NewInternalPort>',
              '<NewInternalClient xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:dt="string">%s</NewInternalClient>',0
              
szAPMBodyC db '<NewEnabled xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:dt="boolean">1</NewEnabled>',
              '<NewPortMappingDescription xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:dt="string">%s</NewPortMappingDescription>',
              '<NewLeaseDuration xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:dt="ui4">0</NewLeaseDuration>','</m:AddPortMapping></SOAP-ENV:Body></SOAP-ENV:Envelope>',13,10,0



.code

;----------------------------------------
; Convert decimal string into dword value
; return value in eax
;----------------------------------------
a2dw proc uses ecx edi edx esi pString:DWORD 
      mov edi, pString
      invoke lstrlen, pString

     xor ecx, ecx
      .while eax != 0
        xor edx, edx
        mov dl, byte ptr [edi]
        sub dl, "0" ; subtrack each digit with "0" to convert it to hex value
        mov esi, eax
        dec esi
        push eax
        mov eax, edx
        push ebx
        mov ebx, 10
          .while esi > 0
            mul ebx
            dec esi
          .endw
        pop ebx
        add ecx, eax
        pop eax
        inc edi
        dec eax
      .endw
        mov eax, ecx
        ret

a2dw endp

;This procedure attempts to locate the 'Content-Length' tag in a http header
;and convert the integer string that follows it into a dword.
;Possible return values are:
;eax = ERROR_BADPROTOCOL (Failed), or eax = #bytes expected in http payload 
parse_content_length proc pJob
LOCAL buf[512]:BYTE 
LOCAL _begin, _end

    DbgText "Parsing Content Length"
    mov eax,pJob
    DbgStr [eax].IOJob.xovl.pbuforig
	invoke StrPos,[eax].IOJob.xovl.pbuforig,$OfsCStr("Content-Length: ")
	.if eax==0
	    DbgWarning "Failed to find Content Length"
		mov eax, ERROR_BADPROTOCOL
	.else
	    add eax,16  ;sizeof "Content-Length: "
	    mov _begin,eax
		mov _end,$invoke (StrScan,_begin,13)
		.if _end==0
		    DbgWarning "Failed to find Content Length delimiter"
			mov eax, ERROR_BADPROTOCOL
		.else
		    ;Parse the content length and convert it to integer
		    ;and store it in the Client StateBlock
    		mov eax,_end
    		sub eax,_begin
    		inc eax
    		push eax
    		invoke lstrcpyn,addr buf,_begin,eax
    		pop eax
    		lea edx,buf
    		mov byte ptr[eax+edx],0
    		invoke a2dw,addr buf           		            		

		.endif
	    

	.endif	
    
    
    ret

parse_content_length endp






;Client class will call this method to determine if we have received
;at least one complete packet, given the rules of your Protocol.
;This is where your derived class examines the received data
;looking to find a valid Delimiter and doing Protocol enforcement.
;When you override this method in a derived class, you should return
;one of the following possible values:
;ERROR_BADPROTOCOL - The buffer content is not following our Protocol - the Client will get Booted for this offence
;ERROR_USERQUIT	   - The user sent a nice QUIT message using our Protocol - the Client will get Booted for this 
;Positive INT32	   - This many Bytes are part of the Complete Packet
;NULL			   - We expect more data to be Appended to this Job
Method UPNPNATProtocol.FindDelimiter,uses esi ebx,pClient,pXOVL
LOCAL payloadsize, headersize

    mov eax,pXOVL
	DbgDec [eax].XOVL.bytes,"bytes due to this recv"
	DbgDec [eax].XOVL.bytesused,"total bytes buffered in this job"
	DbgMem [eax].XOVL.pbuforig,[eax].XOVL.bytesused

    mov eax,pClient
    .if [eax].Client.bSocketIsUDP==TRUE
        ;The UDP packet is delimited by two CRLF's (13,10,13,10)
	    mov eax,pXOVL
	    mov ebx,[eax].XOVL.pbuforig
		invoke StrPos,ebx,$OfsCStr(13,10,13,10)
		.if eax!=0
		    ;We found the Delimiter - return #bytes in packet		    
		    mov edx,pXOVL
    		add eax,4
    		sub eax,[edx].XOVL.pbuforig
    		DbgWarning "UDP PACKET DELIMITER FOUND"
		.endif	 
	
	.else
    	mov eax,pXOVL
	    mov edx,[eax].XOVL.pbuforig
	    .if dword ptr [edx] == 'PTTH' && byte ptr[edx+4]=='/'
	        DbgText "IS HTTP"
	        ;Its a http Header
            ;We have received our first HTTP reply (header only)
            ;We should check for a '200 OK' message, then parse the http reply
            ;looking for the "content length" field.
    		
    		.if $invoke (StrPos,edx,$OfsCStr("200 OK")) == 0
    		    DbgWarning "Failed to find http ok"
    		    invoke StrPos,edx,$OfsCStr("100 Continue")
    		    .if eax==0
        			return ERROR_BADPROTOCOL
        		.else
            		DbgWarning "HAVE COMPLETE PACKET"
            		mov eax,pXOVL
            		mov eax,[eax].XOVL.bytesused
        	    .endif
        	.else       	    
        		mov eax, pXOVL
    	        mov payloadsize,$invoke (parse_content_length,[eax].XOVL.piojob)	       
    	        .if eax!=ERROR_BADPROTOCOL
    	            DbgDec eax,"Content Length (from HTTP header)"
    	            ;Calculate size of http header
            	    mov eax,pXOVL
            	    mov ebx,[eax].XOVL.pbuforig
            		invoke StrPos,ebx,$OfsCStr(13,10,13,10) ;<-- http header delimiter
            		.if eax!=0	
            		    mov edx,pXOVL    
            		    sub eax,[edx].XOVL.pbuforig    
            		    add eax,4                       ;eax = size of http header            		    
            		    mov headersize,eax
            		    DbgDec headersize
            		    mov edx,pXOVL
            		    DbgDec [edx].XOVL.bytesused
            		    mov edx,[edx].XOVL.bytesused    ;edx = size of available payload
            		    sub edx,eax                     ;edx = difference
            		    
            		    .if edx!=payloadsize
            		        DbgWarning "EXPECTING MORE DATA"
            		        DbgDec edx,"#bytes missing"
            		        mov eax,NULL
            		    .else
            		        DbgWarning "HAVE COMPLETE PACKET"
            		        mov eax,pXOVL
            		        mov eax,[eax].XOVL.bytesused
            		    .endif
                	.else
                	    ;Did not find delimiter
                	    mov eax,ERROR_BADPROTOCOL
            		.endif	 
            	.else
            	    DbgWarning "Failed to parse Content Length (from HTTP header)"
    	        .endif	
    		.endif
	

	    .else
	        ;Theres no http header - whats going on?
	        DbgWarning "Error - Expected HTTP Header"
	        int 3
	    .endif
	.endif
MethodEnd

;A client has finished sending us data,
;now we have to do something with that data.
;Make sure your derived method returns one of the following values:
;ERROR_BADPROTOCOL - The buffer content isnt following our Protocol - the Client will get Booted for this offence
;ERROR_USERQUIT	   - The user sent a nice QUIT message using our Protocol - the Client will get Booted for this 
;Positive INT32	   - This many Bytes were cleanly processed and should be cut from the IOJob's buffer
Method UPNPNATProtocol.ProcessReceivedData,uses esi edi ebx,pClientJobs
LOCAL bytes,_begin,_end
LOCAL pJob
local buf[1024]:BYTE
LOCAL buf2[256]:BYTE
    mov bytes,0
    mov edi,pClientJobs
	xor ebx,ebx

    SetObject esi
	.if [edi].Client.bSocketIsUDP==TRUE

;	.while ebx<[edi].DwordCollection.dCount
	    mov pJob,$OCall (edi::DwordCollection.ItemAt,ebx)
	    
		invoke StrPos,[eax].IOJob.xovl.pbuforig,$OfsCStr("200 OK")
		.if eax==0
		    DbgWarning "Failed to find http ok"
			return ERROR_BADPROTOCOL
		.endif	 
        mov eax,pJob
		mov _begin,$invoke (StrPos,[eax].IOJob.xovl.pbuforig,$OfsCStr("http://"))
		.if eax==0
		    DbgWarning "Failed to find url of upnp device controller"
			return ERROR_BADPROTOCOL
		.endif
		mov _end,$invoke (StrScan,_begin,13)
		.if _end==0
		    DbgWarning "Failed to delimit url of upnp device controller"
			return ERROR_BADPROTOCOL
		.endif
		
		mov eax,_end
		sub eax,_begin
		push eax
		inc eax
		invoke lstrcpyn,addr [esi].describe_url,_begin,eax
		pop eax
		lea edx,[esi].describe_url  ;found the upnp device controller url 
		mov byte ptr[eax+edx],0
		
		;We shall now parse the upnp device controller url for the ip and port
		add _begin,7 ;sizeof "http://"		
		mov _end,$invoke (StrScan,_begin,':')
		.if _end==0
		    DbgWarning "Failed to find ip delimiter"
			return ERROR_BADPROTOCOL
		.endif

		mov eax,_end
		sub eax,_begin
		push eax
		inc eax
		invoke lstrcpyn,addr buf,_begin,eax
		pop eax
		inc eax
		add _begin,eax
		lea edx,buf
		mov byte ptr[eax+edx],0

		invoke lstrcpy, addr NAT_IP_String,edx
		;DbgStr NAT_IP_String

		mov _end,$invoke (StrScan,_begin,'/')
		.if _end==0
		    DbgWarning "Failed to find port delimiter"
			return ERROR_BADPROTOCOL
		.endif
		
		mov eax,_end
		sub eax,_begin
		inc eax
		push eax
		invoke lstrcpyn,addr buf2,_begin,eax
		pop eax
		lea edx,buf2
		mov byte ptr[eax+edx],0

		
		mov upnp_tcp_port,$invoke (a2dw,addr buf2)
		
		;Create a TCP connection 
	    invoke inet_addr,addr NAT_IP_String 
		OCall [esi].pOwner::NetEngine.Connect,esi,eax,upnp_tcp_port  
		
    	;Terminate the UDP session    	
    	mov eax,ERROR_BADPROTOCOL

    	
;        inc ebx
;    .endw
    .else
        ;We have received some TCP data from the upnp device controller
        ;This protocol handler will make the dangerous assumption
        ;that a http protocol message ALWAYS fits inside a single IOJob buffer
        ;(ie, is not so large that multiple IOJobs are required to hold it)
        mov pJob,$OCall (edi::DwordCollection.ItemAt,ebx)        
        
        DbgDec [esi].Discovery_Stage,,"stage"
        .switch [esi].Discovery_Stage
        .case 1
            DbgWarning "Received XML reply"
            inc [esi].Discovery_Stage
            ;If stage = 0 , its xml data describing the UPNP Gateway device
            
            ;We need to find this : <deviceType>urn:schemas-upnp-org:device:WANConnectionDevice:1</deviceType>
            ;and then this:         <serviceType>urn:schemas-upnp-org:service:WANIPConnection:1</serviceType>
            ;and finally this:      <controlURL>HERES THE URL WE NEED</controlURL>

            
            mov eax,pJob
            m2m _begin,[eax].IOJob.xovl.pbuforig,edx
            ;DbgStr _begin
            

            mov _begin,$invoke (StrPos,_begin,addr szfirst)
            .if eax==0
        		DbgWarning "Failed to find WANConnectionDevice devicetype"
        		return ERROR_BADPROTOCOL
            .else
                add _begin,sizeof szfirst-1                    
        		mov _begin,$invoke (StrPos,_begin,addr szsecond)
        		.if eax==0
        		    DbgWarning "Failed to find WANIPConnection servicetype"
        			return ERROR_BADPROTOCOL
        	    .else
        	        add _begin,sizeof szsecond-1
        	        mov _begin,$invoke (StrPos,_begin,$OfsCStr("<controlURL>"))
        	        .if eax==0
        		        DbgWarning "Failed to find controlURL"
        		    	return ERROR_BADPROTOCOL
        	        .else
        	            add _begin,12   ;sizeof <controlURL>
        	        .endif
        	        
                    mov _end,$invoke (StrScan,_begin,3Ch)   ;'<'
            		.if _end==0
            		    DbgWarning "Failed to find controlURL delimiter"
            			mov eax, ERROR_BADPROTOCOL
            		.else
            		        ;Good, we found the control url
                		    mov eax,_end
                		    sub eax,_begin
                		    push eax
                		    inc eax            		     
                		    invoke lstrcpyn,addr [esi].controlUrl,_begin,eax
                		    pop eax
                		    lea edx,[esi].controlUrl
                		    mov byte ptr [eax+edx],0
                		    DbgWarning "Found upnp Control URL"
                            
                            mov eax,[esi].pOwner
                            mov [eax].NetEngine.bLocalMachineIsBehindNATDevice,TRUE
                            invoke SetEvent, [eax].NetEngine.hWake
                            
                            mov eax,ERROR_USERQUIT
           		
            		.endif                		
    		    .endif
            .endif
            
        .case 2
            ;Port Mapping has been successfully added
            DbgWarning "SUCCESSFULLY ADDED A PORT FORWARDING"
            mov eax,ERROR_BADPROTOCOL   ;not really error, just wanna terminate session
            
        .default
            DbgWarning "Client Stage not handled yet"
            DbgDec [esi].Discovery_Stage
            mov eax,ERROR_BADPROTOCOL
        .endsw
        

    .endif

MethodEnd

;When the secondary (tcp) client's attempt to connect to the gateway device has completed,
;we will be notified via this method.
;If dError is non-zero, the connect attempt failed.
Method UPNPNATProtocol.OnConnectCompleted,uses esi ,pClient,dError
LOCAL request[512]:BYTE
LOCAL buf[512]:BYTE
    SetObject esi
    .if dError==0
        DbgDec [esi].Discovery_Stage,"UPNPNATProtocol.OnConnectCompleted"
        .switch [esi].Discovery_Stage
        .case 0
            inc [esi].Discovery_Stage
        	DbgWarning "UPNPNATProtocol: Outbound Client has Connected to remote Host"
            ;We just made our first connect to the upnp device on its 'control port'            
            ;Let's build our first request packet and send it.
        	mov request[0],0
        	invoke wsprintf,addr request, addr httpreqtemplate,addr [esi].describe_url,addr NAT_IP_String, upnp_tcp_port	        	
        	DbgWarning "Sending Request for more information about the upnp device controller"
        	invoke lstrlen,addr request
            OCall pClient::Client.QueueWrite,addr request,eax       

        .case 2
            ;We just made a secondary connect to the upnp controller
            ;which means we're trying to add a new port mapping
            invoke SetEvent,[esi].hAddPortMapping
            DbgWarning "ADDING PORT MAPPING"
            
        .default
            DbgDec eax, "Unexptected stage state"
 
        .endsw
    .else
        DbgWarning "UPNPNATProtocol: Failed to connect to upnp device controller."
    .endif
MethodEnd

Method UPNPNATProtocol.OnWriteCompleted,uses esi,pClient,pXOVL,bytes
    SetObject esi
    mov eax,pClient
    .if [eax].Client.bSocketIsUDP==TRUE
        DbgWarning "UPNPNATProtocol : UDP BROADCAST SENT"
    .else
        DbgWarning "UPNPNATProtocol : WRITE COMPLETED"
    .endif
    
MethodEnd

Method UPNPNATProtocol.OnDisconnected,uses esi,pClient,dError
    DbgWarning "Unexpected termination of client session"
    int 3
    mov eax,FALSE
MethodEnd

Method UPNPNATProtocol.AddPortMapping,uses esi, dPort, bWantUDP:BOOL
LOCAL buf[2048]:BYTE
LOCAL body,request
LOCAL localipstring
LOCAL pclient
    DbgWarning "Adding a Port Mapping"    
    SetObject esi
    
    mov [esi].hAddPortMapping,$invoke(CreateEvent,0,FALSE,FALSE,0)
    
    invoke inet_addr,addr NAT_IP_String
	mov pclient,$OCall ([esi].pOwner::NetEngine.Connect,esi,eax,upnp_tcp_port)
    .if eax!=0
        invoke WaitForSingleObject,[esi].hAddPortMapping,20000
        .if eax==WAIT_OBJECT_0
            DbgWarning "WAIT IS OVER"
            invoke gethostname,addr buf,sizeof buf
            .if eax==0
                invoke gethostbyname,addr buf
                .if eax!=0    
                    mov eax,[eax+12]
                    mov eax,[eax]
                    mov eax,[eax]        
                    mov localipstring,$invoke (inet_ntoa,eax)
                    .if eax!=0
                        ;start building the request
                        mov request,$MemAlloc(16384)
                        mov body,$MemAlloc(16384)
                        mov byte ptr[eax],0
                        
                        ;First we build the 'body' , because we need its Length
                        .if bWantUDP==TRUE
                            mov edx,$OfsCStr("UDP")
                        .else
                            mov edx,$OfsCStr("TCP")
                        .endif
                        DbgWarning "BUILDING REQUEST"
                        invoke wsprintf,addr buf,addr szAPMBodyB, dPort, edx, dPort, localipstring
                        invoke lstrcpy,body,addr szAPMBodyA;,addr szExternalIP
                        invoke lstrcat,body,addr buf
                        ;would prefer to use the application name here
                        invoke wsprintf,addr buf,addr szAPMBodyC, $OfsCStr("NetEngine")
                        invoke lstrcat, body,addr buf
                        ;Now format the header
                        invoke lstrlen, body
                        invoke wsprintf, request, addr szAPMHead,addr [esi].controlUrl,addr NAT_IP_String,upnp_tcp_port,eax
                        invoke lstrcat,  request, body
                        invoke lstrlen,  request
                        DbgMem request, eax
                        
                        ;We are now ready to create a new client session
                        ;in order to send this request to the upnp controller
                        OCall pclient::Client.QueueWrite,request,eax
                        
                        MemFree request
                        MemFree body
                    .else
                        DbgWarning "Error - inet_ntoa failed"
                    .endif
                .else
                    DbgWarning "Error - gethostbyname failed"
                .endif
            .else
                DbgWarning "Error - gethostname failed"
            .endif
        .else
            DbgWarning "Error - UPNP.AddPortMapping timed out"
        .endif
    .endif
    
    invoke CloseHandle,[esi].hAddPortMapping


MethodEnd