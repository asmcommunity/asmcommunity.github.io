

EchoClientProtocolID equ 25774
Object EchoClientProtocol,EchoClientProtocolID,NetworkProtocol
  RedefineMethod    OnConnectCompleted ,Pointer,dword
  RedefineMethod    OnReadCompleted,Pointer,Pointer,dword,dword
ObjectEnd

if IMPLEMENT
; ——————————————————————————————————————————————————————————————————————————————————————————————————

Method EchoClientProtocol.OnConnectCompleted, ,pClient,dError
    SetObject esi
    .if dError==0
        DbgWarning "Client has connected"
        ;Enable the Send button
        invoke GetDlgItem,[esi].pOwner,1003
        invoke EnableWindow,eax,TRUE
    .else
        DbgWarning "Connect FAILED"
    .endif
MethodEnd

;A recv job has completed - it may be a PARTIAL receive
;so this method is really just a courtesy notification
;Normally, we would override the ProcessReceivedData method,
;and just use this one to flash a LED icon or something.
Method EchoClientProtocol.OnReadCompleted,uses esi ebx, pClient,pXOVL, bytes, dError
    .if dError==0
        DbgWarning "INCOMING DATA"
        SetObject esi
        ;Ensure the buffer is zeroterminated
        ;Otherwise we'll print junk
    	mov eax,pXOVL
    	mov edx, [eax].XOVL.pbuforig
    	add edx, bytes
    	mov byte ptr[edx],0
    	;Show the data we received
        invoke SetDlgItemText,[esi].pOwner,1002, [eax].XOVL.pbuforig
    .endif
MethodEnd

; ——————————————————————————————————————————————————————————————————————————————————————————————————
endif

