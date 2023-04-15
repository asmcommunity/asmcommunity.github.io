

EchoServerProtocolID equ 25773
Object EchoServerProtocol,EchoServerProtocolID,NetworkProtocol
  RedefineMethod    ProcessReceivedData,        Pointer                  ;pClient
ObjectEnd

if IMPLEMENT
; ——————————————————————————————————————————————————————————————————————————————————————————————————


;We'll simply echo data back via whichever Client received it
Method EchoServerProtocol.ProcessReceivedData,uses esi edi,pClientJobs
LOCAL bytes
    mov bytes,0
    mov edi,pClientJobs
    xor ebx,ebx
    .while ebx<[edi].DwordCollection.dCount
        OCall edi::DwordCollection.ItemAt,ebx
        OCall pClientJobs::Client.QueueWrite,[eax].IOJob.xovl.pbuforig,[eax].IOJob.xovl.bytes
        mov eax,[eax].IOJob.xovl.bytes
        add bytes,eax
        inc ebx
    .endw
    mov eax,bytes
MethodEnd

Method EchoClient.OnConnectCompleted, ,pClient,dError
    
MethodEnd
endif

